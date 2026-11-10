#include "retrieve.h"
#include "../auth/auth.h"
#include <FL/Fl_Progress.H>
#include <fcntl.h>
#include <filesystem>
#include <libssh/sftp.h>
#include "../frontend/gui.h"

void retrieve_file(ssh_session session, fs::path remote_path,
                   fs::path local_path, AppContext* ctx) {
  Fl_Progress* bar = ctx->prog_bar;
  // open sftp_session
  sftp_session serv = open_sftp_session(session, ctx);

  if (fs::is_directory(remote_path)) {
    fs::create_directories(local_path);
    sftp_dir dir = sftp_opendir(serv, remote_path.generic_string().c_str());
    sftp_attributes attr;
    while ((attr = sftp_readdir(serv, dir)) != NULL) {

      std::string item_name = attr->name;
      fs::path next_remote = (remote_path / item_name);
      fs::path next_local = local_path / item_name;
      if (item_name == "." || item_name == "..") {
        sftp_attributes_free(attr);
        continue;
      }
      // directory: recurse into retrieve_file again
      if (attr->type == SSH_FILEXFER_TYPE_DIRECTORY) {
        safe_thread_log(ctx, "Recursing into sub-directory");
        retrieve_file(session, next_remote, local_path, ctx);
      }
      // file: recurse into retrieve file with is_dir = false
      else if (attr->type == SSH_FILEXFER_TYPE_REGULAR) {
        safe_thread_log(ctx, "Retrieving remote file");
        retrieve_file(session, next_remote, local_path, ctx);
      }
    }
    //sftp_closedir(dir);
    return;
  }
  send_status_to_ui(bar, 45.0f);

  // normal retrieve functionality

  // check if the file exists on the remote machine
  sftp_attributes attr = sftp_stat(serv, remote_path.generic_string().c_str());
  // file does not exist
  int err = sftp_get_error(serv);
  if (err == SSH_FX_NO_SUCH_FILE || err == SSH_FX_NO_SUCH_PATH) {
    safe_thread_log(ctx, " No such file or path exists; closing connection");

    return;
  }

  // exists
  if (attr != NULL) {
    sftp_attributes_free(attr);
  }

  // check that local_path is a directory, not a file

  if (!(fs::is_directory(local_path))) {
    safe_thread_log(ctx, "Local path is not a directory; closing connection");

    return;
  }


  // proceed normally
  send_status_to_ui(bar, 50.0f);

  sftp_file sftp_file =
      sftp_open(serv, remote_path.generic_string().c_str(), O_RDONLY, 0);

  if (!sftp_file) {
    std::string err = std::string("Open failed for ") + remote_path.generic_string() + ": " + ssh_get_error(session) + std::string(" (sftp code ") + std::to_string(sftp_get_error(serv)) + ")";
    safe_thread_log(ctx, err);
    return;
  }
  send_status_to_ui(bar, 60.0f);

  // open local file
  local_path /= remote_path.filename();
  safe_thread_log(ctx, "Opening the remote file.");
  FILE *local_file = fopen(local_path.generic_string().c_str(), "wb");
  if (local_file == NULL) {
    safe_thread_log(ctx, "Failed to open the local file; closing connection");

    return;
  }

  // write file
  safe_thread_log(ctx, "Reading from remote file. Writing to local file.");
  char buf[32768]; // 32KB buffer for writing
  ssize_t file;
  while ((file = sftp_read(sftp_file, buf, sizeof(buf))) > 0) {
    size_t nwritten = fwrite(buf, 1, file, local_file);
    if (nwritten != (size_t)file) {
      safe_thread_log(ctx, "Error writing to file; closing connection");
      fclose(local_file);

      return;
    }
  }
  if (file < 0) {
    std::string err = std::string("Error reading remote file: ") + ssh_get_error(session);
    safe_thread_log(ctx, err);
    fclose(local_file);

    return;
  }
  send_status_to_ui(bar, 80.0f);
  safe_thread_log(ctx, "Successfully wrote to file!");

  std::string loc_path = local_path.generic_string();
  if (loc_path.back() != '/') {
    loc_path += "/";
  }

  // clean up
  sftp_close(sftp_file);
  sftp_free(serv);
  fclose(local_file);
  send_status_to_ui(bar, 100.0f);
}

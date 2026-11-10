#include "upload.h"
#include "../auth/auth.h"
#include <FL/Fl_Progress.H>
#include <fcntl.h>
#include <filesystem>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <windows.h>
#include "../frontend/gui.h"

void upload_file(ssh_session session, fs::path local_path, fs::path remote_path, AppContext* ctx) {
    Fl_Progress* bar = ctx->prog_bar;
  if (fs::is_directory(local_path)) {
    for (const auto &entry : fs::directory_iterator(local_path)) {
      fs::path local_item = entry.path();
      std::string item_name = local_item.filename().generic_string();

      if (fs::is_directory(local_item)) {
          safe_thread_log(ctx, "Recursing into sub-directory: ");
        upload_file(session, local_item, remote_path, ctx);
      } else if (fs::is_regular_file(local_item)) {
          safe_thread_log(ctx, "Uploading nested file: ");
        upload_file(session, local_item, remote_path, ctx);
      }
    }
    return;
  }
  send_status_to_ui(bar, 45.0f);
  // check local file
  if (!(fs::exists(local_path) && fs::is_regular_file(local_path))) {
      std::string err = std::string("Invalid local file: ") + local_path.string().c_str();
    safe_thread_log(ctx, err);
    return;
  } else {
      std::string err = std::string("Local file: ") + local_path.string().c_str() + "exists";
    safe_thread_log(ctx, err);
    send_status_to_ui(bar, 50.0f);

    // open file
    std::ifstream file(local_path, std::ios::in | std::ios::binary);

    if (!file) { // check if file was opened
        safe_thread_log(ctx, "Error opening file for reading");
      return;
      return;
    }

    // open sftp
    sftp_session serv = open_sftp_session(session, ctx);
    if (!(serv)) { // confirm valid sftp session
      return;
      return;
    }

    // remote path check
    fs::path remote_file_path = remote_path;
    if (remote_path.string() == "./" || remote_path.string() == "." || remote_path.filename().empty()) {
            remote_file_path /= local_path.filename(); // Use your local file's name variable here
        }

    sftp_attributes attr =
        sftp_stat(serv, remote_file_path.generic_string().c_str());

    if (attr != NULL) {
      if (attr->flags &
          (SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_PERMISSIONS)) {

        // Check if it matches a regular file or directory type
        if (attr->type == SSH_FILEXFER_TYPE_REGULAR ||
            attr->type == SSH_FILEXFER_TYPE_DIRECTORY) {
          sftp_attributes_free(attr);

          int res = showOverwriteAlert();

          if (res != 1) {
            sftp_free(serv);
            return;
            return;
          }
        } else {
          sftp_attributes_free(attr);
        }
      } else {
        sftp_attributes_free(attr);
      }
    }

    std::string rem_path = remote_path.generic_string();
    if (rem_path.back() != '/') {
      rem_path += "/";
    }
    rem_path += local_path.filename().generic_string();
    std::string temp_path = remote_path.generic_string() + "/." +
                            local_path.filename().generic_string() + ".part";

    sftp_file sftp_file =
        sftp_open(serv, temp_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (!sftp_file) {
        std::string err = std::string("Open failed for ") + temp_path + ": " + ssh_get_error(session) + std::string(" (sftp code ") + std::to_string(sftp_get_error(serv)) + ")";
        safe_thread_log(ctx, err);
      return;
    }
    send_status_to_ui(bar, 60.0f);

    // write file
    safe_thread_log(ctx, "Writing to file.");
    std::vector<char> buf(32 * 1024); // 32KB buffer for writing

    while (file) {
      file.read(buf.data(), buf.size());
      std::streamsize n = file.gcount();
      if (n > 0) {
        size_t sent = 0;
        while (sent < (size_t)n) {
          ssize_t w = sftp_write(sftp_file, buf.data() + sent, n - sent);
          if (w < 0) {
              std::string err = std::string("Error writing file: ") + ssh_get_error(session);
              safe_thread_log(ctx, err);
            sftp_close(sftp_file);
            return;
          }
          sent += w;
        }
      }
    }
    send_status_to_ui(bar, 80.0f);
    safe_thread_log(ctx, "Successfully wrote to file!");

    // close sftp session
    sftp_close(sftp_file);
    // rename file after writing
    if (sftp_rename(serv, temp_path.c_str(), rem_path.c_str()) != SSH_OK) {
        std::string err = std::string("Error renaming file: ") + ssh_get_error(session);
        safe_thread_log(ctx, err);
    }
    safe_thread_log(ctx, "File was renamed to the correct name after complete transfer.");

    send_status_to_ui(bar, 100.0f);
    // clean up
    sftp_free(serv);
  }
}

sftp_session open_sftp_session(ssh_session session, AppContext* ctx) {

  sftp_session sftp = sftp_new(session);
  if (sftp == nullptr) {
      std::string err = std::string("Error allocating SFTP session: ") + ssh_get_error(session);
      safe_thread_log(ctx, err);
    return NULL;
  }

  int rc = sftp_init(sftp);
  if (rc != SSH_OK) {
      std::string err = std::string("Error initializing SFTP session: ") + std::to_string(sftp_get_error(sftp));
    sftp_free(sftp);
    return NULL;
  }

  // valid init
  safe_thread_log(ctx, "SFTP session initialized successfully!");

  return sftp;
}

#include "upload.h"
#include "../auth/auth.h"
#include <FL/Fl_Progress.H>
#include <fcntl.h>
#include <filesystem>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <windows.h>
#include <string>
#include "../frontend/gui.h"

void upload_file(ssh_session session, fs::path local_path, fs::path remote_path, AppContext *ctx)
{
    if (!session || !ctx) return;

    Fl_Progress *bar = ctx->prog_bar;
    std::error_code ec;

    // 1. Handle directories recursively
    if (fs::is_directory(local_path, ec))
    {
        fs::path target_remote_dir = remote_path / local_path.filename();

        // Ensure the remote directory exists before uploading into it
        sftp_session serv = open_sftp_session(session, ctx);
        if (serv) {
            sftp_mkdir(serv, target_remote_dir.generic_string().c_str(), 0755); // Ignore error if it already exists
            sftp_free(serv);
        }

        for (const auto &entry : fs::directory_iterator(local_path, fs::directory_options::skip_permission_denied, ec))
        {
            fs::path local_item = entry.path();
            if (fs::is_directory(local_item, ec))
            {
                safe_thread_log(ctx, "Recursing into sub-directory: " + local_item.filename().string());
                upload_file(session, local_item, target_remote_dir, ctx);
            }
            else if (fs::is_regular_file(local_item, ec))
            {
                safe_thread_log(ctx, "Uploading nested file: " + local_item.filename().string());
                upload_file(session, local_item, target_remote_dir, ctx);
            }
        }
        return;
    }

    send_status_to_ui(bar, 45.0f);

    // 2. Validate local file
    if (!fs::exists(local_path, ec) || !fs::is_regular_file(local_path, ec))
    {
        safe_thread_log(ctx, "Invalid local file: " + local_path.string());
        return;
    }

    safe_thread_log(ctx, "Local file exists: " + local_path.string());
    send_status_to_ui(bar, 50.0f);

    std::ifstream file(local_path, std::ios::in | std::ios::binary);
    if (!file)
    {
        safe_thread_log(ctx, "Error opening file for reading: " + local_path.string());
        return;
    }

    // 3. Open SFTP session
    sftp_session serv = open_sftp_session(session, ctx);
    if (!serv)
    {
        return;
    }

    // 4. Safely normalize remote directory & destination file paths
    std::string rem_dir = remote_path.generic_string();
    if (rem_dir.empty()) {
        rem_dir = "."; // Prevent empty string crashes
    }
    // Strip trailing slash unless root "/"
    while (rem_dir.size() > 1 && rem_dir.back() == '/') {
        rem_dir.pop_back();
    }

    std::string filename = local_path.filename().generic_string();
    std::string rem_path = (rem_dir == "/") ? ("/" + filename) : (rem_dir + "/" + filename);
    std::string temp_path = (rem_dir == "/") ? ("/." + filename + ".part") : (rem_dir + "/." + filename + ".part");

    // 5. Check if destination file already exists
    sftp_attributes attr = sftp_stat(serv, rem_path.c_str());
    if (attr != NULL)
    {
        bool is_existing = (attr->type == SSH_FILEXFER_TYPE_REGULAR || attr->type == SSH_FILEXFER_TYPE_DIRECTORY);
        sftp_attributes_free(attr);

        if (is_existing)
        {
            // NOTE: Ensure showOverwriteAlert() is thread-safe if called from a worker thread!
            int res = showOverwriteAlert();
            if (res != 1)
            {
                sftp_free(serv);
                return;
            }
        }
    }

    // 6. Open remote .part file for writing
    sftp_file remote_handle = sftp_open(serv, temp_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (!remote_handle)
    {
        std::string err = "Open failed for " + temp_path + ": " + ssh_get_error(session) +
                          " (sftp code " + std::to_string(sftp_get_error(serv)) + ")";
        safe_thread_log(ctx, err);
        sftp_free(serv);
        return;
    }

    send_status_to_ui(bar, 60.0f);
    safe_thread_log(ctx, "Writing to file...");

    // 7. Stream file chunks
    std::vector<char> buf(32 * 1024);
    while (file)
    {
        file.read(buf.data(), buf.size());
        std::streamsize n = file.gcount();
        if (n > 0)
        {
            size_t sent = 0;
            while (sent < static_cast<size_t>(n))
            {
                ssize_t w = sftp_write(remote_handle, buf.data() + sent, static_cast<size_t>(n) - sent);
                if (w < 0)
                {
                    safe_thread_log(ctx, std::string("Error writing file: ") + ssh_get_error(session));
                    sftp_close(remote_handle);
                    sftp_free(serv);
                    return;
                }
                sent += static_cast<size_t>(w);
            }
        }
    }

    send_status_to_ui(bar, 80.0f);
    safe_thread_log(ctx, "Successfully wrote to file!");

    sftp_close(remote_handle);

    // 8. Rename .part file to final filename
    // Remove existing destination first because SFTP v3 sftp_rename fails if target exists
    sftp_unlink(serv, rem_path.c_str());

    if (sftp_rename(serv, temp_path.c_str(), rem_path.c_str()) != SSH_OK)
    {
        safe_thread_log(ctx, std::string("Error renaming file: ") + ssh_get_error(session));
    }
    else
    {
        safe_thread_log(ctx, "File renamed to final destination.");
    }

    send_status_to_ui(bar, 100.0f);
    sftp_free(serv);
}


sftp_session open_sftp_session(ssh_session session, AppContext *ctx)
{
  ssh_set_blocking(session, 1);
  sftp_session sftp = sftp_new(session);
  if (sftp == nullptr)
  {
    std::string err = std::string("Error allocating SFTP session: ") + ssh_get_error(session);
    safe_thread_log(ctx, err);
    return NULL;
  }

  int rc = sftp_init(sftp);
  if (rc != SSH_OK)
  {
    // std::string err = std::string("Error initializing SFTP session: ") + std::to_string(sftp_get_error(sftp));
    sftp_free(sftp);
    return NULL;
  }

  // valid init
  safe_thread_log(ctx, "SFTP session initialized successfully!");

  return sftp;
}
void populate_local_fl_tree(const std::string &local_path, const std::string &tree_prefix, Fl_Tree *tree, AppContext *ctx)
{
    std::error_code ec;
    fs::path dir_path = fs::weakly_canonical(local_path, ec);
    if (ec) dir_path = fs::path(local_path);

    if (!fs::exists(dir_path, ec) || !fs::is_directory(dir_path, ec))
    {
        safe_thread_log(ctx, "Local directory invalid or inaccessible: " + local_path);
        return;
    }

    tree->clear();

    // Add ".." entry if not already at the drive root (e.g., "C:\")
    if (dir_path.has_parent_path() && dir_path.parent_path() != dir_path)
    {
        std::string up_path = tree_prefix.empty() ? ".." : (tree_prefix + "/..");
        tree->add(up_path.c_str());
    }

    // List immediate folders and files (non-recursive so C:\ loads instantaneously)
    for (const auto &entry : fs::directory_iterator(dir_path, fs::directory_options::skip_permission_denied, ec))
    {
        std::string name = entry.path().filename().string();
        std::string item_tree_path = tree_prefix.empty() ? name : (tree_prefix + "/" + name);
        Fl_Tree_Item *item = tree->add(item_tree_path.c_str());

        // Optional: Add a trailing "/" visually on folders so they stand out from files
        if (item && entry.is_directory(ec))
        {
            item->label((name + "/").c_str());
        }
    }
}

void populate_remote_fl_tree(ssh_session session, sftp_session sftp, const std::string &remote_path, const std::string &tree_prefix, Fl_Tree *tree, AppContext *ctx)
{
    if (!session || !sftp || !tree) return;

    sftp_dir dir = sftp_opendir(sftp, remote_path.c_str());
    if (!dir)
    {
        safe_thread_log(ctx, "Failed to open remote directory '" + remote_path + "': " + ssh_get_error(session));
        return;
    }

    tree->clear();

    // Add ".." entry if we are not already at the root "/"
    if (remote_path != "/" && !remote_path.empty())
    {
        std::string up_path = tree_prefix.empty() ? ".." : (tree_prefix + "/..");
        tree->add(up_path.c_str());
    }

    sftp_attributes attr;
    while ((attr = sftp_readdir(sftp, dir)) != NULL)
    {
        std::string name = attr->name;
        bool is_dir = (attr->type == SSH_FILEXFER_TYPE_DIRECTORY);
        sftp_attributes_free(attr);

        if (name == "." || name == "..")
            continue;

        std::string item_tree_path = tree_prefix.empty() ? name : (tree_prefix + "/" + name);
        Fl_Tree_Item *item = tree->add(item_tree_path.c_str());

        // Visually mark directories with a trailing "/"
        if (item && is_dir)
        {
            item->label((name + "/").c_str());
        }
    }

    sftp_closedir(dir);
}

#include "retrieve.h"
#include "../auth/auth.h"
#include <FL/Fl_Progress.H>
#include <fcntl.h>
#include <filesystem>
#include <libssh/sftp.h>
#include "../frontend/gui.h"
#include <string>

static void retrieve_recursive(ssh_session session, sftp_session serv,
                               const fs::path &remote_path, const fs::path &local_dir,
                               AppContext *ctx)
{
    Fl_Progress *bar = ctx->prog_bar;
    std::string rem_str = remote_path.generic_string();

    // check if remote path exists and get type
    sftp_attributes attr = sftp_stat(serv, rem_str.c_str());
    if (!attr)
    {
        safe_thread_log(ctx, "Remote path not found: " + rem_str + " (" + ssh_get_error(session) + ")");
        return;
    }

    bool is_remote_dir = (attr->type == SSH_FILEXFER_TYPE_DIRECTORY);
    sftp_attributes_free(attr);

    // directory: create local folder and recurse
    if (is_remote_dir)
    {
        fs::path target_local_dir = local_dir / remote_path.filename();
        std::error_code ec;
        fs::create_directories(target_local_dir, ec);

        sftp_dir dir = sftp_opendir(serv, rem_str.c_str());
        if (!dir)
        {
            safe_thread_log(ctx, "Failed to open remote directory: " + rem_str);
            return;
        }

        sftp_attributes entry;
        while ((entry = sftp_readdir(serv, dir)) != NULL)
        {
            std::string item_name = entry->name;
            sftp_attributes_free(entry); // free attributes

            if (item_name == "." || item_name == "..")
                continue;

            retrieve_recursive(session, serv, remote_path / item_name, target_local_dir, ctx);
        }

        sftp_closedir(dir);
        return;
    }

    // check that local_dir is a directory, not a file
    std::error_code ec;
    if (!fs::exists(local_dir, ec) || !fs::is_directory(local_dir, ec))
    {
        safe_thread_log(ctx, "Local destination is not a valid directory: " + local_dir.string());
        return;
    }

    // proceed normally
    send_status_to_ui(bar, 50.0f);

    // open remote file
    sftp_file remote_handle = sftp_open(serv, rem_str.c_str(), O_RDONLY, 0);
    if (!remote_handle)
    {
        std::string err = "Open failed for " + rem_str + ": " + ssh_get_error(session) +
                          " (sftp code " + std::to_string(sftp_get_error(serv)) + ")";
        safe_thread_log(ctx, err);
        return;
    }
    send_status_to_ui(bar, 60.0f);

    // open local file
    fs::path local_file_path = local_dir / remote_path.filename();
    safe_thread_log(ctx, "Downloading to: " + local_file_path.string());

    FILE *local_file = fopen(local_file_path.string().c_str(), "wb");
    if (!local_file)
    {
        safe_thread_log(ctx, "Failed to open local file for writing: " + local_file_path.string());
        sftp_close(remote_handle);
        return;
    }

    // write file
    char buf[32768]; // 32KB buffer for reading
    ssize_t bytes_read;
    while ((bytes_read = sftp_read(remote_handle, buf, sizeof(buf))) > 0)
    {
        size_t nwritten = fwrite(buf, 1, static_cast<size_t>(bytes_read), local_file);
        if (nwritten != static_cast<size_t>(bytes_read))
        {
            safe_thread_log(ctx, "Error writing to local file.");
            fclose(local_file);
            sftp_close(remote_handle);
            return;
        }
    }

    if (bytes_read < 0)
    {
        safe_thread_log(ctx, std::string("Error reading remote file: ") + ssh_get_error(session));
        fclose(local_file);
        sftp_close(remote_handle);
        return;
    }

    send_status_to_ui(bar, 80.0f);
    safe_thread_log(ctx, "Successfully downloaded: " + remote_path.filename().string());

    // clean up
    fclose(local_file);
    sftp_close(remote_handle);
    send_status_to_ui(bar, 100.0f);
}

void retrieve_file(ssh_session session, fs::path remote_path,
                   fs::path local_path, AppContext *ctx)
{
    if (!session || !ctx) return;

    // open sftp_session
    sftp_session serv = open_sftp_session(session, ctx);
    if (!serv)
    {
        safe_thread_log(ctx, "Failed to open SFTP session for download.");
        return;
    }

    retrieve_recursive(session, serv, remote_path, local_path, ctx);

    // clean up
    sftp_free(serv);
}

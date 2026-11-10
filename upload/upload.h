#ifndef UPLOAD_H
#define UPLOAD_H
#include <iostream>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

//forward declaration
struct AppContext;

void upload_file(ssh_session session, fs::path local_path, fs::path remote_path, AppContext* ctx);
sftp_session open_sftp_session(ssh_session session, AppContext* ctx);


#endif

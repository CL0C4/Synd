#ifndef RETRIEVE_H
#define RETRIEVE_H
#include <iostream>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <filesystem>
#include <fstream>
#include <vector>

// include upload for open_sftp_session
#include "../upload/upload.h"

//forward declaration
struct AppContext;

namespace fs = std::filesystem;

void retrieve_file(ssh_session session, fs::path remote_path, fs::path local_path, AppContext* ctx);



#endif

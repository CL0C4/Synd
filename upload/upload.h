#ifndef UPLOAD_H
#define UPLOAD_H
#include <iostream>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <FL/Fl_Tree.H>

namespace fs = std::filesystem;

//forward declaration
struct AppContext;

void upload_file(ssh_session session, fs::path local_path, fs::path remote_path, AppContext* ctx);
sftp_session open_sftp_session(ssh_session session, AppContext* ctx);
void populate_local_fl_tree(const std::string &local_path, const std::string &tree_prefix, Fl_Tree *tree, AppContext *ctx);
void populate_remote_fl_tree(ssh_session session, sftp_session sftp, const std::string& remote_path, const std::string& tree_prefix, Fl_Tree* tree, AppContext* ctx);


#endif

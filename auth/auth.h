#pragma once

#ifndef WIN32_LEAN_AND_MEAN // required to get rid of 'byte' typedef error
#define WIN32_LEAN_AND_MEAN
#endif

#include <cstddef>
#include <winsock2.h>

#include <iostream>
#include <libssh/libssh.h>
#include <stdio.h>
#include <stdlib.h>

//forward declaration
struct AppContext;

ssh_session authenticate_connection(std::string s, bool is_cloudflare, std::string password, AppContext* ctx);
bool validate_host(ssh_session session, AppContext* ctx);
void exit_on_error(ssh_session& s);

void stop_tunnel();

// password
void secureClearString(std::string& str);
std::string getHiddenInput(const std::string& prompt);

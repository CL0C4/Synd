#include "../frontend/gui.h"

#include "../parser/parser.h"
#include "auth.h"
#include <cstdio> // For _popen / popen
#include <string>
#include <vector>


#include <libssh/libssh.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <consoleapi.h>
#include <consoleapi2.h>
#include <handleapi.h>
#include <processenv.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#define popen_cmd _popen
#define pclose_cmd _pclose
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#define popen_cmd popen
#define pclose_cmd pclose
#define INVALID_SOCKET -1
#define closesocket close

// Only apply the integer fallback for POSIX environments
typedef int SOCKET;

// Define Win32 placeholders to prevent compilation breaking on Linux/macOS
typedef void *HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)(long long)-1)
#define STD_INPUT_HANDLE 0
#define DWORD unsigned long
#define ENABLE_ECHO_INPUT 0
inline bool GetStdHandle(int) { return false; }
inline bool GetConsoleMode(HANDLE, DWORD *) { return false; }
inline bool SetConsoleMode(HANDLE, DWORD) { return false; }
#endif

// Forward declarations
bool validate_host(ssh_session session);
void secureClearString(std::string &str);
std::string getHiddenInput(const std::string &prompt);

static HANDLE g_tunnel_proc = NULL;

bool start_tunnel(const std::string &cmdline_in) {
  SECURITY_ATTRIBUTES sa{};
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;
  HANDLE nul = CreateFileA("NUL", GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, &sa,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  STARTUPINFOA si{};
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdInput = nul;
  si.hStdOutput = nul;
  si.hStdError = nul;

  PROCESS_INFORMATION pi{};
  std::string cmdline = cmdline_in;   // CreateProcessA needs a writable buffer
  BOOL ok = CreateProcessA(NULL, cmdline.data(), NULL, NULL, TRUE,
                           CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
  if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
  if (!ok) return false;

  CloseHandle(pi.hThread);
  g_tunnel_proc = pi.hProcess;
  return true;
}

void stop_tunnel() {
  if (g_tunnel_proc) {
    TerminateProcess(g_tunnel_proc, 0);
    WaitForSingleObject(g_tunnel_proc, 2000);
    CloseHandle(g_tunnel_proc);
    g_tunnel_proc = NULL;
  }
}

// exit function for simplicity
void exit_on_error(ssh_session &s) {
  if (s) {
    ssh_disconnect(s);
    ssh_free(s);
    s = NULL;
  }
}

ssh_session authenticate_connection(std::string s, bool is_cloudflare,
                                    std::string password, AppContext *ctx) {


  std::vector<std::string> parsed = parse_address(s);
  std::string user, host, port;
  if (parsed.size() > 2) {
    user = parsed[0];
    host = parsed[1];
    port = parsed[2];
  } else {
    safe_thread_log(ctx, "Failed to parse address");
    return NULL;
  }

#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    safe_thread_log(ctx, "WinSock initialization failed.");
    return NULL;
  }
#endif

  ssh_session test_session = NULL;
  int rc;

  test_session = ssh_new();
  if (test_session == NULL) {
    return NULL;
  }

  // --- This must execute for BOTH Standard and Cloudflare modes ---
  ssh_options_set(test_session, SSH_OPTIONS_HOST, host.c_str());
  ssh_options_set(test_session, SSH_OPTIONS_USER, user.c_str());
  ssh_options_set(test_session, SSH_OPTIONS_PORT_STR, port.c_str());

  SOCKET cloudflare_sock = INVALID_SOCKET;
  FILE *tunnel_process = nullptr;

  Fl_Progress* bar = ctx->prog_bar;

  if (is_cloudflare) {
    safe_thread_log(ctx,
                    "[Tunnel Mode] Spawning cloudflared process silently...");
    send_status_to_ui(bar, 10.0f);
    int local_proxy_port = 2222;
    char exe[MAX_PATH];
    GetModuleFileNameA(NULL, exe, MAX_PATH);
    std::string app_dir = std::filesystem::path(exe).parent_path().string();

    std::string cf_path = app_dir + "\\cloudflared.exe";
    if (!std::filesystem::exists(cf_path)) {
        char found[MAX_PATH];
        if (SearchPathA(NULL, "cloudflared.exe", NULL, MAX_PATH, found, NULL) == 0) {
            safe_thread_log(ctx, "cloudflared not found. Place cloudflared.exe next to synd.exe or add it to PATH.");
            exit_on_error(test_session);
            return NULL;
        }
        cf_path = found;
    }

    std::string cmd = "\"" + cf_path + "\" access tcp --hostname " + host +
                      " --listener 127.0.0.1:" + std::to_string(local_proxy_port);

    if (!start_tunnel(cmd)) {
      safe_thread_log(ctx, "Error: Could not launch cloudflared binary.");
      exit_on_error(test_session);
      return NULL;
    }

#ifdef _WIN32
    Sleep(1500);
#else
    usleep(1500000);
#endif

    // Establish raw socket to the local loopback proxy
    cloudflare_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (cloudflare_sock != INVALID_SOCKET) {
      sockaddr_in clientService;
      clientService.sin_family = AF_INET;
      clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
      clientService.sin_port = htons(local_proxy_port);

      if (connect(cloudflare_sock, (sockaddr *)&clientService,
                  sizeof(clientService)) >= 0) {
// 1. Point libssh to an empty folder context to prevent config parsing crashes
#ifdef _WIN32
        const char *empty_dir = "C:\\Windows\\Temp";
#else
        const char *empty_dir = "/tmp";
#endif
        ssh_options_set(test_session, SSH_OPTIONS_SSH_DIR, empty_dir);

        // 2. Supply the connected socket descriptor
        ssh_options_set(test_session, SSH_OPTIONS_FD, &cloudflare_sock);
        safe_thread_log(
            ctx,
            "[Tunnel Mode] Spliced automated Cloudflare socket into session.");
        send_status_to_ui(bar, 15.0f);
      } else {
        safe_thread_log(
            ctx, "Error: Could not reach automated cloudflared agent on port ");
        stop_tunnel();
        exit_on_error(test_session);
        return NULL;
      }
    }
  } else {
    safe_thread_log(
        ctx,
        "[Standard Mode] Routing connection normally over standard internet.");
    send_status_to_ui(bar, 15.0f);
  }

  // Keep verbosity high to trace the exact frame exchanges
  int verbosity = SSH_LOG_PROTOCOL;
  ssh_options_set(test_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

  bool process_config = false;
  ssh_options_set(test_session, SSH_OPTIONS_PROCESS_CONFIG, &process_config);

  rc = ssh_connect(test_session);

  if (rc != SSH_OK) {
    std::string err = std::string("Error: ") + ssh_get_error(test_session);
    safe_thread_log(ctx, err);
    stop_tunnel();
    exit_on_error(test_session);
    return NULL;
  } else {
    safe_thread_log(ctx, "Successful connection... attempting authentication.");
    send_status_to_ui(bar, 28.0f);
  }

  if (!validate_host(test_session, ctx)) {
    stop_tunnel();
    exit_on_error(test_session);
    return NULL;
  }

  // Password prompt capture
  std::string pass = password;

  int auth_rc = ssh_userauth_password(test_session, NULL, pass.c_str());

  if (auth_rc != SSH_AUTH_SUCCESS) {
    std::string err =
        std::string("Authentication failed: ") + ssh_get_error(test_session);
    safe_thread_log(ctx, err);

    if (auth_rc == SSH_AUTH_INFO) {
      safe_thread_log(ctx, "Server requires keyboard-interactive authentication.");
    }

    stop_tunnel();
    exit_on_error(test_session);
    return NULL;
  }
  safe_thread_log(ctx, "Successfully authenticated and logged in!");
  send_status_to_ui(bar, 35.0f);

  return test_session;
}

bool validate_host(ssh_session session, AppContext *ctx) {
  enum ssh_known_hosts_e state = ssh_session_is_known_server(session);

  if (state == SSH_KNOWN_HOSTS_OK) {
    return true;
  }

  if (state == SSH_KNOWN_HOSTS_UNKNOWN || state == SSH_KNOWN_HOSTS_NOT_FOUND) {
    ssh_key srv_pubkey = NULL;
    unsigned char *hash = NULL;
    size_t hlen = 0;

    if (ssh_get_server_publickey(session, &srv_pubkey) < 0) {
      safe_thread_log(ctx, "Error getting server public key");
      return false;
    }

    if (ssh_get_publickey_hash(srv_pubkey, SSH_PUBLICKEY_HASH_SHA256, &hash,
                               &hlen) < 0) {
      safe_thread_log(ctx, "Error getting SHA256 hash");
      ssh_key_free(srv_pubkey);
      return false;
    }

    char *hexa_string =
        ssh_get_fingerprint_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
    ssh_clean_pubkey_hash(&hash);
    ssh_key_free(srv_pubkey);

    if (hexa_string == NULL) {
      safe_thread_log(ctx, "Error converting hash to fingerprint");
      return false;
    }

    int confirm = showAlertOnNewHost(hexa_string);
    ssh_string_free_char(hexa_string);

    if (confirm != 1) {
      safe_thread_log(ctx, "Connection aborted by user (Host untrusted).");
      return false;
    }

    if (ssh_session_update_known_hosts(session) < 0) {
      safe_thread_log(ctx, std::string("Error writing to known hosts: ") +
                               strerror(errno));
    } else {
      safe_thread_log(ctx,
                      "Host successfully added to known_hosts configuration.");
    }
    return true;
  }

  if (state == SSH_KNOWN_HOSTS_CHANGED) {
    safe_thread_log(ctx, "WARNING: the server's host key has CHANGED since it "
                         "was last saved. This could be a man-in-the-middle "
                         "attack. Connection aborted.");
  } else if (state == SSH_KNOWN_HOSTS_OTHER) {
    safe_thread_log(ctx, "WARNING: the server presented a different type of "
                         "host key than the one on record. Connection aborted.");
  } else {
    safe_thread_log(ctx, "Host verification error (state " +
                             std::to_string((int)state) +
                             "). Connection aborted.");
  }
  return false;
}

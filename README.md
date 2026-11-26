[README.md](https://github.com/user-attachments/files/35438815/README.md)
# Synd

Synd is a small Windows desktop app for sending and retrieving files over SFTP. It has a simple graphical interface with your computer's files on one side and the server's files on the other, and an optional **Cloudflare Tunnel** connection mode for servers that sit behind Cloudflare Access.


## Features

- Two file browsers, **Local Files** and **Remote Files**, so you select files instead of typing paths
- Upload and download with the **Upload** and **Download** buttons
- Two connection types: **Generic** (direct SSH/SFTP) and **Cloudflare Tunnel**
- Host key verification against your `known_hosts` file, with a fingerprint prompt for new servers and a warning if a saved key has changed
- Overwrite confirmation before an existing file is replaced
- Uploads are written to a temporary `.part` file and renamed only when the transfer completes
- Progress bar and a log pane showing what the app is doing
- Windows installer with a Start menu entry, optional desktop shortcut, and uninstaller

## Install

1. Go to the [Releases page](https://github.com/CL0C4/synd/releases) and download the latest `Synd-Setup-x.y.exe`.
2. Run it. Windows SmartScreen may show an "unknown publisher" warning because the installer is not code-signed. Click **More info**, then **Run anyway**.
3. Follow the installer. It installs for the current user and does not need administrator rights.
4. Start **Synd** from the Start menu.

Requires 64-bit Windows 10 or 11. Everything the app needs, including `cloudflared`, is bundled with the installer. Installing a newer version over an older one upgrades it in place.

## Using Synd

1. Enter the server address (for example `user@example.com`) and your password.
2. Choose the connection type:
   - **Generic** connects directly to the server.
   - **Cloudflare Tunnel** starts `cloudflared` in the background and connects through it. The hostname must be set up for Cloudflare Access TCP tunnelling.
3. Click **Connect**. If the server is new to you, Synd shows its SHA256 fingerprint. Check it against the one you expect before you accept it.
4. Browse the two panes. A single click selects an item. Double-click a folder to open it, or double-click `..` to go up. The local pane starts at your Desktop.
5. To **upload**, select a file in **Local Files** and a destination folder in **Remote Files**, then click **Upload**.
6. To **download**, select a file in **Remote Files** and a destination folder in **Local Files**, then click **Download**.
7. Watch the progress bar and the log at the bottom of the window. If a file already exists at the destination, Synd asks before overwriting it.

The connection stays open until you close the app.

## Security notes

- Synd verifies the server's host key against your `known_hosts` file. A new server asks for your confirmation, and a changed key stops the connection.
- Your password is used for the login and is not saved. The password field is cleared after you click Connect.
- Login is by password only. Key-based authentication is not supported yet.
- In Cloudflare Tunnel mode, Synd runs `cloudflared` on `127.0.0.1:2222`, so that port must be free.

## Troubleshooting

- **"Windows protected your PC" when installing:** the installer is unsigned. Choose **More info**, then **Run anyway**.
- **Tunnel mode fails to connect:** make sure nothing else is using port 2222 (a leftover `cloudflared.exe` in Task Manager is the usual cause), and check that the hostname is protected by Cloudflare Access.
- **The log says the host key has changed:** the server's key is different from the one saved in your `known_hosts` file. Don't continue unless you know why it changed.
- **The app shows an old icon after an upgrade:** Windows caches icons. Uninstall, delete the old shortcuts, and install again.

## Building from source

Synd is built with `g++` in the [MSYS2](https://www.msys2.org/) **UCRT64** shell. There is no CMake project.

### Prerequisites

1. Install MSYS2, open the **MSYS2 UCRT64** shell, and update it with `pacman -Syu` (reopen the shell if it asks you to close it).
2. Install the toolchain and libraries:

   ```bash
   pacman -S git make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-binutils \
             mingw-w64-ucrt-x86_64-fltk mingw-w64-ucrt-x86_64-libssh
   ```

3. Install [cloudflared](https://developers.cloudflare.com/cloudflare-one/connections/connect-networks/downloads/) so that `C:\Program Files (x86)\cloudflared\cloudflared.exe` exists. `package.sh` copies it into the package. If yours is elsewhere, edit the `cp` line in `package.sh`.
4. To build the installer, install [Inno Setup](https://jrsoftware.org/isinfo.php).

### Build

```bash
git clone https://github.com/CL0C4/synd.git
cd synd
./package.sh
```

`package.sh` compiles the app, builds the icon resource from `synd.rc` and `synd.ico`, and creates a `dist/` folder containing `synd.exe`, the DLLs it needs, `cloudflared.exe`, and the `frontend/icons` folder. The contents of `dist/` run on any 64-bit Windows machine without MSYS2.

If the script fails with `$'\r': command not found`, git converted its line endings. Fix it with `sed -i 's/\r$//' package.sh` and run it again. If you get "permission denied", run `bash package.sh`.

### Build the installer

1. Open `synd.iss` in Inno Setup.
2. Check `AppVersion` and `OutputBaseFilename`, and leave `AppId` unchanged so new versions upgrade old installs.
3. Press **Ctrl+F9**. The installer is written to `Output\`.

### Project layout

| Path | Contents |
| --- | --- |
| `main.cpp` | Program entry point |
| `auth/` | Connection setup, host key check, password login, Cloudflare tunnel handling |
| `upload/` | Upload logic |
| `retrieve/` | Download logic |
| `parser/` | Address parsing |
| `frontend/` | FLTK interface (`gui.cpp`, `gui.h`) and icons |
| `package.sh` | Builds the app and assembles `dist/` |
| `synd.iss` | Inno Setup installer script |
| `synd.ico`, `synd.rc` | Application icon and its resource file |

## Known limitations

- Password login only
- Windows only
- The installer is not code-signed, so Windows and some antivirus tools may warn about it
- Cloudflare Tunnel mode requires a hostname configured for Cloudflare Access

## Third-party components

Synd uses [libssh](https://www.libssh.org/), [FLTK](https://www.fltk.org/), [OpenSSL](https://www.openssl.org/), and [cloudflared](https://github.com/cloudflare/cloudflared). Each has its own license. Check their terms before redistributing the installer.

## License

[Add your license here, for example MIT, and include a `LICENSE` file in the repository.]

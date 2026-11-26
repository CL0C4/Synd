#include "gui.h"
#include "../auth/auth.h"
#include "../retrieve/retrieve.h"
// #include "../upload/upload.h"
#include <FL/Fl_Choice.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Radio_Button.H>
#include <chrono>
#include <FL/Fl_Box.H>

Fl_Color dark_bg = fl_rgb_color(30, 30, 36);       // Dark window canvas
Fl_Color surface_bg = fl_rgb_color(43, 43, 51);    // Input field background
Fl_Color text_light = fl_rgb_color(240, 240, 245); // Crisp white typography
Fl_Color text_muted = fl_rgb_color(160, 160, 170); // Secondary gray text
Fl_Color btn_color = fl_rgb_color(28, 97, 74);     // green

Fl_Button *create_modern_button(int x, int y, int w, int h,
                                const char *label_text)
{
  Fl_Button *connect_btn =
      new Fl_Button(x, y, w, h, label_text); // x-pos, y-pos, w, h, text
  connect_btn->box(FL_FLAT_BOX);             // flat color
  connect_btn->color(fl_rgb_color(28, 97, 74));
  connect_btn->selection_color(fl_rgb_color(18, 59, 45)); // color on click
  connect_btn->labelcolor(fl_rgb_color(204, 204, 209));
  connect_btn->labelfont(FL_HELVETICA);
  connect_btn->labelsize(12);
  connect_btn->clear_visible_focus();

  return connect_btn;
}

Fl_Input *create_modern_input(int x, int y, int w, int h,
                              const char *label_text)
{
  int label_height = 14;
  int vertical_gap = 8; // <-- spacing between label and box

  // label
  Fl_Box *custom_label =
      new Fl_Box(x, y - label_height - vertical_gap, w, label_height);
  custom_label->copy_label(label_text);
  custom_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  custom_label->labelcolor(fl_rgb_color(160, 160, 170)); // text_muted
  custom_label->labelfont(FL_HELVETICA);
  custom_label->labelsize(12);

  // input field -> need empty label
  Fl_Input *field = new Fl_Input(x, y, w, h, "");
  field->box(FL_FLAT_BOX);
  field->color(fl_rgb_color(43, 43, 51));        // surface_bg
  field->textcolor(fl_rgb_color(240, 240, 245)); // text_light
  field->textfont(FL_HELVETICA);
  field->textsize(14);
  field->cursor_color(fl_rgb_color(240, 240, 245));

  return field;
}

Fl_Secret_Input *create_modern_secret_input(int x, int y, int w, int h,
                                            const char *label_text)
{
  int label_height = 14;
  int vertical_gap = 8; // <-- spacing between label and box

  // label
  Fl_Box *custom_label =
      new Fl_Box(x, y - label_height - vertical_gap, w, label_height);
  custom_label->copy_label(label_text);
  custom_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  custom_label->labelcolor(fl_rgb_color(160, 160, 170)); // text_muted
  custom_label->labelfont(FL_HELVETICA);
  custom_label->labelsize(12);

  // password fields
  Fl_Secret_Input *pass = new Fl_Secret_Input(x, y, w, h, "");
  pass->box(FL_FLAT_BOX);
  pass->color(fl_rgb_color(43, 43, 51));        // surface_bg
  pass->textcolor(fl_rgb_color(240, 240, 245)); // text_light
  pass->textfont(FL_HELVETICA);
  pass->textsize(14);
  pass->cursor_color(fl_rgb_color(240, 240, 245));

  return pass;
}

Fl_Text_Display *create_modern_text_log(int x, int y, int w, int h,
                                        const char *label_text,
                                        Fl_Text_Buffer *buffer)
{
  int label_height = 14;
  int vertical_gap = 8; // Spacing between label and box

  Fl_Box *custom_label =
      new Fl_Box(x, y - label_height - vertical_gap, w, label_height);
  custom_label->copy_label(label_text);
  custom_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  custom_label->labelcolor(fl_rgb_color(160, 160, 170)); // text_muted
  custom_label->labelfont(FL_HELVETICA);
  custom_label->labelsize(12);

  Fl_Text_Display *field = new Fl_Text_Display(x, y, w, h, "");

  // Attach the buffer engine so it can accept text changes
  field->buffer(buffer);

  // Style properties matching your flat design theme colors
  field->box(FL_FLAT_BOX);
  field->color(fl_rgb_color(43, 43, 51));        // surface_bg
  field->textcolor(fl_rgb_color(240, 240, 245)); // text_light
  field->textfont(FL_HELVETICA);
  field->textsize(11);

  // Remove the active text selection highlight color blocks
  field->selection_color(field->color());

  // scrollbar
  field->scrollbar_width(10);

  return field;
}

Fl_Progress *create_modern_progress_bar(int x, int y, int w, int h)
{
  // init
  Fl_Progress *bar = new Fl_Progress(x, y, w, h);

  // data limits
  bar->minimum(0.0f);   //
  bar->maximum(100.0f); //
  bar->value(0.0f);     // Set initial starting state

  // modern design
  bar->box(FL_FLAT_BOX);                // Removes legacy 3D bevel tracks
  bar->color(fl_rgb_color(43, 43, 51)); // Track background (Surface BG)
  bar->selection_color(
      fl_rgb_color(52, 131, 250)); // Loading bar fill (Accent Blue)

  // Update the inner percentage text typography
  bar->labelcolor(fl_rgb_color(240, 240, 245)); // White percentage text
  bar->labelfont(FL_HELVETICA_BOLD);
  bar->labelsize(12);

  return bar;
}

Fl_Radio_Button *create_modern_radio_button(int x, int y, int w, int h,
                                            const char *label_text)
{
  Fl_Radio_Button *radio_button =
      new Fl_Radio_Button(x, y, w, h, label_text); // x-pos, y-pos, w, h, text
  radio_button->box(FL_FLAT_BOX);                  // flat color
  radio_button->color(fl_rgb_color(28, 97, 74));
  radio_button->selection_color(fl_rgb_color(18, 59, 45)); // color on click
  radio_button->labelcolor(fl_rgb_color(204, 204, 209));
  radio_button->labelfont(FL_HELVETICA);
  radio_button->labelsize(12);
  radio_button->clear_visible_focus();

  return radio_button;
}

Fl_Choice *create_modern_dropdown(int x, int y, int w, int h,
                                  const char *label_text)
{
  int label_height = 14;
  int vertical_gap = 8; // <-- spacing between label and box
  // label
  Fl_Box *custom_label =
      new Fl_Box(x, y - label_height - vertical_gap, w, label_height);
  custom_label->copy_label(label_text);
  custom_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  custom_label->labelcolor(fl_rgb_color(160, 160, 170)); // text_muted
  custom_label->labelfont(FL_HELVETICA);
  custom_label->labelsize(12);

  // input field -> need empty label
  Fl_Choice *field = new Fl_Choice(x, y, w, h, "");
  field->box(FL_FLAT_BOX);
  field->color(fl_rgb_color(43, 43, 51));        // surface_bg
  field->textcolor(fl_rgb_color(240, 240, 245)); // text_light
  field->textfont(FL_HELVETICA);
  field->textsize(14);

  field->add("Generic");
  field->add("Cloudflare Tunnel");

  field->value(0); // set value to first selection

  return field;
}

// local and remote file boxes
Fl_Box *create_modern_box(int x, int y, int w, int h,
                          const char *label_text)
{
  int label_height = 14;
  int vertical_gap = 8; // <-- spacing between label and box

  Fl_Box *custom_label =
      new Fl_Box(x, y - label_height - vertical_gap, w, label_height);
  custom_label->copy_label(label_text);
  custom_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  custom_label->labelcolor(fl_rgb_color(160, 160, 170)); // text_muted
  custom_label->labelfont(FL_HELVETICA);
  custom_label->labelsize(12);
  Fl_Box *new_box =
      new Fl_Box(x, y, w, h); // x-pos, y-pos, w, h, text
  new_box->box(FL_FLAT_BOX);  // flat color
  new_box->color(fl_rgb_color(43, 43, 51));
  new_box->labelcolor(fl_rgb_color(204, 204, 209));
  new_box->labelfont(FL_HELVETICA);
  new_box->labelsize(12);
  new_box->clear_visible_focus();

  return new_box;
}

void append_to_log(AppContext *ctx, const std::string &message)
{
  if (!ctx || !ctx->log_buffer || !ctx->log)
    return;

  // Add the log line string with a trailing newline formatting byte
  ctx->log_buffer->append((message + "\n").c_str());

  // Move layout window context focusing tracking to the absolute base row
  ctx->log->insert_position(ctx->log_buffer->length());
  ctx->log->show_insert_position();
}

struct ProgressUpdate
{
  Fl_Progress *bar;
  float value;
};

void update_ui_safely(void *data)
{
  ProgressUpdate *update = (ProgressUpdate *)data;

  // update
  update->bar->value(update->value);
  update->bar->redraw();

  // delete memory
  delete update;
}

void send_status_to_ui(Fl_Progress *bar, float val)
{
  ProgressUpdate *update = new ProgressUpdate{bar, val};

  // tells tltk to run on main thread
  Fl::awake(update_ui_safely, update);
}

void toggle_send_retrieve_callback(Fl_Widget *w, void *data)
{
  Fl_Radio_Button *btn = (Fl_Radio_Button *)w;

  if (!data)
    return;

  AppContext *ctx = (AppContext *)data;

  if (!ctx->log)
    return; // check that log widget is allocated

  // value() returns 1 if this specific button was just turned on
  if (btn->value() == 1)
  {
    std::string msg = "Set to " + std::string(btn->label()) + " file/directory";
    append_to_log(ctx, msg);
  }
}

// thread
struct ThreadMessage
{
  AppContext *ctx;
  std::string text;
};

void ui_log_callback(void *data)
{
  ThreadMessage *msg = (ThreadMessage *)data;
  if (msg && msg->ctx && msg->ctx->log_buffer && msg->ctx->log)
  {
    // Safe to modify widgets here!
    msg->ctx->log_buffer->append((msg->text + "\n").c_str());
    msg->ctx->log->insert_position(msg->ctx->log_buffer->length());
    msg->ctx->log->show_insert_position();
  }
  delete msg; // Clean up the message memory
}

void safe_thread_log(AppContext *ctx, const std::string &text)
{
  ThreadMessage *msg = new ThreadMessage{ctx, text};
  // Fl::awake queues the ui_log_callback to be executed by the main UI thread
  // safely
  Fl::awake(ui_log_callback, msg);
}

void worker_done_callback(void *data)
{
  ((Fl_Widget *)data)->activate();
}

void finish_worker_ui(Fl_Widget *btn)
{
  Fl::awake(worker_done_callback, btn);
}

void ssh_worker_thread(AppContext *ctx, std::string addr, std::string local,
                       std::string remote, std::string password,
                       Fl_Progress *bar, int action, int conn_type, Fl_Widget *btn)
{
  safe_thread_log(ctx, "Starting authentication process background worker...");
  send_status_to_ui(bar, 5.0f);

  ssh_session sess = authenticate_connection(addr, conn_type, password, ctx);
  ctx->sess = sess;

  std::fill(password.begin(), password.end(), '\0');
  password.clear();

  if (!sess)
  {
    safe_thread_log(ctx, "Connection failed; transfer cancelled");
    stop_tunnel();
    send_status_to_ui(bar, 0.0f);
    finish_worker_ui(btn);
    return;
  }

  safe_thread_log(ctx, "Authentication complete! Connection verified.");
  safe_thread_log(ctx, "Populating Trees");

  // In ssh_worker_thread (gui.cpp):
  sftp_session serv = open_sftp_session(sess, ctx);
  if (!serv)
  {
    finish_worker_ui(btn);
    return;
  }

  // Resolve remote "./" safely
  char *real_remote = sftp_canonicalize_path(serv, remote.c_str());
  if (real_remote)
  {
    ctx->remote = real_remote;
    ssh_string_free_char(real_remote);
  }

  // Lock FLTK before modifying Fl_Tree widgets from a worker thread!
  Fl::lock();
  populate_local_fl_tree(ctx->local, "Local", ctx->local_tree, ctx);
  populate_remote_fl_tree(sess, serv, ctx->remote, "Remote", ctx->remote_tree, ctx);
  Fl::unlock();

  // Redraw safely on the main UI thread
  Fl::awake([](void *data)
            {
    AppContext *c = static_cast<AppContext *>(data);
    if (c->local_tree) c->local_tree->redraw();
    if (c->remote_tree) c->remote_tree->redraw(); }, ctx);

  sftp_free(serv);
  finish_worker_ui(btn);
}

void connect_btn_callback(Fl_Widget *w, void *data)
{
  AppContext *ctx = (AppContext *)data;
  Fl_Progress *bar = ctx->prog_bar;
  Fl_Button *btn = ctx->send;
  bool has_error = false;

  // check if there are empty fields
  if (ctx->addr->value()[0] == '\0')
  {
    append_to_log(ctx, "Error: Empty address");
    has_error = true;
  }
  if (ctx->pass->value()[0] == '\0')
  {
    append_to_log(ctx, "Error: Empty password");
    has_error = true;
  }

  // all fields contain something
  if (!has_error)
  {
    append_to_log(ctx, "All fields filled, proceeding.");
  }
  if (has_error)
  {
    return;
  }

  w->deactivate();
  ctx->prog_bar->value(0.0f);
  ctx->prog_bar->redraw();

  // grab addr, local, remote, pass
  std::string address = ctx->addr->value();
  std::string local = ctx->local;
  std::string remote = ctx->remote;
  // password
  const char *raw_pass = ctx->pass->value();
  // upload/download
  int action = btn->value();
  std::string password(raw_pass);
  ctx->pass->value(""); // clear the old password for safety
  std::thread worker(ssh_worker_thread, ctx, address, local, remote, password,
                     bar, action, ctx->conn_type->value(), w);
  worker.detach(); // allows it to run independently of the main GUI loop
}

void OverwriteCallback(void *data)
{
  NewHostAlertContext *ctx = static_cast<NewHostAlertContext *>(data);
  fl_message_title("File Conflict");
  ctx->result = fl_ask("File already exists. Overwrite?");
  ctx->done = true; // Signal the background thread that the user answered
}

void NewHostCallback(void *data)
{
  NewHostAlertContext *ctx = static_cast<NewHostAlertContext *>(data);
  fl_message_title("Unknown Host");
  ctx->result =
      fl_ask("Unknown Host! SHA256 fingerprint: %s", ctx->hexa_string);
  ctx->done = true; // Signal the background thread that the user answered
}

int showAlertOnNewHost(char *hexa_string)
{
  NewHostAlertContext ctx;
  ctx.hexa_string = hexa_string;
  Fl::awake(NewHostCallback, &ctx);
  while (!ctx.done)
  {
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }
  return ctx.result;
}

int showOverwriteAlert()
{
  OverwriteAlertContext ctx;
  Fl::awake(OverwriteCallback, &ctx);
  while (!ctx.done)
  {
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }
  return ctx.result;
}

// upload/download callbacks

#include <chrono>

// Helper: returns true if the same Fl_Tree_Item was clicked twice within 400ms
bool is_tree_double_click(Fl_Tree_Item *item, bool is_local)
{
  using clock = std::chrono::steady_clock;
  static Fl_Tree_Item *last_local_item = nullptr;
  static clock::time_point last_local_time;
  static Fl_Tree_Item *last_remote_item = nullptr;
  static clock::time_point last_remote_time;

  Fl_Tree_Item *&last_item = is_local ? last_local_item : last_remote_item;
  clock::time_point &last_time = is_local ? last_local_time : last_remote_time;

  auto now = clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count();

  bool double_clicked = (item == last_item && elapsed > 0 && elapsed < 400);

  last_item = item;
  last_time = now;

  // Reset after a successful double-click so a 3rd click doesn't trigger again
  if (double_clicked)
  {
    last_item = nullptr;
  }

  return double_clicked;
}

fs::path get_fs_path_from_item(Fl_Tree *tree, Fl_Tree_Item *item, const fs::path &base_dir, const std::string &tree_prefix)
{
  if (!tree || !item)
    return {};

  char pathname[1024];
  if (tree->item_pathname(pathname, sizeof(pathname), item) != 0)
  {
    return {};
  }

  std::string rel_path(pathname);

  // 1. Strip FLTK's default "ROOT/" prefix if present
  if (rel_path.rfind("ROOT/", 0) == 0)
  {
    rel_path.erase(0, 5);
  }
  else if (rel_path == "ROOT")
  {
    return base_dir;
  }

  // 2. Strip your custom "Local/" or "Remote/" prefix
  std::string prefix_with_slash = tree_prefix + "/";
  if (rel_path.rfind(prefix_with_slash, 0) == 0)
  {
    rel_path.erase(0, prefix_with_slash.length());
  }
  else if (rel_path == tree_prefix)
  {
    return base_dir;
  }

  return base_dir / fs::path(rel_path);
}

void upload(Fl_Widget *w, void *data)
{
  AppContext *ctx = (AppContext *)data;
  if (!ctx || !ctx->sess)
  {
    safe_thread_log(ctx, "Not connected to SSH session.");
    return;
  }

  Fl_Tree_Item *local = ctx->local_tree->first_selected_item();
  Fl_Tree_Item *remote = ctx->remote_tree->first_selected_item();
  if (!local)
  {
    safe_thread_log(ctx, "No local file selected.");
    return;
  }
  if (!remote)
  {
    safe_thread_log(ctx, "No remote destination selected.");
    return;
  }

  fs::path local_target = get_fs_path_from_item(ctx->local_tree, local, ctx->local, "Local");
  fs::path remote_target = get_fs_path_from_item(ctx->remote_tree, remote, ctx->remote, "Remote");

  // Run upload_file in a worker thread so showOverwriteAlert() and progress bar updates don't block/deadlock the UI
  std::thread([ctx, local_target, remote_target]()
              { upload_file(ctx->sess, local_target, remote_target, ctx); })
      .detach();
  ctx->send->value(0);
}

void retrieve(Fl_Widget *w, void *data)
{
  AppContext *ctx = (AppContext *)data;
  if (!ctx || !ctx->sess)
  {
    safe_thread_log(ctx, "Not connected to SSH session.");
    return;
  }

  Fl_Tree_Item *local = ctx->local_tree->first_selected_item();
  Fl_Tree_Item *remote = ctx->remote_tree->first_selected_item();
  if (!local)
  {
    safe_thread_log(ctx, "No local file selected.");
    ctx->send->value(0);
    ctx->retrieve->value(0);
    return;
  }
  if (!remote)
  {
    safe_thread_log(ctx, "No remote destination selected.");
    ctx->send->value(0);
    ctx->retrieve->value(0);
    return;
  }

  fs::path local_target = get_fs_path_from_item(ctx->local_tree, local, ctx->local, "Local");
  fs::path remote_target = get_fs_path_from_item(ctx->remote_tree, remote, ctx->remote, "Remote");

  // Run retrieve_file in a worker thread so showOverwriteAlert() and progress bar updates don't block/deadlock the UI
  std::thread([ctx, local_target, remote_target]()
              { retrieve_file(ctx->sess, remote_target, local_target, ctx); })
      .detach();
  ctx->retrieve->value(0);
}

void local_tree_callback(Fl_Widget *w, void *data)
{
  Fl_Tree *tree = (Fl_Tree *)w;
  AppContext *ctx = (AppContext *)data;
  Fl_Tree_Item *item = tree->callback_item();

  Fl_Tree_Reason reason = tree->callback_reason();
  if (!item || (reason != FL_TREE_REASON_SELECTED && reason != FL_TREE_REASON_RESELECTED))
    return;

  // Single click highlights item for Upload/Download; 2 clicks within 400ms navigates
  if (!is_tree_double_click(item, true))
    return;

  std::string label = item->label() ? item->label() : "";
  std::error_code ec;
  fs::path current(ctx->local);

  if (label == "..")
  {
    if (current.has_parent_path() && current.parent_path() != current)
    {
      ctx->local = current.parent_path().string();
      populate_local_fl_tree(ctx->local, "Local", ctx->local_tree, ctx);
      ctx->local_tree->redraw();
    }
    return;
  }

  if (!label.empty() && label.back() == '/')
  {
    std::string folder_name = label.substr(0, label.size() - 1);
    fs::path next_dir = current / folder_name;
    if (fs::is_directory(next_dir, ec))
    {
      ctx->local = next_dir.string();
      populate_local_fl_tree(ctx->local, "Local", ctx->local_tree, ctx);
      ctx->local_tree->redraw();
    }
  }
}

// Helper to safely redraw remote_tree from a background thread
void redraw_remote_tree_cb(void *data)
{
  Fl_Tree *tree = static_cast<Fl_Tree *>(data);
  if (tree)
    tree->redraw();
}

void remote_tree_callback(Fl_Widget *w, void *data)
{
  Fl_Tree *tree = (Fl_Tree *)w;
  AppContext *ctx = (AppContext *)data;
  Fl_Tree_Item *item = tree->callback_item();

  Fl_Tree_Reason reason = tree->callback_reason();
  if (!item || (reason != FL_TREE_REASON_SELECTED && reason != FL_TREE_REASON_RESELECTED))
    return;

  if (!is_tree_double_click(item, false))
    return;

  if (!ctx || !ctx->sess)
  {
    safe_thread_log(ctx, "Not connected to SSH session.");
    return;
  }

  std::string label = item->label() ? item->label() : "";
  std::string current_remote = ctx->remote;
  std::string next_remote;

  // 1. Move up to parent directory on ".."
  if (label == "..")
  {
    if (current_remote == "/" || current_remote.empty())
      return;

    // Strip trailing slash if present
    while (current_remote.size() > 1 && current_remote.back() == '/')
      current_remote.pop_back();

    size_t last_slash = current_remote.find_last_of('/');
    if (last_slash == std::string::npos || last_slash == 0)
      next_remote = "/";
    else
      next_remote = current_remote.substr(0, last_slash);
  }
  // 2. Enter subdirectory if item ends with '/'
  else if (!label.empty() && label.back() == '/')
  {
    std::string folder_name = label.substr(0, label.size() - 1);
    next_remote = (current_remote == "/") ? ("/" + folder_name) : (current_remote + "/" + folder_name);
  }
  else
  {
    // Regular file double-clicked — ignore navigation
    return;
  }

  // Fetch the new remote directory in a worker thread so UI stays responsive
  std::thread([ctx, next_remote]()
              {
        sftp_session serv = open_sftp_session(ctx->sess, ctx);
        if (!serv) return;

        // Verify the directory opens before updating ctx->remote
        sftp_dir test_dir = sftp_opendir(serv, next_remote.c_str());
        if (!test_dir)
        {
            safe_thread_log(ctx, "Cannot open remote folder: " + next_remote);
            sftp_free(serv);
            return;
        }
        sftp_closedir(test_dir);

        Fl::lock();
        ctx->remote = next_remote;
        populate_remote_fl_tree(ctx->sess, serv, ctx->remote, "Remote", ctx->remote_tree, ctx);
        Fl::unlock();

        Fl::awake(redraw_remote_tree_cb, ctx->remote_tree);
        sftp_free(serv); })
      .detach();
}

int run_frontend()
{
  // ----------- theme
  Fl::scheme("gtk+");
  // Set fallback window background color overrides globally
  Fl::background(30, 30, 36);
  // ----------------- theme end
  Fl_Window *win = new Fl_Window(720, 480, "Synd 1.0");
  char exe[MAX_PATH];
  GetModuleFileNameA(NULL, exe, MAX_PATH);
  std::string base = std::filesystem::path(exe).parent_path().string();
  std::string icon_path = base + "\\frontend\\icons\\file-line-icon.png";

  Fl_PNG_Image *icon = new Fl_PNG_Image(icon_path.c_str());
  Fl_RGB_Image *scaled_icon = nullptr;
  if (!icon->fail())
  {
    scaled_icon = (Fl_RGB_Image *)icon->copy(32, 32);
  }
  win->icon(scaled_icon);
  win->color(dark_bg);

  Fl_Box *header_bg = new Fl_Box(0, 0, 720, 60);
  header_bg->box(FL_FLAT_BOX);
  header_bg->color(fl_rgb_color(37, 37, 43));

  Fl_Box *accent_bar = new Fl_Box(20, 14, 4, 32);
  accent_bar->box(FL_FLAT_BOX);
  accent_bar->color(fl_rgb_color(28, 97, 74));

  Fl_Box *title = new Fl_Box(34, 10, 200, 28, "SYND");
  title->labelfont(FL_HELVETICA_BOLD);
  title->labelsize(24);
  title->labelcolor(fl_rgb_color(240, 242, 245));
  title->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  Fl_Box *subtitle = new Fl_Box(34, 36, 300, 16, "\"Secure\" file transfer over SFTP");
  subtitle->labelfont(FL_HELVETICA);
  subtitle->labelsize(11);
  subtitle->labelcolor(fl_rgb_color(140, 148, 160));
  subtitle->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  Fl_Box *header_line = new Fl_Box(0, 58, 720, 2);
  header_line->box(FL_FLAT_BOX);
  header_line->color(fl_rgb_color(28, 97, 74));

  // app context for log
  AppContext *ctx = new AppContext();

  const char *user_profile = std::getenv("USERPROFILE");
  if (user_profile)
  {
    fs::path desktop = fs::path(user_profile) / "Desktop";
    if (fs::exists(desktop))
    {
      ctx->local = desktop.string();
    }
    else
    {
      ctx->local = user_profile;
    }
  }
  else
  {
    ctx->local = "C:\\";
  }

  // main buttons/fields
  Fl_Button *connect_btn = create_modern_button(490, 100, 90, 30, "Connect");
  ctx->addr = create_modern_input(50, 100, 120, 30, "Address");
  ctx->pass = create_modern_secret_input(190, 100, 120, 30, "Password");
  ctx->prog_bar = create_modern_progress_bar(50, 340, 620, 15);
  // connection type
  ctx->conn_type = create_modern_dropdown(330, 100, 120, 30, "Connection Type");

  // log
  ctx->log_buffer = new Fl_Text_Buffer();
  ctx->log = create_modern_text_log(50, 405, 620, 40, "Log", ctx->log_buffer); // changed from 50 240 620 80

  // added 11/25/26
  // boxes for file explore
  ctx->local_files = create_modern_box(50, 160, 285, 150, "Local Files");
  ctx->local_tree = new Fl_Tree(55, 165, 250, 145);
  ctx->local_tree->color(fl_rgb_color(43, 43, 51));           // surface_bg
  ctx->local_tree->selection_color(fl_rgb_color(28, 97, 74)); // visible green highlight
  ctx->local_tree->item_labelfgcolor(fl_rgb_color(240, 240, 245));
  ctx->local_tree->connectorcolor(fl_rgb_color(204, 204, 209));
  ctx->local_tree->item_reselect_mode(FL_TREE_SELECTABLE_ALWAYS); // enables double-click callback
  ctx->local_tree->callback(local_tree_callback, ctx);
  ctx->local_tree->showroot(0);

  ctx->remote_files = create_modern_box(415, 160, 285, 150, "Remote Files");
  ctx->remote_tree = new Fl_Tree(420, 165, 250, 145);
  ctx->remote_tree->color(fl_rgb_color(43, 43, 51));
  ctx->remote_tree->selection_color(fl_rgb_color(28, 97, 74)); // visible green highlight
  ctx->remote_tree->item_labelfgcolor(fl_rgb_color(240, 240, 245));
  ctx->remote_tree->connectorcolor(fl_rgb_color(204, 204, 209));
  ctx->remote_tree->item_reselect_mode(FL_TREE_SELECTABLE_ALWAYS); // enables double-click callback
  ctx->remote_tree->callback(remote_tree_callback, ctx);
  ctx->remote_tree->showroot(0);

  // Upload/retrieve buttons
  ctx->send = create_modern_button(50, 375, 90, 30, "Upload");
  ctx->send->callback(upload, ctx);
  ctx->retrieve = create_modern_button(142, 375, 90, 30, "Download");
  ctx->retrieve->callback(retrieve, ctx);

  // connect button callback
  connect_btn->callback(connect_btn_callback, ctx);

  win->end();
  Fl::lock();
  populate_local_fl_tree(ctx->local, "Local", ctx->local_tree, ctx);
  win->show();
  int r = Fl::run();
  if (ctx->sess)
  {
    ssh_disconnect(ctx->sess);
    ssh_free(ctx->sess);
    ctx->sess = nullptr;
  }
  stop_tunnel();
  return r;
}

#pragma once

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Text_Display.H>
#include <atomic>



#include <thread>
#include <iostream>

struct AppContext {
  Fl_Input *addr;
  Fl_Input *local;
  Fl_Input *remote;
  Fl_Secret_Input *pass;
  Fl_Text_Display *log;
  Fl_Text_Buffer *log_buffer;
  Fl_Progress *prog_bar;
  Fl_Choice *conn_type;
  Fl_Radio_Button* send;
  Fl_Radio_Button* retrieve;
};

struct OverwriteAlertContext {
    std::atomic<bool> done = false;
    int result = 0;
};
struct NewHostAlertContext {
    std::atomic<bool> done = false;
    int result = 0;
    char* hexa_string;
};

Fl_Button* create_modern_button(int x, int y, int w, int h, const char* label_text);

Fl_Input* create_modern_input(int x, int y, int w, int h, const char* label_text);

Fl_Output* create_modern_output(int x, int y, int w, int h, const char* label_text);

Fl_Secret_Input* create_modern_secret_input(int x, int y, int w, int h, const char* label_text);

Fl_Progress* create_modern_progress_bar(int x, int y, int w, int h);

Fl_Radio_Button* create_modern_radio_button(int x, int y, int w, int h, const char* label_text);

Fl_Choice* create_modern_dropdown(int x, int y, int w, int h, const char* label_text);

void update_ui_safely(void* data);

void send_status_to_ui(Fl_Progress* bar, float val);

void toggle_send_retrieve_callback(Fl_Widget *w, void *data);

void connect_btn_callback(Fl_Widget *w, void* data);

void safe_thread_log(AppContext* ctx, const std::string& text);

void OverwriteCallback(void* data);
void NewHostCallback(void* data);
int showAlertOnNewHost(char *hexa_string);
int showOverwriteAlert();


int run_frontend();

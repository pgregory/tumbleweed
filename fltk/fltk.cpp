#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>

#if defined WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

EXPORT int Fl_wait(double time)
{
  return Fl::wait(0.0);
}


EXPORT Fl_Window* createWindow(int width, int height) {
  Fl_Window* window = new Fl_Window(width, height);

  return window;
}

EXPORT void Fl_widget_show(Fl_Widget* widget) {
  widget->show();
}

EXPORT void Fl_widget_hide(Fl_Widget* widget) {
  widget->hide();
}

EXPORT int Fl_widget_visible(Fl_Widget* widget) {
  return widget->visible();
}

EXPORT Fl_Button* createButton(int x, int y, int w, int h, const char* label)
{
  Fl_Button* button = new Fl_Button(x, y, w, h);
  button->copy_label(label);

  return button;
}

EXPORT Fl_Input* Fl_createInput(int x, int y, int w, int h, const char* label)
{
  Fl_Input* input = new Fl_Input(x, y, w, h);
  input->copy_label(label);

  return input;
}


EXPORT void Fl_setCallback(Fl_Widget* widget, Fl_Callback0* cb)
{
  widget->callback(cb);
}


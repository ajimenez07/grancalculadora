// SPDX-FileCopyrightText: 2026 Álex Jiménez
//
// SPDX-License-Identifier: MIT

#include "window.h"
#include <gtkmm/application.h>

int
main (int argc, char *argv[])
{
  auto app = Gtk::Application::create ("es.alexjimenez.grancalculadora");

  // Shows the window and returns when it is closed.
  return app->make_window_and_run<CalcWindow> (argc, argv);
}

// SPDX-FileCopyrightText: 2026 Álex Jiménez
//
// SPDX-License-Identifier: MIT

#include "window.h"
#include "colors.h"

#include <iostream>


CalcWindow::CalcWindow ()
{
  set_title ("Gran Calculadora");

  // set minimum size
  set_size_request (250, 500);

  // configure buttons grid
  m_grid.set_row_spacing (10);
  m_grid.set_column_spacing (10);

  grid_add_button ("C", 0, 0, LIGHT_BLUE);
  grid_add_button ("(", 1, 0, LIGHT_BLUE);
  grid_add_button (")", 2, 0, LIGHT_BLUE);
  grid_add_button ("π", 3, 0, LIGHT_BLUE);
  grid_add_button ("√", 4, 0, LIGHT_BLUE);

  grid_add_button ("7", 0, 1, NAVY_BLUE);
  grid_add_button ("8", 1, 1, NAVY_BLUE);
  grid_add_button ("9", 2, 1, NAVY_BLUE);
  grid_add_button ("÷", 3, 1, LIGHT_BLUE);
  grid_add_button ("×", 4, 1, LIGHT_BLUE);

  grid_add_button ("4", 0, 2, NAVY_BLUE);
  grid_add_button ("5", 1, 2, NAVY_BLUE);
  grid_add_button ("6", 2, 2, NAVY_BLUE);
  grid_add_button ("+", 3, 2, LIGHT_BLUE);
  grid_add_button ("-", 4, 2, LIGHT_BLUE);

  grid_add_button ("1", 0, 3, NAVY_BLUE);
  grid_add_button ("2", 1, 3, NAVY_BLUE);
  grid_add_button ("3", 2, 3, NAVY_BLUE);
  grid_add_button ("x²", 3, 3, LIGHT_BLUE);
  grid_add_button ("%", 4, 3, LIGHT_BLUE);

  grid_add_button ("0", 0, 4, NAVY_BLUE);
  grid_add_button (",", 1, 4, LIGHT_BLUE);
  grid_add_button ("e", 2, 4, LIGHT_BLUE);
  grid_add_button ("=", 3, 4, 2, 1, GREEN);

  Gtk::ScrolledWindow scrolledw;
  /*
  m_entry = Gtk::make_managed<Gtk::Entry> ();
  m_entry->set_hexpand (true);
  m_entry->set_vexpand (true);
  */

  m_display = Gtk::make_managed<Display> ();
  m_display->set_hexpand (true);
  m_display->set_vexpand (true);

  m_display->insert_fraction();
  m_display->insert_symbol('2');
  m_display->insert_symbol('+');
  m_display->insert_symbol('2');
  m_display->wrap_in_fraction_numerator ();
  m_display->insert_symbol('3');

  scrolledw.set_child (*m_display);
  scrolledw.set_policy (Gtk::PolicyType::AUTOMATIC,
                        Gtk::PolicyType::AUTOMATIC);

  scrolledw.set_hexpand (true);
  scrolledw.set_vexpand (true);

  m_main_grid.attach (scrolledw, 0, 1, 1, 1);
  m_main_grid.attach (m_grid, 0, 2, 1, 4);

  set_child (m_main_grid);

  // apply css when ready
  signal_show ().connect ([this] () {
    Glib::signal_idle ().connect_once ([this] () {
      apply_css_to_window ();

      m_display->draw();
    });
  });
}

void
CalcWindow::apply_css_to_window ()
{
  // global css (window background + textview css
  auto css_data = Glib::ustring::compose ("window { background: %1; }"
                                          ".display-wrapper {"
                                          "    background-color: white;"
                                          "}"
                                          "scrolledwindow {"
                                          "    background-color: #0F2B0F;"
                                          "    border-radius: 10px;"
                                          "}",
                                          WHITE);

  // buttons css
  for (const auto &css : buttons_css)
    css_data += css;

  // apply the css
  auto css_provider = Gtk::CssProvider::create ();
  css_provider->load_from_data (css_data);
  Gtk::StyleContext::add_provider_for_display (
      get_display (), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

CalcWindow::~CalcWindow () {}

/*
 * SPDX-FileCopyrightText: 2026 Álex Jiménez
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef GC_WINDOW_H
#define GC_WINDOW_H

#include "colors.h"
#include "display.h"

#include <gtkmm.h>

/* sometimes is necessary to print things,
 but not in release*/
#ifdef DEBUG
#include <iostream>
#endif

/* avoid verbosity */
using namespace GC;

class CalcWindow : public Gtk::Window
{

public:
  CalcWindow ();
  ~CalcWindow () override;

private:
  /* main grid (buttons + textpane) */
  Gtk::Grid m_main_grid;

  /* buttons grid */
  Gtk::Grid m_grid;

  Gtk::Entry *m_entry;

  Display *m_display;
  
  /* vector of CSS strings with the purpose
     of applying the buttons CSS after the windows is mapped */
  std::vector<Glib::ustring> buttons_css;

  /*
    Add a button specifying the width and the height (relative to the cells of
    the grid, not px). It is necessary to specify the color as well.
   */
  void
  grid_add_button (const Glib::ustring text, int col, int row, int width,
                   int height, const GC::Color color)
  {
    Gtk::Button btn (text);

    /* set a unique id to the button */
    auto id = Glib::ustring::compose ("btn_%1_%2", col, row);
    btn.set_name (id);

    /* color is only an alias for 'Glib::ustring' and is a possible value of
     * background*/
    auto css_data = Glib::ustring::compose (
        "#%1 {"
        "background: %2;"
        "font-size: 1.5em;"
        "color: white;"
        "font-weight: bold;"
        "filter: brightness(125%%);" // add some brightness
        "}",
        id, color);
    add_button_css (css_data);

    /* adapt to the windows size */
    btn.set_hexpand (true);
    btn.set_vexpand (true);

    /* add to the grid */
    m_grid.attach (btn, col, row, width, height);
  }

  /*
    Add a button but with the size of one cell of the grid as default.
   */
  void
  grid_add_button (const Glib::ustring text, int col, int row,
                   const GC::Color color)
  {
    grid_add_button (text, col, row, 1, 1, color);
  }

  void
  add_button_css (const Glib::ustring css)
  {
    buttons_css.push_back (css);
  }

  void apply_css_to_window ();
};

#endif // GC_WINDOW_H

/*
 * SPDX-FileCopyrightText: 2026 Álex Jiménez
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef GC_DISPLAY_H
#define GC_DISPLAY_H

#include <memory>
#include <string>
#include <vector>

#include <gtkmm.h>

namespace GC
{
namespace DisplayAst
{

/* Class that encapsulates the attributes needed
   to draw an element. Element ('struct Element') can be
   a fraction, a square root or a symbol (number or operator).

   As well, an expression ('struct Expr')
   has its own geometry. For example, the height would be
   the height of the tallest element. And the width would be the sum of
   the width and the margin of all the elements belonging to the expression.

By default, all attributes (including width and height) are set to 0. */

struct DrawGeometry
{
  double x = 0, y = 0;
  double width = 0, height = 0;
  double margin = 0;

  void
  move_x (double dx)
  {
    x += dx;
  }

  void
  move_y (double dy)
  {
    y += dy;
  }

  DrawGeometry (double x, double y, double width, double height, double margin)
      : x (x), y (y), width (width), height (height), margin (margin) {};

  DrawGeometry () {};
};

struct Expr;

enum class ElementType
{
  FRACTION,
  ROOT,
  SYMBOL
};

struct Element
{
  virtual ~Element () = default;
  ElementType type;
  DrawGeometry geometry;

  /* call this method to update the geometry
     attributes of the element based on the new
     changes */
  virtual void update_geometry () {};
};

struct FractionElement : Element
{
  std::unique_ptr<Expr> numerator;
  std::unique_ptr<Expr> denominator;

  FractionElement ()
  {
    type = ElementType::FRACTION;
    numerator = std::make_unique<Expr> ();
    denominator = std::make_unique<Expr> ();

    update_geometry ();
  }

  void update_geometry () override;
};

struct RootElement : Element
{
  std::unique_ptr<Expr> index;
  std::unique_ptr<Expr> radicand;

  RootElement ()
  {
    type = ElementType::ROOT;
    index = std::make_unique<Expr> ();
    radicand = std::make_unique<Expr> ();

    update_geometry ();
  }

  void update_geometry () override;
};

struct SymbolElement : Element
{
  char symbol;

  SymbolElement (char c) : symbol (c)
  {
    type = ElementType::SYMBOL;
    geometry.width = 24;
    geometry.height = 24;
  }
};

struct Expr
{
  std::vector<std::unique_ptr<Element> > elements;

  DrawGeometry geometry;

  /* with these four methods, the code is less verbose */
  double
  get_width () { return geometry.width; }

  double
  get_height () { return geometry.height; }

  double
  get_x () { return geometry.x; }

  double
  get_y () { return geometry.y; }

  double
  get_margin () { return geometry.margin; }
  
  /* updates the total width of the expression. That is the sum
     of the width and margin of all subelements */
  void
  update_width ()
  {
    double width = 0;
    for (size_t i = 0; i < elements.size (); i++)
      {
        Element *el = elements[i].get ();
        width += el->geometry.width + el->geometry.margin;
      }

    geometry.width = (width == 0) ? 10 : width;
  }

  /* updates the total height of the expression. That is the tallest
     element of the expression (taking the margin into account) */
  void
  update_height ()
  {
    if (elements.size () == 0)
      {
        geometry.height = 0;
        return;
      }

    Element *tallest = elements[0].get ();

    for (size_t i = 1; i < elements.size (); i++)
      {
        Element *el = elements[i].get ();
        if (el->geometry.height + el->geometry.margin
            > tallest->geometry.height + tallest->geometry.margin)

          tallest = el;
      }

    geometry.height = (tallest->geometry.height == 0) ? 10 : tallest->geometry.height;
  }

  /* update the geometry of all elements belonging
     to the expression. Remember that the position is always
     relative to the expression, is not absolute */
  void
  update_elements_geometry ()
  {
    double sep = 0;
    for (size_t i = 0; i < elements.size (); i++)
      {

        Element *el = elements[i].get ();
        el->geometry.x = sep + el->geometry.margin;
        el->geometry.y = el->geometry.margin;
        sep += el->geometry.width;

        el->update_geometry ();
      }
  }
  /* update the whole width and height and the position of each
     element */
  void
  update_geometry ()
  {
    update_width ();
    update_height ();
    update_elements_geometry ();
  }

  Expr ()
  {
    geometry.width = 20;
    geometry.height = 20;
  }
};

struct Cursor
{
  Expr *expr;
  
  Expr *parent = nullptr;
  size_t element_parent_idx = 0;
  union
  {
    // in case we are in the radicand of a root, if not, we are in the index
    bool in_radicand;

    // in case we are in the numerator of a fraction, else we are in the
    // denominator
    bool in_numerator;

  } data;

  // index of the current element
  size_t idx;
};

};

class Display : public Gtk::DrawingArea
{
public:
  void move_left ();
  void move_right ();
  void move_up ();
  void move_down ();

  void insert_symbol (char n);
  void insert_root ();
  void insert_fraction ();

  void wrap_in_fraction_denominator ();
  void wrap_in_fraction_numerator ();
  void wrap_in_root_index ();
  void wrap_in_root_radicand ();
  void enter_root ();
  void enter_fraction ();

  void draw ()
  {
    expr->update_geometry ();
    queue_draw();
  }

  Display ()
  {
    expr = std::make_unique<DisplayAst::Expr>();
    cursor.expr = expr.get();
    cursor.idx = 0;
    set_draw_func(sigc::mem_fun(*this, &Display::on_draw));
    
  }
  
protected:
  void on_draw (const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);  

private:
  DisplayAst::Cursor cursor;
  std::unique_ptr<DisplayAst::Expr> expr;
};
}
#endif

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
#include <stack>
#include <gtkmm.h>

#ifdef DEBUG
#include <iostream>
#endif

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
  double marginX = 0, marginY = 0, margin = 0;

  double
  get_margin_X ()
  {
    if (marginX == 0)
      return margin;

    return marginX;
  }

  double
  get_margin_Y ()
  {
    if (marginY == 0)
      return margin;

    return marginY;
  }
    
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
  POWER,
  SYMBOL
};

struct Element
{
  virtual ~Element () = default;
  ElementType type;
  DrawGeometry geometry;

  bool changed_size = false;
  /* call this method to update the geometry
     attributes of the element based on the new
     changes */
  virtual void update_geometry (const Cairo::RefPtr<Cairo::Context> &cr) {(void) cr;}

  virtual void set_width (double w){(void)w;}
  virtual void set_height (double h){(void)h;}
  
  Element ()
  {
    
  }
};

struct Expr
{
  std::vector<std::unique_ptr<Element> > elements;

  DrawGeometry geometry;

  // if true, don't update the  width or the height here
  bool changed_size = false;

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

  void
  set_width (double w)
  {
    double scale = w / geometry.width;
    
    geometry.width = w;

    
    for (size_t i=0;i<elements.size();i++)
      elements[i]->set_width(elements[i]->geometry.width * scale);


  }

  void
  set_height (double h)
  {
    double scale = h / geometry.height;
    geometry.height = h;


    for (size_t i=0;i<elements.size();i++)
      elements[i]->set_height(elements[i]->geometry.height * scale);

    
  }
  

  
  /* updates the total width of the expression. That is the sum
     of the width and margin of all subelements */
  void
  update_width ()
  {
    double width = 0;
    for (size_t i = 0; i < elements.size (); i++)
      {
        Element *el = elements[i].get ();
        
        width += el->geometry.width + el->geometry.get_margin_X() * 2;
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
        if (el->geometry.height + el->geometry.get_margin_Y() * 2
            > tallest->geometry.height + tallest->geometry.get_margin_Y () * 2)

          tallest = el;
      }

    geometry.height = (tallest->geometry.height == 0) ? 10
      : tallest->geometry.height + tallest->geometry.get_margin_Y () * 2;
  }

  /* update the geometry of all elements belonging
     to the expression. Remember that the position is always
     relative to the expression, is not absolute.
  */
  void
  update_geometry (const Cairo::RefPtr<Cairo::Context> &cr)
  {
    // first, place the elements in the X axis. 
    // Note: also update its geometry to take advantage
    // of this iteration.
    double sep = 0;
    for (size_t i = 0; i < elements.size (); i++)
      {

        Element *el = elements[i].get ();
        // update geometry before doing numbers
        el->update_geometry (cr);
        
        el->geometry.x = sep + el->geometry.get_margin_X ();
        sep += el->geometry.width + el->geometry.get_margin_X() * 2;
      }

    // update_height calculates the necessary height for the expr to fit all elements.
    update_height ();

    // place the elements in the Y axis, aligning them in the center.
    for (size_t i=0;i < elements.size ();i++)
      {
        Element *el = elements[i].get ();
        // remember that elements position are always relative to their expression.
        double y = (geometry.height - el->geometry.height + el->geometry.get_margin_Y()) / 2;

        el->geometry.y = y;
      }

    // finally, update also the width
    update_width ();
   
    
  }
  Expr ()
  {
    geometry.width = 20;
    geometry.height = 20;
  }
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

    geometry.margin = 10;
  }

  void update_geometry (const Cairo::RefPtr<Cairo::Context> &cr) override;
  void set_width (double w) override
  {
    double scale = w / geometry.width;
    geometry.width = w;
    numerator->set_width(numerator->get_width()*scale);
    denominator->set_width(denominator->get_width()*scale);
    
  }
  void set_height (double h) override
  {
    double scale = h / geometry.height;
    geometry.height = h;
    numerator->set_height(numerator->get_height()*scale);
    denominator->set_height(denominator->get_height()*scale);
  }


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

  }

  void update_geometry (const Cairo::RefPtr<Cairo::Context> &cr) override;
  
};

  struct PowerElement : Element
  {
    std::unique_ptr<Expr> base;
    std::unique_ptr<Expr> exponent;

    PowerElement ()
    {
      type = ElementType::POWER;
      base = std::make_unique<Expr> ();
      exponent = std::make_unique<Expr> ();
    }

    void update_geometry (const Cairo::RefPtr<Cairo::Context> &cr) override;
    void set_width (double w) override
    {
      double scale = w / geometry.width;
      geometry.width = w;
      base->set_width(base->get_width()*scale);
      exponent->set_width(exponent->get_width()*scale);
    
    }
    void set_height (double h) override
    {
      double scale = h / geometry.height;
      geometry.height = h;
      base->set_height(base->get_height()*scale);
      exponent->set_height(exponent->get_height()*scale);
    }



  };
  
struct SymbolElement : Element
{
  std::string symbol;
  Glib::ustring symbol_uc;
  
  int font_size = 24;
  
  SymbolElement (Glib::ustring c) : symbol (c), symbol_uc (c)
  {
    type = ElementType::SYMBOL;
    geometry.width = font_size * c.size();
    geometry.height = font_size;
    bool is_digit = (c.size() == 1 && c[0] >= '0' && c[0] <= '9');

    geometry.marginX = (is_digit || c == "e" || c == "π" || c == ".") ? 2 : 10;

    geometry.marginY = 2;
  }

  void update_geometry (const Cairo::RefPtr<Cairo::Context> &cr) override;

  void set_width (double w) override
  {
    geometry.width = w;
    
  }
  void set_height (double h) override
  {
    double scale = h / geometry.height;
    geometry.height = h;
    font_size *= scale;
    
  }



};


struct Cursor
{
  Expr *expr = nullptr;

  /* stack with the parent, the parent of the parent,
     the parent of the parent of the parent, and so on.
     Being the parent the expr containing with an intermediary
    (root radicand/index or fraction numerator/denominator) the
  cursor expr. The size_t is the index of the current element in the expr*/
  std::stack<std::pair<Expr *, size_t>> parents;

  union
  {
    // in case we are in the radicand of a root, if not, we are in the index
    bool in_radicand;

    // in case we are in the numerator of a fraction, else we are in the
    // denominator
    bool in_numerator;

    // in case we are in a power base
    bool in_base;
  } data;

  // index of the current element
  size_t idx = 0;
};

};

class Display : public Gtk::DrawingArea
{
public:
  void move_left ();
  void move_right ();
  void move_up ();
  void move_down ();

  void insert_symbol (std::string n);
  void insert_root ();
  void insert_fraction ();
  void insert_power ();
  
  void wrap_in_fraction_denominator ();
  void wrap_in_fraction_numerator ();
  void wrap_in_root_index ();
  void wrap_in_root_radicand ();
  void wrap_in_power_base ();
  void wrap_in_power_exponent ();
  
  void enter_root ();
  void enter_power_left ();
  void enter_power_right ();
  void enter_fraction_right ();
  void enter_fraction_left ();

  void show_result();
  
  void erase ();
  
  void draw ()
  {
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

  void
  move_to_element ();
};
}
#endif

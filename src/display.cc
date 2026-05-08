// SPDX-FileCopyrightText: 2026 Álex Jiménez
//
// SPDX-License-Identifier: MIT

#include "display.h"

#ifdef DEBUG
#include <iostream>
#endif

namespace GC
{

using namespace DisplayAst;

void
FractionElement::update_geometry ()
{

  // before, update the numerator and denominator geometry.
  numerator->update_geometry ();
  denominator->update_geometry ();

  double width = (numerator->get_width () > denominator->get_width ())
                     ? numerator->get_width ()
                     : denominator->get_width ();

  geometry.width = width;
  geometry.height = numerator->get_height () + denominator->get_height ();

  // set the Y position of denominator and numerator
  denominator->geometry.y = geometry.y;
  numerator->geometry.y = geometry.y + denominator->geometry.height;
    
  // center the numerator and denominator
  denominator->geometry.x = geometry.x + (width - denominator->geometry.width) / 2;
  numerator->geometry.x = geometry.x + (width - numerator->geometry.height) / 2;  

}

void
RootElement::update_geometry ()
{
  // 25px for drawing the root symbol
  double width = radicand->get_width () + index->get_width () + 25;
  double height = radicand->get_height () + index->get_height ();

  geometry.width = width;
  geometry.height = height;

  // justify index to the right and place it at the top
  index->geometry.x = geometry.x - index->geometry.width;
  index->geometry.y = geometry.y;
  radicand->geometry.x = geometry.x + 20;
  radicand->geometry.y = geometry.y;
}

static void
insert_element (std::unique_ptr<Element> el, Cursor cursor)
{
  auto &elements = cursor.expr->elements;
  std::cout << "IDX: " << cursor.idx << std::endl;
  std::cout << "EL: " << el.get() << std::endl;

  elements.insert (elements.begin () + cursor.idx, std::move (el));

  cursor.expr->update_geometry ();
}

/* insert a symbol (number, parenthesis or an operator) */
void
Display::insert_symbol (char s)
{
  auto se = std::make_unique<SymbolElement> (s);
  insert_element (std::move (se), cursor);
  cursor.idx++;
}

/* insert a root and set the cursor into its radicand */
void
Display::insert_root ()
{
  auto re = std::make_unique<RootElement> ();

  // save the raw ptr before std::move
  auto *re_ptr = re.get();
  insert_element (std::move (re), cursor);

  cursor.parent = cursor.expr;
  cursor.element_parent_idx = cursor.idx;
  cursor.expr = re_ptr->radicand.get();
  cursor.data.in_radicand = true;
  cursor.idx = 0;


}

/* insert a fraction and set the cursor ino its denominator */
void Display::insert_fraction ()
{
  auto fe = std::make_unique<FractionElement>();

  // save the raw ptr before std::move
  auto *fe_ptr = fe.get();
  insert_element(std::move(fe), cursor);

  cursor.parent = cursor.expr;
  cursor.element_parent_idx = cursor.idx;
  cursor.expr = fe_ptr->denominator.get();
  cursor.data.in_numerator = false;
  cursor.idx = 0;
}

/* go to the current fraction denominator */
void
Display::wrap_in_fraction_denominator ()
{
  // if we are in a fraction, we are in a nested expression
  if (cursor.parent == nullptr)
    return;
  
  // get the fraction itself
  auto &elements = cursor.parent->elements;
  if (elements.size() == 0)
    return;

  
  auto el = elements[cursor.element_parent_idx].get ();
  
  if (el->type != ElementType::FRACTION)
    return;

  FractionElement *fe = dynamic_cast<FractionElement *> (el);

  cursor.data.in_numerator = false;
  cursor.expr = fe->denominator.get ();
  cursor.idx = 0;
}

/* go to the current fraction numerator */
void
Display::wrap_in_fraction_numerator ()
{
  // if we are in a fraction, we are in a nested expression (probably,
  // the denominator)
  if (cursor.parent == nullptr)
    return;
  
  // get the fraction itself
  auto &elements = cursor.parent->elements;
  if (elements.size() == 0)
    return;

  auto el = elements[cursor.element_parent_idx].get ();

  // check it is really a fraction
  if (el->type != ElementType::FRACTION)
    return;

  FractionElement *fe = dynamic_cast<FractionElement *> (el);

  cursor.data.in_numerator = true;
  cursor.expr = fe->numerator.get ();
  cursor.idx = 0;
}

/* go to the current root index */
void
Display::wrap_in_root_index ()
{
  // the root, as the fraction, is an element in a expression and the index/radicand
  // are expressions in the root. So, the root must be in cursor.parent
  if (cursor.parent == nullptr)
    return;
  
  // get the root itself
  auto &elements = cursor.parent->elements;
  if (elements.size() == 0)
    return;

  
  auto el = elements[cursor.element_parent_idx].get ();

  if (el->type != ElementType::ROOT)
    return;

  RootElement *re = dynamic_cast<RootElement *> (el);

  cursor.data.in_radicand = false;
  cursor.expr = re->index.get ();
  cursor.idx = 0;
}

/* go to the current root radicand */
void
Display::wrap_in_root_radicand ()
{
  // the root, as the fraction, is an element in a expression and the index/radicand
  // are expressions in the root. So, the root must be in cursor.parent
  if (cursor.parent == nullptr)
    return;
  
  // get the root itself
  auto &elements = cursor.parent->elements;
  if (elements.size() == 0)
    return;

  
  auto el = elements[cursor.element_parent_idx].get ();

  if (el->type != ElementType::ROOT)
    return;

  RootElement *re = dynamic_cast<RootElement *> (el);

  cursor.data.in_radicand = true;
  cursor.expr = re->radicand.get ();
  cursor.idx = 0;
}

/* go the following element if it is a root. It will enter
   into the radicand*/
void
Display::enter_root ()
{
  Expr *expr = cursor.expr;
  auto &elements = expr->elements;

  if (cursor.idx >= elements.size ())
    return;

  auto el = elements[cursor.idx].get ();
  if (el->type != ElementType::ROOT)
    return;

  RootElement *re = dynamic_cast<RootElement *>(el);
  
  cursor.parent = cursor.expr;
  cursor.element_parent_idx = cursor.idx;
  cursor.expr = re->radicand.get();
  cursor.data.in_radicand = true;
  cursor.idx = 0;


}

/* go the following element if it is a fraction. It will enter
   into the denominator */
void
Display::enter_fraction ()
{
  Expr *expr = cursor.expr;
  auto &elements = expr->elements;

  if (cursor.idx >= elements.size ())
    return;

  auto el = elements[cursor.idx].get ();
  if (el->type != ElementType::FRACTION)
    return;


  FractionElement *fe = dynamic_cast<FractionElement *>(el);
  
  cursor.parent = cursor.expr;
  cursor.element_parent_idx = cursor.idx;
  cursor.expr = fe->denominator.get();
  cursor.data.in_numerator = false;
  cursor.idx = 0;


  
}

void
draw_rectangles (const Cairo::RefPtr<Cairo::Context> &cr, Expr *expr,
                 double dx, double dy, int width, int height)
{
  
  for (size_t i = 0; i < expr->elements.size (); i++)
    {

      Element *el = expr->elements[i].get ();
      DrawGeometry g = el->geometry;

      /* the Y axis is reversed */
      /* to draw from the lower left corner, i pass 'g.height' as negative value 
      cr->rectangle ((dx + g.x + g.margin),
                     height - (dy + g.y + g.margin), g.width, -g.height);

      cr->stroke ();
      */
      if (el->type == ElementType::FRACTION)
        {
          FractionElement *fe = dynamic_cast<FractionElement *> (el);
          Expr *denominator = fe->denominator.get();
          Expr *numerator = fe->numerator.get();

          double numerator_bottom = numerator->get_y() - numerator->get_margin();

          double denominator_top = denominator->get_y() + denominator->get_height() + denominator->get_margin();
          double y = (numerator_bottom + denominator_top) / 2;


          double width = fe->geometry.width;

          cr->move_to(dx + fe->geometry.x, height - (dy + y));        
          cr->line_to(dx + fe->geometry.x + width, height - (dy + y)); 
          cr->stroke();  
          
          draw_rectangles (cr, denominator,
                           dx+denominator->get_x (),
                           dy+denominator->get_y (),
                           width, height);
          draw_rectangles (cr, numerator,
                           dx+numerator->get_x (),
                           dy+numerator->get_y (),
                           width, height);
        }
      else if (el->type == ElementType::ROOT)
        {
          RootElement *re = dynamic_cast<RootElement *> (el);
          Expr *radicand = re->radicand.get();
          Expr *index = re->index.get();

          draw_rectangles (cr, radicand,
                           dx+radicand->get_x(),
                           dy+radicand->get_y(),
                           width, height);
          draw_rectangles (cr, index,
                           dx+index->get_x(),
                           dy+index->get_y(),
                           width, height);
        }
      else if (el->type == ElementType::SYMBOL)
        {
          SymbolElement *se = dynamic_cast<SymbolElement *> (el);

          // get the text in std::string
          std::string text(1, se->symbol);
          Cairo::TextExtents extents;

          // calculate the position
          cr->get_text_extents(text, extents);

          double x = dx + se->geometry.x + g.margin + (g.width - extents.width) / 2 ;
          double y = dy + se->geometry.y + g.margin + (g.height - extents.height) / 2;

          // show it
          cr->move_to(x, height - y);
          cr->show_text(text);
        }
    }

  std::cout << "hello" << std::endl;
}

  void
  Display::on_draw (const Cairo::RefPtr<Cairo::Context> &cr, int width, int height)
  {
    cr->set_source_rgb(1, 1, 1);
    cr->rectangle(0, 0, width, height);
    cr->fill();

    cr->save();

    cr->set_line_width(2); 
    cr->set_font_size(24);
    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, 
                         Cairo::ToyFontFace::Weight::NORMAL);


    cr->set_source_rgb(0, 0, 0);
    draw_rectangles(cr, expr.get(), 0, 0, width, height);


  }

}

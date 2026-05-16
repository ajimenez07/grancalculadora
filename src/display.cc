// SPDX-FileCopyrightText: 2026 Álex Jiménez
//
// SPDX-License-Identifier: MIT

#include "display.h"
#include "eval.h"


#ifdef DEBUG
#include <iostream>
#endif

#include <algorithm>

// caret size with no element as a reference
#define DEFAULT_CARET_SIZE 24

namespace GC
{

using namespace DisplayAst;

void
FractionElement::update_geometry (const Cairo::RefPtr<Cairo::Context> &cr)
{

  // before, update the numerator and denominator geometry.
  numerator->update_geometry (cr);
  denominator->update_geometry (cr);

  double den_total_width = denominator->get_width() + denominator->get_margin() * 2;
  double num_total_width = numerator->get_width() + numerator->get_margin() * 2;
  
  double width = (den_total_width > num_total_width)
                     ? den_total_width
                     : num_total_width;


  // calculate total height of the denominator and numerator boxes
  double den_total_height = denominator->get_height() + denominator->get_margin() * 2;
  double num_total_height = numerator->get_height() + numerator->get_margin() * 2;

  
  geometry.width = width;
  geometry.height = num_total_height + den_total_height;

  // set the Y position of denominator and numerator
  numerator->geometry.y = den_total_height + numerator->get_margin ();
  denominator->geometry.y = denominator->get_margin();
    
  // center the numerator and denominator
  denominator->geometry.x = (width - den_total_width) / 2 + denominator->get_margin();
  numerator->geometry.x = (width - num_total_width) / 2 + numerator->get_margin();

}

  void
  PowerElement::update_geometry (const Cairo::RefPtr<Cairo::Context> &cr)
  {
    
    exponent->update_geometry (cr);
    base->update_geometry (cr);

    exponent->set_scale(0.75);
    
    geometry.width = base->get_width() + exponent->get_width() + base->get_margin() * 2
      + exponent->get_margin() * 2;
    geometry.height = base->get_height() + exponent->get_height() + base->get_margin() * 2
      + exponent->get_margin() * 2;
    // set positions
    base->geometry.x = geometry.margin;
    base->geometry.y = geometry.margin;
    exponent->geometry.x = base->geometry.x + base->geometry.width + base->geometry.margin;
    exponent->geometry.y = base->geometry.y + base->geometry.height + base->geometry.margin;
  }

  
void
SymbolElement::update_geometry (const Cairo::RefPtr<Cairo::Context> &cr)
{
  Cairo::TextExtents extents;
  cr->get_text_extents(symbol, extents);
    
  geometry.width = extents.width;
  geometry.height = font_size;
}
  
  void Display::move_left()
{
  // case 1: leave parent
  if (cursor.idx == 0 && !cursor.parents.empty())
    {
      auto [parent, idx] = cursor.parents.top();          
      auto el = parent->elements[idx].get();
      // if we are in a fraction, for the user is more comfortable
      // to wrap into the numerator (if we are in the denominator) instead
      // of leaving the whole fraction.
      if (el->type == ElementType::FRACTION)
        {
          if (!cursor.data.in_numerator)
            {
              wrap_in_fraction_numerator();
              draw();
              return;
            }
        }
      // go to power base when moving to left
      else if (el->type == ElementType::POWER)
        {
          {
            if (!cursor.data.in_base)
              {
                wrap_in_power_base();
                draw();
                return;
              }
          }
        }
      cursor.expr = parent;

      cursor.idx = idx;
    
      cursor.parents.pop();
      draw();
      return;
    }
  // case 2: moving in the same expr.
  if (cursor.idx <= cursor.expr->elements.size() && cursor.idx > 0)
    {
      auto el = cursor.expr->elements[cursor.idx - 1].get();
      // enter directly to the element  
      if (el->type == ElementType::FRACTION) 
        enter_fraction_left();  
      else if (el->type == ElementType::POWER)
        enter_power_left();
      else 
        cursor.idx--;  

      draw();
    }
    
}
  void Display::move_right()
  {
    // case 1: leave parent
    if (cursor.idx >= cursor.expr->elements.size() && !cursor.parents.empty())
      {
        
        auto [parent, idx] = cursor.parents.top();
        auto el = parent->elements[idx].get();

        // do the same as we do in move_left but into the last element of the numerator
        // see 'Display::move_left' comments.
        if (el->type == ElementType::FRACTION)
          {
            if (!cursor.data.in_numerator)
              {
                wrap_in_fraction_numerator();
                // 'wrap_in_fraction_numerator' sets the idx to 0,
                // but if we move to right, we want to go the last element
                // of the numerator
                cursor.idx = cursor.expr->elements.size();
                draw();
                return;
              }
          }
        // go to power exponent when moving to right
      else if (el->type == ElementType::POWER)
        {
          
          if (cursor.data.in_base)
            {
              wrap_in_power_exponent();
              draw();
              return;
            }
          
        }

        cursor.expr = parent;
          // check boundaries
        if (idx + 1 <= cursor.expr->elements.size()) 
          cursor.idx = idx + 1;
        else 
          cursor.idx = cursor.expr->elements.size(); 

        cursor.parents.pop();
        draw();
        return;
      }

    // case 2: moving in the same expression.
    if (cursor.idx < cursor.expr->elements.size())
      {
        auto el = cursor.expr->elements[cursor.idx].get();
        
        if (el->type == ElementType::FRACTION) 
          enter_fraction_right();  
        else if (el->type == ElementType::POWER)
          enter_power_right();
        else  
          cursor.idx++;  
        
    }
    
    draw();
  }

  
  void
  Display::move_up ()
  {
    // try to get the current element parent
    if (cursor.parents.empty())
      return;
  
    auto [parent, idx] = cursor.parents.top();
    auto &elements = parent->elements;
    
    if (idx >= elements.size())
      return;

    auto el = elements[idx].get ();

    // go to the numerator if we are in the denominator
    if (el->type == ElementType::FRACTION && !cursor.data.in_numerator)
      wrap_in_fraction_numerator ();

    // go to the exponent if we are in a power
    else if (el->type == ElementType::POWER && cursor.data.in_base)
      wrap_in_power_exponent();
    
    draw();
    
  }
  void
  Display::move_down ()
  {
    // try to get the current element parent
    if (cursor.parents.empty())
      return;
  
    auto [parent, idx] = cursor.parents.top();
    auto &elements = parent->elements;
    
    if (idx >= elements.size())
      return;

    auto el = elements[idx].get ();

    // go to the denominator if we are in the numerator
    if (el->type == ElementType::FRACTION && cursor.data.in_numerator)
      wrap_in_fraction_denominator ();

    // go to the base if we are in a power
    else if (el->type == ElementType::POWER && !cursor.data.in_base)
      wrap_in_power_base();
    

    draw();
  }


  
  
static void
insert_element (std::unique_ptr<Element> el, Cursor cursor)
{
  auto &elements = cursor.expr->elements;

  elements.insert (elements.begin () + cursor.idx, std::move (el));
  
}

/* insert a symbol (number, parenthesis or an operator) */
void
Display::insert_symbol (std::string s)
{
  auto se = std::make_unique<SymbolElement> (s);
  insert_element (std::move (se), cursor);
  cursor.idx++;

  draw();
}

/* insert a fraction and set the cursor into its denominator */
void Display::insert_fraction ()
{
  auto fe = std::make_unique<FractionElement>();

  // save the raw ptr before std::move
  auto *fe_ptr = fe.get();
  insert_element(std::move(fe), cursor);

  
  cursor.parents.push({cursor.expr, cursor.idx});;
  cursor.expr = fe_ptr->denominator.get();
  cursor.data.in_numerator = false;
  cursor.idx = 0;

  draw();
}

/* insert a power and set the cursor into its base */
void Display::insert_power ()
{
  auto pe = std::make_unique<PowerElement>();

  // save the raw ptr before std::move
  auto *pe_ptr = pe.get();
  insert_element(std::move(pe), cursor);

  
  cursor.parents.push({cursor.expr, cursor.idx});;
  cursor.expr = pe_ptr->base.get();
  cursor.data.in_base = true;
  cursor.idx = 0;

  draw();
}


  
/* go to the current fraction denominator.

 Note: for each call of this function it is necessary to check
if we are really in a fraction. Undefined behavior may occur if this is not done. */
void
Display::wrap_in_fraction_denominator ()
{
  
  // get the fraction itself
  auto [parent, idx] = cursor.parents.top();

  auto &elements = parent->elements;
  auto el = elements[idx].get ();

  FractionElement *fe = dynamic_cast<FractionElement *> (el);

  cursor.data.in_numerator = false;
  cursor.expr = fe->denominator.get ();
  cursor.idx = 0;
}

/* go to the current fraction numerator.

 Note: for each call of this function it is necessary to check
if we are really in a fraction. Undefined behavior may occur if this is not done. */
void
Display::wrap_in_fraction_numerator ()
{
  
  // get the fraction itself
  auto [parent, idx] = cursor.parents.top();
  auto &elements = parent->elements;
  
  auto el = elements[idx].get ();

  FractionElement *fe = dynamic_cast<FractionElement *> (el);

  cursor.data.in_numerator = true;
  cursor.expr = fe->numerator.get ();
  cursor.idx = 0;
}


/* go to the current power base */
void
Display::wrap_in_power_base ()
{
  auto [parent, idx] = cursor.parents.top();

  auto &elements = parent->elements;
  auto el = elements[idx].get ();

  PowerElement *pe = dynamic_cast<PowerElement *> (el);

  cursor.data.in_base = true;
  cursor.expr = pe->base.get ();
  cursor.idx = 0;  

}

/* go to the current power exponent */
void
Display::wrap_in_power_exponent ()
{
  auto [parent, idx] = cursor.parents.top();

  auto &elements = parent->elements;
  auto el = elements[idx].get ();

  PowerElement *pe = dynamic_cast<PowerElement *> (el);

  cursor.data.in_base = false;
  cursor.expr = pe->exponent.get ();
  cursor.idx = 0;  

}

static void
enter_fraction(FractionElement *fe, Cursor &cursor)
{
  cursor.parents.push({cursor.expr, cursor.idx});
  cursor.expr = fe->denominator.get();
  cursor.data.in_numerator = false;
  cursor.idx = 0;  
}
  
/* go the previous element if it is a fraction. It will enter
   into the denominator */
void
Display::enter_fraction_left ()
{
  Expr *expr = cursor.expr;
  auto &elements = expr->elements;

  if (cursor.idx <= 0 || elements.size() == 0)
    return;

  auto el = elements[cursor.idx - 1].get ();
  if (el->type != ElementType::FRACTION)
    return;

  FractionElement *fe = dynamic_cast<FractionElement *>(el);

  if (cursor.idx > 0)
    cursor.idx--;
  
  enter_fraction (fe, cursor);
}

/* go the next element if it is a fraction. It will enter
   into the denominator */
void
Display::enter_fraction_right ()
{
  Expr *expr = cursor.expr;
  auto &elements = expr->elements;

  if (cursor.idx >= elements.size ())
    return;

 
  auto el = elements[cursor.idx].get ();
  if (el->type != ElementType::FRACTION)
    return;

  FractionElement *fe = dynamic_cast<FractionElement *>(el);
  if (cursor.idx + 1 < elements.size())
    cursor.idx++;
  
  enter_fraction (fe, cursor);
}

void
Display::enter_power_right ()
{
  Expr *expr = cursor.expr;
  auto &elements = expr->elements;

  if (cursor.idx >= elements.size ())
    return;
 
  auto el = elements[cursor.idx].get ();
  if (el->type != ElementType::POWER)
    return;

  PowerElement *pe = dynamic_cast<PowerElement *>(el);
  if (cursor.idx + 1 < elements.size())
    cursor.idx++;


  cursor.parents.push({cursor.expr, cursor.idx});
  cursor.expr = pe->base.get();
  cursor.data.in_base = true;
  cursor.idx = 0;

}

void
Display::enter_power_left ()
{
  Expr *expr = cursor.expr;
  auto &elements = expr->elements;

  if (cursor.idx <= 0 || elements.size() == 0)
    return;


  auto el = elements[cursor.idx - 1].get ();
  if (el->type != ElementType::POWER)
    return;

  PowerElement *pe = dynamic_cast<PowerElement *>(el);  
  if (cursor.idx > 0)
    cursor.idx--;

  cursor.parents.push({cursor.expr, cursor.idx});
  cursor.expr = pe->exponent.get();
  cursor.data.in_base = false;
  cursor.idx = 0;

}
 


void
Display::erase ()
{
  // if cursor.idx == 0, then there is no element.
  if (cursor.idx == 0)
    return;

  // erase the element
  auto &elements = cursor.expr->elements;
  elements.erase (elements.begin() + cursor.idx - 1);
  // important: update the cursor.
  cursor.idx--;
  
  // update
  draw();
}

void
draw_elements (const Cairo::RefPtr<Cairo::Context> &cr, Expr *expr,
                 double dx, double dy, int width, int height, Cursor cursor)
{

  // don't draw cursor if it is not the moment
  if (expr != cursor.expr)
    goto draw_elements;


  
  if (cursor.expr->elements.size() == 0) // take symbol height as a reference in case there are no elements
    {
      double x = dx;
      double y = dy;

      // take the symbol height as a reference
      cr->move_to(x, height - y);        
      cr->line_to(x, height - (y + DEFAULT_CARET_SIZE)); 
      cr->stroke();
    }
  // we have elements
  else
    {
      Element *reference;

      // provisional enum to indicate if we have to trace the left side or the right side
      enum class Where {LEFT, RIGHT};
      Where where = Where::LEFT;

        // get the element which the cursor points to (trace the left side)     
      if (cursor.idx < cursor.expr->elements.size())     
        reference = expr->elements[cursor.idx].get ();
      
      // the cursor is at the end, so trace the right side of the last element. 
      else if (cursor.idx >= cursor.expr->elements.size())
        {
          reference = expr->elements[cursor.expr->elements.size() - 1].get ();         
          where = Where::RIGHT;
        }
      else
        goto draw_elements;
      

      // draw the caret

      // calculate its absolute position. Note that dx and dy already
      // includes the parent expression and the parent element positions.
      double x = dx + reference->geometry.x;
      
      if (where == Where::RIGHT)
        x += (reference->geometry.get_margin_X() * 2 + reference->geometry.width) * reference->geometry.scale;
      else
        x -= reference->geometry.get_margin_X() * reference->geometry.scale;
      
      double y = dy +
        reference->geometry.y + reference->geometry.get_margin_Y() * reference->geometry.scale;

      // draw the line
      cr->move_to(x, height - y);        
      cr->line_to(x, height - (y + reference->geometry.height * reference->geometry.scale)); 
      cr->stroke();  
    }

 draw_elements:
    
  // draw elements
  for (size_t i = 0; i < expr->elements.size (); i++)
    {

      Element *el = expr->elements[i].get ();
      DrawGeometry g = el->geometry;

      if (el->type == ElementType::FRACTION)
        {
          FractionElement *fe = dynamic_cast<FractionElement *> (el);
          Expr *denominator = fe->denominator.get();
          Expr *numerator = fe->numerator.get();

          double numerator_bottom = numerator->get_y() - numerator->get_margin () * numerator->get_scale();
          double denominator_top = denominator->get_y() + (denominator->get_height ()
                                                           + denominator->get_margin()) * denominator->get_scale();


          // note: don't apply margins. margins are always applied by the expression
          // when calculating relative positions.
          
          double y = (numerator_bottom + denominator_top) / 2 + fe->geometry.y + dy;

          cr->move_to(dx + fe->geometry.x, height - y);        
          cr->line_to(dx + fe->geometry.x + fe->geometry.width * fe->geometry.scale, height  - y); 
          cr->stroke();  
          
          draw_elements (cr, denominator,
                           dx+fe->geometry.x+denominator->get_x (),
                           dy+fe->geometry.y+denominator->get_y (),
                           width, height, cursor);
          draw_elements (cr, numerator,
                           dx+fe->geometry.x+numerator->get_x (),
                           dy+fe->geometry.y+numerator->get_y (),
                           width, height, cursor);
        }
      else if (el->type == ElementType::POWER)
        {
          PowerElement *pe = dynamic_cast<PowerElement *> (el);
          Expr *base = pe->base.get();
          Expr *exponent = pe->exponent.get();

          draw_elements (cr, base,
                         dx + pe->geometry.x + pe->base->get_x(),
                         dy + pe->geometry.y + pe->base->get_y(),
                         width, height, cursor);

          draw_elements (cr, exponent,
                         dx + pe->geometry.x + pe->exponent->get_x(),
                         dy + pe->geometry.y + pe->exponent->get_y(),
                         width, height, cursor);
          

        }
      else if (el->type == ElementType::SYMBOL)
        {
              
          SymbolElement *se = dynamic_cast<SymbolElement *> (el);
          cr->set_font_size(se->font_size * se->geometry.scale);
          Cairo::TextExtents extents;

          // calculate the position
          cr->get_text_extents(se->symbol, extents);
    
          // don't apply margins, they are applied by the expression.
          double x = dx + se->geometry.x + (g.width - extents.width) / (2 * se->geometry.scale);

          // if it is a dot, don't center it vertically.
          double y = dy + se->geometry.y + ((se->symbol == ".") ?
                                            extents.height : (g.height - extents.height) / 2) * se->geometry.scale;
          // show it
          cr->move_to(x, height - y);
          cr->show_text(se->symbol);
        }
    }

}

void
Display::show_result()
{
  Glib::ustring msg;
  Expr *res = eval (expr.get(), msg);

  if (msg == "success")
    {
      expr = std::unique_ptr<Expr>(res);
      cursor.expr = expr.get();
      cursor.idx = 0;
      // empty the parents
      while (!cursor.parents.empty())
        cursor.parents.pop();



    }
  queue_draw();

}
void
Display::clear()
{
  expr = std::make_unique<Expr>();
  cursor.expr = expr.get();
  cursor.idx = 0;
  // empty the parents
  while (!cursor.parents.empty())
    cursor.parents.pop();

  queue_draw();
};
  

void
  Display::on_draw (const Cairo::RefPtr<Cairo::Context> &cr, int width, int height)
  {

    cr->set_line_width(2); 

    // set font size
    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, 
                         Cairo::ToyFontFace::Weight::NORMAL);

    // then update elements geometries
    expr->update_geometry (cr);
    
    cr->set_source_rgb(1, 1, 1);
    cr->rectangle(0, 0, width, height);
    cr->fill();

    cr->save();


    cr->set_source_rgb(0, 0, 0);
    draw_elements(cr, expr.get(), 0, 0, width, height, cursor);

    
  }

}

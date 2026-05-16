// SPDX-FileCopyrightText: 2026 Álex Jiménez
//
// SPDX-License-Identifier: MIT

#include "eval.h"
#include "parser.h"

#include <iomanip>

using namespace GC::Parser;
using namespace GC::DisplayAst;

static inline double
eval_expr (MathExpr *expr, bool &success);

static inline double
eval_unary (UnaryExpr *expr, bool &success)
{
  switch (expr->op)
    {
    case UnaryExpr::Op::PLUS:
    case UnaryExpr::Op::PAREN:
      return eval_expr (expr->operand.get (), success);
    case UnaryExpr::Op::MINUS:  
      return -1 * eval_expr (expr->operand.get (), success);
    }
  success = false;
  return 0;
}

static inline double
eval_binary (BinaryExpr *expr, bool &success)
{
  success = true;
  double r = eval_expr (expr->right.get(), success);
  double l = eval_expr (expr->left.get(), success);
 
  switch (expr->op)
    {
    case BinaryExpr::Op::ADD:
      return l + r;
    case BinaryExpr::Op::SUB:
      return l - r;
    case BinaryExpr::Op::MUL:  
      return l * r;
    case BinaryExpr::Op::DIV:
      // return error on division by zero
      if (r == 0)
        {
          success = false;
          return 0;
        }
      return l / r;
    case BinaryExpr::Op::POWER:
      return std::pow(l, r);
    }
  
  success = false;
  return 0;

}

static inline double
eval_expr (MathExpr *expr, bool &success)
{
  success = true;
  switch (expr->type)
    {
    case MathExpr::Type::LITERAL:
      {
        Literal *lit = dynamic_cast<Literal *>(expr);
        return lit->value;
      }
    case MathExpr::Type::UNARY:
      {
        UnaryExpr *un = dynamic_cast<UnaryExpr *>(expr);
        return eval_unary (un, success);
      }
    case MathExpr::Type::BINARY:
      {
        BinaryExpr *bin = dynamic_cast<BinaryExpr *>(expr);
        return eval_binary (bin, success);
      }
    }
  
  success = false;
  return 0;
}

Expr *
GC::eval (Expr *input, Glib::ustring &msg)
{  
  auto expr = parse (input, msg);
  if (msg == "success")
    {
      bool success = true;
      double res = eval_expr (expr.get(), success);
      if (!success)
        {
          msg = "evaluation error";
          return 0;
        }

      // format the result
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(8) << res;
      Glib::ustring formatted = oss.str();

      Expr *result = new Expr;

      auto &elements = result->elements;      
      for (gunichar c : formatted)
        {
          Glib::ustring s;     
          if (c == ',')
            s += ".";
          else
            s += c;              
          elements.push_back(std::make_unique<SymbolElement>(s));
          
        }
      
      return result;
    }

  return 0;
}
  

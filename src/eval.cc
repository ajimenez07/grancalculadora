#include "eval.h"
#include "parser.h"

using namespace GC::Parser;


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
  double r = eval_expr (expr->left.get(), success);
  double l = eval_expr (expr->right.get(), success);
 
  switch (expr->op)
    {
    case BinaryExpr::Op::ADD:
      return r + l;
    case BinaryExpr::Op::SUB:
      return r - l;
    case BinaryExpr::Op::MUL:  
      return r * l;
    case BinaryExpr::Op::DIV:
      // return error on division by zero
      if (l == 0)
        {
          success = false;
          return 0;
        }
      return r / l;
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

double
GC::eval (GC::DisplayAst::Expr *input, Glib::ustring &msg)
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
      return res;
    }

  return 0;
}
  

/*
 * SPDX-FileCopyrightText: 2026 Álex Jiménez
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef GC_PARSER_H
#define GC_PARSER_H

#include <array>
#include <string>
#include <unordered_map>

#include "display.h"

namespace GC
{

namespace Tokenizer
{

struct Token
{
  enum class TokenType
  {
    PLUS,
    MINUS,
    MUL,
    DIV,
    OPEN_PAREN,
    CLOSE_PAREN,
    NUMBER,
    FRACTION,
    POWER
  };

  TokenType type;

  union
  {
    double value;                          // in case it is a number
    DisplayAst::FractionElement *fraction; // in case it is a fraction
    DisplayAst::PowerElement *power;       // in case it is a power
  } data;

  // for code readability, avoid implicit conversions in constructors

  explicit Token (TokenType type) : type (type) {}

  explicit Token (double val) : type (TokenType::NUMBER) { data.value = val; }

  explicit Token (DisplayAst::FractionElement *fe) : type (TokenType::FRACTION)
  {
    data.fraction = fe;
  }

  explicit Token (DisplayAst::PowerElement *pe) : type (TokenType::POWER)
  {
    data.power = pe;
  }
};

Glib::ustring tokenize (DisplayAst::Expr *expr, std::vector<Token> &tokens);
}

namespace Parser
{

struct MathExpr
{
  virtual ~MathExpr () = default;
  enum class Type
  {
    LITERAL,
    UNARY,
    BINARY
  } type;
  MathExpr (Type type) : type (type) {};
};

struct Literal : MathExpr
{
  double value;
  Literal () : MathExpr (MathExpr::Type::LITERAL) {}
};

struct UnaryExpr : MathExpr
{
  enum class Op
  {
    PLUS,
    MINUS,
    PAREN
  } op;
  std::unique_ptr<MathExpr> operand;

  UnaryExpr () : MathExpr (MathExpr::Type::UNARY) {}
};

struct BinaryExpr : MathExpr
{
  enum class Op
  {
    ADD,
    SUB,
    MUL,
    DIV,
    POWER
  } op;
  std::unique_ptr<MathExpr> left;
  std::unique_ptr<MathExpr> right;

  BinaryExpr () : MathExpr (MathExpr::Type::BINARY) {}
};

MathExpr *parse (GC::DisplayAst::Expr *input_expr);

std::unique_ptr<MathExpr> parse (GC::DisplayAst::Expr *input_expr,
                                 Glib::ustring &msg);
}

}
#endif

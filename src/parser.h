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

namespace GC
{

struct Value;
struct FunctionCall;

struct Function
{
  std::string identifier;
  Value (*handler) (FunctionCall);

  Function (std::string identifier, Value (*handler) (FunctionCall))
      : identifier (identifier), handler (handler)
  {
  }
};

Function funcs[] = {

  Function ("sqrt", nullptr), Function ("root", nullptr)
};

std::unordered_map<std::string, int> func_ids;

enum class BinaryOp
{
  SUM,
  SUB,
  DIV,
  MUL,
  POW
};

enum class UnaryOp
{
  SUM,
  SUB,
  PAREN, // expr with parenthesis: (x + 2)
};

enum class ValueType
{
  DOUBLE,
  INTEGER
};

struct UnaryExpr;
struct BinaryExpr;
struct Function;

enum class OperandType
{
  LITERAL,           // A number
  UNARY_EXPRESSION,  // a subexpression that is unary
  BINARY_EXPRESSION, // a subexpression that is binary
  FUNCTION
};

struct Value
{
  union
  {
    double d;
    int i;
  } number;

  ValueType type;
};

struct Operand
{
  union
  {
    Value *val;
    UnaryExpr *expr;
    BinaryExpr *expr;

  } operand;

  OperandType type;
};

struct UnaryExpr
{
  Operand *operand;
  UnaryOp op;
};

struct BinaryExpr
{
  Operand *left;
  Operand *right;
  BinaryOp op;
};

struct FunctionCall
{
  int id;
  std::array<Operand, 2> args;
};

/* seems weird, but since an Operand can be a number or a function or
   a subexpression, we can treat the whole input as a single expression*/
using MathExpression = Operand;

std::string parse (std::string &input, MathExpression &expr, size_t &last_pos);

}

#endif

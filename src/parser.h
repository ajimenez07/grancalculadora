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
          PLUS, MINUS,
          MUL, DIV,
          OPEN_PAREN, CLOSE_PAREN,
          NUMBER, FRACTION
        };


      TokenType type;
      
      union
      {
        double value; // in case it is a number
        DisplayAst::FractionElement *fraction; // in case it is a fraction
      } data;


      // for code readability, avoid implicit conversions in constructors

      explicit Token (TokenType type) : type(type) {}
      
      explicit Token (double val) : type(TokenType::NUMBER)
      {
        data.value = val;
      }

      explicit Token (DisplayAst::FractionElement *fe) : type(TokenType::FRACTION)
      {
        data.fraction = fe;
      }
    };
    
    Glib::ustring tokenize (DisplayAst::Expr *expr,
                            std::vector<Token> &tokens);
  }

  namespace Parser
  {
    
    struct MathExpr
    {
      virtual ~MathExpr() = default;
    };

    struct Literal : MathExpr
    {
      double value;
    };

    struct UnaryExpr : MathExpr
    {
      enum class Op { PLUS, MINUS, PAREN } op;
      std::unique_ptr<MathExpr> operand;
    };

    struct BinaryExpr : MathExpr
    {
      enum class Op { ADD, SUB, MUL, DIV } op;
      std::unique_ptr<MathExpr> left;
      std::unique_ptr<MathExpr> right;
    };

    Glib::ustring parse (DisplayAst::Expr *input_expr, MathExpr &ast);
  }

}
#endif

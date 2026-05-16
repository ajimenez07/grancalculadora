// SPDX-FileCopyrightText: 2026 Álex Jiménez
//
// SPDX-License-Identifier: MIT

#include "parser.h"

#include <cmath>
#include <cctype>
#include <charconv>

#ifdef DEBUG
#include <iostream>
#endif

using namespace GC::DisplayAst;
using namespace GC::Tokenizer;


#ifdef DEBUG
static std::string token_type_to_string(Token::TokenType type) {
    switch (type) {
        case Token::TokenType::PLUS:        return "PLUS";
        case Token::TokenType::MINUS:       return "MINUS";
        case Token::TokenType::MUL:         return "MUL";
        case Token::TokenType::DIV:         return "DIV";
        case Token::TokenType::OPEN_PAREN:  return "OPEN_PAREN";
        case Token::TokenType::CLOSE_PAREN: return "CLOSE_PAREN";
        case Token::TokenType::NUMBER:      return "NUMBER";
        case Token::TokenType::FRACTION:    return "FRACTION";
    case Token::TokenType::POWER:    return "POWER";
    default:                            return "UNKNOWN";
    }
}

static void print_token(Token token) {
    std::cout << "Token{type=" << token_type_to_string(token.type);
    
    if (token.type == Token::TokenType::NUMBER) {
        std::cout << ", value=" << token.data.value;
    } else if (token.type == Token::TokenType::FRACTION) {
        std::cout << ", fraction=" << token.data.fraction;
    }
    
    std::cout << "}" << std::endl;
}
#endif
 

namespace GC
{

  namespace Tokenizer
  {
    Glib::ustring
    tokenize (DisplayAst::Expr *expr,
              std::vector<Token> &tokens)
    {
      // don't use this macro! It was specially created for this function
#define IS_TOKEN(expected, type)                \
      else if (se->symbol == expected)          \
        do                                      \
          {                                     \
            tokens.push_back (Token(type));     \
          }                                     \
        while(0)                                    
  

      auto &elements = expr->elements;

      // a number is a sequence of digits (characters) where
      // each digit is a separate SymbolElement, so while reading
      // the symbols we need to have a buffer to keep the digits.
      std::string number;

      // to keep the tokens order, we need to track the index we
      // are when starting reading a number.
      size_t idx_for_number = 0;
      
      for (size_t i=0; i < elements.size (); i++)
        {
          Element *el = elements[i].get ();

          // if it is a symbol (basically a character or a string)
          if (el->type == ElementType::SYMBOL)
            {
              // if it is only a character
              SymbolElement *se = dynamic_cast<SymbolElement *>(el);
              char ch = se->symbol[0];
            
              // if is the number PI, push directly the value
              if (se->symbol == "π")
                tokens.push_back (Token (M_PI));

              // characters/strs which are themselves a token
              IS_TOKEN ("(", Token::TokenType::OPEN_PAREN);
              IS_TOKEN (")", Token::TokenType::CLOSE_PAREN);
              IS_TOKEN ("+", Token::TokenType::PLUS);
              IS_TOKEN ("-", Token::TokenType::MINUS);
              IS_TOKEN ("×", Token::TokenType::MUL);
              IS_TOKEN ("÷", Token::TokenType::DIV);

              // if is a digit or a dot, add the character to the buffer
              // and do a 'continue' to avoid pushing a token prematurely.
              else if (isdigit(ch) || ch == '.')             
                {
                  if (number.empty())
                    idx_for_number = tokens.size();
                      
                  number += ch;

                  // if we do a continue in the last element
                  // the number will not be saved, so check it is not the last.
                  if (i < elements.size() - 1)
                    continue;
                }
            }
          else
            {
              // a fraction is treated as a single token
              if (el->type == ElementType::FRACTION)
                tokens.push_back (Token(dynamic_cast<FractionElement *>(el)));

              else if (el->type == ElementType::POWER)
                tokens.push_back (Token(dynamic_cast<PowerElement *>(el)));


              // don't do a continue, we need to check if previously we were
              // reading a number token.
            }

          // if we are here, we need to push a number token.
      
          // if the number buffer is not empty, we finished reading a number token
          // so create the token and add it to the vector.
          if (!number.empty ())
            {
              double value;

              const char *start = number.c_str();
              const char *end = start + number.size();

              auto [ptr, ec] = std::from_chars(start, end, value);

              // easteregg for lazy people: 96. == 96.0
              if (ptr == end && ec == std::errc()) 
                tokens.insert(tokens.begin() + idx_for_number, Token(value));
              else
                return "syntax error";
              

              number.clear ();
                
            }          
        }    

      return "success";
    }

  }
  /*
expr    -> term
        | term '+' expr
        | term '-' expr

term    -> power
        | power '*' term
        | power '/' term

power   -> primary
        | primary '^' power      // asociatividad por derecha

primary -> operand
        | '(' expr ')'
        | '+' primary
        | '-' primary

operand -> FRACTION
        | NUMBER
*/ 
  namespace Parser
  {
    
    struct ParserCursor
    {
      std::vector<Token> tokens;
      size_t idx = 0;

      Token get() {
        Token tk = tokens[(idx  >= tokens.size()) ? tokens.size()-1 : idx++];
#ifdef DEBUG
        std::cout << "[get " << this << "]";
        print_token(tk);
#endif
        return tk;
      }
      
      Token ahead() {
        Token tk = tokens[(idx  >= tokens.size()) ? tokens.size()-1 : idx];
#ifdef DEBUG
        std::cout << "[ahead " << this << "]";
        print_token(tk);
#endif
        return tk;

      }
      bool still_has() {return idx < tokens.size();}
      
      explicit ParserCursor(std::vector<Token> tokens) :
        tokens(tokens) {}
    };

    MathExpr *
    parse_operand (ParserCursor &cursor)
    {

      if (!cursor.still_has())
        return nullptr;
      
      Token tk = cursor.ahead();

      
      if (tk.type == Token::TokenType::NUMBER)
        {
          cursor.get();

          Literal *lit = new Literal;
          lit->value = tk.data.value;
          return dynamic_cast<MathExpr *>(lit);
        }
      else if (tk.type == Token::TokenType::FRACTION)
        {
          
          cursor.get();
         
          Expr *num = tk.data.fraction->numerator.get();
          Expr *den = tk.data.fraction->denominator.get();
          
          MathExpr *parsed_num = parse (num);
          MathExpr *parsed_den = parse (den);

          if (!parsed_num || !parsed_den)
            {              
              if (parsed_num) delete parsed_num;
              if (parsed_den) delete parsed_den;
              return nullptr;
            }
          BinaryExpr *bin = new BinaryExpr;
          
          bin->left = std::unique_ptr<MathExpr>(parsed_num);
          bin->right = std::unique_ptr<MathExpr>(parsed_den);
          bin->op = BinaryExpr::Op::DIV;

          return dynamic_cast<MathExpr *>(bin);


        }
      else
        return nullptr;
    }

    // forward declaration.
    MathExpr * parse_expr (ParserCursor &cursor);
    
    MathExpr *
    parse_primary (ParserCursor &cursor)
    {

      if (!cursor.still_has())
        return nullptr;

      Token tk = cursor.ahead();

      // +x or -x
      if (tk.type == Token::TokenType::PLUS
          || tk.type == Token::TokenType::MINUS)
        {

          cursor.get();
          
          MathExpr *expr = parse_primary(cursor);
          if (!expr)
            return nullptr;

          UnaryExpr *parent = new UnaryExpr;
          parent->operand = std::unique_ptr<MathExpr>(expr);
          parent->op = (tk.type == Token::TokenType::PLUS) ?
            UnaryExpr::Op::PLUS : UnaryExpr::Op::MINUS;
          return parent;
        }

      
      // (x)
      if (tk.type == Token::TokenType::OPEN_PAREN)
        {
          cursor.get();
          
          MathExpr *expr = parse_expr(cursor);
          if (!expr)
            return nullptr;

          if (!cursor.still_has() ||
              cursor.get().type != Token::TokenType::CLOSE_PAREN)
            {
              delete expr;
              return nullptr;
            }
            
          UnaryExpr *parent = new UnaryExpr;
          parent->operand = std::unique_ptr<MathExpr>(expr);
          parent->op = UnaryExpr::Op::PAREN;
          return parent;
        }

      // last case: a single operand
      MathExpr *expr = parse_operand(cursor);
      
      return expr; // expr is already nullptr in failure
    }

    MathExpr *
    parse_power (ParserCursor &cursor)
    {
      
      Token tk = cursor.ahead();
      
      if (tk.type == Token::TokenType::POWER)
        {
          cursor.get();

          Expr *base = tk.data.power->base.get();
          Expr *exp = tk.data.power->exponent.get();
         
          MathExpr *parsed_base = parse (base);
          MathExpr *parsed_exp = parse (exp);

          if (!parsed_base || !parsed_exp)
            {              
              if (parsed_base) delete parsed_base;
              if (parsed_exp) delete parsed_exp;
              return nullptr;
            }
          // create the power
          BinaryExpr *power = new BinaryExpr;
          power->left = std::unique_ptr<MathExpr>(parsed_base);
          power->right = std::unique_ptr<MathExpr>(parsed_exp);
          power->op = BinaryExpr::Op::POWER;

          return dynamic_cast<MathExpr *>(power);

        }

      // not invalid
      return parse_primary (cursor);
    }

    MathExpr *
    parse_term (ParserCursor &cursor)
    {
      MathExpr *power = parse_power (cursor);
      if (!power)
        return nullptr;

      if (!cursor.still_has())
        return power;

      // don't advance here, even with no tokens,
      // may we have a valid input
      Token tk = cursor.ahead();

      if (tk.type == Token::TokenType::MUL
          || tk.type == Token::TokenType::DIV)
        {
          // we know the token that goes next, so is not necessary
          // to update 'tk' variable.
          cursor.get();

          // delete power in failure
          MathExpr *term = parse_term(cursor);
          if (!term)
            {
              delete power;
              return nullptr;
            }

          // create the binary expr and return it
          BinaryExpr *bin = new BinaryExpr;
          bin->left = std::unique_ptr<MathExpr>(power);
          bin->right = std::unique_ptr<MathExpr>(term);
          bin->op = (tk.type == Token::TokenType::MUL) ?
            BinaryExpr::Op::MUL : BinaryExpr::Op::DIV;

          return dynamic_cast<MathExpr *>(bin);
          
        }

      // not invalid input, is totally valid.
      return power;
    }

    // same structure as term
    MathExpr *
    parse_expr (ParserCursor &cursor)
    {
      MathExpr *term = parse_term (cursor);
      if (!term)
        return nullptr;

      if (!cursor.still_has())
        return term;

      Token tk = cursor.ahead();

      if (tk.type == Token::TokenType::PLUS
          || tk.type == Token::TokenType::MINUS)
        {
          
          cursor.get();

          // failure, delete term
          MathExpr *expr= parse_expr(cursor);
          if (!expr)
            {
              delete term;
              return nullptr;
            }

          // create the binary expr and return it
          BinaryExpr *bin = new BinaryExpr;
          bin->left = std::unique_ptr<MathExpr>(term);
          bin->right = std::unique_ptr<MathExpr>(expr);
          bin->op = (tk.type == Token::TokenType::PLUS) ?
            BinaryExpr::Op::ADD : BinaryExpr::Op::SUB;

          return dynamic_cast<MathExpr *>(bin);
          
        }

      // not invalid input, is totally valid.
      return term;
    }

    MathExpr *
    parse (Expr *input_expr)
    {
      std::vector<Tokenizer::Token> tokens;
      auto msg = Tokenizer::tokenize (input_expr, tokens);

      
      if (msg == "success")
          {
          ParserCursor cursor = ParserCursor(tokens);
          auto expr = parse_expr (cursor);
          return expr;
          
        }
             
      return nullptr;
    }

  


    std::unique_ptr<MathExpr>
    parse (Expr *input_expr, Glib::ustring &msg)
    {
      std::vector<Tokenizer::Token> tokens;
      msg = Tokenizer::tokenize (input_expr, tokens);
            
      if (msg == "success")
        {
          ParserCursor cursor = ParserCursor(tokens);
          auto expr = parse_expr (cursor);

          if (expr)
            return std::unique_ptr<MathExpr>(expr);
            
          else
              msg = "syntax error";
              
            
        }
      return nullptr;
    }

  }


}

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
                  continue;
                }
            }
          else
            {
              // a fraction is treated as a single token
              if (el->type == ElementType::FRACTION)
                {
                  tokens.push_back (Token(dynamic_cast<FractionElement *>(el)));
                  continue;
                }
            }

          // if we are here, we need to push a number token.
      
          // if the number buffer is not empty, we finished reading a number token
          // so create the token and add it to the vector.
          if (!number.empty ())
            {
              double value;
              auto [ptr, ec] =
                std::from_chars(number.data(), number.data() + number.size(), value);

              if (ec == std::errc())
                tokens.insert(tokens.begin() + idx_for_number, Token(value));
              else
                return "syntax error";

              number.clear ();
                
            }          
        }    

      return "success";
    }

  }
  namespace Parser
  {
    Glib::ustring
    parse (Expr *input_expr, MathExpr &ast)
    {
      std::vector<Tokenizer::Token> tokens;
      Tokenizer::tokenize (input_expr, tokens);

      std::string types[] = {

        "PLUS",
        "MINUS",
        "MUL",
        "DIV",
        "OPEN_PAREN",
        "CLOSE_PAREN",
        "NUMBER",
        "FRACTION"
      };
    
      for (size_t i=0;i<tokens.size();i++)
        {
          auto &tk = tokens[i];
          std::string type = types[static_cast<int>(tk.type)];
          if (tk.type == Tokenizer::Token::TokenType::NUMBER)
            std::cout << type << " " << tk.data.value << std::endl;
          else
            std::cout << type << std::endl;       
        }

      return "success";

    }
    

  }


}

#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "TokenType.hpp"
#include <any>
#include <iostream>
#include <string>
#include <vector>

// Token class represents a single unit of code (Ikalla unit).
// (Token class: Represents a lone wolf unit of code.)
// Jiddan 'var', '=', '10', ';' sab alag alag token ne.
// (Like 'var', '=', '10', ';', all separate entities partying together.)
class Token {
public:
  const TokenType type;
  const std::string lexeme;
  const std::any literal; // Kuch vi ho sakda (Number, String, etc)
  const int line;

  // Construtor: Nawa token janam lainda hai
  // (Constructor: A star is born!)
  Token(TokenType type, std::string lexeme, std::any literal, int line)
      : type(type), lexeme(lexeme), literal(literal), line(line) {}

  // Debugging laye string representation
  // (String representation for debugging)
  // Veere, eh function dasda ke token asal vich ki hai.
  // (Bro, this function reveals the true identity of the token.)
  std::string toString() const {
    std::string literalStr;
    if (type == TokenType::STRING) {
      // Check if it has a value
      if (literal.has_value()) {
        try {
          literalStr = std::any_cast<std::string>(literal);
        } catch (const std::bad_any_cast &e) {
          literalStr = "null";
        }
      } else {
        literalStr = "null";
      }
    } else if (type == TokenType::NUMBER) {
      if (literal.has_value()) {
        try {
          // Try double first as all Lox numbers are doubles
          double d = std::any_cast<double>(literal);
          // Check if it's actually an integer value (e.g. 12.0) for cleaner
          // printing if needed But typically returning the specific format
          // matters. Let's just print it. Codecrafters might expect specific
          // formatting. Let's use string if pass in constructor or just
          // to_string. For now let's just use what we have.
          literalStr = std::to_string(d);
          // Remove trailing zeros if it has a decimal point
          if (literalStr.find('.') != std::string::npos) {
            literalStr =
                literalStr.substr(0, literalStr.find_last_not_of('0') + 1);
            if (literalStr.back() == '.') {
              literalStr += "0";
            }
          }
        } catch (const std::bad_any_cast &e) {
          literalStr = "null";
        }
      } else {
        literalStr = "null";
      }
    } else {
      literalStr = "null";
    }

    // Return format similar to Java implementation for consistency: TYPE LEXEME
    // LITERAL Convert enum to string using our shiny new helper helper
    return typeToString(type) + " " + lexeme + " " + literalStr;
  }
};

#endif // TOKEN_HPP

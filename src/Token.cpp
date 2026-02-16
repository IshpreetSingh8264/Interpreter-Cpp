#include "Token.hpp"

#include <string>

namespace lox {

Token::Token(TokenType type, std::string lexeme, std::any literal, int line)
    : type(type), lexeme(lexeme), literal(literal), line(line) {}

// Saare Lox numbers are doubles, so a NUMBER token always holds a double. The
// any_cast is still guarded because a token is a plain value struct that
// anything can construct.
static std::string literalText(const Token &token) {
  if (!token.literal.has_value()) {
    return "null";
  }

  if (token.type == TokenType::STRING) {
    const std::string *text = std::any_cast<std::string>(&token.literal);
    return text != nullptr ? *text : "null";
  }

  if (token.type == TokenType::NUMBER) {
    const double *number = std::any_cast<double>(&token.literal);
    if (number == nullptr) {
      return "null";
    }
    // std::to_string always emits six decimal places; trim them back so
    // 12.5 prints as "12.5" and 42 as "42.0".
    std::string text = std::to_string(*number);
    if (text.find('.') != std::string::npos) {
      text = text.substr(0, text.find_last_not_of('0') + 1);
      if (text.back() == '.') {
        text += "0";
      }
    }
    return text;
  }

  return "null";
}

// Return format similar to Java implementation for consistency: TYPE LEXEME
// LITERAL
std::string Token::toString() const {
  return typeToString(type) + " " + lexeme + " " + literalText(*this);
}

} // namespace lox

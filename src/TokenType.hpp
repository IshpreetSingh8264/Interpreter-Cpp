#ifndef TOKENTYPE_HPP
#define TOKENTYPE_HPP

// Oye veere, eh saare tokens ne jo appa parse karne aa.
// (Hey bro, these are all the tokens we gotta parse.)
// Basically, language de building blocks aka 'ittein'.
// (Basically, the building blocks of the language, aka 'bricks'.)
enum class TokenType {
  // Single-character tokens (Ikalla banda)
  // (Single-player mode active)
  LEFT_PAREN,
  RIGHT_PAREN, // ( ) - Gol bracket
  LEFT_BRACE,
  RIGHT_BRACE, // { } - Curly bracket, stylish waali
  COMMA,
  DOT,
  MINUS,
  PLUS, // , . - + basic math stuff
  SEMICOLON,
  SLASH,
  STAR, // ; / * - Punctuation te division/multiplication

  // One or two character tokens (Do bande vi ho sakde ne)
  // (Might be a duo, like Jai and Veeru)
  BANG,
  BANG_EQUAL, // ! != - Nahi! te Bilkul nahi!
  EQUAL,
  EQUAL_EQUAL, // = == - Assignment te Comparison (Barabar hai ke nahi?)
  GREATER,
  GREATER_EQUAL, // > >= - Wadda te Wadda/Barabar
  LESS,
  LESS_EQUAL, // < <= - Chhota te Chhota/Barabar

  // Literals (Asli maal)
  // (The real deal, no fake stuff)
  IDENTIFIER,
  STRING,
  NUMBER, // Naa, Text, te Numbers

  // Keywords (Khaas alfaz/Reserved words)
  // (VIP words, don't touch these)
  AND,
  CLASS,
  ELSE,
  FALSE,
  FUN,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,

  // End of File (Khatam tata bye bye)
  // (Game over, man! Game over!)
  END_OF_FILE
};

#include <string>

// Helper to get string from TokenType
// Naam dasso!
// (State your name, soldier!)
inline std::string typeToString(TokenType type) {
  switch (type) {
  case TokenType::LEFT_PAREN:
    return "LEFT_PAREN";
  case TokenType::RIGHT_PAREN:
    return "RIGHT_PAREN";
  case TokenType::LEFT_BRACE:
    return "LEFT_BRACE";
  case TokenType::RIGHT_BRACE:
    return "RIGHT_BRACE";
  case TokenType::COMMA:
    return "COMMA";
  case TokenType::DOT:
    return "DOT";
  case TokenType::MINUS:
    return "MINUS";
  case TokenType::PLUS:
    return "PLUS";
  case TokenType::SEMICOLON:
    return "SEMICOLON";
  case TokenType::SLASH:
    return "SLASH";
  case TokenType::STAR:
    return "STAR";
  case TokenType::BANG:
    return "BANG";
  case TokenType::BANG_EQUAL:
    return "BANG_EQUAL";
  case TokenType::EQUAL:
    return "EQUAL";
  case TokenType::EQUAL_EQUAL:
    return "EQUAL_EQUAL";
  case TokenType::GREATER:
    return "GREATER";
  case TokenType::GREATER_EQUAL:
    return "GREATER_EQUAL";
  case TokenType::LESS:
    return "LESS";
  case TokenType::LESS_EQUAL:
    return "LESS_EQUAL";
  case TokenType::IDENTIFIER:
    return "IDENTIFIER";
  case TokenType::STRING:
    return "STRING";
  case TokenType::NUMBER:
    return "NUMBER";
  case TokenType::AND:
    return "AND";
  case TokenType::CLASS:
    return "CLASS";
  case TokenType::ELSE:
    return "ELSE";
  case TokenType::FALSE:
    return "FALSE";
  case TokenType::FUN:
    return "FUN";
  case TokenType::FOR:
    return "FOR";
  case TokenType::IF:
    return "IF";
  case TokenType::NIL:
    return "NIL";
  case TokenType::OR:
    return "OR";
  case TokenType::PRINT:
    return "PRINT";
  case TokenType::RETURN:
    return "RETURN";
  case TokenType::SUPER:
    return "SUPER";
  case TokenType::THIS:
    return "THIS";
  case TokenType::TRUE:
    return "TRUE";
  case TokenType::VAR:
    return "VAR";
  case TokenType::WHILE:
    return "WHILE";
  case TokenType::END_OF_FILE:
    return "EOF";
  default:
    return "UNKNOWN_TOKEN";
  }
}

#endif // TOKENTYPE_HPP

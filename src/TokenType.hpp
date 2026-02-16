#ifndef LOX_TOKENTYPE_HPP
#define LOX_TOKENTYPE_HPP

#include <string>

namespace lox {

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

// Implemented in TokenType.cpp.
std::string typeToString(TokenType type);

} // namespace lox

#endif // LOX_TOKENTYPE_HPP

#ifndef SCANNER_HPP
#define SCANNER_HPP

#include "Token.hpp"
#include "TokenType.hpp"
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

// Scanner class: Code nu chabb ke tokens banaun wala machine.
class Scanner {
public:
  // Source code jo appa parse karna
  const std::string source;
  // Tokens di list jo appa wapis karange
  std::vector<Token> tokens;

  // Tracking variables (Kitho tak pohanche?)
  int start = 0;         // Current token da start
  int current = 0;       // Current character pointer
  int line = 1;          // Kaunsi line te aa?
  bool hasError = false; // Koi panga peya? Error flag

  // Reserved words di mapping
  std::map<std::string, TokenType> keywords;

  // Custom constructor
  Scanner(std::string source) : source(source) {
    // Reserved keywords load kar lo
    // Ehna da matlab fix hai, change ni ho sakda.
    keywords["and"] = TokenType::AND;
    keywords["class"] = TokenType::CLASS;
    keywords["else"] = TokenType::ELSE;
    keywords["false"] = TokenType::FALSE;
    keywords["for"] = TokenType::FOR;
    keywords["fun"] = TokenType::FUN;
    keywords["if"] = TokenType::IF;
    keywords["nil"] = TokenType::NIL;
    keywords["or"] = TokenType::OR;
    keywords["print"] = TokenType::PRINT;
    keywords["return"] = TokenType::RETURN;
    keywords["super"] = TokenType::SUPER;
    keywords["this"] = TokenType::THIS;
    keywords["true"] = TokenType::TRUE;
    keywords["var"] = TokenType::VAR;
    keywords["while"] = TokenType::WHILE;
  }

  // Main function: Tokens scan karo!
  std::vector<Token> scanTokens();

private:
  // Helper helper bande (Internal functions)
  void scanToken();
  void addToken(TokenType type);
  void addToken(TokenType type, std::any literal);

  // Movement functions (Agge pichhe dekho)
  bool match(char expected);
  char peek();
  char peekNext();
  char advance();
  bool isAtEnd();

  // Specific scanners (Specific type da maal labho)
  void string();
  void number();
  void identifier();
};

#endif // SCANNER_HPP

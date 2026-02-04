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
// (Scanner class: The machine that chews up code and spits out tokens.)
class Scanner {
public:
  // Source code jo appa parse karna
  // (Source code we gotta parse)
  const std::string source;
  // Tokens di list jo appa wapis karange
  // (List of tokens we'll return, like a receipt)
  std::vector<Token> tokens;

  // Tracking variables (Kitho tak pohanche?)
  // (Tracking variables: How far have we reached?)
  int start = 0; // Current token da start
  // (Start of the current token)
  int current = 0; // Current character pointer
  int line = 1;    // Kaunsi line te aa?
  // (Which line are we on?)
  bool hasError = false; // Koi panga peya? Error flag
  // (Any trouble? Error flag)

  // Reserved words di mapping
  // (Mapping of reserved words)
  std::map<std::string, TokenType> keywords;

  // Custom constructor
  Scanner(std::string source) : source(source) {
    // Reserved keywords load kar lo
    // (Load up the reserved keywords)
    // Ehna da matlab fix hai, change ni ho sakda.
    // (Their meaning is fixed, can't change 'em.)
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
  // (Main function: Scan those tokens!)
  std::vector<Token> scanTokens();

private:
  // Helper helper bande (Internal functions)
  // (Helper helper guys: Internal functions)
  void scanToken();
  void addToken(TokenType type);
  void addToken(TokenType type, std::any literal);

  // Movement functions (Agge pichhe dekho)
  // (Movement functions: Look ahead, look behind)
  bool match(char expected);
  char peek();
  char peekNext();
  char advance();
  bool isAtEnd();

  // Specific scanners (Specific type da maal labho)
  // (Specific scanners: Find the specific type of goods)
  void string();
  void number();
  void identifier();
};

#endif // SCANNER_HPP

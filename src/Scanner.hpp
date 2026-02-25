#ifndef LOX_SCANNER_HPP
#define LOX_SCANNER_HPP

#include "Token.hpp"
#include "TokenType.hpp"
#include <any>
#include <map>
#include <string>
#include <vector>

#include "lox.hpp"

namespace lox {

// Scanner class: Code nu chabb ke tokens banaun wala machine.
// (Scanner class: The machine that chews up code and spits out tokens.)
class Scanner {
public:
  explicit Scanner(std::string source);

  // Main function: Tokens scan karo!
  // (Main function: Scan those tokens!)
  //
  // Lexical errors are reported on stderr as they are found and set hasError;
  // the token list is still returned so a caller can see how far the scan got.
  std::vector<Token> scanTokens();

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

  // Reserved words di mapping
  // (Mapping of reserved words)
  std::map<std::string, TokenType> keywords;
};

} // namespace lox

#endif // LOX_SCANNER_HPP

#ifndef LOX_TOKEN_HPP
#define LOX_TOKEN_HPP

#include "TokenType.hpp"
#include <any>
#include <string>

#include "lox.hpp"

namespace lox {

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
  Token(TokenType type, std::string lexeme, std::any literal, int line);

  // Debugging laye string representation
  // (String representation for debugging)
  // Veere, eh function dasda ke token asal vich ki hai.
  // (Bro, this function reveals the true identity of the token.)
  std::string toString() const;
};

} // namespace lox

#endif // LOX_TOKEN_HPP

#ifndef LOX_RUNTIMEERROR_HPP
#define LOX_RUNTIMEERROR_HPP

#include "Token.hpp"
#include <stdexcept>

#include "lox.hpp"


namespace lox {

// Runtime Error: Chladay hoye panga pe gya!
// (Runtime Error: Trouble while running! Panic mode on!)
class RuntimeError : public std::runtime_error {
public:
  const Token token;

  RuntimeError(Token token, const char *message)
      : std::runtime_error(message), token(token) {}

  RuntimeError(Token token, const std::string &message)
      : std::runtime_error(message), token(token) {}
};

} // namespace lox

#endif // LOX_RUNTIMEERROR_HPP

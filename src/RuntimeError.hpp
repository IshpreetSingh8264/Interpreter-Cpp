#ifndef LOX_RUNTIMEERROR_HPP
#define LOX_RUNTIMEERROR_HPP

#include "Token.hpp"
#include <stdexcept>
#include <string>

#include "lox.hpp"

namespace lox {

// Runtime Error: Chladay hoye panga pe gya!
// (Runtime Error: Trouble while running! Panic mode on!)
//
// Carries the token the error is attributed to, because the two tokens in a
// RuntimeError site (operator vs operand) point at different source lines and
// the harness checks the line number it prints.
class RuntimeError : public std::runtime_error {
public:
  const Token token;

  RuntimeError(Token token, const std::string &message);
};

} // namespace lox

#endif // LOX_RUNTIMEERROR_HPP

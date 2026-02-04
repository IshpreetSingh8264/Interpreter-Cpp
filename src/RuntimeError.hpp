#ifndef RUNTIMEERROR_HPP
#define RUNTIMEERROR_HPP

#include "Token.hpp"
#include <stdexcept>

// Runtime Error: Chladay hoye panga pe gya!
class RuntimeError : public std::runtime_error {
public:
  const Token token;

  RuntimeError(Token token, const char *message)
      : std::runtime_error(message), token(token) {}

  RuntimeError(Token token, const std::string &message)
      : std::runtime_error(message), token(token) {}
};

#endif // RUNTIMEERROR_HPP

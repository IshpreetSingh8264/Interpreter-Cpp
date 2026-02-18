#include "RuntimeError.hpp"

#include <utility>

namespace lox {

RuntimeError::RuntimeError(Token token, const std::string &message)
    : std::runtime_error(message), token(std::move(token)) {}

} // namespace lox

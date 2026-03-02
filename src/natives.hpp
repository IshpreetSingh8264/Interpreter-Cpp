#ifndef LOX_NATIVES_HPP
#define LOX_NATIVES_HPP

#include "Environment.hpp"

#include "lox.hpp"

namespace lox {

// Installs the built-in functions into the global environment.
//
// This is the whole native registry. Adding a native is one class implementing
// LoxCallable plus one line in this file; nothing in the parser, resolver or
// tree walker has to change.
void installNatives(Environment &globals);

} // namespace lox

#endif // LOX_NATIVES_HPP

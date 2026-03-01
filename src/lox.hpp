#ifndef LOX_HPP
#define LOX_HPP

// Every type, function and constant in this interpreter lives in `namespace lox`.
//
// This anchor header exists so any translation unit can name the namespace
// before it has included the module that first opens it. Headers that only
// forward-declare lox types (LoxCallable.hpp, for example) need exactly that.
//
// Do not put logic here. This file is a namespace anchor and nothing else.
//
// No .cpp: there is nothing to define. An empty .cpp would be a stub, and a
// stub is worse than a header with no TU when the header is a declaration.
namespace lox {}

#endif // LOX_HPP

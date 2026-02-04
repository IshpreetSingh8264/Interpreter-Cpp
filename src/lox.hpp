#ifndef LOX_HPP
#define LOX_HPP

// Every type, function and constant in this interpreter lives in `namespace lox`.
//
// This anchor header exists so any translation unit can name the namespace
// before it has included the module that first opens it. Headers that only
// forward-declare lox types (LoxCallable.hpp, for example) need exactly that.
//
// Do not put logic here. This file is a namespace anchor and nothing else.
namespace lox {}

#endif // LOX_HPP

# Lox Interpreter — C++

A tree-walking interpreter for Lox in C++23, built for the
[CodeCrafters "Build your own Interpreter" challenge](https://codecrafters.io/challenges/build-your-own-interpreter).

Scanner, recursive-descent parser, a static resolver that catches scope and class errors before execution, and a visitor
interpreter. Classes with inheritance and `super`, first-class functions, closures, and a proper error taxonomy with
distinct exit codes for compile-time and runtime failures.

> **Scope note.** Lox as the book defines it up to inheritance. There is no `break` or `continue`, no block comments, no
> string escapes, and no container types. See [Not implemented](#not-implemented).

## Contents

- [Quick start](#quick-start)
- [The language](#the-language)
- [Architecture](#architecture)
- [The resolver](#the-resolver)
- [Error handling](#error-handling)
- [Memory](#memory)
- [Tests](#tests)
- [Not implemented](#not-implemented)
- [Project layout](#project-layout)

## Quick start

Requires CMake 3.13+ and a C++23 compiler. No third-party dependencies.

```bash
cmake -B build -S .
cmake --build ./build
./build/interpreter run script.lox
```

```bash
$ cat test_classes.lox
class Circle {
  init(radius) {
    this.radius = radius;
  }

  area() {
    return 3.14159 * this.radius * this.radius;
  }
}

var circle = Circle(5);
print circle.area();
$ ./build/interpreter run test_classes.lox
78.53975
```

Four subcommands, matching what the course's harness invokes:

| Command | Runs | Exit on error |
|---|---|---|
| `tokenize <file>` | scanner only, prints one token per line | 65 |
| `parse <file>` | scanner + parser, prints the expression tree | 65 |
| `evaluate <file>` | scanner + parser + interpreter, **no resolver** | 70 |
| `run <file>` | the full pipeline, including the resolver | 65 / 70 |

`parse` and `evaluate` handle a single expression, not a program. Use `run` for anything with statements.

## The language

**Statements** — `print` `var` `if` / `else` `while` `for` `fun` `return` `class`, and expression statements.

**Expressions** — assignment, the ternary-free conditional via `or`/`and` short-circuit, equality `==` `!=`,
comparison `<` `<=` `>` `>=`, addition and subtraction, multiplication and division, unary `-` `!`, `and` `or`, `this`,
`super.method()`, calls, property get and set, and the primary forms: literals, identifiers, grouping, and `print`-style
output.

**Functions** — declarations, closures over their defining environment, parameters, and recursion.

**Classes** — `init` constructors, methods, field access on `this`, single inheritance with `super.method()` and
`super.init(...)`, and method lookup that walks the superclass chain.

**Natives** — exactly one: `clock()`, returning the current time as a number.

Eight example programs are in the repository root, each demonstrating one area: `test.lox`, `test_for.lox`,
`test_functions.lox`, `test_classes.lox`, `test_inheritance.lox`, `cls-inherit.lox`, `inherit.lox`, and `closures.lox`.
`inherit.lox` deliberately ends in a runtime error to show what that looks like.

## Architecture

```
source text
    │
    ▼
┌──────────────┐   tokens           ┌──────────────┐
│   Scanner    │───────────────────►│    Parser    │
│  Scanner.cpp │                    │  Parser.cpp  │  recursive descent,
│              │                    │              │  hand-written precedence
└──────────────┘                    └──────┬───────┘
                                           │ shared_ptr<Stmt> tree
                          ┌────────────────┴────────────────┐
                          ▼                                 ▼
                   ┌─────────────┐                   ┌─────────────┐
                   │   Resolver  │                   │ Interpreter │
                   │ Resolver.cpp│                   │             │
                   │             │                   │  walk + run │
                   │ static      │                   └──────┬──────┘
                   │ errors      │                          │
                   └─────────────┘                          ▼
                    exit 65                           ┌───────────┐
                                                        │ value.cpp │  truth, equality,
                                                        │ natives   │  stringification
                                                        └───────────┘
                                                           exit 70
```

`ExprVisitor` and `StmtVisitor` are the two interfaces that make the back half table-driven: `Resolver` and `Interpreter`
both implement both, and `AstPrinter` implements only the expression half.

`Interpreter.cpp` holds the constructor, `interpret`, `evaluate`, and `executeBlock`. The 12 expression bodies live in
`visit_expression.cpp` and the 9 statement bodies in `visit_statement.cpp` — a split that exists so the invariant "these
never change scope" is visible at the top of each file.

`value.cpp` is the only place that asks what a `std::any` actually holds: `isTruthy`, `isEqual`, `stringify`,
`requireNumber`, and the `as*` accessors.

## The resolver

The resolver walks the finished AST before execution and reports the errors that would otherwise only surface at runtime
in confusing ways. It catches **seven** distinct problems:

| Error | Where |
|---|---|
| `Already a variable with this name in this scope.` | `declare` |
| `Can't read local variable in its own initializer.` | `visitVariableExpr` |
| `Can't return from top-level code.` | `visitReturnStmt` |
| `Can't return a value from an initializer.` | `visitReturnStmt` |
| `A class can't inherit from itself.` | `visitClassStmt` |
| `Can't use 'this' outside of a class.` | `visitThisExpr` |
| `Can't use 'super' outside of a class.` / `... in a class with no superclass.` | `visitSuperExpr` |

It tracks a scope stack of name → "is this still being initialised", plus the current function type (`NONE`,
`FUNCTION`, `INITIALIZER`, `METHOD`) and class type (`NONE`, `CLASS`, `SUBCLASS`).

A name that is not found in any scope is **not** an error here. It resolves as a global, and reading it at runtime
produces `Undefined variable 'x'`. That is the book's behaviour, and it is why the resolver is a resolver rather than a
second parser.

The depth arithmetic is what makes `super` work. For `class B < A { m() { super.m(); } }` the resolver's scope stack at
that point is `[global, super, this, params]`, so `super` resolves at depth 2 and `this` at depth 1. The interpreter
reads them at exactly those depths.

## Error handling

Four kinds of failure, two error exit codes:

| Kind | Detected | Exit | Stream |
|---|---|---|---|
| lexical | during scanning | 65 | stderr, per occurrence, scanning continues |
| parse | during parsing | 65 | stderr, per occurrence, then resync at the next `;` or statement keyword |
| resolve | after the full AST walk | 65 | stderr, per occurrence, no resync |
| runtime | during execution | 70 | stderr, one message, execution aborts |

Runtime errors are `lox::RuntimeError`, which carries the `Token` so the line number survives to the message.
`ReturnException` unwinds through `executeBlock`; `LoxFunction::call` is the only place that catches it.

`executeBlock` catches `...` purely to restore `environment` and then rethrows, so a `return` deep in a loop cannot leak
a scope.

**One real gap:** `scanner.hasError` is only consulted on the `tokenize` and `parse` paths. On `evaluate` and `run` a
lexical error is printed but does not affect the exit code. In practice a lexical error nearly always also produces a
parse error, which is why this has not caused a visible failure — but it is there.

After a recovered parse error, `declaration()` returns `nullptr` and `parse()` pushes that into the statements vector.
Both the resolver and the interpreter null-check before using a node, so it is handled rather than crashed on.

## Memory

There is **no garbage collector**. No `weak_ptr`, no arena, no freelist, no object header.

- Values are `std::any` holding nil, `bool`, `double`, `std::string`, or a `shared_ptr` to a class, instance, or
  function.
- Scopes are chained through `Environment::enclosing`, a `shared_ptr`, which is exactly what makes closures work.
- Every AST node is a `shared_ptr` owned by the `vector` in `main`, alive for the whole run. `Interpreter::locals` is a
  `std::map<Expr*, int>` of **raw pointers** into that tree — the AST must outlive the map, and it does.
- `LoxClass` and `LoxInstance` both derive from `enable_shared_from_this` because `bind()` and `LoxClass::call()` need
  `shared_from_this()`.

**Known consequence:** reference cycles leak. Two are reachable from Lox source — a closure that captures the
environment holding it (`fun f() { fun g() { f(); } }`), and any subclass whose superclass closes over `super`. Nothing
breaks these cycles.

## Tests

```bash
./tests/run.sh                    # configure, build, both suites, size budget
./tests/run.sh unit
./tests/run.sh corpus
./tests/run.sh unit inh-super     # filter smoke cases by name
./tests/run.sh corpus IB9         # filter corpus cases by stage

cmake -B build -S . && cmake --build build && ctest --test-dir build --output-on-failure
```

**415 cases, and the suites are built very differently on purpose.**

**`tests/unit/smoke.py` — 85 hand-written cases.** Each is a small program with an expected stdout, exit code, and
optional stderr substring, run in a temp directory. A `bug=True` flag exists for known failures; it is currently used
zero times, because everything listed as a bug has since been fixed.

| Area | Cases |
|---|---|
| scanning | 7 |
| parsing | 24 |
| evaluation | 13 |
| running whole programs | 16 |
| functions and closures | 7 |
| classes and inheritance | 18 |

**`tests/corpus/` — 330 cases across 84 stages.** `corpus.json` is a snapshot of the real grader's own output, extracted
from an actual `codecrafters test` log by `extract_corpus.py`. Stages run from `IB9` down to `RY8`; 184 use `run`, 57
`tokenize`, 51 `evaluate`, 38 `parse`.

`run_corpus.py` asserts on **combined stdout+stderr as a multiset** plus the exact exit code. Per-stream equality is
computed but only reported, not enforced — 32 of the 330 cases put output on a different stream than the fixture
recorded, and the real grader merges them. Passing `--strict-streams` turns on per-stream checking, under which 36 cases
fail; that mode is a diagnostic, not a gate.

Two `AV4` cases match a `NONDETERMINISTIC` table instead, asserting only the *shape* of the output, because the fixture
froze `clock()` at a specific value.

`tests/check_size.py` fails if any test file passes 500 lines.

**Not covered:** output ordering, per-stream placement, the 255 parameter/argument limit, the unreachable-operator
errors, `stringify` of an unknown type, numeric edge cases, recursion depth, and the root `*.lox` demo files.

## Not implemented

- **No `break` or `continue`.** Absent from the token types, the grammar, and the AST.
- **No block comments.** `//` only.
- **No string escapes.** `"a\nb"` is a literal backslash-n, and a raw newline inside a string is accepted and bumps the
  line counter.
- **No numeric literals** beyond `digits[.digits]` — no exponents, hex, or `_` separators.
- **`+` never mixes a string and a number**, deliberately.
- **No arrays, dictionaries, sets, or any container type.** Lists are strings.
- **Only `clock()` as a native.**
- **No recursion or step limit**, so runaway Lox recursion will exhaust the C++ stack.
- **`1/0` yields `inf`**, because Lox numbers are `double`. There is no divide-by-zero error.
- **Three number formats coexist on purpose** — `tokenize` prints `42.0`, `parse` prints `42.0`, `print` and `evaluate`
  print `42`. Three separate formatters, deliberately not unified, because each matches what its own stage expects.
- **The `evaluate` subcommand skips the resolver**, so static errors are not reported there.

## Project layout

```
src/
  main.cpp                four subcommands, exit codes
  Scanner.cpp             characters → tokens
  Parser.cpp              tokens → AST
  Resolver.cpp            static scope and class errors
  Interpreter.cpp         construction, execute, executeBlock
  visit_expression.cpp    12 expression bodies
  visit_statement.cpp     9 statement bodies
  Environment.cpp         scope chain
  value.cpp               truth, equality, stringification
  natives.cpp             clock()
  AstPrinter.cpp          tree printing for `parse`
tests/
  unit/smoke.py           85 hand-written cases
  corpus/                 330 cases from the real grader, plus the extractor
  run.sh, check_size.py
*.lox                     eight example programs
```

Every declaration is in `namespace lox`; `main` is the single exception, and it opens the namespace with a `using`
directive rather than the namespace reaching out to it.

Two headers have no translation unit, both on purpose: `lox.hpp` is a namespace anchor with nothing to define, and
`LoxCallable.hpp` is a pure abstract interface. An empty `.cpp` would be a stub, and a stub is worse than a header that
is honestly declaration-only.

## Licence

No licence file is present in this repository. Add one before redistributing.

# Interpreter (Lox) — architecture

A tree-walking interpreter for Lox, the language from *Crafting Interpreters*.
C++23, CMake, no dependencies.

**All 84 CodeCrafters stages pass. That is the acceptance criterion for every
change in this repo — a refactor that makes the code cleaner but puts a stage at
risk has gone too far.** Build green and the stages green, at every commit.

---

## Project overview

`./build/interpreter <command> <file.lox>` where command is one of
`tokenize`, `parse`, `evaluate`, `run`. Each command drives a different prefix
of the pipeline; `run` drives all of it.

```
source text
    |
    v
Scanner      text        -> vector<Token>
    |
    v
Parser       tokens      -> shared_ptr<Stmt> tree
    |
    v
Resolver     tree        -> tree, annotated with scope depths on the Interpreter
    |
    v
Interpreter  tree        -> output
```

Four layers, each calling only the one below it. A layer never reaches back up:
the Scanner does not know what a Stmt is, the Resolver does not run anything,
and the Interpreter never sees source text or tokens.

---

## Module map

| File | Layer | Owns |
|---|---|---|
| `lox.hpp` | — | The `namespace lox` anchor. No logic, no TU. |
| `TokenType.hpp` `.cpp` | data | The token enum, and `typeToString`. |
| `Token.hpp` `.cpp` | data | One lexeme: type, text, literal, line. |
| `Expr.hpp` `.cpp` | data | The 12 expression node types + `ExprVisitor`. |
| `Stmt.hpp` `.cpp` | data | The 9 statement node types + `StmtVisitor`. |
| `ReturnException.hpp` `.cpp` | data | How `return` unwinds the stack. |
| `RuntimeError.hpp` `.cpp` | data | A runtime error plus the token to blame. |
| `value.hpp` `.cpp` | runtime | **What a Lox value is.** Truthiness, equality, printing, operand checks, type predicates. |
| `LoxCallable.hpp` | runtime | The three-method interface every callable implements. No TU. |
| `Environment.hpp` `.cpp` | runtime | A scope: a name→value map plus a parent pointer. |
| `LoxInstance.hpp` `.cpp` | runtime | Fields, resolved against the class's methods. |
| `LoxFunction.hpp` `.cpp` | runtime | A closure: declaration + captured environment. |
| `LoxClass.hpp` `.cpp` | runtime | Name, superclass, method table. |
| `natives.hpp` `.cpp` | runtime | `installNatives()`. Currently one: `clock`. |
| `Scanner.hpp` `.cpp` | lexer | Characters → tokens. Owns the keyword table. |
| `Parser.hpp` `.cpp` | parser | Tokens → AST. Owns the grammar. |
| `AstPrinter.hpp` `.cpp` | parser | AST → text, for the `parse` command. |
| `Resolver.hpp` `.cpp` | resolver | Scope depths + the compile-time error set. |
| `Interpreter.hpp` | runtime | The class contract: state + 21 visitor declarations. |
| `Interpreter.cpp` | runtime | Construction, `interpret`/`evaluate`/`execute`, `executeBlock`. |
| `visit_expression.cpp` | runtime | The 12 `ExprVisitor` bodies. |
| `visit_statement.cpp` | runtime | The 9 `StmtVisitor` bodies. |
| `main.cpp` | app | argv, unbuffered streams, dispatch. No domain logic. |

---

## Data flow

`run` on `class B < A { m() { super.m(); } }`:

1. `Scanner` emits `CLASS IDENTIFIER(<) IDENTIFIER(A) ... EOF`. Lexical errors go
   to stderr and set `hasError`; the token list is still returned.
2. `Parser` builds `Class{name: B, superclass: Variable(A), methods: [m]}` and
   `Function{name: m, params: [], body: [...]}`. A parse error prints on stderr,
   sets `hadError`, and unwinds to the nearest synchronize point in
   `declaration()` so one bad statement does not kill the file.
3. `Resolver` walks the tree. Inside `m`'s body it opens a scope for `this`, a
   scope for `super` (only because `B` has a superclass), and a scope for `m`'s
   parameters. `super` resolves to depth 2, `this` to depth 1. Each depth is
   written into `Interpreter::locals`, keyed by `Expr*`. It also reports the
   compile-time errors: `this` outside a class, `super` outside a subclass,
   `return` at top level, a local read in its own initializer, shadowing.
4. `Interpreter` executes. `visitClassStmt` evaluates `A`, defines `B` as nil,
   pushes a scope holding `super = A`, builds each `LoxFunction` closing over
   that scope, makes the `LoxClass`, pops the scope, and assigns `B`. A call to
   `B.m()` resolves to a `LoxFunction`, `arity()` is checked against the
   argument count, and `LoxFunction::call` makes a parameter scope over the
   closure and runs the body. `visitSuperExpr` reads `super` at distance 2 and
   `this` at distance 1, then binds the found method to the instance.

Everything crosses a layer boundary as a value: `vector<Token>` in, a tree out.
No layer calls back into the one above.

---

## Conventions

- **Everything is in `namespace lox`.** No exceptions. `main` is the one symbol at
  global scope and reaches into the namespace.
- **Headers declare, `.cpp` files define.** Two headers have no `.cpp` on
  purpose, and each says so in a comment: `lox.hpp` (a namespace anchor has
  nothing to define) and `LoxCallable.hpp` (a pure abstract interface).
- **Include guards** are `LOX_<NAME>_HPP`.
- **Bilingual comments.** Every explanatory comment is a Punjabi line followed by
  an English one in parentheses. Keep both; the Punjabi is the original voice and
  the English is what the next reader needs. Do not add single-language comments.
- **`-Wall -Wextra` is clean.** Check with
  `g++ -std=c++23 -Wall -Wextra -fsyntax-only -Isrc src/<file>.cpp`.
- **One concept per file, 600 lines max.** Currently 508 at the top
  (`Parser.cpp`).
- **CMake globs recursively**, so a new `.cpp` is picked up on the next
  configure. No build edit needed to add a file.

---

## Values

A Lox value is a `std::any` holding one of: nil (an empty `std::any`), `bool`,
`double`, `std::string`, or a `shared_ptr` to a `LoxClass`, `LoxInstance` or
`LoxFunction`. Natives are stored as a `shared_ptr<LoxCallable>`.

`value.hpp` is the only place that should ask what a value is. Use
`isNumber`/`isString`/`isBool` and `asNumber`/`asString`/`asBool` rather than
`type() == typeid(double)` and `any_cast` at the call site.

`value::stringify` and `AstPrinter`'s number formatting deliberately differ:
`print 57;` must emit `57`, while `parse` on `57` must print `57.0`. Do not
unify them.

---

## Integration points

- **CodeCrafters** compiles with `.codecrafters/compile.sh` and runs
  `.codecrafters/run.sh`, which execs `build/interpreter "$@"`. `your_program.sh`
  mirrors both for local use. Do not change the argv contract: the harness calls
  `<command> <file.lox>`.
- **Exit codes are part of the contract.** `0` success, `65` lexical/parse/
  resolve error, `70` runtime error, `1` usage error. The harness checks them.
- **Stream contract.** Program output and `print` go to stdout. Diagnostics go
  to stderr. The harness compares both separately, and it also compares the
  interleaving, so do not reorder or buffer.
- **Error message text is part of the contract.** The harness matches it
  literally, including the `[line N]` suffix. Change a message only if a stage
  tells you to.

---

## How to add X

### A new token type

1. Add the enumerator to `TokenType` in `TokenType.hpp`.
2. Add a `case` to `typeToString` in `TokenType.cpp`. The switch has no
   `default`, so the compiler will not let you forget.
3. Scan it in `Scanner::scanToken`.
4. If it can appear in source, add a grammar rule in `Parser.cpp` and a node
   type in `Expr.hpp`/`Stmt.hpp`.

### A new expression or statement node

1. Add the class to `Expr.hpp` / `Stmt.hpp`, deriving from `Expr`/`Stmt`, and
   add `visit<Name>Expr` to the visitor interface.
2. Add the constructor and `accept` to `Expr.cpp` / `Stmt.cpp`.
3. Implement it in `visit_expression.cpp` / `visit_statement.cpp`.
4. Add the `AstPrinter` rendering in `AstPrinter.cpp`. Steps 1 and 4 are both
   compiler-enforced — the visitor interfaces are pure virtual, so a missing
   `visit*` or a missing rendering is a build error, not a runtime "unknown".

### A new native function

1. Write a class deriving from `LoxCallable` in `natives.cpp` (implement
   `arity`, `call`, `toString`).
2. Add one `globals.define(...)` line to `installNatives`.

Nothing in the parser, resolver or tree walker changes.

### A new error

- **Compile-time** (lexical, syntax, scope): raise it in the layer that owns the
  knowledge — `Scanner` sets `hasError`, `Parser` uses `error()`, `Resolver`
  prints to stderr and sets `hadError`. These end at exit 65.
- **Runtime** (wrong type, undefined name, bad call): `throw RuntimeError(token,
  "...")`. Always pass the token the message should point at; the reported line
  is that token's line. This ends at exit 70.
- **The unreachable branch**: throw. Do not return nil or a placeholder string.
  An empty `std::any` reaching `print` comes out as `nil`, which looks like a
  valid program result and hides the bug.

---

## Do not edit these

- **`.codecrafters/compile.sh` and `.codecrafters/run.sh`.** The remote harness
  runs them verbatim.
- **Exit codes and stream routing in `main.cpp`.** The harness checks both.
- **Error message wording.** Matched literally by the harness.
- **`CMakeLists.txt`'s `GLOB_RECURSE`.** It is what makes a new `.cpp` need no
  build edit.

---

## Not implemented, on purpose

`break` and `continue` are not in the grammar and are not on the CodeCrafters
course. Do not add them. The book's "Dart" chapter features are likewise out of
scope.

---

## Verifying a change

```sh
# the local gate: build + 415 cases (85 hand-written + 330 from the tester).
# Runs in seconds, no network.
./tests/run.sh

# narrow it
./tests/run.sh unit inh-super      # one area of the smoke suite
./tests/run.sh corpus IB9          # one CodeCrafters stage
ctest --test-dir build --output-on-failure   # if you prefer ctest

# the real gate
./your_program.sh run script.lox
codecrafters test                 # must report all 84 stages passing
```

`codecrafters test` is the only authority. It takes a couple of minutes because
it rebuilds remotely. `tests/run.sh` is a fast local proxy derived from a
snapshot of that same tester — it exists to catch a regression in one second
instead of in three, not to replace the gate. **Read `tests/README.md` before
you trust a green run**: it lists exactly what these suites do not cover.

# 🔄 Code Flow Documentation

This document explains how code flows through the Lox interpreter from source to output.

Every declaration in the project lives in `namespace lox`. `main` is the one
exception, because it has to be at global scope.

---

## 📍 Entry Point

### main.cpp

`main` does three things: unbuffer the streams, read `argv[1]`, dispatch. It holds
no domain logic — the AST printer that used to live here is now `AstPrinter.cpp`
and the value formatting is in `value.cpp`.

```mermaid
flowchart TD
    A[main.cpp] --> B{command}
    B -->|tokenize| C[Scanner only]
    B -->|parse| D[Scanner + Parser + AstPrinter]
    B -->|evaluate| E[Scanner + Parser + Interpreter<br/>NO Resolver]
    B -->|run| F[Full Pipeline<br/>Scanner -> Parser -> Resolver -> Interpreter]
```

`evaluate` skips the Resolver, which matters for error handling: the Resolver's
compile-time rules (`this` outside a class, a value returned from an
initializer) are not enforced on that path, so the runtime checks in
`visitThisExpr`, `visitSuperExpr` and `LoxFunction::call` have to stand on their
own. They do.

---

## 🔍 Complete Execution Flow

### For a simple program:

```lox
var x = 10;
print x + 5;
```

```mermaid
sequenceDiagram
    participant Main
    participant Scanner
    participant Parser
    participant Resolver
    participant Interpreter
    participant Environment
  
    Note over Main: Read file "script.lox"
  
    Main->>Scanner: scanTokens(source)
    Note over Scanner: Tokenize source code
    Scanner-->>Main: tokens[]
  
    Main->>Parser: parse(tokens)
    Note over Parser: Build AST using<br>recursive descent
    Parser-->>Main: statements[]
  
    Main->>Resolver: resolve(statements)
    Note over Resolver: Bind variables to<br>scope depths
    Resolver-->>Main: locals{} updated
  
    Main->>Interpreter: interpret(statements)
  
    loop For each statement
        Interpreter->>Interpreter: execute(stmt)
      
        alt VarStmt
            Interpreter->>Environment: define("x", 10)
        else PrintStmt
            Interpreter->>Interpreter: evaluate(x + 5)
            Interpreter->>Environment: get("x") → 10
            Interpreter-->>Main: print "15"
        end
    end
```

---

## 📝 Phase 1: Scanning (Lexical Analysis)

**Files**: `Scanner.hpp`, `Scanner.cpp`, `Token.hpp`, `Token.cpp`, `TokenType.hpp`, `TokenType.cpp`

### Input → Output

```
Input:  "var x = 10;"
Output: [VAR] [IDENTIFIER:x] [EQUAL] [NUMBER:10] [SEMICOLON] [EOF]
```

### Flow

```mermaid
flowchart TB
    A[Source String] --> B[scanTokens]
    B --> C{Not at end?}
    C -->|Yes| D[scanToken]
    D --> E[Identify character]
    E --> F{Type?}
    F -->|Single char| G[Add single-char token]
    F -->|Two char| H[Match and add]
    F -->|String| I[Consume until closing quote]
    F -->|Number| J[Consume digits and dot]
    F -->|Identifier| K[Consume alphanumeric]
    K --> L{Is keyword?}
    L -->|Yes| M[Use keyword type]
    L -->|No| N[Use IDENTIFIER]
    G & H & I & J & M & N --> C
    C -->|No| O[Add EOF token]
    O --> P[Return tokens]
```

---

## 📝 Phase 2: Parsing (Syntax Analysis)

**Files**: `Parser.hpp`, `Parser.cpp`, `Expr.hpp`, `Expr.cpp`, `Stmt.hpp`, `Stmt.cpp`, `AstPrinter.hpp`, `AstPrinter.cpp`

`Parser::kMaxArity` (255) caps both parameter and argument lists. Both checks used
to be commented-out `error()` calls, so the limit was tested and then discarded.

### Grammar Hierarchy

```mermaid
flowchart TB
    subgraph "Expression Precedence (Low to High)"
        A[expression] --> B[assignment]
        B --> C[or]
        C --> D[and]
        D --> E[equality]
        E --> F[comparison]
        F --> G[term]
        G --> H[factor]
        H --> I[unary]
        I --> J[call]
        J --> K[primary]
    end
```

### Parsing Example: `x = 1 + 2 * 3`

```mermaid
flowchart TB
    A[AssignExpr] --> B[name: x]
    A --> C[value: BinaryExpr]
    C --> D[left: 1]
    C --> E[op: +]
    C --> F[right: BinaryExpr]
    F --> G[left: 2]
    F --> H[op: *]
    F --> I[right: 3]
```

---

## 📝 Phase 3: Resolving (Static Analysis)

**Files**: `Resolver.hpp`, `Resolver.cpp`

### Purpose

Resolves variable references to their declaration scope **before** interpretation.

### How Scopes Work

```mermaid
flowchart TB
    subgraph "Scope Stack"
        A["Scope 0 (Global)<br>{clock: true}"]
        B["Scope 1 (Function)<br>{x: true, y: true}"]
        C["Scope 2 (Block)<br>{z: true}"]
    end
  
    A --- B
    B --- C
  
    D[Variable z] --> E[Distance: 0]
    F[Variable x] --> G[Distance: 1]
    H[Variable clock] --> I[Distance: 2]
```

### Resolution Process

```mermaid
flowchart LR
    A[Visit Variable] --> B[Search Scopes]
    B --> C[Calculate Distance]
    C --> D[Store in locals map]
    D --> E[Interpreter uses distance<br>for fast lookup]
```

---

## 📝 Phase 4: Interpretation (Execution)

**Files**: `Interpreter.hpp` (contract), `Interpreter.cpp` (core),
`visit_expression.cpp`, `visit_statement.cpp`, `value.cpp`, `natives.cpp`

### Visitor Pattern Execution

```mermaid
flowchart TB
    A[interpret] --> B[execute each statement]
    B --> C[stmt.accept this]
    C --> D{Statement Type}
    D -->|Print| E[visitPrintStmt]
    D -->|Var| F[visitVarStmt]
    D -->|If| G[visitIfStmt]
    D -->|While| H[visitWhileStmt]
    D -->|Fun| I[visitFunctionStmt]
    D -->|Class| J[visitClassStmt]
    D -->|Return| K[visitReturnStmt]
  
    E --> L[evaluate expression]
    L --> M["value::stringify(result)"]

    subgraph "where each piece lives"
        N["Interpreter.cpp<br/>interpret / evaluate / execute<br/>executeBlock"]
        VE["visit_expression.cpp<br/>12 Expr bodies"]
        VS["visit_statement.cpp<br/>9 Stmt bodies"]
        V["value.cpp<br/>truthiness, equality, printing"]
    end

    N --> VE
    N --> VS
    VE --> V
    VS --> V
```

Expression bodies never change the current scope. Statement bodies do, and they
all do it through `executeBlock`, which saves `environment`, swaps in the new
one, runs, and restores it even when a statement unwinds. That restore-on-throw
is what lets `return` escape from inside a nested block.

---

## 🔧 Environment Chain

### How Variables are Stored

```mermaid
flowchart LR
    subgraph "Global Env"
        A[clock: NativeFunction]
        B[myFunc: LoxFunction]
    end
  
    subgraph "Function Env"
        C[param1: value]
        D[localVar: value]
    end
  
    subgraph "Block Env"
        E[blockVar: value]
    end
  
    Global --> Function --> Block
```

### Variable Lookup with Resolution

```cpp
// Without resolution (slow - walks chain)
environment->get(token);

// With resolution (fast - jumps directly)
auto it = locals.find(&expr);
if (it != locals.end()) {
    return environment->getAt(it->second, name);
}
```

`locals` is filled in by the Resolver before anything runs, keyed by `Expr*`.
A node absent from the map is a global. Every `locals.find` result is checked
against `end()` — `visitSuperExpr` used to skip that check and segfault when the
`super` expression had not been annotated.

For the interpreter's own slots the read is `getSlotOrFail`, which throws a
located error on a miss instead of letting `map::operator[]` insert an empty
`any`:

```cpp
// `this` and `super` are created by the interpreter, not the program, so a
// miss means it lost track of the scope chain.
return environment->getSlotOrFail(distance, expr.keyword, "super");
```

---

## 🎯 Function Call Flow

### For: `add(1, 2)`

```mermaid
sequenceDiagram
    participant I as Interpreter
    participant E as Environment
    participant F as LoxFunction
  
    I->>I: visitCallExpr
    I->>I: evaluate(callee) → LoxFunction
    I->>I: evaluate(arg1) → 1
    I->>I: evaluate(arg2) → 2
    I->>F: call(interpreter, [1, 2])
    F->>E: Create new Environment(closure)
    F->>E: define("a", 1)
    F->>E: define("b", 2)
    F->>I: executeBlock(body, newEnv)
    I-->>F: return value
    F-->>I: return result
```

---

## 🎓 Class Instantiation Flow

### For: `var dog = Dog();`

```mermaid
sequenceDiagram
    participant I as Interpreter
    participant C as LoxClass
    participant Inst as LoxInstance
    participant F as LoxFunction(init)
  
    I->>I: visitCallExpr
    I->>I: evaluate(callee) → LoxClass
    I->>C: call(interpreter, [])
    C->>Inst: Create new LoxInstance(this)
    C->>C: findMethod("init")
    alt Has init method
        C->>F: bind(instance)
        F-->>C: BoundMethod
        C->>C: BoundMethod.call([args])
    end
    C-->>I: return instance
    I->>I: environment.define("dog", instance)
```

---

## 🔗 Inheritance and Super

### super.method() Execution

```mermaid
sequenceDiagram
    participant I as Interpreter
    participant E as Environment
    participant Super as LoxClass(Superclass)
    participant F as LoxFunction
  
    I->>I: visitSuperExpr
  
    Note over I: Get distance from<br>resolver's locals
    I->>E: getAt(distance, "super")
    E-->>I: superclass
  
    Note over I: Get this from one<br>level above
    I->>E: getAt(distance-1, "this")
    E-->>I: instance
  
    I->>Super: findMethod("methodName")
    Super-->>I: method
  
    I->>F: method.bind(instance)
    F-->>I: boundMethod
```

---

## 📊 Complete Data Flow Diagram

```mermaid
flowchart TB
    subgraph Input
        A[script.lox]
    end
  
    subgraph "Phase 1: Scanning"
        B[Source String]
        C[Token Vector]
        B -->|scanTokens| C
    end
  
    subgraph "Phase 2: Parsing"
        D[Token Vector]
        E[AST: Vector of Stmt]
        D -->|parse| E
    end
  
    subgraph "Phase 3: Resolving"
        F[AST]
        G[locals: map Expr->int]
        F -->|resolve| G
    end
  
    subgraph "Phase 4: Execution"
        H[AST + locals]
        I[Evaluate / Execute]
        J[Environment Chain]
        H --> I
        I <--> J
    end
  
    subgraph Output
        K[stdout]
    end
  
    A --> B
    C --> D
    E --> F
    E --> H
    G --> H
    I --> K


```

---

## 🔚 Error Handling Flow

Three kinds of error, three exit codes. The CodeCrafters harness checks the code
and matches the message text literally, so both are part of the contract.

```mermaid
flowchart TB
    A[Error Detected] --> B{Error Type}
    B -->|Scan Error| C[Set hasError flag]
    B -->|Parse Error| D[Throw ParseError]
    B -->|Resolve Error| R[Set hadError flag]
    B -->|Runtime Error| E[Throw RuntimeError]
    B -->|Unreachable| U[Throw - never return nil]
  
    C --> F[Continue scanning<br>Report at end]
    D --> G[synchronize<br>Skip to next statement]
    R --> S[Keep walking<br>Report at end]
    E --> H[Catch in main<br>Print message + line<br>Exit 70]
    U --> H

    C --> X[main returns 65]
    D --> X
    R --> X
```

**Streams.** Diagnostics go to stderr; program output and `print` go to stdout.
The harness compares them separately *and* their interleaving, so a diagnostic
printed to stdout, or a `print` sent to stderr, fails a stage.

**Two rules for new errors.**

1. Pass the token the message should point at. `RuntimeError` prints `[line N]`
   from that token and the harness checks the number. For a binary operator that
   is the operator token, not an operand.
2. The unreachable branch throws. Returning an empty `std::any` from
   `visitUnaryExpr` or `visitBinaryExpr`, or the string `"unknown"` from
   `stringify`, produces output that looks like a valid program result and hides
   the bug underneath. Both now throw.

**Scope-lookup safety.** `Environment` has two lookup paths. `getAt`/`assignAt`
are unchecked because the Resolver proved the slot exists and they run on every
variable read. `getSlotOrFail` is checked and used for `this` and `super`, where
a miss means the interpreter lost track of the scope chain. It walks the chain
itself rather than through `ancestor()`, which assumes the distance is in range
and dereferences a null `enclosing` past the end.

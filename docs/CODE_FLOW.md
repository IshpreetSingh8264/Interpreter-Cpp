# 🔄 Code Flow Documentation

This document explains how code flows through the Lox interpreter from source to output.

---

## 📍 Entry Point

### main.cpp

The entry point dispatches to different modes:

```mermaid
flowchart TD
    A[main.cpp] --> B{command}
    B -->|tokenize| C[Scanner only]
    B -->|parse| D[Scanner + Parser]
    B -->|evaluate| E[Scanner + Parser + Interpreter - Single expression]
    B -->|run| F[Full Pipeline - Scanner -> Parser -> Resolver -> Interpreter]
```

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

**File**: `Scanner.hpp`, `Scanner.cpp`

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

**Files**: `Parser.hpp`, `Parser.cpp`, `Expr.hpp`, `Stmt.hpp`

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

**Files**: `Interpreter.hpp`, `Interpreter.cpp`

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
    L --> M[print stringify result]
```

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

```mermaid
flowchart TB
    A[Error Detected] --> B{Error Type}
    B -->|Scan Error| C[Set hasError flag]
    B -->|Parse Error| D[Throw ParseError]
    B -->|Runtime Error| E[Throw RuntimeError]
  
    C --> F[Continue scanning<br>Report at end]
    D --> G[synchronize<br>Skip to next statement]
    E --> H[Catch in main<br>Print error<br>Exit with code 70]
```

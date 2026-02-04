# 🏗️ Architecture Documentation

This document provides a detailed overview of the Lox interpreter architecture.

---

## 📊 System Architecture

```mermaid
graph TB
    subgraph "Frontend (Source → AST)"
        direction TB
        A[Source Code] --> B[Scanner/Lexer]
        B --> C[Token Stream]
        C --> D[Parser]
        D --> E[AST]
    end
    
    subgraph "Middle-End (Static Analysis)"
        direction TB
        E --> F[Resolver]
        F --> G[Scope Resolution Data]
    end
    
    subgraph "Backend (Execution)"
        direction TB
        G --> H[Interpreter]
        H --> I[Output]
    end
    
    subgraph "Runtime Support"
        direction LR
        J[Environment]
        K[LoxFunction]
        L[LoxClass]
        M[LoxInstance]
    end
    
    H <--> J
    H <--> K
    H <--> L
    H <--> M
```

---

## 📦 Component Details

### 1. Scanner (Lexer)

**Purpose**: Converts raw source code into a sequence of tokens.

**Files**: `Scanner.hpp`, `Scanner.cpp`

```mermaid
flowchart LR
    A["print \"hello\";"] --> B[Scanner]
    B --> C["[PRINT] [STRING:hello] [SEMICOLON] [EOF]"]
```

**Key Methods**:
- `scanTokens()` - Main entry, returns `vector<Token>`
- `scanToken()` - Scans a single token
- `string_()` - Handles string literals
- `number()` - Handles numeric literals
- `identifier()` - Handles identifiers and keywords

---

### 2. Parser

**Purpose**: Builds an Abstract Syntax Tree (AST) from tokens using recursive descent.

**Files**: `Parser.hpp`, `Parser.cpp`, `Expr.hpp`, `Stmt.hpp`

```mermaid
flowchart TB
    subgraph "Grammar Rules"
        A[program] --> B[declaration*]
        B --> C[varDecl / funDecl / classDecl / statement]
        C --> D[expression]
        D --> E[equality → comparison → term → factor → unary → primary]
    end
```

**Key Classes**:

| Class | Description |
|-------|-------------|
| `Expr` | Base class for all expressions |
| `Binary` | `a + b`, `a == b` |
| `Unary` | `-a`, `!a` |
| `Literal` | `42`, `"hello"`, `true` |
| `Variable` | Variable references |
| `Assign` | `a = 5` |
| `Call` | `foo(a, b)` |
| `Get` | `obj.property` |
| `Set` | `obj.property = value` |
| `This` | `this` keyword |
| `Super` | `super.method` |

---

### 3. Resolver

**Purpose**: Static analysis pass that resolves variable scopes before interpretation.

**Files**: `Resolver.hpp`, `Resolver.cpp`

```mermaid
flowchart LR
    A[AST] --> B[Resolver]
    B --> C[Scope Stack]
    C --> D[Variable → Depth Map]
    D --> E[Interpreter.locals]
```

**How it works**:
1. Walks the AST before interpretation
2. Maintains a stack of scopes
3. For each variable, records how many scopes away it was declared
4. Stores this information in `Interpreter.locals`

---

### 4. Interpreter

**Purpose**: Executes the AST using the visitor pattern.

**Files**: `Interpreter.hpp`, `Interpreter.cpp`

```mermaid
flowchart TB
    subgraph "Visitor Pattern"
        A[Interpreter] --> B[visitBinaryExpr]
        A --> C[visitPrintStmt]
        A --> D[visitVarStmt]
        A --> E[visitFunctionStmt]
        A --> F[visitClassStmt]
    end
    
    subgraph "Runtime"
        G[Environment]
        H[globals]
    end
    
    B & C & D & E & F --> G
```

---

### 5. Environment

**Purpose**: Stores variables in a scoped chain.

**Files**: `Environment.hpp`, `Environment.cpp`

```mermaid
graph TB
    A[Global Scope] --> B[Block Scope 1]
    B --> C[Function Scope]
    C --> D[Block Scope 2]
    
    A --> |"clock = native"| A1[values map]
    C --> |"x = 5"| C1[values map]
```

**Key Features**:
- `enclosing` pointer forms a linked list of scopes
- `getAt(distance, name)` for resolved variable lookup
- `ancestor(distance)` for scope hopping

---

### 6. LoxCallable Hierarchy

```mermaid
classDiagram
    class LoxCallable {
        <<interface>>
        +int arity()
        +any call(Interpreter, vector~any~)
        +string toString()
    }
    
    class NativeClock {
        +int arity() = 0
        +any call() = current_time
    }
    
    class LoxFunction {
        -Function declaration
        -Environment closure
        +LoxFunction bind(LoxInstance)
    }
    
    class LoxClass {
        +string name
        +LoxClass superclass
        +map methods
        +LoxFunction findMethod(string)
    }
    
    LoxCallable <|-- NativeClock
    LoxCallable <|-- LoxFunction
    LoxCallable <|-- LoxClass
```

---

### 7. Inheritance Model

```mermaid
flowchart TB
    subgraph "Method Resolution"
        A[instance.method] --> B{Found in class?}
        B -->|Yes| C[Return method]
        B -->|No| D{Has superclass?}
        D -->|Yes| E[Search superclass]
        D -->|No| F[Error: Undefined]
        E --> B
    end
```

**super Keyword Execution**:
1. Get `super` from environment at resolved distance
2. Get `this` from one level above `super`
3. Find method in superclass
4. Bind method to current instance

---

## 🔗 Inter-File Dependencies

```mermaid
graph TD
    main --> Scanner
    main --> Parser
    main --> Resolver
    main --> Interpreter
    
    Parser --> Expr
    Parser --> Stmt
    Parser --> Token
    
    Resolver --> Interpreter
    Resolver --> Stmt
    Resolver --> Expr
    
    Interpreter --> Environment
    Interpreter --> LoxFunction
    Interpreter --> LoxClass
    Interpreter --> LoxInstance
    
    LoxFunction --> Environment
    LoxClass --> LoxFunction
    LoxInstance --> LoxClass
```

---

## 📝 Memory Management

The interpreter uses `std::shared_ptr` extensively:
- **Environment chains** - Allows closures to capture enclosing scopes
- **AST nodes** - Shared ownership during parsing and interpretation
- **LoxClass/LoxInstance** - Instances reference their class

---

## 🎭 Visitor Pattern

Both `Expr` and `Stmt` use the visitor pattern:

```cpp
// Base class defines accept
class Expr {
    virtual std::any accept(ExprVisitor& visitor) = 0;
};

// Each subclass implements accept
class Binary : public Expr {
    std::any accept(ExprVisitor& visitor) override {
        return visitor.visitBinaryExpr(*this);
    }
};

// Interpreter implements visitor
class Interpreter : public ExprVisitor, public StmtVisitor {
    std::any visitBinaryExpr(Binary& expr) override {
        // Evaluate binary expression
    }
};
```

---

## 🔄 Execution Flow Example

For the code: `print 1 + 2;`

```mermaid
sequenceDiagram
    participant M as main()
    participant S as Scanner
    participant P as Parser
    participant I as Interpreter
    
    M->>S: scanTokens("print 1 + 2;")
    S-->>M: [PRINT, 1, PLUS, 2, SEMICOLON, EOF]
    
    M->>P: parse(tokens)
    P-->>M: PrintStmt(Binary(1, +, 2))
    
    M->>I: interpret(statements)
    I->>I: visitPrintStmt()
    I->>I: evaluate(Binary)
    I->>I: visitBinaryExpr()
    I->>I: evaluate(1) = 1.0
    I->>I: evaluate(2) = 2.0
    I->>I: 1.0 + 2.0 = 3.0
    I-->>M: print "3"
```

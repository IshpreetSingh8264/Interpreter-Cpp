#!/usr/bin/env python3
"""Fast local smoke suite for the Lox interpreter.

Each case is (name, lox-source, command, expected-stdout-lines, expected-exit,
expected-stderr-substring-or-None).  Run against ./build/interpreter.

Usage: smoke.py <binary> [name-substring ...]

Run the whole thing with ../run.sh, or:
    python3 tests/unit/smoke.py build/interpreter
    python3 tests/unit/smoke.py build/interpreter inh-super   # filter by name

If you find a genuine known defect, pass bug=True to case(). Such a case is
reported separately and never fails the run; if it starts passing it is
reported as FIXED, which is the signal to drop the flag and make it a hard
assertion.
"""
import os
import subprocess
import sys
import tempfile

CASES = []


def case(name, src, cmd, out=(), exit=0, err_has=None, bug=False):
    """bug=True: documents a known defect. Reported separately, never a failure."""
    CASES.append(
        {
            "name": name,
            "src": src,
            "cmd": cmd,
            "out": list(out),
            "exit": exit,
            "err_has": err_has,
            "bug": bug,
        }
    )


# ---------------------------------------------------------------- scanning
case("scan-empty", "", "tokenize", ["EOF  null"])
case("scan-ops", "(){}.,-;/*", "tokenize", [
    "LEFT_PAREN ( null", "RIGHT_PAREN ) null", "LEFT_BRACE { null",
    "RIGHT_BRACE } null", "DOT . null", "COMMA , null", "MINUS - null",
    "SEMICOLON ; null", "SLASH / null", "STAR * null", "EOF  null",
])
case("scan-eq", "! != = == > >= < <=", "tokenize", [
    "BANG ! null", "BANG_EQUAL != null", "EQUAL = null", "EQUAL_EQUAL == null",
    "GREATER > null", "GREATER_EQUAL >= null", "LESS < null",
    "LESS_EQUAL <= null", "EOF  null",
])
case("scan-lits", '12.5 "hi" 42', "tokenize", [
    "NUMBER 12.5 12.5", 'STRING "hi" hi', "NUMBER 42 42.0", "EOF  null",
])
case("scan-keywords", "and class else false fun for if nil or print return super this true var while",
     "tokenize", [
         "AND and null", "CLASS class null", "ELSE else null", "FALSE false null",
         "FUN fun null", "FOR for null", "IF if null", "NIL nil null",
         "OR or null", "PRINT print null", "RETURN return null",
         "SUPER super null", "THIS this null", "TRUE true null",
         "VAR var null", "WHILE while null", "EOF  null",
     ])
case("scan-multiline-comment", "() #comment\n@", "tokenize",
     ["LEFT_PAREN ( null", "RIGHT_PAREN ) null", "IDENTIFIER comment null",
      "EOF  null"], 65, err_has="Unexpected character: #")
case("scan-unterminated-string", '"abc', "tokenize", ["EOF  null"], 65,
     err_has="Unterminated string.")

# ---------------------------------------------------------------- parsing
case("parse-arith", "-123", "parse", ["(- 123.0)"])
case("parse-nested", "-(123 + (456 * 789))", "parse",
     ["(- (group (+ 123.0 (group (* 456.0 789.0)))))"])
case("parse-str", '"hello world"', "parse", ["hello world"])
case("parse-bool", "true", "parse", ["true"])
case("parse-nil", "nil", "parse", ["nil"])
case("parse-eq", "1 == 2", "parse", ["(== 1.0 2.0)"])
case("parse-lt", "1 < 2 < 3", "parse", ["(< (< 1.0 2.0) 3.0)"])
case("parse-var", "y", "parse", ["y"])
case("parse-and-or", "a and b or c", "parse", ["(or (and a b) c)"])
case("parse-call", "max(1, 2, 3)", "parse", ["(max 1.0 2.0 3.0)"])
case("parse-get", "person.name", "parse", ["(. person name)"])
case("parse-assign", "x = 1", "parse", ["(x = 1.0)"])
case("parse-this", "this", "parse", ["this"])
case("parse-set", "a.b = 1", "parse", ["(. a b = 1.0)"])
case("parse-lit-nil", "nil", "parse", ["nil"])
case("parse-lit-num-int", "57", "parse", ["57.0"])
case("parse-lit-num-frac", "11.59", "parse", ["11.59"])
case("parse-lit-str", '"hi"', "parse", ["hi"])
case("parse-nested-call", "a.b(1)(2)", "parse", ["(((. a b) 1.0) 2.0)"])
case("parse-chained-get-set", "a.b.c = d.e", "parse", ["(. (. a b) c = (. d e))"])
case("parse-super", "super.method()", "parse", ["((super method))"])
case("parse-get-chain", "a.b.c", "parse", ["(. (. a b) c)"])
case("parse-syntax-error", "1 + + 2", "parse", [], 65,
     err_has="Expect expression.")
case("parse-bad-assign", "1 = 2", "parse", [], 65,
     err_has="Invalid assignment target.")

# -------------------------------------------------------------- evaluation
case("eval-arith", "1 + 2 * 3", "evaluate", ["7"])
case("eval-str-concat", '"a" + "b"', "evaluate", ["ab"])
case("eval-str-num", '"a" + 1', "evaluate", [], 70,
     err_has="Operands must be two numbers or two strings.")
case("eval-unary", "-12", "evaluate", ["-12"])
case("eval-unary-bad", '-"hello"', "evaluate", [], 70,
     err_has="Operand must be a number.")
case("eval-unary-not-any", '!"hello"', "evaluate", ["false"])
case("eval-unary-not-nil", "!!nil", "evaluate", ["false"])
case("eval-binary-bad", '"x" - 1', "evaluate", [], 70,
     err_has="Operands must be numbers.")
case("eval-bool", "true", "evaluate", ["true"])
case("eval-nil", "nil", "evaluate", ["nil"])
case("eval-grouping", "(3 + 3) / 2", "evaluate", ["3"])
case("eval-eq", "1 == 1.0", "evaluate", ["true"])
case("eval-neq", '"a" != "b"', "evaluate", ["true"])

# ----------------------------------------------------------------- running
case("run-hello", 'print "Hello, World!";', "run", ["Hello, World!"])
case("run-vars", "var a = 1;\nprint a;", "run", ["1"])
case("run-scope", """
var a = "outer";
{
  var a = "inner";
  print a;
}
print a;
""", "run", ["inner", "outer"])
case("run-shadow-init-error", """
var a = "outer";
{
  var a = a;
  print a;
}
""", "run", [], 65,
     err_has="Can't read local variable in its own initializer.")
case("run-shadow-outer", """
var a = "outer";
{
  var a = "inner";
  print a;
}
print a;
""", "run", ["inner", "outer"])
case("run-undefined", "print missing;", "run", [], 70,
     err_has="Undefined variable 'missing'.")
case("run-if-else", "if (true) print 1; else print 2;", "run", ["1"])
case("run-else-if", "if (false) print 1; else if (true) print 2; else print 3;",
     "run", ["2"])
case("run-while", "var i = 0; while (i < 3) { print i; i = i + 1; }", "run",
     ["0", "1", "2"])
case("run-for", "for (var i = 0; i < 3; i = i + 1) print i;", "run",
     ["0", "1", "2"])
case("run-for-classic", "var i = 0; for (i = 0; i < 2; i = i + 1) print i;",
     "run", ["0", "1"])
case("run-and", "print false and 1;", "run", ["false"])
case("run-or", "print true or 2;", "run", ["true"])
case("run-redeclare-block", """
var a = 1;
{
  var a = 2;
  print a;
}
""", "run", ["2"])
case("run-self-init", "{ var a = a; }", "run", [], 65,
     err_has="Can't read local variable in its own initializer.")
case("run-return-toplevel", "return 10;", "run", [], 65,
     err_has="Can't return from top-level code.")

# ------------------------------------------------------------- functions
case("fn-args", """
fun add(a, b) { return a + b; }
print add(1, 2);
""", "run", ["3"])
case("fn-arity", """
fun f(a, b) { return a; }
print f(1);
""", "run", [], 70, err_has="Expected 2 arguments but got 1.")
case("fn-closure", """
fun makeCounter() {
  var count = 0;
  fun inc() {
    count = count + 1;
    return count;
  }
  return inc;
}
var c = makeCounter();
print c();
print c();
""", "run", ["1", "2"])
case("fn-scope-return", """
var a = "global";
fun show() {
  print a;
  var a = "block";
  print a;
}
show();
""", "run", ["global", "block"])
case("fn-native-clock", "print clock() > 0;", "run", ["true"])
case("fn-call-nonfn", 'var x = "s"; x();', "run", [], 70,
     err_has="Can only call functions and classes.")
case("fn-stringify", """
fun hi() {}
print hi;
""", "run", ["<fn hi>"])

# --------------------------------------------------------------- classes
case("cls-decl", """
class Dev {
  getName() { return "dev"; }
}
var d = Dev();
print d.getName();
""", "run", ["dev"])
case("cls-toString", """
class Cake {}
print Cake;
print Cake();
""", "run", ["Cake", "Cake instance"])
case("cls-init", """
class Dev {
  getName() { return "dev"; }
  init(name) { this.name = name; }
  printName() { print this.name; }
}
var d = Dev("ish");
d.printName();
""", "run", ["ish"])
case("cls-getter-setter", """
class Bacon {
  myFavField() { return this.field; }
  setFavField(v) { this.field = v; }
}
var b = Bacon();
b.setFavField("field");
print b.myFavField();
""", "run", ["field"])
case("cls-this-outside", "print this.x;", "run", [], 65,
     err_has="Can't use 'this' outside of a class.")
case("cls-init-bare-return", """
class C {
  init() { print "made"; return; }
}
print C();
""", "run", ["made", "C instance"])
case("cls-init-return-value-rejected", """
class C { init() { return 1; } }
""", "run", [], 65,
     err_has="Can't return a value from an initializer.")
case("cls-undefined-prop", """
class C {}
C().nope;
""", "run", [], 70, err_has="Undefined property 'nope'.")
case("cls-prop-on-noninstance", "var x = 1; x.y;", "run", [], 70,
     err_has="Only instances have properties.")

# ------------------------------------------------------------ inheritance
case("inh-basic", """
class A { f() { return "A.f"; } }
class B < A {}
print B().f();
""", "run", ["A.f"])
case("inh-super", """
class A { f() { return "A.f"; } }
class B < A { f() { return "B.f -> " + super.f(); } }
print B().f();
""", "run", ["B.f -> A.f"])
case("inh-super-init", """
class A { init() { this.name = "A"; } greet() { return "A:" + this.name; } }
class B < A { init() { super.init(); this.name = "B"; } greet() { return "B:" + this.name + " " + super.greet(); } }
print B().greet();
""", "run", ["B:B A:B"])
case("inh-super-missing-method", """
class A {}
class B < A { m() { super.nope(); } }
B().m();
""", "run", [], 70, err_has="Undefined property 'nope'.")
case("inh-self-inherit", "class A < A {}", "run", [], 65,
     err_has="A class can't inherit from itself.")
case("inh-superclass-not-class", 'class A {}\nvar x = 3;\nclass B < x {}', "run", [], 70,
     err_has="Superclass must be a class.")
case("inh-super-outside", "super.foo();", "run", [], 65,
     err_has="Can't use 'super' outside of a class.")
case("inh-super-no-superclass", """
class A { f() { super.f(); } }
""", "run", [], 65,
     err_has="Can't use 'super' in a class with no superclass.")
case("inh-super-ctor", """
class A { init() { print "A"; } }
class B < A { init() { print "B"; super.init(); } }
B();
""", "run", ["B", "A"])


def main():
    binary = os.path.abspath(sys.argv[1])
    only = sys.argv[2:]
    tmp = tempfile.mkdtemp(prefix="loxsmoke")
    path = os.path.join(tmp, "test.lox")
    fails = 0
    ran = 0
    bugs_ok = 0
    bugs_left = 0
    for c in CASES:
        if only and not any(f in c["name"] for f in only):
            continue
        ran += 1
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(c["src"])
        p = subprocess.run([binary, c["cmd"], path], capture_output=True,
                           text=True, cwd=tmp)
        out = p.stdout.split("\n")
        if out and out[-1] == "":
            out.pop()
        problems = []
        if out != c["out"]:
            problems.append(f"stdout {out} != {c['out']}")
        if p.returncode != c["exit"]:
            problems.append(f"exit {p.returncode} != {c['exit']}")
        if c["err_has"] and c["err_has"] not in p.stderr:
            problems.append(f"stderr {p.stderr!r} lacks {c['err_has']!r}")
        if problems:
            if c["bug"]:
                bugs_left += 1
                print(f"KNOWN-BUG still failing: {c['name']}")
            else:
                fails += 1
                print(f"FAIL {c['name']}")
                for pr in problems:
                    print("   ", pr)
        elif c["bug"]:
            bugs_ok += 1
            print(f"FIXED (was a known bug): {c['name']}")
    print(f"\n{ran} cases, {ran - fails} passed, {fails} failed")
    print(f"known bugs: {bugs_left} still failing, {bugs_ok} fixed")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())

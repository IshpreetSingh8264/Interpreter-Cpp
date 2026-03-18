# Tests

Local test infrastructure for the Lox interpreter. **No network, no test
framework** — `cmake`, a C++23 compiler and `python3` are all you need.

## Run everything

```sh
./tests/run.sh
```

That configures, builds, and runs every suite below. It exits non-zero if
anything fails, so it is safe to use as a gate.

| Command | What it runs |
|---|---|
| `./tests/run.sh` | build + both suites (the full 415 cases) |
| `./tests/run.sh unit` | the smoke suite only |
| `./tests/run.sh corpus` | the CodeCrafters corpus only |
| `./tests/run.sh unit inh-super` | smoke cases whose name contains `inh-super` |
| `./tests/run.sh corpus IB9` | corpus cases from stage `IB9` |

`BUILD_DIR` and `PYTHON` are honoured if set.

`ctest` works too, if you prefer it:

```sh
cmake -B build -S . && cmake --build build && ctest --test-dir build --output-on-failure
```

## What is here

```
tests/
  run.sh                     the entrypoint
  check_size.py              fails if a suite file passes 500 lines
  unit/smoke.py              85 hand-written cases
  corpus/corpus.json         330 cases extracted from `codecrafters test`
  corpus/run_corpus.py       replays corpus.json against the binary
  corpus/extract_corpus.py   REGENERATION tool, not run by run.sh
```

### `unit/smoke.py` — 85 hand-written cases

Small, readable, and the reason a refactor cannot quietly break the
interpreter. Every case pins the exact stdout, the exact exit code, and — for
error paths — a substring of stderr. The five commands are exercised in
proportion to their surface area:

- **scanning** (7) — every single-character token, all two-character
  operators, string/number literals, all 16 keywords, an unterminated string,
  and a `#` that is *not* a comment.
- **parsing** (24) — the AST printer for literals, unary/binary/grouping,
  logical operator precedence, calls, the full get/set chain shape, `this`,
  and `super`, plus syntax errors.
- **evaluation** (13) — arithmetic, string concatenation, truthiness, the
  numeric/static type checks, and equality across types.
- **running** (16) — block scoping and shadowing, the "can't read a local in
  its own initializer" rule, control flow, `and`/`or` short-circuiting,
  undefined variables, and the top-level `return` rejection.
- **functions** (7) — arity checking, closures capturing and mutating an
  upvalue, scope shadowing around a function body, the `clock` native,
  calling a non-callable, and function stringification.
- **classes and inheritance** (18) — declaration, instances, `toString`,
  `init`, getter/setter methods, `this` outside a class, a bare `return` from
  an initializer, `return <value>` from an initializer being rejected, and
  method lookup, overriding, `super.method`, `super.init`, self-inheritance,
  a non-class superclass, and `super` in a class with no superclass.

Run it directly with a filter to reproduce one case:

```sh
python3 tests/unit/smoke.py build/interpreter inh-super
```

If you find a genuine known defect, pass `bug=True` to `case()`. Such a case
is reported separately and never fails the run; when it starts passing it is
reported as `FIXED`, which is your signal to drop the flag and promote it to a
hard assertion. **There are currently no such cases** — the three that existed
when this suite was first written (`parse-and-or`, `parse-get`,
`parse-get-chain`) are now hard assertions.

### `corpus/` — 330 cases from the real tester

`corpus.json` was extracted from a real `codecrafters test` run: 330 cases
across all 84 stages, split 184 `run` / 57 `tokenize` / 51 `evaluate` / 38
`parse`. The value of this suite is that the expectations are **not
hand-written** — they are what the grader actually emitted.

The CodeCrafters log interleaves the program's two output streams under a
single `[your_program]` prefix and only reports how many expected lines
matched. So the log alone cannot tell you which stream a line came from.
`extract_corpus.py` therefore *reconstructs* the split from the `[line N]`
diagnostic prefix and stores it per case. That reconstruction is useful
documentation but it is **not a requirement the tester enforces** — see
"Two things this suite will not pretend to check" below.

So `run_corpus.py` asserts exactly what the tester actually enforces:

1. real **stdout+stderr combined** matches the expected combined lines, as a
   **multiset**;
2. the **exit code** matches, exactly.

Run it directly:

```sh
python3 tests/corpus/run_corpus.py tests/corpus/corpus.json build/interpreter
python3 tests/corpus/run_corpus.py tests/corpus/corpus.json build/interpreter IB9
```

To refresh the corpus when the stage list changes:

```sh
codecrafters test 2>&1 | tee /tmp/codecrafters-test.log
python3 tests/corpus/extract_corpus.py /tmp/codecrafters-test.log tests/corpus/corpus.json
```

Review the diff. A shrinking case count means the log was truncated, and the
`stream-classification mismatches:` line must stay at `0`.

### Two things this suite will not pretend to check

Both of these were found while building this suite, and both are cases where a
naive harness would either fail forever or assert something untrue.

**1. Which stream a line arrives on.** The corpus records that stage `YU6`
expects `Operands must be numbers.` on **stdout** and `[line 1]` on
**stderr**. This implementation writes *both* to stderr — and the stage still
passes, verified against a real 84/84 green `codecrafters test` log:

```
[tester::#YU6] [test-2] $ ./your_program.sh evaluate test.lox
[your_program] Operands must be numbers.
[your_program] [line 1]
[tester::#YU6] [test-2] ✓ 1 line(s) match on stdout
[tester::#YU6] [test-2] ✓ Received exit code 70.
```

The tester compares *combined* output, so the stream split is not a
requirement. Asserting it would produce **36 permanent false failures**. So it
is not asserted by default — but it is not swept under the rug either. The
runner always reports how many cases deviate, and `--strict-streams` turns the
split into a hard assertion so the deviation stays measurable:

```sh
python3 tests/corpus/run_corpus.py tests/corpus/corpus.json build/interpreter --strict-streams
# 84 stages, 330 cases, 294 passed, 36 failed   <- expected, and that is the point
```

If you ever deliberately change which stream errors go to, this is the command
that tells you how many corpus cases that moved.

**2. The digits of `clock()`.** Stage `AV4` expects `clock() + 58` to print
`1790510158` and `clock() / 1000` to print `1790510.1`. Both are consistent
with the fixture having captured `clock() == 1790510100`. No future run can
reproduce that. Comparing against the literal would fail every single time, so
those two cases assert the invariant instead: the output is a single line, it
matches the expected *number shape*, and it is positive. Nothing about the
digits is checked, because there is nothing true to check. The two cases are
listed in a `NONDETERMINISTIC` table at the top of `run_corpus.py` — extend it
rather than deleting a case, so the count of compromised expectations stays
visible.

## What this does NOT cover

Be aware of these limits before you trust a green run.

- **`codecrafters test` is still the only authority.** This suite is a fast
  pre-commit gate that runs in seconds. It is a *derived* oracle, not the
  grader. `run.sh` passing does not mean the stages pass; only
  `codecrafters test` does. Never run it to satisfy the grader — use it to
  catch a regression in one second instead of in three minutes.
- **Output ordering is not asserted by the corpus.** The combined comparison
  is a multiset, because the tester's interleaving order is not recoverable
  from the log. A program printing the right lines in the wrong order would
  pass the corpus. The 85 hand-written unit cases *do* assert exact stdout
  order, which is most of why they still exist.
- **Stream placement is not asserted** (36 corpus cases). See above. Use
  `--strict-streams` to measure it.
- **Two corpus cases cannot assert their recorded value** (`AV4` `clock()`).
  See above.
- **No coverage of `break` / `continue`.** They are deliberately not in the
  grammar. The suite does not assert that they are rejected either.
- **No Dart-chapter features.** No closures over class instances, no
  superclass method resolution at the `this` level beyond what
  `inh-super-init` exercises.
- **No numeric edge cases.** Floating-point printing, `nan`, `infinity`, very
  large literals, and integer-overflow behaviour are untested. `AV4` is the
  only arithmetic-on-a-non-literal case in the corpus, and it is exactly the
  one case whose value cannot be asserted.
- **No resource or stress testing.** Deep recursion near the C++ stack limit,
  10k-line files, thousands of globals, and pathological nesting are untested.
- **`clock` is only checked for being a positive number of the right shape.**
  It is non-deterministic, so nothing asserts its precision, unit, or
  monotonicity.
- **The corpus is a snapshot.** It reflects the 84 stages as of extraction.
  If the course adds stages, the corpus will not know about them until you
  re-run `extract_corpus.py`.
- **Build-level errors are not asserted.** Nothing checks that a bad build
  produces no binary; `run.sh` just fails if `build/interpreter` is missing.
- **The root `*.lox` files are not tests.** `test.lox`, `test_classes.lox`,
  `closures.lox` and friends are demo scripts the README tells you to run by
  hand. Nothing checks their output.

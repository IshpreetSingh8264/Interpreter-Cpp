#!/usr/bin/env python3
"""Replay the extracted CodeCrafters corpus against ./build/interpreter.

corpus.json holds 330 cases from a real `codecrafters test` run, across all 84
stages. The expectations are not hand-written: they are what the grader emitted.

WHAT IS ASSERTED (and why only this)
------------------------------------
The CodeCrafters tester log interleaves the program's two output streams under a
single `[your_program]` prefix and then reports how many expected lines matched.
It compares the program's *combined* output; it does not require a given line to
arrive on a particular stream. Evidence, from a real passing run
(84/84 stages green):

    [tester::#YU6] [test-2] $ ./your_program.sh evaluate test.lox
    [your_program] Operands must be numbers.      <- the fixture calls this stdout
    [your_program] [line 1]                       <- the fixture calls this stderr
    [tester::#YU6] [test-2] ✓ 1 line(s) match on stdout
    [tester::#YU6] [test-2] ✓ Received exit code 70.     <- stage PASSED

This implementation writes BOTH of those lines to stderr, and the stage still
passes. So the stream attribution in corpus.json records what the course
fixture *says*, not a requirement the tester enforces.

Therefore this runner asserts exactly what the tester enforces:
  1. real stdout+stderr == expected stdout+stderr, as a multiset
     (multiset, not sequence: the tester does not preserve interleaving order)
  2. the exit code, exactly

Pass --strict-streams to additionally assert each stream separately. That mode
is expected to fail on 36 evaluate-error cases and is provided so the deviation
stays visible and measurable rather than silently dropped. It is NOT the gate.

Usage: run_corpus.py <corpus.json> <binary> [stage ...] [--strict-streams]
"""
import json
import os
import re
import subprocess
import sys
import tempfile
from collections import Counter

argv = sys.argv[1:]
strict = "--strict-streams" in argv
argv = [a for a in argv if a != "--strict-streams"]

corpus = json.load(open(argv[0], encoding="utf-8"))
binary = os.path.abspath(argv[1])
only = set(argv[2:])

# Only used for corpora that predate the stdout/stderr split.
DIAG = re.compile(r"^\[line \d+\]")

tmp = tempfile.mkdtemp(prefix="loxcorpus")
path = os.path.join(tmp, "test.lox")

# ---------------------------------------------------------------------------
# Cases whose recorded expectation can never be reproduced: the fixture froze a
# clock() reading. The captured value was 1790510100 (test-1 expects that plus
# 58 = 1790510158, and test-2 expects that over 1000 = 1790510.1, so both are
# consistent with the same reading). Comparing against the literal would fail
# every run, so each case below asserts the invariant instead: clock() is a
# positive number, and the surrounding arithmetic still yields a number of the
# right shape. Nothing about the digits is checked -- there is nothing true to
# check.
# ---------------------------------------------------------------------------
NONDETERMINISTIC = {
    ("AV4", 1): r"^\d+$",                # print clock() + 58;
    ("AV4", 2): r"^\d+(\.\d+)?$",        # print clock() / 1000;
}

fails = 0
stream_diffs = 0
nondet_seen = 0
total = 0
by_stage = {}


def splitlines(s):
    lines = s.split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    return lines


for c in corpus:
    if only and c["stage"] not in only:
        continue
    total += 1
    with open(path, "w", encoding="utf-8") as fh:
        fh.write(c["source"])
    proc = subprocess.run(
        [binary, c["command"], path], capture_output=True, text=True, cwd=tmp
    )

    out_lines = splitlines(proc.stdout)
    err_lines = splitlines(proc.stderr)
    merged_lines = splitlines(proc.stdout + proc.stderr)

    if "stdout" in c:
        exp_stdout, exp_stderr = c["stdout"], c["stderr"]
    else:
        exp_stdout = [l for l in c["output"] if not DIAG.match(l)]
        exp_stderr = [l for l in c["output"] if DIAG.match(l)]

    # (1) combined output -- the real requirement
    exp_merged = Counter(exp_stdout + exp_stderr)
    got_merged = Counter(merged_lines)
    # (2) exit code -- the real requirement
    problems = []
    shape = NONDETERMINISTIC.get((c["stage"], c["test"]))
    if shape is not None:
        nondet_seen += 1
        if len(got_merged) != 1:
            problems.append(f"expected one line, got {sorted(got_merged.elements())}")
        else:
            (line,) = got_merged.elements()
            if not re.match(shape, line):
                problems.append(f"{line!r} does not match the expected number shape {shape}")
            elif float(line) <= 0:
                problems.append(f"clock()-derived value {line!r} is not positive")
    else:
        if got_merged != exp_merged:
            missing = exp_merged - got_merged
            extra = got_merged - exp_merged
            problems.append(
                f"combined: missing {sorted(missing.elements())} "
                f"extra {sorted(extra.elements())}"
            )
    if proc.returncode != c["exit"]:
        problems.append(f"exit: expected {c['exit']} got {proc.returncode}")

    # per-stream -- advisory unless --strict-streams
    streams_ok = out_lines == exp_stdout and err_lines == exp_stderr
    if not streams_ok:
        stream_diffs += 1
        if strict:
            if out_lines != exp_stdout:
                problems.append(f"stdout: expected {exp_stdout} got {out_lines}")
            if err_lines != exp_stderr:
                problems.append(f"stderr: expected {exp_stderr} got {err_lines}")

    s = by_stage.setdefault(c["stage"], [0, 0])
    s[0] += 1
    if problems:
        fails += 1
        print(f"FAIL {c['stage']} test-{c['test']} ({c['command']})  {c['source']!r}")
        for p in problems:
            print("   ", p)
    else:
        s[1] += 1

print(f"\n{len(by_stage)} stages, {total} cases, {total - fails} passed, {fails} failed")
if nondet_seen:
    print(
        f"note: {nondet_seen} case(s) assert a clock() *shape* instead of the "
        f"fixture's frozen digits."
    )
if stream_diffs:
    print(
        f"note: {stream_diffs} case(s) place lines on a different stream than the "
        f"course fixture does."
    )
    print(
        "      The tester compares combined output, so this is not a failure. "
        "Re-run with"
    )
    print(f"      --strict-streams to see them: {argv[0]} <binary> --strict-streams")
if fails:
    bad = [k for k, v in sorted(by_stage.items()) if v[0] != v[1]]
    print("failing stages:", " ".join(bad))
sys.exit(1 if fails else 0)

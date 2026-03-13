#!/usr/bin/env python3
"""Parse a plain `codecrafters test` log into a JSON regression corpus.

This is a REGENERATION tool, not a test.  It is not run by tests/run.sh and
nothing in the suite depends on it at run time -- corpus.json is checked in.

To refresh the corpus after the stage list changes:
    codecrafters test 2>&1 | tee /tmp/codecrafters-test.log
    python3 tests/corpus/extract_corpus.py /tmp/codecrafters-test.log \\
            tests/corpus/corpus.json

Then review the diff: a shrinking case count means the log was truncated, and
the stream-classification mismatch count on stderr must stay 0 or your stdout /
stderr split is being guessed wrong and the corpus will encode the guess.
"""
import json
import re
import sys

log_path, out_path = sys.argv[1], sys.argv[2]

lines = open(log_path, encoding="utf-8").read().split("\n")

cases = []
stage = None
cur = None
mode = None  # 'input' | 'output'


def flush():
    global cur
    if cur is not None:
        cases.append(cur)
        cur = None


for ln in lines:
    m = re.match(r"\[tester::#(\w+)\] Running tests for Stage #\w+ \((.*)\)\s*$", ln)
    if m:
        flush()
        stage = m.group(1)
        continue

    m = re.match(r"\[tester::#(\w+)\] \[test-(\d+)\] Writing contents to \./(test\S*):\s*$", ln)
    if m:
        flush()
        cur = {
            "stage": m.group(1),
            "test": int(m.group(2)),
            "command": None,
            "input": [],
            "output": [],
            "exit": 0,
        }
        mode = "input"
        continue

    if cur is not None and mode == "input":
        m = re.match(r"\[tester::#\w+\] \[test-\d+\.lox\] ?(.*)$", ln)
        if m:
            cur["input"].append(m.group(1))
            continue

    m = re.match(r"\[tester::#\w+\] \[test-\d+\] \$ \./your_program\.sh (\w+)", ln)
    if m and cur is not None:
        cur["command"] = m.group(1)
        mode = "output"
        continue

    if cur is not None and mode == "output":
        if ln.startswith("[your_program] "):
            cur["output"].append(ln[len("[your_program] "):])
            continue
        if ln == "[your_program]":
            cur["output"].append("")
            continue
        m = re.search(r"\[tester::#\w+\] \[test-\d+\].*Received exit code (\d+)\.", ln)
        if m:
            cur["exit"] = int(m.group(1))
            mode = None
            continue
        m = re.search(r"\[tester::#\w+\] \[test-\d+\].*(\d+) line\(s\) match on stdout", ln)
        if m:
            cur["n_stdout"] = int(m.group(1))
            continue
        m = re.search(r"\[tester::#\w+\] \[test-\d+\].*(\d+) line\(s\) match on stderr", ln)
        if m:
            cur["n_stderr"] = int(m.group(1))
            continue
        if "Test passed." in ln:
            mode = None
            continue

flush()

# Classify each output line by the stream it must belong to, then verify the
# classification against the per-stream line counts the tester reported.
DIAG = re.compile(r"^\[line \d+\]")          # "[line 4] Error at 'x': ..."
BARE = re.compile(r"^\[line \d+\]$")         # "[line 4]"
bad = 0
for c in cases:
    out, err = [], []
    for l in c["output"]:
        (err if (DIAG.match(l) or BARE.match(l)) else out).append(l)
    c["stdout"] = out
    c["stderr"] = err
    if c.get("n_stdout", len(out)) != len(out) or c.get("n_stderr", len(err)) != len(err):
        bad += 1
        print(f"  classify-mismatch {c['stage']}/{c['test']}: "
              f"log said stdout={c.get('n_stdout')} stderr={c.get('n_stderr')}, "
              f"rule says stdout={len(out)} stderr={len(err)}", file=sys.stderr)
    c.pop("n_stdout", None)
    c.pop("n_stderr", None)
    c.pop("output", None)
print(f"  stream-classification mismatches: {bad}", file=sys.stderr)

# Resolve file-content sentinels.
for c in cases:
    src = "\n".join(c["input"])
    if src == "<|EMPTY FILE|>":
        src = ""
    # <|SPACE|>/<|TAB|> are sentinels for whitespace the log would eat
    src = src.replace("<|SPACE|>", " ").replace("<|TAB|>", "\t")
    c["source"] = src
    del c["input"]

json.dump(cases, open(out_path, "w", encoding="utf-8"), indent=1)
print(f"{len(cases)} cases across {len(set(c['stage'] for c in cases))} stages -> {out_path}")

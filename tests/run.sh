#!/usr/bin/env bash
#
# The one command that runs every local test in this repo.
#
#   ./tests/run.sh                  # build, then run everything
#   ./tests/run.sh unit             # just the 85-case smoke suite
#   ./tests/run.sh corpus           # just the 330-case CodeCrafters corpus
#   ./tests/run.sh unit inh-super   # one subset, filtered by case name
#   ./tests/run.sh corpus IB9       # one stage, filtered by stage id
#
# Exits non-zero if any suite fails. No network, no dependencies beyond
# cmake, a C++23 compiler and python3.
set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$REPO/build}"
PY="${PYTHON:-python3}"

cd "$REPO"

# The first argument names the suite. Anything that is not a suite name is
# treated as a filter applied to every suite, so `./tests/run.sh inh-super`
# still works and still builds.
ALL="unit corpus"
SUITES="$ALL"
FILTER=()
case "${1:-}" in
  unit|corpus)
    SUITES="$1"
    shift
    FILTER=("$@")
    ;;
  *)
    FILTER=("$@")
    ;;
esac

# `VCPKG_ROOT` is set by your_program.sh; honour the toolchain file if it is
# there so a clean checkout builds the same way it does for CodeCrafters.
CMAKE_ARGS=(-B "$BUILD_DIR" -S .)
if [ -n "${VCPKG_ROOT:-}" ] && [ -f "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ]; then
  CMAKE_ARGS+=(-DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
fi

want() {
  case " $SUITES " in *" $1 "*) return 0 ;; *) return 1 ;; esac
}

if want unit || want corpus; then
  echo "== configure =="
  cmake "${CMAKE_ARGS[@]}" >/dev/null
  echo "== build =="
  cmake --build "$BUILD_DIR" >/dev/null
  BIN="$BUILD_DIR/interpreter"
  [ -x "$BIN" ] || { echo "FATAL: $BIN was not produced" >&2; exit 1; }
fi

rc=0

if want unit; then
  echo
  echo "== smoke suite (tests/unit/smoke.py) =="
  "$PY" tests/unit/smoke.py "$BIN" ${FILTER[@]+"${FILTER[@]}"} || rc=1
fi

if want corpus; then
  echo
  echo "== CodeCrafters corpus (tests/corpus/run_corpus.py) =="
  "$PY" tests/corpus/run_corpus.py tests/corpus/corpus.json "$BIN" \
    ${FILTER[@]+"${FILTER[@]}"} || rc=1
fi

echo
echo "== suite size budget =="
"$PY" tests/check_size.py tests || rc=1

echo
if [ $rc -eq 0 ]; then
  echo "ALL SUITES PASSED"
else
  echo "SOME SUITES FAILED"
fi
exit $rc

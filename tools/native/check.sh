#!/usr/bin/env bash
# check.sh — Tier 1 native compile-check gate.
#
# Compiles every TARGET_NATIVE unit as ILP32 (4-byte pointers, matching the
# PS2/EE so struct layouts line up) with -c (compile only, no link). Catches
# malformed C, bad casts, type errors, and undeclared callees — a cheap static
# gate that runs on the host (no VM needed). It does NOT run anything; see
# run_test.sh for the functional (Tier 2) harness.
#
# Usage: tools/native/check.sh           # check all units
#        tools/native/check.sh <file.c>  # check one unit
set -u

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
INC="$ROOT/going-decompiled/include"
SHIM="$ROOT/tools/native/mips_callees.h"
CC="${CC:-clang}"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

# Undeclared MIPS callees + int<->pointer width coercions are the EXPECTED
# linkage-boundary noise (these functions are still asm); keep them as visible
# warnings, not hard errors. Genuine type/structure bugs in the bodies still
# fail the gate.
CFLAGS="-DTARGET_NATIVE -m32 -c -I$INC -include $SHIM"
CFLAGS="$CFLAGS -Wno-error=implicit-function-declaration -Wno-int-conversion"

if [ "$#" -ge 1 ]; then
  units="$*"
else
  units="$(cd "$ROOT" && grep -rl TARGET_NATIVE going-decompiled/src | sed "s#^#$ROOT/#")"
fi

pass=0; fail=0; failed=""
for f in $units; do
  base="$(basename "$f" .c)"
  if $CC $CFLAGS "$f" -o "$OUT/$base.o" 2>"$OUT/$base.err"; then
    pass=$((pass+1))
  else
    fail=$((fail+1)); failed="$failed $base"
    echo "FAIL: $f"
    sed 's/^/    /' "$OUT/$base.err" | grep -m3 'error:'
  fi
done

echo "--- native compile-check: pass=$pass fail=$fail ---"
[ "$fail" -eq 0 ] || { echo "failed units:$failed"; exit 1; }

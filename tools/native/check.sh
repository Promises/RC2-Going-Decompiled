#!/usr/bin/env bash
# check.sh — Tier 1 native compile-check gate.
#
# Compiles every TARGET_NATIVE unit as ILP32 (4-byte pointers, matching the
# PS2/EE so struct layouts line up) with -c (compile only, no link). Catches
# malformed C, bad casts, type errors, and undeclared callees — a cheap static
# gate that runs on the host (no VM needed). It does NOT run anything; see
# run_test.sh for the functional (Tier 2) harness.
#
# ⚠️ GATE INSTRUMENT (task #923): tools/ee/landing_gate.sh's NATIVE row runs
# THIS script on both the tip and the base and parses its `FAIL: <path>` lines
# and its `--- native compile-check: pass=N fail=M ---` summary. A change here
# changes the landing gate: validate it with `landing_gate.sh --selftest`
# (arm 17), not as a loose script. It never links — see the row's bound.
#
# A `.cpp` unit (a unit converted to C++, task #1258) is compiled as C++ inside
# ONE `extern "C" { ... }` that this script writes, exactly as the EE build
# (tools/ee/ee_cc1.sh) wraps it, so the source carries no wrapper of its own.
# The mips_callees.h shim goes INSIDE that block (a shim declaration with C++
# linkage would conflict with the unit's extern "C" one). C++ has no implicit
# declarations or int<->pointer conversions to relax, so the two
# -Wno-error/-Wno-int-conversion relaxations below are C-only.
#
# Usage: tools/native/check.sh                  # check all units
#        tools/native/check.sh <file.c|file.cpp> # check one unit
set -u

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
INC="$ROOT/going-decompiled/include"
SHIM="$ROOT/tools/native/mips_callees.h"
# The units are ELF-targeted C: 198FA0.c and others carry `section` attributes
# in ELF syntax. On a Mac, plain `clang` targets Mach-O and rejects them, so
# EVERY such unit fails for a reason that measures the HOST, not the code —
# and a validator reading "fail" everywhere learns nothing (2026-09-28: a real
# native regression in 198FA0.c went unseen that way). Default to an i386 ELF
# target there, borrowing the SDK's libc headers. An explicit CC still wins.
if [ -z "${CC:-}" ] && [ "$(uname -s)" = "Darwin" ]; then
  CC="clang --target=i386-pc-linux-gnu -isystem $(xcrun --show-sdk-path)/usr/include"
fi
CC="${CC:-clang}"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

# Undeclared MIPS callees + int<->pointer width coercions are the EXPECTED
# linkage-boundary noise (these functions are still asm); keep them as visible
# warnings, not hard errors. Genuine type/structure bugs in the bodies still
# fail the gate.
# -ferror-limit=0: report every error, so the per-unit count below is the
# true count, not clang's default 20-error cutoff.
CFLAGS="-DTARGET_NATIVE -m32 -c -ferror-limit=0 -I$INC -include $SHIM"
CFLAGS="$CFLAGS -Wno-error=implicit-function-declaration -Wno-int-conversion"

if [ "$#" -ge 1 ]; then
  units="$*"
else
  units="$(cd "$ROOT" && grep -rl TARGET_NATIVE going-decompiled/src | sed "s#^#$ROOT/#")"
fi

# Each unit is labelled by its path under going-decompiled/src, REGION
# INCLUDED (usa/cod/015180, not 015180): usa and eu share basenames
# (cod/015180.c, cod/0321A0.c), and a bare basename made a usa failure and an
# eu failure print the same `failed` member (task #923).
CXXFLAGS="-x c++ -DTARGET_NATIVE -m32 -c -ferror-limit=0 -I$INC"

pass=0; fail=0; failed=""
for f in $units; do
  label="${f#"$ROOT"/}"; label="${label#going-decompiled/src/}"
  case "$label" in *.cpp) label="${label%.cpp}" ;; *) label="${label%.c}" ;; esac
  obj="$(printf '%s' "$label" | tr '/' '_')"
  case "$f" in
    *.cpp)
      # absolute paths: the wrapper lives in $OUT, not beside the unit
      case "$f" in /*) abs="$f" ;; *) abs="$PWD/$f" ;; esac
      printf 'extern "C" {\n#include "%s"\n#include "%s"\n}\n' "$SHIM" "$abs" > "$OUT/$obj.wrap.cpp"
      cmd="$CC $CXXFLAGS $OUT/$obj.wrap.cpp" ;;
    *) cmd="$CC $CFLAGS $f" ;;
  esac
  if $cmd -o "$OUT/$obj.o" 2>"$OUT/$obj.err"; then
    pass=$((pass+1))
  else
    fail=$((fail+1)); failed="$failed $label"
    echo "FAIL: $f"
    # Print the unit's FULL error count, then a 3-line sample. The sample alone
    # used to be all a reader saw, so a screen built on this output counted at
    # most 3 errors per unit (task #1300/#1311). -ferror-limit=0 (above) stops
    # clang's own 20-error cutoff from capping the count.
    nerr=$(grep -c 'error:' "$OUT/$obj.err" || true)
    echo "    errors: $nerr (first 3 shown)"
    sed 's/^/    /' "$OUT/$obj.err" | grep -m3 'error:'
  fi
done

echo "--- native compile-check: pass=$pass fail=$fail ---"
[ "$fail" -eq 0 ] || { echo "failed units:$failed"; exit 1; }

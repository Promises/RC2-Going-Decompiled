#!/usr/bin/env bash
# run_test.sh — Tier 2 functional-test runner.
#
# Compiles a unit-under-test + a test file together as an ILP32 (4-byte-ptr)
# x86 ELF in the colima native-build image, links with --gc-sections so only
# the function(s) the test actually calls (and their real callees) survive —
# you only mock what's reached — then runs it. Exit status = failed checks.
#
# Usage: tools/native/run_test.sh <unit.c> <test.c>
#   e.g. tools/native/run_test.sh going-decompiled/src/usa/text/250080.c \
#                                 tools/native/tests/test_250080.c
set -eu

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
INC="$ROOT/going-decompiled/include"
NATIVE="$ROOT/tools/native"
IMG="${NATIVE_IMG:-native-build}"
CTX="${DOCKER_CONTEXT:-colima-ee-x86}"

[ "$#" -eq 2 ] || { echo "usage: $0 <unit.c> <test.c>" >&2; exit 2; }
UNIT_REL="${1#$ROOT/}"
TEST_REL="${2#$ROOT/}"

# ILP32, TARGET_NATIVE, force-include the libc prelude + the test header. Keep
# the MIPS-callee linkage boundary non-fatal at compile (real undefined refs in
# *reached* code still fail at link, which is what we want).
CFLAGS="-m32 -DTARGET_NATIVE -O0 -g \
  -I/work/going-decompiled/include -I/work/tools/native \
  -include /work/tools/native/mips_callees.h \
  -ffunction-sections -fdata-sections \
  -Wno-implicit-function-declaration -Wno-int-conversion -Wno-builtin-declaration-mismatch"

docker --context "$CTX" run --rm -v "$ROOT":/work -w /work "$IMG" sh -c "
  set -e
  gcc $CFLAGS -Wl,--gc-sections '/work/$UNIT_REL' '/work/$TEST_REL' -o /tmp/nt_bin
  /tmp/nt_bin
"

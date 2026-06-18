#!/usr/bin/env bash
# run_state_native.sh — host-side state-seeded runner (native twin of the EE
# run_state_suite). Links a test against the M2 globals arena, seeds it from a
# PINE snapshot (tools/ee/eetest/state/*.bin), and runs on the host (ILP32 x86,
# no PCSX2). Cross-validates the arena placement + snapshot<->arena mapping, and
# is the basis for running functions against real state on the host once the
# reached subgraph (M4 stubs) is exercised.
#
# Usage: tools/native/run_state_native.sh [test.c]   (default state_seed_test.c)
# Exit status = the test's exit (0 pass). A trap-stub hit exits 99 (M4).
set -eu

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TEST="${1:-tools/native/state_seed_test.c}"
IMG="${NATIVE_IMG:-native-build}"
CTX="${DOCKER_CONTEXT:-colima-ee-x86}"
ARENA="tools/native/runtime/arena"

docker --context "$CTX" run --rm -v "$ROOT":/work -w /work "$IMG" sh -c "
  set -e
  CF='-m32 -DTARGET_NATIVE -O0 -I/work/going-decompiled/include -I/work/tools/native'
  gcc \$CF -c '$TEST' -o /tmp/test.o
  gcc \$CF -c $ARENA/arena_storage.c -o /tmp/arena.o
  gcc -m32 /tmp/test.o /tmp/arena.o -Wl,-T,$ARENA/arena.ld -lm -o /tmp/nstate
  /tmp/nstate
"

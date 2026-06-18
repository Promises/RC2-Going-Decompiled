#!/usr/bin/env bash
# run_state_batch.sh — host-side batch runner / trap-stub fuzzer. Links the full
# TARGET_NATIVE corpus (M2 arena + M4 stubs + libc) with state_run_batch.c as an
# executable (--gc-sections, so only the reached subgraph links), seeds the arena
# from a PINE snapshot, and runs each target function in a forked child. Reports
# [clean]/[STUB]/[CRASH] per function — the STUB/CRASH cases are decomper's
# next-priority surface (a stub name is printed to stderr).
set -eu

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
IMG="${NATIVE_IMG:-native-build}"
CTX="${DOCKER_CONTEXT:-colima-ee-x86}"

docker --context "$CTX" run --rm -v "$ROOT":/work -w /work "$IMG" sh -c '
  set -e
  CF="-m32 -DTARGET_NATIVE -O0 -I/work/going-decompiled/include -I/work/tools/native -I/work/tools/native/runtime/rt0 -include /work/tools/native/mips_callees.h -ffunction-sections -fdata-sections -Wno-implicit-function-declaration -Wno-int-conversion -Wno-builtin-declaration-mismatch"
  objs=""
  for f in $(grep -rl TARGET_NATIVE going-decompiled/src); do
    b=$(basename "$f" .c); gcc $CF -c "$f" -o /tmp/$b.o 2>/dev/null && objs="$objs /tmp/$b.o" || true
  done
  gcc $CF -c tools/native/runtime/arena/arena_storage.c -o /tmp/arena.o
  gcc $CF -c tools/native/runtime/rt0/stubs.c           -o /tmp/stubs.o
  gcc $CF -c tools/native/runtime/rt0/native_stub.c     -o /tmp/nstub.o
  gcc $CF -c tools/native/state_run_batch.c             -o /tmp/batch.o
  gcc -m32 -Wl,--gc-sections -Wl,-T,tools/native/runtime/arena/arena.ld \
      /tmp/batch.o $objs /tmp/arena.o /tmp/stubs.o /tmp/nstub.o -lm -o /tmp/nbatch 2>/tmp/link.err \
    || { echo "LINK FAILED (undefined = unplaced global or missing stub — decomper priority):";
         grep -iE "undefined reference" /tmp/link.err | head; exit 1; }
  /tmp/nbatch
'

#!/usr/bin/env bash
# run_state_batch.sh — host-side batch runner / trap-stub fuzzer. Links the full
# TARGET_NATIVE corpus (M2 arena + M4 stubs + libc) with state_run_batch.c as an
# executable (--gc-sections, so only the reached subgraph links), seeds the arena
# from a PINE snapshot, and runs each target function in a forked child. Reports
# [clean]/[STUB]/[CRASH] per function — the STUB/CRASH cases are decomper's
# next-priority surface (a stub name is printed to stderr).
set -eu

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"
IMG="${NATIVE_IMG:-native-build}"
CTX="${DOCKER_CONTEXT:-colima-ee-x86}"

# regenerate the fuzz batch from all void(void) TARGET_NATIVE funcs (host python)
python3 tools/native/gen_batch.py going-decompiled/src > tools/native/state_batch_gen.c

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
  gcc $CF -c tools/native/state_batch_gen.c             -o /tmp/batch.o
  # M1 SDK shims + M3 ps2hw backend (strong defs overriding weak trap-stubs) —
  # mirror link_native.sh so the runner tracks the decomper`s evolving backend.
  back="/tmp/arena.o /tmp/stubs.o /tmp/nstub.o"
  for s in tools/native/runtime/sdk/*.c tools/native/runtime/hw/*.c; do
    [ -e "$s" ] || continue; sb=$(basename "$s" .c)
    gcc $CF -c "$s" -o /tmp/rt_$sb.o 2>/dev/null && back="$back /tmp/rt_$sb.o" || true
  done
  # Self-heal: probe-link to find anything STILL undefined (symbols the backend
  # does not yet cover), and auto-generate a weak trap-stub for each so the link
  # succeeds and a hit reports the exact missing symbol (decomper priority).
  gcc -m32 -shared -Wl,--allow-multiple-definition -Wl,--unresolved-symbols=ignore-all -Wl,-T,tools/native/runtime/arena/arena.ld \
      /tmp/batch.o $objs $back -lm -o /tmp/probe.so 2>/dev/null || true
  echo "extern void native_stub_hit(const char *);" > /tmp/auto_stubs.c
  nm -u /tmp/probe.so | sed "s/^ *[Uw] //" | sort -u \
    | grep -v "@GLIBC" | grep -vE "^(_ITM_|__gmon_start__|__cxa_finalize|stderr|native_stub_hit)$" \
    | sed "s/.*/__attribute__((weak)) void \\0(void){ native_stub_hit(\"\\0\"); }/" >> /tmp/auto_stubs.c
  echo "auto-stubbed $(($(wc -l < /tmp/auto_stubs.c)-1)) still-undefined symbols (not in backend)"
  gcc $CF -c /tmp/auto_stubs.c -o /tmp/auto_stubs.o
  gcc -m32 -Wl,--allow-multiple-definition -Wl,--gc-sections -Wl,-T,tools/native/runtime/arena/arena.ld \
      /tmp/batch.o $objs $back /tmp/auto_stubs.o -lm -o /tmp/nbatch 2>/tmp/link.err \
    || { echo "LINK FAILED:"; head -25 /tmp/link.err; exit 1; }
  /tmp/nbatch
'

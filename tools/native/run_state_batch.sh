#!/usr/bin/env bash
# run_state_batch.sh — host-side batch runner / trap-stub fuzzer. Links the full
# TARGET_NATIVE corpus (M2 arena + M4 stubs + libc) with state_run_batch.c as an
# executable (--gc-sections, so only the reached subgraph links), seeds the arena
# from a PINE snapshot, and runs each target function in a forked child. Reports
# [clean]/[STUB]/[CRASH] per function — the STUB/CRASH cases are decomper's
# next-priority surface (a stub name is printed to stderr).
#
# Compilers (task #1460): a .c unit compiles with the image's gcc, a .cpp unit
# with the image's clang (pinned clang-14) as gnu++98 C++ inside one
# `extern "C" { shim; unit }`, as check.sh and runtime/link_native.sh compile it.
# Before #1460 every unit went to gcc with `2>/dev/null ... || true`: g++ 12
# rejects the .cpp units' `_Static_assert`, so 10 of them vanished from the link
# unreported, and the 7 it did compile carried C++-mangled names that no C
# reference could reach. A unit, backend or probe link that fails is now a FAIL
# with the tool's own output.
#
# The final link keeps --allow-multiple-definition: GNU ld then keeps the FIRST
# definition, and the units come before the runtime backends, so where a unit
# and backend_null.c/snd_null.c both define a symbol the unit's body is the one
# that runs here. The duplicates are listed (from the probe link without the
# flag) so that choice is visible, not silent.
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
  CXF="-x c++ -std=gnu++98 -m32 -DTARGET_NATIVE -O0 -I/work/going-decompiled/include -I/work/tools/native -I/work/tools/native/runtime/rt0 -ffunction-sections -fdata-sections"
  objs=""; total=0; failed=""
  for f in $(grep -rl TARGET_NATIVE going-decompiled/src); do
    b=$(basename "$f"); b=${b%.*}; total=$((total+1))
    case "$f" in
      *.cpp) printf "extern \"C\" {\n#include \"%s\"\n#include \"%s\"\n}\n" \
               /work/tools/native/mips_callees.h "/work/$f" > /tmp/$b.wrap.cpp
             set -- clang $CXF /tmp/$b.wrap.cpp ;;
      *)     set -- gcc $CF "$f" ;;
    esac
    if "$@" -c -o /tmp/$b.o 2>/tmp/$b.cerr; then objs="$objs /tmp/$b.o"
    else failed="$failed $f"; fi
  done
  echo "units compiled (.c: gcc, .cpp: clang): $(( total - $(echo $failed | wc -w) )) / $total"
  if [ -n "$failed" ]; then
    echo "FAIL - unit(s) did not compile, so the batch cannot link them:$failed"
    for f in $failed; do b=$(basename "$f"); b=${b%.*}; echo "   $f:"; { grep -m3 "error:" /tmp/$b.cerr || head -3 /tmp/$b.cerr; } | sed "s/^/      /"; done
    exit 1
  fi
  gcc $CF -c tools/native/runtime/arena/arena_storage.c -o /tmp/arena.o
  gcc $CF -c tools/native/runtime/rt0/stubs.c           -o /tmp/stubs.o
  gcc $CF -c tools/native/runtime/rt0/native_stub.c     -o /tmp/nstub.o
  gcc $CF -c tools/native/state_batch_gen.c             -o /tmp/batch.o
  # M1 SDK shims + M3 ps2hw backend (strong defs overriding weak trap-stubs) —
  # mirror link_native.sh so the runner tracks the decomper`s evolving backend.
  back="/tmp/arena.o /tmp/stubs.o /tmp/nstub.o"
  for s in tools/native/runtime/sdk/*.c tools/native/runtime/hw/*.c; do
    [ -e "$s" ] || continue; sb=$(basename "$s" .c)
    gcc $CF -c "$s" -o /tmp/rt_$sb.o; back="$back /tmp/rt_$sb.o"
  done
  # Self-heal: probe-link to find anything STILL undefined (symbols the backend
  # does not yet cover), and auto-generate a weak trap-stub for each so the link
  # succeeds and a hit reports the exact missing symbol (decomper priority).
  gcc -m32 -shared -Wl,--allow-multiple-definition -Wl,--unresolved-symbols=ignore-all -Wl,-T,tools/native/runtime/arena/arena.ld \
      /tmp/batch.o $objs $back -lm -o /tmp/probe.so 2>/tmp/probe.err \
    || { echo "PROBE LINK FAILED:"; sed "s/^/   /" /tmp/probe.err; exit 1; }
  # The same probe without --allow-multiple-definition, only to NAME what that
  # flag resolves (first definition wins); its failure is expected and reported.
  gcc -m32 -shared -Wl,--unresolved-symbols=ignore-all -Wl,-T,tools/native/runtime/arena/arena.ld \
      /tmp/batch.o $objs $back -lm -o /tmp/probe_strict.so 2>/tmp/probe_strict.err || true
  ndup=$(grep -c "multiple definition of" /tmp/probe_strict.err || true)
  echo "multiple definitions resolved first-wins by --allow-multiple-definition: $ndup"
  sed -n "s/.*multiple definition of .\([A-Za-z0-9_]*\).; \/tmp\/\([^:]*\):.*/   \1  (kept: \2)/p" /tmp/probe_strict.err | sort
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

#!/usr/bin/env bash
# link_native.sh — full TARGET_NATIVE link (M1 libc + M2 arena + M4 stubs).
#
# Compiles every TARGET_NATIVE unit + the arena backing block + the M4 weak stub
# table, links them as one ILP32 object set against arena.ld WITH host libc/libm,
# and reports the GENUINE residual gaps — i.e. undefined symbols that are NOT
# dynamic libc imports (@GLIBC...) and NOT linker housekeeping weaks. A clean
# run leaves only the arena's still-unaddressed globals (arena_unresolved.txt,
# being recovered as Track-B); once those land, the residual is empty and the
# whole corpus links 0-undefined.
#
# This is the basis for the runnable harness backend: link a test + the units
# with --gc-sections and only the reached subgraph must resolve; stubs trap on
# any unimplemented function reached. See docs/HLE.md (M1/M2/M4).
set -eu

ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
RT="$ROOT/tools/native/runtime"
IMG="${NATIVE_IMG:-native-build}"
CTX="${DOCKER_CONTEXT:-colima-ee-x86}"

docker --context "$CTX" run --rm -v "$ROOT":/work -w /work "$IMG" sh -c '
  set -e
  CFLAGS="-m32 -DTARGET_NATIVE -O0 -I/work/going-decompiled/include -I/work/tools/native -I/work/tools/native/runtime/rt0 -include /work/tools/native/mips_callees.h -ffunction-sections -fdata-sections -Wno-implicit-function-declaration -Wno-int-conversion -Wno-builtin-declaration-mismatch"
  objs=""
  for f in $(grep -rl TARGET_NATIVE going-decompiled/src); do
    b=$(basename "$f" .c)
    gcc $CFLAGS -c "$f" -o /tmp/$b.o 2>/dev/null && objs="$objs /tmp/$b.o"
  done
  gcc $CFLAGS -c tools/native/runtime/arena/arena_storage.c -o /tmp/arena_storage.o
  gcc $CFLAGS -c tools/native/runtime/rt0/stubs.c           -o /tmp/stubs.o
  gcc $CFLAGS -c tools/native/runtime/rt0/native_stub.c     -o /tmp/native_stub.o
  gcc -m32 -shared -Wl,--unresolved-symbols=ignore-all \
      -Wl,-T,tools/native/runtime/arena/arena.ld \
      $objs /tmp/arena_storage.o /tmp/stubs.o /tmp/native_stub.o -lm \
      -o /tmp/native_full.so 2>/dev/null
  # Genuine gaps only: drop versioned libc imports + housekeeping weaks.
  nm -u /tmp/native_full.so | sed "s/^ *[Uw] //" | sort -u \
    | grep -v "@GLIBC" | grep -vE "^(_ITM_|__gmon_start__|__cxa_finalize|stderr$)" \
    > /work/tools/native/runtime/.residual.txt || true
'

RES="$RT/.residual.txt"
n=$(wc -l < "$RES" | tr -d ' ')
echo "=== full native link: genuine residual gaps = $n ==="
if [ "$n" -gt 0 ]; then
  cat "$RES" | sed 's/^/  /'
  echo "(these are the arena's still-unaddressed globals — Track-B; see arena_unresolved.txt)"
fi
rm -f "$RES"

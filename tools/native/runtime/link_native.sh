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
#
# Compilers (task #1460): a .c unit compiles with the image's gcc; a .cpp unit
# compiles with the image's clang (pinned clang-14, tools/native/Dockerfile) as
# gnu++98 C++ inside one `extern "C" { shim; unit }`, exactly as check.sh and
# arena/verify_link.sh compile it. Before #1460 the .cpp units went to gcc
# unwrapped: g++ 12 rejects their `_Static_assert`, which dropped 10 of them,
# and the 7 it did compile carried C++-MANGLED names (`_Z13DrawGlyphQuad...`),
# so none of their definitions could satisfy a C reference.
#
# Nothing here discards a diagnostic. A unit that does not compile, or a link
# that fails (e.g. a runtime backend and a unit both defining a symbol), is a
# FAIL with the compiler's or linker's own output; until #1460 the link's
# stderr went to /dev/null and its failure read as a bare rc 1.
set -eu

ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
RT="$ROOT/tools/native/runtime"
IMG="${NATIVE_IMG:-native-build}"
CTX="${DOCKER_CONTEXT:-colima-ee-x86}"

docker --context "$CTX" run --rm -v "$ROOT":/work -w /work "$IMG" sh -c '
  set -e
  CFLAGS="-m32 -DTARGET_NATIVE -O0 -I/work/going-decompiled/include -I/work/tools/native -I/work/tools/native/runtime/rt0 -include /work/tools/native/mips_callees.h -ffunction-sections -fdata-sections -Wno-implicit-function-declaration -Wno-int-conversion -Wno-builtin-declaration-mismatch"
  # .cpp units: the CXXFLAGS of check.sh (gnu++98 pinned, task #1361). The shim is
  # included INSIDE the extern "C" wrapper, not by -include. -fno-strict-return
  # (RULING #9179 part 2, task #1489): clang otherwise plants a ud2 trap where a
  # non-void C++ function falls off its end. 39 such sites (198FA0.cpp 38,
  # 1907F0.cpp 1) are not yet classified by ROM epilogue under RULING #9122. Until
  # that row lands the flag makes them return whatever is in the return register,
  # as a C compiler would, instead of trapping. clang-only: the .c units compile
  # as C with gcc, where falling off the end is not trapped.
  CXXFLAGS="-x c++ -std=gnu++98 -fno-strict-return -m32 -DTARGET_NATIVE -O0 -I/work/going-decompiled/include -I/work/tools/native -I/work/tools/native/runtime/rt0 -ffunction-sections -fdata-sections"
  # USA-only native build: the arena (arena.ld), stub table, and symbol_addrs are
  # all USA. EU units with #else bodies belong to a separate (future) EU native
  # build with its own EU arena - do NOT pull them into this link.
  objs=""; total=0; okc=0; failed=""
  for f in $(grep -rl TARGET_NATIVE going-decompiled/src/usa | sort); do
    b=$(basename "$f"); b=${b%.*}; total=$((total+1))
    case "$f" in
      *.cpp) printf "extern \"C\" {\n#include \"%s\"\n#include \"%s\"\n}\n" \
               /work/tools/native/mips_callees.h "/work/$f" > /tmp/$b.wrap.cpp
             set -- clang $CXXFLAGS /tmp/$b.wrap.cpp ;;
      *)     set -- gcc $CFLAGS "$f" ;;
    esac
    if "$@" -c -o /tmp/$b.o 2>/tmp/$b.cerr; then
      okc=$((okc+1)); objs="$objs /tmp/$b.o"
    else
      failed="$failed $f"
    fi
  done
  echo "=== units compiled (.c: gcc, .cpp: clang): $okc / $total ==="
  if [ -n "$failed" ]; then
    echo "FAIL - unit(s) did not compile, so the link cannot see them:$failed"
    for f in $failed; do b=$(basename "$f"); b=${b%.*}; echo "   $f:"; { grep -m3 "error:" /tmp/$b.cerr || head -3 /tmp/$b.cerr; } | sed "s/^/      /"; done
    exit 1
  fi
  gcc $CFLAGS -c tools/native/runtime/arena/arena_storage.c -o /tmp/arena_storage.o
  gcc $CFLAGS -c tools/native/runtime/rt0/stubs.c           -o /tmp/stubs.o
  gcc $CFLAGS -c tools/native/runtime/rt0/native_stub.c     -o /tmp/native_stub.o
  # M1 SDK/EE utility shims + M3 ps2hw backend (no-op GS/VIF/DMA). Both are
  # TARGET_NATIVE-only strong defs that override weak M4 trap-stubs.
  sdkobjs=""
  for s in tools/native/runtime/sdk/*.c tools/native/runtime/hw/*.c; do
    [ -e "$s" ] || continue
    # no `&&`: under set -e a failing left side of `&&` would skip the backend silently
    sb=$(basename "$s" .c); gcc $CFLAGS -c "$s" -o /tmp/rt_$sb.o; sdkobjs="$sdkobjs /tmp/rt_$sb.o"
  done
  gcc -m32 -shared -Wl,--unresolved-symbols=ignore-all \
      -Wl,-T,tools/native/runtime/arena/arena.ld \
      $objs /tmp/arena_storage.o /tmp/stubs.o /tmp/native_stub.o $sdkobjs -lm \
      -o /tmp/native_full.so 2>/tmp/link.err || {
    echo "FAIL - the full native link failed; the linker said:"
    sed "s/^/   /" /tmp/link.err
    echo "   ($(grep -c "multiple definition of" /tmp/link.err || true) multiple definition(s))"
    exit 1
  }
  [ ! -s /tmp/link.err ] || { echo "linker diagnostics:"; sed "s/^/   /" /tmp/link.err; }
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

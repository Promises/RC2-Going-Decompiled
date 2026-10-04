#!/usr/bin/env bash
# verify_link.sh — M2 arena verification (real ILP32 link).
#
# Links every TARGET_NATIVE USA unit + arena_storage.c against arena.ld (with
# --unresolved-symbols=ignore-all so the still-missing funcs/libc don't abort
# the link) and asserts that NO global arena.ld PROVIDEs remains undefined —
# i.e. the arena actually resolves what it claims to. The residual undefined
# set is the other phases' work (M1 libc, M3 hardware, M4 still-asm funcs).
#
# Before linking it runs regen_arena.sh --check, which FAILS if a unit
# references a global that arena.ld neither PROVIDEs nor lists in
# arena_unresolved.txt. The link alone cannot see that: an unprovided global
# is just one more name in the residual undefined set.
#
# Units compile as check.sh compiles them: a .cpp unit as gnu++98 C++ inside
# `extern "C" { shim; unit }`. Until task #1419 this script handed .cpp units
# to gcc unwrapped with stderr discarded, so all eight dropped out of the link
# unseen; a unit that fails to compile is now a FAIL, not a smaller link.
#
# Arm: the `native-build` image (tools/native/build_image.sh) when it exists,
# else the host ($CC, default clang). Both arms compile with clang and link with
# GNU ld via -fuse-ld=bfd; the image pins clang-14 (task #1445 — its gcc/g++ 12
# rejects the .cpp units' `_Static_assert` at gnu++98). NATIVE_LINK=docker|host
# forces one. The arm used is printed.
set -eu

ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
ARENA="$ROOT/tools/native/runtime/arena"
IMG="${NATIVE_IMG:-native-build}"
CTX="${DOCKER_CONTEXT:-colima-ee-x86}"

bash "$ARENA/regen_arena.sh" --check

arm="${NATIVE_LINK:-}"
if [ -z "$arm" ]; then
  if docker --context "$CTX" image inspect "$IMG" > /dev/null 2>&1; then arm=docker; else arm=host; fi
fi

# The compile + link, run from the tree root; writes $W/still_undef.txt.
# $1 = C compiler driver, $2 = extra link flag, $3 = work dir (all relative-safe).
link_script='
  set -eu
  CC="$1"; LDX="$2"; W="$3"
  INC="going-decompiled/include"; SHIM="tools/native/mips_callees.h"
  CFLAGS="-m32 -DTARGET_NATIVE -O0 -I$INC -I tools/native -include $SHIM -ffunction-sections -fdata-sections -Wno-implicit-function-declaration -Wno-int-conversion -Wno-builtin-declaration-mismatch"
  CXXFLAGS="-x c++ -std=gnu++98 -m32 -DTARGET_NATIVE -O0 -I$INC -I tools/native -ffunction-sections -fdata-sections"
  objs=""; n=0; failed=""
  for f in $(/usr/bin/grep -rl TARGET_NATIVE going-decompiled/src/usa | sort); do
    stem=$(printf "%s" "${f#going-decompiled/src/}" | sed "s#/#__#g; s#\.cpp\$##; s#\.c\$##")
    case "$f" in
      *.cpp) printf "extern \"C\" {\n#include \"%s\"\n#include \"%s\"\n}\n" "$PWD/$SHIM" "$PWD/$f" > "$W/$stem.wrap.cpp"
             set -- $CC $CXXFLAGS "$W/$stem.wrap.cpp" ;;
      *)     set -- $CC $CFLAGS "$f" ;;
    esac
    if "$@" -c -o "$W/$stem.o" 2> "$W/$stem.err"; then objs="$objs $W/$stem.o"; n=$((n+1))
    else failed="$failed $f"; fi
  done
  if [ -n "$failed" ]; then
    echo "FAIL — unit(s) did not compile, so the link cannot see them:$failed" >&2
    for f in $failed; do s=$(printf "%s" "${f#going-decompiled/src/}" | sed "s#/#__#g; s#\.cpp\$##; s#\.c\$##"); head -5 "$W/$s.err" >&2; done
    exit 1
  fi
  $CC $CFLAGS -c tools/native/runtime/arena/arena_storage.c -o "$W/arena_storage.o"
  $CC -m32 -nostdlib -shared $LDX -Wl,--unresolved-symbols=ignore-all \
      -Wl,-T,tools/native/runtime/arena/arena.ld $objs "$W/arena_storage.o" -o "$W/native_link.so"
  nm -u "$W/native_link.so" | sed "s/^ *U //" | sort -u > "$W/still_undef.txt"
  echo "linked $n units + arena_storage.o"
'

# $W is bind-mounted into the docker arm as /out, so it must be a path the docker
# daemon can see. On the M1 that daemon runs in a colima VM that shares only
# $HOME; a macOS `mktemp -d` lands in /var/folders, so docker created /out inside
# the VM and still_undef.txt never reached the host (task #1459; #1373 fixed the
# mktemp TEMPLATE half of the same macOS-vs-colima mismatch). Keep $TMPDIR when it
# is already under $HOME, else use $HOME itself. The template form works for both
# BSD and GNU mktemp.
case "${TMPDIR:-}" in
  "$HOME"/*) WBASE="${TMPDIR%/}" ;;
  *)         WBASE="$HOME" ;;
esac
W="$(mktemp -d "$WBASE/.verify_link.XXXXXX")"
trap 'rm -rf "$W"' EXIT
case "$arm" in
  docker)
    echo "verify_link: arm docker ($IMG, context $CTX)"
    docker --context "$CTX" run --rm -v "$ROOT":/work -v "$W":/out -w /work "$IMG" \
      sh -c "$link_script" sh clang "-fuse-ld=bfd" /out ;;
  host)
    HCC="${CC:-clang}"
    echo "verify_link: arm host ($($HCC --version | head -1); GNU ld)"
    (cd "$ROOT" && sh -c "$link_script" sh "$HCC" "-fuse-ld=bfd" "$W") ;;
  *) echo "verify_link: NATIVE_LINK must be docker or host" >&2; exit 2 ;;
esac

# Placed = the names arena.ld PROVIDEs — read from the artefact the link used,
# not re-derived from its input lists.
sed -nE 's/^PROVIDE\(([A-Za-z_][A-Za-z0-9_]*) = .*/\1/p' "$ARENA/arena.ld" | sort -u > "$W/placed.txt"

# Any placed global still undefined is a FAIL (python for the set logic — comm
# collation is unreliable across these inputs).
python3 - "$W/still_undef.txt" "$W/placed.txt" <<'PY'
import sys
undef = set(l.strip() for l in open(sys.argv[1]) if l.strip())
placed = set(l.strip() for l in open(sys.argv[2]) if l.strip())
leak = sorted(placed & undef)
print("placed=%d  residual-undefined=%d  placed-leaks=%d" % (len(placed), len(undef), len(leak)))
if leak:
    print("FAIL — placed globals still undefined:")
    for n in leak[:50]:
        print("  " + n)
    sys.exit(1)
print("PASS — every placed global resolves through the arena.")
PY

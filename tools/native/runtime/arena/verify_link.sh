#!/usr/bin/env bash
# verify_link.sh — M2 arena verification (real ILP32 link on the colima image).
#
# Links every TARGET_NATIVE unit + arena_storage.c against arena.ld (with
# --unresolved-symbols=ignore-all so the still-missing funcs/libc don't abort
# the link) and asserts that NO placed data global remains undefined — i.e. the
# arena actually resolves the 362 it claims to. The residual undefined set is
# the other phases' work (M1 libc, M3 hardware, M4 still-asm funcs).
set -eu

ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
ARENA="$ROOT/tools/native/runtime/arena"
IMG="${NATIVE_IMG:-native-build}"
CTX="${DOCKER_CONTEXT:-colima-ee-x86}"

docker --context "$CTX" run --rm -v "$ROOT":/work -w /work "$IMG" sh -c '
  set -e
  CFLAGS="-m32 -DTARGET_NATIVE -O0 -I/work/going-decompiled/include -I/work/tools/native -include /work/tools/native/mips_callees.h -ffunction-sections -fdata-sections -Wno-implicit-function-declaration -Wno-int-conversion -Wno-builtin-declaration-mismatch"
  objs=""
  for f in $(grep -rl TARGET_NATIVE going-decompiled/src); do
    b=$(basename "$f" .c)
    gcc $CFLAGS -c "$f" -o /tmp/$b.o 2>/dev/null && objs="$objs /tmp/$b.o"
  done
  gcc $CFLAGS -c tools/native/runtime/arena/arena_storage.c -o /tmp/arena_storage.o
  gcc -m32 -nostdlib -shared -Wl,--unresolved-symbols=ignore-all \
      -Wl,-T,tools/native/runtime/arena/arena.ld \
      $objs /tmp/arena_storage.o -o /tmp/native_link.so 2>/dev/null
  nm -u /tmp/native_link.so | sed "s/^ *U //" | sort -u > /tmp/still_undef.txt
  cp /tmp/still_undef.txt /work/tools/native/runtime/arena/.still_undef.txt
'

# Placed = data_globals minus the generator-unresolved tail.
grep -vE '^#' "$ARENA/arena_unresolved.txt" | sort -u > /tmp/_unres.txt
sort -u "$ARENA/data_globals.txt" > /tmp/_names.txt
comm -23 /tmp/_names.txt /tmp/_unres.txt > /tmp/_placed.txt

# Any placed global still undefined is a FAIL (use python for set logic — comm
# collation is unreliable across these inputs).
python3 - "$ARENA/.still_undef.txt" /tmp/_placed.txt <<'PY'
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
rm -f "$ARENA/.still_undef.txt"

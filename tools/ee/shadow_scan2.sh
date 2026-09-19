#!/bin/bash
# shadow_scan2.sh <region> [--all-arms]  — one scan for every "name duality" a
# symbol_addrs rename leaves behind in src/<region> (task #448; FACT #7249 for
# class 1, FACT #7291 for class 2). Run from the repo root, or set ROOT=<tree>.
#
# A row is a func_ADDR that the tree still spells the OLD way while
# symbol_addrs/<region> names ADDR differently; the two spellings are held
# together only by build.sh's PROVIDE(Name = ADDR) line and the blanket
# `func_X = 0xX` grep of asm/. Output, one row per site, sorted by class:
#
#   CLASS1 <file>:<line> func_ADDR -> Name   INCLUDE_ASM(dir, func_ADDR) whose
#          leftover func_ADDR.s a populated split never rewrites (NOTE #7245 §3)
#   CLASS2 <file>:<line> func_ADDR -> Name   a C function DEFINITION the EE build
#          compiles under the old name — the object defines func_ADDR, never
#          Name (tools/ee/shadow_cdef_scan.py; --all-arms adds CLASS2X rows for
#          definitions in arms build.sh does not compile, e.g. #else of
#          #ifndef TARGET_NATIVE)
#   CLASS3 <file>:<line> alabel|glabel func_ADDR -> Name   a label INSIDE an
#          INCLUDE_ASM'd .s (not the file's own function) still under the old
#          name — an interior label splat degrades a type:func pin to
#          (EU 0x2A1928 at 8b536f0c)
#
# Failure observable: a member absent from the output. The seeded controls in
# tools/ee/shadow_scan2_selftest.sh plant one site of each class in a scratch
# copy and require exactly one new row per class.
set -u
R=${1:?region}
ALLARMS=""
[ "${2:-}" = "--all-arms" ] && ALLARMS="--all-arms"
ROOT=${ROOT:-.}
HERE=$(cd "$(dirname "$0")" && pwd)
SA="$ROOT/going-decompiled/symbol_addrs/$R/symbol_addrs.txt"
SRC="$ROOT/going-decompiled/src/$R"
[ -f "$SA" ] || { echo "no symbol_addrs at $SA" >&2; exit 2; }
[ -d "$SRC" ] || { echo "no src at $SRC" >&2; exit 2; }
T=$(mktemp)
trap 'rm -f "$T"' EXIT

# ADDR(8 hex, upper) -> Name, first definition wins (mirrors splat)
/usr/bin/sed -nE 's/^[[:space:]]*([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*=[[:space:]]*0x([0-9A-Fa-f]+)[[:space:]]*;.*/\2 \1/p' "$SA" \
 | python3 -c 'import sys
seen=set()
for l in sys.stdin:
    a,n=l.split(); a="%08X"%int(a,16)
    if a in seen: continue
    seen.add(a); print(a,n)' > "$T"

lookup() {   # lookup func_ADDR -> Name or empty; empty also when Name == func_ADDR
  local a; a=$(echo "${1#func_}" | tr a-f A-F)
  local n; n=$(/usr/bin/grep -E "^$a " "$T" | awk '{print $2}')
  [ -n "$n" ] && [ "$n" != "$1" ] && echo "$n"
}

{
# ---- CLASS1: INCLUDE_ASM arguments (shadow_scan.sh, NOTE #7245 §7) ----
/usr/bin/grep -rnoE 'INCLUDE_ASM\("[^"]+",[[:space:]]*func_[0-9A-Fa-f]{8}' "$SRC" \
 | /usr/bin/sed -E 's/^([^:]+:[0-9]+):INCLUDE_ASM\("([^"]+)",[[:space:]]*(func_[0-9A-Fa-f]{8})/\1 \2 \3/' \
 | while read -r loc dir old; do
     new=$(lookup "$old"); [ -n "$new" ] && echo "CLASS1 ${loc#$ROOT/} $old -> $new"
   done

# ---- CLASS2: compiled C definitions (preprocessor-aware) ----
python3 "$HERE/shadow_cdef_scan.py" "$ROOT" "$R" $ALLARMS

# ---- CLASS3: interior labels inside INCLUDE_ASM'd .s files ----
/usr/bin/grep -rhoE 'INCLUDE_ASM\("[^"]+",[[:space:]]*[A-Za-z_][A-Za-z0-9_]*' "$SRC" \
 | /usr/bin/sed -E 's/INCLUDE_ASM\("([^"]+)",[[:space:]]*([A-Za-z0-9_]+)/\1\/\2.s/' | sort -u \
 | while read -r f; do
     p="$ROOT/$f"; [ -f "$p" ] || continue
     fn=$(basename "$f" .s)
     /usr/bin/grep -nE '^[[:space:]]*(glabel|alabel|jlabel|dlabel)[[:space:]]+func_[0-9A-Fa-f]{8}[[:space:]]*$' "$p" \
      | while IFS=: read -r ln rest; do
          set -- $rest; kind=$1; sym=$2
          [ "$sym" = "$fn" ] && continue
          new=$(lookup "$sym"); [ -n "$new" ] && echo "CLASS3 $f:$ln $kind $sym -> $new"
        done
   done
} | sort -k1,1 -k2,2

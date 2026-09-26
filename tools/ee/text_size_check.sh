#!/usr/bin/env bash
# text_size_check.sh — whole-unit layout check a per-function gate cannot make
# (FACT #7982).
#
#   tools/ee/text_size_check.sh <region> <unit>
#   e.g. tools/ee/text_size_check.sh usa text/191238
#
# Run AFTER tools/ee/objdiff_build.sh <region> <unit>. Compares the sdk29 BASE
# object (build/<region>/obj/<unit>.o) against the TARGET object
# (build/<region>/expected/<unit>.o):
#   1. .text section size;
#   2. every function/object symbol present in both, joined by NAME: its
#      offset within .text.
# A promotion that drops words splat left after a function's `endlabel` makes
# every later function land low. The unit objdiff report, verify_match_unit.sh
# and per-function compares all score each function in isolation and stay
# green; this check does not.
#
# Exit 0 = sizes equal and 0 symbols moved; 1 = mismatch (details printed);
# 2 = an object is missing (not a pass). Only the sdk29 arm is checked: that is
# the arm build.sh links into the image (MATCH_-guarded functions keep their
# INCLUDE_ASM there).
set -u
region=${1:?region}; unit=${2:?unit}
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
base="going-decompiled/build/$region/obj/$unit.o"
exp="going-decompiled/build/$region/expected/$unit.o"
for f in "$base" "$exp"; do
    [ -f "$ROOT/$f" ] || { echo "text_size_check: MISSING $f"; exit 2; }
done
echo "text_size_check: base mtime $(stat -f %Sm "$ROOT/$base" 2>/dev/null || stat -c %y "$ROOT/$base")"
out=$("$ROOT/tools/ee/vm.sh" "
  for o in $base $exp; do
    mips-linux-gnu-objdump -h \$o | awk -v o=\$o '\$2==\".text\"{print \"SIZE\", o, \$3}'
    mips-linux-gnu-nm \$o | awk -v o=\$o '\$2 ~ /^[TtDd]\$/ && \$3 !~ /NON_MATCHING/ {print \"SYM\", o, \$3, \$1}'
  done")
printf '%s\n' "$out" | awk -v B="$base" -v E="$exp" '
  $1=="SIZE" { sz[$2]=$3 }
  $1=="SYM" && $2==B { b[$3]=$4 }
  $1=="SYM" && $2==E { e[$3]=$4 }
  END {
    if (!(B in sz) || !(E in sz)) { print "text_size_check: no .text size read"; exit 2 }
    joined=0; moved=0
    for (n in e) if (n in b) { joined++; if (b[n]!=e[n]) { moved++; if (moved<=5) printf "  moved %s base %s expected %s\n", n, b[n], e[n] } }
    printf "text_size_check: .text base 0x%s expected 0x%s; joined %d, moved %d\n", sz[B], sz[E], joined, moved
    if (joined==0) { print "text_size_check: 0 symbols joined -- nothing compared"; exit 2 }
    exit (sz[B]!=sz[E] || moved>0) ? 1 : 0
  }'

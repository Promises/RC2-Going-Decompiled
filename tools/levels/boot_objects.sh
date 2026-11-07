#!/bin/bash
# BOOT OBJECTS for level placement (task #2516, RULING #13219 condition 1,
# NOTE #13218 option (c)): the boot build's OWN final unit asm, every compiled C
# function in its own .text.<fn> section, assembled once per tree.
#
#   bash tools/levels/boot_objects.sh            # build if stale, else reuse
#   bash tools/levels/boot_objects.sh --check    # freshness only; exit 1 if stale
#
# Output (gitignored): going-decompiled/build/usa_levels/boot_objs/
#   <unit>.s / <unit>.o    sectioned unit asm and its object (one per boot unit)
#   units.tsv sections.tsv refused.tsv   (tools/levels/boot_sections.py)
#   stamp.txt              the tree these objects were built from (see below)
#   boot_objects.log       the container run (asm_unit.sh output per unit)
#
# FRESHNESS. The boot build's record of what it built is the landing gate's
# tools/ee/.gate_landing/usa/built_tree.txt (`work_tree=<tree hash>` of the
# whole working tree, .gitignore honoured). It must equal the tree hash of this
# worktree NOW (the same `git add -A` into a scratch index + write-tree as
# landing_gate.sh's worktree_hash; on a clean tree that is HEAD^{tree}). If it
# does not, this script runs the boot build first, i.e. the landing gate's
# BUILDING form `bash tools/ee/landing_gate.sh usa`, and requires its
# `OK   cmp: 0 differing bytes` row in THIS run's log and a record that now
# matches. Then the objects are reused only if stamp.txt names the same tree
# and the same sha1 for every <unit>._u.s; otherwise they are rebuilt.
#
# BUILD, in the ee-build container (same docker context as the gates):
#   1. python3 -I tools/levels/boot_sections.py: <unit>._u.s -> sectioned
#      <unit>.s (boot sources are never compiled a second time);
#   2. sh tools/ee/asm_unit.sh usa <unit>.s <unit>.o <-G> (read-only; -G is the
#      unit's sdk29 -G from the gate's cc1_args.tsv), then strip as build.sh does;
#   3. mips-linux-gnu-objcopy --set-section-alignment .text.<fn>=<2^N> from each
#      block's own `.align N`: GNU as records 16 for every text section (FACT
#      #13214: 490 of the 1,000 L24 places are not 16-aligned).
# Prints the wall time of the boot-object step with host and docker context,
# separately from any boot build it had to run.
#
# Paths: reads tools/ee/.gate_landing/usa/{built_tree.txt,cc1_args.tsv} and the
# boot build dir; runs tools/ee/asm_unit.sh and (if stale) landing_gate.sh,
# both read-only; writes only going-decompiled/build/usa_levels/boot_objs/.
set -u
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 2
CTX=${EE_DOCKER_CONTEXT:-colima-ee-x86}
OUT=going-decompiled/build/usa_levels/boot_objs
REC=tools/ee/.gate_landing/usa/built_tree.txt
ARGS=tools/ee/.gate_landing/usa/cc1_args.tsv
BB=going-decompiled/build/usa
USDIR=$BB/going-decompiled/src/usa/text

now() { date +%s.%N; }
secs() { awk -v a="$1" -v b="$2" 'BEGIN{printf "%.2f", b-a}'; }
worktree_hash() {
  local idx; idx=$(mktemp); cp "$(git rev-parse --git-path index)" "$idx"
  GIT_INDEX_FILE="$idx" git add -A >/dev/null 2>&1
  GIT_INDEX_FILE="$idx" git write-tree; rm -f "$idx"
}
rec_tree() { [ -f "$REC" ] && sed -n 's/^work_tree=//p' "$REC" | head -1; }
us_sums() { (cd "$USDIR" && sha1sum *._u.s 2>/dev/null); }

CHECK=0; [ "${1:-}" = "--check" ] && CHECK=1
TREE=$(worktree_hash)
R=$(rec_tree)
if [ "$R" = "$TREE" ]; then
  echo "boot_objects: FRESH boot build: $REC work_tree $R == this worktree's tree $TREE"
else
  echo "boot_objects: STALE boot build: $REC work_tree '${R:-none}' != this worktree's tree $TREE"
  [ $CHECK = 1 ] && { echo "#### boot_objects: STALE"; exit 1; }
  echo "boot_objects: running the boot build first: bash tools/ee/landing_gate.sh usa (building form)"
  t=$(now); LG=going-decompiled/build/usa_levels/boot_build_gate.log
  mkdir -p going-decompiled/build/usa_levels
  bash tools/ee/landing_gate.sh usa > "$LG" 2>&1; lrc=$?
  echo "boot_objects: boot build + landing gate rc $lrc in $(secs "$t" "$(now)") s (host $(uname -n), docker context $CTX); log $LG"
  /usr/bin/grep -a -E '^(OK|FAIL) +(cmp|sha1|e_entry)' "$LG" | sed 's/^/   /'
  /usr/bin/grep -a -q -E '^OK +cmp: 0 differing bytes' "$LG" \
    || { echo "boot_objects: the boot build's cmp row is not 0 in $LG"; echo "#### boot_objects: FAIL (boot build)"; exit 1; }
  R=$(rec_tree)
  [ "$R" = "$TREE" ] || { echo "boot_objects: after the boot build $REC work_tree '${R:-none}' != $TREE"; echo "#### boot_objects: FAIL (record)"; exit 1; }
  echo "boot_objects: FRESH boot build now: $REC work_tree $R"
fi

WANT=$(printf 'tree=%s\n%s\n' "$TREE" "$(us_sums)")
if [ -f "$OUT/stamp.txt" ] && [ "$(cat "$OUT/stamp.txt")" = "$WANT" ] && [ -s "$OUT/sections.tsv" ]; then
  echo "boot_objects: REUSED $OUT: stamp.txt names tree $TREE and the same $(us_sums | wc -l) _u.s sha1s"
  echo "#### boot_objects: PASS (reused)"
  exit 0
fi
[ $CHECK = 1 ] && { echo "boot_objects: $OUT/stamp.txt does not match the tree and _u.s sha1s"; echo "#### boot_objects: STALE"; exit 1; }

rm -rf "$OUT"; mkdir -p "$OUT"
t=$(now)
docker --context "$CTX" run --rm --user="$(id -u):$(id -g)" -e HOME=/tmp -v "$ROOT":/work -w /work ee-build sh -c '
set -u
OUT=$1; BB=$2; ARGS=$3
python3 -I tools/levels/boot_sections.py "$BB" "$ARGS" "$OUT" || exit 1
n=0; f=0
while IFS="$(printf "\t")" read -r unit g src; do
  if sh tools/ee/asm_unit.sh usa "/work/$OUT/$unit.s" "/work/$OUT/$unit.o" "$g" > "$OUT/$unit.aslog" 2>&1 && [ -s "$OUT/$unit.o" ]; then
    mips-linux-gnu-strip "$OUT/$unit.o" -N dummy-symbol-name 2>/dev/null || true
    args=$(awk -F "\t" -v u="$unit" "\$1==u {printf \" --set-section-alignment .text.%s=%s\", \$2, \$3}" "$OUT/sections.tsv")
    mips-linux-gnu-objcopy $args "$OUT/$unit.o" || { echo "OBJCOPY FAIL $unit"; f=$((f+1)); continue; }
    echo "   $unit: asm_unit.sh $g ok, $(awk -F "\t" -v u="$unit" "\$1==u" "$OUT/sections.tsv" | wc -l) .text.<fn> sections aligned, sha1 $(sha1sum "$OUT/$unit.o" | cut -c1-40)"
    n=$((n+1))
  else
    echo "AS FAIL $unit:"; tail -5 "$OUT/$unit.aslog"; f=$((f+1))
  fi
  w=$(/usr/bin/grep -c -E "WARNING|REFUSED|FAIL" "$OUT/$unit.aslog" || true)
  [ "${w:-0}" = 0 ] || { echo "   $unit: asm_unit.sh printed $w WARNING/REFUSED/FAIL lines:"; /usr/bin/grep -E "WARNING|REFUSED|FAIL" "$OUT/$unit.aslog" | sed "s/^/     /"; f=$((f+1)); }
done < "$OUT/units.tsv"
echo "boot_objects: assembled $n units, $f failed"
[ $f = 0 ]
' sh "$OUT" "$BB" "$ARGS" > "$OUT/boot_objects.log" 2>&1; rc=$?
cat "$OUT/boot_objects.log"
echo "boot_objects: wall $(secs "$t" "$(now)") s for the boot-object step (sectionize + asm_unit.sh x $(wc -l < "$OUT/units.tsv" 2>/dev/null || echo 0) + align) (host $(uname -n), docker context $CTX -> $(docker context inspect "$CTX" --format '{{.Endpoints.docker.Host}}' 2>/dev/null), rc $rc)"
[ $rc = 0 ] || { echo "#### boot_objects: FAIL"; exit 1; }
printf '%s\n' "$WANT" > "$OUT/stamp.txt"
echo "#### boot_objects: PASS (built)"

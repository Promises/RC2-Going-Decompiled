#!/usr/bin/env bash
# regen_arena.sh — derive the arena population and regenerate the arena (M2).
#
# data_globals.txt is gen_arena.py's INPUT: the native data globals the arena
# must place. It is DERIVED from linkgap.sh's runtime-global bucket (globals
# referenced by some TARGET_NATIVE unit and defined by none). Until task #1419
# it was a hand-committed snapshot from 2026-06-19, so globals referenced by
# later #else bodies (all eight .cpp units included) were never placed and
# never flagged: arena.ld said "0 unresolved" about an input that had not seen
# them.
#
#   regen_arena.sh          refresh data_globals.txt, then regenerate arena.ld,
#                           arena_map.txt, arena_storage.c, arena_unresolved.txt
#   regen_arena.sh --check  write nothing; FAIL (rc 1) if any global a native
#                           unit references is neither PROVIDEd by the committed
#                           arena.ld nor listed in arena_unresolved.txt, or if
#                           regenerating would change any committed artefact
#
# The refresh is ADD-ONLY: data_globals.txt = (its current members ∪ the live
# bucket) − data_const.txt. Names are never pruned automatically, because the
# arena is also bound by consumers linkgap does not scan (EU units,
# tools/native/runtime/sdk, tools/ee/eetest/cmp tests read arena_map.txt).
# Prune by hand, naming the members, if a name is truly dead.
set -u

ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
ARENA="$ROOT/tools/native/runtime/arena"
SYMS="$ROOT/going-decompiled/symbol_addrs/usa/symbol_addrs.txt"
check=0
case "${1:-}" in
  --check) check=1 ;;
  "") ;;
  *) echo "usage: regen_arena.sh [--check]" >&2; exit 2 ;;
esac

T="$(mktemp -d)"
trap 'rm -rf "$T"' EXIT

# 1. The live population. linkgap must compile every unit: a unit that fails
#    contributes no references, which is exactly how this gap hid, so a WARN
#    from it is fatal here rather than a smaller list.
bash "$ROOT/tools/native/linkgap.sh" "$T/linkgap.txt" > /dev/null 2> "$T/linkgap.err"
rc=$?
if [ "$rc" != 0 ] || [ -s "$T/linkgap.err" ]; then
  echo "regen_arena: FAIL — linkgap.sh rc $rc:" >&2
  cat "$T/linkgap.err" >&2
  exit 1
fi
awk '/^--- global ---$/ {f=1; next} /^---/ {f=0} f && NF {print $1}' "$T/linkgap.txt" \
  | LC_ALL=C sort -u > "$T/live.txt"
if [ ! -s "$T/live.txt" ]; then
  echo "regen_arena: FAIL — linkgap.sh reported an empty runtime-global bucket" >&2
  exit 1
fi
names() { awk '{sub(/#.*/, "")} NF {print $1}' "$@" | LC_ALL=C sort -u; }

# 2. The add-only population and a fresh generation from it.
names "$ARENA/data_const.txt" > "$T/const.txt"
names "$ARENA/data_globals.txt" "$T/live.txt" | LC_ALL=C comm -23 - "$T/const.txt" > "$T/data_globals.txt"
# gen_arena.py reads data_const.txt from beside its names file.
cp "$ARENA/data_const.txt" "$T/data_const.txt"
python3 "$ARENA/gen_arena.py" "$T/data_globals.txt" "$SYMS" "$T/out" > "$T/gen.log" || {
  echo "regen_arena: FAIL — gen_arena.py:" >&2; cat "$T/gen.log" >&2; exit 1; }

if [ "$check" = 0 ]; then
  cp "$T/data_globals.txt" "$ARENA/data_globals.txt"
  for f in arena.ld arena_map.txt arena_storage.c arena_unresolved.txt; do
    cp "$T/out/$f" "$ARENA/$f"
  done
  cat "$T/gen.log"
  echo "regen_arena: live bucket $(wc -l < "$T/live.txt" | tr -d ' '), population $(wc -l < "$T/data_globals.txt" | tr -d ' ') (+ const $(wc -l < "$T/const.txt" | tr -d ' '))"
  exit 0
fi

# 3. --check. The predicate is read from the COMMITTED artefacts, not from the
#    population that produced them, so a stale data_globals.txt cannot vouch
#    for itself.
sed -nE 's/^PROVIDE\(([A-Za-z_][A-Za-z0-9_]*) = .*/\1/p' "$ARENA/arena.ld" | LC_ALL=C sort -u > "$T/placed.txt"
names "$ARENA/arena_unresolved.txt" > "$T/unresolved.txt"
LC_ALL=C sort -u "$T/placed.txt" "$T/unresolved.txt" > "$T/accounted.txt"
LC_ALL=C comm -23 "$T/live.txt" "$T/accounted.txt" > "$T/gap.txt"
fail=0
echo "regen_arena --check: live globals $(wc -l < "$T/live.txt" | tr -d ' '), PROVIDEd $(wc -l < "$T/placed.txt" | tr -d ' '), unresolved $(wc -l < "$T/unresolved.txt" | tr -d ' '), neither $(wc -l < "$T/gap.txt" | tr -d ' ')"
if [ -s "$T/gap.txt" ]; then
  echo "FAIL — referenced by a native unit, neither PROVIDEd nor in arena_unresolved.txt:"
  sed 's/^/  /' "$T/gap.txt"
  fail=1
fi
for f in data_globals.txt:"$T/data_globals.txt" arena.ld:"$T/out/arena.ld" \
         arena_map.txt:"$T/out/arena_map.txt" arena_storage.c:"$T/out/arena_storage.c" \
         arena_unresolved.txt:"$T/out/arena_unresolved.txt"; do
  name="${f%%:*}"; fresh="${f#*:}"
  if ! cmp -s "$ARENA/$name" "$fresh"; then
    echo "FAIL — $name is stale (regenerate with tools/native/runtime/arena/regen_arena.sh):"
    diff "$ARENA/$name" "$fresh" | /usr/bin/grep -E '^[<>]' | head -20 | sed 's/^/  /'
    fail=1
  fi
done
[ "$fail" = 0 ] && echo "PASS — every referenced native global is PROVIDEd or listed unresolved; artefacts are current."
exit "$fail"

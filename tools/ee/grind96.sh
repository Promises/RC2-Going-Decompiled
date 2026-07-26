#!/usr/bin/env bash
# grind96.sh — engine-2.96 per-function grind WITH the mandatory RAW byte+reloc gate.
#
#   tools/ee/grind96.sh <region> <unit> <func> <cfile>
#
# WHY: diff96.sh only prints objdiff FUZZY match% (normalizes relocs -> the banked
# [[feedback_fuzzy_pct_not_byte_exact]] false-100% trap), and its base.o is the WHOLE
# unit (all siblings INCLUDE_ASM), so it can't be handed to verify_match.sh directly.
# This wrapper runs diff96.sh (fuzzy screen), then extracts JUST the compiled target
# function from the post-passed base.s, applies the assemble-time fixups (move_fixup +
# vu0_fixup — the move->daddu / break-7->break-0,7 rules asm_unit.sh applies at assemble
# time and which diff96's PRE-assemble base.s therefore lacks), assembles a single-
# function base.o, and runs the RAW verify_match.sh gate against diff96's target.o.
#
# Exit 0 + "BYTE+RELOC IDENTICAL" = a CONFIRMED engine-2.96 match. Anything else is NOT.
#
# EXIT BANDS (fleet convention; the LAST line of this script is the verify_match_unit
# call, so that tool's rc BECOMES this script's rc — the bands must be the same set):
#   0  MATCH        — byte+reloc identical to the ROM
#   1  DIFFERS      — a real byte difference
#   2  COULD-NOT-LOOK — the tool cannot decide: unverifiable (missing ROM, unmodelled
#                     reloc, unknown symbol) OR the anti-vacuous guard tripped (FUNC is
#                     not compiled, so a slice would be VACUOUS). Not a pass, not a fail.
#   3  USAGE/ARG    — you invoked me wrong (bad arg count, wrong object shape, unknown
#                     region). A misuse must never look like a verdict.
set -euo pipefail
REGION="$1"; UNIT="$2"; FUNC="$3"; CFILE="$4"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"
W="${GRIND_SCRATCH:-tools/ee/.diff96}"   # honor isolated scratch (must match diff96.sh)
ASF='-march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -Igoing-decompiled/build/'"$REGION"'/include'

# NOTE: ASMFIX_SHARED is deliberately left UNSET (the tester's GATE_HAZARDS.md
# documented SAFE mode) — asm_unit.sh then derives a worktree-isolated FIXROOT under
# .diff96, so no cross-worktree scratch collision / false-PASS path is open. diff96.sh
# does not forward -e ASMFIX_SHARED into its container anyway, so a host share is inert.
echo "=== diff96 fuzzy screen ==="
bash tools/ee/diff96.sh "$REGION" "$UNIT" "$FUNC" "$CFILE"

# ANTI-VACUOUS GUARD (tester, authoritative gate): verify_match_unit.sh slices FUNC out
# of the whole-unit base.o — but if FUNC was NOT actually compiled to engine C (no
# MATCH_<func> arm, so it stayed INCLUDE_ASM), base.o holds the ORIGINAL asm bytes and
# the slice compares original-vs-original -> a VACUOUS "BYTE+RELOC IDENTICAL". Confirmed
# on func_002AB210 (INCLUDE_ASM, no MATCH arm): diff96 says 0% but the raw slice falsely
# read IDENTICAL. So REQUIRE the cc1 output (base.s) to actually contain the compiled
# function before trusting the raw verdict.
if ! grep -qE "^[[:space:]]*\.(ent|globl)[[:space:]]+$FUNC([[:space:]]|\$)" "$W/base.s"; then
  echo "RAW GATE: '$FUNC' is NOT compiled in base.s (still INCLUDE_ASM / no MATCH_$FUNC arm)." >&2
  echo "  -> cannot verify: a slice would be VACUOUS (original-vs-original). Add a MATCH_$FUNC arm." >&2
  # Band 2, NOT 3: "the slice would be VACUOUS" is a COULD-NOT-LOOK, not a distinct
  # verdict — there is nothing to compare, so the tool cannot decide. Band 3 is
  # reserved for USAGE/ARG ("you invoked me wrong"). This matters because line 46 is
  # the last line: verify_match_unit's rc BECOMES grind96's rc, so a bare 3 here would
  # collapse "this function cannot be verified" onto "you invoked me wrong".
  exit 2
fi

# RAW gate: scope FUNC out of diff96's whole-unit base.o (already built through
# asm_unit.sh's full SN-parity fixup pipeline) and compare to the single-function
# target.o. No re-assembly — base.o is authoritative. (Anti-false-NEGATIVE.)
echo "=== RAW byte+reloc gate ==="
bash tools/ee/verify_match_unit.sh "$FUNC" "$W/base.o" "$W/target.o"

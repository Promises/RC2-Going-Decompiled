#!/bin/bash
# gmodel_scan.sh — the -G model screen (task #919, FACT #8246): a unit the
# build compiles at -G0 must not have C-compiled functions whose ROM words are
# %gp_rel.
#
#   bash tools/ee/gmodel_scan.sh <region> [--root TREE] [--flags UNIT_FLAGS_SH]
#
# WHY: cc1 emits a bare `lw/sw $r,sym` macro and the assembler turns it into a
# one-word %gp_rel access only at a nonzero -G. At -G0 a ROM %gp_rel word
# cannot come from C, so a -G0 unit whose compiled function carries one is a
# MODEL mismatch: it can reach the ROM's bytes only by something that is not
# the compiler (an explicit `%gp_rel` in inline asm, an assembler-emulation
# post-pass). GP differs by region (0x1AEFF0 USA vs 0x1AF070 EU), so such a
# body can match one region by luck and not the other. 1FCF48 and 183558 sat
# in that state until #889 moved them to -G8 — 1FCF48 because it had no case
# line at all and defaulted to -G0, 183558 because a comment pinned it there.
#
# The -G of a unit is taken from unit_flags.sh itself (sourced, not parsed), so
# this reads what the build uses; flagdiff.py (the gate's FLAGS row) holds
# build.sh / objdiff_build.sh / diff.sh to the same table.
#
# A "gp word" is read from the ROM WORD, not from splat's text: every line of a
# nonmatchings .s carries `/* <off> <vaddr> <hex> */`, the hex being the word's
# bytes in memory order on a disassembled line; on a raw `.word 0x...` line the
# operand (the value that is assembled) is decoded instead. A gp word is an I-type with base rs == $28 whose opcode is
# addiu/daddiu or a load/store (lq/sq, lb..sd, lwc1/swc1, ...). Why not grep
# `%gp_rel`: splat can print a function as raw `.word`s, and there the
# annotation is absent — at a709057e USA func_00131DE8 (snd_Init, 8 gp words,
# cod/022FA8) and func_001325E8 (snd_BankLoadFromEE_CB, 6, cod/0321A0) read 0 by
# grep; since task #1255 both are emitted decoded under their symbol_addrs names. Measured at a709057e: the
# word count equals the %gp_rel line count in every other file (USA 2719 of
# 2721, EU 2166 of 2166), so the decode is the annotation plus those two.
#
# Per C unit going-decompiled/src/<region>/<unit>.c, over the ROM-derived
# going-decompiled/asm/<region>/nonmatchings/<unit>/<fn>.s:
#   UNIT     <unit> <G> <n>     every unit, n = gp words across its .s (the -G8
#                                units' nonzero counts show the screen can see
#                                gp words at all)
#   MISMATCH <unit> <G> <fn> <n> -G0 unit, <fn>.s carries n gp words, and the .c
#                                has NO INCLUDE_ASM line naming <fn>: the image
#                                compiles it from C  -> exit 1
#   LATENT   <unit> <G> <fn> <n> the same, but <fn> is still INCLUDE_ASM: the ROM
#                                words assemble as given at any -G, so the image
#                                is unaffected; promoting <fn> at this -G would
#                                be a MISMATCH. Listed, never a failure.
# Exit 0 no MISMATCH / 1 at least one / 2 could not run.
#
# "Image compiles <fn> from C" is read as: no line of the .c that STARTS with
# INCLUDE_ASM("...", <fn>) — comment lines never start that way. build.sh
# defines no MATCH_* macro, so a MATCH_-guarded INCLUDE_ASM is the image arm.
# Blind to: an INCLUDE_ASM inside `#if 0`; a gp access through a register
# other than $28 (a copy of gp); a promoted function whose .s is absent under
# both its old and new name (the SHADOW row's NOTARGET).
set -u
REGION="${1:-}"; shift || true
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TREE="$ROOT"; FLAGS="$ROOT/tools/ee/unit_flags.sh"
while [ $# -gt 0 ]; do
  case "$1" in
    --root)  TREE="$2"; shift 2 ;;
    --flags) FLAGS="$2"; shift 2 ;;
    *) echo "gmodel_scan: unknown option $1" >&2; exit 2 ;;
  esac
done
case "$REGION" in usa|eu) ;; *) echo "usage: gmodel_scan.sh <usa|eu> [--root TREE] [--flags FILE]" >&2; exit 2 ;; esac
SRC="$TREE/going-decompiled/src/$REGION"; ASM="$TREE/going-decompiled/asm/$REGION/nonmatchings"
[ -d "$SRC" ] && [ -d "$ASM" ] && [ -f "$FLAGS" ] || { echo "gmodel_scan: missing $SRC, $ASM or $FLAGS" >&2; exit 2; }
. "$FLAGS"
type unit_flags >/dev/null 2>&1 || { echo "gmodel_scan: $FLAGS defines no unit_flags" >&2; exit 2; }

# gp_words FILE — the number of gp words (see header) in one nonmatchings .s.
gp_words() {
  awk '
    function hexv(h,   i, v) { v = 0; h = tolower(h); for (i = 1; i <= length(h); i++) v = v * 16 + index("0123456789abcdef", substr(h, i, 1)) - 1; return v }
    match($0, /\/\* [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ \*\//) {
      split(substr($0, RSTART, RLENGTH), g, " "); b = g[4]
      if (length(b) != 8) next
      if (match($0, /\.word[ \t]+0x[0-9A-Fa-f]+/)) { b = substr($0, RSTART, RLENGTH); sub(/.*0x/, "", b); w = hexv(b) }
      else w = hexv(substr(b, 7, 2) substr(b, 5, 2) substr(b, 3, 2) substr(b, 1, 2))
      op = int(w / 67108864); rs = int(w / 2097152) % 32
      # addiu 0x09, daddiu 0x19, lq 0x1e, sq 0x1f, 0x20-0x3f loads/stores
      # except cache 0x2f, pref 0x33 and the unused 0x3b
      if (rs == 28 && (op == 9 || op == 25 || op == 30 || op == 31 || (op >= 32 && op != 47 && op != 51 && op != 59))) n++
    }
    END { print n + 0 }' "$1"
}

nunits=0; bad=0
# .c and .cpp units both (task #1258): a .cpp unit dropped here would leave
# the screen silently, with no row to show it was ever scanned.
for c in $(/usr/bin/find "$SRC" \( -name '*.c' -o -name '*.cpp' \) | LC_ALL=C sort); do
  unit="${c#"$SRC"/}"; case "$unit" in *.cpp) unit="${unit%.cpp}" ;; *) unit="${unit%.c}" ;; esac
  unit_flags "$c"
  nunits=$((nunits+1))
  d="$ASM/$unit"; total=0
  if [ -d "$d" ]; then
    for s in $(/usr/bin/find "$d" -maxdepth 1 -name '*.s' | LC_ALL=C sort); do
      n=$(gp_words "$s")
      [ "$n" -gt 0 ] || continue
      total=$((total+n))
      [ "$GFLAG" = "-G0" ] || continue
      fn=$(basename "$s" .s)
      if /usr/bin/grep -qE "^[[:space:]]*INCLUDE_ASM\(\"[^\"]*\",[[:space:]]*$fn[[:space:]]*\)" "$c"; then
        echo "LATENT $unit $GFLAG $fn $n"
      else
        echo "MISMATCH $unit $GFLAG $fn $n"; bad=$((bad+1))
      fi
    done
  fi
  echo "UNIT $unit $GFLAG $total"
done
[ "$nunits" -gt 0 ] || { echo "gmodel_scan: no .c or .cpp under $SRC" >&2; exit 2; }
[ "$bad" = 0 ]

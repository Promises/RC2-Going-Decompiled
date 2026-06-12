#!/bin/sh
# asm_unit.sh — assemble a whole compiled unit (cc1 output .s that pulls in
# per-function asm via INCLUDE_ASM's `.include "going-decompiled/asm/.../F.s"`)
# into a single object, applying the VU0 fixup to the included asm.
#
# Runs INSIDE the ee-build container (colima x86 VM):
#   docker --context colima-ee-x86 run --rm -v "$PWD":/work ee-build \
#       sh tools/ee/asm_unit.sh <region> <unit.s> <out.o>
#   e.g. ... sh tools/ee/asm_unit.sh usa /work/build/cod_015180.s out.o
#
# WHY the mirror+cd dance:
#   INCLUDE_ASM hardcodes a *source-relative* include path
#   (`going-decompiled/asm/<region>/nonmatchings/<unit>/<func>.s`). GNU as
#   resolves a relative `.include` against the CWD before any -I dir, so we
#   cannot redirect it with -I alone. Instead we build a filtered MIRROR of the
#   asm tree (each .s passed through tools/ee/vu0_fixup.sed, which only rewrites
#   VU0 Q/ACC operands and is a no-op everywhere else) plus macro.inc, then run
#   `as` with its CWD at the mirror root so every `.include` resolves to the
#   fixed-up copy. Encoding is byte-identical to the original (the fixup only
#   adds the `$` prefix GNU as requires on the Q/ACC special registers).
set -e
REGION="$1"; UNIT_S="$2"; OUT_O="$3"; GFLAG="${4:--G0}"
# GFLAG: optional -G<N> for the assembler (default -G0). cc1 references small
# externs by plain name + `.extern sym,size`, and GNU as decides gp-relativity
# from its own -G threshold - so a base unit compiled at -G8 (cod/0321A0) must
# also be ASSEMBLED at -G8 or the gp_rel accesses macro-expand to lui/lw.
# Explicit %gp_rel/%hi/%lo in the included original asm is unaffected by -G.
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
VU0FIX="$ROOT/tools/ee/vu0_fixup.sed"
MOVEFIX="$ROOT/tools/ee/move_fixup.sed"   # cc1 `move` pseudo -> `daddu` (0x2d) for EE
ASMSRC="$ROOT/going-decompiled/asm/$REGION/nonmatchings"
MACINC="$ROOT/going-decompiled/build/$REGION/include/macro.inc"

# Build the filtered mirror in a scratch dir next to the output object.
# Suffixed with the object name so concurrent unit builds that share an output
# dir (e.g. expected/cod/015180.o and expected/cod/0321A0.o) cannot race on one
# mirror (one run's rm -rf would yank the tree out from under the other's
# mkdir/sed, failing with ENOENT).
FIXROOT="$(dirname "$OUT_O")/.asmfix-$REGION-$(basename "$OUT_O" .o)"
rm -rf "$FIXROOT"
mkdir -p "$FIXROOT/going-decompiled/asm/$REGION/nonmatchings" "$FIXROOT/include"
# Mirror every nonmatching .s through the VU0 fixup, preserving subdirs.
find "$ASMSRC" -name '*.s' | while read -r s; do
  rel="${s#"$ROOT"/}"
  mkdir -p "$FIXROOT/$(dirname "$rel")"
  sed -f "$VU0FIX" "$s" > "$FIXROOT/$rel"
done
cp "$MACINC" "$FIXROOT/include/macro.inc"

# Assemble with CWD at the mirror so source-relative `.include`s resolve there.
cd "$FIXROOT"
# Apply the cc1 `move`->`daddu` fixup to the (cc1-emitted) unit asm before
# assembling. The .include'd original asm is read from the mirror by `as` and is
# untouched (it has explicit `daddu`, never the `move` pseudo).
#
# At -G8 also fix the `la` pseudo (bare `la $r,SYM` and the `la $r,SYM+OFF`
# form cc1 emits for address-plus-constant, e.g. text/198FA0 func_0029C418).
# The SN ee-as expands both with 32-bit adds (proven by the original bytes):
# `addiu $r,$gp,%gp_rel(SYM)` for a small-data symbol, `lui $r,%hi(SYM);
# addiu $r,$r,%lo(SYM)` for an absolute one. GNU as uses the 64-bit `daddiu`
# in both cases, so we expand the pseudo ourselves, deciding smallness exactly
# like the assembler does - from the `.extern SYM, SIZE` directives in the
# same unit .s, keyed on the BASE symbol for the +OFF form (first directive
# wins, matching observed GAS behaviour; a file-scope __asm__(".extern SYM,
# 16") in the C overrides cc1's own size, which is how a cc1-small but
# assembler-absolute original symbol is reproduced). Not applied at -G0,
# where cc1 never relies on gp-relative `la`.
#
# Also reproduce the SN ee-as COP1 load-delay flush (proven by the original
# bytes in text/183178): when a `.set noreorder` region begins directly after
# a cop1 load macro (`l.s`/`lwc1`), the SN assembler conservatively pads the
# load delay with a `nop` before entering the region (it can no longer reorder
# inside it). GNU as treats r5900 cop1 loads as interlocked and emits nothing,
# so we insert the nop ourselves. Not applied at -G0: no currently-matched
# -G0 function has a cop1-load/noreorder boundary, and the -G0 units' matches
# were proven WITHOUT the pad.
#
# At -G8 also expand the `li.s` float-constant pseudo when its IEEE bits need
# a 2-insn materialisation (low half nonzero). The SN ee-as always expands
# `li.s $fN,<c>` inline as `lui $at,hi[; ori $at,$at,lo]; mtc1 $at,$fN`
# (proven by the original bytes in text/1907F0 func_00290EF8); GNU as does
# the same at -G0 (which is why the -G0 units never hit this) and for
# lui-only constants at any -G, but with a nonzero -G it places an
# ori-needing constant in a gp-relative `.lit4` pool instead. We pre-expand
# exactly like the SN assembler so no `.lit4` is emitted. Not applied at -G0
# (GNU as is already byte-identical there).
#
# And the inverse hazard of the COP1 flush above: when a `.set noreorder`
# region begins directly after an `mfc1`, GNU as pads the cop1-move hazard
# with a nop at the region boundary; the SN ee-as does not (the r5900
# interlocks, proven by the original bytes in text/1907F0 func_00290EF8:
# `mfc1 $a3,$f1` directly followed by `beqz $a3`). We hold the mfc1 and emit
# it just inside the region, where GNU as adds no hazard padding. Not applied
# at -G0 (no matched -G0 function has an mfc1/noreorder boundary).
# (cc1 is a Win32 PE - its .s lines end in CRLF, hence the \r-stripping.)
if [ "$GFLAG" = "-G8" ]; then
  sed -E -f "$MOVEFIX" "$UNIT_S" | tr -d '\r' | awk '
    NR==FNR {
      if ($0 ~ /^[ \t]*\.extern[ \t]/) {
        line=$0; sub(/^[ \t]*\.extern[ \t]+/,"",line)
        n=split(line,a,/[, \t]+/)
        if (n>=2 && !(a[1] in sz)) sz[a[1]]=a[2]+0
      }
      next
    }
    function f32bits(v,    s, e, m) {
      s = 0; if (v < 0) { s = 1; v = -v }
      if (v == 0) return s * 2147483648
      e = 0
      while (v >= 2) { v /= 2; e++ }
      while (v < 1)  { v *= 2; e-- }
      m = int((v - 1) * 8388608 + 0.5)
      if (m == 8388608) { m = 0; e++ }
      return s * 2147483648 + (e + 127) * 8388608 + m
    }
    # flush a held mfc1 unless the next line opens a noreorder region (or is
    # a comment-only line, which we let pass while still holding)
    {
      if (pend != "" && $0 !~ /^[ \t]*\.set[ \t]+noreorder/ && $0 !~ /^[ \t]*#/) {
        print pend; pend = ""
      }
    }
    /^\tmfc1\t/ { pend = $0; prevcop = 0; next }
    /^\tli\.s\t\$f[0-9]+,/ {
      s = $0; sub(/^\tli\.s\t/, "", s)
      split(s, q, ","); r = q[1]; bits = f32bits(q[2] + 0)
      hi = int(bits / 65536); lo = bits % 65536
      if (lo != 0) {
        printf "\tlui\t$1,0x%x\n\tori\t$1,$1,0x%x\n\tmtc1\t$1,%s\n", hi, lo, r
        prevcop = 0
        next
      }
      # lui-only constants: GNU as already expands these identically.
    }
    /^[ \t]*\.set[ \t]+noreorder/ {
      if (prevcop) print "\tnop"
      print
      if (pend != "") { print pend; pend = "" }
      next
    }
    /^\tla\t\$[0-9]+,[A-Za-z_][A-Za-z0-9_]*(\+[0-9]+)?$/ {
      s=$0; sub(/^\tla\t/,"",s)
      split(s,p,","); r=p[1]; sym=p[2]
      base=sym; sub(/\+[0-9]+$/,"",base)   # smallness is decided by the BASE symbol
      if ((base in sz) && sz[base]<=8)
        printf "\taddiu\t%s,$gp,%%gp_rel(%s)\n", r, sym
      else
        printf "\tlui\t%s,%%hi(%s)\n\taddiu\t%s,%s,%%lo(%s)\n", r, sym, r, r, sym
      prevcop=0
      next
    }
    { print; prevcop = ($0 ~ /^\t(l\.s|lwc1)\t/) }
    END { if (pend != "") print pend }
  ' "$UNIT_S" - \
    | mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL "$GFLAG" -I. -o "$OUT_O" -
else
  sed -E -f "$MOVEFIX" "$UNIT_S" \
    | mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL "$GFLAG" -I. -o "$OUT_O" -
fi

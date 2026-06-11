#!/usr/bin/env bash
# Per-function matching loop: compile a hand-written C function and diff it
# against the original game bytes with objdiff. This is the core decomp loop.
#
#   tools/ee/diff.sh <region> <unit> <func> <cfile>
#   e.g. tools/ee/diff.sh usa cod/015180 func_00115200 going-decompiled/src/usa/cod/015180.c
#
# Prints the function's match percentage. For the interactive red/green TUI:
#   tools/objdiff-cli-macos-arm64 diff -1 tools/ee/.diff/target.o \
#       -2 tools/ee/.diff/base.o <func> -c mips.instrCategory=r5900 -c mips.abi=eabi32
#
# Requires the colima `ee-x86` VM + `ee-build` image (see CLAUDE.md).
set -euo pipefail
REGION="$1"; UNIT="$2"; FUNC="$3"; CFILE="$4"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"
W=tools/ee/.diff; mkdir -p "$W"
OBJDIFF=tools/objdiff-cli-macos-arm64
INC="-Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common"
ASF="-march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -Igoing-decompiled/build/$REGION/include"
CPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DINCLUDE_ASM_USE_MACRO_INC=1"

# Per-unit -G override for the BASE compile - the cod/0321A0 989snd sub-TU was
# originally built at nonzero -G (uniform %gp_rel small-data access), so its C
# is compiled at -G8. Everything else stays -G0. CC1EXTRA holds additional
# per-unit cc1-only flags (NOT passed to the assembler): the gameplay-text
# TUs were built by a later SN cc1 without the load-PRE pass, which
# -fno-gcse reproduces (proven byte-exact on text/1907F0). Keep in sync with
# objdiff_build.sh.
GFLAG="-G0"
CC1EXTRA=""
case "$REGION/$UNIT" in
  usa/cod/0321A0) GFLAG="-G8";;
  usa/text/183178) GFLAG="-G8";; # scale/round accessor sub-TU (D_1A7910..D_1A792C)
  usa/text/188580) GFLAG="-G8";; # camera-aux sub-TU (D_1A8A60..D_1A8AE0)
  usa/text/1907F0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # level-init/screen-fade sub-TU (D_1A9000..D_1A9020)
esac

# Pre-filter the original asm so VU0 (COP2) macro-mode instructions assemble
# with GNU as: spimdisasm emits the Q/ACC special registers as bare tokens,
# GNU as wants them `$`-prefixed (byte-identical encoding). No-op otherwise.
sed -f "$(dirname "$0")/vu0_fixup.sed" \
  "going-decompiled/asm/$REGION/nonmatchings/$UNIT/$FUNC.s" > "$W/$FUNC.s"

# target wrapper: assemble the ORIGINAL function asm (.set noreorder/noat, as INCLUDE_ASM does)
cat > "$W/target.s" <<EOF
.include "macro.inc"
.section .text, "ax"
.set noat
.set noreorder
.include "$W/$FUNC.s"
.set reorder
.set at
EOF

docker --context colima-ee-x86 run --rm -v "$ROOT":/work ee-build sh -c "
  set -e; cd /work; WIBO=/usr/local/bin/wibo; G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
  mips-linux-gnu-as $ASF -o $W/target.o $W/target.s
  \$WIBO \$G/cpp.exe $CPPDEF $INC $CFILE $W/base.i
  \$WIBO \$G/cc1.exe -quiet -O2 $GFLAG $CC1EXTRA $W/base.i -o $W/base.s
  sh tools/ee/asm_unit.sh $REGION /work/$W/base.s /work/$W/base.o $GFLAG
"

"$OBJDIFF" diff -1 "$W/target.o" -2 "$W/base.o" "$FUNC" -o - --format json-pretty \
  -c mips.instrCategory=r5900 -c mips.abi=eabi32 \
  | python3 -c "import sys,json; d=json.load(sys.stdin); s=[x for x in d['left']['sections'] if x['name']=='.text'][0]; pct=s.get('match_percent',0); print(f'$FUNC: {pct:.2f}% match' + (' ✅ MATCH' if pct==100 else ''))"

#!/usr/bin/env bash
# Per-function matching loop: compile a hand-written C function and diff it
# against the original game bytes with objdiff. This is the core decomp loop.
#
#   tools/ee/diff.sh <region> <unit> <func> [<cfile>]
#   e.g. tools/ee/diff.sh usa cod/015180 func_00115200
#
# <cfile> defaults to the unit's source, src/<region>/<unit>.c or .cpp, picked
# by `ee_cc1.sh --resolve` (task #1285); pass it to diff a scratch copy. A .cpp
# source compiles through cc1plus with the build's extern "C" wrap, exactly as
# build.sh and objdiff_build.sh compile it: ee_cc1.sh is the one compile path.
#
# Prints the function's match percentage. For the interactive red/green TUI:
#   tools/objdiff-cli-macos-arm64 diff -1 tools/ee/.diff/target.o \
#       -2 tools/ee/.diff/base.o <func> -c mips.instrCategory=r5900 -c mips.abi=eabi32
#
# Requires the colima `ee-x86` VM + `ee-build` image (see CLAUDE.md).
set -euo pipefail
REGION="$1"; UNIT="$2"; FUNC="$3"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"
CFILE="${4:-$(sh tools/ee/ee_cc1.sh --resolve "going-decompiled/src/$REGION/$UNIT")}"
[ -f "$CFILE" ] || { echo "diff.sh: FATAL — no source $CFILE" >&2; exit 2; }
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
  eu/cod/0321A0) GFLAG="-G8";; # EU mirror of the 989snd sub-TU (same -G8 model)
  usa/text/183178) GFLAG="-G8";; # scale/round accessor sub-TU (D_1A7910..D_1A792C)
  usa/text/188580) GFLAG="-G8";; # camera-aux sub-TU (D_1A8A60..D_1A8AE0)
  usa/text/188858) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # Tier-1-A carve (.text mid 2; later-cc1 gameplay/UI TU model)
  usa/text/1907F0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # level-init/screen-fade sub-TU (D_1A9000..D_1A9020)
  usa/text/191238) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # Tier-1-B carve (.text mid 3; boot/IRX init + sky render + segment loader + map system)
  usa/text/198FA0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # save/GUI-wrapper unit (g_guiInstance modeled cc1-small/assembler-absolute)
  usa/text/1A00F0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # Tier-1-C carve (.text tail head; moby render/anim/grid + ammo-drop + bestiary)
  usa/text/1A8180) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing";; # game-state cluster (carve pick #1; later-cc1 TU model: sized externs under -G8, no load-PRE). -fno-strict-aliasing: the u16 slot-list reads are ordered after the pointer-global cursor stores and re-read after the slot/moby stores in func_002A9550 (task #1100)
  usa/text/250080) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # segment-tail FMV/debug-stub unit (carve pick #2; same later-cc1 TU model)
  usa/text/16E980) GFLAG="-G8";; # 16E980 head: camera/screen-FX unit (carve pick #6). Plain -G8, NO -fno-gcse (original keeps the %hi CSE - load-PRE present)
  usa/text/248B50) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # GUI sub-chunk 1 (carve pick #3a; later-cc1 TU model, sized externs under -G8)
  usa/text/235FE8) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing";; # GUI widget-method band (carve pick #3b; same later-cc1 GUI TU model). -fno-strict-aliasing restores the per-store pointer-member reload in GuiElementSetPos/GuiElementSetScale (#75)
  usa/text/1CA080) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # menu-screens A (carve pick #5; later-cc1 TU model, gp-dense)
  usa/text/1D54C0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # menu-screens B (carve pick #5; later-cc1 TU model, gp-dense)
  usa/text/1B4218) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # moby-bind band (later-cc1 TU model, sized externs under -G8)
  # USA CARVE MEGA-BATCH PHASE A (2026-06-14): 7 new c-units from TILE A/B/C/D.
  usa/text/178E88) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE A render/draw-2D A
  usa/text/1823B8) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE A render/draw-2D B
  usa/text/1DFF80) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B moby-glow/shrub/sky/sound-emit/cinematic
  usa/text/1EFFC0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B tfrag/tie draw + vendor shop + GS/VIF
  usa/text/1FCF48) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # frame-arena/render-task-list sub-TU: the ROM stores g_sceneArenaCursor, g_frameArenaFlip and g_renderTaskWorkBuf %gp_rel, which cc1 cannot emit at -G0 (task #889)
  usa/text/183558) GFLAG="-G8";; # math C-helper band: -G8 so the ROM's %gp_rel store of g_bProgressiveScan is reachable; measured harmless to the unit's existing C (task #889)
  usa/text/1FFBA0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B bolt economy + turret weapon
  usa/text/24D728) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE D GUI/camera helpers
  # EU TEXT RE-TILE (Phase A, 2026-06-14): EU twins of the 11 USA text c-units.
  eu/text/16E7B8) GFLAG="-G8";;                          # USA 16E980 twin
  eu/text/183088) GFLAG="-G8";;                          # USA 183178 twin
  eu/text/188470) GFLAG="-G8";;                          # USA 188580 twin
  eu/text/190808) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1907F0 twin
  eu/text/198B58) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 198FA0 twin
  eu/text/1A7D10) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1A8180 twin
  eu/text/1C9F58) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1CA080 twin
  eu/text/1D5488) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1D54C0 twin
  eu/text/236ED8) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing";;    # USA 235FE8 twin (#75 -fsa parity: reload-class unlock, applies when EU carves GuiElementSetPos/Scale etc.)
  eu/text/249FE8) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 248B50 twin
  eu/text/251520) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 250080 twin
  # REGION-AXIS CARVE (2026-06-15): EU twins of the 3 recent USA text carves.
  eu/text/188748) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 188858 twin
  eu/text/191240) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 191238 twin
  eu/text/19FC78) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1A00F0 twin
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
  set -e; cd /work
  mips-linux-gnu-as $ASF -o $W/target.o $W/target.s
  EE_CC1_UNIT='$REGION $UNIT' sh tools/ee/ee_cc1.sh sdk29 $CFILE $W/base.i $W/base.s '$CPPDEF $INC' '-O2 $GFLAG $CC1EXTRA'
  sh tools/ee/asm_unit.sh $REGION /work/$W/base.s /work/$W/base.o $GFLAG
"

"$OBJDIFF" diff -1 "$W/target.o" -2 "$W/base.o" "$FUNC" -o - --format json-pretty \
  -c mips.instrCategory=r5900 -c mips.abi=eabi32 \
  | python3 -c "import sys,json; d=json.load(sys.stdin); s=[x for x in d['left']['sections'] if x['name']=='.text'][0]; pct=s.get('match_percent',0); print(f'$FUNC: {pct:.2f}% match' + (' ✅ MATCH' if pct==100 else ''))"

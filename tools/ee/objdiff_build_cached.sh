#!/usr/bin/env bash
# objdiff_build_cached.sh — TARGET-OBJECT-CACHED variant of objdiff_build.sh.
#
#   tools/ee/objdiff_build_cached.sh <region> <unit> [base_cfile]
#   e.g. tools/ee/objdiff_build_cached.sh usa cod/015180
#
# IDENTICAL outputs to objdiff_build.sh (byte-for-byte EXPECTED + OBJ), but the
# immutable TARGET/EXPECTED object is content-addressed and cached so each grind
# iteration only rebuilds the BASE. The target depends ONLY on the per-unit asm
# tree (asm/<region>/nonmatchings/<unit>/*.s) — worktree-independent and frozen
# during a decomp session — so it is safe to cache keyed on a hash of that tree.
#
#   - CACHE MISS: run the FULL docker (target+base) exactly as objdiff_build.sh,
#     then atomically store the freshly-built EXPECTED under its asm-hash key.
#   - CACHE HIT : cp the cached EXPECTED into place and run a BASE-ONLY docker
#     (skips target cpp/cc1/asm_unit + the target .NON_MATCHING strip).
#
# CACHE_ROOT is configurable via OBJDIFF_TARGET_CACHE (default /tmp/...). The
# cache is purely a build accelerator; the EXPECTED bytes are identical with or
# without it, which the companion validation proves via `cmp`.
#
# Single ops only; the one compound command is the necessary `docker run`.
# Requires the colima `ee-x86` VM + `ee-build` image (see CLAUDE.md).
set -euo pipefail
REGION="$1"; UNIT="$2"; BASECFILE="${3:-going-decompiled/src/$REGION/$UNIT.c}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"

EXPECTED="going-decompiled/build/$REGION/expected/$UNIT.o"
OBJ="going-decompiled/build/$REGION/obj/$UNIT.o"
W="tools/ee/.objdiff/$REGION/$UNIT"
ASMDIR="going-decompiled/asm/$REGION/nonmatchings/$UNIT"
mkdir -p "$(dirname "$EXPECTED")" "$(dirname "$OBJ")" "$W"

INC="-Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common"
CPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DINCLUDE_ASM_USE_MACRO_INC=1"

# Per-unit -G override (BASE compile only; the target is pure INCLUDE_ASM so
# -G is irrelevant there). The cod/0321A0 989snd sub-TU is an original separate
# TU built at nonzero -G: its sdata cluster (0x1A7480..0x1A74F8) is accessed
# uniformly via %gp_rel, which only a nonzero -G reproduces. Everything else
# stays at the proven -O2 -G0. CC1EXTRA holds additional per-unit cc1-only
# flags (NOT passed to the assembler): the gameplay-text TUs were built by a
# later SN cc1 without the load-PRE pass, which -fno-gcse reproduces (proven
# byte-exact on text/1907F0). Keep this list in sync with diff.sh.
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
  usa/text/1A8180) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # game-state cluster (carve pick #1; later-cc1 TU model: sized externs under -G8, no load-PRE)
  usa/text/250080) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # segment-tail FMV/debug-stub unit (carve pick #2; same later-cc1 TU model)
  usa/text/16E980) GFLAG="-G8";; # 16E980 head: camera/screen-FX unit (carve pick #6). Plain -G8, NO -fno-gcse: the original KEEPS the %hi CSE in a register across AddScreenSpriteFx (load-PRE present), unlike the gameplay-text TUs
  usa/text/248B50) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # GUI sub-chunk 1 (carve pick #3a; later-cc1 TU model, sized externs under -G8)
  usa/text/235FE8) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing";; # GUI widget-method band (carve pick #3b; same later-cc1 GUI TU model). -fno-strict-aliasing restores the per-store pointer-member reload in GuiElementSetPos/GuiElementSetScale (#75)
  usa/text/1CA080) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # menu-screens A (carve pick #5; later-cc1 TU model, gp-dense)
  usa/text/1D54C0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # menu-screens B (carve pick #5; later-cc1 TU model, gp-dense)
  usa/text/1B4218) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # moby-bind band (carve pick #5/moby-bind; later-cc1 TU model, sized externs under -G8)
  # USA CARVE MEGA-BATCH PHASE A (2026-06-14): 7 new c-units carved from the
  # TILE A/B/C/D asm tiles. 6 are later-cc1 gameplay/UI TUs (-G8 -fno-gcse);
  # text/183558 is the math C-helper band built at the default -O2 -G0.
  usa/text/178E88) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE A render/draw-2D A
  usa/text/1823B8) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE A render/draw-2D B
  usa/text/1DFF80) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B moby-glow/shrub/sky/sound-emit/cinematic
  usa/text/1EFFC0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B tfrag/tie draw + vendor shop + GS/VIF
  usa/text/1FFBA0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B bolt economy + turret weapon
  usa/text/24D728) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE D GUI/camera helpers
  # usa/text/183558 stays -G0 (TILE C math C-helper band; default, no case)
  # EU TEXT RE-TILE (Phase A, 2026-06-14): EU twins of the 11 USA text c-units.
  # Each EU unit is named text/<eu-fileoff> and mirrors its USA sibling's GFLAG.
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

# ---------------------------------------------------------------------------
# TARGET-OBJECT CACHE
# ---------------------------------------------------------------------------
# Content-address the immutable target on a STABLE hash of the unit's asm tree:
# the sorted file LIST plus every byte of every .s (so adding/removing/editing
# any nonmatching .s invalidates the key). This is exactly the target's only
# input — the all-INCLUDE_ASM target_unit.c is derived purely from this tree.
ASMHASH="$( { find "$ASMDIR" -name '*.s' | LC_ALL=C sort; find "$ASMDIR" -name '*.s' | LC_ALL=C sort | xargs cat; } | shasum -a 256 | cut -d' ' -f1)"
CACHE_ROOT="${OBJDIFF_TARGET_CACHE:-/tmp/objdiff-target-cache}"
CACHED_TGT="$CACHE_ROOT/$REGION/$UNIT/$ASMHASH.o"

# Generate a PRISTINE all-INCLUDE_ASM unit C for the TARGET, straight from the
# asm tree — one INCLUDE_ASM per <func>.s, emitted in ASCENDING VRAM-ADDRESS order
# (each .s carries its address in the first `/* off addr bytes */` comment). This
# preserves the original section layout (alphabetical glob order would misplace
# named funcs like memset) and keeps the target == the original bytes even after
# the editable src/<unit>.c gains real decompiled C (the base uses that source;
# the target must never drift). Only needed on a cache MISS (the target build).
TGTC="$W/target_unit.c"
gen_target_c() {
  printf '#include "common.h"\n' > "$TGTC"
  python3 - "$ASMDIR" >> "$TGTC" <<'PY'
import sys, re, glob, os
asmdir = sys.argv[1]
items = []
for s in sorted(glob.glob(os.path.join(asmdir, "*.s"))):
    func = os.path.basename(s)[:-2]
    addr = None
    with open(s) as fh:
        for line in fh:
            m = re.search(r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+[0-9A-Fa-f]+\s*\*/", line)
            if m:
                addr = int(m.group(1), 16)
                break
    items.append((addr if addr is not None else (1 << 62), func))
for _, func in sorted(items):
    print(f'INCLUDE_ASM("{asmdir}", {func});')
PY
}

# Compile the pristine *target* C (MISS only) and the *base* C (your decomp /
# override, every iteration). Each cc1 emits a .s; asm_unit.sh assembles it
# (with the VU0 fixup mirror) into one object. Then:
#   - target: strip the `.NON_MATCHING` object markers (a base-side flag; objdiff
#     special-cases them, so leaving them on the target would drop those symbols).
#   - base:   strip the ee-gcc placeholder symbols so the symbol tables line up.
#     The base KEEPS the `.NON_MATCHING` markers for functions still on INCLUDE_ASM
#     — that is exactly how objdiff knows they aren't decompiled yet.
if [ -f "$CACHED_TGT" ]; then
  # ---- CACHE HIT: reuse the immutable target, rebuild only the base. ----
  echo "[objdiff_build] target CACHE HIT $CACHED_TGT"
  cp "$CACHED_TGT" "$EXPECTED"
  docker --context colima-ee-x86 run --rm -e ASMFIX_SHARED -v "$ROOT":/work ee-build sh -c "
    set -e; cd /work; WIBO=/usr/local/bin/wibo; G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
    \$WIBO \$G/cpp.exe $CPPDEF $INC $BASECFILE $W/base.i
    \$WIBO \$G/cc1.exe -quiet -O2 $GFLAG $CC1EXTRA $W/base.i -o $W/base.s
    sh tools/ee/asm_unit.sh $REGION /work/$W/base.s /work/$OBJ $GFLAG
    mips-linux-gnu-strip $OBJ -N gcc2_compiled. -N __gnu_compiled_c -N dummy-symbol-name
  "
else
  # ---- CACHE MISS: full target+base build (identical to objdiff_build.sh). ----
  echo "[objdiff_build] target CACHE MISS — building + storing $CACHED_TGT"
  gen_target_c
  docker --context colima-ee-x86 run --rm -e ASMFIX_SHARED -v "$ROOT":/work ee-build sh -c "
    set -e; cd /work; WIBO=/usr/local/bin/wibo; G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
    \$WIBO \$G/cpp.exe $CPPDEF $INC $TGTC $W/target.i
    \$WIBO \$G/cc1.exe -quiet -O2 -G0 $W/target.i -o $W/target.s
    sh tools/ee/asm_unit.sh $REGION /work/$W/target.s /work/$EXPECTED
    \$WIBO \$G/cpp.exe $CPPDEF $INC $BASECFILE $W/base.i
    \$WIBO \$G/cc1.exe -quiet -O2 $GFLAG $CC1EXTRA $W/base.i -o $W/base.s
    sh tools/ee/asm_unit.sh $REGION /work/$W/base.s /work/$OBJ $GFLAG
    mips-linux-gnu-nm $EXPECTED | awk '/\\.NON_MATCHING\$/{print \"-N\", \$3}' > $W/nmstrip.txt
    test -s $W/nmstrip.txt && mips-linux-gnu-strip $EXPECTED \$(cat $W/nmstrip.txt) || true
    mips-linux-gnu-strip $OBJ -N gcc2_compiled. -N __gnu_compiled_c -N dummy-symbol-name
  "
  # Atomically publish the freshly-built target into the content-addressed cache.
  mkdir -p "$(dirname "$CACHED_TGT")"
  cp "$EXPECTED" "$CACHED_TGT.tmp.$$"
  mv "$CACHED_TGT.tmp.$$" "$CACHED_TGT"
fi

echo "[objdiff_build] target -> $EXPECTED"
echo "[objdiff_build] base   -> $OBJ"

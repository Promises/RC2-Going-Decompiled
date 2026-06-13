#!/usr/bin/env bash
# objdiff_build.sh — pre-build per-unit TARGET and BASE objects for project-wide
# objdiff (objdiff.json at the repo root reads these pre-built .o files).
#
#   tools/ee/objdiff_build.sh <region> <unit> [base_cfile]
#   e.g. tools/ee/objdiff_build.sh usa cod/015180
#        tools/ee/objdiff_build.sh usa cod/015180 some/override.c   # wrong-base test
#
# TARGET object (immutable original bytes) ->
#     going-decompiled/build/<region>/expected/<unit>.o
#   Assembled from the original splat asm tree: we compile a target-only,
#   all-INCLUDE_ASM copy of the unit C with cc1 so its .s pulls in every
#   per-function asm via `.include`, then assemble that with asm_unit.sh
#   (applies vu0_fixup). splat's `nonmatching` macro tags every function with a
#   `<func>.NON_MATCHING` object symbol; that marker is a *base*-side decomp
#   flag, never meaningful on the target, so we strip it from the target object
#   (objdiff special-cases `.NON_MATCHING` and would otherwise drop the symbol).
#
# BASE object (your decomp under test) ->
#     going-decompiled/build/<region>/obj/<unit>.o
#   Compiled from src/<region>/<unit>.c (or the optional override cfile). Every
#   function that is still `INCLUDE_ASM` pulls in its asm AND that asm's
#   `<func>.NON_MATCHING` marker, so objdiff correctly EXCLUDES it from the
#   report (not yet decompiled). When you replace a stub with real C, its marker
#   disappears and objdiff starts scoring it — 100% once the bytes match. The
#   ee-gcc placeholder symbols are stripped so the symbol tables line up.
#
# To prove the harness end-to-end on the current all-INCLUDE_ASM unit (where the
# base still carries every marker), tools/ee/objdiff_demo.sh builds a base==target
# variant so the unit reads 100% across all 707 functions.
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
  usa/text/1907F0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # level-init/screen-fade sub-TU (D_1A9000..D_1A9020)
  usa/text/198FA0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # save/GUI-wrapper unit (g_guiInstance modeled cc1-small/assembler-absolute)
  usa/text/1A8180) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # game-state cluster (carve pick #1; later-cc1 TU model: sized externs under -G8, no load-PRE)
  usa/text/250080) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # segment-tail FMV/debug-stub unit (carve pick #2; same later-cc1 TU model)
  usa/text/16E980) GFLAG="-G8";; # 16E980 head: camera/screen-FX unit (carve pick #6). Plain -G8, NO -fno-gcse: the original KEEPS the %hi CSE in a register across AddScreenSpriteFx (load-PRE present), unlike the gameplay-text TUs
  usa/text/248B50) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # GUI sub-chunk 1 (carve pick #3a; later-cc1 TU model, sized externs under -G8)
  usa/text/235FE8) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # GUI widget-method band (carve pick #3b; same later-cc1 GUI TU model)
  usa/text/1CA080) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # menu-screens A (carve pick #5; later-cc1 TU model, gp-dense)
  usa/text/1D54C0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # menu-screens B (carve pick #5; later-cc1 TU model, gp-dense)
esac

# Generate a PRISTINE all-INCLUDE_ASM unit C for the TARGET, straight from the
# asm tree — one INCLUDE_ASM per <func>.s, emitted in ASCENDING VRAM-ADDRESS order
# (each .s carries its address in the first `/* off addr bytes */` comment). This
# preserves the original section layout (alphabetical glob order would misplace
# named funcs like memset) and keeps the target == the original bytes even after
# the editable src/<unit>.c gains real decompiled C (the base uses that source;
# the target must never drift).
TGTC="$W/target_unit.c"
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

# Compile the pristine *target* C and the *base* C (your decomp / override).
# Each cc1 emits a .s; asm_unit.sh assembles it (with the VU0 fixup mirror) into
# one object. Then:
#   - target: strip the `.NON_MATCHING` object markers (a base-side flag; objdiff
#     special-cases them, so leaving them on the target would drop those symbols).
#   - base:   strip the ee-gcc placeholder symbols so the symbol tables line up.
#     The base KEEPS the `.NON_MATCHING` markers for functions still on INCLUDE_ASM
#     — that is exactly how objdiff knows they aren't decompiled yet.
docker --context colima-ee-x86 run --rm -v "$ROOT":/work ee-build sh -c "
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

echo "[objdiff_build] target -> $EXPECTED"
echo "[objdiff_build] base   -> $OBJ"

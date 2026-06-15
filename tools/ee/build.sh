#!/bin/sh
# Asm round-trip / matching build for one region. Runs INSIDE the ee-build
# container in the colima x86 VM:
#
#   docker --context colima-ee-x86 run --rm -v "$PWD":/work ee-build \
#       sh tools/ee/build.sh usa
#
# Assembles every split .s with modern GNU mips binutils (the period ee-gcc is
# only for compiling C), links with the splat-generated ld script, flattens to
# a .rom, and diffs vs the original. C compilation of src/ is added later.
set -e
REGION="${1:-usa}"
cd "$(dirname "$0")/../.."        # repo root inside container (/work)

case "$REGION" in
  usa) BASENAME=SCUS_972.68 ;;
  eu)  BASENAME=SCES_516.07 ;;
  *) echo "unknown region $REGION"; exit 2 ;;
esac

ASM=going-decompiled/asm/$REGION
BUILD=going-decompiled/build/$REGION
INC=$BUILD/include
LD=going-decompiled/linker_scripts/$BASENAME.ld
ORIG=extracted/$REGION/$BASENAME.rom
ELF=$BUILD/$BASENAME.elf
ROM=$BUILD/$BASENAME.rom

ASFLAGS="-march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -I $INC -I $ASM -I $BUILD"
VU0FIX="$(dirname "$0")/vu0_fixup.sed"   # spimdisasm VU0 macro op -> GNU-as syntax
SRC=going-decompiled/src/$REGION
WIBO=/usr/local/bin/wibo
G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
INCC="-Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common"
CPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DINCLUDE_ASM_USE_MACRO_INC=1"

# 1) Assemble plain data/code .s files. EXCLUDE per-function nonmatchings/ and
#    matchings/ — those belong to a `c` unit and are pulled in by compiling its
#    src .c (INCLUDE_ASM), not assembled standalone (they lack macro.inc context).
echo "== [$REGION] assembling section .s files =="
n=0
for s in $(find $ASM -name '*.s' -not -path '*/nonmatchings/*' -not -path '*/matchings/*'); do
  o="$BUILD/${s%.s}.o"
  mkdir -p "$(dirname "$o")"
  # VU0 fixup + rewrite splat's absolute asset .incbin paths to repo-relative
  # (splat absolutizes them; CWD is the repo root /work in the container).
  sed -f "$VU0FIX" "$s" | sed 's|"/[^"]*/going-decompiled/|"going-decompiled/|g' \
    | mips-linux-gnu-as $ASFLAGS -o "$o" - 2> "$o.log" || { echo "AS FAIL $s:"; tail -5 "$o.log"; exit 1; }
  n=$((n+1))
done
echo "   assembled $n section objects"

# 2) Compile each `c` unit (src/<region>/**/*.c) into the object the .ld expects:
#    $BUILD/<full src path>.o. cc1 -> .s (with INCLUDE_ASM .include lines) ->
#    asm_unit.sh assembles it through the VU0-fixed mirror.
if [ -d "$SRC" ]; then
  echo "== [$REGION] compiling src/ c units =="
  m=0
  for c in $(find "$SRC" -name '*.c'); do
    o="$BUILD/${c%.c}.o"
    mkdir -p "$(dirname "$o")"
    # Per-unit -G override - the cod/0321A0 989snd sub-TU was originally built
    # at nonzero -G (uniform %gp_rel small-data). CC1EXTRA = per-unit cc1-only
    # flags (NOT passed to the assembler; -fno-gcse for the later-cc1
    # gameplay-text TUs). Keep in sync with objdiff_build.sh / diff.sh.
    GFLAG="-G0"
    CC1EXTRA=""
    case "$c" in
      */cod/0321A0.c) GFLAG="-G8";;
      */usa/text/183178.c) GFLAG="-G8";; # scale/round accessor sub-TU
      */usa/text/188580.c) GFLAG="-G8";; # camera-aux sub-TU
      */usa/text/188858.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # Tier-1-A carve (.text mid 2)
      */usa/text/1907F0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # level-init/screen-fade sub-TU
      */usa/text/191238.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # Tier-1-B carve (.text mid 3)
      */usa/text/198FA0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # save/GUI-wrapper unit
      */usa/text/1A00F0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # Tier-1-C carve (.text tail head)
      */usa/text/1A8180.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # game-state cluster sub-TU
      */usa/text/250080.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # segment-tail FMV/debug-stub sub-TU
      */usa/text/16E980.c) GFLAG="-G8";; # 16E980 head camera/screen-FX unit (carve pick #6; plain -G8, original keeps the %hi CSE)
      */usa/text/248B50.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # GUI sub-chunk 1 (carve pick #3a)
      */usa/text/235FE8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # GUI widget-method band (carve pick #3b)
      */usa/text/1CA080.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # menu-screens A (carve pick #5)
      */usa/text/1D54C0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # menu-screens B (carve pick #5)
      */usa/text/1B4218.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # moby-bind band (carve pick #5/moby-bind)
      # USA CARVE MEGA-BATCH PHASE A (2026-06-14): 7 new c-units from TILE A/B/C/D.
      */usa/text/178E88.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE A render/draw-2D A
      */usa/text/1823B8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE A render/draw-2D B
      */usa/text/1DFF80.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B moby-glow/shrub/sky/sound-emit/cinematic
      */usa/text/1EFFC0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B tfrag/tie draw + vendor shop + GS/VIF
      */usa/text/1FFBA0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B bolt economy + turret weapon
      */usa/text/24D728.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE D GUI/camera helpers
      # */usa/text/183558.c stays -G0 (TILE C math C-helper band; default, no case)
      # EU TEXT RE-TILE (Phase A, 2026-06-14): EU twins of the 11 USA text c-units.
      */eu/text/16E7B8.c) GFLAG="-G8";;                          # USA 16E980 twin
      */eu/text/183088.c) GFLAG="-G8";;                          # USA 183178 twin
      */eu/text/188470.c) GFLAG="-G8";;                          # USA 188580 twin
      */eu/text/190808.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1907F0 twin
      */eu/text/198B58.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 198FA0 twin
      */eu/text/1A7D10.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1A8180 twin
      */eu/text/1C9F58.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1CA080 twin
      */eu/text/1D5488.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1D54C0 twin
      */eu/text/236ED8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 235FE8 twin
      */eu/text/249FE8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 248B50 twin
      */eu/text/251520.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 250080 twin
    esac
    "$WIBO" "$G/cpp.exe" $CPPDEF $INCC "$c" "$BUILD/_unit.i"
    "$WIBO" "$G/cc1.exe" -quiet -O2 $GFLAG $CC1EXTRA "$BUILD/_unit.i" -o "$BUILD/_unit.s"
    sh tools/ee/asm_unit.sh "$REGION" "/work/$BUILD/_unit.s" "/work/$o" "$GFLAG"
    mips-linux-gnu-strip "$o" -N dummy-symbol-name 2>/dev/null || true
    m=$((m+1))
  done
  echo "   compiled $m c units"
fi

echo "== [$REGION] linking with $LD =="
SYMS="$BUILD/undefined_syms_auto.txt"
# Blanket-define every D_<hex> symbol to its absolute address (spimdisasm names
# auto-symbols by address). Resolves references it didn't emit labels for.
ALLSYMS="$BUILD/all_addr_syms.ld"
grep -rhoE '(D_|func_)[0-9A-Fa-f]{4,}' "$ASM" | sort -u | sed -E 's/^(D_|func_)([0-9A-Fa-f]+)$/\1\2 = 0x\2;/' > "$ALLSYMS"
echo "   defined $(wc -l < "$ALLSYMS") address symbols"
mips-linux-gnu-ld -EL --allow-multiple-definition -T "$LD" -T "$SYMS" -T "$ALLSYMS" -Map "$BUILD/$BASENAME.map" -o "$ELF" 2> "$BUILD/ld.log" \
  || { echo "LD errors (first 20):"; head -20 "$BUILD/ld.log"; }

# Second link for .rom flattening: identical, but with a linker script whose
# load addresses equal the virtual addresses (LMA == VMA). splat's script packs
# sections by LMA via `AT(<seg>_ROM_START)`/`__romPos`, which drops the bss/gap
# zeros and does NOT match the original rom layout. Stripping the `AT(...)`
# clauses makes ld default LMA = VMA, so objcopy -O binary produces the
# VMA-contiguous, zero-gap-filled image the original .rom actually is.
LDLMA="$BUILD/$BASENAME.lma.ld"
ELFLMA="$BUILD/$BASENAME.lma.elf"
sed -E 's/ AT\([A-Za-z0-9_]+\)//g' "$LD" > "$LDLMA"
mips-linux-gnu-ld -EL --allow-multiple-definition -T "$LDLMA" -T "$SYMS" -T "$ALLSYMS" -o "$ELFLMA" 2> "$BUILD/ld.lma.log" \
  || { echo "LD(LMA) errors (first 20):"; head -20 "$BUILD/ld.lma.log"; }

if [ -f "$ELFLMA" ]; then
  echo "== [$REGION] ELF built; flattening + diff =="
  # The original .rom is a VMA-CONTIGUOUS image of the loadable sections: every
  # PROGBITS section sits at (VMA - base), and the gaps (NOBITS/bss ranges and
  # the huge jump up to the 0x1800000 segment) are zero-filled. Reconstructing
  # the original ELF this way is byte-identical to extracted/$REGION/$BASENAME.rom
  # (proven). objcopy -O binary lays out by LMA and zero-fills inter-section
  # gaps, so we just need LMA == VMA. The relinked ELF above used the LMA==VMA
  # script ($LDLMA), so a plain objcopy reproduces the original layout model.
  mips-linux-gnu-objcopy -O binary "$ELFLMA" "$ROM" 2>/dev/null || true
  echo -n "orig  "; sha1sum "$ORIG" | awk '{print $1, '$(stat -c%s "$ORIG" 2>/dev/null || echo "?")'}'
  echo -n "built "; sha1sum "$ROM"  2>/dev/null | awk '{print $1}'
  if cmp -s "$ORIG" "$ROM"; then echo "MATCH: byte-identical .rom"; else
    echo "DIFF: $(cmp -l "$ORIG" "$ROM" 2>/dev/null | wc -l) differing bytes (of $(stat -c%s "$ORIG") )"
  fi
else
  echo "== no ELF produced (link incomplete) =="
fi

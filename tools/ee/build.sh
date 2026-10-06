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

# 2) Compile each `c` unit (src/<region>/**/*.c, or *.cpp for a unit converted
#    to C++) into the object the .ld expects: $BUILD/<src path minus extension>.o.
#    tools/ee/ee_cc1.sh (cpp + cc1, or cpp -lang-c++ + cc1plus inside one
#    extern "C") -> .s (with INCLUDE_ASM .include lines) -> asm_unit.sh
#    assembles it through the VU0-fixed mirror.
if [ -d "$SRC" ]; then
  echo "== [$REGION] compiling src/ c units =="
  # WARM MIRROR BY DEFAULT (#398/#438). asm_unit.sh rebuilds its VU0-fixed copy
  # of the whole nonmatchings tree for EVERY unit (~3 min each over the VM
  # mount, ~75 min/region) unless ASMFIX_SHARED names a mirror to reuse. The
  # default — a per-worktree mirror named by a content fingerprint of the
  # mirror's inputs, so a re-split or an edited .s selects a NEW path instead
  # of serving stale asm — lives in tools/ee/asmfix_default.sh, shared with
  # objdiff_build.sh, and that file aborts the build if either script stops
  # sourcing it. Runs here IN the container, so the prune runs directly (no
  # host-side deletion of a container-written path, #391). A caller-supplied
  # `-e ASMFIX_SHARED=…` is honoured verbatim and not fingerprinted.
  . tools/ee/asmfix_default.sh
  eval "$ASMFIX_PRUNE"
  echo "   asmfix mirror -> $ASMFIX_SHARED ($ASMFIX_STATE)"
  # The s136os splice's own seeded arms, run HERE (the container's mawk) before
  # any unit is spliced: its refusal check once passed on the host's awk and
  # refused every member in the container (task #1326).
  sh tools/ee/s136os_splice.sh --selftest > "$BUILD/s136os_splice_selftest.log" 2>&1 \
    || { cat "$BUILD/s136os_splice_selftest.log" >&2; echo "BUILD FAIL (s136os_splice --selftest)" >&2; exit 1; }
  echo "   $(tail -1 "$BUILD/s136os_splice_selftest.log")"
  m=0
  for c in $(find "$SRC" \( -name '*.c' -o -name '*.cpp' \)); do
    # ukey: the unit's path spelled .c whatever its language, so the per-unit
    # flag table below (and its copies in objdiff_build.sh / diff.sh /
    # unit_flags.sh, held equal by flagdiff.py) is keyed once per unit.
    case "$c" in *.cpp) ukey="${c%.cpp}.c" ;; *) ukey="$c" ;; esac
    o="$BUILD/${ukey%.c}.o"
    mkdir -p "$(dirname "$o")"
    # Per-unit -G override - the cod/0321A0 989snd sub-TU was originally built
    # at nonzero -G (uniform %gp_rel small-data). CC1EXTRA = per-unit cc1-only
    # flags (NOT passed to the assembler; -fno-gcse for the later-cc1
    # gameplay-text TUs). Keep in sync with objdiff_build.sh / diff.sh.
    # S136EXTRA = the cc1 flags for the unit's s136os splice compile (SN 1.36);
    # an arm that does not set it gets CC1EXTRA, so only a unit whose two
    # compilers need different flags names it (RULING #9004: 1B4218; RULING #9070: 191238; RULING #9450: 188858).
    GFLAG="-G0"
    CC1EXTRA=""
    unset S136EXTRA
    case "$ukey" in
      */cod/0321A0.c) GFLAG="-G8";;
      */usa/text/183178.c) GFLAG="-G8";; # scale/round accessor sub-TU
      */usa/text/188580.c) GFLAG="-G8";; # camera-aux sub-TU
      */usa/text/188858.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse"; S136EXTRA="";; # Tier-1-A carve (.text mid 2). S136EXTRA: the s136os arm compiles at the -O2 default (RULING #9450, FACTs #9441/#9449); the 2.9 compile keeps -fno-gcse
      */usa/text/1907F0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # level-init/screen-fade sub-TU
      */usa/text/191238.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse"; S136EXTRA="";; # Tier-1-B carve (.text mid 3). S136EXTRA: the s136os arm compiles at the -O2 default (RULING #9070, FACT #9069); the 2.9 compile keeps -fno-gcse
      */usa/text/198FA0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # save/GUI-wrapper unit
      */usa/text/1A00F0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # Tier-1-C carve (.text tail head)
      */usa/text/1A8180.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing"; S136EXTRA="-fno-strict-aliasing";; # game-state cluster sub-TU. S136EXTRA: the s136os arm drops -fno-gcse only and keeps -fno-strict-aliasing (RULING #9336, FACTs #9333/#9334); the 2.9 compile keeps both
      */usa/text/250080.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # segment-tail FMV/debug-stub sub-TU
      */usa/text/16E980.c) GFLAG="-G8";; # 16E980 head camera/screen-FX unit (carve pick #6; plain -G8, original keeps the %hi CSE)
      */usa/text/248B50.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # GUI sub-chunk 1 (carve pick #3a)
      */usa/text/235FE8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing";; # GUI widget-method band (carve pick #3b)
      */usa/text/1CA080.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse"; S136EXTRA="-fstrict-aliasing";; # menu-screens A (carve pick #5). S136EXTRA: RULING #9235 (task #1540) — the s136os arm adds -fstrict-aliasing (func_002D33A8's store/lui order); the 2.9 compile is unchanged; RULING #9491 (task #1685) drops -fno-gcse from the s136os arm
      */usa/text/1D54C0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # menu-screens B (carve pick #5)
      */usa/text/1B4218.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse"; S136EXTRA="";; # moby-bind band (carve pick #5/moby-bind). S136EXTRA: the s136os arm compiles at the -O2 default (RULING #9004, FACT #9003); the 2.9 compile keeps -fno-gcse
      # USA CARVE MEGA-BATCH PHASE A (2026-06-14): 7 new c-units from TILE A/B/C/D.
      */usa/text/178E88.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE A render/draw-2D A
      */usa/text/1823B8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE A render/draw-2D B
      */usa/text/1DFF80.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B moby-glow/shrub/sky/sound-emit/cinematic
      */usa/text/1EFFC0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B tfrag/tie draw + vendor shop + GS/VIF
      */usa/text/1FCF48.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # frame-arena/render-task-list sub-TU: the ROM stores g_sceneArenaCursor, g_frameArenaFlip and g_renderTaskWorkBuf %gp_rel, which cc1 cannot emit at -G0 (task #889)
      */usa/text/183558.c) GFLAG="-G8";; # math C-helper band: -G8 so the ROM's %gp_rel store of g_bProgressiveScan is reachable; measured harmless to the unit's existing C (task #889)
      */usa/text/1FFBA0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B bolt economy + turret weapon
      */usa/text/24D728.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE D GUI/camera helpers
      # EU TEXT RE-TILE (Phase A, 2026-06-14): EU twins of the 11 USA text c-units.
      */eu/text/16E7B8.c) GFLAG="-G8";;                          # USA 16E980 twin
      */eu/text/183088.c) GFLAG="-G8";;                          # USA 183178 twin
      */eu/text/188470.c) GFLAG="-G8";;                          # USA 188580 twin
      */eu/text/190808.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1907F0 twin
      */eu/text/198B58.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 198FA0 twin
      */eu/text/1A7D10.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1A8180 twin
      */eu/text/1C9F58.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1CA080 twin
      */eu/text/1D5488.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1D54C0 twin
      */eu/text/236ED8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing";;    # USA 235FE8 twin
      */eu/text/249FE8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 248B50 twin
      */eu/text/251520.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 250080 twin
      # REGION-AXIS CARVE (2026-06-15): EU twins of the 3 recent USA text carves.
      */eu/text/188748.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 188858 twin
      */eu/text/191240.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 191238 twin
      */eu/text/19FC78.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1A00F0 twin
    esac
    S136EXTRA="${S136EXTRA-$CC1EXTRA}"
    # PER-UNIT intermediates (next to the object), NOT a shared $BUILD/_unit.s:
    # under qemu virtio-9p a rewritten same-PATH scratch file can serve STALE
    # cached content to the subsequent `as` read, so a shared _unit.s let one
    # unit's cc1 output be assembled into ANOTHER unit's object (cross-
    # contaminated objects -> PC16 branch truncations at link). A unique path
    # per unit is never rewritten, so the 9p cache cannot alias across units.
    ui="${o%.o}._u.i"; us="${o%.o}._u.s"
    # FAIL-LOUD: clear stale object + intermediates first so a failed compile can
    # NEVER leave a stale .o behind; abort non-zero on ANY step error; verify the
    # object actually materialized. (A silent stale .o = false 'byte-exact'/boot.)
    rm -f "$o" "$ui" "$us"
    sh tools/ee/ee_cc1.sh sdk29 "$c" "$ui" "$us" "$CPPDEF $INCC" "-O2 $GFLAG $CC1EXTRA" \
      || { echo "BUILD FAIL (compile): $c" >&2; exit 1; }
    # s136os arm (task #1257): the unit's tools/ee/s136os_functions.txt rows are
    # compiled alone by SN 2.95.3 v1.36 -fopt-stack and spliced over their
    # S136OS_SLOT lines; a unit with neither is untouched. Shared with
    # objdiff_build.sh — the helper is the one copy.
    u="${c#$SRC/}"
    sh tools/ee/s136os_splice.sh "$REGION" "${u%.*}" "$c" "$us" "$GFLAG" "$S136EXTRA" \
      || { echo "BUILD FAIL (s136os splice): $c" >&2; exit 1; }
    sh tools/ee/asm_unit.sh "$REGION" "/work/$us" "/work/$o" "$GFLAG" \
      || { echo "BUILD FAIL (as): $c" >&2; exit 1; }
    [ -s "$o" ] || { echo "BUILD FAIL (no object produced): $c" >&2; exit 1; }
    mips-linux-gnu-strip "$o" -N dummy-symbol-name 2>/dev/null || true
    m=$((m+1))
  done
  echo "   compiled $m c units"
fi

# 3) libgcc.a: the members the .ld links as splat `lib` subsegments, built from
#    GCC's own verbatim source in going-decompiled/libgcc/ (RULING #8206). A
#    region whose .ld names none (EU today) builds nothing and links as before.
sh tools/ee/build_libgcc.sh "$REGION"

echo "== [$REGION] linking with $LD =="
SYMS="$BUILD/undefined_syms_auto.txt"
# Blanket-define every D_<hex> symbol to its absolute address (spimdisasm names
# auto-symbols by address). Resolves references it didn't emit labels for.
ALLSYMS="$BUILD/all_addr_syms.ld"
grep -rhoE '(D_|func_)[0-9A-Fa-f]{4,}' "$ASM" | sort -u | sed -E 's/^(D_|func_)([0-9A-Fa-f]+)$/\1\2 = 0x\2;/' > "$ALLSYMS"
# Old address-named spellings the C sources still use for symbols symbol_addrs
# has renamed (task #1255); see tools/ee/src_alias_provides.sh.
sh tools/ee/src_alias_provides.sh "$REGION" >> "$ALLSYMS"
# jtbl_<hex>_text jump-table symbols (address encoded in the name): splat
# references them by name from the code but does not emit a label, and the
# D_/func_ blanket above does not cover them. Define each at its named address.
grep -rhoE 'jtbl_[0-9A-Fa-f]+_text' "$ASM" | sort -u \
  | sed -E 's/^jtbl_([0-9A-Fa-f]+)_text$/jtbl_\1_text = 0x\1;/' >> "$ALLSYMS"
# Computed-referenced .L<hex> local labels: splat keeps branch/jump-table targets
# local, but a %hi/%lo/.word reference to one can be CROSS-UNIT (the label is
# defined in another unit, where it stays local -> undefined in the referrer).
# Define each at its named address (address-in-name). Link-only; any within-unit
# duplicate is address-consistent under --allow-multiple-definition.
grep -rhoE '%(hi|lo)\(\.L[0-9A-Fa-f]+\)|\.word[[:space:]]+\.L[0-9A-Fa-f]+' "$ASM" \
  | grep -oE '\.L[0-9A-Fa-f]+' | sort -u \
  | sed -E 's/^\.L([0-9A-Fa-f]+)$/.L\1 = 0x\1;/' >> "$ALLSYMS"
# Also branch-operand .L<hex> refs (b/beq/bltz/...): a CROSS-SPLIT-BOUNDARY branch
# can target a label defined in an adjacent unit (where it stays local) -> undefined
# in the branching unit. Define those too (address-in-name). Within-unit duplicates
# are address-consistent; a too-far branch would already have failed in the original.
grep -rhoE '[[:space:],]\.L[0-9A-Fa-f]{6,8}([[:space:]]|$)' "$ASM" \
  | grep -oE '\.L[0-9A-Fa-f]+' | sort -u \
  | sed -E 's/^\.L([0-9A-Fa-f]+)$/.L\1 = 0x\1;/' >> "$ALLSYMS"
# libgcc.a (step 3): the archive the .ld's `libgcc.a:<member>.o(...)` lines pick
# sections from. An `archive:member` pattern only matches an archive that is an
# input, so it is INPUT here, and EXTERN makes each member's global symbols
# undefined up front so the member is pulled out of the archive whatever
# references it. Written into this file, not onto the ld command line, so every
# relink of it (landing_gate.sh's PROVIDE check) links the same library.
LIBGCC="$BUILD/lib/libgcc.a"
LIBSYMS=""
if [ -f "$LIBGCC" ]; then
  for m in $(cat "$BUILD/lib/members.txt"); do
    LIBSYMS="$LIBSYMS $(mips-linux-gnu-nm -g --defined-only "$BUILD/lib/$m.o" | awk '{print $3}')"
  done
  echo "INPUT($LIBGCC)" >> "$ALLSYMS"
  for sym in $LIBSYMS; do echo "EXTERN($sym);" >> "$ALLSYMS"; done
  # Names a member references that the game defines under another name
  # (going-decompiled/libgcc/LINK_ALIASES says why they are not symbol_addrs rows).
  grep -vE '^[[:space:]]*(#|$)' going-decompiled/libgcc/LINK_ALIASES \
    | awk 'NF == 2 { print $1 " = " $2 ";" }' >> "$ALLSYMS"
fi
# Track-B NAMED symbols (snd_PrintError, AssertFail, Mc*, WrapAngle*, rand, ...)
# from symbol_addrs: matched C calls these by name, but they are not address-named
# so the blanket misses them. PROVIDE name=addr (first-wins/only-if-undefined, so
# it never clashes with a real definition). Mirrors run_state_suite.sh.
SYMADDR="going-decompiled/symbol_addrs/$REGION/symbol_addrs.txt"
# A library-defined name gets NO PROVIDE: ld resolves a PROVIDE before it
# searches the archive, so a PROVIDE(__divdi3 = 0x11FC68) left the member
# unextracted and the lib's .text/.rodata placed nothing (measured, task #879).
LIBSYMS_RE=$(printf '%s\n' $LIBSYMS | sed '/^$/d' | paste -sd'|' -)
[ -f "$SYMADDR" ] && sed -nE 's@^[[:space:]]*([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*=[[:space:]]*(0x[0-9A-Fa-f]+).*@PROVIDE(\1 = \2);@p' "$SYMADDR" \
  | { if [ -n "$LIBSYMS_RE" ]; then grep -v -E "^PROVIDE\(($LIBSYMS_RE) = "; else cat; fi; } >> "$ALLSYMS"
echo "   defined $(wc -l < "$ALLSYMS") address symbols (D_/func_/jtbl_ + symbol_addrs PROVIDE + libgcc INPUT/EXTERN)"
# FAIL-LOUD: clear stale link outputs so a failed/partial link can NEVER be
# mistaken for a fresh success (the `|| {…}` below only REPORTS ld errors; the
# `[ -f "$ELFLMA" ]` gate then operates on a guaranteed-fresh file).
# ORDERING IS LOAD-BEARING: $ELFLMA must be assigned BEFORE this rm.
# It used to be set ~11 lines below, so the rm expanded to an empty word and
# the stale .lma.elf SURVIVED -- a failed re-link then reused it, printed
# "ELF built", and reported a differing-byte count from the PREVIOUS link.
# $ROM is regenerated from it below, so a fresh .rom mtime proves nothing.
ELFLMA="$BUILD/$BASENAME.lma.elf"
rm -f "$ELF" "$ELFLMA" "$ROM"
# ENTRY POINT: the splat-generated .ld carries no ENTRY() (splat 0.41 has no
# option for one), so without -e ld defaulted to the first .text section
# (e_entry 0x0026EA00, the start of text/16E980) and the BIOS would have
# jumped into the middle of the engine. The retail header says 0x00131AE8 =
# _start (crt0), link-defined in cod/015180 for USA and PROVIDEd from
# symbol_addrs for EU. This lives in the ELF header only: the .rom (a flat
# objcopy of the LMA==VMA link below) is unaffected, so cmp vs retail stays 0.
mips-linux-gnu-ld -EL --allow-multiple-definition -e _start -T "$LD" -T "$SYMS" -T "$ALLSYMS" -Map "$BUILD/$BASENAME.map" -o "$ELF" 2> "$BUILD/ld.log" \
  || { echo "LD errors (first 20):"; head -20 "$BUILD/ld.log"; }

# Second link for .rom flattening: identical, but with a linker script whose
# load addresses equal the virtual addresses (LMA == VMA). splat's script packs
# sections by LMA via `AT(<seg>_ROM_START)`/`__romPos`, which drops the bss/gap
# zeros and does NOT match the original rom layout. Stripping the `AT(...)`
# clauses makes ld default LMA = VMA, so objcopy -O binary produces the
# VMA-contiguous, zero-gap-filled image the original .rom actually is.
LDLMA="$BUILD/$BASENAME.lma.ld"
sed -E 's/ AT\([A-Za-z0-9_]+\)//g' "$LD" > "$LDLMA"
mips-linux-gnu-ld -EL --allow-multiple-definition -e _start -T "$LDLMA" -T "$SYMS" -T "$ALLSYMS" -o "$ELFLMA" 2> "$BUILD/ld.lma.log" \
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
  # FAIL LOUD. Without this the script returns 0 on a DEAD LINK, so every
  # caller reading exit status sees success and a non-linking tree can be
  # landed with its gates green -- exactly how bb754675 reached master.
  # DEMONSTRATED, not asserted: #22557 observed armA/runner.log exit=0 on a
  # link that produced no ELF at all. Reported as #22826.
  exit 1
fi

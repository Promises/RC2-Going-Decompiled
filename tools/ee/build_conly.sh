#!/bin/sh
# build_conly.sh — ONE-OFF "C-only boot" probe build (do NOT edit; copy of
# build.sh + a TARGET_NATIVE C-alt overlay). Runs INSIDE the ee-build container:
#
#   tools/ee/vm.sh 'sh tools/ee/build_conly.sh usa'
#
# Goal: produce an EE ELF where every function that HAS a portable-C alternative
# (#else arm under #ifndef TARGET_NATIVE) is built from that C instead of its
# original INCLUDE_ASM, while every other function (and _start/crt0) stays asm.
#
# Mechanics:
#   1) assemble the section .s files                          (verbatim build.sh)
#   2) compile each src/<region>/**/*.c NORMALLY (INCLUDE_ASM arm)  (verbatim)
#   2b) compile each src unit that contains >=1 `^#else` a SECOND time WITH
#       -DTARGET_NATIVE -> $BUILD/<path>.calt.o (ONLY its #else funcs, as C).
#       Recipe mirrors tools/ee/eetest/run_state_suite.sh's proven cc(): cpp
#       (+ -Itools/ee/eetest/shim), cc1 -O2, SN as.exe.
#   3) LINK. build.sh pulls objects IN VIA THE .ld (none on the ld cmdline) and
#      ends with `/DISCARD/ : { *(*); }`, so a .calt.o merely *passed on the
#      cmdline* is an orphan -> DISCARDED (Attempt A below confirms this: no-op).
#      To actually make the C-alts WIN at the pinned addresses, the .calt.o must
#      be referenced in the .ld BEFORE the unit's normal .o (first def wins under
#      --allow-multiple-definition). Attempt B builds that modified .ld.
set -e
REGION="${1:-usa}"
cd "$(dirname "$0")/../.."

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
ELF=$BUILD/$BASENAME.conly.elf

ASFLAGS="-march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -I $INC -I $ASM -I $BUILD"
VU0FIX="$(dirname "$0")/vu0_fixup.sed"
SRC=going-decompiled/src/$REGION
WIBO=/usr/local/bin/wibo
G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
INCC="-Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common"
CPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DINCLUDE_ASM_USE_MACRO_INC=1"
# TARGET_NATIVE compile includes (mirror run_state_suite.sh BUILD_CMD cc()):
#   shim FIRST (overrides math.h/stdint.h), then the rtl include dirs incl iop.
NINCC="-Itools/ee/eetest/shim -Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common -Igoing-decompiled/include/rtl/iop -Itools/ee/eetest"
NCPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DTARGET_NATIVE"
SNAS=tools/ee/cc/ee/bin/as.exe

# C-ONLY build (Option 1, user-approved): NORMAL objects at per-unit -G8 (matching
# build.sh) so they are BYTE-EXACT (fixes the 0x1AA9C8->0x352D08 data divergence =
# the -G0 code-lengthening of the -G8 sub-TUs). The earlier da17f51 truncation was
# NOT the normal objects (their far globals already carry .extern,16 overrides ->
# absolute, e.g. g_levelDialogToc; g_savePromptLatch is in-window) — it was da17f51
# compiling the CALT (#else/TARGET_NATIVE) objects at -G8 too via the shared
# unit_flags; the portable #else arms lack those overrides so their %gp_rel reaches
# far globals -> GPREL16 truncation. FIX: NORMAL = per-unit -G8 (unit_flags.sh);
# CALT = forced -G0 (overlay at 0xC00000, functional not byte-matching -> no
# truncation). See progress/2026-06-27-conly-data-residual.md.
. "$(dirname "$0")/unit_flags.sh"

# 1) Assemble section .s files (verbatim from build.sh).
echo "== [$REGION] assembling section .s files =="
n=0
for s in $(find $ASM -name '*.s' -not -path '*/nonmatchings/*' -not -path '*/matchings/*'); do
  o="$BUILD/${s%.s}.o"
  mkdir -p "$(dirname "$o")"
  sed -f "$VU0FIX" "$s" | sed 's|"/[^"]*/going-decompiled/|"going-decompiled/|g' \
    | mips-linux-gnu-as $ASFLAGS -o "$o" - 2> "$o.log" || { echo "AS FAIL $s:"; tail -5 "$o.log"; exit 1; }
  n=$((n+1))
done
echo "   assembled $n section objects"

# 2) Compile each src unit NORMALLY (INCLUDE_ASM arm) -> the .o the .ld expects.
echo "== [$REGION] compiling src/ c units (normal/asm arm) =="
m=0
for c in $(find "$SRC" -name '*.c'); do
  o="$BUILD/${c%.c}.o"
  mkdir -p "$(dirname "$o")"
  unit_flags "$c"
  # FAIL-LOUD: clear stale object+intermediates, abort on ANY step error, verify
  # the object materialized — a silent stale .o = false 'byte-exact'/boot result.
  rm -f "$o" "$BUILD/_unit.i" "$BUILD/_unit.s"
  "$WIBO" "$G/cpp.exe" $CPPDEF $INCC "$c" "$BUILD/_unit.i" \
    || { echo "BUILD FAIL (cpp): $c" >&2; exit 1; }
  "$WIBO" "$G/cc1.exe" -quiet -O2 $GFLAG $CC1EXTRA "$BUILD/_unit.i" -o "$BUILD/_unit.s" \
    || { echo "BUILD FAIL (cc1): $c" >&2; exit 1; }
  sh tools/ee/asm_unit.sh "$REGION" "/work/$BUILD/_unit.s" "/work/$o" "$GFLAG" \
    || { echo "BUILD FAIL (as): $c" >&2; exit 1; }
  [ -s "$o" ] || { echo "BUILD FAIL (no object produced): $c" >&2; exit 1; }
  mips-linux-gnu-strip "$o" -N dummy-symbol-name 2>/dev/null || true
  m=$((m+1))
done
echo "   compiled $m c units (normal)"

# 2b) Compile every #else-bearing unit a SECOND time WITH -DTARGET_NATIVE.
echo "== [$REGION] compiling C-alt (TARGET_NATIVE) objects =="
CALT_LIST="$BUILD/conly_calt_units.txt"
: > "$CALT_LIST"
calt_ok=0; calt_fail=0
for c in $(find "$SRC" -name '*.c'); do
  grep -q '^#else' "$c" || continue
  o="$BUILD/${c%.c}.calt.o"
  unit_flags "$c"; GFLAG="-G0"; CC1EXTRA=""  # CALT overlay: force -G0 (portable
  # #else arms lack the .extern,16 far-global overrides -> -G8 would %gp_rel-truncate;
  # the overlay is at 0xC00000, functional not byte-matching, so -G0 is correct).
  rm -f "$o"
  if ! "$WIBO" "$G/cpp.exe" $NCPPDEF $NINCC "$c" "$BUILD/_calt.i" 2> "$o.cpp.log"; then
    echo "   CALT CPP FAIL  $c"; tail -3 "$o.cpp.log"; calt_fail=$((calt_fail+1)); continue
  fi
  if ! "$WIBO" "$G/cc1.exe" -quiet -O2 $GFLAG $CC1EXTRA "$BUILD/_calt.i" -o "$BUILD/_calt.s" 2> "$o.cc1.log"; then
    echo "   CALT CC1 FAIL  $c"; tail -5 "$o.cc1.log"; calt_fail=$((calt_fail+1)); continue
  fi
  if ! "$WIBO" "$SNAS" -EL "$GFLAG" -o "$o" "$BUILD/_calt.s" 2> "$o.as.log"; then
    echo "   CALT AS  FAIL  $c"; tail -5 "$o.as.log"; calt_fail=$((calt_fail+1)); continue
  fi
  echo "$o" >> "$CALT_LIST"
  calt_ok=$((calt_ok+1))
done
echo "   C-alt objects: $calt_ok ok, $calt_fail failed"

# Common symbol scripts (verbatim build.sh).
SYMS="$BUILD/undefined_syms_auto.txt"
ALLSYMS="$BUILD/all_addr_syms.ld"
grep -rhoE '(D_|func_)[0-9A-Fa-f]{4,}' "$ASM" | sort -u | sed -E 's/^(D_|func_)([0-9A-Fa-f]+)$/\1\2 = 0x\2;/' > "$ALLSYMS"
echo "   defined $(wc -l < "$ALLSYMS") address symbols"

# ---- Attempt A: literal task recipe — same .ld, .calt.o passed on the ld
#      command line BEFORE everything, --allow-multiple-definition. Expected:
#      orphan -> /DISCARD/ -> no-op (documents WHY it cannot work as stated).
echo "== [$REGION] LINK Attempt A: unmodified .ld + .calt.o on cmdline =="
CALT_OBJS="$(cat "$CALT_LIST" 2>/dev/null | tr '\n' ' ')"
mips-linux-gnu-ld -EL --allow-multiple-definition -T "$LD" -T "$SYMS" -T "$ALLSYMS" \
  -Map "$BUILD/$BASENAME.conlyA.map" -o "$BUILD/$BASENAME.conlyA.elf" $CALT_OBJS \
  2> "$BUILD/ld.conlyA.log" || true
echo "   --- Attempt A ld.log (head) ---"; head -30 "$BUILD/ld.conlyA.log" || true

# ---- Attempt B: inject `<unit>.calt.o(<sec>)` BEFORE each `<unit>.o(<sec>)`
#      in the .ld so the C-alts are placed first and WIN. This is what actually
#      exercises the C-alts at the pinned addresses.
echo "== [$REGION] building C-alt-overlay .ld (Attempt B) =="
LDB="$BUILD/$BASENAME.conly.ld"
python3 - "$LD" "$LDB" "$CALT_LIST" "$BUILD" <<'PY'
import sys, re, os
src, dst, listf, build = sys.argv[1:5]
units = set()
if os.path.exists(listf):
    for ln in open(listf):
        ln = ln.strip()
        if not ln: continue
        # $BUILD/<srcpath>.calt.o  ->  <srcpath> stem (region/unit)
        units.add(ln[:-len('.calt.o')] if ln.endswith('.calt.o') else ln)
out = []
# match a placement line: <obj>.o(<secspec>);  where <obj> is a src unit path
pat = re.compile(r'^(\s*)(\S+/src/\S+?)\.o\((\.text\*|\.data\*|\.rodata\*|\.bss COMMON \.scommon)\);\s*$')
ins = 0
for line in open(src):
    m = pat.match(line)
    if m:
        indent, obj, sec = m.group(1), m.group(2), m.group(3)
        calt = obj + '.calt.o'
        if calt in units:
            out.append("%s%s.calt.o(%s);\n" % (indent, obj, sec))
            ins += 1
    out.append(line)
open(dst, 'w').write(''.join(out))
print("   injected %d calt placement lines for %d units" % (ins, len(units)))
PY

echo "== [$REGION] LINK Attempt B: C-alt-overlay .ld =="
# FAIL-LOUD: clear stale ELF/rom so the `[ -s "$ELF" ]` gate below can NEVER pass
# on a previous run's image when THIS link fails.
rm -f "$ELF" "$BUILD/$BASENAME.conly.rom"
mips-linux-gnu-ld -EL --allow-multiple-definition -T "$LDB" -T "$SYMS" -T "$ALLSYMS" \
  -Map "$BUILD/$BASENAME.conly.map" -o "$ELF" 2> "$BUILD/ld.conly.log" || true
echo "   --- Attempt B ld.log (head) ---"; head -40 "$BUILD/ld.conly.log" || true

if [ -s "$ELF" ]; then
  echo "== [$REGION] Attempt B produced an ELF: $ELF =="
  ls -l "$ELF"
  mips-linux-gnu-objcopy -O binary "$ELF" "$BUILD/$BASENAME.conly.rom" 2>/dev/null || true
  [ -f "$BUILD/$BASENAME.conly.rom" ] && ls -l "$BUILD/$BASENAME.conly.rom"
  echo -n "orig rom size: "; wc -c < "$ORIG" 2>/dev/null || echo "?"
else
  echo "== [$REGION] Attempt B produced NO ELF (link failed) =="
fi

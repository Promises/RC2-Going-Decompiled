#!/bin/sh
# build_conly.sh — ONE-OFF "C-only boot" probe build (EDITED 2026-08-01 under direct human authorisation; was "do NOT edit". Copy of
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
  # PER-UNIT intermediates (next to the object), NOT a shared $BUILD/_unit.s:
  # under qemu virtio-9p a rewritten same-PATH scratch file can serve STALE
  # cached content to the subsequent `as` read, so a shared _unit.s let one
  # unit's cc1 output be assembled into ANOTHER unit's object (proven: linked
  # 183178.o held 1EFFC0's code, 1EFFC0.o held cod/0321A0's, etc -> PC16
  # branch truncations). A unique path per unit is never rewritten, so the 9p
  # cache cannot alias across units.
  ui="${o%.o}._u.i"; us="${o%.o}._u.s"
  # FAIL-LOUD: clear stale object+intermediates, abort on ANY step error, verify
  # the object materialized — a silent stale .o = false 'byte-exact'/boot result.
  rm -f "$o" "$ui" "$us"
  "$WIBO" "$G/cpp.exe" $CPPDEF $INCC "$c" "$ui" \
    || { echo "BUILD FAIL (cpp): $c" >&2; exit 1; }
  "$WIBO" "$G/cc1.exe" -quiet -O2 $GFLAG $CC1EXTRA "$ui" -o "$us" \
    || { echo "BUILD FAIL (cc1): $c" >&2; exit 1; }
  sh tools/ee/asm_unit.sh "$REGION" "/work/$us" "/work/$o" "$GFLAG" \
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
  # PER-UNIT intermediates (see NORMAL loop): a shared $BUILD/_calt.s can serve
  # stale 9p-cached content across units -> cross-contaminated .calt.o objects.
  ci="${o%.o}._c.i"; cs="${o%.o}._c.s"
  rm -f "$o" "$ci" "$cs"
  if ! "$WIBO" "$G/cpp.exe" $NCPPDEF $NINCC "$c" "$ci" 2> "$o.cpp.log"; then
    echo "   CALT CPP FAIL  $c"; tail -3 "$o.cpp.log"; calt_fail=$((calt_fail+1)); continue
  fi
  if ! "$WIBO" "$G/cc1.exe" -quiet -O2 $GFLAG $CC1EXTRA "$ci" -o "$cs" 2> "$o.cc1.log"; then
    echo "   CALT CC1 FAIL  $c"; tail -5 "$o.cc1.log"; calt_fail=$((calt_fail+1)); continue
  fi
  if ! "$WIBO" "$SNAS" -EL "$GFLAG" -o "$o" "$cs" 2> "$o.as.log"; then
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
        # Store the line VERBATIM. It is already "$BUILD/<stem>.calt.o", which is
        # exactly what the placement loop constructs below. Master stripped the
        # ".calt.o" here and then compared against obj+".calt.o" -> stem vs path,
        # never equal, so `ins` could not be anything but 0.
        # CONVERGED with 36007293 and 9869358c, which fix it this way and are the
        # only variants exercised by a real build. Not re-fixed a second way.
        units.add(ln)
out = []
# match a placement line: <obj>.o(<secspec>);  where <obj> is a src unit path
pat = re.compile(r'^(\s*)(\S+/src/\S+?)\.o\((\.text\*|\.data\*|\.rodata\*|\.bss COMMON \.scommon)\);\s*$')
ins = 0
by_spec = {}
for line in open(src):
    m = pat.match(line)
    if m:
        indent, obj, sec = m.group(1), m.group(2), m.group(3)
        calt = obj + '.calt.o'
        if calt in units:
            out.append("%s%s.calt.o(%s);\n" % (indent, obj, sec))
            ins += 1
            by_spec[sec] = by_spec.get(sec, 0) + 1
    out.append(line)
open(dst, 'w').write(''.join(out))
print("   injected %d calt placement lines for %d units" % (ins, len(units)))

# ---- DIAGNOSTIC, NOT AN ASSERTION -------------------------------------------
# Per-section breakdown and the per-unit rate. Measured 2026-08-01 in two
# independent trees: 3.00 in both (25/25/0/25 over 25 units; 26/26/0/26 over 26).
# The regex admits FOUR section specs and exactly THREE fire, because the base
# .ld carries no src `.rodata` placement line for the injector to insert before.
# So 3.00 is the BASE SCRIPT'S SECTION COVERAGE, not a property of either tree.
#
# DELIBERATELY NOT ASSERTED. If the base .ld ever gains a src `.rodata` line the
# rate legitimately becomes 4.00, and a hard `rate == 3` check would fail a
# correct build -- the manufacture-a-false-finding direction. Printed so a reader
# or a gate can notice a change; not enforced, so it cannot invent one.
#
# THE GENERAL TEST, for whoever is next tempted to tighten this (tester-m1, /5459,
# correcting its own suggestion to enforce it):
#
#   AN INVARIANT THAT IS TRUE TODAY AND CONTINGENT ON A DESIGN CHOICE IS A
#   DIAGNOSTIC, NOT A GATE.
#
# "Cheap" and "stronger than the raw count" are both true of a rate check and
# neither is the question. The question is what it does ON A CORRECT BUILD. The
# 3.00 is the base script's section coverage, and section coverage is a thing
# that may change on purpose.
if units:
    print("   per-section: " + " ".join(
        "%s=%d" % (s.split()[0], by_spec.get(s, 0))
        for s in ('.text*', '.data*', '.rodata*', '.bss COMMON .scommon')))
    print("   rate: %.2f placement lines per unit" % (float(ins) / len(units)))

# ---- STRUCTURAL GUARD: the emitted .ld MUST DIFFER from the base .ld ----------
# Keys on the HARM, not on a cause. An identical .ld makes "Attempt B" link the
# RETAIL IMAGE under the C-only name and exit 0 -- a clean-looking build that did
# nothing. Empty CALT_LIST, a wrong glob, silent injector failure, and a stem/path
# key mismatch ALL fail this one check; enumerating those causes is not required.
#
# Deliberately NOT keyed on `ins == 0`: `ins` is a SIGNATURE shared by the benign
# and the harmful case. Measured (decomper-2-m1): an empty list and a broken key
# both yield ins == 0 AND a byte-identical .ld, from different causes -- so a
# guard on `ins` provably cannot separate them and one on the OUTPUT provably can.
if open(dst, 'rb').read() == open(src, 'rb').read():
    sys.stderr.write(
        "FATAL: emitted %s is BYTE-IDENTICAL to base %s\n"
        "       Attempt B would link the RETAIL IMAGE under the C-only name.\n"
        "       injected=%d units=%d\n" % (dst, src, ins, len(units)))
    sys.exit(3)
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

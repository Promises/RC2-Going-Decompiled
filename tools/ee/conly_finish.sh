#!/bin/sh
# conly_finish.sh — finish the C-only probe: REUSE the normal .o objects already
# built by build_conly.sh's phase-2, then do ONLY (2b) the TARGET_NATIVE C-alt
# compile and (3) the Attempt-B overlay link. Runs INSIDE the ee-build VM:
#   tools/ee/vm.sh 'sh tools/ee/conly_finish.sh usa'
set -u
REGION="${1:-usa}"
cd "$(dirname "$0")/../.."
case "$REGION" in
  usa) BASENAME=SCUS_972.68 ;;
  eu)  BASENAME=SCES_516.07 ;;
  *) echo "unknown region $REGION"; exit 2 ;;
esac
ASM=going-decompiled/asm/$REGION
BUILD=going-decompiled/build/$REGION
LD=going-decompiled/linker_scripts/$BASENAME.ld
ORIG=extracted/$REGION/$BASENAME.rom
ELF=$BUILD/$BASENAME.conly.elf
SRC=going-decompiled/src/$REGION
WIBO=/usr/local/bin/wibo
G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
NINCC="-Itools/ee/eetest/shim -Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common -Igoing-decompiled/include/rtl/iop -Itools/ee/eetest"
NCPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DTARGET_NATIVE"
SNAS=tools/ee/cc/ee/bin/as.exe

unit_flags() {
  # C-ONLY build: ALL units at -G0 (per 54a809a) — dissolves the gp_rel overflow.
  # The C-only image is functional, not byte-matching, so no -G8 small-data needed.
  GFLAG="-G0"; CC1EXTRA=""
}

echo "== [$REGION] (2b) compiling C-alt (TARGET_NATIVE) objects =="
CALT_LIST="$BUILD/conly_calt_units.txt"; : > "$CALT_LIST"
calt_ok=0; calt_fail=0
for c in $(find "$SRC" -name '*.c'); do
  /usr/bin/grep -q '^#else' "$c" || continue
  o="$BUILD/${c%.c}.calt.o"; unit_flags "$c"; rm -f "$o"
  if ! "$WIBO" "$G/cpp.exe" $NCPPDEF $NINCC "$c" "$BUILD/_calt.i" 2> "$o.cpp.log"; then
    echo "  CALT CPP FAIL $c"; tail -2 "$o.cpp.log"; calt_fail=$((calt_fail+1)); continue; fi
  cs="$BUILD/${c%.c}.calt.s"; mkdir -p "$(dirname "$cs")"
  if ! "$WIBO" "$G/cc1.exe" -quiet -O2 $GFLAG $CC1EXTRA "$BUILD/_calt.i" -o "$cs" 2> "$o.cc1.log"; then
    echo "  CALT CC1 FAIL $c"; tail -4 "$o.cc1.log"; calt_fail=$((calt_fail+1)); continue; fi
  # Assemble with GNU as (NOT SN as.exe) so GNU ld accepts the symtab. The
  # TARGET_NATIVE arm has no INCLUDE_ASM, so the .s is pure code — no macro ctx.
  # Apply move_fixup.sed (move->daddu, cvt.w.s->.word) like asm_unit.sh: binutils
  # 2.40 refuses cvt.w.s on r5900. (Byte-exactness fixups for -G8 are not needed
  # for a BOOTING build — only valid assembly is.)
  if ! sed -E -f tools/ee/move_fixup.sed "$cs" | mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL "$GFLAG" -o "$o" - 2> "$o.as.log"; then
    echo "  CALT AS FAIL $c"; tail -4 "$o.as.log"; calt_fail=$((calt_fail+1)); continue; fi
  echo "$o" >> "$CALT_LIST"; calt_ok=$((calt_ok+1))
done
echo "   C-alt objects: $calt_ok ok, $calt_fail failed"

SYMS="$BUILD/undefined_syms_auto.txt"
ALLSYMS="$BUILD/all_addr_syms.ld"
/usr/bin/grep -rhoE '(D_|func_)[0-9A-Fa-f]{4,}' "$ASM" | sort -u | sed -E 's/^(D_|func_)([0-9A-Fa-f]+)$/\1\2 = 0x\2;/' > "$ALLSYMS"

echo "== [$REGION] (3) building C-alt-overlay .ld (isolated catch-all) =="
LDB="$BUILD/$BASENAME.conly.ld"
# Isolate the calt objects into a FIXED high-VRAM catch-all placed FIRST in
# SECTIONS{} (script-order-first wins symbol resolution under
# --allow-multiple-definition), leaving the base layout VERBATIM so every
# raw-asm %gp_rel symbol keeps its clean-link address (no gp-window drift).
# See tools/ee/conly_overlay_ld.py for the full rationale.
python3 tools/ee/conly_overlay_ld.py "$LD" "$CALT_LIST" "$LDB" 0x00C00000

echo "== [$REGION] (3) LINK Attempt B: overlay .ld, first-def-wins =="
# conly_provides.ld: link-only PROVIDE aliases for the compiler-runtime helpers
# (__divdi3/.../fptodp) the calt objects reference; NOT symbol_addrs (would break
# matches). Add the tester's symbol -T scripts (ghidra_named_funcs.ld, arena map,
# symbol_addrs PROVIDE) to this command as needed — they compose orthogonally.
mips-linux-gnu-ld -EL --allow-multiple-definition \
  -T "$LDB" -T tools/ee/conly_provides.ld -T "$SYMS" -T "$ALLSYMS" \
  -Map "$BUILD/$BASENAME.conly.map" -o "$ELF" 2> "$BUILD/ld.conly.log" || true
echo "   --- ld.conly.log (head 40) ---"; head -40 "$BUILD/ld.conly.log"
echo "   --- ld.conly.log line count: $(wc -l < "$BUILD/ld.conly.log") ---"
if [ -s "$ELF" ]; then
  echo "== LINK OK: $ELF =="; ls -l "$ELF"
  mips-linux-gnu-objcopy -O binary "$ELF" "$BUILD/$BASENAME.conly.rom" 2>/dev/null || true
  [ -f "$BUILD/$BASENAME.conly.rom" ] && { echo -n "conly rom: "; wc -c < "$BUILD/$BASENAME.conly.rom"; }
  echo -n "orig  rom: "; wc -c < "$ORIG"
else
  echo "== LINK FAILED: no ELF produced =="
fi

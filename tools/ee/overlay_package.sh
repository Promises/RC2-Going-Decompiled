#!/bin/sh
# overlay_package.sh — HOST orchestrator: build + package the C-only OVERLAY boot
# image (calt @0xC00000) from already-compiled normal .o, then cmp vs original.
#
# Assumes build_conly.sh has ALREADY produced the normal .o (its python3 .ld step
# fails in the VM — harmless, we only need the .o + CALT_LIST). This script then:
#   1) (VM) rebuild calt objects via GNU-as -G0 (conly_finish 2b) so GNU ld accepts
#      the symtab.
#   2) (HOST) generate overlay .ld (conly_overlay_ld.py, catch-all @0xC00000 +
#      SUBALIGN(8) + .cod_bss pin) from the ORIGINAL .ld WITH AT, then strip AT so
#      LMA==VMA.
#   3) (HOST) generate PROVIDE symbol scripts (all_addr_syms + named globals).
#   4) (VM) link (-N omagic, entry 0x131ae8).
#   5) (HOST) elf_contiguous_repack.py -> bootable contiguous-PT_LOAD ELF.
#   6) (HOST) cmp vs original main segment (expect 0 except ->0xC00000 calt ptrs).
set -u
REGION=usa
BASENAME=SCUS_972.68
cd "$(dirname "$0")/../.."
BUILD=going-decompiled/build/$REGION
ASM=going-decompiled/asm/$REGION
SRC=going-decompiled/src/$REGION
LD=going-decompiled/linker_scripts/$BASENAME.ld
ORIG=extracted/$REGION/$BASENAME.rom
SA=going-decompiled/symbol_addrs/$REGION
CALT_LIST=$BUILD/conly_calt_units.txt
ELF=$BUILD/$BASENAME.overlay.elf
REPACK=$BUILD/$BASENAME.overlay.repack.elf

echo "== (1) VM: rebuild calt objects via GNU-as -G0 =="
# Skip if GNU-as calt objects already built (SKIP_CALT=1 to force-skip).
if [ "${SKIP_CALT:-0}" = "1" ] && [ -s "$CALT_LIST" ]; then
  echo "   SKIP_CALT — reusing $(wc -l < "$CALT_LIST") existing calt objects"
else
  tools/ee/vm.sh 'sh tools/ee/conly_finish.sh usa' > /tmp/overlay_calt.log 2>&1 || true
  grep -E 'C-alt objects:' /tmp/overlay_calt.log || true
fi
calt_n=$(wc -l < "$CALT_LIST" 2>/dev/null || echo 0)
echo "   CALT_LIST has $calt_n objects"

echo "== (2) HOST: generate overlay .ld (catch-all 0xC00000 + SUBALIGN(8)) =="
# EXCLUDE units whose #else arms are non-bootable on EE: text/183558 is the VU0
# vector-math unit — its portable #else bodies are scalar stand-ins for VU0
# hardware (strictly worse than the real microcode) and reference float libcalls
# (sinf/cosf/sqrtf/asinf/atan2f) that have no linkable EE symbol. Those functions
# MUST keep their original asm (real VU0). This is NOT a coverage compromise to
# fit the slot — the #else arms are non-functional-on-EE by design.
CALT_EXCLUDE="${CALT_EXCLUDE:-text/183558}"
CALT_LIST_USE="$BUILD/conly_calt_units.overlay.txt"
if [ "${NULL_OVERLAY:-0}" = "1" ]; then
  # control: empty calt set -> NO redirects -> byte-exact base in the same
  # 2-PT_LOAD repacked+relocated packaging (isolates base/packaging faults from
  # calt-body faults).
  : > "$CALT_LIST_USE"
  echo "   NULL_OVERLAY — empty calt set (control: byte-exact base packaging)"
else
  cp "$CALT_LIST" "$CALT_LIST_USE"
  for ex in $CALT_EXCLUDE; do
    grep -v "/$ex\." "$CALT_LIST_USE" > "$CALT_LIST_USE.tmp" && mv "$CALT_LIST_USE.tmp" "$CALT_LIST_USE"
  done
fi
echo "   excluded [$CALT_EXCLUDE] -> $(wc -l < "$CALT_LIST_USE") calt units in overlay (was $(wc -l < "$CALT_LIST"))"
/usr/bin/python3 tools/ee/conly_overlay_ld.py "$LD" "$CALT_LIST_USE" "$BUILD/overlay_AT.ld" ${CALT_VRAM:-0x00C00000}
# strip AT() -> LMA==VMA (SUBALIGN clause survives the strip)
sed -E 's/ AT\([A-Za-z0-9_]+\)//g' "$BUILD/overlay_AT.ld" > "$BUILD/overlay_lma.ld"

echo "== (3) HOST: PROVIDE symbol scripts =="
# all_addr_syms: PROVIDE form (NOT plain assignment — plain overrides object-local
# defs and breaks intra-unit PC16 branches). func_/D_ from asm AND src (the calt
# #else C references address-named data globals D_<hex> that never appear in .s) +
# jtbl + .L labels. D_<hex>/func_<hex> are address-named so 0x<hex> is the address.
/usr/bin/grep -rhoE '(D_|func_)[0-9A-Fa-f]{4,}' "$ASM" "$SRC" | sort -u \
  | sed -E 's/^(D_|func_)([0-9A-Fa-f]+)$/PROVIDE(\1\2 = 0x\2);/' > "$BUILD/aas_prov.ld"
/usr/bin/grep -rhoE 'jtbl_[0-9A-Fa-f]+_text' "$ASM" | sort -u \
  | sed -E 's/^(jtbl_([0-9A-Fa-f]+)_text)$/PROVIDE(\1 = 0x\2);/' >> "$BUILD/aas_prov.ld"
/usr/bin/grep -rhoE '\.L[0-9A-Fa-f]{6,}' "$ASM" | sort -u \
  | sed -E 's/^\.L([0-9A-Fa-f]+)$/PROVIDE(.L\1 = 0x\1);/' >> "$BUILD/aas_prov.ld"
echo "   aas_prov.ld: $(wc -l < "$BUILD/aas_prov.ld") PROVIDEs"
# named globals from symbol_addrs (g_*, sce*, etc.) + manual gui globals.
cat "$SA"/symbol_addrs.txt "$SA"/cheat_globals.staged.txt 2>/dev/null \
  | sed -E 's@//.*$@@' \
  | grep -E '^[A-Za-z_][A-Za-z0-9_]* *= *0x[0-9A-Fa-f]+' \
  | sed -E 's/^([A-Za-z_][A-Za-z0-9_]*) *= *(0x[0-9A-Fa-f]+).*/PROVIDE(\1 = \2);/' \
  | sort -u > "$BUILD/named_prov.ld"
printf 'PROVIDE(g_guiTintEnabled = 0x1AD9CC);\nPROVIDE(g_guiTintRgb = 0x1AD9D0);\n' >> "$BUILD/named_prov.ld"
echo "   named_prov.ld: $(wc -l < "$BUILD/named_prov.ld") PROVIDEs"

echo "== (4) VM: link overlay image =="
# Symbol scripts: aas_conly.ld vs the freshly generated trio. The choice is a
# FRESHNESS test against the generating inputs, NOT an existence test — the old
# `[ -s ... ]` was satisfied by a two-month-old file and unsatisfied only by a
# missing one. Sourced (not run) so it can set SYMS_LD; it is a separate file so
# the three arms can be exercised without a VM or a linker.
# See tools/ee/overlay_syms_select.sh for the defect and for what it does NOT fix.
. tools/ee/overlay_syms_select.sh
rm -f "$ELF"
tools/ee/vm.sh "mips-linux-gnu-ld -EL -N --allow-multiple-definition -e 0x00131ae8 \
  -T $BUILD/overlay_lma.ld -T tools/ee/conly_provides.ld \
  -T $BUILD/undefined_syms_auto.txt $SYMS_LD \
  -Map $BUILD/$BASENAME.overlay.map -o $ELF" > /tmp/overlay_link.log 2>&1 || true
echo "   --- link log (head 30) ---"; head -30 /tmp/overlay_link.log
if [ ! -s "$ELF" ]; then echo "!! LINK FAILED — no ELF"; exit 1; fi
echo "   link OK: $(ls -l "$ELF" | awk '{print $5}') bytes"

echo "== (5) HOST: contiguous repack =="
/usr/bin/python3 tools/ee/elf_contiguous_repack.py "$ELF" "$REPACK"
echo "   repack: $(wc -c < "$REPACK") bytes (orig slot 2621256)"

echo "== (6) HOST: cmp vs original main segment =="
/usr/bin/python3 - "$REPACK" "$ORIG" <<'PY'
import sys,struct
elf=open(sys.argv[1],'rb').read(); orig=open(sys.argv[2],'rb').read()
# objcopy-equivalent: load PT_LOAD of the MAIN region (vaddr 0x100080) to a flat
# image at file offset 0, compare against orig .rom. The .rom is the loadable
# flat image starting at the MAIN PT_LOAD vaddr 0x100080 (header-stripped:
# .rom[0]==vaddr 0x100080, NOT 0x100000), so base MUST be 0x100080 — using
# 0x100000 mis-aligns by the 0x80 ELF header and reports a bogus ~1.1M diffs.
# (NOTE: nonzero diffs here are EXPECTED — every calt-redirected reference
# (jal/%lo/.word retargeted into the 0x017Exxxx island) differs from retail; the
# authoritative base-exactness check is nm placed-addr-vs-original per function.)
en='<'
e_phoff,=struct.unpack_from('<I',elf,28); e_phnum,=struct.unpack_from('<H',elf,44)
diffs=0; calt_ptr=0; first=None; base=0x100080
for i in range(e_phnum):
    o=e_phoff+i*32
    p_type,p_off,p_vaddr,p_paddr,p_filesz,p_memsz,_,_=struct.unpack_from('<IIIIIIII',elf,o)
    if p_type!=1: continue
    if p_vaddr>=0x00400000: continue  # calt/DVP overlay regions (above main image)
    seg=elf[p_off:p_off+p_filesz]
    for j in range(p_filesz):
        oa=p_vaddr-base+j
        if oa<0 or oa>=len(orig): continue
        if seg[j]!=orig[oa]:
            diffs+=1
            if first is None: first=p_vaddr+j
print("MAIN-segment diffs vs original:", diffs, "first@",hex(first) if first else None)
PY
echo "== DONE =="
#!/bin/sh
# overlay_probe.sh — after a per-function #else edit in a calt unit, rebuild calt
# (no SKIP_CALT, so the edit is picked up), relink the overlay at 0x017E0000,
# relocate into the ISO, boot, wait, and print the 0x29e620-trap verdict.
# For per-function bisection of the post-pad-config wild-pointer fault.
set -u
cd "$(dirname "$0")/../.."
EMU="${EMU:-$HOME/Library/Application Support/PCSX2/logs/emulog.txt}"
PRISTINE="source-isos/Ratchet & Clank - Going Commando (USA) (v2.00).iso"
echo "== rebuild calt + relink overlay =="
CALT_VRAM=0x017E0000 sh tools/ee/overlay_package.sh > /tmp/probe.log 2>&1
grep -E 'C-alt objects|link OK|repack:' /tmp/probe.log | tail -3
[ -s going-decompiled/build/usa/SCUS_972.68.overlay.repack.elf ] || { echo "!! no repack ELF"; tail -8 /tmp/probe.log; exit 1; }
/usr/bin/python3 tools/ee/iso_relocate_boot.py "$PRISTINE" \
  going-decompiled/build/usa/SCUS_972.68.overlay.repack.elf /tmp/rc2_probe.iso SCUS >/dev/null 2>&1
pkill -f "PCSX2-v2.6.3" 2>/dev/null; sleep 3
: > "$EMU"
open -a "/Applications/PCSX2-v2.6.3.app" --args -- /tmp/rc2_probe.iso
echo "booted; waiting 27s for the fault window..."
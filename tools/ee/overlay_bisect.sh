#!/bin/sh
# overlay_bisect.sh — relink the C-only overlay excluding a given unit set, boot
# it, and print the emulog verdict (did the 0x29e620 trap clear?). Reuses the
# already-compiled normal+calt objects (SKIP_CALT). For pinpointing which calt
# #else body causes the post-pad-config wild-pointer trap.
#   tools/ee/overlay_bisect.sh "text/183558 text/1CA080 ..."   (space-sep excludes)
set -u
cd "$(dirname "$0")/../.."
EXCL="$1"
PRISTINE="source-isos/Ratchet & Clank - Going Commando (USA) (v2.00).iso"
EMU="~/Library/Application Support/PCSX2/logs/emulog.txt"
echo "== relink excluding: $EXCL =="
CALT_VRAM=0x017E0000 CALT_EXCLUDE="$EXCL" SKIP_CALT=1 sh tools/ee/overlay_package.sh > /tmp/bisect_pkg.log 2>&1
grep -E 'excluded|link OK|repack:' /tmp/bisect_pkg.log | tail -3
/usr/bin/python3 tools/ee/iso_relocate_boot.py "$PRISTINE" \
  going-decompiled/build/usa/SCUS_972.68.overlay.repack.elf /tmp/rc2_bisect.iso SCUS >/dev/null 2>&1
pkill -f PCSX2 2>/dev/null; sleep 2
: > "$EMU"
open -a "/Applications/PCSX2-v2.6.3.app" --args -- /tmp/rc2_bisect.iso
echo "booting; waiting 26s..."
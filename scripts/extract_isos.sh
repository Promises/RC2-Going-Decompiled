#!/usr/bin/env bash
# Extract the boot executables from both source ISOs and produce the flat
# ".rom" images that splat consumes. Idempotent; safe to re-run.
#
#   usa      : Ratchet & Clank - Going Commando (USA) (v2.00)  -> SCUS_972.68  (PRIMARY)
#   eu       : Ratchet & Clank 2 (Europe, Australia)           -> SCES_516.07  (region validator)
#   usa_v101 : Ratchet & Clank - Going Commando (USA) (v1.01)  -> SCUS_972.68  (version reference)
#
# v1.01 shares the same boot name/serial as v2.00 (only the disc content/CRC
# differs); it extracts to a separate extracted/usa_v101/ dir.
#
# Optional CLI args limit which regions to extract (default: all):
#   scripts/extract_isos.sh usa_v101
#
# Requires bash >= 4 (associative arrays), plus:
#   - bsdtar (libarchive). It reads ISO9660 directly, with no mount. macOS ships
#     it as /usr/bin/bsdtar (and `tar`). On Debian/Ubuntu: apt install libarchive-tools.
#   - an objcopy that reads 32-bit little-endian MIPS ELF: llvm-objcopy
#     (brew install llvm), or mips-linux-gnu-objcopy (apt install
#     binutils-mips-linux-gnu). Override with OBJCOPY=...
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ISO_DIR="$ROOT/source-isos"

find_tool() {  # first candidate that runs
  local c
  for c in "$@"; do
    if [ -n "$c" ] && command -v "$c" >/dev/null 2>&1; then echo "$c"; return 0; fi
  done
  return 1
}
BSDTAR="$(find_tool "${BSDTAR:-}" bsdtar)" \
  || { echo "ERROR: bsdtar not found (macOS: /usr/bin/bsdtar; Debian/Ubuntu: apt install libarchive-tools)"; exit 1; }
OBJCOPY="$(find_tool "${OBJCOPY:-}" llvm-objcopy /opt/homebrew/opt/llvm/bin/llvm-objcopy mips-linux-gnu-objcopy)" \
  || { echo "ERROR: no MIPS-capable objcopy (brew install llvm, or apt install binutils-mips-linux-gnu)"; exit 1; }

declare -a REGIONS=(usa eu usa_v101)
[ "$#" -gt 0 ] && REGIONS=("$@")
declare -A ISO=(
  [usa]="Ratchet & Clank - Going Commando (USA) (v2.00).iso"
  [eu]="Ratchet & Clank 2 (Europe, Australia) (En,Fr,De,Es,It).iso"
  [usa_v101]="Ratchet & Clank - Going Commando (USA) (v1.01).iso"
)
declare -A BOOT=(
  [usa]="SCUS_972.68"
  [eu]="SCES_516.07"
  [usa_v101]="SCUS_972.68"
)


extract_one() {
  local region="$1"
  local iso="$ISO_DIR/${ISO[$region]}"
  local boot="${BOOT[$region]}"
  local out="$ROOT/extracted/$region"
  [ -f "$iso" ] || { echo "ERROR: ISO missing: $iso"; return 1; }
  mkdir -p "$out"

  echo "==> [$region] reading $(basename "$iso")"
  # bsdtar reads the ISO9660 image directly. The boot ELF and SYSTEM.CNF must
  # exist. IOPRP255.IMG (IOP module image, handy for later IOP/asset work) is
  # optional, so it is extracted on its own.
  rm -f "$out/$boot" "$out/SYSTEM.CNF" "$out/IOPRP255.IMG"
  "$BSDTAR" -xf "$iso" -C "$out" "$boot" SYSTEM.CNF
  "$BSDTAR" -xf "$iso" -C "$out" IOPRP255.IMG 2>/dev/null || true
  chmod u+w "$out"/* 2>/dev/null || true

  echo "==> [$region] generating flat .rom via objcopy"
  "$OBJCOPY" -O binary "$out/$boot" "$out/$boot.rom"

  echo "==> [$region] sha1:"
  if command -v shasum >/dev/null 2>&1; then shasum "$out/$boot" "$out/$boot.rom"; else sha1sum "$out/$boot" "$out/$boot.rom"; fi
}

for r in "${REGIONS[@]}"; do extract_one "$r"; done
echo "Done. Extracted boot ELFs + .rom images under extracted/{usa,eu}/"

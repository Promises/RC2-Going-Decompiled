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
# Requires: macOS hdiutil (ISO9660 mount) + llvm-objcopy (brew install llvm).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ISO_DIR="$ROOT/source-isos"
OBJCOPY="${OBJCOPY:-/opt/homebrew/opt/llvm/bin/llvm-objcopy}"

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

command -v "$OBJCOPY" >/dev/null 2>&1 || { echo "ERROR: objcopy not found at $OBJCOPY (brew install llvm)"; exit 1; }

extract_one() {
  local region="$1"
  local iso="$ISO_DIR/${ISO[$region]}"
  local boot="${BOOT[$region]}"
  local out="$ROOT/extracted/$region"
  [ -f "$iso" ] || { echo "ERROR: ISO missing: $iso"; return 1; }
  mkdir -p "$out"

  echo "==> [$region] mounting $(basename "$iso")"
  local mnt vol
  mnt="$(hdiutil attach -nobrowse -readonly "$iso")"
  vol="$(echo "$mnt" | grep -oE '/Volumes/.*' | head -1)"
  trap '[ -n "${vol:-}" ] && hdiutil detach "$vol" >/dev/null 2>&1 || true' RETURN

  cp -f "$vol/$boot" "$out/$boot"
  cp -f "$vol/SYSTEM.CNF" "$out/SYSTEM.CNF"
  # IOP module image + level header, handy for later IOP/asset work.
  [ -f "$vol/IOPRP255.IMG" ] && cp -f "$vol/IOPRP255.IMG" "$out/IOPRP255.IMG" || true

  hdiutil detach "$vol" >/dev/null 2>&1; vol=""

  echo "==> [$region] generating flat .rom via objcopy"
  "$OBJCOPY" -O binary "$out/$boot" "$out/$boot.rom"

  echo "==> [$region] sha1:"
  shasum "$out/$boot" "$out/$boot.rom"
}

for r in "${REGIONS[@]}"; do extract_one "$r"; done
echo "Done. Extracted boot ELFs + .rom images under extracted/{usa,eu}/"

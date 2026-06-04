#!/usr/bin/env bash
# Fetch the matching EE compiler: SN Systems ProDG ee-gcc 2.95.2 (SN BUILD v2.73a).
# These are i386 Win32 PE binaries (run via wibo). Proprietary -> not committed.
#
# Evidence this is the right compiler: the boot ELF carries .DVP.* (SN dvp-as)
# VU sections and host0: DECI2 paths; SCE EE SDK + SN ProDG used gcc 2.95.2.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="$ROOT/tools/ee"
URL="https://github.com/TheOnlyZac/compilers/releases/download/ee-gcc2.95.2-SN-v2.73a/ee-gcc2.95.2-SN-v2.73a.tar.gz"

mkdir -p "$DEST"
if [ -f "$DEST/cc/lib/gcc-lib/ee/2.95.2/cc1.exe" ]; then
  echo "ee-gcc already present at $DEST/cc"; exit 0
fi
echo "Fetching $URL"
curl -fsSL "$URL" | tar -xz -C "$DEST"
echo "Installed SN ProDG ee-gcc 2.95.2 under $DEST/cc"
ls "$DEST/cc/lib/gcc-lib/ee/2.95.2/" | head

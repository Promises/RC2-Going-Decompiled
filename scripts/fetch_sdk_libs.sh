#!/usr/bin/env bash
# Fetch the SCE SDK static libraries / crt0 that match the pinned ee-gcc
# (2.9-ee-991111). The game statically linked these, so their compiled bytes
# appear verbatim in the binary -> they both auto-match and serve as
# known-source validation (see tools/ee/find_libc_funcs.py).
#
# Proprietary (© Sony) -> gitignored. Source: the SN ProDG 2.0 package.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="$ROOT/tools/ee/sdk-lib"
BASE="https://raw.githubusercontent.com/AngheloAlf/SN-Systems-ProDG_for_PS2_2.0/main/usr/local/sce/ee/gcc/ee/lib"
mkdir -p "$DEST"
for f in crt0.o pcrt0.o libc.a libm.a; do
  if [ -f "$DEST/$f" ]; then echo "have $f"; continue; fi
  echo "fetching $f"
  curl -fsSL "$BASE/$f" -o "$DEST/$f" && echo "  -> $DEST/$f ($(stat -f%z "$DEST/$f" 2>/dev/null || stat -c%s "$DEST/$f") bytes)"
done
echo "Done. Extract objects for matching with:"
echo "  docker --context colima-ee-x86 run --rm -v \"\$PWD\":/work ee-build \\"
echo "    sh -c 'cd /work/tools/ee/sdk-lib && mkdir -p obj && cd obj && ar x ../libc.a'"

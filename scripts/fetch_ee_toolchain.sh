#!/usr/bin/env bash
# Fetch the matching EE compiler. i386 Win32 PE binaries (run via wibo).
# Proprietary -> not committed.
#
# IMPORTANT: the byte-matching compiler is **ee-gcc 2.9-ee-991111** (the SCE EE
# 991111 snapshot), NOT 2.95.2. They differ in codegen: 2.95.2 saves registers
# with `sq`/`lq` (128-bit) and the original/2.9 use `sd`/`ld` (64-bit) — so 2.95.2
# only matches frameless leaf functions. The base tarball (TheOnlyZac) ships the
# driver+libs but only the 2.95.2 `cc1`; we overlay the correct 2.9-ee-991111
# `cc1`/`cpp` from the full SN ProDG package.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="$ROOT/tools/ee"
URL="https://github.com/TheOnlyZac/compilers/releases/download/ee-gcc2.95.2-SN-v2.73a/ee-gcc2.95.2-SN-v2.73a.tar.gz"
PRODG="https://raw.githubusercontent.com/AngheloAlf/SN-Systems-ProDG_for_PS2_2.0/main/usr/local/sce/ee/gcc/lib/gcc-lib/ee/2.9-ee-991111"

mkdir -p "$DEST"
if [ ! -f "$DEST/cc/lib/gcc-lib/ee/2.95.2/cc1.exe" ]; then
  echo "Fetching base toolchain: $URL"
  curl -fsSL "$URL" | tar -xz -C "$DEST"
fi

# Overlay the correct cc1/cpp (2.9-ee-991111) — this is what the build scripts use.
CC1DIR="$DEST/cc/lib/gcc-lib/ee/2.9-ee-991111"
mkdir -p "$CC1DIR"
for f in cc1.exe cpp.exe; do
  [ -f "$CC1DIR/$f" ] || curl -fsSL "$PRODG/$f" -o "$CC1DIR/$f"
done
echo "Installed: 2.9-ee-991111 cc1/cpp (matching) + 2.95.2 base under $DEST/cc"
ls "$CC1DIR"

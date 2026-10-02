#!/usr/bin/env bash
# Fetch the matching EE compiler. i386 Win32 PE binaries (run via wibo).
# Proprietary -> not committed.
#
# IMPORTANT: the byte-matching compiler is **ee-gcc 2.9-ee-991111** (the SCE EE
# 991111 snapshot), NOT 2.95.2. They differ in codegen: 2.95.2 saves registers
# with `sq`/`lq` (128-bit) and the original/2.9 use `sd`/`ld` (64-bit) — so 2.95.2
# only matches frameless leaf functions. The base tarball (TheOnlyZac) ships the
# driver+libs but only the 2.95.2 `cc1`; we overlay the correct 2.9-ee-991111
# `cc1`/`cpp` from the full SN ProDG package, and the same revision's `cc1plus`
# (2.9-ee-991111b/r4, the C++ front end for src/<region>/**/*.cpp units, FACT
# #8809) from decompme's ee-gcc2.9-991111 archive. The s136os arm's compilers
# (SN 2.95.3 BUILD 1.36: cc1 for .c units, cc1plus for .cpp units; FACT #8810,
# tools/ee/s136os_splice.sh) come from AngheloAlf's ProDG 3.01 tree, pinned to
# one commit.
#
# EVERY DOWNLOAD IS SHA256-PINNED and a mismatch FAILS the fetch (exit 1, the
# file is not installed). Hashes measured 2026-10-01 (task #1258) with
# `shasum -a 256`; the installed 2.9 cc1.exe/cpp.exe were already these blobs.
# A blob that is already installed is re-verified on every run, so a wrong file
# left by an earlier unpinned fetch is reported rather than trusted.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="$ROOT/tools/ee"
URL="https://github.com/TheOnlyZac/compilers/releases/download/ee-gcc2.95.2-SN-v2.73a/ee-gcc2.95.2-SN-v2.73a.tar.gz"
URL_SHA256=3e48e8d9b349b95aed476596d351bda439abebf7c4c5ad3318bedd81163253d7
PRODG="https://raw.githubusercontent.com/AngheloAlf/SN-Systems-ProDG_for_PS2_2.0/main/usr/local/sce/ee/gcc/lib/gcc-lib/ee/2.9-ee-991111"
CC1_SHA256=124f7c6870b7facbaecbc51f0b87c0aad46db5f6ed491c21638e1da991e45be5
CPP_SHA256=8f282abc89a6d2281e87617a7de7ebb0edbdc8dbb02d3e0a9ecc5d4825146a19
CXX_URL="https://github.com/decompme/compilers/releases/download/compilers/ee-gcc2.9-991111.tar.gz"
CXX_URL_SHA256=9ebbb042eb1e5d33d772293c5e9286a54dbec29865a3d7fbb398c97436f33583
CC1PLUS_SHA256=65b5134f24377544bb2e5d1d2dd3436807cc50ce2ce77d759e4fca1add18f9a4
PRODG301="https://raw.githubusercontent.com/AngheloAlf/SN-Systems-ProDG_for_PS2_3.01/d74f6fe08d24e7cf0df48cb570d85ad04db167c5/usr/local/sce/ee/gcc/lib/gcc-lib/ee/2.95.3"
S136_CC1_SHA256=0393bcd31f91a6b9f0255db97f1cc99eba78ee8fc003e9a04dfabed1ae1d522e
S136_CC1PLUS_SHA256=78a0df900a396098986cbc97a8d3eab6dbbc587a343d4dbd22929a4112c003fb
SDK24="https://raw.githubusercontent.com/AngheloAlf/sce_ps2_sdk_24/5c8bdf31f6bdd82ba4456413594198b5bd469f43/local/sce/ee/gcc/lib/gcc-lib/ee/2.96-ee-001003-1"
E96_CC1_SHA256=59ec92b3f9f3513e0633331af304733e3094de30884e662a8cb584a51c1c42b5
E96_CC1PLUS_SHA256=560e6f276134d109e2b4c05ee9c72d4f9215ffc86d784eb4491a3dadf316bf9a
GLIBC236="https://raw.githubusercontent.com/parappadev/parappa2/a5ac297135ad3a1969da7faa520d9f7bb6944d82/tools/toolchain/ee-gcc29/lib"
LD236_SHA256=d5a16aee4a04db7fa27a5c071a0bef62002ee679bf5ecce51e383003cf840fd8
LIBC236_SHA256=7b091aee7173e3a7cbddc6630fa6b0d0106944b5618b0b27c2c058ea9c3d92eb

# verify FILE EXPECTED LABEL — exit 1 unless FILE's sha256 is EXPECTED.
verify() {
  local got
  got="$(shasum -a 256 "$1" | awk '{print $1}')"
  if [ "$got" != "$2" ]; then
    echo "fetch_ee_toolchain: FATAL sha256 mismatch for $3" >&2
    echo "  file:     $1" >&2
    echo "  expected: $2" >&2
    echo "  got:      $got" >&2
    exit 1
  fi
  echo "  sha256 ok: $3 ${got:0:16}"
}

mkdir -p "$DEST"
TMP="$(mktemp -d "$DEST/.fetch.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

if [ ! -f "$DEST/cc/lib/gcc-lib/ee/2.95.2/cc1.exe" ]; then
  echo "Fetching base toolchain: $URL"
  curl -fsSL "$URL" -o "$TMP/base.tar.gz"
  verify "$TMP/base.tar.gz" "$URL_SHA256" "base tarball (TheOnlyZac ee-gcc2.95.2-SN-v2.73a)"
  tar -xz -C "$DEST" -f "$TMP/base.tar.gz"
fi

# Overlay the correct cc1/cpp (2.9-ee-991111) — this is what the build scripts use.
CC1DIR="$DEST/cc/lib/gcc-lib/ee/2.9-ee-991111"
mkdir -p "$CC1DIR"
for f in cc1.exe cpp.exe; do
  case "$f" in cc1.exe) want="$CC1_SHA256";; cpp.exe) want="$CPP_SHA256";; esac
  if [ ! -f "$CC1DIR/$f" ]; then
    curl -fsSL "$PRODG/$f" -o "$TMP/$f"
    verify "$TMP/$f" "$want" "ProDG 2.9-ee-991111 $f"
    mv "$TMP/$f" "$CC1DIR/$f"
  fi
  verify "$CC1DIR/$f" "$want" "installed $f"
done

# cc1plus (2.9-ee-991111b/r4). The archive's own cc1.exe must be the cc1 above:
# that identity is what makes its cc1plus the matching sibling (FACT #8809).
if [ ! -f "$CC1DIR/cc1plus.exe" ]; then
  echo "Fetching C++ front end: $CXX_URL"
  curl -fsSL "$CXX_URL" -o "$TMP/cxx.tar.gz"
  verify "$TMP/cxx.tar.gz" "$CXX_URL_SHA256" "decompme ee-gcc2.9-991111 archive"
  mkdir -p "$TMP/cxx"
  tar -xz -C "$TMP/cxx" -f "$TMP/cxx.tar.gz"
  verify "$TMP/cxx/lib/gcc-lib/ee/2.9-ee-991111/cc1.exe" "$CC1_SHA256" "archive cc1.exe == our cc1 (identity check)"
  verify "$TMP/cxx/lib/gcc-lib/ee/2.9-ee-991111/cc1plus.exe" "$CC1PLUS_SHA256" "archive cc1plus.exe"
  mv "$TMP/cxx/lib/gcc-lib/ee/2.9-ee-991111/cc1plus.exe" "$CC1DIR/cc1plus.exe"
fi
verify "$CC1DIR/cc1plus.exe" "$CC1PLUS_SHA256" "installed cc1plus.exe"

# SN 2.95.3 BUILD 1.36 cc1 + cc1plus (the s136os arm, task #1284). The tree's
# cc1.exe must be the s136 cc1 the arm was proven with (FACT #8810, 0393bcd3):
# that identity, at the same pinned commit, is what makes its cc1plus the
# sibling front end. The arm preprocesses with the 2.9 cpp above, so the 2.95.3
# cpp.exe is not needed.
S136DIR="$DEST/cc/lib/gcc-lib/ee/2.95.3"
mkdir -p "$S136DIR"
if [ ! -f "$S136DIR/cc1.exe" ]; then
  curl -fsSL "$PRODG301/cc1.exe" -o "$TMP/s136cc1.exe"
  verify "$TMP/s136cc1.exe" "$S136_CC1_SHA256" "ProDG 3.01 2.95.3 cc1.exe"
  mv "$TMP/s136cc1.exe" "$S136DIR/cc1.exe"
fi
verify "$S136DIR/cc1.exe" "$S136_CC1_SHA256" "installed 2.95.3 cc1.exe"
if [ ! -f "$S136DIR/cc1plus.exe" ]; then
  echo "Fetching SN 2.95.3 v1.36 C++ front end: $PRODG301/cc1plus.exe"
  curl -fsSL "$PRODG301/cc1.exe" -o "$TMP/s136cc1.idcheck.exe"
  verify "$TMP/s136cc1.idcheck.exe" "$S136_CC1_SHA256" "ProDG 3.01 cc1.exe == our s136 cc1 (identity check)"
  curl -fsSL "$PRODG301/cc1plus.exe" -o "$TMP/s136cc1plus.exe"
  verify "$TMP/s136cc1plus.exe" "$S136_CC1PLUS_SHA256" "ProDG 3.01 2.95.3 cc1plus.exe"
  mv "$TMP/s136cc1plus.exe" "$S136DIR/cc1plus.exe"
fi
verify "$S136DIR/cc1plus.exe" "$S136_CC1PLUS_SHA256" "installed 2.95.3 cc1plus.exe"

# ee-gcc 2.96-ee-001003-1 (the engine96 arm: tools/ee/ee_cc1.sh engine96,
# objdiff_build.sh, diff96.sh) into tools/ee/cc-296/. These are native i386 ELF
# binaries, run through the bundled glibc-2.3.6 loader so the container's own
# libc is untouched. The old scripts/fetch_2.96.sh that used to do this exists
# only at non-ancestor commit 8d9ce18f (FACT #6235); this block replaces it.
# The cc1 is the one every engine96 result was measured with (NOTE #6229), and
# the SDK 2.4 tree's 2.96 cc1 must be that same blob: that identity is what
# makes the tree's cc1plus its sibling C++ front end (task #1302).
# ⚠️ Take the ELF `cc1`/`cc1plus`, NEVER the `.exe` files in the same
# directory: those self-identify as `2.95.3 SN BUILD v1.07` (NOTE #6287 §3).
# ⚠️ The 2.96 cc1plus is NOT codegen-identical to this cc1 over the MATCH_
# members (task #1302's FACT); provisioning it proves nothing about using it.
E96DIR="$DEST/cc-296/lib/gcc-lib/ee/2.96-ee-001003-1"
mkdir -p "$E96DIR"
if [ ! -f "$E96DIR/cc1" ]; then
  curl -fsSL "$SDK24/cc1" -o "$TMP/e96cc1"
  verify "$TMP/e96cc1" "$E96_CC1_SHA256" "SDK 2.4 2.96-ee-001003-1 cc1"
  mv "$TMP/e96cc1" "$E96DIR/cc1"
fi
verify "$E96DIR/cc1" "$E96_CC1_SHA256" "installed 2.96 cc1"
if [ ! -f "$E96DIR/cc1plus" ]; then
  echo "Fetching ee-gcc 2.96-ee-001003-1 C++ front end: $SDK24/cc1plus"
  curl -fsSL "$SDK24/cc1" -o "$TMP/e96cc1.idcheck"
  verify "$TMP/e96cc1.idcheck" "$E96_CC1_SHA256" "SDK 2.4 2.96 cc1 == our engine96 cc1 (identity check)"
  curl -fsSL "$SDK24/cc1plus" -o "$TMP/e96cc1plus"
  verify "$TMP/e96cc1plus" "$E96_CC1PLUS_SHA256" "SDK 2.4 2.96-ee-001003-1 cc1plus"
  mv "$TMP/e96cc1plus" "$E96DIR/cc1plus"
fi
verify "$E96DIR/cc1plus" "$E96_CC1PLUS_SHA256" "installed 2.96 cc1plus"
for f in ld-2.3.6.so libc.so.6; do
  case "$f" in ld-2.3.6.so) want="$LD236_SHA256";; libc.so.6) want="$LIBC236_SHA256";; esac
  if [ ! -f "$DEST/cc-296/$f" ]; then
    curl -fsSL "$GLIBC236/$f" -o "$TMP/$f"
    verify "$TMP/$f" "$want" "glibc-2.3.6 i386 $f"
    mv "$TMP/$f" "$DEST/cc-296/$f"
  fi
  verify "$DEST/cc-296/$f" "$want" "installed $f"
done
chmod +x "$E96DIR/cc1" "$E96DIR/cc1plus" "$DEST/cc-296/ld-2.3.6.so"

echo "Installed: 2.9-ee-991111 cc1/cpp/cc1plus (matching) + SN 2.95.3 v1.36 cc1/cc1plus (s136os arm) + 2.95.2 base under $DEST/cc; 2.96-ee-001003-1 cc1/cc1plus + glibc-2.3.6 loader under $DEST/cc-296"
ls "$CC1DIR" "$S136DIR" "$E96DIR"

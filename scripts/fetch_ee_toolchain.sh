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

echo "Installed: 2.9-ee-991111 cc1/cpp/cc1plus (matching) + SN 2.95.3 v1.36 cc1/cc1plus (s136os arm) + 2.95.2 base under $DEST/cc"
ls "$CC1DIR" "$S136DIR"

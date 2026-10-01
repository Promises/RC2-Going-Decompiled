#!/usr/bin/env bash
# Fetch the SCE PS2 SDK runtime-library headers the C sources compile against
# into going-decompiled/include/rtl/. Proprietary (© Sony) -> never committed.
#
# Source: the parappa2 matching decomp (github.com/parappadev/parappa2), whose
# include/rtl/ is the "Runtime Library Release 2.4" header set, pinned to one
# commit. Every file is verified against scripts/sdk_headers.manifest by git
# blob sha1, so a moved or edited upstream file fails loudly instead of
# silently changing what the build compiles.
#
#   scripts/fetch_sdk_headers.sh            # fetch + verify
#   scripts/fetch_sdk_headers.sh --verify   # verify an existing tree only
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="$ROOT/going-decompiled/include/rtl"
MANIFEST="$ROOT/scripts/sdk_headers.manifest"
REPO="parappadev/parappa2"
COMMIT="45694de3b6ed8d8bb0514e27bcaf091198698faf"

verify() {
  local bad=0 n=0 want path got
  while read -r want path; do
    case "$want" in ''|\#*) continue ;; esac
    n=$((n + 1))
    if [ ! -f "$DEST/$path" ]; then
      echo "MISSING  $path"; bad=$((bad + 1)); continue
    fi
    got="$(git hash-object --no-filters "$DEST/$path")"
    if [ "$got" != "$want" ]; then
      echo "MISMATCH $path (have $got, want $want)"; bad=$((bad + 1))
    fi
  done < "$MANIFEST"
  if [ "$bad" -ne 0 ]; then
    echo "fetch_sdk_headers: FAIL - $bad of $n headers missing or different" >&2
    return 1
  fi
  echo "fetch_sdk_headers: OK - $n headers verified in $DEST"
}

if [ "${1:-}" = "--verify" ]; then
  verify
  exit $?
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
echo "Fetching $REPO@${COMMIT:0:12} include/rtl"
curl -fsSL "https://codeload.github.com/$REPO/tar.gz/$COMMIT" | tar -xz -C "$TMP"
SRC="$(find "$TMP" -maxdepth 3 -type d -path '*/include/rtl' | head -n 1)"
if [ -z "$SRC" ]; then
  echo "fetch_sdk_headers: FAIL - include/rtl not found in the $REPO archive" >&2
  exit 1
fi
mkdir -p "$DEST"
# Copy only the manifest's files, so nothing unlisted reaches the build.
while read -r want path; do
  case "$want" in ''|\#*) continue ;; esac
  if [ -f "$SRC/$path" ]; then
    mkdir -p "$DEST/$(dirname "$path")"
    cp "$SRC/$path" "$DEST/$path"
  fi
done < "$MANIFEST"
verify

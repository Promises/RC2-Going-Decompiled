#!/bin/sh
# asmfix_stamp.sh — fingerprint of everything asm_unit.sh's VU0-fixed mirror
# for one region is derived from, so a mirror can be NAMED by its inputs.
#
#   tools/ee/asmfix_stamp.sh <region>      -> 16 hex chars on stdout
#
# Covers, by CONTENT (path + sha256 of each file, paths sorted):
#   going-decompiled/asm/<region>/nonmatchings/**/*.s   the tree the mirror copies
#   tools/ee/vu0_fixup.sed                              the filter it copies through
#   going-decompiled/build/<region>/include/macro.inc   copied into the mirror
#   tools/ee/asm_unit.sh                                the builder itself
# A change to any of these — an edited or re-split .s, an added or removed
# function file, a fixup change — changes the stamp, and objdiff_build.sh then
# selects a NEW mirror path instead of reusing the old one (#398). Nothing else
# about the build (the unit C, cc1 flags, the region's obj/expected outputs) is
# part of the mirror, so nothing else is part of the stamp.
#
# Runs on the HOST (~0.15 s for the 2,700-file USA tree). Any missing input is a
# hard error, never an empty stamp: an empty stamp would name a mirror that
# nothing invalidates.
set -eu
REGION="$1"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"
ASMSRC="going-decompiled/asm/$REGION/nonmatchings"
MACINC="going-decompiled/build/$REGION/include/macro.inc"
for f in "$ASMSRC" tools/ee/vu0_fixup.sed "$MACINC" tools/ee/asm_unit.sh; do
  [ -e "$f" ] || { echo "asmfix_stamp: missing input $f" >&2; exit 2; }
done
if command -v sha256sum >/dev/null 2>&1; then SHA="sha256sum"; else SHA="shasum -a 256"; fi
# (/usr/bin/find, not find: a shimmed `find` on this fleet fails into a silent
# empty list — reference_bare_find_is_dead — and an empty list must not stamp.)
LIST="$( { /usr/bin/find "$ASMSRC" -type f -name '*.s'; echo tools/ee/vu0_fixup.sed; echo "$MACINC"; echo tools/ee/asm_unit.sh; } | LC_ALL=C sort )"
n="$(printf '%s\n' "$LIST" | /usr/bin/grep -c '\.s$' || true)"
[ "$n" -gt 0 ] || { echo "asmfix_stamp: no .s files under $ASMSRC" >&2; exit 2; }
printf '%s\n' "$LIST" | xargs $SHA | $SHA | cut -c1-16

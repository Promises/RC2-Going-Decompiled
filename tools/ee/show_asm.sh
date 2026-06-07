#!/usr/bin/env bash
# show_asm.sh — dump one or more per-function nonmatching .s files with the
# boilerplate (alignment, .nonmatching markers, end labels, blank lines)
# stripped, so you see just the instructions. Collapses the recurring
# `cd <unit>; for f in ...; do grep -vE ... $f.s; done` one-liner into a single
# allow-listable command — only the args (unit, funcs, region, pattern) vary.
#
#   tools/ee/show_asm.sh cod/015180 func_0011D810 func_0012E980 func_00131A98
#   tools/ee/show_asm.sh -r eu cod/099900 func_00099A10
#   tools/ee/show_asm.sh -p 'align|^\s*$' cod/015180 func_0012F9C8
#
# Flags (must precede the unit arg):
#   -r <region>   asm region under going-decompiled/asm/ (default: usa)
#   -p <regex>    extended-regex of lines to strip (default: boilerplate)
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
region=usa
pat='align|nonmatching|endlabel|^\s*$'
while getopts 'r:p:' opt; do
  case "$opt" in
    r) region=$OPTARG ;;
    p) pat=$OPTARG ;;
    *) echo "usage: show_asm.sh [-r region] [-p regex] <unit> <func>..." >&2; exit 2 ;;
  esac
done
shift $((OPTIND - 1))

unit=$1; shift
if [ -z "$unit" ] || [ $# -eq 0 ]; then
  echo "usage: show_asm.sh [-r region] [-p regex] <unit> <func>..." >&2
  exit 2
fi

dir="$ROOT/going-decompiled/asm/$region/nonmatchings/$unit"
for f in "$@"; do
  printf '=== %s ===\n' "$f"
  grep -vE "$pat" "$dir/$f.s"
done

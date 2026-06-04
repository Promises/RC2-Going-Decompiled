#!/bin/sh
# Compile a C file with the matching SN ProDG ee-gcc and disassemble the result.
# Run INSIDE the ee-build container in the colima x86 VM:
#
#   docker context use colima-ee-x86
#   docker run --rm -v "$PWD":/work ee-build sh tools/ee/match.sh [CFILE] [FLAGS...]
#
# Default: tools/ee/probe/test.c at -O2 -G0 (the confirmed Going Commando flags).
set -e
cd "$(dirname "$0")/../.."        # repo root inside container (/work)
W=/usr/local/bin/wibo
EE=tools/ee/cc/lib/gcc-lib/ee/2.95.2
BIN=tools/ee/cc/bin
AS=tools/ee/cc/ee/bin/as.exe
CFILE="${1:-tools/ee/probe/test.c}"; shift 2>/dev/null || true
FLAGS="${*:--O2 -G0}"
S="${CFILE%.c}.s"; O="${CFILE%.c}.o"

$W "$EE/cc1.exe" -quiet $FLAGS "$CFILE" -o "$S"
$W "$AS" -EL -G0 -o "$O" "$S"
echo "# ==== $CFILE  ($FLAGS) ===="
cat "$S"
echo "# ==== object disassembly ===="
$W "$BIN/ee-objdump.exe" -dr "$O"

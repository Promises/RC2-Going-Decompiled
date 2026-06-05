#!/bin/sh
# Compile a C file with the matching SN ProDG ee-gcc (2.9-ee-991111) and
# disassemble the result, to compare against the original Going Commando asm.
# Run INSIDE the ee-build container in the colima x86 VM:
#
#   docker --context colima-ee-x86 run --rm -v "$PWD":/work ee-build \
#       sh tools/ee/match.sh [CFILE] [EXTRA_CC1_FLAGS...]
#
# We invoke cpp + cc1 directly (the gcc *driver* segfaults under wibo). cpp is
# given the macros the driver normally defines (__GNUC__ gates the SDK's 128-bit
# u_long128 typedef in eetypes.h). Default flags: -O2 -G0 (confirmed match).
set -e
cd "$(dirname "$0")/../.."          # repo root inside container (/work)
W=/usr/local/bin/wibo
G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
AS=tools/ee/cc/ee/bin/as.exe
INC=going-decompiled/include
CFILE="${1:-tools/ee/probe/test.c}"; shift 2>/dev/null || true
CC1FLAGS="${*:--O2 -G0}"
I="${CFILE%.c}.i"; S="${CFILE%.c}.s"; O="${CFILE%.c}.o"

# Macros the gcc driver injects for this target (needed by the SCE SDK headers).
CPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__"
CPPINC="-I$INC -I$INC/rtl/ee -I$INC/rtl/common -I$INC/rtl/iop"

$W "$G/cpp.exe" $CPPDEF $CPPINC "$CFILE" "$I"
$W "$G/cc1.exe" -quiet $CC1FLAGS "$I" -o "$S"
echo "# ==== $CFILE  ($CC1FLAGS) ===="
cat "$S"
# Assemble too (best-effort; needs labels.inc for INCLUDE_ASM TUs).
if $W "$AS" -EL -G0 -o "$O" "$S" 2>/dev/null; then
  echo "# ==== object .text ===="
  ${OBJDUMP:-tools/ee/cc/bin/ee-objdump.exe} >/dev/null 2>&1 || true
  $W tools/ee/cc/bin/ee-objdump.exe -dr "$O" 2>/dev/null || true
fi

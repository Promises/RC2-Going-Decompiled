#!/bin/sh
# Emit EE asm for a TU compiled in the C-only (TARGET_NATIVE) flavour, so the
# #else portable bodies can be inspected as MIPS (argument registers visible).
#
# Unlike match.sh, -DTARGET_NATIVE is passed to *cpp*, where it has to be for
# the #ifndef TARGET_NATIVE / #else arms to select the portable body. The
# include set mirrors build_conly.sh's NINCC/NCPPDEF exactly -- shim FIRST, so
# its stdint.h/math.h override the ones the EE SDK does not ship.
#
#   tools/ee/vm.sh 'sh tools/ee/codegen_native.sh <file.c> <out.s>'
set -e
cd "$(dirname "$0")/../.."
W=/usr/local/bin/wibo
G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
CFILE="$1"; OUT="$2"
NCPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DTARGET_NATIVE"
NINCC="-Itools/ee/eetest/shim -Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common -Igoing-decompiled/include/rtl/iop -Itools/ee/eetest"
$W "$G/cpp.exe" $NCPPDEF $NINCC "$CFILE" "$OUT.i"
$W "$G/cc1.exe" -quiet -O2 -G0 "$OUT.i" -o "$OUT"

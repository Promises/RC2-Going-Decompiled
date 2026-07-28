#!/bin/sh
# Compile every src/<region> unit's TARGET_NATIVE (#else) arm exactly as
# tools/ee/conly_finish.sh does, then list each unit's UNDEFINED symbols.
# Read-only w.r.t. the tree (outputs go to /tmp/caltscan).
# Run INSIDE the ee-build container.
REGION="${1:-usa}"
SRC="going-decompiled/src/$REGION"
OUT=/tmp/caltscan/$REGION
mkdir -p "$OUT"
WIBO=/usr/local/bin/wibo
G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
NINCC="-Itools/ee/eetest/shim -Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common -Igoing-decompiled/include/rtl/iop -Itools/ee/eetest"
NCPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DTARGET_NATIVE"
for c in $(find "$SRC" -name '*.c' | sort); do
  grep -q '^#else' "$c" || continue
  u=$(echo "$c" | sed "s|$SRC/||; s|/|_|g; s|\.c$||")
  i="$OUT/$u.i"; s="$OUT/$u.s"; o="$OUT/$u.o"
  if ! "$WIBO" "$G/cpp.exe" $NCPPDEF $NINCC "$c" "$i" 2>"$OUT/$u.cpp.log"; then
    echo "CPPFAIL $c"; continue; fi
  if ! "$WIBO" "$G/cc1.exe" -quiet -O2 -G0 "$i" -o "$s" 2>"$OUT/$u.cc1.log"; then
    echo "CC1FAIL $c"; continue; fi
  if ! sed -E -f tools/ee/move_fixup.sed "$s" \
      | mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -o "$o" - 2>"$OUT/$u.as.log"; then
    echo "ASFAIL $c"; continue; fi
  echo "OK $c"
  mips-linux-gnu-nm -u "$o" | awk -v u="$c" '{print u"\t"$2}' >> "$OUT/undef.tsv"
done

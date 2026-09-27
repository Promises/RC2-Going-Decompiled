#!/bin/sh
# build_libgcc.sh <region> — build the libgcc.a members the region's linker
# script links, from the VERBATIM GCC source in going-decompiled/libgcc/
# (RULING #8206: libgcc lives as GCC's own source, linked as a library, never
# transcribed into a game .c). Runs INSIDE the ee-build container; build.sh
# calls it before the link.
#
# Which members: every `libgcc.a:<member>.o` the splat-generated .ld names (a
# `lib` subsegment in the region's yaml). None named => nothing is built and
# no archive exists, so the link is unchanged (today: EU).
# How: going-decompiled/libgcc/MEMBERS gives each member's source and GCC's own
# module selection (-DFINE_GRAINED_LIBRARIES -DL_<module>). Compiler is the
# game's SDK compiler, 2.9-ee-991111 cpp.exe + cc1.exe -O2 -G0 (-G0: the SDK
# archive has 0 GPREL16 relocs in all 58 members, NOTE #8217). The ee-gcc
# driver segfaults under wibo, so its predefines are passed by hand; what must
# hold is that longlong.h sees __mips__/__R5900__ and picks the MIPS umul_ppmm.
# The cc1 OUTPUT goes through the same move_fixup.sed every unit's does; the
# GCC source is never edited.
# Output: going-decompiled/build/<region>/lib/libgcc.a (the yaml's lib_path) and
# lib/members.txt (one member per line) for build.sh's INPUT/EXTERN lines.
set -e
REGION="${1:-usa}"
cd "$(dirname "$0")/../.."
case "$REGION" in
  usa) BASENAME=SCUS_972.68 ;;
  eu)  BASENAME=SCES_516.07 ;;
  *) echo "unknown region $REGION"; exit 2 ;;
esac
LD=going-decompiled/linker_scripts/$BASENAME.ld
SRC=going-decompiled/libgcc
OUT=going-decompiled/build/$REGION/lib
WIBO=/usr/local/bin/wibo
G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
PRE="-undef -D__GNUC__=2 -D__GNUC_MINOR__=9 -Dmips -DMIPSEL -DR5900 -D_mips -D_MIPSEL -D_R5900 -D__mips__ -D__MIPSEL__ -D__R5900__ -D___mips__ -D___MIPSEL__ -D___R5900__ -D__MIPSEL -D__R5900 -D___mips -D___MIPSEL -D___R5900 -D__mips=3 -D__mips64 -D__mips_eabi -D__mips_single_float -D__LONG_MAX__=9223372036854775807L -D__LANGUAGE_C -D_LANGUAGE_C -DLANGUAGE_C -D__OPTIMIZE__ -Asystem(unix) -Acpu(mips) -Amachine(mips)"

# Never link a stale archive: start from nothing every build.
rm -rf "$OUT"
MEMBERS=$(grep -oE 'libgcc\.a:[A-Za-z0-9_-]+\.o' "$LD" | sed -E 's/^libgcc\.a:(.*)\.o$/\1/' | sort -u)
if [ -z "$MEMBERS" ]; then
  echo "   libgcc: $LD names no libgcc.a member; nothing built"
  exit 0
fi
mkdir -p "$OUT"
objs=""
for m in $MEMBERS; do
  row=$(awk -v m="$m" '$1 == m' "$SRC/MEMBERS")
  [ -n "$row" ] || { echo "BUILD FAIL (libgcc): $LD names libgcc.a:$m.o but $SRC/MEMBERS has no row for it" >&2; exit 1; }
  src=$(echo "$row" | awk '{print $2}')
  defs=$(echo "$row" | awk '{ $1 = ""; $2 = ""; sub(/^ +/, ""); print }')
  i="$OUT/$m.i"; s="$OUT/$m.s"; o="$OUT/$m.o"
  "$WIBO" "$G/cpp.exe" $PRE -I"$SRC/shim" $defs "$SRC/$src" "$i" \
    || { echo "BUILD FAIL (libgcc cpp): $m" >&2; exit 1; }
  "$WIBO" "$G/cc1.exe" -quiet -O2 -G0 "$i" -o "$s" \
    || { echo "BUILD FAIL (libgcc cc1): $m" >&2; exit 1; }
  sed -E -f tools/ee/move_fixup.sed "$s" | tr -d '\r' \
    | mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -o "$o" - \
    || { echo "BUILD FAIL (libgcc as): $m" >&2; exit 1; }
  [ -s "$o" ] || { echo "BUILD FAIL (libgcc: no object): $m" >&2; exit 1; }
  objs="$objs $o"
  echo "$m" >> "$OUT/members.txt"
done
mips-linux-gnu-ar rcs "$OUT/libgcc.a" $objs
echo "   libgcc: built $(wc -l < "$OUT/members.txt" | tr -d ' ') member(s) into $OUT/libgcc.a:" $MEMBERS

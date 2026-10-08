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
# archive has 0 GPREL16 relocs in all 57 object members, NOTE #8217 as
# narrowed by FACT #8239: `ar t` prints 58 lines, one of them `/`, the archive
# symbol index). The ee-gcc driver segfaults under wibo, so its predefines
# are passed by hand; what must hold is that longlong.h sees
# __mips__/__R5900__ and picks the MIPS umul_ppmm.
# The cc1 OUTPUT goes through the same move_fixup.sed every unit's does, then
# tools/ee/libgcc_lid_dli.pl (li.d -> dli, which binutils refuses at r5900; a
# libgcc-only rule, see its header); the GCC source is never edited.
# Two more MEMBERS row kinds, told apart by column 2 alone (RULING #9817):
#  - a source ending in `.S` is a vendored hand-written assembly file. It gets
#    cpp -lang-asm and then the WHOLE file goes through the held SN
#    Ps2EeAs.exe, the SDK's own assembler (no cc1, no move_fixup.sed, no
#    mips-linux-gnu-as). EE gas and GNU as both give different bytes for it
#    (FACT #9816).
#  - the literal `ld-r` makes a group: the members named in the rest of the row
#    are built from their own rows and combined, unmodified and in that order,
#    by `ld -r`. Only the group goes into the archive. It exists because .cod is
#    SUBALIGN(8): a lone 4-aligned member cannot sit at a 4 mod 8 ROM address,
#    but ld -r keeps each input's own 4-alignment inside the group, as the SN
#    link did.
# Output: going-decompiled/build/<region>/lib/libgcc.a (the yaml's lib_path) and
# lib/members.txt (one member per line) for build.sh's INPUT/EXTERN lines.
set -e
REGION="${1:-usa}"
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
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
# build_member <member>: $OUT/<member>.o from its MEMBERS row.
build_member() {
  m=$1
  row=$(awk -v m="$m" '$1 == m' "$SRC/MEMBERS")
  [ -n "$row" ] || { echo "BUILD FAIL (libgcc): $LD names libgcc.a:$m.o but $SRC/MEMBERS has no row for it" >&2; exit 1; }
  src=$(echo "$row" | awk '{print $2}')
  defs=$(echo "$row" | awk '{ $1 = ""; $2 = ""; sub(/^ +/, ""); print }')
  i="$OUT/$m.i"; s="$OUT/$m.s"; o="$OUT/$m.o"
  case "$src" in
    ld-r)
      # build_member reuses these globals (POSIX sh has no `local`), so the
      # group's own are restored before ld -r.
      group=$m; group_parts=$defs; parts=""
      for p in $group_parts; do build_member "$p"; parts="$parts $OUT/$p.o"; done
      m=$group; o="$OUT/$m.o"
      mips-linux-gnu-ld -EL -r -o "$o" $parts \
        || { echo "BUILD FAIL (libgcc ld -r): $m" >&2; exit 1; } ;;
    *.S)
      "$WIBO" "$G/cpp.exe" $PRE -lang-asm $defs "$SRC/$src" "$i" \
        || { echo "BUILD FAIL (libgcc cpp): $m" >&2; exit 1; }
      tr -d '\r' < "$i" > "$s"
      # Ps2EeAs writes beside its output, so it runs in $OUT.
      (cd "$OUT" && "$WIBO" "$ROOT/tools/ee/cc/ee/bin/Ps2EeAs.exe" -o "$m.o" "$m.s" > /dev/null) \
        || { echo "BUILD FAIL (libgcc Ps2EeAs): $m" >&2; exit 1; } ;;
    *)
      "$WIBO" "$G/cpp.exe" $PRE -I"$SRC/shim" $defs "$SRC/$src" "$i" \
        || { echo "BUILD FAIL (libgcc cpp): $m" >&2; exit 1; }
      "$WIBO" "$G/cc1.exe" -quiet -O2 -G0 "$i" -o "$s" \
        || { echo "BUILD FAIL (libgcc cc1): $m" >&2; exit 1; }
      sed -E -f tools/ee/move_fixup.sed "$s" | tr -d '\r' | perl tools/ee/libgcc_lid_dli.pl \
        | mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -o "$o" - \
        || { echo "BUILD FAIL (libgcc as): $m" >&2; exit 1; } ;;
  esac
  [ -s "$o" ] || { echo "BUILD FAIL (libgcc: no object): $m" >&2; exit 1; }
}
objs=""
for m in $MEMBERS; do
  build_member "$m"
  objs="$objs $OUT/$m.o"
  echo "$m" >> "$OUT/members.txt"
done
mips-linux-gnu-ar rcs "$OUT/libgcc.a" $objs
echo "   libgcc: built $(wc -l < "$OUT/members.txt" | tr -d ' ') member(s) into $OUT/libgcc.a:" $MEMBERS

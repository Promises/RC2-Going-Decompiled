#!/bin/sh
# Asm round-trip / matching build for one region. Runs INSIDE the ee-build
# container in the colima x86 VM:
#
#   docker --context colima-ee-x86 run --rm -v "$PWD":/work ee-build \
#       sh tools/ee/build.sh usa
#
# Assembles every split .s with modern GNU mips binutils (the period ee-gcc is
# only for compiling C), links with the splat-generated ld script, flattens to
# a .rom, and diffs vs the original. C compilation of src/ is added later.
set -e
REGION="${1:-usa}"
cd "$(dirname "$0")/../.."        # repo root inside container (/work)

case "$REGION" in
  usa) BASENAME=SCUS_972.68 ;;
  eu)  BASENAME=SCES_516.07 ;;
  *) echo "unknown region $REGION"; exit 2 ;;
esac

ASM=going-decompiled/asm/$REGION
BUILD=going-decompiled/build/$REGION
INC=$BUILD/include
LD=going-decompiled/linker_scripts/$BASENAME.ld
ORIG=extracted/$REGION/$BASENAME.rom
ELF=$BUILD/$BASENAME.elf
ROM=$BUILD/$BASENAME.rom

ASFLAGS="-march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -I $INC -I $ASM -I $BUILD"

echo "== [$REGION] assembling $(find $ASM -name '*.s' | wc -l) asm files =="
n=0
for s in $(find $ASM -name '*.s'); do
  # object path must match what the ld script references: $BUILD/<full .s path>.o
  o="$BUILD/${s%.s}.o"
  mkdir -p "$(dirname "$o")"
  mips-linux-gnu-as $ASFLAGS -o "$o" "$s" 2> "$o.log" || { echo "AS FAIL $s:"; tail -5 "$o.log"; exit 1; }
  n=$((n+1))
done
echo "   assembled $n objects"

echo "== [$REGION] linking with $LD =="
SYMS="$BUILD/undefined_syms_auto.txt"
# Blanket-define every D_<hex> symbol to its absolute address (spimdisasm names
# auto-symbols by address). Resolves references it didn't emit labels for.
ALLSYMS="$BUILD/all_addr_syms.ld"
grep -rhoE '(D_|func_)[0-9A-Fa-f]{4,}' "$ASM" | sort -u | sed -E 's/^(D_|func_)([0-9A-Fa-f]+)$/\1\2 = 0x\2;/' > "$ALLSYMS"
echo "   defined $(wc -l < "$ALLSYMS") address symbols"
mips-linux-gnu-ld -EL --allow-multiple-definition -T "$LD" -T "$SYMS" -T "$ALLSYMS" -Map "$BUILD/$BASENAME.map" -o "$ELF" 2> "$BUILD/ld.log" \
  || { echo "LD errors (first 20):"; head -20 "$BUILD/ld.log"; }

if [ -f "$ELF" ]; then
  echo "== [$REGION] ELF built; flattening + diff =="
  mips-linux-gnu-objcopy -O binary "$ELF" "$ROM" 2>/dev/null || true
  echo -n "orig  "; sha1sum "$ORIG" | awk '{print $1, '$(stat -c%s "$ORIG" 2>/dev/null || echo "?")'}'
  echo -n "built "; sha1sum "$ROM"  2>/dev/null | awk '{print $1}'
  if cmp -s "$ORIG" "$ROM"; then echo "MATCH: byte-identical .rom"; else
    echo "DIFF: $(cmp -l "$ORIG" "$ROM" 2>/dev/null | wc -l) differing bytes (of $(stat -c%s "$ORIG") )"
  fi
else
  echo "== no ELF produced (link incomplete) =="
fi

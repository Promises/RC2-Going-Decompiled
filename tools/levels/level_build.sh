#!/bin/sh
# Build one USA level program from its committed split asm and compare it with the
# disc (task #2457, NOTE #13098 row L4).
#
#   tools/levels/level_build.sh 24 [--expect <programs.tsv>]
#
# On the host it re-runs itself in the ee-build image, in the same docker context
# as the boot gate (EE_DOCKER_CONTEXT, default colima-ee-x86), and prints the wall
# time with the host and the context. In the container (`--in-container`):
#   1. GNU as, the boot's flags (-march=r5900 -mabi=eabi -no-pad-sections -EL -G0),
#      every .s under going-decompiled/levels/usa/LNN/asm, through the boot's VU0
#      operand fixup (tools/ee/vu0_fixup.sed, read-only) -> <build>/obj/...o;
#   1b. C rows (config/LNN_c.tsv, task #2495): gen_ld.py --cut cuts each listed
#      function's block out of the .text asm, and the pieces are assembled
#      instead of the whole file. Each function is compiled ALONE by
#      tools/ee/ee_cc1.sh <arm> at -O2 <gflag> (+ -fopt-stack for s136, + the
#      row's cc1flags column), then assembled by the row's `as` column (task
#      #2552, RULING #13291 section 1; a 4-column row is `as -`):
#        as:       tools/ee/move_fixup.sed, then GNU as at <gflag>;
#        asm_unit: the boot's tools/ee/asm_unit.sh usa <cc1 .s> <obj> <gflag>
#                  (read-only), on cc1's own output: asm_unit.sh applies
#                  move_fixup.sed itself, then its SN-as model passes. It runs
#                  with ASMFIX_SHARED set to an EMPTY level-scoped mirror
#                  (<build>/c/asmfix_level, `.built` only), so no boot
#                  nonmatchings asm or boot macro.inc is copied or reachable;
#                  its stderr must say `dli: 0 transforms` (no RULING #8549
#                  boot allowlist row reached level code), else FAIL.
#      Pieces and C objects get
#      their source's own .text alignment (objcopy), and level_elf.py cobj
#      checks each C object (only .text, of the block's size, one global).
#      After the link, a `C-PLACED ok` row needs the map to put the object at
#      the function's address and size, with its glabel in no assembled asm;
#      then `level progress LNN: <C rows> of <LNN_functions.txt entries>`.
#   1c. Boot C placed (config/LNN_boot_c.tsv, task #2516, RULING #13219): the same
#      cut takes each row's block out; the pieces of a cut asm file are ONE file
#      of `.text.cutNNNN` sections (gen_ld.py: its `.align` lines dropped, each
#      checked already satisfied), assembled once, every piece section given
#      alignment 4. The row's object is the boot's own sectioned unit object
#      (tools/levels/boot_objects.sh, run on the host by level_gate.sh), never a
#      compile here. Before the link: gen_boot_names.py names --check (the
#      committed config/LNN_boot_c.tsv and symbol_addrs/LNN_boot_names.txt are
#      what the generator writes from this tree), then BOOTLINK: the same
#      sections linked at their BOOT addresses with the boot values of the same
#      names (gen_boot_names.py bootld) must equal the boot build's ELF there
#      (bootcmp). LNN_boot_names.txt, `//` comments stripped, is an ld script
#      (-T) of the level link. After it, gen_boot_names.py placed prints one
#      `PLACED ok` row per placed function (map address = place = names-map
#      value, map size = catalogue size, alias present, glabel in no assembled
#      asm) and `PLACED-DIFFERS` members; then the ledger line
#      `LNN: boot-copy placed N + level C M of <catalogue functions>`.
#   2. tools/levels/gen_ld.py --check: the committed LNN.ld must be current;
#   3. nm -u over the objects -> level_elf.py undef: core names come from LNN.ld,
#      address-named ones are valued from their name, anything else fails;
#      with placements OFF (no LNN_boot_c.tsv rows) the level's boot-name map
#      entries that some object references as undefined are still linked, and
#      only those (task #2646: level C names A2 objects by the map's name);
#   4. GNU ld -T LNN.ld -T undefined_syms.ld -> <build>/LNN.elf (+ .map, ld.log);
#   5. level_elf.py check: 7 section blobs out of the ELF's one PT_LOAD -> cmp
#      against extracted/usa_levels/NN_*/, entry, repacked archive-form sha1;
#      then `cmp` (the system tool) on every blob pair as a second instrument.
# Prints `#### level_build LNN: PASS|FAIL`. Output goes to
# going-decompiled/build/usa_levels/LNN/ (gitignored). Needs the level extracted
# first (tools/levels/extract_levels.py extract --only NN).
#
# Paths: reads only going-decompiled/levels/, extracted/usa_levels/, tools/levels/
# and, read-only, tools/ee/vu0_fixup.sed, plus for C rows (config/LNN_c.tsv)
# tools/ee/ee_cc1.sh, tools/ee/move_fixup.sed and the compilers under tools/ee/cc,
# plus for `asm_unit` C rows tools/ee/asm_unit.sh and what it reads (its
# tools/ee/vu0_fixup.sed, move_fixup.sed, ps2eeas_dli.awk and the RULING #8549
# allowlist tools/ee/ps2eeas_dli_sites.txt, which keys rows by boot function),
# plus for boot rows (config/LNN_boot_c.tsv) the boot build's SCUS_972.68.elf/.map
# and going-decompiled/build/usa_levels/boot_objs/;
# writes only going-decompiled/build/usa_levels/.
set -e

if [ "$1" != "--in-container" ]; then
  [ -n "$1" ] || { echo "usage: $0 <level number> [--expect <programs.tsv>]" >&2; exit 2; }
  ROOT=$(cd "$(dirname "$0")/../.." && pwd)
  CTX=${EE_DOCKER_CONTEXT:-colima-ee-x86}
  HOSTN=$(uname -n)
  t0=$(date +%s.%N)
  rc=0
  set -- "$1" "$2" "$3"
  if [ "$2" = "--expect" ]; then
    # the TSV (e.g. OpenRAC programs.tsv, read as data) usually lives outside the
    # mounted worktree: hand the container a copy in the gitignored build dir
    E=going-decompiled/build/usa_levels/expect_programs.tsv
    mkdir -p "$ROOT/going-decompiled/build/usa_levels"
    cp "$3" "$ROOT/$E"
    echo "level_build: --expect $3 (sha1 $(sha1sum "$3" | cut -c1-40))"
    set -- "$1" --expect "$E"
  fi
  docker --context "$CTX" run --rm --user="$(id -u):$(id -g)" -e HOME=/tmp \
    -v "$ROOT":/work -w /work ee-build sh tools/levels/level_build.sh --in-container "$@" || rc=$?
  t1=$(date +%s.%N)
  echo "level_build: wall $(awk -v a="$t0" -v b="$t1" 'BEGIN{printf "%.2f", b-a}') s (host $HOSTN, docker context $CTX -> $(docker context inspect "$CTX" --format '{{.Endpoints.docker.Host}}'), rc $rc)"
  exit $rc
fi
shift

NN=$(printf '%02d' "$(expr "$1" + 0)"); shift
EXPECT=""
[ "$1" = "--expect" ] && EXPECT="$2"
TAG=L$NN
LVL=going-decompiled/levels/usa/$TAG
YAML=$LVL/config/$TAG.yaml
LD=$LVL/linker_scripts/$TAG.ld
ASM=$LVL/asm
BUILD=going-decompiled/build/usa_levels/$TAG
EXT=$(ls -d extracted/usa_levels/${NN}_* 2>/dev/null | head -1)
INC=going-decompiled/levels/usa/include
VU0FIX=tools/ee/vu0_fixup.sed
ASFLAGS="-march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -I $INC"
[ -f "$YAML" ] || { echo "level_build: no $YAML" >&2; exit 2; }
[ -n "$EXT" ] && [ -f "$EXT/manifest.json" ] || { echo "level_build: level $NN not extracted under extracted/usa_levels/" >&2; exit 2; }

now() { date +%s.%N; }
lap() { awk -v a="$1" -v b="$(now)" 'BEGIN{printf "%.2f", b-a}'; }
T0=$(now)
rm -rf "$BUILD/obj" "$BUILD/blobs" "$BUILD/$TAG.elf" "$BUILD/$TAG.map" "$BUILD/ld.log" "$BUILD/undefined_syms.ld" "$BUILD/nm_u.txt" "$BUILD/nm_def.txt" "$BUILD/core_names.txt" \
  "$BUILD/bootlink.ld" "$BUILD/bootlink.elf" "$BUILD/bootlink.map" "$BUILD/bootlink.log" "$BUILD/boot_names.ld" \
  "$BUILD/boot_names.txt" "$BUILD/boot_names_check.log" "$BUILD/placed.log"
mkdir -p "$BUILD/obj"

# C functions (task #2495): config/LNN_c.tsv names them; gen_ld.py --cut writes
# the .text asm pieces without their blocks and prints CUT/CFN lines.
CTSV=$LVL/config/${TAG}_c.tsv
CUTS="$BUILD/cut.txt"
rm -rf "$BUILD/cut" "$BUILD/c"; : > "$CUTS"
BTSV=$LVL/config/${TAG}_boot_c.tsv
if [ -f "$CTSV" ] || [ -f "$BTSV" ]; then
  echo "== [$TAG] cutting the C / boot-placed functions of $CTSV / $BTSV out of the .text asm =="
  python3 -I tools/levels/gen_ld.py "$YAML" --cut > "$CUTS" || { cat "$CUTS"; echo "#### level_build $TAG: FAIL (cut)"; exit 1; }
  cat "$CUTS"
fi

as_one() { # <src .s> <out .o>
  mkdir -p "$(dirname "$2")"
  sed -f "$VU0FIX" "$1" | mips-linux-gnu-as $ASFLAGS -o "$2" - 2> "$2.log" \
    || { echo "AS FAIL $1:"; tail -5 "$2.log"; echo "#### level_build $TAG: FAIL (as)"; exit 1; }
}
# GNU as for MIPS records .text alignment 16 whatever the source asks, so ld
# would start a cut piece or a C object at the next 16-byte boundary
# (func_L24_0034D598 landed at 0x34D5A0, not 0x34D598, before this). Set the
# object's .text alignment to the largest .align/.p2align its own source
# requests (2^N, at least 4). Only pieces and C objects get this; a whole,
# uncut .s keeps as's alignment, and it starts at a 16-byte-aligned dest.
realign() { # <obj> <src .s>
  pmax=$(sed -n -E 's/^[[:space:]]*\.(p2)?align[[:space:]]+([0-9]+).*/\2/p' "$2" | sort -n | tail -1)
  a=$((1 << ${pmax:-2})); [ "$a" -ge 4 ] || a=4
  mips-linux-gnu-objcopy --set-section-alignment .text=$a "$1" \
    || { echo "#### level_build $TAG: FAIL (objcopy alignment $1)"; exit 1; }
  echo "   $(basename "$1"): .text alignment set to $a (largest .align in its source: 2^${pmax:-2})"
}

echo "== [$TAG] assembling $ASM =="
T=$(now); n=0
for s in $(find "$ASM" -name '*.s' | sort); do
  rel=${s#$ASM/}; o="$BUILD/obj/$rel"; o="${o%.s}.o"
  np=$(awk -v f="$s" '$1=="CUT" && $2==f {print $3}' "$CUTS")
  if [ -n "$np" ]; then
    cs=$(awk -v f="$s" '$1=="CUT" && $2==f {print $4}' "$CUTS")
    as_one "$cs" "${o%.o}.cut.o"
    # every piece section to alignment 4: gen_ld.py dropped the pieces' .align
    # lines (each checked already satisfied by its next address), so a piece is
    # placed by its bytes alone, right after the previous section
    k=0; args=""
    while [ $k -lt "$np" ]; do args="$args --set-section-alignment $(printf '.text.cut%04d' $k)=4"; k=$((k+1)); done
    mips-linux-gnu-objcopy $args "${o%.o}.cut.o" || { echo "#### level_build $TAG: FAIL (objcopy piece alignment)"; exit 1; }
    nsec=$(mips-linux-gnu-readelf -SW "${o%.o}.cut.o" | /usr/bin/grep -c ' \.text\.cut[0-9]' || true)
    [ "$nsec" = "$np" ] || { echo "   $rel: $nsec .text.cutNNNN sections in the object, $np pieces"; echo "#### level_build $TAG: FAIL (pieces)"; exit 1; }
    n=$((n+1))
    echo "   $rel: assembled as one object of $np piece sections (alignment 4; C and boot blocks cut out)"
  else
    as_one "$s" "$o"; n=$((n+1))
  fi
done
echo "   assembled $n objects in $(lap $T) s"

# compile each C row ALONE (tools/ee/ee_cc1.sh read-only: the boot's cpp/cc1
# lines per arm, plus the row's cc1flags), then assemble it by the row's `as`
# column: `as` = the boot's move/cvt.w.s/break fixup (tools/ee/move_fixup.sed,
# read-only) and GNU as at the row's -G; `asm_unit` = tools/ee/asm_unit.sh
# (read-only) on cc1's output at the row's -G. Either way the object then gets
# its source's .text alignment and level_elf.py cobj checks it.
CPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__"
# asm_unit.sh's mirror, level-scoped and EMPTY: asm_unit.sh skips building a
# mirror whose `.built` marker exists, so the boot's nonmatchings asm and the
# boot build's macro.inc are never copied (a level C unit .includes nothing).
AUMIRROR="$PWD/$BUILD/c/asmfix_level"
NC=0
while read -r kind fn addr size src arm g asr cfl; do
  [ "$kind" = CFN ] || continue
  T=$(now)
  mkdir -p "$BUILD/c" "$BUILD/obj/c"
  tailf=""; [ "$arm" = s136 ] && tailf="-fopt-stack"
  extra=""; [ "$cfl" = "-" ] || extra=$(printf '%s' "$cfl" | tr ',' ' ')
  inc=""; [ -d "$LVL/include" ] && inc="-I$LVL/include"
  sh tools/ee/ee_cc1.sh "$arm" "$src" "$BUILD/c/$fn.i" "$BUILD/c/$fn.s" "$CPPDEF $inc" "-O2 $g $tailf${extra:+ $extra}" > "$BUILD/c/$fn.cc.log" 2>&1 \
    || { cat "$BUILD/c/$fn.cc.log"; echo "#### level_build $TAG: FAIL (cc1 $fn)"; exit 1; }
  [ -s "$BUILD/c/$fn.cc.log" ] && { echo "   cc1 printed for $fn (all $(wc -l < "$BUILD/c/$fn.cc.log") lines):"; sed 's/^/     /' "$BUILD/c/$fn.cc.log"; }
  case "$asr" in
    as)
      sed -E -f tools/ee/move_fixup.sed "$BUILD/c/$fn.s" > "$BUILD/c/$fn.fx.s"
      mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL "$g" -o "$BUILD/obj/c/$fn.o" "$BUILD/c/$fn.fx.s" 2> "$BUILD/c/$fn.as.log" \
        || { cat "$BUILD/c/$fn.as.log"; echo "#### level_build $TAG: FAIL (as $fn)"; exit 1; }
      realign "$BUILD/obj/c/$fn.o" "$BUILD/c/$fn.fx.s"
      ;;
    asm_unit)
      rm -rf "$AUMIRROR"; mkdir -p "$AUMIRROR"; : > "$AUMIRROR/.built"
      ASMFIX_SHARED="$AUMIRROR" sh tools/ee/asm_unit.sh usa "$PWD/$BUILD/c/$fn.s" "$PWD/$BUILD/obj/c/$fn.o" "$g" 2> "$BUILD/c/$fn.as.log" \
        || { cat "$BUILD/c/$fn.as.log"; echo "#### level_build $TAG: FAIL (asm_unit $fn)"; exit 1; }
      # the boot allowlist must not have reached level code, and the mirror
      # must still be empty (nothing of the boot's asm was copied in)
      dli=$(/usr/bin/grep -a -c '^asm_unit.sh: dli: 0 transforms ' "$BUILD/c/$fn.as.log" || true)
      nmir=$(find "$AUMIRROR" -mindepth 1 ! -name .built | wc -l | tr -d ' ')
      sed 's/^/     /' "$BUILD/c/$fn.as.log"
      [ "$dli" = 1 ] && [ "$nmir" = 0 ] \
        || { echo "   asm_unit $fn: 'dli: 0 transforms' lines $dli (want 1), files in the level mirror besides .built $nmir (want 0)"; echo "#### level_build $TAG: FAIL (asm_unit boot-key check $fn)"; exit 1; }
      realign "$BUILD/obj/c/$fn.o" "$BUILD/c/$fn.s"
      ;;
    *) echo "level_build: $fn: assembler '$asr' not as|asm_unit"; echo "#### level_build $TAG: FAIL (as column)"; exit 1;;
  esac
  python3 -I tools/levels/level_elf.py cobj "$BUILD/obj/c/$fn.o" "$fn" "$size" || { echo "#### level_build $TAG: FAIL (cobj $fn)"; exit 1; }
  echo "   C $fn: $src, arm $arm, cc1 -O2 $g $tailf${extra:+ $extra}, assembler $asr, object sha1 $(sha1sum "$BUILD/obj/c/$fn.o" | cut -c1-40), in $(lap $T) s"
  NC=$((NC+1))
done < "$CUTS"

# boot C placed (task #2516): the committed name map must be the generator's
# output for this tree, and the placed sections must reproduce the boot image
# at their boot addresses (the boot gate proved that image == ROM)
NB=$(awk '$1=="BFN"' "$CUTS" | wc -l | tr -d ' ')
LATE=""  # failures that still let the link and the byte compare run (their members matter)
BNAMES=$LVL/symbol_addrs/${TAG}_boot_names.txt
BOOTLD=""
if [ "$NB" -gt 0 ]; then
  T=$(now)
  echo "== [$TAG] boot C placed: $NB rows in $LVL/config/${TAG}_boot_c.tsv =="
  for u in $(awk '$1=="BFN"{print $6}' "$CUTS" | sort -u); do
    [ -s "going-decompiled/build/usa_levels/boot_objs/$u.o" ] || { echo "level_build: no boot object going-decompiled/build/usa_levels/boot_objs/$u.o (run tools/levels/boot_objects.sh)"; echo "#### level_build $TAG: FAIL (boot objects)"; exit 1; }
  done
  grc=0; python3 -I tools/levels/gen_boot_names.py names --level "$NN" --check > "$BUILD/boot_names_check.log" 2>&1 || grc=$?
  /usr/bin/grep -a -E '^(REFUSED|   )' "$BUILD/boot_names_check.log"
  tail -1 "$BUILD/boot_names_check.log"
  [ $grc = 0 ] || { echo "level_build: FAIL boot names map stale (continuing to the link and compare)"; LATE="$LATE boot-names-stale"; }
  python3 -I tools/levels/gen_boot_names.py bootld --level "$NN" "$BUILD/bootlink.ld" || { echo "#### level_build $TAG: FAIL (bootld)"; exit 1; }
  if mips-linux-gnu-ld -EL -T "$BUILD/bootlink.ld" -o "$BUILD/bootlink.elf" -Map "$BUILD/bootlink.map" > "$BUILD/bootlink.log" 2>&1; then
    python3 -I tools/levels/gen_boot_names.py bootcmp --level "$NN" "$BUILD/bootlink.elf" || { echo "level_build: FAIL bootlink cmp (continuing)"; LATE="$LATE bootlink-cmp"; }
  else
    echo "level_build: FAIL bootlink ld (first 20 of $(wc -l < "$BUILD/bootlink.log") lines of $BUILD/bootlink.log; continuing to the level link):"
    head -20 "$BUILD/bootlink.log"; LATE="$LATE bootlink-ld"
  fi
  sed 's|//.*$||' "$BNAMES" > "$BUILD/boot_names.ld"
  BOOTLD="$BUILD/boot_names.ld"
  echo "   boot C checks in $(lap $T) s"
fi

T=$(now)
python3 -I tools/levels/gen_ld.py "$YAML" --check || { echo "#### level_build $TAG: FAIL (stale $LD)"; exit 1; }

OBJS=$(find "$BUILD/obj" -name '*.o' | sort)
mips-linux-gnu-nm -u $OBJS | awk 'NF==2 && $1=="U"{print $2}' | sort -u > "$BUILD/nm_u.txt"
mips-linux-gnu-nm -g --defined-only $OBJS | awk 'NF==3{print $3}' | sort -u > "$BUILD/nm_def.txt"
if [ -z "$BOOTLD" ] && [ -f "$BNAMES" ]; then
  # placements OFF (no config/LNN_boot_c.tsv rows), task #2646: level C names
  # level-space objects by the level's boot-name map (g_mobyTableBase, not the
  # address-named second spelling D_L02_001B359C; class A2). Link ONLY the map's
  # level-space entries (core names come from LNN.ld) that some level object
  # references and none defines, never an ADDR_NAME (level_elf.py values those
  # from their names): nothing is placed and no other map name enters the link.
  # Unsanctioned by RULING #13219 until watcher-2 accepts it: dropping it means
  # reverting the A2 respell.
  sed 's|//.*$||' "$BNAMES" | awk 'FILENAME==ARGV[1]{u[$1]=1; next} FILENAME==ARGV[2]{d[$1]=1; next}
    $2=="=" && ($1 in u) && !($1 in d) && $1 !~ /^(func|D|jtbl|B|RO|T|FLT|DBL|STR|PSTR)_(L[0-9][0-9]_)?[0-9A-F]+$/ { v=$3; sub(/;.*/, "", v); if (length(v) == 10 && v >= "0x001A7C80") print }' "$BUILD/nm_u.txt" "$BUILD/nm_def.txt" - > "$BUILD/boot_names.ld"
  NREF=$(wc -l < "$BUILD/boot_names.ld" | tr -d ' ')
  if [ "$NREF" -gt 0 ]; then
    BOOTLD="$BUILD/boot_names.ld"
    echo "   placements off: $NREF boot-name map entries referenced by level objects linked from $BNAMES: $(awk '{print $1}' "$BOOTLD" | tr '\n' ' ')"
  fi
fi
python3 -I tools/levels/level_elf.py undef "$LD" "$BUILD/nm_u.txt" "$BUILD/nm_def.txt" "$BUILD/undefined_syms.ld" $BOOTLD \
  || { echo "#### level_build $TAG: FAIL (unresolved symbols)"; exit 1; }
if [ -n "$BOOTLD" ]; then
  # a name the boot names script assigns that a level object ALSO defines would
  # be silently overridden by the assignment
  sed -n 's/^[[:space:]]*\([A-Za-z_][A-Za-z0-9_.$]*\) = .*/\1/p' "$BOOTLD" | sort -u > "$BUILD/boot_names.txt"
  BDUP=$(comm -12 "$BUILD/nm_def.txt" "$BUILD/boot_names.txt")
  [ -z "$BDUP" ] || { echo "level_build: boot names also defined by a level object: $BDUP"; echo "#### level_build $TAG: FAIL (boot/level name clash)"; exit 1; }
fi
# a core name the level ALSO defines would be silently overridden by the script's assignment
CORE=$(sed -n 's/^\([A-Za-z_][A-Za-z0-9_.$]*\) = ABSOLUTE(.*/\1/p' "$LD" | sort -u)
printf '%s\n' "$CORE" > "$BUILD/core_names.txt"
DUP=$(comm -12 "$BUILD/nm_def.txt" "$BUILD/core_names.txt")
[ -z "$DUP" ] || { echo "level_build: core names also defined by the level: $DUP"; echo "#### level_build $TAG: FAIL (core/level name clash)"; exit 1; }

echo "== [$TAG] linking with $LD =="
mips-linux-gnu-ld -EL -T "$LD" ${BOOTLD:+-T "$BOOTLD"} -T "$BUILD/undefined_syms.ld" -Map "$BUILD/$TAG.map" -o "$BUILD/$TAG.elf" > "$BUILD/ld.log" 2>&1 \
  || { echo "LD FAIL (all of ld.log, $(wc -l < "$BUILD/ld.log") lines):"; cat "$BUILD/ld.log"; echo "#### level_build $TAG: FAIL (ld)"; exit 1; }
[ -s "$BUILD/ld.log" ] && { echo "ld printed (all $(wc -l < "$BUILD/ld.log") lines):"; cat "$BUILD/ld.log"; }
echo "   linked $BUILD/$TAG.elf ($(wc -c < "$BUILD/$TAG.elf") B, sha1 $(sha1sum "$BUILD/$TAG.elf" | cut -c1-40)) in $(lap $T) s"

# each C function: the map places its object's .text at the address in its
# name, and no assembled asm defines its glabel (its block was cut, not linked).
CRC=0
while read -r kind fn addr size src arm g asr cfl; do
  [ "$kind" = CFN ] || continue
  mline=$(awk -v o="$BUILD/obj/c/$fn.o" '$1==".text" && $4==o' "$BUILD/$TAG.map")
  maddr=""; msize=""
  if [ -n "$mline" ]; then  # mawk has no strtonum: the shell's printf reads 0x...
    maddr=$(printf '0x%08X' "$(printf '%s' "$mline" | awk '{print $2}')")
    msize=$(printf '0x%X' "$(printf '%s' "$mline" | awk '{print $3}')")
  fi
  nasm=$(cat "$BUILD"/cut/*/*.s "$BUILD"/cut/*.s 2>/dev/null | awk -v f="$fn" '$1=="glabel" && $2==f' | wc -l)
  if [ "$maddr" = "$addr" ] && [ "$msize" = "$size" ] && [ "$nasm" = 0 ]; then
    echo "C-PLACED ok $fn: map '.text $maddr $msize $BUILD/obj/c/$fn.o'; glabel $fn in the assembled asm pieces: $nasm"
  else
    echo "C-PLACED FAIL $fn: map line '${mline:-none}' (want $addr $size), glabel lines in assembled asm $nasm (want 0)"; CRC=1
  fi
done < "$CUTS"
[ $CRC = 0 ] || { echo "level_build: FAIL C placement (continuing to the compare)"; LATE="$LATE C-placement"; }
NFN=$(awk '/^func_/' "$LVL/symbol_addrs/${TAG}_functions.txt" | wc -l)
echo "level progress $TAG: $NC of $NFN catalogue functions from C (C rows in config/${TAG}_c.tsv; functions = entries in symbol_addrs/${TAG}_functions.txt)"
NPL=0
if [ "$NB" -gt 0 ]; then
  prc=0; python3 -I tools/levels/gen_boot_names.py placed --level "$NN" "$BUILD/$TAG.elf" "$BUILD/$TAG.map" "$EXT/text.bin" "$BUILD/cut" > "$BUILD/placed.log" 2>&1 || prc=$?
  cat "$BUILD/placed.log"
  NPL=$(/usr/bin/grep -a -c '^PLACED ok ' "$BUILD/placed.log" || true)
  [ $prc = 0 ] && [ "$NPL" = "$NB" ] || { echo "level_build: FAIL boot placement: $NPL PLACED ok of $NB rows (continuing to the compare)"; LATE="$LATE boot-placement"; }
fi
echo "ledger $TAG: boot-copy placed $NPL + level C $NC of $NFN (a level ledger line: never added to the USA INCLUDE_ASM figure, RULING #8118)"

echo "== [$TAG] comparing with $EXT =="
T=$(now)
rc=0
if [ -n "$EXPECT" ]; then
  python3 -I tools/levels/level_elf.py check "$BUILD/$TAG.elf" "$EXT" "$BUILD/blobs" "$YAML" --expect "$EXPECT" --version us2 --program "$(basename "$EXT")" || rc=1
else
  python3 -I tools/levels/level_elf.py check "$BUILD/$TAG.elf" "$EXT" "$BUILD/blobs" "$YAML" || rc=1
fi
for st in lit bss data vtbl camvtbl sndvtbl text; do
  c=$(cmp -l "$BUILD/blobs/$st.bin" "$EXT/$st.bin" 2>&1 | wc -l)
  echo "cmp(1) $st: $c differing ($(wc -c < "$BUILD/blobs/$st.bin") vs $(wc -c < "$EXT/$st.bin") B)"
  [ "$c" = 0 ] || rc=1
done
echo "   compared in $(lap $T) s; in-container total $(lap $T0) s"
[ -z "$LATE" ] || { echo "level_build: earlier FAILs:$LATE"; rc=1; }
if [ $rc = 0 ]; then echo "#### level_build $TAG: PASS"; else echo "#### level_build $TAG: FAIL"; fi
exit $rc

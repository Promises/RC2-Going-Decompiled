#!/bin/sh
# s136os_splice.sh — the s136os compile arm (task #1257, FACT #8810): compile a
# SELECTED function alone with SN 2.95.3 v1.36 `-fopt-stack` and splice its
# `.ent`..`.end` block into the unit's cc1 2.9 output, so the image carries C
# compiled by the compiler the ROM's function was built with.
#
#   sh tools/ee/s136os_splice.sh <region> <unit> <src> <unit.s> <GFLAG> [CC1EXTRA]
#   e.g. sh tools/ee/s136os_splice.sh usa text/248B50 \
#            going-decompiled/src/usa/text/248B50.c $BUILD/.../248B50._u.s -G8 -fno-gcse
#
# ONE helper, two callers: build.sh (the image) and objdiff_build.sh (the unit
# report's sdk29 base), each between its cc1 2.9 step and asm_unit.sh. Runs
# INSIDE the ee-build container at the repo root, POSIX sh + awk. <unit.s> is
# rewritten in place; a unit with no selector row and no slot is left untouched.
#
# SELECTOR: tools/ee/s136os_functions.txt, one `<region> <unit> <function>`
# row per selected function (unit without extension, as objdiff names it).
# THE SOURCE SIDE of a selected function is its guard, re-predicated from
#     #ifndef TARGET_NATIVE / INCLUDE_ASM(...) / #else <C> / #endif
# to
#     #if !defined(TARGET_NATIVE) && !defined(S136OS_<fn>)
#     S136OS_SLOT(<fn>);
#     #else
#     <C>
#     #endif
# The 2.9 compile sees S136OS_SLOT, which emits only the asm comment
# `#S136OS_SLOT <fn>` (include_asm.h): no bytes, no INCLUDE_ASM. This helper
# compiles the same source with -DS136OS_<fn>, which opens THAT function's C
# (every other guard in the unit is unchanged, so the s136os TU is the unit as
# the 2.9 arm sees it plus that one body: the configuration FACT #8810
# measured), and replaces the slot line with the block. Native compiles the C,
# as before.
# ⇒ If this helper never runs, the slot assembles to NOTHING: the function is
#   absent from the object and every later byte of the image shifts, so a
#   skipped or no-op splice cannot read as cmp 0 (it would if the slot were the
#   INCLUDE_ASM it replaces).
#
# FATAL (rc 3, nothing written) — each lists every offender:
#   - a malformed selector row, or a row repeated;
#   - a row with no slot in <unit.s>, or a slot with no row (the source guard
#     and the selector disagree);
#   - the unit's s136 front end missing or not its pinned sha256: a `.c` unit
#     needs the SN 1.36 cc1 (0393bcd3..., FACT #8810), a `.cpp` unit the SN 1.36
#     cc1plus (78a0df90..., task #1284). A `.cpp` unit NEVER falls back to cc1:
#     an unprovisioned cc1plus is fatal (run scripts/fetch_ee_toolchain.sh);
#   - the s136 compile failing (for C++ that includes any cc1plus diagnostic
#     exit and a missing __gnu_compiled_cplusplus marker; ee_cc1.sh checks);
#   - the s136os output has no single `.ent <fn>`..`.end <fn>` block;
#   - SHAPE: the block switches section (rodata/data/sdata literal, jump table)
#     or references a `$L` label it does not define (a `$LC` string/float
#     literal). Those live outside the block and are not spliced, so such a
#     function is out of this arm's domain until the rodata side is solved.
# THE COMPILE is `tools/ee/ee_cc1.sh s136`, the one place the C/C++ rule lives:
# a `.c` unit runs cpp + 1.36 cc1 (the command lines this helper ran before);
# a `.cpp` unit runs cpp -lang-c++, ONE extern "C" wrapper, and 1.36 cc1plus
# -fno-exceptions -fno-rtti — the same front-end handling as the unit's 2.9
# compile. Measured codegen-identical to 1.36 cc1 on every selector member,
# block for block (task #1284).
# The block's own `$L<n>` labels are renamed `$L<n>_s136_<fn>` (still `$L`, so
# local and absent from the symtab, as cc1's are) because both TUs number
# their labels from the same counter.
#
# .extern CARRY (task #1281, FACT #8838/#8842): gas decides a bare-symbol load's
# gp-relativity from `.extern <sym>, <size>` (-G8 units), and the 2.9 TU never
# saw the body that needs it, so the s136os TU's `.extern` lines for symbols
# <unit.s> does NOT declare are carried in front of the block. KEYED ON THE
# SYMBOL NAME: a symbol <unit.s> already declares, at any size, is never
# carried. Measured on the container's GNU as 2.40 (-G8): a size > -G declared
# BEFORE a use pins that use absolute and a later smaller size does not undo
# it, but a size <= -G placed BEFORE the use (as the carry places it) makes it
# gp-relative. So a carried `, 4`/`, 1` in front of the block overrode a unit's
# deliberate `.extern <sym>, 16` absolute device (1CA080.c, 1FFBA0.c,
# 235FE8.c) for the block AND every later use in the unit, and walled 6 rows.
# A symbol the unit never declares is unaffected by placement: gas defers that
# decision to the end of the file, so the carry still decides it as the s136os
# TU's own end-of-file line does.
#   sh tools/ee/s136os_splice.sh --selftest   host or container; rc 0 PASS
set -eu

# carry_externs <unit.s> <s136os.s>: print, tab-indented and sorted, the
# s136os TU's .extern lines for symbols <unit.s> has no .extern for.
carry_externs() {
  awk '{ sub(/\r$/, "") }
    FNR == 1 { file++ }
    $1 != ".extern" { next }
    { x = $0; gsub(/[ \t]+/, " ", x); sub(/^ /, "", x); sym = $2; sub(/,.*/, "", sym) }
    file == 1 { have[sym] = 1; next }
    !(sym in have) { print x }' "$1" "$2" | sort -u | sed 's/^/\t/'
}

# extract_block <fn> <s136os.s> <out>: write the block — the .align/.p2align/
# .globl/.text/.section .text directives cc1 prints right before `.ent <fn>`,
# through `.end <fn>` — to <out>; on no single .ent/.end pair, print why.
# `.file` (task #1291, FACT #8856): cc1plus prints `.file 2 "<unit>.cpp"`
# between `.text` and `.ent`, so the scan steps OVER it (stopping there lost the
# member's `.globl` and `.align` and bound it LOCAL), but the line is NOT
# spliced: the unit's own 2.9 TU already declares the same `.file 2` for the
# same source, and a `.c` member's block never carried one either.
extract_block() {
  awk -v fn="$1" -v out="$3" '
    function isdir(l) { return l ~ /^[ \t]*(\.align|\.p2align|\.text|\.section[ \t]+\.text|\.file)([ \t]|$)/ || l ~ ("^[ \t]*\\.globl[ \t]+" fn "[ \t]*$") }
    { L[NR] = $0; sub(/\r$/, ""); K[NR] = $0 }
    $1 == ".ent" && $2 == fn { ne++; s = NR }
    $1 == ".end" && $2 == fn { nd++; e = NR }
    END {
      if (ne != 1 || nd != 1 || e < s) { print "BLOCK " ne " .ent / " nd " .end"; exit }
      b = s; while (b > 1 && isdir(K[b - 1])) b--
      for (i = b; i <= e; i++) if (i >= s || K[i] !~ /^[ \t]*\.file([ \t]|$)/) print L[i] > out
    }' "$2"
}

if [ "${1:-}" = "--selftest" ]; then
  # Arms: X is declared by the unit at 16 (a device) and by the solo TU at 4
  # -> no X line (FACT #8838's wall); Y is solo-only -> carried; Z is declared
  # by both at the same size -> no Z line. A whole-line key (the pre-#1281
  # helper) emits X and fails arm 1; a carry of nothing fails arm 2.
  T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
  printf '\t.extern\tX, 16\n\t.text\n#S136OS_SLOT f\n\t.extern\tZ, 4\n' > "$T/unit.s"
  printf '\t.extern\tX, 16\n\t.ent f\nf:\n\tlw\t$5,X\n\tlw\t$4,Y\n\t.end f\n\t.extern\tZ, 4\n\t.extern\tX, 4\n\t.extern\tY, 4\n' > "$T/solo.s"
  carry_externs "$T/unit.s" "$T/solo.s" > "$T/ext"
  rc=0
  n=$(awk '$1 == ".extern" && $2 ~ /^X,?$/' "$T/ext" | wc -l | tr -d ' ')
  if [ "$n" = 0 ]; then echo "  OK   arm 1: unit .extern X, 16 kept, no .extern X carried"
  else echo "  FAIL arm 1: $n .extern X line(s) carried over the unit's .extern X, 16:"; sed 's/^/        /' "$T/ext"; rc=1; fi
  if [ "$(cat "$T/ext")" = "$(printf '\t.extern Y, 4')" ]; then echo "  OK   arm 2: solo-only .extern Y, 4 carried (the whole carry is exactly that line)"
  else echo "  FAIL arm 2: carry is not exactly '.extern Y, 4':"; sed 's/^/        /' "$T/ext"; rc=1; fi
  # Arm 3 (task #1291, FACT #8856): cc1plus's preamble has `.file 2` between
  # `.text` and `.ent`. The block must keep `.align 3` and `.globl f` and drop
  # the `.file`; a scan that stops at `.file` (the pre-#1291 helper) starts the
  # block at `.ent` and fails here. The `.end g` above bounds the scan.
  printf '\t.file\t1 "u.i"\n\t.end\tg\n\t.align\t3\n\t.globl\tf\n\t.text\n\t.file\t2 "u.cpp"\n\t.ent\tf\nf:\n\tjr\t$31\n\t.end\tf\n' > "$T/cpp.s"
  extract_block f "$T/cpp.s" "$T/blk" > "$T/why"
  if [ ! -s "$T/why" ] && [ "$(cat "$T/blk")" = "$(printf '\t.align\t3\n\t.globl\tf\n\t.text\n\t.ent\tf\nf:\n\tjr\t$31\n\t.end\tf')" ]; then
    echo "  OK   arm 3: .cpp preamble with .file before .ent keeps .align 3 and .globl f, drops .file"
  else echo "  FAIL arm 3: block is not .align 3/.globl f/.text/.ent f..end f:"; cat "$T/why" "$T/blk" 2>/dev/null | sed 's/^/        /'; rc=1; fi
  [ "$rc" = 0 ] && echo "#### s136os_splice --selftest: PASS" || echo "#### s136os_splice --selftest: FAIL"
  exit "$rc"
fi
REGION="$1"; UNIT="$2"; SRC="$3"; UNIT_S="$4"; GFLAG="$5"; CC1EXTRA="${6:-}"
SEL=tools/ee/s136os_functions.txt
G136=tools/ee/cc/lib/gcc-lib/ee/2.95.3
CC1_136_SHA256=0393bcd31f91a6b9f0255db97f1cc99eba78ee8fc003e9a04dfabed1ae1d522e
CC1PLUS_136_SHA256=78a0df900a396098986cbc97a8d3eab6dbbc587a343d4dbd22929a4112c003fb
INC="-Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common"
CPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DINCLUDE_ASM_USE_MACRO_INC=1"

fatal() { echo "s136os_splice: FATAL [$REGION/$UNIT] — $1" >&2; shift; for x in "$@"; do echo "    $x" >&2; done; exit 3; }

[ -f "$SEL" ] || fatal "selector $SEL missing"
[ -f "$UNIT_S" ] || fatal "unit .s $UNIT_S missing"
# MOUNT-SYNC (#542): the selector is host-written; a caller that took its host
# md5 passes it here (objdiff_build.sh, landing_gate.sh). Every short read is
# loud anyway (a dropped row leaves a slot with no row), this only retries it.
if [ -n "${S136OS_FUNCS_MD5:-}" ]; then sh tools/ee/mount_sync.sh check "$SEL" "$S136OS_FUNCS_MD5"; fi

BAD="$(awk '{ sub(/#.*/, "") } NF && (NF != 3 || $1 !~ /^(usa|eu)$/ || $3 !~ /^[A-Za-z_][A-Za-z0-9_]*$/) { print FILENAME ":" FNR ": " $0 }' "$SEL")"
[ -z "$BAD" ] || fatal "malformed row(s) in $SEL (want: <usa|eu> <unit> <function>)" "$BAD"
DUP="$(awk '{ sub(/#.*/, "") } NF { k = $1 " " $2 " " $3; if (seen[k]++) print k }' "$SEL")"
[ -z "$DUP" ] || fatal "repeated row(s) in $SEL" "$DUP"

ROWS="$(awk -v r="$REGION" -v u="$UNIT" '{ sub(/#.*/, "") } NF && $1 == r && $2 == u { print $3 }' "$SEL")"
SLOTS="$(awk '{ sub(/\r$/, "") } $1 == "#S136OS_SLOT" { print $2 }' "$UNIT_S")"
[ -z "$ROWS" ] && [ -z "$SLOTS" ] && exit 0

NOSLOT=""; for f in $ROWS; do printf '%s\n' "$SLOTS" | /usr/bin/grep -qx "$f" || NOSLOT="$NOSLOT $f"; done
NOROW="";  for f in $SLOTS; do printf '%s\n' "$ROWS" | /usr/bin/grep -qx "$f" || NOROW="$NOROW $f"; done
[ -z "$NOSLOT" ] || fatal "selector row(s) with no S136OS_SLOT in $UNIT_S — re-predicate the guard in $SRC to !defined(S136OS_<fn>) with S136OS_SLOT(<fn>):" $NOSLOT
[ -z "$NOROW" ] || fatal "S136OS_SLOT(s) with no row in $SEL — the function would be absent from the object:" $NOROW
case "$SRC" in
  *.c)   FE=cc1.exe;     FE_SHA256="$CC1_136_SHA256" ;;
  *.cpp) FE=cc1plus.exe; FE_SHA256="$CC1PLUS_136_SHA256" ;;
  *) fatal "$SRC is neither a .c nor a .cpp unit; its s136os rows cannot compile:" $ROWS ;;
esac
[ -f "$G136/$FE" ] || fatal "$G136/$FE missing — $SRC needs the SN 2.95.3 v1.36 $FE for its s136os rows (run scripts/fetch_ee_toolchain.sh); no fallback to another front end:" $ROWS
SHA="$(sha256sum "$G136/$FE" | awk '{print $1}')"
[ "$SHA" = "$FE_SHA256" ] || fatal "$G136/$FE sha256 $SHA, not the s136 $FE $FE_SHA256"

TMP="${UNIT_S%.s}._s136"
rm -rf "$TMP"; mkdir -p "$TMP"
OUT="$UNIT_S"
for f in $ROWS; do
  sh tools/ee/ee_cc1.sh s136 "$SRC" "$TMP/$f.i" "$TMP/$f.s" "$CPPDEF $INC -DS136OS_$f" "-O2 $GFLAG $CC1EXTRA -fopt-stack" \
    || fatal "s136 $FE compile failed for $f (ee_cc1.sh s136 $SRC)"
  extract_block "$f" "$TMP/$f.s" "$TMP/$f.blk" > "$TMP/$f.why"
  [ ! -s "$TMP/$f.why" ] || fatal "no single .ent/.end block for $f in $TMP/$f.s ($(cat "$TMP/$f.why"))"
  # SHAPE: no section switch after the preamble, every $L reference defined here.
  SHAPE="$(awk '
    { sub(/\r$/, "") }
    $1 == ".ent" { body = 1 }
    body && /^[ \t]*\.(section|rdata|data|sdata|sbss|bss|rodata|lit4|lit8|text)([ \t,]|$)/ { print "section switch: " $0 }
    /^\$L[A-Za-z0-9_]*:/ { lab = $0; sub(/:.*/, "", lab); def[lab] = 1 }
    { s = $0; sub(/#.*/, "", s)
      while (match(s, /\$L[A-Za-z0-9_]*/)) { ref[substr(s, RSTART, RLENGTH)] = 1; s = substr(s, RSTART + RLENGTH) } }
    END { for (r in ref) if (!(r in def)) print "label defined outside the block: " r }' "$TMP/$f.blk")"
  [ -z "$SHAPE" ] || fatal "$f is outside the s136os arm's domain (its block needs data spliced from outside .ent..end):" "$SHAPE"
  # Rename the block's labels, carry the missing .extern lines, splice.
  awk -v fn="$f" '{ o = ""; s = $0
      while (match(s, /\$L[0-9]+/)) { o = o substr(s, 1, RSTART + RLENGTH - 1) "_s136_" fn; s = substr(s, RSTART + RLENGTH) }
      print o s }' "$TMP/$f.blk" > "$TMP/$f.blk2"
  awk '{ sub(/\r$/, "") } $1 == ".extern" { x = $0; gsub(/[ \t]+/, " ", x); sub(/^ /, "", x); print x }' "$OUT" | sort -u > "$TMP/$f.have"
  carry_externs "$OUT" "$TMP/$f.s" > "$TMP/$f.ext"
  # The size conflicts the name key resolved in the unit's favour (the lines a
  # whole-line key would have carried), listed in the build log by name.
  KEPT="$(awk '{ sub(/\r$/, "") }
    FNR == 1 { file++ }
    $1 != ".extern" { next }
    { x = $0; gsub(/[ \t]+/, " ", x); sub(/^ /, "", x); sym = $2; sub(/,.*/, "", sym) }
    file == 1 { have[sym] = 1; line[x] = 1; next }
    (sym in have) && !(x in line) && !seen[x]++ { printf "%s%s", (n++ ? " " : ""), x }' "$OUT" "$TMP/$f.s")"
  awk -v fn="$f" -v blk="$TMP/$f.blk2" -v ext="$TMP/$f.ext" '
    { k = $0; sub(/\r$/, "", k); split(k, F) }
    F[1] == "#S136OS_SLOT" && F[2] == fn {
      print "#S136OS_BEGIN " fn " (tools/ee/s136os_splice.sh: SN 2.95.3 v1.36 -fopt-stack)"
      while ((getline l < ext) > 0) print l
      while ((getline l < blk) > 0) print l
      print "#S136OS_END " fn
      n++; next }
    { print }
    END { if (n != 1) exit 1 }' "$OUT" > "$TMP/unit.s" || fatal "slot for $f not replaced exactly once"
  cp "$TMP/unit.s" "$OUT"
  echo "s136os_splice: $REGION/$UNIT: $f spliced ($(awk '{ sub(/\r$/, "") } NF && $1 !~ /^[.#$]/ && $1 !~ /:$/' "$TMP/$f.blk2" | wc -l | tr -d ' ') insn lines, $(wc -l < "$TMP/$f.ext" | tr -d ' ') .extern carried; not carried, the unit declares the name: ${KEPT:-none})"
done
LEFT="$(awk '{ sub(/\r$/, "") } $1 == "#S136OS_SLOT" { print $2 }' "$OUT")"
[ -z "$LEFT" ] || fatal "slot(s) still present after the splice:" $LEFT

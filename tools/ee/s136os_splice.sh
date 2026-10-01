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
#   - <src> not a `.c` file: no SN 1.36 cc1plus is held (FACT #8810 "Not
#     tested"), so a `.cpp` unit's member needs a decision, not a fallback;
#   - the s136 cc1 missing, or not sha256 0393bcd3... (FACT #8810);
#   - the s136os output has no single `.ent <fn>`..`.end <fn>` block;
#   - SHAPE: the block switches section (rodata/data/sdata literal, jump table)
#     or references a `$L` label it does not define (a `$LC` string/float
#     literal). Those live outside the block and are not spliced, so such a
#     function is out of this arm's domain until the rodata side is solved.
# The block's own `$L<n>` labels are renamed `$L<n>_s136_<fn>` (still `$L`, so
# local and absent from the symtab, as cc1's are) because both TUs number
# their labels from the same counter. `.extern` lines the s136os TU emits and
# <unit.s> lacks are carried in front of the block: gas decides gp-relativity
# from them (-G8 units), and the 2.9 TU never saw the body that needs them.
set -eu
REGION="$1"; UNIT="$2"; SRC="$3"; UNIT_S="$4"; GFLAG="$5"; CC1EXTRA="${6:-}"
SEL=tools/ee/s136os_functions.txt
WIBO=/usr/local/bin/wibo
G29=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
G136=tools/ee/cc/lib/gcc-lib/ee/2.95.3
CC1_136_SHA256=0393bcd31f91a6b9f0255db97f1cc99eba78ee8fc003e9a04dfabed1ae1d522e
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
  *.c) ;;
  *) fatal "$SRC is not a .c file; no SN 1.36 cc1plus is held, so its s136os rows cannot compile:" $ROWS ;;
esac
[ -f "$G136/cc1.exe" ] || fatal "$G136/cc1.exe missing (FACT #8810: provision the SN 2.95.3 v1.36 cc1)"
SHA="$(sha256sum "$G136/cc1.exe" | awk '{print $1}')"
[ "$SHA" = "$CC1_136_SHA256" ] || fatal "$G136/cc1.exe sha256 $SHA, not the s136 cc1 $CC1_136_SHA256"

TMP="${UNIT_S%.s}._s136"
rm -rf "$TMP"; mkdir -p "$TMP"
OUT="$UNIT_S"
for f in $ROWS; do
  "$WIBO" "$G29/cpp.exe" $CPPDEF $INC "-DS136OS_$f" "$SRC" "$TMP/$f.i" || fatal "cpp failed for $f"
  "$WIBO" "$G136/cc1.exe" -quiet -O2 $GFLAG $CC1EXTRA -fopt-stack "$TMP/$f.i" -o "$TMP/$f.s" || fatal "s136 cc1 failed for $f"
  # The block: the .align/.p2align/.globl/.text/.section .text directives cc1
  # prints right before `.ent <fn>`, through `.end <fn>`.
  awk -v fn="$f" -v out="$TMP/$f.blk" '
    function isdir(l) { return l ~ /^[ \t]*(\.align|\.p2align|\.text|\.section[ \t]+\.text)([ \t]|$)/ || l ~ ("^[ \t]*\\.globl[ \t]+" fn "[ \t]*$") }
    { L[NR] = $0; sub(/\r$/, ""); K[NR] = $0 }
    $1 == ".ent" && $2 == fn { ne++; s = NR }
    $1 == ".end" && $2 == fn { nd++; e = NR }
    END {
      if (ne != 1 || nd != 1 || e < s) { print "BLOCK " ne " .ent / " nd " .end"; exit }
      b = s; while (b > 1 && isdir(K[b - 1])) b--
      for (i = b; i <= e; i++) print L[i] > out
    }' "$TMP/$f.s" > "$TMP/$f.why"
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
  awk '{ sub(/\r$/, "") } $1 == ".extern" { x = $0; gsub(/[ \t]+/, " ", x); sub(/^ /, "", x); print x }' "$TMP/$f.s" | sort -u \
    | comm -23 - "$TMP/$f.have" | sed 's/^/\t/' > "$TMP/$f.ext"
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
  echo "s136os_splice: $REGION/$UNIT: $f spliced ($(awk '{ sub(/\r$/, "") } NF && $1 !~ /^[.#$]/ && $1 !~ /:$/' "$TMP/$f.blk2" | wc -l | tr -d ' ') insn lines, $(wc -l < "$TMP/$f.ext" | tr -d ' ') .extern carried)"
done
LEFT="$(awk '{ sub(/\r$/, "") } $1 == "#S136OS_SLOT" { print $2 }' "$OUT")"
[ -z "$LEFT" ] || fatal "slot(s) still present after the splice:" $LEFT

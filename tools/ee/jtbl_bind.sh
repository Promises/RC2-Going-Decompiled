#!/bin/sh
# jtbl_bind.sh — bind a compiled C switch's jump table to the ROM's own table
# (task #2247; the wall measured by FACT #12092 on the s136os arm and FACT
# #12095 on the plain 2.9 arm).
#
#   sh tools/ee/jtbl_bind.sh <region> <unit> <unit.s> <GFLAG>
#   e.g. sh tools/ee/jtbl_bind.sh usa text/188858 $BUILD/.../188858._u.s -G8
#   sh tools/ee/jtbl_bind.sh --selftest   container (reads the retail ELF); rc 0 PASS
#
# ONE helper, two callers: build.sh (the image) and objdiff_build.sh (the unit
# report's sdk29 base), each after s136os_splice.sh and before asm_unit.sh, so
# a table from the 2.9 compile and one from a spliced s136os block are bound
# the same way. Runs INSIDE the ee-build container at the repo root, POSIX sh +
# awk + binutils. <unit.s> is replaced only on success; a unit with no table to
# bind is left untouched.
#
# WHY. cc1 (2.9 and SN 1.36 alike) lowers a dense `switch` to
#     lui $2,%hi($L63) / addiu $2,$2,%lo($L63) / addu / lw / j $4
#     .rdata / .align 4 / $L63: / .word $L62 / .word $L57 ... / .text
# with the table emitted INSIDE .ent..end. The ROM's table for that function
# already exists as data, a `jtbl_<addr>[_text]` dlabel in a data-asm unit
# (asm/<region>/data/**), which the image links from there. The compiled copy
# has nowhere to go: the link script places no compiled .rodata except
# text/198FA0's, and its final /DISCARD/ drops it (FACT #12095: the USA link
# fails), and s136os_splice.sh refused such a block outright (FACT #12092).
#
# WHAT. For every jump table inside a function `F` of <unit.s>:
#  (a) RECOGNISE it: a `.rdata` (or `.section .rodata`) switch inside .ent F..
#      .end F holding exactly one `$L` label and only `.word $L...` lines, then
#      `.text`. Any other section switch is not a table and is not touched.
#  (b) PAIR it with F's ROM table: the n-th table of F, in emission order, with
#      the n-th distinct `jtbl_` symbol of F's splat .s
#      (asm/<region>/nonmatchings/<unit>/<F>.s), in order of appearance. Their
#      counts must be equal.
#  (c) VERIFY it word for word: the unit is assembled (asm_unit.sh, exactly as
#      the caller assembles it next) with a probe label on each table; every
#      entry must be an R_MIPS_32 against .text whose target, relative to F's
#      start, equals the ROM table's word at that index minus F's ROM address.
#      The ROM words are read from the RETAIL ELF (extracted/<region>/<exe>),
#      never from a .s comment; the table's extent is the dlabel's words in the
#      data .s, trailing zero words (alignment) excluded, and must have exactly
#      as many entries as the compiled table. Every entry, built and ROM, must
#      land inside F.
#  (d) BIND it: drop the compiled `.rdata` block and rewrite the table's
#      `%hi($L..)` / `%lo($L..)` operands to the ROM symbol, so the object
#      carries no copy and its relocations name the table the image links.
# FATAL (rc 3, <unit.s> untouched) — each lists every offender:
#   - a table referenced other than through %hi()/%lo() (the rewrite cannot
#     cover it);
#   - F's compiled table count differs from the distinct jtbl_ symbols of its
#     ROM .s;
#   - a ROM table's extent cannot be read (no data .s line, words not in the
#     retail ELF), or its entry count differs from the compiled table's;
#   - an entry differs, is not an R_MIPS_32 against .text, or lands outside F;
#   - the unit did not assemble for the check.
# KEPT (printed, not fatal; the table stays in the unit's .rodata, the
# behaviour before this helper): F has no ROM .s (an EU function whose .s was
# deleted on match), or the paired jtbl_ symbol has no dlabel in any data .s —
# the ROM table is then supplied by the unit's own compiled .rodata, which the
# link script places (text/198FA0's jtbl_0026CA70, task #947's route). Binding
# that table would leave the image with no table at all.
# Not covered: objdiff_build.sh's engine96 (2.96, MATCH_) arm is not passed
# through this helper.
set -eu

# hex2dec <hex>: awk-side helper text, shared by every awk below.
AWK_HEX='function hex(x,   i, c, v) { v = 0; x = tolower(x); sub(/^0x/, "", x)
  for (i = 1; i <= length(x); i++) { c = index("0123456789abcdef", substr(x, i, 1)); if (!c) return -1; v = v * 16 + c - 1 }
  return v }'

# find_tables <unit.s>: one line per recognised table,
#   T <F> <label> <entries> <first line> <last line>
# (the line range is the `.rdata` line through its closing `.text`), followed by
#   E <label> <index> <target label>
# for each entry, and one line per table label
#   R <label> <%hi refs> <%lo refs> <other refs>
# counting references outside the table itself.
find_tables() {
  awk '
    function reset() { st = 0; lab = ""; n = 0 }
    { sub(/\r$/, ""); L[NR] = $0 }
    $1 == ".ent" { fn = $2; inb = 1; reset(); next }
    $1 == ".end" { inb = 0; reset(); next }
    !inb { next }
    st == 0 && ($0 ~ /^[ \t]*\.rdata[ \t]*$/ || $0 ~ /^[ \t]*\.section[ \t]+\.rodata[ \t]*$/) { st = 1; s = NR; next }
    st == 1 && $0 ~ /^[ \t]*$/ { next }
    st == 1 && $0 ~ /^[ \t]*\.(p2)?align[ \t]+[0-9]+[ \t]*$/ { next }
    st == 1 && $0 ~ /^\$L[A-Za-z0-9_]+:[ \t]*$/ { lab = $0; sub(/:.*/, "", lab); st = 2; next }
    st == 2 && $1 == ".word" && NF == 2 && $2 ~ /^\$L[A-Za-z0-9_]+$/ { ent[lab, n++] = $2; next }
    st == 2 && n > 0 && ($0 ~ /^[ \t]*\.text[ \t]*$/ || $0 ~ /^[ \t]*\.section[ \t]+\.text[ \t]*$/) {
      nt++; tf[nt] = fn; tl[nt] = lab; tn[nt] = n; ts[nt] = s; te[nt] = NR; for (i = s; i <= NR; i++) intab[i] = 1
      st = 0; next }
    st { st = 0 }
    END {
      for (t = 1; t <= nt; t++) {
        print "T", tf[t], tl[t], tn[t], ts[t], te[t]
        for (i = 0; i < tn[t]; i++) print "E", tl[t], i, ent[tl[t], i]
        islab[tl[t]] = t; hi[t] = lo[t] = ot[t] = 0
      }
      for (r = 1; r <= NR; r++) {
        if (r in intab) continue
        s = L[r]; sub(/#.*/, "", s)
        while (match(s, /\$L[A-Za-z0-9_]+/)) {
          tok = substr(s, RSTART, RLENGTH); pre = substr(s, 1, RSTART - 1); post = substr(s, RSTART + RLENGTH)
          if (tok in islab) {
            t = islab[tok]
            if (pre ~ /%hi\($/ && post ~ /^\)/) hi[t]++
            else if (pre ~ /%lo\($/ && post ~ /^\)/) lo[t]++
            else if (!(substr(post, 1, 1) == ":" && pre ~ /^[ \t]*$/)) ot[t]++
          }
          s = post
        }
      }
      for (t = 1; t <= nt; t++) print "R", tl[t], hi[t], lo[t], ot[t]
    }' "$1"
}

# rom_jtbls <fn.s>: the distinct jtbl_ symbols of a ROM function, in order of
# first appearance.
rom_jtbls() {
  awk '{ s = $0; while (match(s, /jtbl_[0-9A-Fa-f]+(_text)?/)) { t = substr(s, RSTART, RLENGTH); if (!(t in seen)) { seen[t] = 1; print t }; s = substr(s, RSTART + RLENGTH) } }' "$1"
}

# rom_fn <fn.s> <fn>: `<ROM vaddr hex> <bytes>` of the ROM function, from its
# glabel's first word and its nonmatching header; or ORACLE <why>.
rom_fn() {
  awk -v fn="$2" "$AWK_HEX"'
    { sub(/\r$/, "") }
    $1 == "nonmatching" && $2 == fn "," { h = hex($3) }
    $1 == "glabel" && $2 == fn { g = 1; next }
    g && $1 == "/*" && length($3) == 8 { a = $3; g = 0 }
    END { if (a == "" || h <= 0) { print "ORACLE no glabel word or nonmatching size for " fn " in " FILENAME; exit } printf "%s %d\n", a, h }' "$1"
}

# rom_extent <data .s> <jtbl>: the vaddrs (hex) of the words under `dlabel
# <jtbl>`, up to the next dlabel/enddlabel/nonmatching/glabel line.
rom_extent() {
  awk -v j="$2" '
    { sub(/\r$/, "") }
    $1 == "dlabel" && $2 == j { on = 1; next }
    on && ($1 == "dlabel" || $1 == "enddlabel" || $1 == "nonmatching" || $1 == "glabel") { exit }
    on && $1 == "/*" && length($3) == 8 && $3 ~ /^[0-9A-Fa-f]+$/ { print $3 }' "$1"
}

# objdump_words <objdump -s output>: `<vaddr dec> <word dec>` per 32-bit LE word.
objdump_words() {
  awk "$AWK_HEX"'
    $1 ~ /^[0-9a-f]+$/ && NF >= 2 { a = hex($1)
      for (i = 2; i <= 5 && i <= NF; i++) { w = $i; if (w !~ /^[0-9a-f]+$/) break
        for (k = 0; k + 8 <= length(w); k += 8) { b = substr(w, k + 1, 8); printf "%d %d\n", a, hex(substr(b, 7, 2) substr(b, 5, 2) substr(b, 3, 2) substr(b, 1, 2)); a += 4 } } }' "$1"
}

# table_compare <fn> <label> <jtbl> <built fn bytes> <ROM fn bytes> <built> <rom>:
# print one line per reason the table cannot be bound; empty = bind. <built>
# and <rom> hold one entry per line, `<index> <target relative to F, bytes>`
# (<built> entries that are not R_MIPS_32 against .text carry `RELOC <what>`
# in place of the offset).
table_compare() {
  awk -v fn="$1" -v lab="$2" -v j="$3" -v bsz="$4" -v rsz="$5" '
    FNR == 1 { file++ }
    file == 1 { nb++; b[$1] = substr($0, length($1) + 2); next }
    { nr++; r[$1] = substr($0, length($1) + 2) }
    END {
      if (nb != nr) printf "%s: %s has %d entries, %s has %d (ROM words to the next label, trailing zero words excluded)\n", fn, lab, nb, j, nr
      for (i = 0; i < nb; i++) {
        if (b[i] ~ /^RELOC/) { printf "%s: %s entry %d is not an R_MIPS_32 against .text (%s)\n", fn, lab, i, b[i]; continue }
        if (b[i] + 0 < 0 || b[i] + 0 >= bsz + 0) printf "%s: %s entry %d targets %s+0x%x, outside its 0x%x bytes\n", fn, lab, i, fn, b[i], bsz
        if (!(i in r)) continue
        if (r[i] + 0 < 0 || r[i] + 0 >= rsz + 0) printf "%s: %s word %d targets %s+0x%x, outside the ROM function'"'"'s 0x%x bytes\n", fn, j, i, fn, r[i], rsz
        else if (b[i] != r[i]) printf "%s: %s entry %d targets %s+0x%x, %s word %d targets %s+0x%x\n", fn, lab, i, fn, b[i], j, i, fn, r[i]
      }
    }' "$6" "$7"
}

# rewrite <unit.s> <binds>: <unit.s> without each bound table's line range and
# with its %hi/%lo operands naming the ROM symbol. <binds> lines are
# `<label> <jtbl> <first line> <last line>`.
rewrite() {
  awk 'FNR == NR { j[$1] = $2; for (i = $3; i <= $4; i++) drop[i] = 1; next }
    FNR in drop { next }
    { o = ""; s = $0
      while (match(s, /%(hi|lo)\(\$L[A-Za-z0-9_]+\)/)) {
        t = substr(s, RSTART + 4, RLENGTH - 5)
        o = o substr(s, 1, RSTART + 3) ((t in j) ? j[t] : t) ")"; s = substr(s, RSTART + RLENGTH) }
      print o s }' "$2" "$1"
}

# probe <unit.s> <binds>: <unit.s> with a `jtbl_bind_probe_<label>:` label after
# each table label, so the assembled object names where each table starts.
probe() {
  awk 'FNR == NR { want[$1] = 1; next }
    { print; k = $0; sub(/\r$/, "", k); sub(/:[ \t]*$/, "", k) }
    (k in want) && $0 ~ /^\$L/ { p = k; sub(/^\$/, "", p); print "jtbl_bind_probe_" p ":" }' "$2" "$1"
}

if [ "${1:-}" = "--selftest" ]; then
  T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
  rc=0; n=0
  pass() { n=$((n + 1)); echo "  OK   arm $1"; }
  flunk() { n=$((n + 1)); echo "  FAIL arm $1"; rc=1; }
  # A unit with one real table (3 entries, referenced %hi/%lo), a .rdata
  # string literal (not a table) and a second function that names nothing.
  printf '\t.ent\tf\nf:\n\tlui\t$2,%%hi($L9) # high\n\taddiu\t$2,$2,%%lo($L9) # low\n\tj\t$4\n\t.p2align 2\n\t.rdata\n\t.align\t4\n$L9:\n\t.word\t$L3\n\t.word\t$L4\n\t.word\t$L3\n\t.text\n\t.p2align 2\n$L3:\n\tli\t$2,1\n$L4:\n\tj\t$31\n\t.end\tf\n\t.ent\tg\ng:\n\t.rdata\n\t.align\t2\n$LC0:\n\t.ascii\t"x\\000"\n\t.text\n\tla\t$2,$LC0\n\t.end\tg\n' > "$T/u.s"
  find_tables "$T/u.s" > "$T/ft"
  # 1: the table is found with its range and entries; the $LC literal is not.
  if [ "$(/usr/bin/grep '^T ' "$T/ft")" = "T f \$L9 3 7 13" ] && [ "$(/usr/bin/grep -c '^E ' "$T/ft")" = 3 ] && [ "$(/usr/bin/grep '^R ' "$T/ft")" = "R \$L9 1 1 0" ]; then pass "1: one table f \$L9, 3 entries, lines 7-13, refs hi 1 lo 1 other 0; the .rdata string literal in g is not a table"
  else flunk "1: find_tables gave:"; sed 's/^/        /' "$T/ft"; fi
  # 2: a table also reached through `la` counts an OTHER reference.
  sed 's/^\taddiu\t\$2,\$2,%lo(\$L9) # low$/\tla\t$3,$L9/' "$T/u.s" > "$T/u2.s"
  if [ "$(find_tables "$T/u2.s" | /usr/bin/grep '^R ')" = "R \$L9 1 0 1" ]; then pass "2: \`la \$3,\$L9\` counts as an other reference (the bind refuses it)"
  else flunk "2: refs: $(find_tables "$T/u2.s" | /usr/bin/grep '^R ')"; fi
  # 3: rewrite drops lines 7-13 and rebinds only that label's operands.
  printf '$L9 jtbl_00200100_text 7 13\n' > "$T/binds"
  rewrite "$T/u.s" "$T/binds" > "$T/rw"
  if [ "$(/usr/bin/grep -c 'jtbl_00200100_text' "$T/rw")" = 2 ] && ! /usr/bin/grep -q '\$L9\|\.word' "$T/rw" && /usr/bin/grep -q '^\$LC0:' "$T/rw" && [ "$(wc -l < "$T/rw" | tr -d ' ')" = $(( $(wc -l < "$T/u.s" | tr -d ' ') - 7 )) ]; then pass "3: the table's 7 lines dropped, %hi/%lo rebound to jtbl_00200100_text, g's literal untouched"
  else flunk "3: rewrite gave:"; sed 's/^/        /' "$T/rw"; fi
  # 4-7: table_compare. 4 equal -> bind; 5 one entry seeded -> refused naming
  # it; 6 the ROM one entry longer -> refused on the count; 7 an entry that is
  # not a .text reloc -> refused.
  printf '0 16\n1 20\n2 16\n' > "$T/b"; printf '0 16\n1 20\n2 16\n' > "$T/r"
  v="$(table_compare f '$L9' J 24 24 "$T/b" "$T/r")"; if [ -z "$v" ]; then pass "4: equal tables -> bound"; else flunk "4: '$v'"; fi
  printf '0 16\n1 16\n2 16\n' > "$T/r"
  v="$(table_compare f '$L9' J 24 24 "$T/b" "$T/r")"; case "$v" in 'f: $L9 entry 1 targets f+0x14, J word 1 targets f+0x10') pass "5: seeded entry -> refused: $v" ;; *) flunk "5: '$v'" ;; esac
  printf '0 16\n1 20\n2 16\n3 20\n' > "$T/r"
  v="$(table_compare f '$L9' J 24 24 "$T/b" "$T/r")"; case "$v" in 'f: $L9 has 3 entries, J has 4'*) pass "6: ROM table one longer -> refused: $v" ;; *) flunk "6: '$v'" ;; esac
  printf '0 16\n1 RELOC R_MIPS_32/.data\n2 16\n' > "$T/b"; printf '0 16\n1 20\n2 16\n' > "$T/r"
  v="$(table_compare f '$L9' J 24 24 "$T/b" "$T/r")"; case "$v" in 'f: $L9 entry 1 is not an R_MIPS_32 against .text'*) pass "7: a non-.text entry -> refused: $v" ;; *) flunk "7: '$v'" ;; esac
  # 8: an entry outside the function -> refused.
  printf '0 16\n1 24\n2 16\n' > "$T/b"; printf '0 16\n1 24\n2 16\n' > "$T/r"
  v="$(table_compare f '$L9' J 24 24 "$T/b" "$T/r")"; case "$v" in *'entry 1 targets f+0x18, outside its 0x18 bytes'*) pass "8: an entry past the function's end -> refused: $v" ;; *) flunk "8: '$v'" ;; esac
  # 9-10: the REAL ROM table. UpgradeWeaponToMax's jtbl_0026C6D0_text read from
  # the retail ELF through this helper's own extent + word readers, against
  # the entries the ROM's own code implies (word = 0x289244 at 0,2,5,6,41,
  # else 0x289248, FACT #12092's case set {0xC,0xE,0x11,0x12,0x35} - 0xC):
  # equal -> bound (9); the same with entry 5 seeded to the other target ->
  # refused naming it (10). Skipped, as a FAIL, when the tree lacks the data.
  D=going-decompiled/asm/usa/data/data/138B80.data.s; E=extracted/usa/SCUS_972.68; R=going-decompiled/asm/usa/nonmatchings/text/188858/UpgradeWeaponToMax.s
  if [ -f "$D" ] && [ -f "$E" ] && [ -f "$R" ] && command -v mips-linux-gnu-objdump > /dev/null; then
    rom_extent "$D" jtbl_0026C6D0_text > "$T/ext"
    first=$(head -1 "$T/ext"); last=$(tail -1 "$T/ext")
    mips-linux-gnu-objdump -s --start-address=0x$first --stop-address=$(printf '0x%x' $((0x$last + 4))) "$E" > "$T/od"
    set -- $(rom_fn "$R" UpgradeWeaponToMax); fa=$((0x$1)); fs=$2
    objdump_words "$T/od" | awk -v fa="$fa" '{ w[NR - 1] = $2; m = NR } END { while (m > 0 && w[m - 1] == 0) m--; for (i = 0; i < m; i++) print i, w[i] - fa }' > "$T/r"
    awk 'BEGIN { for (i = 0; i < 42; i++) print i, (i == 0 || i == 2 || i == 5 || i == 6 || i == 41) ? 108 : 112 }' > "$T/b"
    v="$(table_compare UpgradeWeaponToMax '$L63' jtbl_0026C6D0_text "$fs" "$fs" "$T/b" "$T/r")"
    if [ -z "$v" ] && [ "$(wc -l < "$T/r" | tr -d ' ')" = 42 ] && [ "$(wc -l < "$T/ext" | tr -d ' ')" = 44 ]; then pass "9: real jtbl_0026C6D0_text from $E: 44 words, 2 trailing zeros excluded, 42 entries = the case set -> bound"
    else flunk "9: real table: $(wc -l < "$T/ext" | tr -d ' ') extent words, $(wc -l < "$T/r" | tr -d ' ') entries, compare '$v'"; fi
    awk '$1 == 5 { $2 = 112 } { print }' "$T/b" > "$T/b5"
    v="$(table_compare UpgradeWeaponToMax '$L63' jtbl_0026C6D0_text "$fs" "$fs" "$T/b5" "$T/r")"
    case "$v" in 'UpgradeWeaponToMax: $L63 entry 5 targets UpgradeWeaponToMax+0x70, jtbl_0026C6D0_text word 5 targets UpgradeWeaponToMax+0x6c') pass "10: the real table with entry 5 seeded -> refused: $v" ;; *) flunk "10: '$v'" ;; esac
  else flunk "9-10: cannot read the real table ($D, $E, $R, mips-linux-gnu-objdump): run in the ee-build container on a provisioned tree"; fi
  echo "jtbl_bind --selftest: $n arms"
  [ "$rc" = 0 ] && echo "#### jtbl_bind --selftest: PASS" || echo "#### jtbl_bind --selftest: FAIL"
  exit "$rc"
fi

REGION="$1"; UNIT="$2"; UNIT_S="$3"; GFLAG="$4"
case "$REGION" in usa) EXE=SCUS_972.68 ;; eu) EXE=SCES_516.07 ;; *) echo "jtbl_bind: FATAL — unknown region $REGION" >&2; exit 3 ;; esac
fatal() { echo "jtbl_bind: FATAL [$REGION/$UNIT] — $1 Nothing was written to $UNIT_S." >&2; shift; for x in "$@"; do echo "    $x" >&2; done; exit 3; }
[ -f "$UNIT_S" ] || fatal "unit .s $UNIT_S missing."

TMP="${UNIT_S%.s}._jtbl"
rm -rf "$TMP"; mkdir -p "$TMP"
find_tables "$UNIT_S" > "$TMP/tables"
/usr/bin/grep -q '^T ' "$TMP/tables" || { rm -rf "$TMP"; exit 0; }

ASM=going-decompiled/asm/$REGION
ELF=extracted/$REGION/$EXE
: > "$TMP/binds"; BAD=""
nl='
'
# (b) pair every function's tables with its ROM .s, in order.
for fn in $(awk '$1 == "T" { print $2 }' "$TMP/tables" | awk '!s[$0]++'); do
  p="$ASM/nonmatchings/$UNIT/$fn.s"
  if [ ! -f "$p" ]; then
    echo "jtbl_bind: $REGION/$UNIT: $fn kept: no ROM .s at $p, its table(s) stay in the unit's .rodata"; continue
  fi
  awk -v f="$fn" '$1 == "T" && $2 == f { print $3, $5, $6, $4 }' "$TMP/tables" > "$TMP/$fn.t"
  rom_jtbls "$p" > "$TMP/$fn.j"
  nt=$(wc -l < "$TMP/$fn.t" | tr -d ' '); nj=$(wc -l < "$TMP/$fn.j" | tr -d ' ')
  if [ "$nt" != "$nj" ]; then BAD="$BAD$nl$fn: $nt compiled jump table(s), $nj distinct jtbl_ symbol(s) in $p ($(tr '\n' ' ' < "$TMP/$fn.j"))"; continue; fi
  paste -d' ' "$TMP/$fn.t" "$TMP/$fn.j" | while read -r lab s e cnt j; do
    set -- $(awk -v l="$lab" '$1 == "R" && $2 == l { print $3, $4, $5 }' "$TMP/tables")
    if [ "$3" != 0 ] || [ "$1" = 0 ] || [ "$2" = 0 ]; then echo "BAD $fn: $lab is referenced $1 x %hi, $2 x %lo and $3 x otherwise; only %hi()/%lo() operands can be rebound to $j"; continue; fi
    dfile=$(/usr/bin/grep -rlE "^[[:space:]]*dlabel[[:space:]]+$j([[:space:]]|\$)" "$ASM/data" 2>/dev/null | head -1 || true)
    if [ -z "$dfile" ]; then echo "KEPT $fn $lab $j"; continue; fi
    echo "BIND $fn $lab $j $s $e $cnt $dfile"
  done >> "$TMP/binds.all"
done
[ -f "$TMP/binds.all" ] || : > "$TMP/binds.all"
BAD="$BAD$nl$(sed -n 's/^BAD //p' "$TMP/binds.all")"
BAD="$(printf '%s' "$BAD" | sed '/^$/d')"
[ -z "$BAD" ] || fatal "a jump table cannot be paired with its ROM table (task #2247):" "$(printf '%s' "$BAD")"
awk -v r="$REGION/$UNIT" -v d="$ASM/data" '$1 == "KEPT" { print "jtbl_bind: " r ": " $2 " " $3 " kept: " $4 " has no dlabel in " d ", so the unit'"'"'s own .rodata is that table" }' "$TMP/binds.all"
awk '$1 == "BIND" { print $3, $4, $5, $6 }' "$TMP/binds.all" > "$TMP/binds"
awk '$1 == "BIND"' "$TMP/binds.all" > "$TMP/binds.all.bind"
[ -s "$TMP/binds" ] || { rm -rf "$TMP"; exit 0; }

# (c) assemble the unit as the caller will, with a probe on each table.
[ -f "$ELF" ] || fatal "the retail ELF $ELF is missing, so no ROM table can be read."
probe "$UNIT_S" "$TMP/binds" > "$TMP/vfy.s"
case "$TMP" in /*) ATMP="$TMP" ;; *) ATMP="$(pwd)/$TMP" ;; esac
if ! ASM_UNIT_S_MD5= sh tools/ee/asm_unit.sh "$REGION" "$ATMP/vfy.s" "$ATMP/vfy.o" "$GFLAG" > "$TMP/vfy.asm.log" 2>&1 || [ ! -s "$TMP/vfy.o" ]; then
  echo "jtbl_bind: FATAL [$REGION/$UNIT] — the unit did not assemble for the table check; asm_unit.sh said:" >&2
  sed 's/^/    | /' "$TMP/vfy.asm.log" >&2; exit 3
fi
rm -rf "$TMP/.asmfix-$REGION-vfy"
mips-linux-gnu-nm -S --defined-only "$TMP/vfy.o" > "$TMP/vfy.nm"
mips-linux-gnu-objdump -s -j .rodata "$TMP/vfy.o" > "$TMP/vfy.rod"
mips-linux-gnu-objdump -r -j .rodata "$TMP/vfy.o" > "$TMP/vfy.rel"
objdump_words "$TMP/vfy.rod" > "$TMP/vfy.words"
BADV=""
while read -r _ fn lab j s e cnt dfile; do
  p="$ASM/nonmatchings/$UNIT/$fn.s"
  pr="jtbl_bind_probe_${lab#\$}"
  set -- $(awk -v f="$fn" -v p="$pr" "$AWK_HEX"'
    NF == 4 && $3 ~ /^[Tt]$/ && $4 == f { fv = hex($1); fs = hex($2); nf++ }
    NF == 3 && $3 == p { pv = hex($1); np++ }
    END { printf "%s %s %s\n", (nf == 1 ? fv : "-"), (nf == 1 ? fs : "-"), (np == 1 ? pv : "-") }' "$TMP/vfy.nm")
  fv=$1; fs=$2; pv=$3
  if [ "$fv" = - ] || [ "$pv" = - ]; then BADV="$BADV$nl$fn: no single sized symbol $fn or probe $pr in the assembled unit"; continue; fi
  # built entries: word (REL addend) at probe+4i, its reloc must be R_MIPS_32 .text
  awk -v pv="$pv" -v cnt="$cnt" -v fv="$fv" "$AWK_HEX"'
    FNR == 1 { file++ }
    file == 1 { w[$1] = $2; next }
    $1 ~ /^[0-9a-f]+$/ && NF >= 3 { rt[hex($1)] = $2 "/" $3 }
    END { for (i = 0; i < cnt; i++) { o = pv + 4 * i
      if (rt[o] != "R_MIPS_32/.text") print i, "RELOC " (o in rt ? rt[o] : "none")
      else print i, w[o] - fv } }' "$TMP/vfy.words" "$TMP/vfy.rel" > "$TMP/$fn.$pr.b"
  # ROM entries: the dlabel's words, read from the retail ELF
  rom_extent "$dfile" "$j" > "$TMP/$fn.$pr.ext"
  if [ ! -s "$TMP/$fn.$pr.ext" ]; then BADV="$BADV$nl$fn: no words under dlabel $j in $dfile"; continue; fi
  rf="$(rom_fn "$p" "$fn")"
  case "$rf" in ORACLE*) BADV="$BADV$nl$fn: $rf"; continue ;; esac
  set -- $rf; fa=$((0x$1)); rs=$2
  first=$(head -1 "$TMP/$fn.$pr.ext"); last=$(tail -1 "$TMP/$fn.$pr.ext")
  mips-linux-gnu-objdump -s --start-address=0x$first --stop-address=$(printf '0x%x' $((0x$last + 4))) "$ELF" > "$TMP/$fn.$pr.od"
  objdump_words "$TMP/$fn.$pr.od" | awk -v fa="$fa" -v m0="$(wc -l < "$TMP/$fn.$pr.ext" | tr -d ' ')" '{ w[NR - 1] = $2; m = NR } END { if (m != m0) { print "COUNT", m, m0; exit } while (m > 0 && w[m - 1] == 0) m--; for (i = 0; i < m; i++) print i, w[i] - fa }' > "$TMP/$fn.$pr.r"
  if /usr/bin/grep -q '^COUNT' "$TMP/$fn.$pr.r"; then BADV="$BADV$nl$fn: $ELF holds $(awk '{print $2}' "$TMP/$fn.$pr.r") of $j's $(awk '{print $3}' "$TMP/$fn.$pr.r") words (0x$first..0x$last)"; continue; fi
  v="$(table_compare "$fn" "$lab" "$j" "$fs" "$rs" "$TMP/$fn.$pr.b" "$TMP/$fn.$pr.r")"
  [ -z "$v" ] || BADV="$BADV$nl$v"
done < "$TMP/binds.all.bind"
BADV="$(printf '%s' "$BADV" | sed '/^$/d')"
[ -z "$BADV" ] || fatal "a compiled jump table differs from the ROM table it would be bound to (task #2247):" "$(printf '%s' "$BADV")"

# (d) bind: drop the copies, rebind %hi/%lo, replace <unit.s>.
rewrite "$UNIT_S" "$TMP/binds" > "$TMP/bound.s"
awk -v r="$REGION/$UNIT" -v e="$ELF" '{ printf "jtbl_bind: %s: %s %s -> %s (%d entries = ROM words read from %s; .rdata copy dropped)\n", r, $2, $3, $4, $7, e }' "$TMP/binds.all.bind"
cp "$TMP/bound.s" "$UNIT_S"

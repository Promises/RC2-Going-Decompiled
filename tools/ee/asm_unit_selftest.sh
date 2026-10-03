#!/bin/sh
# asm_unit_selftest.sh [asm_unit.sh] - seeded controls for tools/ee/asm_unit.sh's
# refusals and for the rules they guard (task #1124). Runs INSIDE the ee-build
# container:
#
#   docker --context colima-ee-x86 run --rm -v "$PWD":/work -w /work ee-build \
#       sh tools/ee/asm_unit_selftest.sh
#
# Every arm has a stated expected observable, and the refusal arms are paired
# with arms that must NOT refuse. A refusal nobody has seen fire is the same as
# no refusal, and one that fires on good input breaks what already matches.
#   ADJ  (RULING #8549 rev 3, FACT #8623) a listed `dli` directly before a
#        reorder-mode branch is refused (rc 2, `asm_unit.sh: FAIL:`, no object)
#        at -G0 and -G8, by the pass's own message: `No object written.` and no
#        generic `REFUSED:` or `dli pass, condition` line (FACT #8689; at
#        f6c2bae9e the exit code was right and that message was not, FACT
#        #8670). The same dli before a non-branch, before a branch in
#        noreorder, before a label, and the same literal in a function that is
#        not listed all assemble (rc 0), and the listed ones carry Ps2EeAs's
#        words, the unlisted one GNU's.
#   TAIL (FACT #8616) at -G8, a bare-symbol slot memop with a trailing comment
#        or space is refused; the same seed without the tail is live (1
#        WARNING at -G8, 0 at -G0: the matcher sees it); a trailing comment on
#        a non-memop line is still accepted.
#   LAY  (#1103) the C2-A layout refusals still fire (8 dead seeds) and the
#        healthy seeds (directives-only, CRLF) are still accepted.
#   EMPTY (FACT #8640, task #1147) the dli pass is refused (rc 2, FAIL naming
#        its condition, no object) at -G0 and -G8 when the allowlist has no
#        valid row (0 bytes, comments only, every row malformed), when the pass
#        exits non-zero, prints nothing, or prints fewer lines than it read, and
#        for a relative <unit.s> path. Each runs on a copy of $AU's tools/ee
#        with the one file changed, so the real allowlist is never touched.
#   STATIC (FACT #8652) at -G8, cc1's own `name.N` function-static memops,
#        one in a `j $31` slot, assemble with no WARNING and the slot store as
#        a 1-insn %gp_rel `sw`; with a trailing comment one is still refused.
#   LIKELY (FACT #8653) a listed `dli` directly before each of the 14 likely
#        branch forms assembles, in the dli's order (GNU as does not swap into
#        an annulled slot). SWAP: before each of the 19 measured swapping forms
#        (jal, jalr, jr, bc1f among them) it is still refused, by the same
#        text test as ADJ.
#   NORE (FACT #8672, FACT #8698, task #1170) a listed `dli` emitted under
#        `.set noreorder`. Under cc1's `.set nomacro` slot bracket it is
#        refused by its own nomacro message, never the swap message: FACT
#        #8672's seed (then `jal bar`) and its control (an addu before it).
#        Without nomacro it assembles to the words Ps2EeAs.exe 1.9.25.758 gives
#        for the same input (measured for #1170): a `jal` slot then a reorder
#        `jal`, `bnel` or addu, and a noreorder block that is not a slot.
#   SPELL the memop mnemonics are spelled once in $AU (one alternation with two
#        or more of them on a code line), and each of the ten cc1 memops is
#        seen by BOTH the layout scan and the slot rule: in a bnel slot at -G8,
#        bare it gives 1 WARNING (MEMOP_RE), with a trailing comment it is
#        refused (the scan's `mem`). A second copy that diverges from MEMOP_MN
#        fails the content arms; one that is identical fails the count (FACT
#        #8616, FACT #8689).
#   DLINE (task #1205) every assembled unit prints exactly one
#        `asm_unit.sh: dli: N transforms (M allowlist rows for <region>)`, the
#        spelling GATE-F3 (#1158) reads, compared whole-line: N is 0, 1 and 2
#        for seeds with that many listed sites, M is counted here from the
#        allowlist. EU prints 0 of 0 and keeps GNU's words. A refused unit
#        prints no such line.
#   ADJ2 (task #1205) a refusal at the SECOND site of func_0027C020's row
#        (adjacency, and nomacro) names `site 2` and its pass input line; the
#        first-site seed names `site 1`. The row's ROM address alone named the
#        first site for both.
#   SYNC (task #1205, FACT #8713) an allowlist copy missing its last row, so
#        well-formed and shorter, is refused as condition (m) when the host md5
#        of the whole file is passed, and assembles with one row fewer when it
#        is not: the row count cannot see a boundary truncation. The real
#        allowlist with its own md5 assembles with no mount_sync line.
#   MTC1 (task #1352, FACT #8063) at -G8, `mtc1 $fN; <held noreorder branch>;
#        slot reads $fN` assembles with no nop anywhere from the mtc1 to the
#        slot (a `jal` and a `bc1tl`), while an mtc1 directly before its
#        reader still gets the nop and a non-reader slot gets none. The arms
#        match the whole mtc1-branch-slot run (task #1378, FACT #9035).
#
# The seeds are written in cc1 layout (TAB, mnemonic, TAB, operands) into the
# container's own /tmp, so no VM mount sits between writing and assembling
# them. The mirror is an empty one: no seed has an `.include`.
#
# The optional argument is the asm_unit.sh under test (default: this tree's).
# It must sit in a tree with tools/ee/{ps2eeas_dli.awk,ps2eeas_dli_sites.txt,
# move_fixup.sed,mount_sync.sh}. asm_unit.sh runs $ROOT/tools/ee/mount_sync.sh
# whenever ASM_UNIT_DLISITES_MD5 is set, as the SYNC arms set it, so a copy
# missing it fails the 4 SYNC md5 arms with rc 2 even when it is unmodified
# (FACT #9011). Against 735a49e1a's copy (the parent of f6c2bae9e, which
# added both refusals), the ADJ and TAIL refusal arms must FAIL: that is the
# check that this selftest can fail. Against f6c2bae9e's copy, EMPTY, STATIC st_cc1, LIKELY, SPELL, and the ADJ/SWAP
# refusal arms (by their text) must FAIL (tasks #1147, #1170). EMPTY's exit-5
# arm fails there on its message alone: f6c2bae9e already refused a failing
# pass, as `REFUSED:`. Against 81a8ba72f's copy (#1147), the EMPTY 0-byte and
# comments-only arms and NORE's nomacro and slot-then-`jal` arms must FAIL.
# Against 205a13914's copy (task #1205), exactly the 18 DLINE, ADJ2 and SYNC
# arms that read the line, the site or the md5 must FAIL, and the rest pass.
# The arms that pin what already held pass on every copy; that they can fail
# is shown by mutants, not by an older tree (task #1170).
#
# Prints one line per arm and `asm_unit_selftest: N arms, F failed`.
# Exit 0 only when F is 0.
set -u
AU="${1:-$(cd "$(dirname "$0")" && pwd)/asm_unit.sh}"
[ -r "$AU" ] || { echo "asm_unit_selftest: cannot read $AU" >&2; exit 2; }
T="$(mktemp -d)"
trap 'rm -rf "$T"' EXIT
# An empty mirror for the seeds, passed per call below. It is deliberately not
# an assignment of the build's mirror default, which lives only in
# asmfix_default.sh (its stray-assignment lint).
mkdir -p "$T/mirror/include"; : > "$T/mirror/include/macro.inc"; : > "$T/mirror/.built"
N=0; F=0

# run <seed> <-G> [asm_unit.sh [region]] : sets RC, FAILS (count of
# `asm_unit.sh: FAIL:`), WARNS (WARNING lines), DLINE (the `asm_unit.sh: dli:`
# lines), OBJ (1 when an object exists) and WORDS (the .text words). XENV, when
# set, is extra `env` assignments for the run.
XENV=""
run() {
  rm -f "$T/o.o"
  RC=0; env ASMFIX_SHARED="$T/mirror" $XENV sh "${3:-$AU}" "${4:-usa}" "$T/$1.s" "$T/o.o" "$2" > "$T/err" 2>&1 || RC=$?
  FAILS=$(grep -c 'asm_unit\.sh: FAIL:' "$T/err" || true)
  WARNS=$(grep -c 'asm_unit\.sh: WARNING' "$T/err" || true)
  DLINE=$(grep '^asm_unit\.sh: dli:' "$T/err" || true)
  OBJ=0; WORDS=""
  if [ -s "$T/o.o" ]; then
    OBJ=1
    WORDS=$(mips-linux-gnu-objdump -d -z -j .text "$T/o.o" \
      | awk '$1 ~ /^[0-9a-f]+:$/ && $2 ~ /^[0-9a-f]+$/ && length($2) == 8 { printf "%s%s", sep, $2; sep = " " }')
  fi
}
verdict() {  # verdict <arm> <ok 0|1> <expected>
  N=$((N + 1))
  if [ "$2" = 1 ]; then r=PASS; else r=FAIL; F=$((F + 1)); fi
  echo "$r $1: expected $3; got rc=$RC fail=$FAILS warn=$WARNS obj=$OBJ${WORDS:+ words=[$WORDS]}"
  [ "$r" = FAIL ] && sed 's/^/    | /' "$T/err" | head -8
  return 0
}
refused() {  # a layout/adjacency refusal: rc 2, a FAIL line, no object
  [ "$RC" = 2 ] && [ "$FAILS" -ge 1 ] && [ "$OBJ" = 0 ] && echo 1 || echo 0
}
# passrefused <text> : refused by the dli pass's own message, which carries
# <text>, then `No object written.`; no generic REFUSED or condition line
passrefused() {
  [ "$(refused)" = 1 ] && grep -qF "$1" "$T/err" && grep -qx '  No object written\.' "$T/err" \
    && ! grep -q 'asm_unit\.sh: REFUSED:' "$T/err" && ! grep -q 'dli pass, condition' "$T/err" && echo 1 || echo 0
}
accepted() {  # rc 0, no FAIL line, an object
  [ "$RC" = 0 ] && [ "$FAILS" = 0 ] && [ "$OBJ" = 1 ] && echo 1 || echo 0
}
has() { case " $WORDS " in *" $1 "*) echo 1 ;; *) echo 0 ;; esac; }  # a contiguous run
seed() { printf "$2" > "$T/$1.s"; }  # seed <name> <printf body>

# Ps2EeAs's words for the listed `$12,0x4400000000008001` site in
# func_00290320 (tools/ee/ps2eeas_dli_sites.txt), and GNU as's for the same
# literal. In ADJ the listed seeds must carry PS2 as a contiguous run.
PS2="340c8800 000c63fc 358c8001"
GNU="3c0c4400 000c603c"
DLI='\tdli\t$12,0x4400000000008001\n'
FN0='\t.text\n\t.align\t3\n\t.globl\tfunc_00290320\n\t.ent\tfunc_00290320\nfunc_00290320:\n'
FN1='\t.end\tfunc_00290320\n'

# --- ADJ: must refuse -------------------------------------------------------
seed adj_j    "$FN0$DLI\tj\t\$31\n$FN1"
# cc1's own shape (ledger-29240): an empty #APP block between the dli and j
seed adj_app  "$FN0$DLI#APP\n#NO_APP\n\tj\t\$31\n$FN1"
seed adj_bne  "$FN0\tdli\t\$4,0xfffff000000000\n\tbne\t\$5,\$0,\$L9\n\taddu\t\$2,\$2,\$5\n\$L9:\n\tj\t\$31\n$FN1"
seed adj_crlf "$(printf "$FN0$DLI\tj\t\$31\n$FN1" | sed 's/$/\r/')\n"
# --- ADJ: must NOT refuse ----------------------------------------------------
seed nadj_addu "$FN0$DLI\taddu\t\$2,\$2,\$5\n\tj\t\$31\n$FN1"
seed nadj_nore "$FN0$DLI\t.set\tnoreorder\n\tj\t\$31\n\tnop\n\t.set\treorder\n$FN1"
seed nadj_lab  "$FN0$DLI\$L2:\n\tj\t\$31\n$FN1"
seed nadj_unl  "\t.text\n\t.globl\tfunc_00123456\n\t.ent\tfunc_00123456\nfunc_00123456:\n$DLI\tj\t\$31\n\t.end\tfunc_00123456\n"

for G in -G0 -G8; do
  for s in adj_j adj_app adj_bne adj_crlf; do
    run $s $G
    verdict "ADJ $s $G" "$(passrefused 'FACT #8623')" "refused by the pass (rc 2, FAIL naming FACT #8623, 'No object written.', no REFUSED/condition line)"
  done
  for s in nadj_addu nadj_nore nadj_lab; do
    run $s $G
    ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "$PS2")
    verdict "ADJ $s $G" "$ok" "assembled, Ps2EeAs words [$PS2]"
  done
  run nadj_unl $G
  ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "$GNU")
  verdict "ADJ nadj_unl $G" "$ok" "assembled, GNU words [$GNU ...] (not listed)"
done

# --- TAIL (-G8 rule; the -G0 value of the live control is its dead value) ---
TB='\t.text\n\t.ent\tF\nF:\n\t.set\tnoreorder\n\tbnel\t$4,$0,$L1\n'
TE='$L1:\n\tj\t$31\n\tnop\n\t.set\treorder\n\t.end\tF\n\t.extern\tg_hx, 16\n'
seed tt_bnel "$TB\tlw\t\$3,g_hx\n$TE"
seed tt_cmt  "$TB\tlw\t\$3,g_hx\t# c\n$TE"
seed tt_trsp "$TB\tlw\t\$3,g_hx \n$TE"
seed tt_ok   "\t.text\n\t.ent\tF\nF:\n\taddu\t\$2,\$3,\$4\t# high\n\tj\t\$31\n\t.end\tF\n"
# symbol + base register is cc1 layout but not a bare symbol (text/188858's
# engine arm has `lbu $2,g_itemEquippedSlot($4)`): the scan must not key it
seed tt_symreg "\t.text\n\t.ent\tF\nF:\n\tlbu\t\$2,g_hx(\$4)\n\tlhu\t\$4,g_hx+136(\$3)\n\tj\t\$31\n\t.end\tF\n\t.extern\tg_hx, 16\n"
run tt_bnel -G8
ok=$(accepted); [ "$WARNS" = 1 ] || ok=0
verdict "TAIL tt_bnel -G8 (live control)" "$ok" "rc 0, 1 WARNING"
run tt_bnel -G0
ok=$(accepted); [ "$WARNS" = 0 ] || ok=0
verdict "TAIL tt_bnel -G0 (its dead value)" "$ok" "rc 0, 0 WARNING"
for s in tt_cmt tt_trsp; do
  run $s -G8
  verdict "TAIL $s -G8" "$(refused)" "refused (rc 2, FAIL, no object)"
done
run tt_ok -G8
verdict "TAIL tt_ok -G8" "$(accepted)" "accepted (comment on a non-memop line)"
run tt_symreg -G8
verdict "TAIL tt_symreg -G8" "$(accepted)" "accepted (sym(\$r) memops are not bare-symbol)"

# --- LAY: #1103's refusals and healthy inputs -------------------------------
seed lay_indent "$TB    lw\t\$3,g_hx\n$TE"
seed lay_sep    "$TB\tlw \$3,g_hx\n$TE"
seed lay_comma  "$TB\tlw\t\$3, g_hx\n$TE"
seed lay_brsp   "\t.text\n\t.ent\tF\nF:\n\t.set\tnoreorder\n\tbnel \$4,\$0,\$L1\n\tlw\t\$3,g_hx\n$TE"
seed lay_label  "\t.text\n\t.ent\tF\nF:\n\t.set\tnoreorder\nX: bnel\t\$4,\$0,\$L1\n\tlw\t\$3,g_hx\n$TE"
seed lay_unind  "\t.text\n\t.ent\tF\nF:\n\t.set\tnoreorder\nbnel\t\$4,\$0,\$L1\n\tlw\t\$3,g_hx\n$TE"
seed lay_alldead "  .text\n  addu \$2,\$3,\$4\n  nop\n"
seed lay_dirs   "\t.text\n\t.globl\tF\n\t.extern\tg_hx, 16\n"
seed lay_crlf   "$(printf "$TB\tlw\t\$3,g_hx\n$TE" | sed 's/$/\r/')\n"
for s in lay_indent lay_sep lay_comma lay_brsp lay_label lay_unind lay_alldead; do
  run $s -G8
  verdict "LAY $s -G8" "$(refused)" "refused (rc 2, FAIL, no object)"
done
run lay_missing -G8
verdict "LAY lay_missing -G8" "$(refused)" "refused (rc 2, FAIL, no object)"
run lay_dirs -G8
verdict "LAY lay_dirs -G8" "$(accepted)" "accepted (no instruction lines)"
run lay_crlf -G8
ok=$(accepted); [ "$WARNS" = 1 ] || ok=0
verdict "LAY lay_crlf -G8" "$ok" "accepted, 1 WARNING (CRLF cc1 layout is live)"

# --- EMPTY: the dli pass must not be disabled with a green exit --------------
# tree <name> : a copy of $AU with its tools/ee siblings, at $T/<name>; prints
# the copy's asm_unit.sh. The arm then changes one file in it.
AUDIR="$(cd "$(dirname "$AU")" && pwd)"
tree() {
  mkdir -p "$T/$1/tools/ee"
  for f in ps2eeas_dli.awk ps2eeas_dli_sites.txt move_fixup.sed vu0_fixup.sed mount_sync.sh; do
    cp "$AUDIR/$f" "$T/$1/tools/ee/$f"
  done
  cp "$AU" "$T/$1/tools/ee/asm_unit.sh"
  echo "$T/$1/tools/ee/asm_unit.sh"
}
# a named condition of the dli pass fired, and no object is left
dlifail() {
  [ "$(refused)" = 1 ] && grep -q "asm_unit\.sh: FAIL: RULING #8549 dli pass, condition $1" "$T/err" && echo 1 || echo 0
}
seed nadj_bnel "$FN0$DLI\tbnel\t\$5,\$0,\$L9\n\taddu\t\$2,\$2,\$5\n\$L9:\n\tj\t\$31\n$FN1"
E0=$(tree e0);  : > "${E0%/asm_unit.sh}/ps2eeas_dli_sites.txt"
EC=$(tree ec);  printf '# comments only\n\n   # indented\n' > "${EC%/asm_unit.sh}/ps2eeas_dli_sites.txt"
EM=$(tree em);  printf 'usa func_00290320 0x0029033C $12,0x4400000000008001 3c0c440\neu x 0x1 bad 00000000 00000000\n' > "${EM%/asm_unit.sh}/ps2eeas_dli_sites.txt"
# (mawk runs END after a BEGIN exit, so the count mode must be spared)
EB=$(tree eb);  printf 'END { if (!count) exit 5 }\n' >> "${EB%/asm_unit.sh}/ps2eeas_dli.awk"
# a pass that honours the count mode but prints nothing / drops lines: only the
# outside check can see these
EN=$(tree en);  printf 'BEGIN { if (count) { print "7 7 0 0"; exit 0 } }\n' > "${EN%/asm_unit.sh}/ps2eeas_dli.awk"
ED=$(tree ed);  printf 'BEGIN { if (count) { print "7 7 0 0"; exit 0 } }\nNR %% 2 { print }\n' > "${ED%/asm_unit.sh}/ps2eeas_dli.awk"
for G in -G0 -G8; do
  # each of the three (a) cases by its own line (task #1170: they were one)
  run nadj_addu $G "$E0"; verdict "EMPTY 0-byte allowlist $G" "$(dlifail '(a): the allowlist is 0 bytes')" "refused, condition (a) '0 bytes', no object"
  run nadj_addu $G "$EC"; verdict "EMPTY comments-only allowlist $G" "$(dlifail '(a): the allowlist holds no row, only comments')" "refused, condition (a) 'only comments', no object"
  run nadj_addu $G "$EM"; verdict "EMPTY all-malformed allowlist $G" "$(dlifail '(a): no valid row (2 malformed')" "refused, condition (a) '2 malformed', no object"
  run nadj_addu $G "$EB"; ok=$(refused); grep -q 'exited 5' "$T/err" || ok=0
  verdict "EMPTY pass exits 5 $G" "$ok" "refused (rc 2, FAIL naming exit 5, no object)"
  run nadj_addu $G "$EN"; verdict "EMPTY pass prints nothing $G" "$(dlifail '(c)')" "refused, condition (c), no object"
  run nadj_addu $G "$ED"; verdict "EMPTY pass drops lines $G" "$(dlifail '(c)')" "refused, condition (c), no object"
  # a relative path is read after asm_unit.sh's cd into the mirror
  rm -f "$T/o.o"; RC=0
  (cd "$T" && env ASMFIX_SHARED="$T/mirror" sh "$AU" usa nadj_addu.s "$T/o.o" $G) > "$T/err" 2>&1 || RC=$?
  FAILS=$(grep -c 'asm_unit\.sh: FAIL:' "$T/err" || true); WARNS=0; OBJ=0; WORDS=""; [ -s "$T/o.o" ] && OBJ=1
  verdict "EMPTY relative input path $G" "$(dlifail '(b)')" "refused, condition (b), no object"
done
# the real allowlist, same seed: the must-not direction of every arm above
for G in -G0 -G8; do
  run nadj_addu $G; ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "$PS2")
  verdict "EMPTY real allowlist $G (control)" "$ok" "assembled, Ps2EeAs words [$PS2]"
done

# --- STATIC: cc1's function-static spelling (FACT #8652) ---------------------
# cc1 2.9 -O2 -G8 output for `static int s_count; static short s_small; static
# struct { int a, b; } s_pair; static int s_init = 3;` (the #1147 probes),
# trimmed: bare `name.N` and `name.N+off`, the last one in the `j $31` slot.
# The slot store must stay the 1-insn gp_rel `sw $8,..($28)`.
SB='\t.section\t.sbss\ns_count.3:\n\t.align\t2\n\t.space\t4\n\t.previous\n\t.section\t.sbss\ns_small.4:\n\t.align\t1\n\t.space\t2\n\t.previous\n\t.section\t.sbss\ns_pair.5:\n\t.align\t2\n\t.space\t8\n\t.previous\n\t.sdata\n\t.align\t2\ns_init.6:\n\t.word\t3\n\t.text\n\t.ent\tF\nF:\n'
SE='\tsh\t$4,s_small.4\n\t.set\tnoreorder\n\t.set\tnomacro\n\tj\t$31\n\tsw\t$8,s_init.6\n\t.set\tmacro\n\t.set\treorder\n\t.end\tF\n'
seed st_cc1 "$SB\tlw\t\$7,s_count.3\n\tlw\t\$8,s_init.6\n\taddu\t\$7,\$7,\$4\n\tsw\t\$7,s_count.3\n\tsw\t\$4,s_pair.5+4\n$SE"
seed st_cmt "$SB\tlw\t\$7,s_count.3\t# c\n\tlw\t\$8,s_init.6\n$SE"
run st_cc1 -G8; ok=$(accepted); [ "$WARNS" = 0 ] || ok=0
case " $WORDS " in *" 03e00008 af88"*) ;; *) ok=0 ;; esac
verdict "STATIC st_cc1 -G8" "$ok" "assembled, 0 WARNING, \`jr ra; sw \$8,..(\$gp)\` (03e00008 af88....)"
run st_cmt -G8
verdict "STATIC st_cmt -G8" "$(refused)" "refused (rc 2, FAIL, no object): a trailing comment is outside MEMOP_RE"

# --- LIKELY / SWAP: the adjacency refusal by measured branch class -----------
for B in 'beql $5,$0,$L9' 'bnel $5,$0,$L9' 'beqzl $5,$L9' 'bnezl $5,$L9' 'blezl $5,$L9' 'bgezl $5,$L9' 'bgtzl $5,$L9' 'bltzl $5,$L9' 'bgezall $5,$L9' 'bltzall $5,$L9' 'bc0fl $L9' 'bc0tl $L9' 'bc1fl $L9' 'bc1tl $L9'; do
  printf "$FN0$DLI\t${B%% *}\t${B#* }\n\taddu\t\$2,\$2,\$5\n\$L9:\n\tj\t\$31\n$FN1" > "$T/lk.s"
  for G in -G0 -G8; do
    run lk $G; ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "$PS2")
    verdict "LIKELY ${B%% *} $G" "$ok" "assembled, Ps2EeAs words [$PS2] then the branch"
  done
done
for B in 'j $31' 'j $L9' 'jal foo' 'jalr $25' 'jalr $31,$25' 'jr $5' 'b $L9' 'beq $5,$0,$L9' 'bne $5,$0,$L9' 'beqz $5,$L9' 'bnez $5,$L9' 'blez $5,$L9' 'bgez $5,$L9' 'bgtz $5,$L9' 'bltz $5,$L9' 'bgezal $5,$L9' 'bltzal $5,$L9' 'bc0f $L9' 'bc0t $L9' 'bc1f $L9' 'bc1t $L9'; do
  printf "$FN0$DLI\t${B%% *}\t${B#* }\n\taddu\t\$2,\$2,\$5\n\$L9:\n\tj\t\$31\n$FN1" > "$T/sw.s"
  for G in -G0 -G8; do
    run sw $G
    verdict "SWAP ${B%% *} ${B#* } $G" "$(passrefused 'FACT #8623')" "refused by the pass (rc 2, FAIL naming FACT #8623, 'No object written.', no REFUSED/condition line)"
  done
done

# --- NORE: a listed dli emitted under .set noreorder (task #1170) -------------
# FACT #8672's seed and control, verbatim but for the function's .align/.globl
NS='\t.set\tnoreorder\n\t.set\tnomacro\n\tjal\tfoo\n'"$DLI"'\t.set\tmacro\n\t.set\treorder\n\n'
seed nore_q2    "$FN0$NS\tjal\tbar\n\tj\t\$31\n$FN1"
seed nore_q2ctl "$FN0$NS\taddu\t\$2,\$2,\$5\n\tjal\tbar\n\tj\t\$31\n$FN1"
# no nomacro: what Ps2EeAs.exe 1.9.25.758 assembled the same input to
NO='\t.set\tnoreorder\n\tjal\tfoo\n'"$DLI"'\t.set\treorder\n'
seed nore_jal  "$FN0$NO\n\tjal\tbar\n\tj\t\$31\n$FN1"
seed nore_bnel "$FN0$NO\n\tbnel\t\$5,\$0,\$L9\n\taddu\t\$2,\$2,\$5\n\$L9:\n\tj\t\$31\n$FN1"
seed nore_addu "$FN0$NO\taddu\t\$2,\$2,\$5\n\tj\t\$31\n$FN1"
seed nore_blk  "$FN0\t.set\tnoreorder\n\taddu\t\$3,\$3,\$5\n$DLI\taddu\t\$2,\$2,\$5\n\t.set\treorder\n\tj\t\$31\n$FN1"
for G in -G0 -G8; do
  for s in nore_q2 nore_q2ctl; do
    run $s $G; ok=$(passrefused 'is under `.set nomacro`'); grep -q 'moves the expansion' "$T/err" && ok=0
    verdict "NORE $s $G" "$ok" "refused by the nomacro message (FACT #8698), no swap message, 'No object written.'"
  done
  for a in "nore_jal:0c000000 $PS2 0c000000 00000000 03e00008 00000000" \
           "nore_bnel:0c000000 $PS2 54a00002 00000000 00451021 03e00008 00000000" \
           "nore_addu:0c000000 $PS2 00451021 03e00008 00000000" \
           "nore_blk:00651821 $PS2 00451021 03e00008 00000000"; do
    run ${a%%:*} $G; ok=$(accepted); [ "$WORDS" = "${a#*:}" ] || ok=0
    verdict "NORE ${a%%:*} $G" "$ok" "assembled, exactly Ps2EeAs's words [${a#*:}]"
  done
done

# --- SPELL: one memop list, seen by both its readers (FACT #8616, #8689) -----
# (1) the spelling: code lines (not comments) carrying an alternation with two
# or more memop mnemonics. MEMOP_MN is the one; a copy of any content is a second.
N=$((N + 1))
c=$(grep -v '^[[:space:]]*#' "$AU" | grep -oE '[a-z0-9.]+(\|[a-z0-9.]+)+' \
  | awk -F'|' '{ m = 0; for (i = 1; i <= NF; i++) if ($i ~ /^(sw|sh|sb|sd|lw|lh|lhu|lb|lbu|ld)$/) m++; if (m >= 2) n++ } END { print n + 0 }')
if [ "$c" = 1 ]; then echo "PASS SPELL count: one memop alternation on a code line of \$AU"
else F=$((F + 1)); echo "FAIL SPELL count: expected one memop alternation on a code line of \$AU; got $c"; fi
# (2) the content: each cc1 memop, bare in a bnel slot, is the slot rule's
# (1 WARNING); with a trailing comment it is the layout scan's (refused).
for m in sw sh sb sd lw lh lhu lb lbu ld; do
  seed sp_bare "$TB\t$m\t\$3,g_hx\n$TE"
  seed sp_cmt  "$TB\t$m\t\$3,g_hx\t# c\n$TE"
  run sp_bare -G8; ok=$(accepted); [ "$WARNS" = 1 ] || ok=0
  verdict "SPELL $m bare -G8" "$ok" "rc 0, 1 WARNING (MEMOP_RE keys it)"
  run sp_cmt -G8
  verdict "SPELL $m tail -G8" "$(refused)" "refused (the scan's mem keys it)"
done

# --- DLINE: the per-unit transform line is an interface (task #1205) --------
# GATE-F3 (#1158) reads `asm_unit.sh: dli: N transforms (M allowlist rows for
# <region>)`, so the spelling is pinned whole-line here. N must follow the
# seed (0, 1, 2 substituted sites), M is the allowlist's rows for the region,
# counted here independently of the awk, and a refused unit prints no line.
MU=$(sed 's/#.*//' "$AUDIR/ps2eeas_dli_sites.txt" | awk '$1 == "usa" && NF >= 6 { n++ } END { print n + 0 }')
ME=$(sed 's/#.*//' "$AUDIR/ps2eeas_dli_sites.txt" | awk '$1 == "eu" && NF >= 6 { n++ } END { print n + 0 }')
seed dl_two "$FN0$DLI\taddu\t\$2,\$2,\$5\n$DLI\taddu\t\$3,\$3,\$5\n\tj\t\$31\n$FN1"
for G in -G0 -G8; do
  for a in "nadj_unl 0 usa $MU" "nadj_addu 1 usa $MU" "dl_two 2 usa $MU" "nadj_addu 0 eu $ME"; do
    set -- $a
    want="asm_unit.sh: dli: $2 transforms ($4 allowlist rows for $3)"
    run "$1" $G "$AU" "$3"; ok=$(accepted); [ "$DLINE" = "$want" ] || ok=0
    verdict "DLINE $1 $3 $G" "$ok" "assembled, exactly one line '$want'; got '$DLINE'"
  done
  # EU has no row: the listed seed keeps GNU's words there (inert, not clean)
  run nadj_addu $G "$AU" eu; ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "$GNU")
  verdict "DLINE nadj_addu eu $G words" "$ok" "assembled, GNU words [$GNU ...] (0 eu rows)"
  run adj_j $G; ok=$(refused); [ -z "$DLINE" ] || ok=0
  verdict "DLINE adj_j $G (refused)" "$ok" "refused, and no dli line"
done

# --- ADJ2: a refusal names the site that failed, not the row (task #1205) ---
# func_0027C020's one row covers two ROM sites, 0x0027C068 and 0x0027C088.
# Seed the SECOND site to fail: a message naming the row's address alone cannot
# tell it from the first. The first-site seed is the other answer.
R9='\tdli\t$5,0x8000000044\n'
C0='\t.text\n\t.align\t3\n\t.globl\tfunc_0027C020\n\t.ent\tfunc_0027C020\nfunc_0027C020:\n'
C1='\t.end\tfunc_0027C020\n'
seed adj2_2nd "$C0$R9\taddu\t\$2,\$2,\$5\n$R9\tj\t\$31\n$C1"
seed adj2_1st "$C0$R9\tj\t\$31\n$R9\taddu\t\$2,\$2,\$5\n$C1"
seed adj2_nm  "$C0$R9\taddu\t\$2,\$2,\$5\n\t.set\tnoreorder\n\t.set\tnomacro\n\tjal\tfoo\n$R9\t.set\tmacro\n\t.set\treorder\n\tj\t\$31\n$C1"
for G in -G0 -G8; do
  for a in "adj2_2nd|2|8|FACT #8623" "adj2_1st|1|6|FACT #8623" "adj2_nm|2|11|is under \`.set nomacro\`"; do
    oifs=$IFS; IFS='|'; set -- $a; IFS=$oifs
    at="site $2 in func_0027C020 of the row at ROM 0x0027C068, pass input line $3"
    run "$1" $G; ok=$(passrefused "$4"); grep -qF "$at" "$T/err" || ok=0
    [ "$(grep -c 'asm_unit\.sh: FAIL: listed dli' "$T/err")" = 1 ] || ok=0
    verdict "ADJ2 $1 $G" "$ok" "refused by the pass, one FAIL line naming '$at'"
  done
done

# --- SYNC: the allowlist's host md5 (task #1205, FACT #8713) ----------------
# A read truncated on a row boundary is a well-formed, SHORTER allowlist: the
# row count passes it. Seeded here as a copy missing its last row, with the
# md5 of the whole file as the host's: the md5 check must refuse it (condition
# (m)), and without the md5 the same copy must assemble, which is the hole.
SY=$(tree sy); SYL="${SY%/asm_unit.sh}/ps2eeas_dli_sites.txt"
SYMD5=$(md5sum < "$SYL" | cut -d' ' -f1)
last=$(awk '!/^[[:space:]]*(#|$)/ { n = NR } END { print n }' "$SYL")
awk -v l="$last" 'NR < l' "$SYL" > "$SYL.cut"; mv "$SYL.cut" "$SYL"
for G in -G0 -G8; do
  XENV="ASM_UNIT_DLISITES_MD5=$SYMD5 MOUNT_SYNC_TRIES=2 MOUNT_SYNC_SLEEP=0"
  run nadj_addu $G "$SY"; ok=$(dlifail '(m)'); grep -q '^MOUNT-SYNC FAIL' "$T/err" || ok=0
  verdict "SYNC short allowlist, host md5 $G" "$ok" "refused, condition (m) and a MOUNT-SYNC FAIL line, no object"
  XENV=""
  run nadj_addu $G "$SY"; ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "$PS2")
  [ "$DLINE" = "asm_unit.sh: dli: 1 transforms ($((MU - 1)) allowlist rows for usa)" ] || ok=0
  verdict "SYNC short allowlist, no md5 $G (the hole)" "$ok" "assembled with $((MU - 1)) rows: the row count cannot see it"
  XENV="ASM_UNIT_DLISITES_MD5=$(md5sum < "$AUDIR/ps2eeas_dli_sites.txt" | cut -d' ' -f1)"
  run nadj_addu $G; ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "$PS2")
  grep -q 'mount_sync\|MOUNT-SYNC' "$T/err" && ok=0
  verdict "SYNC real allowlist, its md5 $G (control)" "$ok" "assembled, Ps2EeAs words, no mount_sync line"
  XENV=""
done

# --- MTC1: the hazard nop and a held noreorder branch (task #1352, FACT #8063)
# At -G8 an mtc1 directly followed by an FPU op reading its register gets one
# nop inside noreorder. A branch between them ends that window: the ROM has
# 100 `mtc1 $r,$fN; jump; slot reads $fN` sites and none with a nop (NOTE
# #8972). Before task #1352 the held branch skipped the window's reset, so
# the nop landed between the branch and its slot (wrong code). Against a copy
# from before that task, the two slot arms must FAIL; the other two pass on
# both copies (the rule still pads, and a non-reader slot was never padded).
# The three branch arms assert the whole run from the mtc1 to the slot, not
# only the branch and its slot: a nop printed before the held branch gives
# `mtc1; nop; jal; reader`, a shape with 0 ROM sites, and the adjacent pair
# alone passed it (FACT #9035, task #1378). Against asm_unit.sh with
# `if (lastmtc != "") print "\tnop";` before the hold's `pendbr = $0`, mt_slot
# and mt_nordr must FAIL (the jal hold) and mt_lkly must FAIL (the bc1tl one).
MH='\t.text\n\t.ent\tF\nF:\n'; ME='\t.end\tF\n'
NM='\t.set\tnoreorder\n\t.set\tnomacro\n'; RM='\t.set\tmacro\n\t.set\treorder\n'
seed mt_slot  "$MH\tli.s\t\$f12,1.00000000000000000000e0\n$NM\tjal\tG\n\tsub.s\t\$f12,\$f12,\$f20\n$RM\tj\t\$31\n$ME"
seed mt_lkly  "$MH\tmtc1\t\$1,\$f12\n\t.set\tnoreorder\n\tbc1tl\t\$L1\n\tadd.s\t\$f0,\$f12,\$f2\n\t.set\treorder\n\$L1:\n\tj\t\$31\n$ME"
seed mt_dirct "$MH\t.set\tnoreorder\n\tmtc1\t\$1,\$f12\n\tmul.s\t\$f12,\$f14,\$f12\n\t.set\treorder\n\tj\t\$31\n$ME"
seed mt_nordr "$MH\tmtc1\t\$1,\$f12\n$NM\tjal\tG\n\tmul.s\t\$f0,\$f14,\$f2\n$RM\tj\t\$31\n$ME"
run mt_slot -G8
ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "0c000000 46146301"); [ "$ok" = 1 ] && ok=$(has "44816000 0c000000 46146301")
verdict "MTC1 mt_slot -G8" "$ok" "mtc1, jal, then sub.s \$f12 in its slot, no nop anywhere in the run [44816000 0c000000 46146301]"
run mt_lkly -G8
ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "46026000"); [ "$(has "00000000 46026000")" = 0 ] || ok=0
[ "$ok" = 1 ] && ok=$(has "44816000 45030001 46026000")
verdict "MTC1 mt_lkly -G8" "$ok" "mtc1, bc1tl, then add.s \$f0,\$f12 in its slot, no nop anywhere in the run [44816000 45030001 46026000]"
run mt_dirct -G8
ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "44816000 00000000 460c7302")
verdict "MTC1 mt_dirct -G8 (the rule still pads)" "$ok" "mtc1, nop, mul.s reading \$f12 [44816000 00000000 460c7302]"
run mt_nordr -G8
ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "0c000000 46027002"); [ "$ok" = 1 ] && ok=$(has "44816000 0c000000 46027002")
verdict "MTC1 mt_nordr -G8 (non-reader slot)" "$ok" "mtc1, jal, then mul.s \$f0,\$f14,\$f2 in its slot, no nop anywhere in the run [44816000 0c000000 46027002]"

# One arm per further held branch kind (task #1385). The arms above seed only
# jal and bc1tl, so a hold that pads every OTHER kind passed 181/0 (FACT
# #9086's `nonjal` mutant). The kinds are the ones the ROM has after an mtc1,
# largest population first: a ROM-word census over EE code (rom 0x15180-0x33B00
# and 0x16E980-0x253000) of `mtc1 $r,$fN` directly followed by the branch,
# whose slot is a .s arithmetic/compare/cvt op reading $fN, gives b 10, beql 3,
# j 3, beq 2, bne 2, bc1t 2 and bc1f 0 (jal 63). bc1f's 5 adjacent ROM sites
# all carry a non-reader slot (a nop, a swc1, a daddu); it is seeded with a
# reader anyway, like mt_lkly. Every seed is cc1's slot bracket around the
# same `mtc1 $1,$f12; <branch>; add.s $f0,$f12,$f2` run, and each arm asserts
# that run with no word between. Against asm_unit.sh with
# `if (lastmtc != "" && $0 ~ /^\t<kind>\t/) print "\tnop";` before line 543's
# `pendbr = $0`, exactly that kind's arm must FAIL.
MT='\tmtc1\t$1,$f12\n'; MR='\tadd.s\t$f0,$f12,$f2\n'; ML='$L1:\n\tj\t$31\n'
for k in "b:\$L1:10000001" "beql:\$2,\$0,\$L1:50400001" "j:\$L1:08000003" \
         "beq:\$2,\$0,\$L1:10400001" "bne:\$2,\$0,\$L1:14400001" \
         "bc1t:\$L1:45010001" "bc1f:\$L1:45000001"; do
  op=${k%%:*}; w=${k##*:}; arg=${k#*:}; arg=${arg%:*}
  seed "mt_$op" "$MH$MT$NM\t$op\t$arg\n$MR$RM$ML$ME"
  run "mt_$op" -G8
  ok=$(accepted); [ "$ok" = 1 ] && ok=$(has "44816000 $w 46026000")
  verdict "MTC1 mt_$op -G8" "$ok" "mtc1, $op, then add.s \$f0,\$f12 in its slot, no nop anywhere in the run [44816000 $w 46026000]"
done

echo "asm_unit_selftest: $N arms, $F failed ($AU)"
[ "$F" = 0 ]

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
#        at -G0 and -G8. The same dli before a non-branch, before a branch in
#        noreorder, before a label, and the same literal in a function that is
#        not listed all assemble (rc 0), and the listed ones carry Ps2EeAs's
#        words, the unlisted one GNU's.
#   TAIL (FACT #8616) at -G8, a bare-symbol slot memop with a trailing comment
#        or space is refused; the same seed without the tail is live (1
#        WARNING at -G8, 0 at -G0: the matcher sees it); a trailing comment on
#        a non-memop line is still accepted.
#   LAY  (#1103) the C2-A layout refusals still fire (8 dead seeds) and the
#        healthy seeds (directives-only, CRLF) are still accepted.
#
# The seeds are written in cc1 layout (TAB, mnemonic, TAB, operands) into the
# container's own /tmp, so no VM mount sits between writing and assembling
# them. The mirror is an empty one: no seed has an `.include`.
#
# The optional argument is the asm_unit.sh under test (default: this tree's).
# It must sit in a tree with tools/ee/{ps2eeas_dli.awk,ps2eeas_dli_sites.txt,
# move_fixup.sed}. Run against master's copy, the ADJ and TAIL refusal arms
# must FAIL: that is the check that this selftest can fail.
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

# run <seed> <-G> : sets RC, FAILS (count of `asm_unit.sh: FAIL:`), WARNS
# (WARNING lines), OBJ (1 when an object exists) and WORDS (the .text words).
run() {
  rm -f "$T/o.o"
  RC=0; env ASMFIX_SHARED="$T/mirror" sh "$AU" usa "$T/$1.s" "$T/o.o" "$2" > "$T/err" 2>&1 || RC=$?
  FAILS=$(grep -c 'asm_unit\.sh: FAIL:' "$T/err" || true)
  WARNS=$(grep -c 'asm_unit\.sh: WARNING' "$T/err" || true)
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
    ok=$(refused)
    [ "$ok" = 1 ] && ! grep -q 'FACT #8623' "$T/err" && ok=0
    verdict "ADJ $s $G" "$ok" "refused (rc 2, FAIL naming FACT #8623, no object)"
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

echo "asm_unit_selftest: $N arms, $F failed ($AU)"
[ "$F" = 0 ]

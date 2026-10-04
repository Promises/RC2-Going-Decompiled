#!/bin/sh
# s136os_splice.sh — the s136os compile arm (task #1257, FACT #8810): compile a
# SELECTED function alone with SN 2.95.3 v1.36 `-fopt-stack` and splice its
# `.ent`..`.end` block into the unit's cc1 2.9 output, so the image carries C
# compiled by the compiler the ROM's function was built with.
#
#   sh tools/ee/s136os_splice.sh <region> <unit> <src> <unit.s> <GFLAG> [CC1EXTRA]
#   The 6th argument is the CALLER's S136EXTRA for the unit: the 2.9 compile's
#   CC1EXTRA unless the flag table gives this arm its own (RULING #9004).
#   e.g. sh tools/ee/s136os_splice.sh usa text/248B50 \
#            going-decompiled/src/usa/text/248B50.c $BUILD/.../248B50._u.s -G8 -fno-gcse
#
# ONE helper, two callers: build.sh (the image) and objdiff_build.sh (the unit
# report's sdk29 base), each between its cc1 2.9 step and asm_unit.sh. Runs
# INSIDE the ee-build container at the repo root, POSIX sh + awk. <unit.s> is
# replaced only on success; a unit with no selector row and no slot is left
# untouched.
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
#   - REFUSED (task #1326): once every block is spliced, a block would not
#     assemble as it did in its s136os TU: a symbol's gp-relative/absolute
#     class differs (ADDRESSING), or it needs a definition the s136os TU has
#     outside .ent..end and the 2.9 TU lacks (DEFINITION). See verify_block.
#     Without it these spliced silently and surfaced only as a whole-image cmp
#     (BuildTieDrawSegment, 328,879 B) or a link error (SelectSceneSubChunk).
#   - LENGTH (task #1531): once every block verifies, the spliced unit is
#     assembled (asm_unit.sh, as the caller does next) and any row whose
#     st_size differs from its ROM length is refused, naming both lengths in
#     words. The ROM length is read from the row's splat .s by rom_size (its
#     `nonmatching` header, which must equal its glabel..endlabel word count;
#     an .s that disagrees with itself is an ORACLE refusal). Without it a
#     short body spliced silently and only the image cmp saw it (#1509, #1512).
#     On success each row prints a `length:` line.
#   Every FATAL leaves <unit.s> untouched: the splice works on a copy.
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
# TU's own end-of-file line does — UNLESS the s136os TU declares it at two
# sizes, one > -G: the carry sorts them, so the last one in front of the block
# may not be the one in force there in the s136os TU. verify_block refuses that
# case (SelectSceneSubChunk's in-arm `.extern …Abs, 16` + cc1's `, 4`).
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

# verify_block <G> <fn> <s136os.s> <spliced unit.s> <block>: print, one per
# line, every reason the spliced <block> would not assemble as it did in the
# s136os TU it was compiled (and measured, FACT #8830) in. Empty = admit.
# Two conditions (task #1326, FACT #8838 + task #1309's three image failures):
#  (1) ADDRESSING: for each symbol the block names that either file declares
#      `.extern`, the gp-relativity GNU as gives the block's bare-symbol access
#      differs between the two files. The model, measured on the container's
#      GNU as 2.40 at -G8 (8 probes, task #1326; FACT #8853 had 4 of them):
#      if the last `.extern <sym>, N` BEFORE the use has N > G the access is
#      pinned absolute there; otherwise gas defers to the end of the file and
#      it is gp-relative iff the file's LAST size is in 1..G. The position is
#      the block's `.ent <fn>` in both files (cc1
#      prints no `.extern` inside .ent..end; a carried line sits in front of
#      it). This is what FACT #8838's
#      "size conflict" means: a conflict that leaves the class the same (the
#      unit's ,16 device before the block and cc1's ,4 at the end, which both
#      files carry) is admitted, one that flips it (BuildTieDrawSegment: the
#      s136os TU sized the store's symbol 4, the unit 16) is refused.
#  (2) DEFINITION: the block names a symbol the s136os TU DEFINES outside the
#      block — an asm equate `<sym> = …` / `.set <sym>, …` / `.equ`, a label,
#      `.comm`/`.lcomm` — and the spliced unit has no identical definition
#      line. That definition came from source the 2.9 TU did not see (an
#      equate inside the member's own guard arm: SelectSceneSubChunk), so the
#      block would link to nothing, or to a different definition.
# A text scan, so it fails CLOSED: anything it cannot see as identical in the
# unit (e.g. the same equate spelled differently) is a loud false refusal,
# never a false admit. Its bound: symbols defined only inside an `.include`d
# file are invisible to both sides alike, and the model covers bare-symbol
# macro accesses (explicit %gp_rel/%hi/%lo operands are unaffected by -G).
verify_block() {
  awk -v G="$1" -v fn="$2" '
    function norm(l) { sub(/\r$/, "", l); sub(/#.*/, "", l); gsub(/[ \t]+/, " ", l); sub(/^ /, "", l); sub(/ $/, "", l); return l }
    function cls(pre, fin) { return (pre + 0 > G) ? "abs" : ((fin + 0 > 0 && fin + 0 <= G) ? "gp" : "abs") }
    function why(pre, fin) { return (pre + 0 > G) ? "absolute (.extern size " pre " before the block)" : (cls(pre, fin) == "gp" ? "gp-relative (" (pre == "" ? "no" : "size " pre) " .extern before the block, final size " fin ")" : "absolute (" (pre == "" ? "no" : "size " pre) " .extern before the block, final size " (fin == "" ? "none" : fin) ")") }
    FNR == 1 { file++; past = 0 }
    { raw = $0; sub(/\r$/, "", raw); l = norm(raw); split(l, F, " ") }
    # file 3 = the block: every identifier its instruction lines name.
    file == 3 {
      if (l == "" || F[1] ~ /^[.#]/ || F[1] ~ /:$/) next
      s = l; sub(/^[^ ]+ ?/, "", s)
      while (match(s, /[A-Za-z_][A-Za-z0-9_.]*/)) { t = substr(s, RSTART, RLENGTH); if (RSTART == 1 || substr(s, RSTART - 1, 1) !~ /[$0-9]/) used[t] = 1; s = substr(s, RSTART + RLENGTH) }
      next }
    # files 1 (s136os TU) and 2 (spliced unit): .extern sizes before/at the end, definitions.
    F[1] == ".ent" && F[2] == fn { past = 1; inblk = 1 }
    F[1] == ".end" && F[2] == fn { inblk = 0; next }
    inblk { next }
    F[1] == ".extern" {
      sym = F[2]; sub(/,.*/, "", sym); sz = l; sub(/^[^,]*, ?/, "", sz); if (sz == l) sz = ""
      fin[file, sym] = sz; ext[sym] = 1; if (!past) pre[file, sym] = sz; next }
    {
      d = ""
      if (F[1] ~ /^[A-Za-z_][A-Za-z0-9_.]*:$/) { d = F[1]; sub(/:$/, "", d) }
      else if (F[2] == "=") d = F[1]
      else if (F[1] ~ /^[A-Za-z_][A-Za-z0-9_.]*=/) { d = F[1]; sub(/=.*/, "", d) }
      else if (F[1] ~ /^\.(set|equ|equiv|comm|lcomm)$/ && F[2] ~ /,/) { d = F[2]; sub(/,.*/, "", d) }
      if (d == "") next
      # (two statements: mawk creates def1[d] before testing `d in def1`)
      if (file == 1) { prev = (d in def1) ? def1[d] SUBSEP : ""; def1[d] = prev l }
      else def2[d, l] = 1
    }
    END {
      for (t in used) {
        if (t in ext) {
          if (cls(pre[1, t], fin[1, t]) != cls(pre[2, t], fin[2, t]))
            print "ADDRESSING " t ": " why(pre[1, t], fin[1, t]) " in the s136os TU, " why(pre[2, t], fin[2, t]) " in the spliced unit"
        }
        if (t in def1) {
          n = split(def1[t], D, SUBSEP)
          for (i = 1; i <= n; i++) if (!((t, D[i]) in def2)) print "DEFINITION " t ": the s136os TU defines it outside the block (`" D[i] "`), the spliced unit has no such line"
        }
      }
    }' "$3" "$4" "$5" | sort
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

# rom_size <fn.s> <fn>: print the ROM function's length in BYTES from its
# splat .s (task #1531), or `ORACLE <why>` when it cannot be read or disagrees
# with itself. Two readings of the one file must agree: the `nonmatching <fn>,
# 0xNN` header splat writes, and the count of `/* rom vaddr word */` lines
# strictly between `glabel <fn>` and `endlabel <fn>` (FACT #7740: splat puts
# alignment padding AFTER endlabel, so a whole-file count is too high). They
# agree on all 268 USA rows at 4e6750ea; symbol_addrs `size:` is on none of
# them, so it is not consulted. A disagreement is reported as the ORACLE's
# fault, naming both numbers, never resolved by picking one.
rom_size() {
  awk -v fn="$2" '
    function hex(x,   i, c, v) { v = 0; x = tolower(x); sub(/^0x/, "", x)
      for (i = 1; i <= length(x); i++) { c = index("0123456789abcdef", substr(x, i, 1)); if (!c) return -1; v = v * 16 + c - 1 }
      return v }
    { sub(/\r$/, "") }
    $1 == "nonmatching" && $2 == fn "," { nh++; h = hex($3) }
    $1 == "endlabel" && $2 == fn { ne++; inb = 0 }
    inb && $1 == "/*" && $5 == "*/" && length($4) == 8 && $4 ~ /^[0-9A-Fa-f]+$/ { n++ }
    $1 == "glabel" && $2 == fn { ng++; inb = 1 }
    END {
      if (nh != 1 || ng != 1 || ne != 1) { printf "ORACLE %d nonmatching / %d glabel / %d endlabel line(s) for %s in %s, want 1 each\n", nh, ng, ne, fn, FILENAME; exit }
      if (h <= 0 || h % 4) { printf "ORACLE nonmatching size %s for %s is not a positive word multiple\n", h, fn; exit }
      if (h != n * 4) { printf "ORACLE %s: nonmatching header 0x%x (%d words) but %d words between glabel and endlabel in %s\n", fn, h, h / 4, n, FILENAME; exit }
      print h
    }' "$1"
}

# length_check <nm -S file> <rows file>: one line per row whose assembled
# st_size is not its ROM length (task #1531); empty = every length matches.
# <rows file> lines are `<fn> <ROM bytes>`; <nm -S file> is `mips-linux-gnu-nm
# -S` of the spliced unit assembled exactly as the build assembles it.
# EITHER direction refuses here: st_size is the body alone (cc1 ends it at
# `.end`, before any alignment word), so unlike vmu's word compare there is no
# legitimate LONGER case. A missing or repeated sized symbol refuses too.
length_check() {
  awk '
    function hex(x,   i, c, v) { v = 0; x = tolower(x)
      for (i = 1; i <= length(x); i++) { c = index("0123456789abcdef", substr(x, i, 1)); if (!c) return -1; v = v * 16 + c - 1 }
      return v }
    FNR == 1 { file++ }
    file == 1 { if (NF == 4 && $3 ~ /^[Tt]$/) { sz[$4] = hex($2); cnt[$4]++ } next }
    {
      if (!($1 in cnt)) print $1 ": no sized .text symbol in the assembled unit (ROM " $2 / 4 " words)"
      else if (cnt[$1] != 1) print $1 ": " cnt[$1] " sized .text symbols in the assembled unit"
      else if (sz[$1] != $2) printf "%s: built %d words (assembled st_size 0x%x), ROM %d words (0x%x) — %s\n", $1, sz[$1] / 4, sz[$1], $2 / 4, $2, (sz[$1] < $2 ? "SHORTER" : "LONGER")
    }' "$1" "$2"
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
  # Arms 4-8 (task #1326): verify_block, seeded. Each refuse arm must name its
  # member's symbol and class; each admit arm must print nothing. A verifier
  # that admits everything fails 4, 6 and 8; one that refuses every size or
  # definition difference fails 5 and 7.
  B='\t.ent\tf\nf:\n\tsw\t$5,X\n\tlw\t$4,Y\n\tjr\t$31\n\t.end\tf\n'
  printf "$B" > "$T/blk"
  vb() { verify_block 8 f "$T/solo.s" "$T/unit.s" "$T/blk"; }
  # 4: BuildTieDrawSegment (#1309): the s136os TU sizes X 4, the unit 16.
  printf "\t.extern\tX, 4\n${B}\t.extern\tX, 4\n" > "$T/solo.s"; printf "\t.extern\tX, 16\n#S136OS_BEGIN f\n${B}#S136OS_END f\n" > "$T/unit.s"
  v="$(vb)"; case "$v" in "ADDRESSING X: gp-relative"*"absolute (.extern size 16 before the block) in the spliced unit") echo "  OK   arm 4: X sized 4 in the s136os TU, 16 in the unit -> refused: $v" ;; *) echo "  FAIL arm 4: not refused as ADDRESSING X: '$v'"; rc=1 ;; esac
  # 5: FACT #8838's members as the #1281 helper splices them: the unit's ,16
  # device precedes both blocks, cc1's ,4 ends the s136os TU -> absolute twice.
  printf "\t.extern\tX, 16\n${B}\t.extern\tX, 4\n" > "$T/solo.s"; printf "\t.extern\tX, 16\n#S136OS_BEGIN f\n${B}#S136OS_END f\n" > "$T/unit.s"
  v="$(vb)"; if [ -z "$v" ]; then echo "  OK   arm 5: ,16 device before the block in both files, ,4 only at the s136os end -> admitted"; else echo "  FAIL arm 5: refused a same-class size difference: '$v'"; rc=1; fi
  # 8: the pre-#1281 defect itself: a carried ,4 in front of the block over the
  # unit's ,16 device.
  printf "\t.extern\tX, 16\n#S136OS_BEGIN f\n\t.extern X, 4\n${B}#S136OS_END f\n" > "$T/unit.s"
  v="$(vb)"; case "$v" in "ADDRESSING X: absolute"*"gp-relative"*"in the spliced unit") echo "  OK   arm 8: a ,4 carried over the unit's ,16 device -> refused: $v" ;; *) echo "  FAIL arm 8: not refused as ADDRESSING X: '$v'"; rc=1 ;; esac
  # 6: SelectSceneSubChunk (#1309): Y's equate is only in the s136os TU.
  printf "\tY = Z\n${B}" > "$T/solo.s"; printf "#S136OS_BEGIN f\n${B}#S136OS_END f\n" > "$T/unit.s"
  v="$(vb)"; case "$v" in "DEFINITION Y: "*) echo "  OK   arm 6: equate Y only in the s136os TU -> refused: $v" ;; *) echo "  FAIL arm 6: not refused as DEFINITION Y: '$v'"; rc=1 ;; esac
  # 7: the #1309 fix: the same equate in both files.
  printf "\tY = Z\n#S136OS_BEGIN f\n${B}#S136OS_END f\n" > "$T/unit.s"
  v="$(vb)"; if [ -z "$v" ]; then echo "  OK   arm 7: equate Y identical in both files -> admitted"; else echo "  FAIL arm 7: refused an equate both files carry: '$v'"; rc=1; fi
  # Arms 9-10 (task #1531): the ROM length oracle. 9 reads a 3-word function
  # (with a pad word after endlabel, FACT #7740, which must not count); 10 is
  # the same file with a header that disagrees, which must be refused naming
  # both readings, never resolved by picking one.
  ROMS='.align 3\nnonmatching f, %s\n\nglabel f\n    /* 000100 00200100 27BDFFF0 */  addiu $29,$29,-16\n  .Lx:\n    /* 000104 00200104 03E00008 */  jr $31\n    /* 000108 00200108 27BD0010 */   addiu $29,$29,16\nendlabel f\n    /* 00010C 0020010C 00000000 */  nop\n'
  printf "$ROMS" 0xC > "$T/f.s"; v="$(rom_size "$T/f.s" f)"
  if [ "$v" = 12 ]; then echo "  OK   arm 9: 3-word function, pad word after endlabel not counted -> 12 bytes"; else echo "  FAIL arm 9: want 12, got '$v'"; rc=1; fi
  printf "$ROMS" 0x10 > "$T/f.s"; v="$(rom_size "$T/f.s" f)"
  case "$v" in "ORACLE f: nonmatching header 0x10 (4 words) but 3 words between glabel and endlabel"*) echo "  OK   arm 10: header 0x10 vs 3 words -> refused: $v" ;; *) echo "  FAIL arm 10: not refused as an ORACLE disagreement: '$v'"; rc=1 ;; esac
  # Arms 11-14 (task #1531): the length compare, on #1509's V1 shape. 11 is
  # V1 itself (st_size 0x5c, ROM 0x60): refused naming the function, SHORTER
  # and both lengths. 12: equal, admitted. 13: LONGER, refused. 14: no sized
  # symbol, refused. A check that admits everything fails 11, 13 and 14; one
  # that refuses everything fails 12.
  printf 'func_002CE8A8 96\n' > "$T/rows"
  for a in "11 0000005c" "12 00000060" "13 00000064" "14 -"; do
    set -- $a
    if [ "$2" = - ]; then printf '000047a8 t $L5_s136_func_002CE8A8\n' > "$T/nm"; else printf "000047a8 $2 T func_002CE8A8\n000047a8 t gcc2_compiled.\n" > "$T/nm"; fi
    v="$(length_check "$T/nm" "$T/rows")"
    case "$1:$v" in
      "11:func_002CE8A8: built 23 words (assembled st_size 0x5c), ROM 24 words (0x60) — SHORTER") echo "  OK   arm 11: V1's 23 words vs ROM 24 -> refused: $v" ;;
      "12:") echo "  OK   arm 12: st_size 0x60 = ROM 0x60 -> admitted" ;;
      "13:func_002CE8A8: built 25 words (assembled st_size 0x64), ROM 24 words (0x60) — LONGER") echo "  OK   arm 13: 25 words vs ROM 24 -> refused: $v" ;;
      "14:func_002CE8A8: no sized .text symbol"*) echo "  OK   arm 14: no sized symbol -> refused: $v" ;;
      *) echo "  FAIL arm $1: '$v'"; rc=1 ;;
    esac
  done
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
# The -G the unit is assembled at (asm_unit.sh takes the same GFLAG): the
# threshold verify_block's addressing model compares sizes against.
GNUM="${GFLAG#-G}"
case "$GNUM" in ''|*[!0-9]*) fatal "GFLAG '$GFLAG' is not -G<N>; the addressing check cannot be computed for:" $ROWS ;; esac
# Spliced into a copy; <unit.s> is replaced only after every block verifies.
OUT="$TMP/work.s"
cp "$UNIT_S" "$OUT"
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
  # (No line count here: cc1's text lines are macro instructions, not words.
  # #1509's 23-word and 24-word bodies both printed "22 insn lines". The body
  # length is the `length:` line below, from the assembled unit, task #1531.)
  echo "s136os_splice: $REGION/$UNIT: $f spliced ($(wc -l < "$TMP/$f.ext" | tr -d ' ') .extern carried; not carried, the unit declares the name: ${KEPT:-none})"
done
LEFT="$(awk '{ sub(/\r$/, "") } $1 == "#S136OS_SLOT" { print $2 }' "$OUT")"
[ -z "$LEFT" ] || fatal "slot(s) still present after the splice:" $LEFT
# VERIFY (task #1326): every block against the FINAL unit, so a line carried for
# one member is seen by every other (FACT #8842 point 1). Fails closed: rc 3,
# <unit.s> untouched, every offender listed by member and cause. No fallback.
BADV=""
for f in $ROWS; do
  verify_block "$GNUM" "$f" "$TMP/$f.s" "$OUT" "$TMP/$f.blk2" > "$TMP/$f.verify"
  BADV="$BADV$(sed "s/^/$f: /" "$TMP/$f.verify")
"
done
BADV="$(printf '%s' "$BADV" | sed '/^$/d')"
if [ -n "$BADV" ]; then
  echo "s136os_splice: FATAL [$REGION/$UNIT] — REFUSED (task #1326): a spliced block would not assemble as it did in the s136os TU it was measured in. ADDRESSING = the unit's .extern sizes flip a bare-symbol access between gp-relative and absolute; DEFINITION = the block needs a symbol the s136os TU defines outside .ent..end (e.g. an asm equate inside the member's own guard arm) and the 2.9 TU does not. Move the device or equate where both TUs see it, identically. Nothing was written to $UNIT_S." >&2
  printf '%s\n' "$BADV" | sed 's/^/    /' >&2
  exit 3
fi
# LENGTH (task #1531): every spliced body must be exactly as long as the ROM
# function it replaces. A SHORT body shifts every later byte of the unit, and
# before this check nothing per-function saw it: the splice line said
# `spliced`, vmu compared only the built words, and only the image cmp failed
# (#1509 V1 23 vs 24 words, cmp 47; V2 28 vs 31, cmp 454251; #1512 26 vs 32,
# cmp 445028). The built length is measured, not estimated: the spliced unit is
# assembled by asm_unit.sh exactly as the caller assembles it next (cc1 text
# lines are macro instructions and asm_unit.sh rewrites some, so no count of
# them is a length), and each row's st_size is read from that object. The ROM
# length is rom_size's. Fails closed: rc 3, <unit.s> untouched, every
# offender named with both lengths.
ROMSZ="$TMP/rom_sizes"; : > "$ROMSZ"; BADO=""
for f in $ROWS; do
  v="$(rom_size "going-decompiled/asm/$REGION/nonmatchings/$UNIT/$f.s" "$f" 2>&1)"
  case "$v" in ''|*[!0-9]*) BADO="$BADO
$f: ${v:-no ROM .s at going-decompiled/asm/$REGION/nonmatchings/$UNIT/$f.s}" ;; *) echo "$f $v" >> "$ROMSZ" ;; esac
done
BADO="$(printf '%s' "$BADO" | sed '/^$/d')"
[ -z "$BADO" ] || { echo "s136os_splice: FATAL [$REGION/$UNIT] — the ROM length of a row cannot be read from its splat .s, so its spliced length cannot be checked (task #1531). Nothing was written to $UNIT_S." >&2; printf '%s\n' "$BADO" | sed 's/^/    /' >&2; exit 3; }
case "$TMP" in /*) ATMP="$TMP" ;; *) ATMP="$(pwd)/$TMP" ;; esac
# asm_unit.sh cds into its mirror, so its paths are absolute. ASM_UNIT_S_MD5
# is a caller's md5 of ITS input; it is not this copy's, so it is cleared.
if ! ASM_UNIT_S_MD5= sh tools/ee/asm_unit.sh "$REGION" "$ATMP/work.s" "$ATMP/len.o" "$GFLAG" > "$TMP/len.asm.log" 2>&1 \
   || [ ! -s "$TMP/len.o" ]; then
  echo "s136os_splice: FATAL [$REGION/$UNIT] — the spliced unit did not assemble for the length check (task #1531); asm_unit.sh said:" >&2
  sed 's/^/    | /' "$TMP/len.asm.log" >&2
  exit 3
fi
rm -rf "$TMP/.asmfix-$REGION-len"
mips-linux-gnu-nm -S --defined-only "$TMP/len.o" > "$TMP/len.nm"
BADL="$(length_check "$TMP/len.nm" "$ROMSZ")"
if [ -n "$BADL" ]; then
  echo "s136os_splice: FATAL [$REGION/$UNIT] — LENGTH (task #1531): a spliced body is not as long as the ROM function it replaces, so every later byte of the unit would shift. Nothing was written to $UNIT_S." >&2
  printf '%s\n' "$BADL" | sed 's/^/    /' >&2
  exit 3
fi
while read -r f b; do
  echo "s136os_splice: $REGION/$UNIT: $f length: built $((b / 4)) words = ROM $((b / 4)) words (assembled st_size; nonmatching header = glabel..endlabel words)"
done < "$ROMSZ"
cp "$OUT" "$UNIT_S"

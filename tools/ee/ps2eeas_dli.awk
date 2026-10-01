# ps2eeas_dli.awk: RULING #8549's scoped Ps2EeAs `dli` expansion. It is a
# pre-assembly pass over one cc1 unit .s, run by tools/ee/asm_unit.sh:
#
#   awk -v region=usa -v sites=tools/ee/ps2eeas_dli_sites.txt \
#       -f tools/ee/ps2eeas_dli.awk < unit.s
#   awk -v count=1 -v sites=tools/ee/ps2eeas_dli_sites.txt \
#       -f tools/ee/ps2eeas_dli.awk < /dev/null
#
# The second form reads only the allowlist and prints `<valid> <usa> <eu>
# <bad>`: its valid rows, those per region, and its malformed, duplicate or
# unspellable ones. asm_unit.sh refuses a unit when <valid> is 0.
#
# The ROM's assembler was SN Ps2EeAs.exe. At the sites listed in
# ps2eeas_dli_sites.txt, its expansion of a 64-bit `dli` differs from the one
# GNU as 2.40 picks, and the ROM carries Ps2EeAs's (FACT #8544/#8547/#8554). A
# `dli` line is replaced by that row's words, spelled as mnemonics, only when all
# three of these equal a row:
#   - the region (-v region);
#   - the enclosing function, from cc1's `.ent`;
#   - the operands, written exactly as cc1 prints them.
# Every other line, including every other `dli`, is printed unchanged, CR and
# all. The pass is keyed by site on purpose: as a tree-wide rule Ps2EeAs is wrong
# in at least 11 engine chains per region (FACT #8518).
#
# The allowlist is read in BEGIN, from -v sites, and not as a first file
# argument. The two-file `NR == FNR` idiom took a 0-byte allowlist's place for
# the unit itself: every unit line was read as an allowlist row, nothing was
# printed, and `as` wrote an empty object at rc 0 (FACT #8640). An allowlist
# that cannot be read, or holds no valid row, exits 4 before any unit line is
# read.
#
# The words are decoded back to mnemonics (addiu, ori, lui, dsll, dsll32) rather
# than emitted as `.word`. Plain `ori`/`dsll`/`dsll32` lines are what the t1077
# probe substituted and measured byte-identical (NOTE #8543). As instructions,
# GNU as keeps its normal hazard and reorder bookkeeping around them - GNU's
# bookkeeping, which is not Ps2EeAs's (see ADJACENCY below). A `.word`, or a
# `.set noreorder` bracket, is a directive boundary, where GNU as can pad
# differently (see the mfc1 rule in asm_unit.sh). A word outside that opcode set
# cannot be spelled, and neither can a malformed or duplicate row. Either one
# emits an `.error`, so the unit fails to assemble. It is never silently skipped.
#
# ADJACENCY REFUSAL (RULING #8549 rev 3, FACT #8623). When the first instruction
# after a listed dli is a non-likely branch in reorder mode, GNU as moves the
# expansion's last word into the branch delay slot; Ps2EeAs keeps the order and
# puts a nop in the slot. The expansion is then not Ps2EeAs's, so the pass
# refuses: it prints `asm_unit.sh: FAIL:` naming the site on stderr and exits 3,
# and asm_unit.sh removes the object and exits 2. Blank lines, comments (cc1's
# empty #APP/#NO_APP block is the measured case, ledger-29240) and directives
# other than `.set [no]reorder` do not end the adjacency; a label does, since
# GNU as never swaps an instruction across a branch target. Measured on GNU as
# 2.40 through asm_unit.sh at -G0 and -G8, one listed literal (FACT #8623,
# FACT #8653, task #1147): every non-likely form swaps - j, jal, jalr, jr, b,
# beq, bne, beqz, bnez, blez, bgez, bgtz, bltz, bgezal, bltzal, bc0f, bc0t,
# bc1f, bc1t - and is refused. No branch-LIKELY form swaps (GNU never fills an
# annulled slot from before the branch): beql, bnel, beqzl, bnezl, blezl,
# bgezl, bgtzl, bltzl, bgezall, bltzall, bc0fl, bc0tl, bc1fl, bc1tl assemble in
# the dli's order with a nop in the slot, which is the order Ps2EeAs keeps:
# measured on Ps2EeAs.exe itself for bnel, beql and bc1fl (FACT #8688); the
# other 11 likely forms are inferred from FACT #8623, and none is ROM-measured at
# a likely branch. They are not refused. A branch mnemonic outside both lists is
# refused, since it is unmeasured. Only a dli emitted in REORDER mode arms the
# refusal: GNU as never moves an instruction emitted under `.set noreorder` into
# a later delay slot (FACT #8672: the second `jal`'s slot stays a nop).
#
# NOMACRO REFUSAL (task #1170). A listed dli under `.set nomacro` is refused
# (exit 3, its own `asm_unit.sh: FAIL:` line). cc1 brackets every delay-slot
# fill in `.set noreorder` + `.set nomacro` (FACT #8698), and there Ps2EeAs.exe
# rejects a multi-word dli (`error: Macro expansion is disabled`, rc 3), while
# GNU as expands it with only its first word in the slot and a warning. Every
# listed dli is multi-word: a 1-word dli has the same words on both assemblers,
# so it is never a row (FACT #8698). Substituting the row's words would give an
# object for an input the ROM's assembler, if it is Ps2EeAs, does not assemble,
# and would silence GNU's warning. cc1 2.9 was not seen to emit this (0 of 8
# probe contexts, FACT #8698). A listed dli under `.set noreorder` WITHOUT
# nomacro is substituted: Ps2EeAs assembles it, a delay slot taking only the
# first word, and its words equal this pass's for a `jal` slot followed by an
# addu, a reorder `jal` or a reorder `bnel`, and for a non-slot noreorder block
# (task #1170, Ps2EeAs 1.9.25.758, one literal).
#
# Only a TAB-laid `\tdli\t` line matches, which is cc1's layout. Splat's asm
# carries no `dli` at all (it prints the expanded words), so INCLUDE_ASM code
# never reaches this pass.

function hexval(h,    i, c, v) {
  v = 0
  for (i = 1; i <= length(h); i++) {
    c = index("0123456789abcdef", substr(h, i, 1))
    if (c == 0) return -1
    v = v * 16 + c - 1
  }
  return v
}

# cc1 prints `$12,0x4400000000008001`. Lower-case the operands and drop leading
# zero digits, so a row may pad the value without missing the line.
function normops(s) {
  s = tolower(s)
  sub(/,0x0+/, ",0x", s)
  if (s ~ /,0x$/) s = s "0"
  return s
}

# One Ps2EeAs expansion word back to its mnemonic, or "" when outside the set.
function decode(w,    op, rs, rt, rd, sa, fn) {
  op = int(w / 67108864); rs = int(w / 2097152) % 32; rt = int(w / 65536) % 32
  rd = int(w / 2048) % 32; sa = int(w / 64) % 32; fn = w % 64
  if (op == 9) return sprintf("addiu\t$%d,$%d,%d", rt, rs, (w % 65536 >= 32768) ? w % 65536 - 65536 : w % 65536)
  if (op == 13) return sprintf("ori\t$%d,$%d,0x%x", rt, rs, w % 65536)
  if (op == 15 && rs == 0) return sprintf("lui\t$%d,0x%x", rt, w % 65536)
  if (op == 0 && rs == 0 && fn == 56) return sprintf("dsll\t$%d,$%d,%d", rd, rt, sa)
  if (op == 0 && rs == 0 && fn == 60) return sprintf("dsll32\t$%d,$%d,%d", rd, rt, sa)
  return ""
}

# Pass 1, in BEGIN: the allowlist. Every row is validated, whatever its region,
# so a typo in a row for the other region still fails loudly.
BEGIN {
  if (sites == "") {
    print "asm_unit.sh: FAIL: ps2eeas_dli.awk: no allowlist given (-v sites=FILE, RULING #8549)" | "cat 1>&2"
    exit 4
  }
  nvalid = 0; nusa = 0; neu = 0; nbad = 0; lno = 0
  while ((st = (getline row < sites)) > 0) {
    lno++
    sub(/\r$/, "", row); sub(/#.*/, "", row)
    n = split(row, f)
    if (n == 0) continue
    ok = (n >= 6 && f[1] ~ /^(usa|eu)$/ && f[2] ~ /^[A-Za-z_][A-Za-z0-9_]*$/ \
          && f[3] ~ /^0x[0-9A-Fa-f]+$/ && f[4] ~ /^\$[0-9]+,0x[0-9a-f]+$/)
    words = ""
    for (i = 5; ok && i <= n; i++) {
      m = ""
      if (length(f[i]) == 8 && f[i] ~ /^[0-9a-f]+$/) m = decode(hexval(f[i]))
      if (m == "") ok = 0
      words = words "\t" m "\n"
    }
    key = f[1] SUBSEP f[2] SUBSEP normops(f[4])
    if (ok && (key in sites_)) ok = 0
    if (!ok) { bad = bad " " lno; nbad++; continue }
    sites_[key] = words
    addr[key] = f[3]
    nvalid++; if (f[1] == "usa") nusa++; else neu++
  }
  if (st < 0) {
    print "asm_unit.sh: FAIL: ps2eeas_dli.awk: cannot read the allowlist " sites " (RULING #8549)" | "cat 1>&2"
    exit 4
  }
  close(sites)
  if (count) { print nvalid, nusa, neu, nbad; exit 0 }
  if (nvalid == 0) {
    printf "asm_unit.sh: FAIL: ps2eeas_dli.awk: the allowlist %s has 0 valid rows (%d malformed, duplicate or unspellable; RULING #8549, FACT #8640)\n", sites, nbad | "cat 1>&2"
    exit 4
  }
  # The measured branch classes (see ADJACENCY above).
  swaps = "^(j|jal|jalr|jr|b|beq|bne|beqz|bnez|blez|bgez|bgtz|bltz|bgezal|bltzal|bc0f|bc0t|bc1f|bc1t)$"
  likely = "^(beql|bnel|beqzl|bnezl|blezl|bgezl|bgtzl|bltzl|bgezall|bltzall|bc0fl|bc0tl|bc1fl|bc1tl)$"
}

# Pass 2: the unit.
!started {
  started = 1
  if (bad != "")
    print "\t.error \"ps2eeas_dli_sites.txt: malformed, duplicate or unspellable row at line" bad " (RULING #8549)\""
}
{
  line = $0; sub(/\r$/, "", line)
  if (line ~ /^[ \t]*\.set[ \t]+noreorder([ \t#]|$)/) nore = 1
  else if (line ~ /^[ \t]*\.set[ \t]+reorder([ \t#]|$)/) nore = 0
  else if (line ~ /^[ \t]*\.set[ \t]+nomacro([ \t#]|$)/) nomac = 1
  else if (line ~ /^[ \t]*\.set[ \t]+macro([ \t#]|$)/) nomac = 0
  # ADJACENCY (see header): `held` is the listed site printed last, until the
  # next instruction or label shows whether a reorder-mode branch follows it.
  if (held != "") {
    if (line ~ /^[ \t]*[A-Za-z0-9_$.]+:/) held = ""
    else if (line ~ /^[ \t]*[a-z][a-z0-9.]*([ \t]|$)/) {
      mn = line; sub(/^[ \t]+/, "", mn); sub(/[ \t].*$/, "", mn)
      if (!nore && mn != "break" && mn !~ likely && mn ~ /^(j|jal|jalr|jr|b[a-z0-9]*)$/) {
        br = line; sub(/^[ \t]+/, "", br); gsub(/\t/, " ", br)
        if (mn ~ swaps)
          printf "asm_unit.sh: FAIL: %s, directly before the reorder-mode branch `%s`: GNU as moves the expansion's last word into the delay slot, where Ps2EeAs keeps the order and pads a nop (FACT #8623, FACT #8653, RULING #8549 rev 3)\n", held, br | "cat 1>&2"
        else
          printf "asm_unit.sh: FAIL: %s, directly before the reorder-mode branch `%s`, a mnemonic whose delay-slot placement by GNU as is not measured: refused rather than trusted (FACT #8623, RULING #8549 rev 3)\n", held, br | "cat 1>&2"
        refused++
      }
      held = ""
    }
  }
  if (line ~ /^[ \t]*\.ent[ \t]/) {
    fn = line; sub(/^[ \t]*\.ent[ \t]+/, "", fn); sub(/[ \t,].*$/, "", fn)
  } else if (line ~ /^[ \t]*\.end[ \t]/) {
    fn = ""
  } else if (fn != "" && line ~ /^\tdli\t/) {
    ops = line; sub(/^\tdli\t/, "", ops); sub(/[ \t#].*$/, "", ops)
    key = region SUBSEP fn SUBSEP normops(ops)
    if (key in sites_ && nomac) {
      printf "asm_unit.sh: FAIL: listed dli %s in %s (ROM %s, line %d) is under `.set nomacro`, where Ps2EeAs rejects a multi-word dli (`Macro expansion is disabled`) and GNU as would put only the expansion's first word in a delay slot; this pass does not substitute it (FACT #8698, task #1170, RULING #8549)\n", ops, fn, addr[key], FNR | "cat 1>&2"
      refused++
    } else if (key in sites_) {
      printf "\t# ps2eeas_dli_sites.txt %s: dli %s (ROM %s, RULING #8549)\n", fn, ops, addr[key]
      printf "%s", sites_[key]
      # only a reorder-mode dli can lose a word to a later slot (see ADJACENCY)
      if (!nore) held = "listed dli " ops " in " fn " (ROM " addr[key] ", line " FNR ")"
      next
    }
  }
  print
}

# The whole unit is still printed (a nomacro-refused dli as cc1 wrote it), so
# `as` reads a complete file and its own diagnostics stay meaningful; the
# non-zero status is what fails the unit.
END {
  if (refused) { close("cat 1>&2"); exit 3 }
}

# ps2eeas_dli.awk: RULING #8549's scoped Ps2EeAs `dli` expansion. It is a
# pre-assembly pass over one cc1 unit .s, run by tools/ee/asm_unit.sh:
#
#   awk -v region=usa -f tools/ee/ps2eeas_dli.awk tools/ee/ps2eeas_dli_sites.txt -
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
# after a listed dli is a branch in reorder mode, GNU as moves the expansion's
# last word into the branch delay slot; Ps2EeAs keeps the order and puts a nop
# in the slot. The expansion is then not Ps2EeAs's, so the pass refuses: it
# prints `asm_unit.sh: FAIL:` naming the site on stderr and exits 3, and
# asm_unit.sh removes the object and exits 2. Blank lines, comments (cc1's
# empty #APP/#NO_APP block is the measured case, ledger-29240) and directives
# other than `.set [no]reorder` do not end the adjacency; a label does, since
# GNU as never swaps an instruction across a branch target. Every branch
# mnemonic counts, likely ones included, although only `j $31` and `bne` were
# measured to swap: an unmeasured branch is refused rather than trusted. Not
# covered: a listed dli inside a `.set noreorder` region (e.g. in a delay slot).
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

# Pass 1: the allowlist. Every row is validated, whatever its region, so a typo
# in a row for the other region still fails loudly.
NR == FNR {
  row = $0; sub(/\r$/, "", row); sub(/#.*/, "", row)
  n = split(row, f)
  if (n == 0) next
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
  if (ok && (key in sites)) ok = 0
  if (!ok) { bad = bad " " FNR; next }
  sites[key] = words
  addr[key] = f[3]
  next
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
  # ADJACENCY (see header): `held` is the listed site printed last, until the
  # next instruction or label shows whether a reorder-mode branch follows it.
  if (held != "") {
    if (line ~ /^[ \t]*[A-Za-z0-9_$.]+:/) held = ""
    else if (line ~ /^[ \t]*[a-z][a-z0-9.]*([ \t]|$)/) {
      mn = line; sub(/^[ \t]+/, "", mn); sub(/[ \t].*$/, "", mn)
      if (!nore && mn != "break" && mn ~ /^(j|jal|jalr|jr|b[a-z0-9]*)$/) {
        br = line; sub(/^[ \t]+/, "", br); gsub(/\t/, " ", br)
        printf "asm_unit.sh: FAIL: %s, directly before the reorder-mode branch `%s`: GNU as would move the expansion's last word into the delay slot, where Ps2EeAs keeps the order and pads a nop (FACT #8623, RULING #8549 rev 3)\n", held, br | "cat 1>&2"
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
    if (key in sites) {
      printf "\t# ps2eeas_dli_sites.txt %s: dli %s (ROM %s, RULING #8549)\n", fn, ops, addr[key]
      printf "%s", sites[key]
      held = "listed dli " ops " in " fn " (ROM " addr[key] ", line " FNR ")"
      next
    }
  }
  print
}

# The whole unit is still printed, so `as` reads a complete file and its own
# diagnostics stay meaningful; the non-zero status is what fails the unit.
END {
  if (refused) { close("cat 1>&2"); exit 3 }
}

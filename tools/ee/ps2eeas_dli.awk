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
# GNU as keeps its normal hazard and reorder bookkeeping around them. A `.word`,
# or a `.set noreorder` bracket, is a directive boundary, where GNU as can pad
# differently (see the mfc1 rule in asm_unit.sh). A word outside that opcode set
# cannot be spelled, and neither can a malformed or duplicate row. Either one
# emits an `.error`, so the unit fails to assemble. It is never silently skipped.
# Not measured: a listed dli directly before a reorder-mode branch. None of the
# current sites has one, and a promotion's own byte gate would show a difference.
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
      next
    }
  }
  print
}

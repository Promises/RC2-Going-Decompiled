#!/bin/bash
# LEVEL GATE: is one USA level program, built from its COMMITTED split asm,
# byte-equal to the disc? (task #2458, NOTE #13098 row L5)
#
#   bash tools/levels/level_gate.sh <NN> [--expect <programs.tsv>]
#   bash tools/levels/level_gate.sh --selftest <NN>
#
# The last line of a real run is `#### level_gate LNN: PASS|FAIL`; the wall time
# (host, docker context) follows it. Exit 0 PASS, 1 FAIL, 2 usage/prerequisite.
# NOT called by tools/ee/landing_gate.sh (the boot gate): levels are a separate
# ledger, and nothing under tools/ee/ reads this file.
#
# Rows (each prints ok/FAIL; any FAIL fails the gate):
#   TREE     the level's committed inputs (going-decompiled/levels/usa/LNN,
#            going-decompiled/levels/usa/include, tools/levels) have no
#            uncommitted change, so the PASS is about HEAD's asm. Prints HEAD.
#   INPUT    extracted/usa_levels/NN_*/: each section .bin's sha1 equals the
#            manifest's, and the manifest's program sha1 equals LNN.yaml's
#            header (the extractor checked the manifest against the disc).
#   BUILD    tools/levels/level_build.sh NN (as -> gen_ld --check -> undef ->
#            ld -> level_elf check -> cmp(1)); its log, LNN.elf and blobs must
#            have an mtime from THIS run, and its own verdict must be PASS.
#   C        only with config/LNN_c.tsv (task #2495): one `C-PLACED ok` build
#            row per table row (the map puts the C object's .text at the
#            function's address and size; its glabel is in no assembled asm).
#   FRESH    only with config/LNN_boot_c.tsv (task #2516, RULING #13219): runs
#            tools/levels/boot_objects.sh first, which requires the boot
#            build's built-tree record (tools/ee/.gate_landing/usa/built_tree.txt)
#            to equal this worktree's tree, running the boot build (the landing
#            gate's building form) if it does not, then builds or reuses the
#            sectioned boot objects; its verdict must be PASS.
#   BOOTC    only with config/LNN_boot_c.tsv: one `PLACED ok` build row per
#            table row (map address = place = names-map value, map size =
#            catalogue size, alias present, glabel func_LNN_<place> in no
#            assembled asm), no `PLACED FAIL` and no `PLACED-DIFFERS` row,
#            `bootcmp: N of N` (the same sections linked at their BOOT addresses
#            equal the boot build's ELF), and the committed name map up to date
#            (gen_boot_names.py names --check). Prints every PLACED row, every
#            generator REFUSED member and the `ledger LNN:` line.
#   SECTIONS exactly 7 `check: cmp` rows, every one `0 differing` and
#            `dest ok size ok type ok`; exactly 7 `cmp(1)` rows, every one 0.
#   ENTRY    `check: entry ... OK`, and the ELF's e_entry equals the manifest's.
#   SHA1     the repacked archive-form sha1 equals LNN.yaml's header AND the
#            manifest's program_sha1 (and the --expect TSV row when given);
#            plus a third instrument on the host: sha1 of every built blob ==
#            the manifest's per-section sha1.
#   ANCHOR   MANDATORY external anchor: the repacked sha1/size/entry equals
#            this level's row of the COMMITTED tools/levels/expected_programs.tsv
#            (tools/levels/disc_programs.py: the raw program range read off the
#            disc by a reader sharing no code with extract_levels.py, cross-
#            checked 27/27 against OpenRAC programs.tsv us2). Every comparison
#            above is a round trip through extract_levels.py's blobs, so a level
#            with no table row FAILs as UNANCHORED (validator #2490 addendum).
#   DISC     when source-isos/ resolves: disc_programs.py re-reads the program's
#            raw range from the ISO now and it must equal the table row.
#   CONTROL  the negative control, in the boot gate's style and in the same
#            run: a copy of the built .text blob with 4 bytes flipped (XOR 0xFF
#            at offset 4096) must `cmp -l` against the disc's .text as > 0 (4 on
#            a byte-equal build), as the boot gate's ROW control does.
#
# --selftest NN: copies the level's inputs (tools/levels, the boot's read-only
# tools/ee/vu0_fixup.sed, going-decompiled/levels/usa/{include,LNN},
# extracted/usa_levels/NN_*) into a scratch root under the gitignored
# going-decompiled/build/usa_levels/level_gate_selftest/ and runs the gate there
# once per arm: UNSEEDED (must PASS: the copy is a valid tree) and SEEDED, with the
# program's entry instruction replaced by `.word <its value XOR 1>` (must FAIL,
# on .text only, first differing byte at the entry address); then, asm restored,
# with the level's expected_programs.tsv sha1 changed in its last hex digit (must
# FAIL on ANCHOR and nothing else) and with the row deleted (must FAIL as
# UNANCHORED). For a level with C rows (config/LNN_c.tsv, task #2495), two more
# arms on the first row: SEEDED C, the first decimal literal >= 1 in the
# function body incremented (must FAIL on .text only, first difference inside
# the function's cut block), and C ARM OFF, the table removed and LNN.ld
# regenerated so the function is assembled from its asm again (must PASS, with
# a built .text sha1 equal to the unseeded C-on run's). The C compile needs
# tools/ee/{ee_cc1.sh,move_fixup.sed,cc}, copied read-only into the scratch
# root. For a level with boot rows (config/LNN_boot_c.tsv, task #2516), the
# boot objects (going-decompiled/build/usa_levels/boot_objs) and the boot
# build's SCUS_972.68.elf/.map are copied in too, and three more arms run:
# SEEDED MAP, g_mobyClassHeaders +4 in LNN_boot_names.txt (must FAIL, on .text
# only, with PLACED-DIFFERS members EXACTLY `gen_boot_names.py users` of that
# name, read from the objects' relocations); NO ALIGNMENT, every .text.<fn>
# section of the copied boot objects set back to as's 16 (must FAIL with
# MISPLACED rows); PLACEMENTS OFF, LNN_boot_c.tsv removed and LNN.ld
# regenerated (must PASS, with a built .text sha1 equal to arm 1's). Then, for
# every level, arm 10 ASSEMBLER COLUMN (task #2552, RULING #13291 section 1),
# on func_L00_0042B6A8-class code (FACT #13288/#13289: cc1 prints `la $2,SYM`
# for a small datum at -G8; GNU as expands it to the 64-bit `daddiu`, SN's
# assembler to `addiu`). It finds the level's copy by its SHAPE in the split
# .text asm (the first 8-instruction block `addiu $sp,-0x10; addiu $v0,$gp,
# %gp_rel(D); sd $ra; jal F; sw $v0,0($a0); ld $ra; jr $ra; addiu $sp`), writes
# a fixture TU of our own for it (src/selftest_la_fixture.c in the scratch copy
# only: `*obj = &D; F(obj);`, never a real row), and builds the level twice
# with that one row added at s136 -G8: assembler `as`, then `asm_unit`. PASS
# only if the `asm_unit` build PASSes (the ROM's bytes), the `as` build FAILs
# on .text only with its first difference inside the fixture's block (not the
# ROM's bytes), and the two fixture objects' .text differ. Seeded in the same
# run: the same verdict computed with the `asm_unit` object compared with
# ITSELF must FAIL. The asm_unit route needs tools/ee/{asm_unit.sh,
# ps2eeas_dli.awk,ps2eeas_dli_sites.txt}, copied read-only into the scratch
# root. The tracked tree is never edited. Scratch-run lines are indented with `  [unseeded]`/`  [seeded]`,
# so no scratch verdict can be mistaken for a real one. Last line:
# `#### level_gate LNN SELFTEST: PASS|FAIL`.
#
# Paths: reads going-decompiled/levels/, extracted/usa_levels/, tools/levels/
# and, read-only, tools/ee/vu0_fixup.sed, plus for C rows tools/ee/ee_cc1.sh,
# tools/ee/move_fixup.sed and tools/ee/cc, and for `asm_unit` rows (and arm 10)
# tools/ee/asm_unit.sh, ps2eeas_dli.awk and ps2eeas_dli_sites.txt; writes only
# going-decompiled/build/usa_levels/.
set -u

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
CTX=${EE_DOCKER_CONTEXT:-colima-ee-x86}
ISO="$ROOT/source-isos/Ratchet & Clank - Going Commando (USA) (v2.00).iso"

usage() { echo "usage: $0 <NN> [--expect <programs.tsv>] | $0 --selftest <NN>" >&2; exit 2; }

now() { date +%s.%N; }
secs() { awk -v a="$1" -v b="$2" 'BEGIN{printf "%.2f", b-a}'; }
sha1() { sha1sum "$1" | cut -c1-40; }
# manifest field reader (python3 -I: the manifest is untrusted data, read as JSON only)
man() { python3 -I -c 'import json,sys
m=json.load(open(sys.argv[1]))
a=sys.argv[2]
if a=="sections":
    for s in m["sections"]: print(s["file"], s["sha1"])
else: print(m[a])' "$1" "$2"; }

# gate_core <root> <NN> <check_tree 0|1> [<expect tsv>]
# Runs every row against the tree at <root>; prints rows and the verdict line;
# returns 0 PASS / 1 FAIL / 2 prerequisite missing.
gate_core() {
  local R=$1 NN TAG LVL YAML EXT BUILD LOG T0 ok=1 rows=""
  NN=$(printf '%02d' "$((10#$2))"); local CT=$3 EXPECT=${4:-}
  TAG=L$NN; LVL=going-decompiled/levels/usa/$TAG; YAML=$R/$LVL/config/$TAG.yaml
  BUILD=$R/going-decompiled/build/usa_levels/$TAG; LOG=$BUILD/level_gate_build.log
  EXT=$(ls -d "$R"/extracted/usa_levels/${NN}_* 2>/dev/null | head -1)
  pass() { echo "  ok    $1"; }
  fail() { echo "  FAIL  $1"; ok=0; }
  [ -f "$YAML" ] || { echo "level_gate: no $LVL/config/$TAG.yaml"; echo "#### level_gate $TAG: FAIL (no level config)"; return 2; }
  [ -n "$EXT" ] && [ -f "$EXT/manifest.json" ] || { echo "level_gate: level $NN not extracted (python3 -I tools/levels/extract_levels.py extract --only $((10#$NN)))"; echo "#### level_gate $TAG: FAIL (not extracted)"; return 2; }

  echo "== level_gate $TAG: root $R"
  # TREE
  if [ "$CT" = 1 ]; then
    local dirty head
    head=$(git -C "$R" rev-parse --short=9 HEAD)
    dirty=$(git -C "$R" status --porcelain -- "$LVL" going-decompiled/levels/usa/include tools/levels)
    if [ -z "$dirty" ]; then pass "TREE: HEAD $head, level inputs clean ($LVL, going-decompiled/levels/usa/include, tools/levels)"
    else fail "TREE: HEAD $head, uncommitted level inputs: $(printf '%s' "$dirty" | tr '\n' ';')"; fi
  else
    echo "  --    TREE: not checked (selftest scratch root, not a git tree)"
  fi

  # INPUT
  local f want got bad=""
  while read -r f want; do
    got=$(sha1 "$EXT/$f"); [ "$got" = "$want" ] || bad="$bad $f($got!=$want)"
  done <<EOF
$(man "$EXT/manifest.json" sections)
EOF
  local msha ysha mentry
  msha=$(man "$EXT/manifest.json" program_sha1); mentry=$(man "$EXT/manifest.json" entry)
  ysha=$(sed -n 's/.*program sha1 \([0-9a-f]\{40\}\).*/\1/p' "$YAML" | head -1)
  [ -z "$bad" ] && pass "INPUT: 7 extracted section sha1 == manifest ($(basename "$EXT"))" || fail "INPUT: extracted section sha1 != manifest:$bad"
  [ "$msha" = "$ysha" ] && pass "INPUT: manifest program sha1 $msha == $TAG.yaml header" || fail "INPUT: manifest program sha1 $msha != $TAG.yaml header $ysha"

  # FRESH (task #2516): the sectioned boot objects the boot rows link, for this tree
  local btsv=$R/$LVL/config/${TAG}_boot_c.tsv
  if [ -f "$btsv" ]; then
    if [ "$CT" = 1 ]; then
      local bol=$R/going-decompiled/build/usa_levels/boot_objects_gate.log
      mkdir -p "$(dirname "$bol")"
      bash "$R/tools/levels/boot_objects.sh" > "$bol" 2>&1
      local bov; bov=$(grep_a '^#### boot_objects: ' "$bol")
      case "$bov" in "#### boot_objects: PASS"*) pass "FRESH: ${bov#\#\#\#\# }";; *) fail "FRESH: '${bov:-no verdict}' (log $bol)";; esac
      { /usr/bin/grep -a -E '^boot_objects: (FRESH|STALE|REUSED|wall|boot build)' "$bol" || true; } | sed 's/^/        /'
    else
      echo "  --    FRESH: not checked (selftest scratch root: boot objects copied from the real tree)"
    fi
  fi

  # BUILD
  mkdir -p "$BUILD"
  local stamp=$BUILD/.level_gate_stamp; : > "$stamp"
  local brc=0
  if [ -n "$EXPECT" ]; then "$R/tools/levels/level_build.sh" "$((10#$NN))" --expect "$EXPECT" > "$LOG" 2>&1 || brc=$?
  else "$R/tools/levels/level_build.sh" "$((10#$NN))" > "$LOG" 2>&1 || brc=$?; fi
  local fresh=1 x
  for x in "$LOG" "$BUILD/$TAG.elf" "$BUILD/blobs/text.bin"; do
    if [ ! -e "$x" ] || [ ! "$x" -nt "$stamp" ]; then fresh=0; fi
  done
  local bline; bline=$(grep_a "^#### level_build $TAG: " "$LOG")
  [ "$bline" = "#### level_build $TAG: PASS" ] && [ $brc = 0 ] && pass "BUILD: level_build.sh rc $brc, '$bline'" || fail "BUILD: level_build.sh rc $brc, '${bline:-no verdict line}' (log $LOG)"
  [ $fresh = 1 ] && pass "BUILD: log, $TAG.elf and blobs/text.bin written by THIS run" || fail "BUILD: log/$TAG.elf/blobs/text.bin missing or older than this run (stale artefact)"
  grep_a '^level_build: wall ' "$LOG" | sed 's/^/        /'

  # C (task #2495): every LNN_c.tsv row linked from its C object at its place,
  # with its glabel in none of the assembled asm (level_build's C-PLACED rows)
  local ctsv=$R/$LVL/config/${TAG}_c.tsv nrow ncp ncf
  if [ -f "$ctsv" ]; then
    nrow=$(awk -F'\t' '!/^#/ && (NF==4 || NF==6)' "$ctsv" | wc -l | tr -d ' ')
    ncp=$(grep_c '^C-PLACED ok ' "$LOG"); ncf=$(grep_c '^C-PLACED FAIL ' "$LOG")
    [ "$nrow" -gt 0 ] && [ "$ncp" = "$nrow" ] && [ "$ncf" = 0 ] \
      && pass "C: $ncp of $nrow ${TAG}_c.tsv functions linked from their C object at their place (map), none assembled from asm" \
      || fail "C: $ncp of $nrow ${TAG}_c.tsv functions placed from C, $ncf placement FAIL rows"
    grep_a '^C-PLACED ' "$LOG" | sed 's/^/        /'
    grep_a '^level progress ' "$LOG" | sed 's/^/        /'
  else
    echo "  --    C: no config/${TAG}_c.tsv; the level is built from asm only"
  fi

  # BOOTC (task #2516): every LNN_boot_c.tsv row placed from the boot object
  if [ -f "$btsv" ]; then
    local nb npl npf npd bcl upd
    nb=$(awk -F'\t' '!/^#/ && NF==4' "$btsv" | wc -l | tr -d ' ')
    npl=$(grep_c '^PLACED ok ' "$LOG"); npf=$(grep_c '^PLACED FAIL ' "$LOG"); npd=$(grep_c '^PLACED-DIFFERS ' "$LOG")
    bcl=$(grep_a '^bootcmp: [0-9]* of [0-9]* ' "$LOG"); upd=$(grep_c ' up to date$' "$LOG")
    [ "$nb" -gt 0 ] && [ "$npl" = "$nb" ] && [ "$npf" = 0 ] && [ "$npd" = 0 ] && [ "$upd" -ge 2 ] \
      && case "$bcl" in "bootcmp: $nb of $nb "*) true;; *) false;; esac \
      && pass "BOOTC: $npl of $nb ${TAG}_boot_c.tsv functions PLACED from the sectioned boot objects (0 FAIL, 0 differ); $bcl; name map up to date" \
      || fail "BOOTC: $npl PLACED ok / $npf FAIL / $npd differ of $nb rows; '${bcl:-no bootcmp row}'; up-to-date rows $upd (want 2: gen_ld and gen_boot_names)"
    { /usr/bin/grep -a -E '^(REFUSED |   REFUSED |   PLACED |   not ours)' "$LOG" || true; } | sed 's/^/        /'
    { /usr/bin/grep -a -E '^(PLACED FAIL|PLACED-DIFFERS|gen_boot_names: .* STALE|bootcmp: DIFFERS)' "$LOG" || true; } | sed 's/^/        /'
    grep_a '^PLACED ok ' "$LOG" | sed 's/^/        /'
  fi
  grep_a '^ledger ' "$LOG" | sed 's/^/        /'

  # SECTIONS
  local nck nck0 ncm ncm0
  nck=$(grep_c '^check: cmp ' "$LOG"); nck0=$(grep_c '^check: cmp .* 0 differing bytes of .*dest ok size ok type ok' "$LOG")
  ncm=$(grep_c '^cmp(1) ' "$LOG");     ncm0=$(grep_c '^cmp(1) [a-z]*: 0 differing' "$LOG")
  [ "$nck" = 7 ] && [ "$nck0" = 7 ] && pass "SECTIONS: level_elf 7 of 7 cmp 0 (dest/size/type ok)" || fail "SECTIONS: level_elf $nck0 of $nck cmp rows clean (need 7 of 7)"
  grep_a '^check: cmp ' "$LOG" | sed 's/^/        /'
  [ "$ncm" = 7 ] && [ "$ncm0" = 7 ] && pass "SECTIONS: cmp(1) 7 of 7 differ 0" || fail "SECTIONS: cmp(1) $ncm0 of $ncm rows 0 (need 7 of 7)"

  # ENTRY
  local eline eelf
  eline=$(grep_a '^check: entry ' "$LOG")
  eelf=$([ -f "$BUILD/$TAG.elf" ] && python3 -I -c 'import struct,sys;print("0x%08X"%struct.unpack_from("<I",open(sys.argv[1],"rb").read(),24)[0])' "$BUILD/$TAG.elf")
  case "$eline" in *" OK") okline=1;; *) okline=0;; esac
  [ $okline = 1 ] && [ "$eelf" = "$mentry" ] && pass "ENTRY: e_entry $eelf == manifest $mentry ('$eline')" || fail "ENTRY: e_entry ${eelf:-none} vs manifest $mentry ('${eline:-no entry row}')"

  # SHA1
  local rline rsha
  rline=$(grep_a '^check: repacked sha1 ' "$LOG")
  rsha=$(printf '%s' "$rline" | sed -n 's/^check: repacked sha1 \([0-9a-f]\{40\}\).*/\1/p')
  case "$rline" in *" EQUAL") okline=1;; *) okline=0;; esac
  [ $okline = 1 ] && [ "$rsha" = "$msha" ] && pass "SHA1: repacked $rsha == $TAG.yaml header == manifest program_sha1" || fail "SHA1: repacked '${rsha:-none}' vs manifest $msha ('${rline:-no repack row}')"
  if [ -n "$EXPECT" ]; then
    local xline; xline=$(grep_a '^check: expect ' "$LOG")
    case "$xline" in *" EQUAL") pass "SHA1: --expect row: ${xline#check: }";; *) fail "SHA1: --expect row: '${xline:-none}'";; esac
  fi
  bad=""
  while read -r f want; do
    [ -f "$BUILD/blobs/$f" ] && got=$(sha1 "$BUILD/blobs/$f") || got=missing
    [ "$got" = "$want" ] || bad="$bad $f($got)"
  done <<EOF
$(man "$EXT/manifest.json" sections)
EOF
  [ -z "$bad" ] && pass "SHA1: host sha1 of 7 of 7 built blobs == manifest section sha1" || fail "SHA1: built blob sha1 != manifest:$bad"

  # ANCHOR (mandatory, external): the committed disc table's row for this level
  # initialised: the DISC row reads them even when there is no table row (set -u)
  local tab=$R/tools/levels/expected_programs.tsv trow tsha="" tsize="" tentry="" rsize rentry
  rsize=$(printf '%s' "$rline" | sed -n 's/.* size \([0-9]*\) entry .*/\1/p')
  rentry=$(printf '%s' "$rline" | sed -n 's/.* entry \([0-9A-F]\{8\}\);.*/\1/p')
  trow=$([ -f "$tab" ] && awk -F'\t' -v n="$((10#$NN))" '$1==n' "$tab")
  if [ -z "$trow" ] || [ "$(printf '%s\n' "$trow" | wc -l | tr -d ' ')" != 1 ]; then
    fail "ANCHOR: UNANCHORED: no single row for level $((10#$NN)) in tools/levels/expected_programs.tsv (the manifest/repack comparison alone is a round trip)"
  else
    tsha=$(printf '%s' "$trow" | cut -f4); tsize=$(printf '%s' "$trow" | cut -f5); tentry=$(printf '%s' "$trow" | cut -f6)
    [ "$rsha $rsize $rentry" = "$tsha $tsize $tentry" ] && pass "ANCHOR: repacked $rsha $rsize $rentry == expected_programs.tsv row (disc-derived, OpenRAC-crosschecked)" \
      || fail "ANCHOR: repacked '${rsha:-none} ${rsize:-?} ${rentry:-?}' != expected_programs.tsv row '$tsha $tsize $tentry'"
  fi
  # DISC: the same raw slot-0 range read off the ISO now, when the symlink resolves
  if [ -r "$ISO" ]; then
    local drow; drow=$(python3 -I "$ROOT/tools/levels/disc_programs.py" read --iso "$ISO" --level "$((10#$NN))" 2>&1)
    [ "$(printf '%s' "$drow" | cut -f3-5)" = "$(printf '%s\t%s\t%s' "$tsha" "$tsize" "$tentry")" ] && [ -n "$tsha" ] \
      && pass "DISC: raw read of $(printf '%s' "$drow" | cut -f2) on the ISO == expected_programs.tsv row" \
      || fail "DISC: raw ISO read '$(printf '%s' "$drow" | tr '\t' ' ')' != expected_programs.tsv row '${tsha:-none} ${tsize:-} ${tentry:-}'"
  else
    echo "  --    DISC: ISO not readable at $ISO; the anchor is the committed table alone"
  fi

  # CONTROL (boot gate style: a flipped copy must cmp != 0, observed in this run)
  local flip=$BUILD/level_gate_flipped_text.bin cf
  if [ -f "$BUILD/blobs/text.bin" ]; then
    python3 -I -c 'import sys
b=bytearray(open(sys.argv[1],"rb").read())
for i in range(4096,4100): b[i]^=0xFF
open(sys.argv[2],"wb").write(b)' "$BUILD/blobs/text.bin" "$flip"
    cf=$(cmp -l "$flip" "$EXT/text.bin" 2>/dev/null | wc -l | tr -d ' ')
    rm -f "$flip"
  fi
  [ "${cf:-0}" -gt 0 ] 2>/dev/null && pass "CONTROL: negative control: 4 bytes flipped in a copy of built .text -> cmp $cf" || fail "CONTROL: negative control did not fire: flipped copy cmp '${cf:-}' (the cmp check cannot fail)"

  if [ $ok = 1 ]; then echo "#### level_gate $TAG: PASS"; return 0; fi
  echo "#### level_gate $TAG: FAIL"; return 1
}

# grep without the harness's shims; a zero count is a value, never an exit status
grep_a() { /usr/bin/grep -a -- "$1" "$2" 2>/dev/null || true; }
grep_c() { local n; n=$(/usr/bin/grep -a -c -- "$1" "$2" 2>/dev/null || true); echo "${n:-0}"; }

# sha1 of an ELF object's .text section bytes (host python, no ELF library); empty if none
textsha() { [ -f "$1" ] && python3 -I -c 'import hashlib,struct,sys
b=open(sys.argv[1],"rb").read()
sho,=struct.unpack_from("<I",b,0x20); n,si=struct.unpack_from("<HH",b,0x30)
sh=[struct.unpack_from("<10I",b,sho+40*i) for i in range(n)]
nm=lambda s: b[sh[si][4]+s[0]:b.index(b"\0",sh[si][4]+s[0])].decode()
t=[s for s in sh if nm(s)==".text"]
print(hashlib.sha1(b[t[0][4]:t[0][4]+t[0][5]]).hexdigest() if t else "")' "$1"; }

footer() { echo "level_gate: wall $(secs "$1" "$(now)") s (host $(uname -n), docker context $CTX -> $(docker context inspect "$CTX" --format '{{.Endpoints.docker.Host}}' 2>/dev/null), rc $2)"; }

selftest() {
  local NN TAG S rc t0 ok=1 seedline file word addr ent out
  NN=$(printf '%02d' "$((10#$1))"); TAG=L$NN; t0=$(now)
  S=$ROOT/going-decompiled/build/usa_levels/level_gate_selftest
  rm -rf "$S"; mkdir -p "$S/tools/ee" "$S/going-decompiled/levels/usa" "$S/extracted/usa_levels"
  cp -a "$ROOT/tools/levels" "$S/tools/"
  cp -a "$ROOT/tools/ee/vu0_fixup.sed" "$S/tools/ee/"
  # the C rows' compile (task #2495): the boot's cpp/cc1 wrapper, fixup and compilers, read-only copies
  cp -a "$ROOT/tools/ee/ee_cc1.sh" "$ROOT/tools/ee/move_fixup.sed" "$ROOT/tools/ee/cc" "$S/tools/ee/"
  # the `asm_unit` assembler column (task #2552): the boot's SN-as model and the
  # two files it reads besides the fixups above, read-only copies
  cp -a "$ROOT/tools/ee/asm_unit.sh" "$ROOT/tools/ee/ps2eeas_dli.awk" "$ROOT/tools/ee/ps2eeas_dli_sites.txt" "$S/tools/ee/"
  cp -a "$ROOT/going-decompiled/levels/usa/include" "$ROOT/going-decompiled/levels/usa/$TAG" "$S/going-decompiled/levels/usa/"
  local E; E=$(ls -d "$ROOT"/extracted/usa_levels/${NN}_* 2>/dev/null | head -1)
  [ -n "$E" ] || { echo "level_gate: level $NN not extracted"; echo "#### level_gate $TAG SELFTEST: FAIL (not extracted)"; return 2; }
  cp -a "$E" "$S/extracted/usa_levels/"
  # boot rows (task #2516): the sectioned boot objects and the boot build's ELF/map, read-only copies
  local hasboot=0
  if [ -f "$ROOT/going-decompiled/levels/usa/$TAG/config/${TAG}_boot_c.tsv" ]; then
    hasboot=1
    mkdir -p "$S/going-decompiled/build/usa" "$S/going-decompiled/build/usa_levels"
    cp -a "$ROOT/going-decompiled/build/usa/SCUS_972.68.elf" "$ROOT/going-decompiled/build/usa/SCUS_972.68.map" "$S/going-decompiled/build/usa/"
    cp -a "$ROOT/going-decompiled/build/usa_levels/boot_objs" "$S/going-decompiled/build/usa_levels/"
  fi
  echo "== selftest $TAG: scratch root $S"

  echo "-- arm 1 UNSEEDED (must PASS: the scratch copy is a valid tree)"
  out=$(gate_core "$S" "$NN" 0); rc=$?
  printf '%s\n' "$out" | sed 's/^/  [unseeded] /'
  if [ $rc = 0 ]; then echo "  ok    arm 1: unseeded scratch gate rc 0 PASS"; else echo "  FAIL  arm 1: unseeded scratch gate rc $rc (a seeded FAIL would prove nothing)"; ok=0; fi
  local SB=$S/going-decompiled/build/usa_levels/$TAG text1
  text1=$([ -f "$SB/blobs/text.bin" ] && sha1 "$SB/blobs/text.bin")

  # seed: the program's entry instruction -> .word (value XOR 1)
  ent=$(man "$E/manifest.json" entry); addr=$(printf '%08X' "$ent")
  file=$(/usr/bin/grep -l -a -E "^ +/\\* [0-9A-F]+ $addr [0-9A-F]{8} \\*/" -r "$S/going-decompiled/levels/usa/$TAG/asm/text" | head -1)
  seedline=$(/usr/bin/grep -a -n -E "^ +/\\* [0-9A-F]+ $addr [0-9A-F]{8} \\*/" "$file" | head -1)
  [ -n "$seedline" ] || { echo "  FAIL  seed: no asm line at entry $addr"; echo "#### level_gate $TAG SELFTEST: FAIL (cannot seed)"; return 1; }
  local ln=${seedline%%:*} bytes
  bytes=$(printf '%s' "$seedline" | sed -E 's/^[0-9]+: +\/\* [0-9A-F]+ [0-9A-F]+ ([0-9A-F]{8}) \*\/.*/\1/')
  word=$(python3 -I -c 'import sys;print("0x%08X"%(int.from_bytes(bytes.fromhex(sys.argv[1]),"little")^1))' "$bytes")
  sed -i "${ln}s|\\*/.*|*/  .word $word|" "$file"
  echo "-- arm 2 SEEDED: ${file#$S/}:$ln (0x$addr, bytes $bytes) -> '$(sed -n "${ln}p" "$file" | sed 's/^ *//')' (must FAIL)"
  out=$(gate_core "$S" "$NN" 0); rc=$?
  printf '%s\n' "$out" | sed 's/^/  [seeded] /'
  local tl others
  tl=$(printf '%s\n' "$out" | /usr/bin/grep -a -E '^ +check: cmp text +\.text ' || true)
  others=$(printf '%s\n' "$out" | /usr/bin/grep -a -E '^ +check: cmp ' | /usr/bin/grep -a -v -E 'cmp text ' | /usr/bin/grep -a -c ' 0 differing bytes ' || true)
  if [ $rc = 1 ] && printf '%s\n' "$out" | /usr/bin/grep -q -a "^#### level_gate $TAG: FAIL$"; then echo "  ok    arm 2: seeded gate rc 1, verdict '#### level_gate $TAG: FAIL'"; else echo "  FAIL  arm 2: seeded gate rc $rc did not FAIL (the gate cannot fail)"; ok=0; fi
  case "$tl" in *" 1 differing bytes "*" first at 0x$addr "*) echo "  ok    arm 2: the FAIL names the seed: ${tl#"${tl%%check*}"}";; *) echo "  FAIL  arm 2: .text row does not show 1 byte at 0x$addr: '$tl'"; ok=0;; esac
  [ "${others:-0}" = 6 ] && echo "  ok    arm 2: the other 6 sections still cmp 0" || { echo "  FAIL  arm 2: $others of 6 other sections cmp 0"; ok=0; }

  # restore the asm word: arms 3/4 seed the ANCHOR only
  cp -a "$ROOT/going-decompiled/levels/usa/$TAG/asm/text/." "$S/going-decompiled/levels/usa/$TAG/asm/text/"
  local tab=$S/tools/levels/expected_programs.tsv
  cp "$ROOT/tools/levels/expected_programs.tsv" "$tab"
  awk -F'\t' -v OFS='\t' -v n="$((10#$NN))" '$1==n{ $4=substr($4,1,39) (substr($4,40,1)=="0"?"1":"0") }1' "$ROOT/tools/levels/expected_programs.tsv" > "$tab"
  echo "-- arm 3 SEEDED ANCHOR: asm restored; expected_programs.tsv L$NN sha1 -> $(awk -F'\t' -v n="$((10#$NN))" '$1==n{print $4}' "$tab") (must FAIL on ANCHOR)"
  out=$(gate_core "$S" "$NN" 0); rc=$?
  printf '%s\n' "$out" | sed 's/^/  [anchor] /'
  local an; an=$(printf '%s\n' "$out" | /usr/bin/grep -a -c -E '^  FAIL  ANCHOR: repacked ' || true)
  local nf; nf=$(printf '%s\n' "$out" | /usr/bin/grep -a -E '^  FAIL ' | /usr/bin/grep -a -v -E '^  FAIL  (ANCHOR|DISC): ' | /usr/bin/grep -a -c . || true)
  if [ $rc = 1 ] && [ "$an" = 1 ] && [ "$nf" = 0 ]; then echo "  ok    arm 3: wrong expected sha1 -> rc 1, ANCHOR FAIL, no row outside ANCHOR/DISC failed"; else echo "  FAIL  arm 3: rc $rc, ANCHOR FAIL rows $an, other FAIL rows $nf (want 1, 1, 0)"; ok=0; fi

  awk -F'\t' -v n="$((10#$NN))" '$1!=n' "$ROOT/tools/levels/expected_programs.tsv" > "$tab"
  echo "-- arm 4 UNANCHORED: expected_programs.tsv without a level $((10#$NN)) row (must FAIL)"
  out=$(gate_core "$S" "$NN" 0); rc=$?
  printf '%s\n' "$out" | sed 's/^/  [unanchored] /'
  an=$(printf '%s\n' "$out" | /usr/bin/grep -a -c -E '^  FAIL  ANCHOR: UNANCHORED' || true)
  if [ $rc = 1 ] && [ "$an" = 1 ]; then echo "  ok    arm 4: no table row -> rc 1, 'ANCHOR: UNANCHORED'"; else echo "  FAIL  arm 4: rc $rc, UNANCHORED rows $an (want 1, 1)"; ok=0; fi
  cp "$ROOT/tools/levels/expected_programs.tsv" "$tab"

  # arms 5-6 (task #2495), only for a level with C rows
  local ctsv=$S/going-decompiled/levels/usa/$TAG/config/${TAG}_c.tsv crow cfn csrc caddr csize cend seed summary="" t6
  crow=$([ -f "$ctsv" ] && awk -F'\t' '!/^#/ && (NF==4 || NF==6)' "$ctsv" | head -1)
  if [ -z "$crow" ]; then
    echo "  --    arms 5-6: no config/${TAG}_c.tsv row; the level has no C to seed"
  else
    # arm 5: a one-token change in the C body: the first decimal literal >= 1
    # after the function's opening line, incremented. Rows are tried in table
    # order until one has such a literal (FACT #13345: a first row with only
    # float literals left the arm with no seed); none at all FAILs explicitly.
    seed=""
    while IFS= read -r crow; do
      cfn=$(printf '%s' "$crow" | cut -f1); csrc=$S/going-decompiled/levels/usa/$TAG/$(printf '%s' "$crow" | cut -f2)
      caddr=$(printf '%s' "$cfn" | sed -E 's/.*_([0-9A-F]{8})$/\1/')
      seed=$(python3 -I -c 'import re,sys
p,fn=sys.argv[1],sys.argv[2]
L=open(p).read().split("\n")
s=next(i for i,l in enumerate(L) if re.match(r"^\S.*\b%s\s*\(.*\{\s*$"%fn,l))
for i in range(s+1,len(L)):
    m=re.search(r"(?<![\w.])([1-9][0-9]*)(?![\w.])",L[i])
    if m:
        old=L[i]; L[i]=L[i][:m.start()]+str(int(m.group(1))+1)+L[i][m.end():]
        open(p,"w").write("\n".join(L)); print("%d: %s -> %s"%(i+1,old.strip(),L[i].strip())); break' "$csrc" "$cfn")
      [ -n "$seed" ] && break
    done < <(awk -F'\t' '!/^#/ && (NF==4 || NF==6)' "$ctsv")
    [ -n "$seed" ] || echo "  FAIL  arm 5: cannot seed: no ${TAG}_c.tsv row's body has a decimal literal >= 1"
    echo "-- arm 5 SEEDED C: ${csrc#$S/}:$seed (must FAIL on .text inside $cfn)"
    out=$(gate_core "$S" "$NN" 0); rc=$?
    printf '%s\n' "$out" | sed 's/^/  [seeded C] /'
    tl=$(printf '%s\n' "$out" | /usr/bin/grep -a -E '^ +check: cmp text +\.text ' || true)
    others=$(printf '%s\n' "$out" | /usr/bin/grep -a -E '^ +check: cmp ' | /usr/bin/grep -a -v -E 'cmp text ' | /usr/bin/grep -a -c ' 0 differing bytes ' || true)
    local first; first=$(printf '%s' "$tl" | sed -n -E 's/.* first at 0x([0-9A-F]{8}).*/\1/p')
    csize=$(awk -v f="$cfn" '$1=="CFN" && $2==f {print $4}' "$SB/cut.txt")  # the cut block's size
    cend=$(printf '%08X' "$((0x$caddr + ${csize:-0}))")
    if [ $rc = 1 ] && [ -n "$seed" ] && [ -n "$first" ] && [ -n "$csize" ] && [ "$((0x$first >= 0x$caddr && 0x$first < 0x$cend))" = 1 ] && [ "${others:-0}" = 6 ]; then
      echo "  ok    arm 5: seeded C -> rc 1, FAIL on .text first at 0x$first, inside $cfn [0x$caddr, 0x$cend); the other 6 sections cmp 0: ${tl#"${tl%%check*}"}"
    else echo "  FAIL  arm 5: rc $rc, .text row '$tl', first '${first:-none}' (want inside [0x$caddr, 0x$cend)), other sections cmp 0: $others of 6"; ok=0; fi
    cp -a "$ROOT/going-decompiled/levels/usa/$TAG/." "$S/going-decompiled/levels/usa/$TAG/"
    # arm 6: the C arm off. No LNN_c.tsv and its linker script regenerated, so the
    # function is assembled from its asm again: must PASS, with a .text blob
    # identical to arm 1's (C on), so the C arm is equal by bytes, not vacuous
    rm -f "$ctsv"
    (cd "$S" && python3 -I tools/levels/gen_ld.py "going-decompiled/levels/usa/$TAG/config/$TAG.yaml" > /dev/null)
    echo "-- arm 6 C ARM OFF: ${ctsv#$S/} removed, $TAG.ld regenerated (must PASS, .text sha1 == arm 1's)"
    out=$(gate_core "$S" "$NN" 0); rc=$?
    printf '%s\n' "$out" | sed 's/^/  [C off] /'
    t6=$([ -f "$SB/blobs/text.bin" ] && sha1 "$SB/blobs/text.bin")
    local glab; glab=$(/usr/bin/grep -a -c "^glabel $cfn\$" "$S/going-decompiled/levels/usa/$TAG/asm/text/"*.s | awk -F: '{s+=$NF} END {print s+0}')
    if [ $rc = 0 ] && [ -n "$text1" ] && [ "$t6" = "$text1" ] && [ "$glab" = 1 ]; then
      echo "  ok    arm 6: C arm off -> rc 0 PASS from asm ($cfn glabel back in the assembled asm: $glab); built .text sha1 $t6 == arm 1 (C on) $text1"
    else echo "  FAIL  arm 6: rc $rc, .text sha1 '${t6:-none}' vs arm 1 '${text1:-none}', glabel $cfn in asm $glab"; ok=0; fi
    cp -a "$ROOT/going-decompiled/levels/usa/$TAG/." "$S/going-decompiled/levels/usa/$TAG/"
    summary="; seeded C FAIL on .text inside $cfn; C arm off PASS with identical .text"
  fi

  # arm 10 (task #2552): the assembler column changes bytes on la-class code
  local fx fxfn fxd fxf fxaddr fxend fxsrc=$S/going-decompiled/levels/usa/$TAG/src/selftest_la_fixture.c asr
  local -A fxrc fxfirst fxoth fxsha
  fx=$(python3 -I - "$S/going-decompiled/levels/usa/$TAG/asm/text" <<'PYEOF'
import os, re, sys
pat = [r"addiu \$sp, \$sp, -0x10", r"addiu \$v0, \$gp, %gp_rel\((D_\w+)\)", r"sd \$ra, 0x0\(\$sp\)",
       r"jal (func_\w+)", r"sw \$v0, 0x0\(\$a0\)", r"ld \$ra, 0x0\(\$sp\)", r"jr \$ra", r"addiu \$sp, \$sp, 0x10"]
for f in sorted(os.listdir(sys.argv[1])):
    fn = None; ins = []
    for l in open(os.path.join(sys.argv[1], f)):
        m = re.match(r"^glabel (func_L\d\d_[0-9A-F]{8})$", l)
        if m: fn, ins = m.group(1), []; continue
        m = re.match(r"^\s*/\* [0-9A-F]+ [0-9A-F]{8} [0-9A-F]{8} \*/\s+(.*?)\s*$", l)
        if m and fn: ins.append(re.sub(r"\s+", " ", m.group(1))); continue
        if l.startswith("endlabel ") and fn:
            if len(ins) == 8:
                ms = [re.fullmatch(p, i) for p, i in zip(pat, ins)]
                if all(ms):
                    print(fn, ms[1].group(1), ms[3].group(1)); sys.exit(0)
            fn = None
PYEOF
)
  if [ -z "$fx" ]; then
    echo "  --    arm 10: no func_L00_0042B6A8-shaped block in $TAG's .text asm; the assembler column is not exercised here"
  else
    read -r fxfn fxd fxf <<< "$fx"
    fxaddr=${fxfn##*_}; fxend=$(printf '%08X' "$((0x$fxaddr + 0x20))")
    # a level with no level C yet has no src/ directory
    mkdir -p "${fxsrc%/*}"
    cat > "$fxsrc" <<CEOF
/* level_gate.sh --selftest arm 10 fixture (scratch copy only, never a row of the real tree) */
extern int $fxd;
extern void $fxf(void *);
void $fxfn(int **obj)
{
    *obj = &$fxd;
    $fxf(obj);
}
CEOF
    for asr in as asm_unit; do
      { [ -f "$ROOT/going-decompiled/levels/usa/$TAG/config/${TAG}_c.tsv" ] && cat "$ROOT/going-decompiled/levels/usa/$TAG/config/${TAG}_c.tsv"
        printf '%s\tsrc/selftest_la_fixture.c\ts136\t-G8\t%s\t-\n' "$fxfn" "$asr"; } > "$ctsv"
      (cd "$S" && python3 -I tools/levels/gen_ld.py "going-decompiled/levels/usa/$TAG/config/$TAG.yaml" > /dev/null)
      echo "-- arm 10 ASSEMBLER COLUMN ($asr): fixture row '$fxfn src/selftest_la_fixture.c s136 -G8 $asr -' (*obj = &$fxd; $fxf(obj)) added, $TAG.ld regenerated"
      out=$(gate_core "$S" "$NN" 0); fxrc[$asr]=$?
      printf '%s\n' "$out" | /usr/bin/grep -a -v -E '^ +PLACED ok ' | sed "s/^/  [la $asr] /"
      tl=$(printf '%s\n' "$out" | /usr/bin/grep -a -E '^ +check: cmp text +\.text ' || true)
      fxfirst[$asr]=$(printf '%s' "$tl" | sed -n -E 's/.* first at 0x([0-9A-F]{8}).*/\1/p')
      fxoth[$asr]=$(printf '%s\n' "$out" | /usr/bin/grep -a -E '^ +check: cmp ' | /usr/bin/grep -a -v -E 'cmp text ' | /usr/bin/grep -a -c ' 0 differing bytes ' || true)
      cp -a "$SB/obj/c/$fxfn.o" "$S/arm10_$asr.o" 2>/dev/null || rm -f "$S/arm10_$asr.o"
    done
    # verdict <obj as> <obj asm_unit> <rc as> <rc asm_unit>: echoes PASS or FAIL: <why>
    la_verdict() {
      local ta tu
      ta=$(textsha "$1"); tu=$(textsha "$2")
      if [ "$4" != 0 ]; then echo "FAIL: the asm_unit build rc $4 (want 0: the ROM's bytes)"
      elif [ "$3" != 1 ] || [ -z "${fxfirst[as]}" ] || [ "$((0x${fxfirst[as]:-0} >= 0x$fxaddr && 0x${fxfirst[as]:-0} < 0x$fxend))" != 1 ] || [ "${fxoth[as]:-0}" != 6 ]; then
        echo "FAIL: the as build rc $3, .text first difference '${fxfirst[as]:-none}' (want rc 1 inside [0x$fxaddr, 0x$fxend)), other sections cmp 0: ${fxoth[as]:-0} of 6"
      elif [ -z "$ta" ] || [ -z "$tu" ] || [ "$ta" = "$tu" ]; then echo "FAIL: the two objects' .text sha1 '${ta:-none}' and '${tu:-none}' (want two different values)"
      else echo "PASS: .text sha1 as $ta != asm_unit $tu"; fi
    }
    local v vs
    v=$(la_verdict "$S/arm10_as.o" "$S/arm10_asm_unit.o" "${fxrc[as]}" "${fxrc[asm_unit]}")
    case "$v" in PASS*) echo "  ok    arm 10: asm_unit build rc 0 PASS; as build rc ${fxrc[as]}, FAIL on .text first at 0x${fxfirst[as]} inside $fxfn [0x$fxaddr, 0x$fxend), the other 6 sections cmp 0; ${v#PASS: }";;
                    *) echo "  FAIL  arm 10: $v"; ok=0;; esac
    # seeded: the same verdict, the same two builds' outcomes, but the asm_unit
    # object in place of the as one, so only the object comparison can fail it
    vs=$(la_verdict "$S/arm10_asm_unit.o" "$S/arm10_asm_unit.o" "${fxrc[as]}" "${fxrc[asm_unit]}")
    case "$vs" in "FAIL: the two objects' .text sha1 "*) echo "  ok    arm 10 seeded: the asm_unit object compared with itself -> verdict '$vs'";;
                  *) echo "  FAIL  arm 10 seeded: the asm_unit object compared with itself -> '$vs' (the arm cannot fail)"; ok=0;; esac
    rm -f "$fxsrc" "$S/arm10_as.o" "$S/arm10_asm_unit.o"
    cp -a "$ROOT/going-decompiled/levels/usa/$TAG/." "$S/going-decompiled/levels/usa/$TAG/"
    [ -f "$ROOT/going-decompiled/levels/usa/$TAG/config/${TAG}_c.tsv" ] || rm -f "$ctsv"
    summary="$summary; assembler column: as FAIL / asm_unit PASS on $fxfn, objects differ, self-compare FAIL"
  fi

  # arms 7-9 (task #2516), only for a level with boot rows
  if [ $hasboot = 0 ]; then
    echo "  --    arms 7-9: no config/${TAG}_boot_c.tsv; the level places no boot C"
  else
    local bn=$S/going-decompiled/levels/usa/$TAG/symbol_addrs/${TAG}_boot_names.txt seedn=g_mobyClassHeaders users diffs ov
    users=$(python3 -I "$S/tools/levels/gen_boot_names.py" users --level "$((10#$NN))" "$seedn")
    ov=$(sed -n "s/^$seedn = \(0x[0-9A-F]*\);.*/\1/p" "$bn")
    python3 -I -c 'import re,sys
p,n=sys.argv[1],sys.argv[2]
t=open(p).read()
t2=re.sub(r"^(%s = )0x([0-9A-F]+);" % re.escape(n), lambda m: "%s0x%08X;" % (m.group(1), int(m.group(2), 16) + 4), t, count=1, flags=re.M)
assert t2 != t
open(p, "w").write(t2)' "$bn" "$seedn"
    echo "-- arm 7 SEEDED MAP: ${bn#$S/}: $seedn $ov -> $(sed -n "s/^$seedn = \(0x[0-9A-F]*\);.*/\1/p" "$bn") (must FAIL on .text only; differing placed functions must be exactly its users: ${users:-none})"
    out=$(gate_core "$S" "$NN" 0); rc=$?
    printf '%s\n' "$out" | sed 's/^/  [seeded map] /'
    diffs=$(printf '%s\n' "$out" | sed -n -E 's/^ +PLACED-DIFFERS (func_[A-Z0-9_]+) .*/\1/p' | sort | tr '\n' ' ' | sed 's/ $//')
    others=$(printf '%s\n' "$out" | /usr/bin/grep -a -E '^ +check: cmp ' | /usr/bin/grep -a -v -E 'cmp text ' | /usr/bin/grep -a -c ' 0 differing bytes ' || true)
    if [ $rc = 1 ] && [ -n "$users" ] && [ "$diffs" = "$users" ] && [ "${others:-0}" = 6 ]; then
      echo "  ok    arm 7: seeded map -> rc 1; PLACED-DIFFERS members == the users of $seedn: $diffs; the other 6 sections cmp 0"
    else echo "  FAIL  arm 7: rc $rc; PLACED-DIFFERS '$diffs' vs users '$users'; other sections cmp 0: $others of 6"; ok=0; fi
    cp -a "$ROOT/going-decompiled/levels/usa/$TAG/." "$S/going-decompiled/levels/usa/$TAG/"

    # arm 8: the alignment step removed: every .text.<fn> back to the 16 GNU as records
    local bo=$S/going-decompiled/build/usa_levels/boot_objs nmis
    echo "-- arm 8 NO ALIGNMENT: every .text.<fn> section of the copied boot objects set to 16, as GNU as records (must FAIL as MISPLACED)"
    docker --context "$CTX" run --rm --user="$(id -u):$(id -g)" -e HOME=/tmp -v "$S":/work -w /work ee-build sh -c '
for o in going-decompiled/build/usa_levels/boot_objs/*.o; do
  args=$(mips-linux-gnu-readelf -SW "$o" | sed -n "s/.*\] \(\.text\.[A-Za-z0-9_.\$]*\) .*/ --set-section-alignment \1=16/p" | tr -d "\n")
  mips-linux-gnu-objcopy $args "$o" || exit 1
done' || echo "  FAIL  arm 8: could not realign the copied objects"
    out=$(gate_core "$S" "$NN" 0); rc=$?
    printf '%s\n' "$out" | /usr/bin/grep -a -v -E '^ +PLACED ok ' | sed 's/^/  [no align] /'
    nmis=$(printf '%s\n' "$out" | /usr/bin/grep -a -c -E '^ +PLACED FAIL .*; MISPLACED$' || true)
    if [ $rc = 1 ] && [ "${nmis:-0}" -gt 0 ]; then
      echo "  ok    arm 8: no alignment -> rc 1, $nmis PLACED FAIL rows MISPLACED"
    else echo "  FAIL  arm 8: rc $rc, MISPLACED rows ${nmis:-0} (want rc 1 and > 0)"; ok=0; fi
    rm -rf "$bo"; cp -a "$ROOT/going-decompiled/build/usa_levels/boot_objs" "$S/going-decompiled/build/usa_levels/"

    # arm 9: placements off
    rm -f "$S/going-decompiled/levels/usa/$TAG/config/${TAG}_boot_c.tsv"
    (cd "$S" && python3 -I tools/levels/gen_ld.py "going-decompiled/levels/usa/$TAG/config/$TAG.yaml" > /dev/null)
    echo "-- arm 9 PLACEMENTS OFF: ${TAG}_boot_c.tsv removed, $TAG.ld regenerated (must PASS, .text sha1 == arm 1's)"
    out=$(gate_core "$S" "$NN" 0); rc=$?
    printf '%s\n' "$out" | sed 's/^/  [placements off] /'
    local t9; t9=$([ -f "$SB/blobs/text.bin" ] && sha1 "$SB/blobs/text.bin")
    local nbg; nbg=$(printf '%s\n' "$out" | /usr/bin/grep -a -c -E '^ +PLACED ok ' || true)
    if [ $rc = 0 ] && [ -n "$text1" ] && [ "$t9" = "$text1" ] && [ "${nbg:-0}" = 0 ]; then
      echo "  ok    arm 9: placements off -> rc 0 PASS from asm (PLACED rows $nbg); built .text sha1 $t9 == arm 1 (placements on) $text1"
    else echo "  FAIL  arm 9: rc $rc, .text sha1 '${t9:-none}' vs arm 1 '${text1:-none}', PLACED rows $nbg"; ok=0; fi
    cp -a "$ROOT/going-decompiled/levels/usa/$TAG/." "$S/going-decompiled/levels/usa/$TAG/"
    summary="$summary; seeded map FAIL with exactly its users differing; no alignment FAIL MISPLACED; placements off PASS with identical .text"
  fi

  rc=0; [ $ok = 1 ] || rc=1
  if [ $ok = 1 ]; then echo "#### level_gate $TAG SELFTEST: PASS (unseeded PASS; seeded word FAIL on .text at 0x$addr; wrong anchor FAIL; no anchor FAIL$summary)"; else echo "#### level_gate $TAG SELFTEST: FAIL"; fi
  footer "$t0" "$rc"
  return $rc
}

case "${1:-}" in
  ""|-h|--help) usage;;
  --selftest) [ -n "${2:-}" ] || usage; selftest "$2"; exit $?;;
esac
NN=$1; shift
EXPECT=""
if [ "${1:-}" = "--expect" ]; then [ -n "${2:-}" ] || usage; EXPECT=$(cd "$(dirname "$2")" && pwd)/$(basename "$2"); fi
case "$NN" in *[!0-9]*) usage;; esac
T0=$(now)
gate_core "$ROOT" "$NN" 1 "$EXPECT"; RC=$?
footer "$T0" "$RC"
exit $RC

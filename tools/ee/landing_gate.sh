#!/usr/bin/env bash
# landing_gate.sh — the checks a master landing must pass, executed, not
# remembered (task #449 ENFORCE-1). Every one of them was written down this
# week and enforced by nothing (#447: "four seats have now written it; nothing
# executes it"):
#
#   FLAGS    tools/ee/flagdiff.py — the per-unit cc1 flag tables of build.sh,
#            objdiff_build.sh, diff.sh and unit_flags.sh agree (FACT #7164:
#            lever 2 and WARM-1 drifted under a "keep in sync" comment).
#   SHADOW   tools/ee/shadow_scan2.sh — CLASS1 INCLUDE_ASM leftovers, CLASS2
#            compiled C definitions under func_ names, CLASS3 interior labels
#            (FACT #7249, #7291, #7294), counted and LISTED, plus NOTARGET: the
#            CLASS2 members with no nonmatchings/<unit>/*.s under either name,
#            which the unit objdiff gate has never scored (FACT #7295). Each
#            count may go DOWN against tools/ee/shadow_baseline.txt, never up.
#   BUILD    tools/ee/build.sh <region> in the ee-build container, objects and
#            link outputs wiped first so nothing stale can be measured.
#   ROW      cmp vs retail .rom (count), sha1 == the yaml's, e_entry == the
#            retail ELF's, ld.log 0 B with an mtime from THIS run, nm -u 0 — and
#            the negative control RULING #7208 condition 2 asks for, observed in
#            the same run: cmp of a copy with 4 bytes flipped prints 4. EU does
#            not link (FACT #22173, 11 undefined): its row is "no ELF, ld.log
#            undefined set within tools/ee/landing_baseline/ldundef_eu.txt".
#   PROVIDE  the discriminating relink (#447 BD-2): all_addr_syms.ld with ONLY
#            its PROVIDE( lines stripped (dropping the whole file fails on any
#            tree — the func_/D_ blanket goes with it). It must FAIL, and its
#            undefined set — every name the tree spells two ways and holds
#            together only by PROVIDE — may shrink against
#            tools/ee/landing_baseline/noprovide_<region>.txt, never grow.
#
#   tools/ee/landing_gate.sh <region>            all of the above; exit 0 = PASS
#   tools/ee/landing_gate.sh <region> --no-build  reuse the outputs of an earlier
#                                                 build (row mtime check still
#                                                 applies to the reused ld.log)
#   tools/ee/landing_gate.sh --selftest [region]  seed every check's failing arm
#                                                 and require it to fire, then
#                                                 run the real gate and require
#                                                 PASS (default region usa)
#
# Exit 0 PASS / 1 a check failed (every failing member printed) / 2 could not
# run (missing input, VM, baseline). Output files: tools/ee/.gate_landing/<region>/
# (gitignored via tools/ee/.gate*/). EE_DOCKER_CONTEXT selects the VM exactly as
# objdiff_build.sh does (default colima-ee-x86).
#
# What this gate does NOT see (say it, so nobody reads PASS as more than it is):
#   - a data re-attribution that is byte-identical after assembly (NOTE #7245
#     P5; FACT #7248: `.word Name+off` → raw word) — cmp 0 on both sides;
#   - a boot regression — no emulator is run here;
#   - a stale splat cache — configure.py's default is now a full split and
#     tools/ee/splache_selftest.sh proves the invalidation, but this script does
#     not re-split; it measures the asm that is committed;
#   - the unit objdiff gate's fuzzy rows (objdiff_build.sh + unit_report.sh) —
#     a byte-exact ROM makes them redundant for USA and they are not run here;
#   - EU bytes — EU has no link, so its row is a link-property, not a cmp.
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"; cd "$ROOT"
HERE="tools/ee"
EE_CTX="${EE_DOCKER_CONTEXT:-colima-ee-x86}"
BASE_DIR="$HERE/landing_baseline"
SHADOW_BASELINE="$HERE/shadow_baseline.txt"

usage() { sed -n '2,45p' "$0" | sed 's/^# \{0,1\}//'; exit 2; }
say()  { printf '%s\n' "$*"; }
fail() { say "FAIL $*"; FAILED=$((FAILED+1)); }
ok()   { say "OK   $*"; }
FAILED=0

region_vars() {
  REGION="$1"
  case "$REGION" in
    usa) BASENAME=SCUS_972.68 ;;
    eu)  BASENAME=SCES_516.07 ;;
    *) say "unknown region $REGION"; exit 2 ;;
  esac
  BUILD="going-decompiled/build/$REGION"
  OUT="$HERE/.gate_landing/$REGION"
  mkdir -p "$OUT"
  RETAIL_ELF="extracted/$REGION/$BASENAME"
  RETAIL_ROM="extracted/$REGION/$BASENAME.rom"
  YAML="going-decompiled/config/$REGION/$BASENAME.yaml"
  LDSCRIPT="going-decompiled/linker_scripts/$BASENAME.ld"
  for f in "$RETAIL_ELF" "$RETAIL_ROM" "$YAML" "$LDSCRIPT" "$SHADOW_BASELINE" "$BASE_DIR/noprovide_$REGION.txt"; do
    [ -f "$f" ] || { say "landing_gate: missing input $f"; exit 2; }
  done
}

in_vm() {  # in_vm '<sh script>' — one docker run, repo mounted at /work
  docker --context "$EE_CTX" run --rm -v "$ROOT":/work -w /work ee-build sh -c "$1"
}

# ---------------------------------------------------------------- FLAGS ----
check_flags() {  # check_flags [build.sh objdiff_build.sh ...]
  say "== FLAGS: per-unit cc1 flag tables must agree"
  local out; out=$(python3 "$HERE/flagdiff.py" "$@" 2>&1); local rc=$?
  say "$out" | sed 's/^/     /'
  case $rc in
    0) ok "flag tables agree" ;;
    1) fail "flag DRIFT between the tables above" ;;
    *) fail "flagdiff could not run (rc $rc)" ;;
  esac
}

# --------------------------------------------------------------- SHADOW ----
# shadow_scan REGION TREE OUTFILE — scan TREE (a repo root) and append the
# NOTARGET rows: CLASS2 members whose unit has no nonmatchings/<unit>/<old>.s
# and no <new>.s (FACT #7295: never scored by the unit gate).
shadow_scan() {
  local region=$1 tree=$2 outfile=$3
  ROOT="$tree" bash "$HERE/shadow_scan2.sh" "$region" > "$outfile" || return 2
  /usr/bin/grep -E '^CLASS2 ' "$outfile" | while read -r cls loc old arrow new; do
    local unit; unit=$(printf '%s' "$loc" | sed -E "s#^going-decompiled/src/$region/(.*)\.c:[0-9]+\$#\1#")
    local d="$tree/going-decompiled/asm/$region/nonmatchings/$unit"
    [ -f "$d/$old.s" ] || [ -f "$d/$new.s" ] || printf 'NOTARGET %s %s -> %s\n' "$loc" "$old" "$new"
  done >> "$outfile"
}

# shadow_compare REGION SCANFILE BASELINE — counts vs baseline, never up.
shadow_compare() {
  local region=$1 scan=$2 baseline=$3 cls n base
  say "== SHADOW [$region]: name dualities (scan: $scan)"
  for cls in CLASS1 CLASS2 CLASS3 NOTARGET; do
    n=$(/usr/bin/grep -c "^$cls " "$scan" || true)
    base=$(awk -v r="$region" -v c="$cls" '$1==r && $2==c {print $3}' "$baseline")
    if [ -z "$base" ]; then fail "$cls [$region]: no baseline row in $baseline"; continue; fi
    if [ "$n" -gt "$base" ]; then
      fail "$cls [$region]: $n rows, baseline $base — GREW by $((n-base)); a landing may lower this, never raise it"
    elif [ "$n" -lt "$base" ]; then
      ok "$cls [$region]: $n rows, baseline $base (lower — update the baseline in the same landing)"
    else
      ok "$cls [$region]: $n rows == baseline"
    fi
  done
  say "     members:"; sed 's/^/       /' "$scan"
}

check_shadow() {  # check_shadow REGION [TREE] [BASELINE]
  local region=$1 tree=${2:-.} baseline=${3:-$SHADOW_BASELINE}
  local scan="$OUT/shadow_scan.txt"
  shadow_scan "$region" "$tree" "$scan" || { fail "shadow_scan2.sh could not run"; return; }
  shadow_compare "$region" "$scan" "$baseline"
}

# ---------------------------------------------------------------- BUILD ----
do_build() {
  say "== BUILD [$REGION]: build.sh in $EE_CTX, objects and link outputs wiped first"
  date +%s > "$OUT/build_start"
  in_vm "B=$BUILD; rm -rf \$B/going-decompiled \$B/$BASENAME.elf \$B/$BASENAME.lma.elf \$B/$BASENAME.rom \$B/ld.log \$B/ld.lma.log \$B/$BASENAME.map \$B/all_addr_syms.ld; sh tools/ee/build.sh $REGION" > "$OUT/build.log" 2>&1
  say "     build.sh rc=$? ($(wc -l < "$OUT/build.log" | tr -d ' ') log lines -> $OUT/build.log)"
  tail -4 "$OUT/build.log" | sed 's/^/     /'
}

# ------------------------------------------------------------------ ROW ----
# measure_row ROMPATH ELFPATH LDLOG OUTFILE — everything measured IN the
# container, written as key=value. The flipped-copy cmp is the negative control.
measure_row() {
  local rom=$1 elf=$2 ldlog=$3 outfile=$4
  in_vm "
    R=$rom; E=$elf; L=$ldlog; O=$RETAIL_ROM
    if [ -f \$R ]; then
      echo cmp_count=\$(cmp -l \$O \$R 2>/dev/null | wc -l | tr -d ' ')
      echo rom_size=\$(stat -c%s \$R); echo retail_size=\$(stat -c%s \$O)
      echo sha1_built=\$(sha1sum \$R | awk '{print \$1}')
      cp \$R /tmp/flipped.rom; printf '\\377\\377\\377\\377' | dd of=/tmp/flipped.rom bs=1 seek=4096 conv=notrunc 2>/dev/null
      echo cmp_flipped=\$(cmp -l \$O /tmp/flipped.rom 2>/dev/null | wc -l | tr -d ' ')
    else echo rom_present=no; fi
    if [ -f \$E ]; then
      echo elf_present=yes
      echo e_entry=\$(mips-linux-gnu-readelf -h \$E | awk '/Entry point/{print tolower(\$4)}')
      echo nm_u=\$(mips-linux-gnu-nm -u \$E | wc -l | tr -d ' ')
    else echo elf_present=no; fi
    if [ -f \$L ]; then echo ldlog_size=\$(stat -c%s \$L); echo ldlog_mtime=\$(stat -c%Y \$L); else echo ldlog_present=no; fi
  " > "$outfile" 2>&1
  /usr/bin/grep -oE "undefined reference to \`[^']+'" "$ldlog" 2>/dev/null | sed -E "s/^undefined reference to \`(.*)'\$/\1/" | LC_ALL=C sort -u > "$outfile.undefined"
}

rowval() { awk -F= -v k="$1" '$1==k{print $2}' "$2"; }

# check_row ROWFILE START_EPOCH — evaluate a measured row for $REGION.
check_row() {
  local row=$1 start=$2 v
  say "== ROW [$REGION]: link outputs ($row)"
  local want_sha1; want_sha1=$(awk '/^sha1:/{print $2}' "$YAML")
  local want_entry; want_entry=$(python3 -c 'import struct,sys; print(hex(struct.unpack("<I", open(sys.argv[1],"rb").read(0x1c)[0x18:0x1c])[0]))' "$RETAIL_ELF")
  if [ "$REGION" = usa ]; then
    v=$(rowval cmp_count "$row"); [ "$v" = 0 ] && ok "cmp: 0 differing bytes of $(rowval retail_size "$row")" || fail "cmp: ${v:-no rom} differing bytes (retail $(rowval retail_size "$row"), built $(rowval rom_size "$row"))"
    v=$(rowval cmp_flipped "$row"); [ "${v:-0}" -gt 0 ] && ok "negative control: 4 bytes flipped in a copy -> cmp $v" || fail "negative control did not fire: flipped copy cmp '${v:-}' (the cmp check cannot fail)"
    v=$(rowval sha1_built "$row"); [ "$v" = "$want_sha1" ] && ok "sha1 built == yaml $want_sha1" || fail "sha1 built '${v:-}' != yaml $want_sha1"
    v=$(rowval elf_present "$row"); [ "$v" = yes ] && ok "ELF present" || fail "no ELF"
    v=$(rowval e_entry "$row"); [ "$v" = "$want_entry" ] && ok "e_entry $v == retail ELF" || fail "e_entry '${v:-}' != retail $want_entry"
    v=$(rowval nm_u "$row"); [ "$v" = 0 ] && ok "nm -u 0" || fail "nm -u '${v:-}'"
    v=$(rowval ldlog_size "$row"); [ "$v" = 0 ] && ok "ld.log 0 B" || fail "ld.log ${v:-absent} B: $(head -c 300 "$row.undefined" | tr '\n' ' ')"
  else
    # EU does not link today (FACT #22173). Its row is a link property.
    v=$(rowval elf_present "$row"); [ "$v" = no ] && ok "EU: no ELF (expected while EU does not link; a linking EU retires this arm — re-argue the row)" || fail "EU produced an ELF — the EU row must be rewritten, this gate has no byte check for it"
    local ldbase="$BASE_DIR/ldundef_eu.txt"
    if [ -f "$ldbase" ]; then
      local grew; grew=$(LC_ALL=C comm -23 "$row.undefined" <(LC_ALL=C sort -u "$ldbase"))
      local n; n=$(wc -l < "$row.undefined" | tr -d ' ')
      if [ -n "$grew" ]; then fail "EU ld.log undefined set GREW ($n vs baseline $(wc -l < "$ldbase" | tr -d ' ')): $(printf '%s ' $grew)"; else ok "EU ld.log undefined set within baseline ($n members: $(tr '\n' ' ' < "$row.undefined"))"; fi
    else fail "no $ldbase"; fi
  fi
  v=$(rowval ldlog_mtime "$row")
  if [ -n "$v" ] && [ "$v" -ge "$start" ]; then ok "ld.log mtime $v >= run start $start (this run)"; else fail "ld.log mtime '${v:-absent}' < run start $start — a STALE log"; fi
}

# -------------------------------------------------------------- PROVIDE ----
# relink LDSYMS OUTELF OUTLOG — the link step of build.sh with a substitute
# address-symbol script, written to scratch paths (never over the gate outputs).
relink() {
  local ldsyms=$1 outelf=$2 outlog=$3
  in_vm "mips-linux-gnu-ld -EL --allow-multiple-definition -e _start -T $LDSCRIPT -T $BUILD/undefined_syms_auto.txt -T $ldsyms -o $outelf 2> $outlog; echo rc=\$?"
}

check_noprovide() {  # check_noprovide [BASELINE]
  local baseline=${1:-$BASE_DIR/noprovide_$REGION.txt}
  say "== PROVIDE [$REGION]: relink with only the PROVIDE( lines stripped must fail; its undefined set may not grow"
  local all="$BUILD/all_addr_syms.ld" np="$OUT/all_addr_syms.noprovide.ld"
  [ -f "$all" ] || { fail "no $all (build first)"; return; }
  /usr/bin/grep -v '^PROVIDE(' "$all" > "$np"
  say "     $(wc -l < "$all" | tr -d ' ') address symbols, $(/usr/bin/grep -c '^PROVIDE(' "$all" || true) PROVIDE lines stripped"
  local rc; rc=$(relink "$np" "$OUT/noprovide.elf" "$OUT/noprovide.ld.log")
  /usr/bin/grep -oE "undefined reference to \`[^']+'" "$OUT/noprovide.ld.log" | sed -E "s/^undefined reference to \`(.*)'\$/\1/" | LC_ALL=C sort -u > "$OUT/noprovide.undefined"
  # names undefined WITH PROVIDE (EU's 11) are not PROVIDE-held; subtract them
  local with="$OUT/row.txt.undefined"; [ -f "$with" ] || : > "$with"
  LC_ALL=C comm -23 "$OUT/noprovide.undefined" <(LC_ALL=C sort -u "$with") > "$OUT/noprovide.held"
  local n; n=$(wc -l < "$OUT/noprovide.held" | tr -d ' ')
  if [ "$rc" != rc=0 ] && [ "$n" -gt 0 ]; then ok "relink without PROVIDE fails ($rc): $n names held only by PROVIDE"; else fail "relink without PROVIDE did not fail ($rc, $n undefined) — PROVIDE holds nothing, or the relink did not run"; fi
  local grew; grew=$(LC_ALL=C comm -23 "$OUT/noprovide.held" <(LC_ALL=C sort -u "$baseline"))
  local gone; gone=$(LC_ALL=C comm -13 "$OUT/noprovide.held" <(LC_ALL=C sort -u "$baseline"))
  if [ -n "$grew" ]; then fail "PROVIDE-held set GREW vs $baseline ($(wc -l < "$baseline" | tr -d ' ')): NEW $(printf '%s ' $grew)"; else ok "PROVIDE-held set within baseline ($(wc -l < "$baseline" | tr -d ' '))$( [ -n "$gone" ] && printf ' — %s no longer held: %s (lower the baseline in the same landing)' "$(printf '%s\n' $gone | wc -l | tr -d ' ')" "$(printf '%s ' $gone)")"; fi
  say "     members -> $OUT/noprovide.held"
}

# ----------------------------------------------------------------- GATE ----
run_gate() {  # run_gate REGION [--no-build]
  region_vars "$1"; local build=1; [ "${2:-}" = --no-build ] && build=0
  FAILED=0
  say "#### landing_gate $REGION at $(git rev-parse --short HEAD) ($(git status --porcelain --no-renames | wc -l | tr -d ' ') dirty paths), VM $EE_CTX, $(date -u +%FT%TZ)"
  check_flags
  check_shadow "$REGION"
  if [ $build = 1 ]; then do_build; else say "== BUILD [$REGION]: skipped (--no-build), start epoch taken from $OUT/build_start"; fi
  local start; start=$(cat "$OUT/build_start" 2>/dev/null || echo 0)
  measure_row "$BUILD/$BASENAME.rom" "$BUILD/$BASENAME.elf" "$BUILD/ld.log" "$OUT/row.txt"
  check_row "$OUT/row.txt" "$start"
  check_noprovide
  say "#### landing_gate $REGION: $([ $FAILED = 0 ] && echo PASS || echo "FAIL ($FAILED)")"
  [ $FAILED = 0 ]
}

# ------------------------------------------------------------- SELFTEST ----
# Every check's failing arm, seeded and required to fire, before the real run.
selftest() {
  region_vars "${1:-usa}"
  local T="$OUT/selftest"; rm -rf "$T"; mkdir -p "$T"; local bad=0
  say "#### landing_gate --selftest [$REGION]: seeded failing arms"

  say "-- (1) FLAGS: one flag perturbed in a copy of build.sh"
  sed 's/usa\/text\/235FE8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing"/usa\/text\/235FE8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse"/' "$HERE/build.sh" > "$T/build_pert.sh"
  cmp -s "$HERE/build.sh" "$T/build_pert.sh" && { say "SELFTEST-BROKEN: perturbation did not change build.sh"; bad=1; }
  FAILED=0; check_flags "$T/build_pert.sh" "$HERE/objdiff_build.sh" > "$T/flags.txt"
  if [ $FAILED -gt 0 ] && /usr/bin/grep -q 'DRIFT usa/text/235FE8' "$T/flags.txt"; then ok "fired: $(/usr/bin/grep -E '^ +DRIFT' "$T/flags.txt" | sed 's/^ *//')"; else say "SELFTEST-FAIL flags control did not fire"; cat "$T/flags.txt"; bad=1; fi

  say "-- (2) SHADOW: planted INCLUDE_ASM + planted C definition + planted interior label in a scratch tree; counts must GROW vs baseline"
  local SC="$T/tree"; mkdir -p "$SC/going-decompiled/asm/$REGION"
  cp -R going-decompiled/src "$SC/going-decompiled/src"
  cp -R going-decompiled/symbol_addrs "$SC/going-decompiled/symbol_addrs"
  cp -R "going-decompiled/asm/$REGION/nonmatchings" "$SC/going-decompiled/asm/$REGION/nonmatchings"
  local C; C=$(/usr/bin/find "$SC/going-decompiled/src/$REGION" -name '*.c' | sort | head -1)
  local S; S=$(/usr/bin/grep -rhoE 'INCLUDE_ASM\("[^"]+",[[:space:]]*[A-Za-z_][A-Za-z0-9_]*' "$SC/going-decompiled/src/$REGION" | /usr/bin/sed -E 's/INCLUDE_ASM\("([^"]+)",[[:space:]]*([A-Za-z0-9_]+)/\1\/\2.s/' | sort -u | head -1)
  printf '\n/* t449 selftest seeds */\nINCLUDE_ASM("%s", func_00DEAD00);\ns32 func_00DEAD04(void) { return 0; }\n' "$(dirname "$S")" >> "$C"
  printf 'alabel func_00DEAD08\n' >> "$SC/$S"
  printf 'T449SeedC1 = 0x00DEAD00; // type:func\nT449SeedC2 = 0x00DEAD04; // type:func\nT449SeedC3 = 0x00DEAD08; // type:func\n' >> "$SC/going-decompiled/symbol_addrs/$REGION/symbol_addrs.txt"
  FAILED=0; check_shadow "$REGION" "$SC" > "$T/shadow.txt"; cp "$OUT/shadow_scan.txt" "$T/shadow_scan_seeded.txt"
  local fired; fired=$(/usr/bin/grep -cE '^FAIL (CLASS1|CLASS2|CLASS3|NOTARGET) .*GREW' "$T/shadow.txt" || true)
  # the planted C definition has no .s under either name -> NOTARGET grows too: 4 classes
  if [ "$fired" = 4 ] && /usr/bin/grep -q 'func_00DEAD04 -> T449SeedC2' "$T/shadow.txt"; then ok "fired: $(/usr/bin/grep -E '^FAIL' "$T/shadow.txt" | sed -E 's/ — .*//' | tr '\n' ';')"; else say "SELFTEST-FAIL shadow controls: $fired of 4 classes GREW"; /usr/bin/grep -E '^(OK|FAIL)' "$T/shadow.txt"; bad=1; fi

  say "-- (3) BUILD once (real arm; the PROVIDE and ROW controls relink/measure against its objects)"
  do_build; local start; start=$(cat "$OUT/build_start")
  measure_row "$BUILD/$BASENAME.rom" "$BUILD/$BASENAME.elf" "$BUILD/ld.log" "$OUT/row.txt"

  say "-- (4) ROW: one byte flipped in a rom copy (in-run negative control) + a stale-log arm"
  FAILED=0; check_row "$OUT/row.txt" "$start" > "$T/row_clean.txt"
  /usr/bin/grep -q '^OK   negative control: 4 bytes flipped' "$T/row_clean.txt" && ok "fired: $(/usr/bin/grep '^OK   negative control' "$T/row_clean.txt")" || { say "SELFTEST-FAIL flipped-copy control absent"; cat "$T/row_clean.txt"; bad=1; }
  FAILED=0; check_row "$OUT/row.txt" $((start + 100000)) > "$T/row_stale.txt"
  /usr/bin/grep -q 'STALE log' "$T/row_stale.txt" && ok "fired: stale-mtime arm -> $(/usr/bin/grep -c '^FAIL' "$T/row_stale.txt") FAIL row(s)" || { say "SELFTEST-FAIL stale-log arm did not fire"; bad=1; }
  if [ "$REGION" = usa ]; then
    sed 's/^cmp_count=.*/cmp_count=4/; s/^sha1_built=.*/sha1_built=deadbeef/' "$OUT/row.txt" > "$T/row_bad.txt"; cp "$OUT/row.txt.undefined" "$T/row_bad.txt.undefined"
    FAILED=0; check_row "$T/row_bad.txt" "$start" > "$T/row_bad_eval.txt"
    [ "$FAILED" -ge 2 ] && ok "fired: a row with cmp 4 / wrong sha1 -> $FAILED FAIL rows" || { say "SELFTEST-FAIL bad-row arm: $FAILED"; cat "$T/row_bad_eval.txt"; bad=1; }
  fi

  say "-- (5) PROVIDE: one PROVIDE line removed from all_addr_syms.ld -> the full link must fail (undefined name, no ELF)"
  local all="$BUILD/all_addr_syms.ld"
  local victim; victim=$(/usr/bin/grep -E '^PROVIDE\((rand|GetRandomFloatRange|strcmp) = ' "$all" | head -1)
  [ -n "$victim" ] || victim=$(/usr/bin/grep -E '^PROVIDE\(' "$all" | head -1)
  /usr/bin/grep -vF "$victim" "$all" > "$T/one_provide_removed.ld"
  [ "$(/usr/bin/grep -c '^PROVIDE(' "$all" || true)" -eq "$(( $(/usr/bin/grep -c '^PROVIDE(' "$T/one_provide_removed.ld" || true) + 1 ))" ] || { say "SELFTEST-BROKEN: victim line not removed"; bad=1; }
  local rc; rc=$(relink "$T/one_provide_removed.ld" "$T/one_provide_removed.elf" "$T/one_provide_removed.log")
  local name; name=$(printf '%s' "$victim" | sed -E 's/^PROVIDE\(([A-Za-z0-9_]+) = .*/\1/')
  local n; n=$(/usr/bin/grep -c "undefined reference to \`$name'" "$T/one_provide_removed.log" || true)
  if [ "$REGION" = usa ]; then
    if [ "$rc" != rc=0 ] && [ "$n" -gt 0 ] && [ ! -f "$T/one_provide_removed.elf" ]; then ok "fired: without '$victim' the link fails ($rc, $n references to $name undefined, no ELF)"; else say "SELFTEST-FAIL removing '$victim' did not break the link ($rc, $n undefined, elf=$([ -f "$T/one_provide_removed.elf" ] && echo present || echo absent))"; bad=1; fi
  else
    if [ "$n" -gt 0 ]; then ok "fired: without '$victim' $n more references undefined ($rc; EU never links)"; else say "SELFTEST-FAIL removing '$victim' added no undefined reference"; bad=1; fi
  fi
  say "-- (6) PROVIDE baseline: one member dropped from a copy of the baseline -> GREW"
  FAILED=0; check_noprovide > "$T/np_clean.txt"; local npf=$FAILED
  local held="$OUT/noprovide.held"; sed '1d' "$BASE_DIR/noprovide_$REGION.txt" > "$T/np_baseline_short.txt"
  FAILED=0; check_noprovide "$T/np_baseline_short.txt" > "$T/np_short.txt"
  local dropped; dropped=$(head -1 "$BASE_DIR/noprovide_$REGION.txt")
  if /usr/bin/grep -q "GREW.*NEW.*$dropped" "$T/np_short.txt"; then ok "fired: baseline minus '$dropped' -> $(/usr/bin/grep -oE 'GREW[^:]*' "$T/np_short.txt" | head -1)"; else say "SELFTEST-FAIL noprovide baseline arm did not fire"; cat "$T/np_short.txt"; bad=1; fi

  say "-- (7) the real gate on this tree (--no-build, the build above) must PASS"
  if run_gate "$REGION" --no-build > "$T/gate.txt"; then ok "real gate PASS"; else say "SELFTEST-FAIL the real gate does not pass on this tree:"; /usr/bin/grep -E '^FAIL' "$T/gate.txt"; bad=1; fi
  say "     full gate output -> $T/gate.txt"
  say "#### landing_gate --selftest [$REGION]: $([ $bad = 0 ] && echo PASS || echo FAIL)"
  return $bad
}

# sourceable (`. tools/ee/landing_gate.sh`) for the individual check functions
if [ "${BASH_SOURCE[0]}" = "$0" ]; then
  case "${1:-}" in
    --selftest) selftest "${2:-usa}" ;;
    usa|eu) run_gate "$1" "${2:-}" ;;
    *) usage ;;
  esac
fi

#!/usr/bin/env bash
# landing_gate.sh — the checks a master landing must pass, executed, not
# remembered (task #449 ENFORCE-1). Every one of them was written down this
# week and enforced by nothing (#447: "four seats have now written it; nothing
# executes it"):
#
#   FLAGS    tools/ee/flagdiff.py — the per-unit cc1 flag tables of build.sh,
#            objdiff_build.sh, diff.sh and unit_flags.sh agree (FACT #7164:
#            lever 2 and WARM-1 drifted under a "keep in sync" comment).
#   SPLIT    (building form only, task #464 ENFORCE-3) scripts/configure.py
#            --region <r> — a FULL split from the committed tree — regenerates
#            the two link inputs that live under the gitignored build/<r>/:
#            undefined_syms_auto.txt (build.sh `-T`) and include/{macro,labels}.inc
#            + include_asm.h (`-I`). FACT #7324: they ride under the TREE hash,
#            so a hybrid file copied from another tree (CLAUDE.md's trap) or a
#            0 B one left by a cached configure.py (FACT #7150) was invisible.
#            Now the gate makes them from the tree (~8 s a region, measured
#            7.8 s usa / 8.9 s eu at c06bd392), FAILS if the split rewrote any
#            tracked path (RULING #7208 condition 5's fixed point, executed —
#            every later check then measures the re-split tree, and this line
#            says whether that is the committed one), FAILS on a 0 B
#            undefined_syms_auto.txt, and records each input's size + sha256
#            in built_tree.txt so --no-build FAILS naming the file when one has
#            changed since the link. Needs .venv-decomp (exit 2 without it).
#   SHADOW   tools/ee/shadow_scan2.sh — CLASS1 INCLUDE_ASM leftovers, CLASS2
#            compiled C definitions under func_ names, CLASS3 interior labels
#            (FACT #7249, #7291, #7294), counted and LISTED, plus NOTARGET: the
#            CLASS2 members with no nonmatchings/<unit>/*.s under either name,
#            which the unit objdiff gate has never scored (FACT #7295). Each
#            class is a MEMBER set (the row minus its line number) compared
#            with comm against tools/ee/landing_baseline/shadow_<class>_<r>.txt:
#            a NEW member FAILS naming it, a member no longer observed WARNs
#            "lower the baseline in this landing" (RULING #7317). The count is
#            printed as a summary only — FACT #7303 (#453, #458): the count
#            ratchet passed a same-count swap with the new member printed, not
#            flagged.
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
#   ORPHAN   tools/ee/blanket_orphans.sh (task #457, FACT #7301): D_/func_
#            tokens compiled C references whose only definer is build.sh's
#            blanket grep of asm/. ORPHAN = no holder at all (undefined at link
#            now; landing_baseline/orphans_<region>.txt, 0 on USA, EU's 7 are
#            the D_/func_ members of its ld.log set); ORPHAN_LATENT = every
#            holder is a nonmatchings/**/func_*.s a rename deletes (#452 deleted
#            278 and the USA link lost four tokens; orphans_latent_<region>.txt).
#            Both member sets may shrink, never grow.
#   SYNC     (building form, task #542 MOUNT-SYNC-1) the four link inputs the
#            SPLIT step just rewrote on the host are read by the BUILD container
#            over the VM's fuse.sshfs mount, which can serve a STALE or TRUNCATED
#            view of a just-rewritten file (FACT #7449, NOTE #7430). do_build
#            takes their host md5s and tools/ee/mount_sync.sh verifies each in
#            the container (retrying) before build.sh runs; a file that never
#            agrees aborts the build rc 9 naming it. The same helper guards
#            objdiff_build.sh's own host-write -> container-read edges.
#   LIBGCC   (task #893, RULING #8206) tools/ee/libgcc_transcription_scan.py
#            over going-decompiled/src/<region>, comments and string literals
#            stripped first (doc comments DESCRIBE GCC's algorithm; that is not
#            a transcription): FAILS on a definition of a libgcc.a entry point
#            by its libgcc name, or on any GCC-only identifier (DIunion,
#            umul_ppmm, USItype, tfraction ...) in code. `extern` declarations
#            pass. The TARGET_NATIVE spec one-liners (`return a * b;` class) are
#            allowed and LISTED as KEEP-SPEC; in the EE arm every definition
#            fails. Blind to a transcription under a func_<addr> name that uses
#            no listed identifier (the script header says so).
#   TREE     the ROW is tied to the tree it was built from (task #457, #451 gap
#            1): do_build records HEAD^{tree}, a hash of the WHOLE working tree
#            (tracked + modified + untracked, .gitignore honoured) and the dirty
#            count in .gate_landing/<region>/built_tree.txt; --no-build re-hashes
#            and FAILS on any mismatch. RULING #7208 condition 1 is discharged
#            ONLY by the BUILDING form on a 0-dirty tree at the SHA-named landing
#            — the summary says so whenever this run is not that.
#
#   tools/ee/landing_gate.sh <region> [--strict]  all of the above; exit 0 = PASS
#   tools/ee/landing_gate.sh <region> --no-build  reuse the outputs of an earlier
#                                                 build — only if the working
#                                                 tree still hashes to the one
#                                                 that build recorded AND the
#                                                 gitignored link inputs still
#                                                 hash to what it linked with
#                                                 (no split; row mtime check
#                                                 still applies)
#   tools/ee/landing_gate.sh --selftest [region]  seed every check's failing arm
#                                                 and require it to fire, then
#                                                 run the real gate and require
#                                                 PASS (default region usa)
#
# A baseline HIGHER than the observation (a count, or a member no longer
# observed) is a WARN naming the exact value/member to set (#451 gap 2: the
# never-grow rule let baselines drift high silently). WARNs are counted in the
# summary and exit 0; --strict turns every WARN into a FAIL so a landing brief
# can require the baselines be lowered in the same landing.
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
#   - a --use-cache split's damage (FACT #7248): the building form re-splits
#     WITHOUT the cache and requires the fixed point, but --no-build measures
#     whatever asm the working tree holds;
#   - the unit objdiff gate's fuzzy rows (objdiff_build.sh + unit_report.sh) —
#     a byte-exact ROM makes them redundant for USA and they are not run here;
#   - EU bytes — EU has no link, so its row is a link-property, not a cmp.
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"; cd "$ROOT"
HERE="tools/ee"
EE_CTX="${EE_DOCKER_CONTEXT:-colima-ee-x86}"
BASE_DIR="$HERE/landing_baseline"
SHADOW_CLASSES="CLASS1 CLASS2 CLASS3 NOTARGET"
PYTHON=".venv-decomp/bin/python"

usage() { sed -n '2,111p' "$0" | sed 's/^# \{0,1\}//'; exit 2; }
say()  { printf '%s\n' "$*"; }
fail() { say "FAIL $*"; FAILED=$((FAILED+1)); }
ok()   { say "OK   $*"; }
# warn: a baseline that is stale-HIGH. Counted; a FAIL under --strict.
warn() { if [ "$STRICT" = 1 ]; then say "FAIL(strict) $*"; FAILED=$((FAILED+1)); else say "WARN $*"; fi; WARNED=$((WARNED+1)); }
FAILED=0; WARNED=0; STRICT=${LANDING_GATE_STRICT:-0}

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
  for f in "$RETAIL_ELF" "$RETAIL_ROM" "$YAML" "$LDSCRIPT" "$BASE_DIR/noprovide_$REGION.txt" "$BASE_DIR/orphans_$REGION.txt" "$BASE_DIR/orphans_latent_$REGION.txt" $(for c in $SHADOW_CLASSES; do shadow_baseline_file "$c" "$REGION" "$BASE_DIR"; done); do
    [ -f "$f" ] || { say "landing_gate: missing input $f"; exit 2; }
  done
}

# The gitignored link inputs the SPLIT step regenerates and the TREE record
# ties the ROW to (relative to $BUILD): build.sh `-T` and `-I` consumers.
GATE_INPUTS="undefined_syms_auto.txt include/macro.inc include/labels.inc include/include_asm.h"

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

# shadow_baseline_file CLASS REGION BASEDIR — the committed member file.
shadow_baseline_file() { printf '%s/shadow_%s_%s.txt\n' "$3" "$(printf '%s' "$1" | tr 'A-Z' 'a-z')" "$2"; }

# shadow_members SCANFILE CLASS — the class's rows as MEMBERS: the location's
# :<line> dropped (an edit above a site moves the line, not the duality), the
# class word dropped (one file per class), LC_ALL=C sorted, unique. Measured at
# c06bd392: members == rows in every class of both regions (t464, P2).
shadow_members() { /usr/bin/grep "^$2 " "$1" | cut -d' ' -f2- | sed -E 's/^([^ ]+):[0-9]+ /\1 /' | LC_ALL=C sort -u; }

# shadow_compare REGION SCANFILE BASEDIR — per class, the observed member set vs
# landing_baseline/shadow_<class>_<region>.txt: a NEW member is a FAIL naming
# it, a member no longer observed a WARN naming it (lower the baseline in this
# landing, RULING #7317); the row count is a summary only.
shadow_compare() {
  local region=$1 scan=$2 basedir=$3 cls n b base obs grew gone
  say "== SHADOW [$region]: name dualities (scan: $scan; member files: $basedir/shadow_<class>_$region.txt)"
  for cls in $SHADOW_CLASSES; do
    n=$(/usr/bin/grep -c "^$cls " "$scan" || true)
    base=$(shadow_baseline_file "$cls" "$region" "$basedir")
    obs="$OUT/shadow_$(printf '%s' "$cls" | tr 'A-Z' 'a-z').txt"; shadow_members "$scan" "$cls" > "$obs"
    if [ ! -f "$base" ]; then fail "$cls [$region]: no member baseline $base"; continue; fi
    b=$(wc -l < "$base" | tr -d ' ')
    grew=$(LC_ALL=C comm -23 "$obs" <(LC_ALL=C sort -u "$base"))
    gone=$(LC_ALL=C comm -13 "$obs" <(LC_ALL=C sort -u "$base"))
    if [ -n "$grew" ]; then
      fail "$cls [$region]: NEW $(printf '%s\n' "$grew" | wc -l | tr -d ' ') member(s) not in $base ($n rows observed, $b baselined — a landing may lower this set, never grow it): $(printf '%s' "$grew" | tr '\n' ';' | sed 's/;$//; s/;/ ; /g')"
    else
      ok "$cls [$region]: $n rows, $(wc -l < "$obs" | tr -d ' ') members within baseline ($b)"
    fi
    [ -n "$gone" ] && warn "$cls [$region]: $(printf '%s\n' "$gone" | wc -l | tr -d ' ') baseline member(s) no longer observed — lower the baseline in this landing, remove from $base: $(printf '%s' "$gone" | tr '\n' ';' | sed 's/;$//; s/;/ ; /g')"
  done
  say "     members:"; sed 's/^/       /' "$scan"
}

check_shadow() {  # check_shadow REGION [TREE] [BASEDIR]
  local region=$1 tree=${2:-.} basedir=${3:-$BASE_DIR}
  local scan="$OUT/shadow_scan.txt"
  shadow_scan "$region" "$tree" "$scan" || { fail "shadow_scan2.sh could not run"; return; }
  shadow_compare "$region" "$scan" "$basedir"
}

# --------------------------------------------------------------- ORPHAN ----
# member_compare LABEL OBSERVED_FILE BASELINE_FILE — a sorted member set may
# shrink against its baseline, never grow; a baseline member no longer observed
# is a WARN naming it (stale-high).
member_compare() {
  local label=$1 obs=$2 base=$3
  local grew; grew=$(LC_ALL=C comm -23 <(LC_ALL=C sort -u "$obs") <(LC_ALL=C sort -u "$base"))
  local gone; gone=$(LC_ALL=C comm -13 <(LC_ALL=C sort -u "$obs") <(LC_ALL=C sort -u "$base"))
  local n; n=$(wc -l < "$obs" | tr -d ' '); local b; b=$(wc -l < "$base" | tr -d ' ')
  if [ -n "$grew" ]; then fail "$label GREW ($n vs baseline $b): NEW $(printf '%s ' $grew)"; else ok "$label within baseline ($n of $b)"; fi
  [ -n "$gone" ] && warn "$label baseline has $(printf '%s\n' $gone | wc -l | tr -d ' ') member(s) no longer observed — remove from $base: $(printf '%s ' $gone)"
  return 0
}

check_orphans() {  # check_orphans REGION [TREE] [BASEDIR]
  local region=$1 tree=${2:-.} basedir=${3:-$BASE_DIR}
  local scan="$OUT/orphan_scan.txt"
  say "== ORPHAN [$region]: blanket-only tokens (scan: $scan)"
  bash "$HERE/blanket_orphans.sh" "$region" --root "$tree" > "$scan" 2> "$scan.err" || { fail "blanket_orphans.sh could not run: $(head -c 300 "$scan.err")"; return; }
  /usr/bin/grep -E '^ORPHAN ' "$scan" | awk '{print $2}' > "$scan.live"
  /usr/bin/grep -E '^ORPHAN_LATENT ' "$scan" | awk '{print $2}' > "$scan.latent"
  member_compare "ORPHAN [$region] (no holder — undefined at link)" "$scan.live" "$basedir/orphans_$region.txt"
  member_compare "ORPHAN_LATENT [$region] (held only by nonmatchings func_*.s)" "$scan.latent" "$basedir/orphans_latent_$region.txt"
  say "     members:"; sed 's/^/       /' "$scan"
}

# --------------------------------------------------------------- LIBGCC ----
# check_libgcc REGION [SRCDIR] — no GCC runtime source text in game C (#893).
check_libgcc() {
  local region=$1 src=${2:-going-decompiled/src/$1}
  local scan="$OUT/libgcc_scan.txt"
  say "== LIBGCC [$region]: no libgcc source in game C, RULING #8206 (scan: $scan)"
  python3 "$HERE/libgcc_transcription_scan.py" "$src" > "$scan" 2>&1; local rc=$?
  local nk nf; nk=$(/usr/bin/grep -c '^KEEP-SPEC ' "$scan" || true); nf=$(/usr/bin/grep -cE '^(DEF|IDENT) ' "$scan" || true)
  case $rc in
    0) ok "no libgcc definition or GCC-only identifier in code under $src ($nk TARGET_NATIVE spec one-liner(s) allowed, listed)" ;;
    1) fail "LIBGCC [$region]: $nf member(s) in code under $src: $(/usr/bin/grep -E '^(DEF|IDENT) ' "$scan" | head -20 | tr '\n' ';')" ;;
    *) fail "libgcc_transcription_scan.py could not run (rc $rc): $(head -c 300 "$scan")" ;;
  esac
  say "     members:"; sed 's/^/       /' "$scan"
}

# ----------------------------------------------------------------- TREE ----
# worktree_hash [OVERLAY_PATH OVERLAY_FILE] — one hash for the WHOLE working
# tree: a copy of the index with `git add -A` applied (modified + untracked,
# .gitignore honoured, so .gate_landing/ outputs do not move it), written as a
# tree object. The optional overlay substitutes OVERLAY_FILE's content at
# OVERLAY_PATH without touching the working tree (the selftest's #451 edit).
worktree_hash() {
  local idx; idx=$(mktemp); cp "$(git rev-parse --git-path index)" "$idx"
  GIT_INDEX_FILE="$idx" git add -A >/dev/null 2>&1
  if [ -n "${1:-}" ]; then
    local blob; blob=$(git hash-object -w "$2")
    GIT_INDEX_FILE="$idx" git update-index --add --cacheinfo "100644,$blob,$1"
  fi
  GIT_INDEX_FILE="$idx" git write-tree; rm -f "$idx"
}

# inputs_rows — one `input <name> <bytes> <sha256>` row per gitignored link
# input of $BUILD (absent -> `input <name> absent -`), the members the TREE
# hash cannot see (FACT #7324).
inputs_rows() {
  local f
  for f in $GATE_INPUTS; do
    if [ -f "$BUILD/$f" ]; then printf 'input %s %s %s\n' "$f" "$(wc -c < "$BUILD/$f" | tr -d ' ')" "$(shasum -a 256 "$BUILD/$f" | cut -d' ' -f1)"; else printf 'input %s absent -\n' "$f"; fi
  done
}

record_tree() {  # record_tree OUTFILE — what the build about to run is built from
  { printf 'head=%s\nhead_tree=%s\nwork_tree=%s\ndirty=%s\n' "$(git rev-parse HEAD)" "$(git rev-parse 'HEAD^{tree}')" "$(worktree_hash)" "$(git status --porcelain --no-renames | wc -l | tr -d ' ')"; inputs_rows; } > "$1"
}

# check_inputs RECORD — the gitignored link inputs now must be the ones the
# recorded build linked with (size + sha256 per file, named on mismatch).
check_inputs() {
  local rec=$1 f want now
  for f in $GATE_INPUTS; do
    want=$(awk -v f="$f" '$1=="input" && $2==f {print $3, $4}' "$rec")
    now=$(inputs_rows | awk -v f="$f" '$2==f {print $3, $4}')
    if [ -z "$want" ]; then fail "inputs: $rec has no row for $BUILD/$f — the outputs were built by a gate that did not record its link inputs; rebuild (drop --no-build)"
    elif [ "$now" = "$want" ]; then ok "inputs: $BUILD/$f ${now% *} B sha256 $(printf '%s' "${now#* }" | cut -c1-12)… == the built record"
    else fail "inputs: ROW was linked with $BUILD/$f ${want% *} B sha256 $(printf '%s' "${want#* }" | cut -c1-12)…, the file now is ${now% *} B sha256 $(printf '%s' "${now#* }" | cut -c1-12)… — a stale or hybrid gitignored input (FACT #7324); rebuild (drop --no-build)"; fi
  done
}

# check_syms_nonempty FILE — FACT #7150: a 0 B undefined_syms_auto.txt is what a
# cached configure.py leaves after a split; the link then has no undefined-
# symbol script and may still succeed — silently, with different bytes.
check_syms_nonempty() {
  local f=$1 n
  [ -f "$f" ] || { fail "$f absent after the split"; return; }
  n=$(wc -c < "$f" | tr -d ' ')
  if [ "$n" -gt 0 ]; then ok "$f is $n B (non-empty)"; else fail "$f is 0 B after the split (FACT #7150: a cached configure.py ran after a split, or the split wrote nothing) — the link would run without its undefined-symbol script"; fi
}

# split_inputs — the building form's SPLIT step: configure.py --region $REGION
# (full split, no cache) regenerates $BUILD/undefined_syms_auto.txt and
# $BUILD/include/* from the committed inputs. It must be a FIXED POINT of the
# working tree (RULING #7208 condition 5): the whole-worktree hash before ==
# after, else FAIL naming every rewritten tracked path. Outputs after this step
# are measured on the RE-SPLIT tree, so a non-fixed-point tree is also dirty.
split_inputs() {
  say "== SPLIT [$REGION]: $PYTHON scripts/configure.py --region $REGION (full split) regenerates the gitignored link inputs; must be a fixed point of the tree"
  [ -x "$PYTHON" ] || { say "landing_gate: no $PYTHON — provision .venv-decomp (CLAUDE.md, Build) before the building form can regenerate the link inputs"; exit 2; }
  local before; before=$(worktree_hash); local t0; t0=$(date +%s)
  "$PYTHON" scripts/configure.py --region "$REGION" > "$OUT/split.log" 2>&1; local rc=$?
  local after; after=$(worktree_hash); local dt=$(( $(date +%s) - t0 ))
  [ $rc = 0 ] || { fail "SPLIT [$REGION]: configure.py rc=$rc ($OUT/split.log): $(tail -3 "$OUT/split.log" | tr '\n' ' ')"; }
  if [ "$after" = "$before" ]; then
    ok "SPLIT [$REGION]: fixed point — the split rewrote 0 tracked or untracked paths (worktree $before, ${dt}s)"
  else
    local paths; paths=$(git diff-tree -r --name-only "$before" "$after")
    fail "SPLIT [$REGION]: NOT a fixed point of the tree — the split rewrote $(printf '%s\n' "$paths" | wc -l | tr -d ' ') path(s) (worktree $before -> $after, ${dt}s): $(printf '%s' "$paths" | tr '\n' ' ')"
  fi
  check_syms_nonempty "$BUILD/undefined_syms_auto.txt"
  say "     inputs: $(inputs_rows | awk '{printf "%s %s B sha256 %s… · ", $2, $3, substr($4,1,12)}' | sed 's/ · $//')"
}

# check_tree RECORD [OVERLAY_PATH OVERLAY_FILE] — the working tree now must
# hash to the one RECORD was written from, or the ROW measures another tree.
check_tree() {
  local rec=$1; shift
  say "== TREE [$REGION]: the ROW must be for THIS tree ($rec)"
  [ -f "$rec" ] || { fail "no built_tree record at $rec — the outputs were not built by this gate's building form; run without --no-build"; return; }
  local want; want=$(rowval work_tree "$rec"); local wanth; wanth=$(rowval head_tree "$rec"); local wantd; wantd=$(rowval dirty "$rec")
  local now; now=$(worktree_hash "$@"); local nowh; nowh=$(git rev-parse 'HEAD^{tree}'); local nowd; nowd=$(git status --porcelain --no-renames | wc -l | tr -d ' ')
  if [ "$now" = "$want" ] && [ "$nowh" = "$wanth" ]; then
    ok "tree: worktree $now (HEAD^{tree} $nowh, $nowd dirty) == the built tree"
  else
    fail "ROW is for tree $want (HEAD^{tree} $wanth, +$wantd dirty), worktree is tree $now (HEAD^{tree} $nowh, +$nowd dirty) — rebuild (drop --no-build)"
  fi
  check_inputs "$rec"
}

# ---------------------------------------------------------------- BUILD ----
do_build() {
  say "== BUILD [$REGION]: build.sh in $EE_CTX, objects and link outputs wiped first"
  date +%s > "$OUT/build_start"
  record_tree "$OUT/built_tree.txt"
  say "     built from: $(tr '\n' ' ' < "$OUT/built_tree.txt")"
  # SYNC: the split-regenerated link inputs, host md5 -> verified in-container
  # first (mount_sync.sh check retries a stale sshfs view, rc 9 names the file).
  local f sync=""
  for f in $GATE_INPUTS; do
    [ -f "$BUILD/$f" ] && sync="$sync sh tools/ee/mount_sync.sh check $BUILD/$f $(sh "$HERE/mount_sync.sh" md5 "$BUILD/$f") &&"
  done
  in_vm "B=$BUILD; rm -rf \$B/going-decompiled \$B/$BASENAME.elf \$B/$BASENAME.lma.elf \$B/$BASENAME.rom \$B/ld.log \$B/ld.lma.log \$B/$BASENAME.map \$B/all_addr_syms.ld;$sync sh tools/ee/build.sh $REGION" > "$OUT/build.log" 2>&1
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
check_row() {  # check_row ROWFILE START_EPOCH [EU_LDUNDEF_BASELINE]
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
    local ldbase="${3:-$BASE_DIR/ldundef_eu.txt}"
    if [ -f "$ldbase" ]; then
      local grew; grew=$(LC_ALL=C comm -23 "$row.undefined" <(LC_ALL=C sort -u "$ldbase"))
      local n; n=$(wc -l < "$row.undefined" | tr -d ' ')
      if [ -n "$grew" ]; then fail "EU ld.log undefined set GREW ($n vs baseline $(wc -l < "$ldbase" | tr -d ' ')): $(printf '%s ' $grew)"; else ok "EU ld.log undefined set within baseline ($n members: $(tr '\n' ' ' < "$row.undefined"))"; fi
      local gone; gone=$(LC_ALL=C comm -13 "$row.undefined" <(LC_ALL=C sort -u "$ldbase"))
      [ -n "$gone" ] && warn "EU ld.log baseline has $(printf '%s\n' $gone | wc -l | tr -d ' ') member(s) no longer undefined — remove from $ldbase: $(printf '%s ' $gone)"
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
  if [ -n "$grew" ]; then fail "PROVIDE-held set GREW vs $baseline ($(wc -l < "$baseline" | tr -d ' ')): NEW $(printf '%s ' $grew)"; else ok "PROVIDE-held set within baseline ($n of $(wc -l < "$baseline" | tr -d ' '))"; fi
  [ -n "$gone" ] && warn "PROVIDE-held baseline has $(printf '%s\n' $gone | wc -l | tr -d ' ') member(s) no longer held — remove from $baseline: $(printf '%s ' $gone)"
  say "     members -> $OUT/noprovide.held"
}

# ----------------------------------------------------------------- GATE ----
run_gate() {  # run_gate REGION [--no-build] [--strict]
  region_vars "$1"; shift; local build=1
  while [ $# -gt 0 ]; do case "$1" in --no-build) build=0 ;; --strict) STRICT=1 ;; '') ;; *) say "unknown option $1"; exit 2 ;; esac; shift; done
  FAILED=0; WARNED=0
  local dirty; dirty=$(git status --porcelain --no-renames | wc -l | tr -d ' ')
  say "#### landing_gate $REGION at $(git rev-parse --short HEAD) ($dirty dirty paths), VM $EE_CTX, $(date -u +%FT%TZ)$([ "$STRICT" = 1 ] && echo ', --strict')"
  say "     RULING #7208 condition 1 is discharged only by the BUILDING form on a 0-dirty tree at the SHA-named landing; this run is $([ $build = 1 ] && echo building || echo '--no-build'), $dirty dirty"
  [ $build = 1 ] && split_inputs   # first: every check below measures the re-split tree
  check_flags
  check_shadow "$REGION"
  check_orphans "$REGION"
  check_libgcc "$REGION"
  if [ $build = 1 ]; then
    do_build
    check_tree "$OUT/built_tree.txt"   # the tree did not move during the build
  else
    say "== BUILD [$REGION]: skipped (--no-build), start epoch taken from $OUT/build_start"
    check_tree "$OUT/built_tree.txt"
  fi
  local start; start=$(cat "$OUT/build_start" 2>/dev/null || echo 0)
  measure_row "$BUILD/$BASENAME.rom" "$BUILD/$BASENAME.elf" "$BUILD/ld.log" "$OUT/row.txt"
  check_row "$OUT/row.txt" "$start"
  check_noprovide
  local verdict; verdict=$([ $FAILED = 0 ] && echo PASS || echo "FAIL ($FAILED)")
  [ $WARNED -gt 0 ] && verdict="$verdict ($WARNED warning$([ $WARNED = 1 ] || echo s)$([ "$STRICT" = 1 ] && echo ', counted as FAIL under --strict'))"
  say "#### landing_gate $REGION: $verdict"
  if [ $build = 0 ] || [ "$dirty" != 0 ]; then say "#### NOTE: RULING #7208 condition 1 is NOT discharged by this run ($([ $build = 0 ] && echo '--no-build')$([ $build = 0 ] && [ "$dirty" != 0 ] && echo ', ')$([ "$dirty" != 0 ] && echo "$dirty dirty paths")) — it needs the building form on a 0-dirty tree"; fi
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
  local fired; fired=$(/usr/bin/grep -cE '^FAIL (CLASS1|CLASS2|CLASS3|NOTARGET) \[[a-z]+\]: NEW 1 member' "$T/shadow.txt" || true)
  # the planted C definition has no .s under either name -> NOTARGET grows too: 4 classes
  if [ "$fired" = 4 ] && /usr/bin/grep -q '^FAIL CLASS2 .*: .*func_00DEAD04 -> T449SeedC2' "$T/shadow.txt"; then ok "fired: $(/usr/bin/grep -E '^FAIL' "$T/shadow.txt" | sed -E 's/ not in .*: / NEW: /' | tr '\n' ';')"; else say "SELFTEST-FAIL shadow controls: $fired of 4 classes reported a NEW member"; /usr/bin/grep -E '^(OK|FAIL)' "$T/shadow.txt"; bad=1; fi

  say "-- (3) SPLIT + BUILD once (real arms; the PROVIDE and ROW controls relink/measure against its objects)"
  FAILED=0; split_inputs > "$T/split_real.txt"; /usr/bin/grep -E '^(OK|FAIL)' "$T/split_real.txt" | sed 's/^/     /'
  [ "$FAILED" = 0 ] || { say "SELFTEST-BROKEN: the real split is not a fixed point of this tree — fix the tree before trusting any arm below"; bad=1; }
  do_build; local start; start=$(cat "$OUT/build_start")
  measure_row "$BUILD/$BASENAME.rom" "$BUILD/$BASENAME.elf" "$BUILD/ld.log" "$OUT/row.txt"

  say "-- (4) ROW: the row's failing arms (usa: flipped rom copy, cmp 4 / wrong sha1; eu: one member dropped from the ld.log baseline) + a stale-log arm"
  FAILED=0; check_row "$OUT/row.txt" $((start + 100000)) > "$T/row_stale.txt"
  /usr/bin/grep -q 'STALE log' "$T/row_stale.txt" && ok "fired: stale-mtime arm -> $(/usr/bin/grep -c '^FAIL' "$T/row_stale.txt") FAIL row(s)" || { say "SELFTEST-FAIL stale-log arm did not fire"; bad=1; }
  if [ "$REGION" = usa ]; then
    FAILED=0; check_row "$OUT/row.txt" "$start" > "$T/row_clean.txt"
    /usr/bin/grep -q '^OK   negative control: 4 bytes flipped' "$T/row_clean.txt" && ok "fired: $(/usr/bin/grep '^OK   negative control' "$T/row_clean.txt" | sed 's/^OK   //')" || { say "SELFTEST-FAIL flipped-copy control absent"; cat "$T/row_clean.txt"; bad=1; }
    sed 's/^cmp_count=.*/cmp_count=4/; s/^sha1_built=.*/sha1_built=deadbeef/' "$OUT/row.txt" > "$T/row_bad.txt"; cp "$OUT/row.txt.undefined" "$T/row_bad.txt.undefined"
    FAILED=0; check_row "$T/row_bad.txt" "$start" > "$T/row_bad_eval.txt"
    [ "$FAILED" -ge 2 ] && ok "fired: a row with cmp 4 / wrong sha1 -> $FAILED FAIL rows" || { say "SELFTEST-FAIL bad-row arm: $FAILED"; cat "$T/row_bad_eval.txt"; bad=1; }
  else
    sed '1d' "$BASE_DIR/ldundef_eu.txt" > "$T/ldundef_short.txt"; local dropped1; dropped1=$(head -1 "$BASE_DIR/ldundef_eu.txt")
    FAILED=0; check_row "$OUT/row.txt" "$start" "$T/ldundef_short.txt" > "$T/row_ldundef.txt"
    /usr/bin/grep -q "GREW.*$dropped1" "$T/row_ldundef.txt" && ok "fired: EU ld.log baseline minus '$dropped1' -> $(/usr/bin/grep -oE 'undefined set GREW \([^)]*\)' "$T/row_ldundef.txt")" || { say "SELFTEST-FAIL EU ld.log baseline arm did not fire"; cat "$T/row_ldundef.txt"; bad=1; }
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

  say "-- (7) TREE: the #451 reproduction — one C function appended to a src/$REGION .c as an OVERLAY (the working tree is untouched) -> the --no-build tree check must FAIL naming both trees; without the overlay it must pass"
  local V; V=$(git ls-files "going-decompiled/src/$REGION" | /usr/bin/grep '\.c$' | LC_ALL=C sort | head -1)
  { cat "$V"; printf '\nvoid T457Probe(void) { volatile int x = 457; x++; }\n'; } > "$T/tree_overlay.c"
  cmp -s "$V" "$T/tree_overlay.c" && { say "SELFTEST-BROKEN: overlay identical to $V"; bad=1; }
  FAILED=0; check_tree "$OUT/built_tree.txt" "$V" "$T/tree_overlay.c" > "$T/tree_dirty.txt"
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q '^FAIL ROW is for tree [0-9a-f]* .*, worktree is tree [0-9a-f]* ' "$T/tree_dirty.txt"; then ok "fired: $(/usr/bin/grep '^FAIL ROW' "$T/tree_dirty.txt" | sed -E 's/ \(HEAD[^)]*\)//g; s/ — rebuild.*//')"; else say "SELFTEST-FAIL tree check did not fire on the overlaid edit ($V):"; cat "$T/tree_dirty.txt"; bad=1; fi
  FAILED=0; check_tree "$OUT/built_tree.txt" > "$T/tree_clean.txt"
  if [ "$FAILED" = 0 ] && /usr/bin/grep -q '^OK   tree:' "$T/tree_clean.txt"; then ok "control: the unchanged tree passes ($(/usr/bin/grep -oE 'worktree [0-9a-f]{12}' "$T/tree_clean.txt" | head -1)...)"; else say "SELFTEST-FAIL the unchanged tree does not pass the tree check:"; cat "$T/tree_clean.txt"; bad=1; fi

  say "-- (8) STALE-HIGH: one extra member in a copy of shadow_class1_$REGION.txt, and one extra member in a copy of noprovide_$REGION.txt -> WARN naming the member (rc 0); under --strict -> FAIL"
  shadow_scan "$REGION" . "$T/shadow_real.txt" || { say "SELFTEST-BROKEN: shadow_scan on the real tree failed"; bad=1; }
  local HB="$T/base_high"; rm -rf "$HB"; cp -R "$BASE_DIR" "$HB"
  local extra="going-decompiled/src/$REGION/cod/015180.c func_00DEAD10 -> T464StaleHighMember"
  { cat "$(shadow_baseline_file CLASS1 "$REGION" "$BASE_DIR")"; printf '%s\n' "$extra"; } | LC_ALL=C sort -u > "$(shadow_baseline_file CLASS1 "$REGION" "$HB")"
  FAILED=0; WARNED=0; STRICT=0; shadow_compare "$REGION" "$T/shadow_real.txt" "$HB" > "$T/high_warn.txt"
  if [ "$FAILED" = 0 ] && [ "$WARNED" = 1 ] && /usr/bin/grep -q "^WARN CLASS1 \[$REGION\]: 1 baseline member(s) no longer observed — lower the baseline in this landing, remove from $HB/shadow_class1_$REGION.txt: $extra\$" "$T/high_warn.txt"; then ok "fired: $(/usr/bin/grep '^WARN' "$T/high_warn.txt" | sed -E 's/ remove from [^:]*:/ remove:/') (FAILED=$FAILED WARNED=$WARNED)"; else say "SELFTEST-FAIL stale-high shadow member did not WARN (FAILED=$FAILED WARNED=$WARNED):"; /usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/high_warn.txt"; bad=1; fi
  FAILED=0; WARNED=0; STRICT=1; shadow_compare "$REGION" "$T/shadow_real.txt" "$HB" > "$T/high_strict.txt"; STRICT=0
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^FAIL(strict) CLASS1 \[$REGION\]: 1 baseline member(s) no longer observed — lower the baseline" "$T/high_strict.txt"; then ok "fired: under --strict the same line is FAIL (FAILED=$FAILED)"; else say "SELFTEST-FAIL --strict did not turn the stale-high WARN into a FAIL (FAILED=$FAILED):"; /usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/high_strict.txt"; bad=1; fi
  { cat "$BASE_DIR/noprovide_$REGION.txt"; echo T457ExtraMember; } | LC_ALL=C sort -u > "$T/np_high.txt"
  FAILED=0; WARNED=0; STRICT=0; check_noprovide "$T/np_high.txt" > "$T/np_high_warn.txt"
  if [ "$FAILED" = 0 ] && [ "$WARNED" = 1 ] && /usr/bin/grep -q '^WARN PROVIDE-held baseline has 1 member(s) no longer held — remove from .*: T457ExtraMember' "$T/np_high_warn.txt"; then ok "fired: $(/usr/bin/grep '^WARN' "$T/np_high_warn.txt" | sed -E 's/ — remove from [^:]*:/ — remove:/')"; else say "SELFTEST-FAIL extra noprovide member did not WARN (FAILED=$FAILED WARNED=$WARNED):"; /usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/np_high_warn.txt"; bad=1; fi
  FAILED=0; WARNED=0; STRICT=1; check_noprovide "$T/np_high.txt" > "$T/np_high_strict.txt"; STRICT=0
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q '^FAIL(strict) PROVIDE-held baseline has 1 member(s) no longer held' "$T/np_high_strict.txt"; then ok "fired: under --strict the extra member is a FAIL (FAILED=$FAILED)"; else say "SELFTEST-FAIL --strict did not fail on the extra noprovide member (FAILED=$FAILED):"; /usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/np_high_strict.txt"; bad=1; fi

  say "-- (9) ORPHAN: the ORPHAN_LATENT token with the fewest holders has every holder func_*.s deleted in a scratch copy of asm/$REGION -> ORPHAN must GROW naming the token"
  local OT="$T/otree"; rm -rf "$OT"; mkdir -p "$OT/going-decompiled/asm" "$OT/going-decompiled/symbol_addrs"
  cp -R going-decompiled/src "$OT/going-decompiled/src"; cp -R "going-decompiled/symbol_addrs/$REGION" "$OT/going-decompiled/symbol_addrs/$REGION"; cp -R "going-decompiled/asm/$REGION" "$OT/going-decompiled/asm/$REGION"
  bash "$HERE/blanket_orphans.sh" "$REGION" > "$T/orphan_real.txt" 2>/dev/null
  # fewest holders first (holders=a.s,b.s -> count the commas), then by token
  local vrow; vrow=$(/usr/bin/grep '^ORPHAN_LATENT ' "$T/orphan_real.txt" | awk '{h=$NF; n=gsub(/,/,",",h); print n, $0}' | LC_ALL=C sort -k1,1n -k3,3 | head -1 | cut -d' ' -f2-)
  if [ -z "$vrow" ]; then say "SELFTEST-BROKEN: no ORPHAN_LATENT row on this tree to seed from"; bad=1; else
    local vtok; vtok=$(printf '%s' "$vrow" | awk '{print $2}'); local vholders; vholders=$(printf '%s' "$vrow" | sed 's/.*holders=//' | tr ',' ' ')
    local h vpaths=0
    for h in $vholders; do
      local f; for f in $(/usr/bin/find "$OT/going-decompiled/asm/$REGION/nonmatchings" -name "$h"); do rm -f "$f"; vpaths=$((vpaths+1)); done
    done
    [ "$vpaths" -gt 0 ] || { say "SELFTEST-BROKEN: none of '$vholders' found in the scratch copy"; bad=1; }
    FAILED=0; WARNED=0; STRICT=0; check_orphans "$REGION" "$OT" > "$T/orphan_seeded.txt"
    if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^FAIL ORPHAN \\[$REGION\\] .*GREW .*: NEW $vtok " "$T/orphan_seeded.txt"; then ok "fired: deleting $vpaths holder(s) [$vholders] -> $(/usr/bin/grep -oE "ORPHAN \\[$REGION\\] \\([^)]*\\) GREW \\([^)]*\\): NEW $vtok" "$T/orphan_seeded.txt")"; else say "SELFTEST-FAIL orphan control did not fire for $vtok / $vholders (FAILED=$FAILED):"; /usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/orphan_seeded.txt"; bad=1; fi
    /usr/bin/grep -q "^WARN ORPHAN_LATENT \\[$REGION\\] .* no longer observed — remove from .*: $vtok" "$T/orphan_seeded.txt" && ok "and the LATENT baseline reports $vtok stale-high (WARN)" || { say "SELFTEST-FAIL the LATENT set did not report $vtok as no longer observed"; bad=1; }
  fi

  say "-- (11) SWAP (FACT #7303): in a scratch tree one real CLASS1 shadow is FIXED (its INCLUDE_ASM arg renamed to the symbol_addrs name) and one NEW one planted — same count — the member check must FAIL naming ONLY the planted member and WARN the fixed one"
  local SW="$T/swaptree"; rm -rf "$SW"; mkdir -p "$SW/going-decompiled/asm/$REGION"
  cp -R going-decompiled/src "$SW/going-decompiled/src"; cp -R going-decompiled/symbol_addrs "$SW/going-decompiled/symbol_addrs"
  cp -R "going-decompiled/asm/$REGION/nonmatchings" "$SW/going-decompiled/asm/$REGION/nonmatchings"
  local vrow1; vrow1=$(/usr/bin/grep '^CLASS1 ' "$T/shadow_real.txt" | LC_ALL=C sort | head -1)
  local vfile vline vold vnew; vfile=$(printf '%s' "$vrow1" | awk '{print $2}' | cut -d: -f1); vline=$(printf '%s' "$vrow1" | awk '{print $2}' | cut -d: -f2); vold=$(printf '%s' "$vrow1" | awk '{print $3}'); vnew=$(printf '%s' "$vrow1" | awk '{print $5}')
  local vdir; vdir=$(sed -n "${vline}p" "$SW/$vfile" | sed -E 's/.*INCLUDE_ASM\("([^"]+)".*/\1/')
  if [ -z "$vrow1" ] || [ -z "$vdir" ]; then say "SELFTEST-BROKEN: no CLASS1 row to swap from ($vrow1)"; bad=1; else
    sed -i.bak "${vline}s/${vold})/${vnew})/" "$SW/$vfile"; rm -f "$SW/$vfile.bak"
    /usr/bin/grep -q "INCLUDE_ASM(\"$vdir\", $vold)" "$SW/$vfile" && { say "SELFTEST-BROKEN: $vold still INCLUDE_ASM'd in the scratch $vfile:$vline"; bad=1; }
    printf '\n/* t464 selftest swap seed */\nINCLUDE_ASM("%s", func_00DEAD00);\n' "$vdir" >> "$SW/$vfile"
    printf 'T464SwapSeed = 0x00DEAD00; // type:func\n' >> "$SW/going-decompiled/symbol_addrs/$REGION/symbol_addrs.txt"
    FAILED=0; WARNED=0; STRICT=0; check_shadow "$REGION" "$SW" > "$T/swap.txt"; cp "$OUT/shadow_scan.txt" "$T/shadow_scan_swapped.txt"
    local nreal nswap; nreal=$(/usr/bin/grep -c '^CLASS1 ' "$T/shadow_real.txt" || true); nswap=$(/usr/bin/grep -c '^CLASS1 ' "$T/shadow_scan_swapped.txt" || true)
    [ "$nreal" = "$nswap" ] || { say "SELFTEST-BROKEN: swap changed the CLASS1 count ($nreal -> $nswap) — not a same-count swap"; bad=1; }
    if [ "$FAILED" = 1 ] && [ "$WARNED" = 1 ] && /usr/bin/grep -q "^FAIL CLASS1 \[$REGION\]: NEW 1 member(s) not in .*: $vfile func_00DEAD00 -> T464SwapSeed\$" "$T/swap.txt" && ! /usr/bin/grep -q "^FAIL.*$vold" "$T/swap.txt" && /usr/bin/grep -q "^WARN CLASS1 \[$REGION\]: 1 baseline member(s) no longer observed — lower the baseline in this landing, remove from .*: $vfile $vold -> $vnew\$" "$T/swap.txt"; then ok "fired: same count ($nswap == $nreal) and $(/usr/bin/grep '^FAIL CLASS1' "$T/swap.txt" | sed -E 's/ not in [^(]*\(/ (/') ; WARN gone: $vfile $vold -> $vnew"; else say "SELFTEST-FAIL swap arm (FAILED=$FAILED WARNED=$WARNED, count $nreal -> $nswap):"; /usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/swap.txt"; bad=1; fi
  fi

  say "-- (12) INPUTS (FACT #7324/#7150): $BUILD/undefined_syms_auto.txt truncated to 0 B after the build -> the --no-build tree check must FAIL naming it and the non-empty check must FAIL; the SPLIT step must regenerate it to the recorded sha256"
  local SY="$BUILD/undefined_syms_auto.txt"; cp "$SY" "$T/syms_saved.txt"; : > "$SY"
  FAILED=0; WARNED=0; STRICT=0; check_tree "$OUT/built_tree.txt" > "$T/inputs_zero.txt"
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^FAIL inputs: ROW was linked with $SY [1-9][0-9]* B sha256 [0-9a-f]*…, the file now is 0 B sha256 e3b0c44298fc…" "$T/inputs_zero.txt"; then ok "fired: $(/usr/bin/grep '^FAIL inputs' "$T/inputs_zero.txt" | sed -E 's/ — a stale.*//')"; else say "SELFTEST-FAIL the 0 B undefined_syms_auto.txt did not fail the inputs check (FAILED=$FAILED):"; /usr/bin/grep -E '^(OK|FAIL)' "$T/inputs_zero.txt"; bad=1; fi
  FAILED=0; check_syms_nonempty "$SY" > "$T/inputs_nonempty.txt"
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^FAIL $SY is 0 B after the split" "$T/inputs_nonempty.txt"; then ok "fired: $(/usr/bin/grep '^FAIL' "$T/inputs_nonempty.txt" | sed -E 's/ \(FACT.*//')"; else say "SELFTEST-FAIL the 0 B file passed check_syms_nonempty (FAILED=$FAILED):"; cat "$T/inputs_nonempty.txt"; bad=1; fi
  FAILED=0; WARNED=0; split_inputs > "$T/inputs_regen.txt"
  if [ "$FAILED" = 0 ] && cmp -s "$SY" "$T/syms_saved.txt" && /usr/bin/grep -q '^OK   SPLIT .*fixed point' "$T/inputs_regen.txt"; then ok "regenerated: $(/usr/bin/grep -oE "^OK   $SY is [0-9]+ B" "$T/inputs_regen.txt") — byte-identical to the file the build linked with (cmp); $(/usr/bin/grep -oE 'fixed point[^,]*, [0-9]+s' "$T/inputs_regen.txt" | sed 's/fixed point — //')"; else say "SELFTEST-FAIL the split did not regenerate $SY to the linked bytes (FAILED=$FAILED):"; /usr/bin/grep -E '^(OK|FAIL)' "$T/inputs_regen.txt"; cp "$T/syms_saved.txt" "$SY"; bad=1; fi
  FAILED=0; WARNED=0; check_tree "$OUT/built_tree.txt" > "$T/inputs_restored.txt"
  if [ "$FAILED" = 0 ] && /usr/bin/grep -q "^OK   inputs: $SY .* == the built record" "$T/inputs_restored.txt"; then ok "control: after the regeneration the inputs check passes again"; else say "SELFTEST-FAIL inputs check does not pass on the regenerated file (FAILED=$FAILED):"; /usr/bin/grep -E '^(OK|FAIL)' "$T/inputs_restored.txt"; bad=1; fi

  say "-- (13) SPLIT fixed point: a marker line appended to a tracked asm/$REGION .s the split owns -> split_inputs must FAIL naming the path; the split itself restores the file (the tree is clean again, checked)"
  # a code segment's .s: the split rewrites those every run (data/cod/000000.s, a textbin wrapper, it does NOT — first USA selftest, t464)
  local SF; SF=$(git ls-files "going-decompiled/asm/$REGION/text" | /usr/bin/grep '\.s$' | LC_ALL=C sort | head -1)
  if [ -z "$SF" ] || [ -n "$(git status --porcelain --no-renames -- "$SF")" ]; then say "SELFTEST-BROKEN: no clean tracked .s to seed the split arm ($SF)"; bad=1; else
    printf '\n# t464 selftest marker\n' >> "$SF"
    FAILED=0; WARNED=0; split_inputs > "$T/split_seeded.txt"
    if /usr/bin/grep -q '# t464 selftest marker' "$SF"; then say "SELFTEST-BROKEN: the split did not rewrite $SF — restoring it with git checkout"; git checkout -q -- "$SF"; bad=1; fi
    if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^FAIL SPLIT \[$REGION\]: NOT a fixed point of the tree — the split rewrote 1 path(s) .*: $SF *\$" "$T/split_seeded.txt" && [ -z "$(git status --porcelain --no-renames -- "$SF")" ]; then ok "fired: $(/usr/bin/grep '^FAIL SPLIT' "$T/split_seeded.txt" | sed -E 's/ \(worktree [^)]*\)//') ; $SF is clean again"; else say "SELFTEST-FAIL split fixed-point arm (FAILED=$FAILED, $SF status '$(git status --porcelain --no-renames -- "$SF")'):"; /usr/bin/grep -E '^(OK|FAIL)' "$T/split_seeded.txt"; bad=1; fi
  fi

  selftest_mount_sync "${MOUNT_SYNC_SH:-$HERE/mount_sync.sh}" "$T" || bad=1

  selftest_libgcc "$T" || bad=1

  say "-- (14) the real gate on this tree (--no-build, the build above) must PASS"
  STRICT=0
  if run_gate "$REGION" --no-build > "$T/gate.txt"; then ok "real gate PASS"; else say "SELFTEST-FAIL the real gate does not pass on this tree:"; /usr/bin/grep -E '^FAIL' "$T/gate.txt"; bad=1; fi
  say "     full gate output -> $T/gate.txt"
  say "#### landing_gate --selftest [$REGION]: $([ $bad = 0 ] && echo PASS || echo FAIL)"
  return $bad
}

# selftest_libgcc OUTDIR — arm (16), callable on its own after sourcing this
# file (`. tools/ee/landing_gate.sh; region_vars eu; selftest_libgcc /tmp/x`).
# Seeds the DANGEROUS class, not a stand-in: the libgcc2 `DIunion` __muldi3 body
# EU carried in its TARGET_NATIVE arm until #893 removed it, appended to a
# scratch copy of the region's src/. A seed in a comment would prove nothing —
# comments are stripped. Then the clean tree must pass.
selftest_libgcc() {
  local T="$1"; local L="$T/libgcc_src"; rm -rf "$L"; mkdir -p "$L"
  say "-- (16) LIBGCC (#893): the libgcc2 DIunion __muldi3 body EU removed in #893, re-inserted in a TARGET_NATIVE arm of a scratch src/$REGION copy -> must FAIL naming DEF __muldi3; the real tree must pass"
  cp -R "going-decompiled/src/$REGION/." "$L/"
  cat >> "$L/libgcc_seed.c" <<'SEED'
#ifdef TARGET_NATIVE
s64 __muldi3(s64 a, s64 b) {
    union { struct { s32 low; s32 high; } s; s64 ll; } w, uu, vv;
    uu.ll = a;
    vv.ll = b;
    w.ll = (s64)((u64)(u32)uu.s.low * (u32)vv.s.low);
    w.s.high += uu.s.low * vv.s.high + uu.s.high * vv.s.low;
    return w.ll;
}
#endif
SEED
  local b=0
  FAILED=0; check_libgcc "$REGION" "$L" > "$T/libgcc_seeded.txt"
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q '^       DEF libgcc_seed.c:2 __muldi3 (not-a-spec-one-liner)$' "$T/libgcc_seeded.txt"; then ok "fired: $(/usr/bin/grep '^FAIL LIBGCC' "$T/libgcc_seeded.txt")"; else say "SELFTEST-FAIL the seeded DIunion __muldi3 did not fail LIBGCC (FAILED=$FAILED):"; cat "$T/libgcc_seeded.txt"; b=1; fi
  FAILED=0; check_libgcc "$REGION" > "$T/libgcc_clean.txt"
  if [ "$FAILED" = 0 ]; then ok "control: the real src/$REGION passes LIBGCC"; else say "SELFTEST-FAIL the real src/$REGION fails LIBGCC:"; cat "$T/libgcc_clean.txt"; b=1; fi
  return $b
}

# selftest_mount_sync HELPER OUTDIR — arm (15), callable on its own after
# sourcing this file (`. tools/ee/landing_gate.sh; region_vars usa;
# selftest_mount_sync tools/ee/mount_sync.sh /tmp/x`). Seeds the trap the
# helper exists for: the host md5 is taken, THEN the file is shortened (what a
# stale sshfs view hands the container) -> `check` must FAIL rc 9 naming the
# file; the unshortened file must pass rc 0; and a file that is repaired after
# the first read must pass on a LATER try (the retry path). Run against a
# blinded copy of the helper (MOUNT_SYNC_SH=<copy> with the comparison removed)
# every arm here reads SELFTEST-FAIL and the function returns 1.
selftest_mount_sync() {
  local helper=$1 T=$2 bad=0 rc out
  say "-- (15) SYNC (#542): $helper — md5 taken, file truncated afterwards -> check must FAIL rc 9 naming the file; intact -> rc 0; repaired mid-retry -> rc 0 on a later try"
  local F="$T/sync_probe.txt"; local FD="$T/sync_probe_rel"
  printf 'landing_gate selftest 15: %s\n' "$(date +%s)" > "$F"; head -c 3000 /dev/urandom | base64 >> "$F"
  local want; want=$(sh "$helper" md5 "$F")
  [ -n "$want" ] || { say "SELFTEST-BROKEN: $helper md5 printed nothing for $F"; return 1; }
  # (a) intact: the container's md5sum of the same bytes must agree on try 1
  out=$(in_vm "MOUNT_SYNC_TRIES=3 MOUNT_SYNC_SLEEP=0.2 sh $helper check $F $want" 2>&1); rc=$?
  if [ $rc = 0 ] && [ -z "$out" ]; then ok "control: intact file agrees with the host md5 (rc 0, silent)"; else say "SELFTEST-FAIL intact file did not pass silently (rc $rc): $out"; bad=1; fi
  # (b) truncated AFTER the md5: what the stale mount serves. Shortened on the
  # host, so the container's every read is the short file -> FAIL naming it.
  head -c 1000 "$F" > "$F.short"; mv "$F.short" "$F"
  out=$(in_vm "MOUNT_SYNC_TRIES=3 MOUNT_SYNC_SLEEP=0.2 sh $helper check $F $want" 2>&1); rc=$?
  if [ $rc = 9 ] && printf '%s' "$out" | /usr/bin/grep -q "^MOUNT-SYNC FAIL: $F — container md5 [0-9a-f]* (1000 B) != host md5 $want after 3 tries"; then ok "fired: $(printf '%s' "$out" | /usr/bin/grep '^MOUNT-SYNC FAIL' | sed -E 's/; the VM.*//')"; else say "SELFTEST-FAIL truncated file did not FAIL rc 9 naming it (rc $rc): $out"; bad=1; fi
  # (c) the retry path: the check starts on the short file and the file is
  # restored 1.5 s later (inside the same container, so the timing is not at
  # the mercy of docker's start-up latency) -> it must agree on a try > 1, rc 0,
  # and SAY so on stderr.
  local want2; want2=$(printf 'landing_gate selftest 15: restored\n' | { command -v md5 >/dev/null 2>&1 && md5 -q || md5sum | cut -d' ' -f1; })
  out=$(in_vm "( sleep 1.5; printf 'landing_gate selftest 15: restored\\n' > $F ) & MOUNT_SYNC_TRIES=20 MOUNT_SYNC_SLEEP=0.5 sh $helper check $F $want2; rc=\$?; wait; exit \$rc" 2>&1); rc=$?
  if [ $rc = 0 ] && printf '%s' "$out" | /usr/bin/grep -q "^mount_sync: $F agreed with the host on try [2-9][0-9]* of 20"; then ok "fired: $(printf '%s' "$out" | /usr/bin/grep '^mount_sync:' | sed -E 's/ \(the mount.*//')"; else say "SELFTEST-FAIL retry path: rc $rc, output: $out"; bad=1; fi
  return $bad
}

# sourceable (`. tools/ee/landing_gate.sh`) for the individual check functions
if [ "${BASH_SOURCE[0]}" = "$0" ]; then
  case "${1:-}" in
    --selftest) selftest "${2:-usa}" ;;
    usa|eu) run_gate "$@" ;;
    *) usage ;;
  esac
fi

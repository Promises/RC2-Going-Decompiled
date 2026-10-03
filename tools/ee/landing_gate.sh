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
#   CC1ARGS  (task #1377, RULING #9004) the BUILD runs with EE_CC1_ARGLOG set,
#            so tools/ee/ee_cc1.sh logs every compile's arm, source and cc1
#            flag string to .gate_landing/<region>/cc1_args.tsv, and
#            tools/ee/cc1_arglog_check.py asserts that every sdk29 compile
#            carries `-O2 GFLAG CC1EXTRA` and every s136 compile `-O2 GFLAG
#            S136EXTRA -fopt-stack`, as build.sh's flag table assigns them
#            (derived from the table, not listed). It also asserts that every
#            source has one sdk29 line and that each unit has as many s136
#            lines as the selector has rows. On usa it adds RULING #9004's
#            floor: 1B4218's 2.9 compile keeps -fno-gcse, its s136 compiles carry
#            none, and 1DFF80 keeps it on both arms. WHY a row: #1364 measured
#            that unpinning BOTH 1B4218 arms leaves the unit object
#            BYTE-IDENTICAL, so the image cmp cannot see it (FACT #9034). Fails
#            CLOSED: an absent, empty or older-than-this-build log is a FAIL,
#            never "nothing to check". --no-build reads the recorded build's
#            log (the TREE row ties it to this tree). Adds no build: it reads
#            the one above.
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
#            over the VM's fuse.sshfs mount, which can serve a TRUNCATED view
#            of a just-rewritten file (FACT #7449, NOTE #7430): the class is
#            FACT #7464's grow-only cut at the VM's cached length, narrowed by
#            #7479 (cached = last-read size, multi-try heal), trigger = size
#            growth (FACT #8713); #7464 saw 0 old-content (STALE_OLD) reads.
#            md5, not size, is compared so reads outside that class (e.g.
#            FACT #8683's reader-dependent rows) are caught too. do_build
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
#   GMODEL   (task #919, FACT #8246) tools/ee/gmodel_scan.sh: FAILS when a unit
#            unit_flags.sh compiles at -G0 has a function compiled from C (no
#            INCLUDE_ASM line for it) whose ROM words are gp-relative (base
#            $28, decoded from the word, so raw `.word` functions count too).
#            cc1 cannot emit those at -G0, and GP differs by region, so such a
#            body can pass one region's cmp by luck. INCLUDE_ASM members with
#            gp words are listed as LATENT, never a failure.
#   NATIVE   (task #923) tools/native/check.sh over EVERY TARGET_NATIVE unit
#            under going-decompiled/src (both regions, whichever region this
#            run is for), BASE-RELATIVE: the base is `git merge-base HEAD
#            origin/master` (override LANDING_GATE_NATIVE_BASE=<rev>), its
#            src/ + include/ + tools/native/ extracted with git archive and
#            compiled by THIS tree's check.sh, so both arms are one instrument.
#            VACUOUS (task #992, watcher-2's ruling on FACT ledger-28525): once
#            origin/master has reached the tip, that merge-base IS HEAD, both
#            arms compile one tree and neither rule below can fire. When the
#            DEFAULT base resolves to HEAD and no NATIVE input (src/, include/,
#            tools/native/) is dirty, the row prints `NATIVE: base == tip,
#            VACUOUS` as a WARN — a FAIL under --strict. A post-landing
#            validator pins the master the landing was cut from — the parent
#            of the landing's FIRST commit (LANDING_GATE_NATIVE_BASE=<sha>);
#            the tip's own parent is that commit only for a one-commit landing
#            (task #1116). A pin EQUAL TO HEAD is treated exactly as the unpinned
#            case (task #1011, watcher-2's ruling on FACT ledger-28625): an
#            explicit pin to the tip is the same self-comparison with extra
#            steps, and pinning the tip is the easiest mistake a seat told to
#            "pin the base" can make — if it passed, the instruction would
#            manufacture false confidence.
#            NO C CHANGE (task #1034, watcher-2's ruling Q1 on FACT ledger-
#            28740): a base with ANOTHER sha whose NATIVE inputs (src/,
#            include/, tools/native/ but check.sh, which both arms take from
#            the tip) equal HEAD's, on a tree with none of them dirty, prints
#            `WARN NATIVE: no C change` — NOT counted and NOT a FAIL under
#            --strict (a tools-only landing has nothing native to regress), but
#            its row proves nothing and must not be quoted as a control. A base
#            == HEAD by sha stays the WARN/strict FAIL above. The run_gate
#            verdict line then ends ` (native: no C change)` (task #1065), on
#            the $tag line, never inside gate_verdict.
#            WRONG BASE (task #1065, #1073's spec gap): a PINNED base must be an
#            ancestor of HEAD AND ancestor-or-equal of merge-base(HEAD,
#            origin/master) — on the mainline at or before the fork point.
#            Else a WARN naming it, a FAIL under --strict. A pin != HEAD is not
#            enough: an unrelated commit, or one on the branch's own line, is
#            non-vacuous and still the wrong base.
#            FALSE NO C CHANGE (task #1065): when the row reads `no C change`,
#            every commit of origin/master..HEAD is checked for a touched NATIVE
#            input (the same set, check.sh excluded); any is a WARN naming path
#            and commit, a FAIL under --strict — the WARN must be true of the
#            landing, not only of the two trees compared.
#            FAILS naming each unit that fails at the tip and passed at the
#            base, or is absent at the base; a unit failing on BOTH arms is
#            tolerated and listed (a pre-existing failure must not turn the row
#            permanently red). BLIND (task #1365, #1363's instance): a tolerated
#            unit the landing TOUCHED — its own source, or a shared input
#            (include/, tools/native/ but check.sh), differs between the two
#            trees the row compiled — is a WARN naming each unit, a FAIL under
#            --strict: failing before and after, its change is invisible to
#            the comparison. Fails closed: a comparison that cannot run makes
#            every tolerated unit BLIND (unverifiable); on a base == tip row
#            they are listed under the VACUOUS line. Recomputed every run — there is no checked-in
#            baseline to raise. Units are keyed by region-qualified path
#            (usa/cod/015180.c): usa and eu share basenames. FAILS as could-not-
#            run on an empty population, a count that does not partition it, or
#            a base arm passing 0 units (the row could not fire). ⛔ COMPILE-
#            ONLY: check.sh runs `-c`, it never links. A PASS proves every
#            TARGET_NATIVE unit still COMPILES natively — NOT that the native
#            build links (A2/#917's g_savePromptLatch has no native storage and
#            this row is green on it). ~2 s per arm, host-only, no VM.
#            SHRINK (FACT #8359, watcher-2's ruling for #945): also FAILS naming
#            each unit in the base's population and absent from the tip's
#            (deleted, carved away, or its TARGET_NATIVE token removed) unless
#            a `Native-Left: <region>/<path>.c <reason>` line in a commit
#            message of base..HEAD names it with a non-empty reason; excused
#            units are listed with their reason and source commit. A RENAME is
#            not a departure (task #963): a base-only unit is paired with a
#            tip-only unit of IDENTICAL bytes in the SAME region, so a pure
#            in-region `git mv` passes; a move to another region is a shrink of
#            the region it left and needs the trailer (task #984, RULING #5339);
#            an edited rename still needs it too (git's -M pairing is printed
#            as a hint, never trusted — it would excuse a partial carve).
#            $LANDING_GATE_NATIVE_LEFT is honoured ONLY by --selftest's scratch
#            arms; set on a real run it excuses nothing and is a WARN (a FAIL
#            under --strict), because an env excuse leaves no git record.
#            ⛔ INTENDED, NOT A BUG: the population is BOTH regions whichever
#            region the run is for, so `landing_gate.sh usa` FAILS on an EU
#            unit leaving it. RULING #5339 DEFERS EU work; it does not license
#            EU regression (watcher-2, 21:45 on task #995): nothing may
#            silently shrink the EU native population, and the Native-Left
#            trailer is the escape if one ever must. Do not scope this row to
#            the run's region.
#            The counts are printed PER REGION beside the old total (task
#            #1006, FACT ledger-28603): `tip usa 29/29, eu 15/15 = 44/44`
#            (pass/units). A total alone read 44 -> 44 while USA went 29 -> 28
#            and EU 15 -> 16; the shrink rule still FAILED that unit, but the
#            count hid it. The per-region line is a readout, not a verdict.
#   DLISITES (task #1116 GATE-F item a, RULING #8549 rev 2/3) tools/ee/
#            ps2eeas_dli_sites.py --ps2eeas over tools/ee/ps2eeas_dli_sites.txt,
#            the allowlist asm_unit.sh trusts: every row must load its value
#            (64-bit simulation), sit inside its function's splat file, equal
#            the ROM AND equal what Ps2EeAs.exe itself emits. FAILS naming each
#            failing row and, after them, each `CNR` row (task #1185: a missing
#            site must not vanish from the headline because another row
#            failed), and as could-not-run on no summary, 0 rows, or a
#            summary without `(Ps2EeAs checked)` — which includes a row whose
#            site is ABSENT from Ps2EeAs's output (`CNR` line, task #1142):
#            no emission is not a different emission (FACT #8645).
#            The Ps2EeAs arm is the point:
#            the host-only checks PASS a ROM-true row Ps2EeAs does not emit
#            (func_002E5074@0x2E50E4, #1105), which --selftest arm (22) seeds.
#            ⚠️ The checker reaches Ps2EeAs through tools/ee/vm.sh, which is
#            hardcoded to colima-ee-x86 (VM a) — this row runs on VM a whatever
#            EE_DOCKER_CONTEXT says, and prints so. The assembly source goes
#            INSIDE the vm.sh command, not over the sshfs mount (FACT #8645
#            saw that mount hand Ps2EeAs a stale sites.s), and the container's
#            md5 of it must equal the host's or the row is could-not-run. It
#            writes nothing in the worktree.
#   ASMUNIT  (task #1116 GATE-F item b, FACT #8610) the BUILD's build.log
#            carries no `asm_unit.sh: WARNING:`, `REFUSED:` or `FAIL:` line.
#            FACT #8610 observed such a line reach build.log and no row read it:
#            on USA the image cmp is the backstop (a WARNING site emits code the
#            ROM lacks), on EU there is no cmp at all. FAILS listing each line;
#            an `asm_unit.sh:` line with none of the three verbs is a WARN (a
#            FAIL under --strict), so a new diagnostic kind is not read as 0.
#            --no-build reads the recorded build's log (the TREE row ties it
#            to this tree).
#   TREE     the ROW is tied to the tree it was built from (task #457, #451 gap
#            1): do_build records HEAD^{tree}, a hash of the WHOLE working tree
#            (tracked + modified + untracked, .gitignore honoured) and the dirty
#            count in .gate_landing/<region>/built_tree.txt; --no-build re-hashes
#            and FAILS on any mismatch. RULING #7208 condition 1 is discharged
#            ONLY by the BUILDING form on a 0-dirty tree at the SHA-named landing
#            — the summary says so whenever this run is not that.
#   DIRTY    (task #1011, FACT ledger-28624) the gate ENFORCES the 0-dirty half
#            of that sentence instead of only printing it: any path in `git
#            status --porcelain` (tracked + modified + untracked, .gitignore
#            honoured) is listed, a WARN, and under --strict a FAIL whose
#            verdict reads `#### landing_gate <region>: FAIL (dirty)`. Until
#            this row, "0 dirty" was enforced only by seats quoting the header.
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
#                                                 PASS (default region usa). It
#                                                 can NEVER print a landing
#                                                 verdict line (`#### landing_gate
#                                                 usa|eu ...`): say() exits 3 if
#                                                 anything tries (task #984). Its
#                                                 own summary is `#### SELFTEST
#                                                 <region>: PASS|FAIL`, with no
#                                                 `landing_gate` in it (#997).
#                                                 Its stdout AND stderr go
#                                                 through one guard that stops
#                                                 the run (exit 3) on a verdict
#                                                 line from ANY emitter, not
#                                                 only say() (task #1006)
#:usage-end — usage() prints the header down to this line (task #1006)
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
#   - EU bytes — EU has no link, so its row is a link-property, not a cmp;
#   - the NATIVE LINK — the NATIVE row is compile-only (check.sh -c); an
#     undefined or storage-less symbol natively is invisible to it.
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"; cd "$ROOT"
HERE="tools/ee"
EE_CTX="${EE_DOCKER_CONTEXT:-colima-ee-x86}"
BASE_DIR="$HERE/landing_baseline"
SHADOW_CLASSES="CLASS1 CLASS2 CLASS3 NOTARGET"
PYTHON=".venv-decomp/bin/python"

# the header down to the `#:usage-end` marker — a marker, not a line range
# (task #1006: '2,154p' silently truncated whenever the header grew)
usage() { awk 'NR > 1 && /^#:usage-end/ { exit } NR > 1' "$0" | sed 's/^# \{0,1\}//'; exit 2; }
# IN_SELFTEST: set only by selftest(), never from the environment. While it is
# 1, say() refuses to print a line of the landing-verdict form a landing is
# read from (task #984, watcher-2): a selftest arm must not be able to forge
# the fleet's trust anchor, even into a file, even through a future edit.
IN_SELFTEST=0
LANDING_LINE_RE='^#### landing_gate (usa|eu)[ :]'
say()  {
  if [ "$IN_SELFTEST" = 1 ] && printf '%s\n' "$*" | /usr/bin/grep -qE "$LANDING_LINE_RE"; then
    printf 'SELFTEST-BROKEN: --selftest tried to print a landing verdict line, refused and aborted (task #984): %s\n' "$*" >&2; exit 3
  fi
  printf '%s\n' "$*"
}
# show — print stdin line by line through say(), so a file or command output
# the selftest echoes meets the same guard as its own lines (task #1006: #997's
# arm (0) OK line quoted a forged verdict, and arm (14)'s raw `grep '^####'`
# would have re-emitted one). Feed it by redirection or process substitution
# (`show < f`, `show < <(cmd)`), never by a pipe: a pipe runs it in a subshell,
# where say()'s exit 3 would end only the subshell.
show() { local l; while IFS= read -r l || [ -n "$l" ]; do say "$l"; done; }
# selftest_stdout_guard — the chokepoint for everything --selftest writes,
# whatever the emitter (say() only sees what is routed through it; a raw
# printf, a heredoc or a python one-liner is not). A landing verdict line is
# replaced by a SELFTEST-BROKEN line and the guard exits 3 at once, so nothing
# after it — the forger's own `#### SELFTEST <region>: PASS` included — reaches
# the output; the selftest dies of SIGPIPE on its next write.
selftest_stdout_guard() {
  awk -v re="$LANDING_LINE_RE" '$0 ~ re { print "SELFTEST-BROKEN: a landing verdict line reached --selftest'"'"'s output without say() and was suppressed; the run is aborted (task #1006)"; fflush(); exit 3 }
                                { print; fflush() }'
}
# print_path_scan FILE — the static half of the guard (task #1006, watcher-2
# 22:10): one `<line>: <verb> | <line text>` row per statement in a selftest*
# function body of FILE (selftest_stdout_guard excepted — it IS the output)
# whose output reaches stdout from ANY emitter verb (cat, grep, sed, awk,
# head, tail, printf, echo, tr, sort, cut, paste, comm, column, python, perl;
# a heredoc is `cat <<`), rather than through say()/show(). Not a row: a
# statement inside $( ) or <( ), redirected to a file (a brace group's redirect
# included), grep -q, sed -i, or a pipeline ending in a line-prefixing
# `sed 's/^/<text not starting with #>/'`. `>&2` IS a row: stderr joins the
# guarded output. A line scan: blind to a verb reached by indirection ($cmd),
# to verbs not listed (tee, dd, git), and to functions that print — the runtime
# guard is what covers those.
print_path_scan() {
  perl -ne '
    BEGIN { ($in, $buf, $start) = (0, "", 0) }
    chomp; my $l = $_;
    if ($l =~ /^(?!selftest_stdout_guard)(selftest[a-z_]*)\(\) \{/) { $in = 1; next }
    if ($in && $l =~ /^\}/) { $in = 0; next }
    next unless $in;
    $start = $. if $buf eq "";
    if ($l =~ /\\$/) { $buf .= substr($l, 0, -1) . " "; next }
    my $s = $buf . $l; $buf = "";
    next if $s =~ /^\s*#/;
    $s =~ s{sed (?:-n )?\x27[0-9,]*s/\^ ?\*?/[^#\x27/][^\x27/]*/p?\x27}{__PREFIX__}g;
    $s =~ s/\x27[^\x27]*\x27/Q/g;
    $s =~ s/\$\(\([^()]*\)\)/N/g;
    1 while $s =~ s/[\$<]\([^()]*\)/C/g;
    $s =~ s/\\"/E/g;
    $s =~ s/"[^"]*"/D/g;
    $s =~ s/\{[^{}]*\}[^;{}]*?(?<![0-9&])>{1,2}\s*[^&\s]/G/g;
    for my $seg (split /;|&&|\|\|/, $s) {
      my @st = split /\|/, $seg; my $last = $st[-1];
      $last =~ s/^\s*(?:(?:\{|\}|\(|then|else|do|if|while|until|!)\s+)*//;
      my ($verb) = $last =~ /^\s*(\S+)/; next unless defined $verb;
      next unless $verb =~ m{^(?:/usr/bin/)?(?:cat|grep|egrep|sed|awk|head|tail|printf|echo|tr|sort|cut|paste|comm|column|python3?|perl)$};
      next if $seg =~ /(?:^|[^0-9&])>{1,2}\s*[^&\s]/;
      next if $verb =~ /sed$/ && $last =~ /\s-i/;
      next if $verb =~ /grep$/ && $last =~ /\s-[A-Za-z]*q/;
      print "$start: $verb | $l\n";
    }' "$1"
}
fail() { say "FAIL $*"; FAILED=$((FAILED+1)); }
ok()   { say "OK   $*"; }
# warn: a baseline that is stale-HIGH. Counted; a FAIL under --strict.
warn() { if [ "$STRICT" = 1 ]; then say "FAIL(strict) $*"; FAILED=$((FAILED+1)); else say "WARN $*"; fi; WARNED=$((WARNED+1)); }
FAILED=0; WARNED=0; STRICT=${LANDING_GATE_STRICT:-0}
NATIVE_LEFT_FROM_ENV=0   # never from the environment: only native_arm sets it
# NATIVE_TIP_REV: the commit check_native treats as the tip for its git-side
# predicates (base == tip, no C change, the pin's ancestry, the Native-Left and
# touched-input logs). Never from the environment: only --selftest's probe arms
# set it, to a commit object whose tree IS HEAD's tree (so the working tree the
# tip arm compiles is that commit's content) and whose history they built.
NATIVE_TIP_REV=""
# NATIVE_NO_C_CHANGE: set to 1 by check_native when its row is the
# `WARN NATIVE: no C change` row; run_gate annotates its verdict line with it.
NATIVE_NO_C_CHANGE=0

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

# --user: the container runs as the INVOKING uid:gid (task #1361). ee-build's
# default user is root, and on native-Linux docker container-root IS host-root,
# so every build output under going-decompiled/build was root-owned and the
# next HOST-side write there (configure.py's split, --selftest arm 13) died
# with PermissionError. colima already maps the mount to the host user (why
# the M1 never saw this) and lima's guest user carries the host uid, so the
# same flag should be correct there too — no host conditional. ⚠️ That half is
# REASONING, not measured: #1361 ran on the XPS only. HOME=/tmp: the image's /root is 0700, unwritable as a
# non-root uid. Nothing in the image needs root: the toolchain runs under wibo
# (no wine prefix) and writes only into /work and mktemp.
in_vm() {  # in_vm '<sh script>' — one docker run, repo mounted at /work
  docker --context "$EE_CTX" run --rm --user="$(id -u):$(id -g)" -e HOME=/tmp -v "$ROOT":/work -w /work ee-build sh -c "$1"
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
    local unit; unit=$(printf '%s' "$loc" | sed -E "s#^going-decompiled/src/$region/(.*)\.c(pp)?:[0-9]+\$#\1#")
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

# --------------------------------------------------------------- GMODEL ----
# check_gmodel REGION [TREE] [UNIT_FLAGS] — no C-compiled function in a -G0
# unit whose ROM words are gp-relative (task #919, FACT #8246).
check_gmodel() {
  local region=$1 tree=${2:-.} flags=${3:-$HERE/unit_flags.sh}
  local scan="$OUT/gmodel_scan.txt"
  say "== GMODEL [$region]: no -G0 unit compiles a function whose ROM words are gp-relative (scan: $scan)"
  bash "$HERE/gmodel_scan.sh" "$region" --root "$tree" --flags "$flags" > "$scan" 2>&1; local rc=$?
  local nl ng8; nl=$(/usr/bin/grep -c '^LATENT ' "$scan" || true); ng8=$(/usr/bin/grep -cE '^UNIT [^ ]+ -G[1-9][0-9]* [1-9]' "$scan" || true)
  case $rc in
    0) ok "no -G0 unit compiles a gp-word function ($ng8 nonzero -G unit(s) show gp words; $nl INCLUDE_ASM member(s) LATENT, listed)" ;;
    1) fail "GMODEL [$region]: $(/usr/bin/grep -c '^MISMATCH ' "$scan" || true) function(s) compiled at -G0 whose ROM words are gp-relative — move the unit to -G8 or keep them INCLUDE_ASM: $(/usr/bin/grep '^MISMATCH ' "$scan" | cut -d' ' -f2- | tr '\n' ';' | sed 's/;$//; s/;/ ; /g')" ;;
    *) fail "gmodel_scan.sh could not run (rc $rc): $(head -c 300 "$scan")" ;;
  esac
  say "     members:"; /usr/bin/grep -vE '^UNIT [^ ]+ -G[^ ]+ 0$' "$scan" | sed 's/^/       /'
}

# ------------------------------------------------------------- DLISITES ----
# check_dlisites [SITES_FILE] — every allowlist row re-derived with Ps2EeAs.exe
# itself (task #1116, RULING #8549). A failing row is a FAIL, not a WARN: it is
# a wrong build input, not a stale baseline.
# dlisites_rows KIND SCAN — SCAN's `KIND line N:` rows, each with its indented
# reason lines joined by ` | `, the rows joined by ` ; `.
dlisites_rows() {
  awk -v k="$1" 'index($0, k " line ") == 1 { if (m) { printf "%s%s", sep, m; sep = " ; " } m = $0; next } /^      / && m { sub(/^ +/, ""); m = m " | " $0; next } { if (m) { printf "%s%s", sep, m; sep = " ; " } m = "" } END { if (m) printf "%s%s", sep, m }' "$2"
}
check_dlisites() {
  local sites=${1:-$HERE/ps2eeas_dli_sites.txt} scan="$OUT/dlisites_scan.txt"
  say "== DLISITES: every row of $sites re-derived by ps2eeas_dli_sites.py --ps2eeas — 64-bit simulation, splat file, ROM words AND Ps2EeAs.exe's own emission, RULING #8549 (scan: $scan). Ps2EeAs runs via tools/ee/vm.sh = colima-ee-x86 (VM a), not $EE_CTX"
  python3 "$HERE/ps2eeas_dli_sites.py" --ps2eeas "$sites" > "$scan" 2>&1; local rc=$?
  local sum nrows nfail
  sum=$(/usr/bin/grep -E '^ps2eeas_dli_sites: [0-9]+ rows, [0-9]+ failed' "$scan" | tail -1)
  nrows=$(printf '%s' "$sum" | awk '{print $2}'); nfail=$(printf '%s' "$sum" | awk '{print $4}')
  if [ "$rc" = 0 ] && [ "${nfail:-x}" = 0 ] && [ "${nrows:-0}" -gt 0 ] && printf '%s' "$sum" | /usr/bin/grep -q '(Ps2EeAs checked)$'; then
    ok "DLISITES: all $nrows allowlist row(s) equal the ROM and Ps2EeAs.exe's emission"
  elif [ "$rc" = 1 ] && [ "${nfail:-0}" -gt 0 ]; then
    fail "DLISITES: $nfail of $nrows allowlist row(s) fail — asm_unit.sh would expand them as written: $(dlisites_rows FAIL "$scan")$(n=$(/usr/bin/grep -c '^CNR  line ' "$scan" || true); [ "$n" -gt 0 ] && printf ' ; and %s row(s) could not run: %s' "$n" "$(dlisites_rows 'CNR ' "$scan")")"
  else
    fail "ps2eeas_dli_sites.py could not run, or printed no Ps2EeAs-checked summary with at least one row (rc $rc, summary '${sum:-none}'): $(if /usr/bin/grep -q '^CNR  line ' "$scan"; then awk '/^CNR  line /{ if (m) { printf "%s%s", sep, m; sep = " ; " } m = $0; next } /^      / && m { sub(/^ +/, ""); m = m " | " $0; next } { if (m) { printf "%s%s", sep, m; sep = " ; " } m = "" } END { if (m) printf "%s%s", sep, m }' "$scan"; else head -c 300 "$scan" | tr '\n' ' '; fi)"
  fi
  say "     members:"; sed 's/^/       /' "$scan"
}

# -------------------------------------------------------------- ASMUNIT ----
# check_asmunit [BUILD_LOG] — no asm_unit.sh WARNING/REFUSED/FAIL line in the
# build's log (task #1116, FACT #8610).
check_asmunit() {
  local log=${1:-$OUT/build.log} n na
  say "== ASMUNIT [$REGION]: no asm_unit.sh WARNING/REFUSED/FAIL line in $log — a WARNING site emits code the ROM lacks (EU has no cmp to catch it), a REFUSED/FAIL writes no object (FACT #8610)"
  [ -f "$log" ] || { fail "ASMUNIT [$REGION]: no $log — the build did not run, the row cannot read it"; return; }
  n=$(/usr/bin/grep -cE 'asm_unit\.sh: (WARNING|REFUSED|FAIL):' "$log" || true)
  # every assembled unit's `dli: N transforms` line (task #1205) is a known kind,
  # in its exact spelling only: a reworded one stays unknown and WARNs, because
  # GATE-F3 (#1158) reads that spelling
  nd=$(/usr/bin/grep -cE "$ASMUNIT_DLI_RE" "$log" || true)
  na=$(( $(/usr/bin/grep -c 'asm_unit\.sh:' "$log" || true) - nd ))
  if [ "$n" = 0 ]; then
    ok "ASMUNIT [$REGION]: 0 asm_unit.sh WARNING/REFUSED/FAIL lines in $(wc -l < "$log" | tr -d ' ') log lines"
  else
    fail "ASMUNIT [$REGION]: $n asm_unit.sh WARNING/REFUSED/FAIL line(s) in $log: $(/usr/bin/grep -noE 'asm_unit\.sh: (WARNING|REFUSED|FAIL): [^ ]*' "$log" | tr '\n' ';' | sed 's/;$//; s/;/ ; /g')"
    show < <(/usr/bin/grep -nE 'asm_unit\.sh: (WARNING|REFUSED|FAIL):' "$log" | cut -c1-400 | sed 's/^/       /')
  fi
  if [ "$na" != "$n" ]; then
    warn "ASMUNIT [$REGION]: $((na - n)) asm_unit.sh: line(s) with none of WARNING/REFUSED/FAIL in $log — a diagnostic kind this row does not know; read it and extend the row: $(/usr/bin/grep -n 'asm_unit\.sh:' "$log" | /usr/bin/grep -vE 'asm_unit\.sh: (WARNING|REFUSED|FAIL):' | /usr/bin/grep -vE "$ASMUNIT_DLI_RE" | cut -c1-200 | tr '\n' ';')"
  fi
}
# asm_unit.sh's per-unit transform line, whole-line anchored (task #1205)
ASMUNIT_DLI_RE='^asm_unit\.sh: dli: [0-9]+ transforms \([0-9]+ allowlist rows for (usa|eu)\)$'

# --------------------------------------------------------------- NATIVE ----
# native_scan TREE OUTFILE — compile every TARGET_NATIVE unit under TREE's
# going-decompiled/src with THIS tree's tools/native/check.sh (copied into a
# TREE that is not this one, so base and tip are one instrument; check.sh takes
# include/ and mips_callees.h from the tree it sits in). Writes one
# `PASS|FAIL <region>/<path>` row per unit to OUTFILE and check.sh's output to
# OUTFILE.log. Keys come from check.sh's `FAIL: <full path>` lines, never from
# its `failed units:` summary. rc 0, or 2 = could not run (empty population, or
# the rows do not partition it).
native_scan() {
  local tree=$1 out=$2 units n np nf
  if [ "$(cd "$tree" && pwd -P)" != "$(cd "$ROOT" && pwd -P)" ]; then
    mkdir -p "$tree/tools/native"; cp "$ROOT/tools/native/check.sh" "$tree/tools/native/check.sh"
  fi
  units=$(cd "$tree" && /usr/bin/grep -rl TARGET_NATIVE going-decompiled/src | LC_ALL=C sort)
  n=$(printf '%s\n' "$units" | /usr/bin/grep -c . || true)
  [ "$n" -gt 0 ] || { printf 'no TARGET_NATIVE unit under %s/going-decompiled/src\n' "$tree" > "$out.log"; : > "$out"; return 2; }
  # shellcheck disable=SC2086  # one unit per word; no unit path holds a space
  (cd "$tree" && bash tools/native/check.sh $units) > "$out.log" 2>&1
  sed -n 's#^FAIL: going-decompiled/src/##p' "$out.log" | LC_ALL=C sort -u > "$out.fail"
  printf '%s\n' "$units" | sed 's#^going-decompiled/src/##' > "$out.all"
  { LC_ALL=C comm -23 "$out.all" "$out.fail" | sed 's/^/PASS /'; sed 's/^/FAIL /' "$out.fail"; } > "$out"
  np=$(/usr/bin/grep -c '^PASS ' "$out" || true); nf=$(/usr/bin/grep -c '^FAIL ' "$out" || true)
  # the partition must close three ways: rows == population, check.sh's own
  # summary == the rows, and every FAIL key is a member of the population
  /usr/bin/grep -q "^--- native compile-check: pass=$np fail=$nf ---\$" "$out.log" && [ $((np+nf)) = "$n" ] \
    && [ -z "$(LC_ALL=C comm -23 "$out.fail" "$out.all")" ] || return 2
}

# native_region_counts ROWS — `usa P/N, eu P/N` from native_scan's rows: per
# region (the first path component), units passing / units in the population.
# Every region with a unit is printed, so a region emptied at one arm shows as
# absent there rather than as a smaller total.
native_region_counts() {
  awk '{ r = $2; sub(/\/.*/, "", r); n[r]++; if ($1 == "PASS") p[r]++ }
       END { for (r in n) printf "%s %d/%d\n", r, p[r], n[r] }' "$1" | LC_ALL=C sort | paste -sd, - | sed 's/,/, /g'
}
# Readers of check_native's `NATIVE per region` line, for --selftest (task
# #1006). ARM is tip|base. native_region_units OUT ARM REGION -> that region's
# unit count; native_total_units OUT ARM -> the arm's total unit count;
# native_region_sums_close OUT -> rc 0 iff both arms are present and each
# arm's per-region pass and unit counts sum to its printed total.
native_region_arm() { sed -n 's/^     NATIVE per region (pass\/units): //p' "$1" | tr ';' '\n' | sed 's/^ *//' | /usr/bin/grep "^$2 " | sed "s/^$2 //"; }
native_region_units() { native_region_arm "$1" "$2" | sed 's/ = .*//' | tr ',' '\n' | awk -v r="$3" '$1 == r { split($2, v, "/"); print v[2] }'; }
native_total_units() { native_region_arm "$1" "$2" | sed -n 's#.* = [0-9]*/\([0-9]*\)$#\1#p'; }
native_region_sums_close() {
  local arm n=0 body tot
  for arm in tip base; do
    body=$(native_region_arm "$1" $arm); [ -n "$body" ] || return 1
    tot=${body##* = }
    [ "$(printf '%s\n' "${body% = *}" | tr ',' '\n' | awk '{ split($2, v, "/"); p += v[1]; u += v[2] } END { print p "/" u }')" = "$tot" ] || return 1
    n=$((n+1))
  done
  [ $n = 2 ]
}

# native_left_overrides [BASEREF [TIPREV]] — the Native-Left overrides in force,
# one `<unit>\t<reason>\t<source>` row each: every `Native-Left: <region>/<path>.c
# <reason>` line in the commit messages of BASEREF..TIPREV (TIPREV defaults to
# HEAD; source = the commit),
# then — ONLY inside --selftest's scratch arms (NATIVE_LEFT_FROM_ENV=1, set by
# selftest_native's native_arm) — every such line in $LANDING_GATE_NATIVE_LEFT
# (source = env). Everywhere else the env var is ignored and check_native
# reports it (task #963: an excuse from the environment leaves no git record).
# A line with no reason is emitted with an empty reason and does not excuse
# its unit.
native_left_overrides() {
  { if [ -n "${1:-}" ]; then git log --format='@@%h%n%B' "$1..${2:-HEAD}" 2>/dev/null; fi
    if [ "$NATIVE_LEFT_FROM_ENV" = 1 ] && [ -n "${LANDING_GATE_NATIVE_LEFT:-}" ]; then printf '@@env\n%s\n' "$LANDING_GATE_NATIVE_LEFT"; fi
  } | awk '/^@@/ { src = substr($0, 3); next }
           sub(/^Native-Left:[ \t]+/, "") { u = $1; r = $0; sub(/^[^ \t]+[ \t]*/, "", r); sub(/[ \t]+$/, "", r); print u "\t" r "\t" src }'
}

# native_renames BASE_TREE TIP_TREE "LEFT" "ARRIVED" — one `= <old> <new>` row
# per unit that left the base's population and arrived in the tip's with
# IDENTICAL bytes IN THE SAME REGION (units are region-qualified paths under
# going-decompiled/src; the region is the first component). One-to-one: each
# arrival pairs at most one departure. An arrival identical to a departure of
# ANOTHER region is not paired (task #984, watcher-2: a unit leaving src/usa is
# a USA shrink whatever lands in src/eu, RULING #5339); it is emitted as an
# `x <old> <new>` row, a hint for the FAIL message only.
native_renames() {
  [ -n "$3" ] && [ -n "$4" ] || return 0
  # shellcheck disable=SC2086  # one unit per word; no unit path holds a space
  { (cd "$1/going-decompiled/src" && shasum $3) | sed 's/^/L /'
    (cd "$2/going-decompiled/src" && shasum $4) | sed 's/^/A /'
  } | awk 'function region(p) { sub(/\/.*/, "", p); return p }
           $1 == "L" { k = $2 SUBSEP region($3); l[k] = l[k] " " $3; x[$2] = x[$2] " " $3; next }
           $1 != "A" { next }
           { k = $2 SUBSEP region($3) }
           (k in l) && l[k] != "" { o = l[k]; sub(/^ /, "", o); split(o, v, " "); print "=", v[1], $3
                                    sub(/^ [^ ]+/, "", l[k]); next }
           ($2 in x) { o = x[$2]; sub(/^ /, "", o); split(o, v, " "); print "x", v[1], $3 }'
}

# native_touched BASE_TREE TIP_TREE UNIT... — what differs between the two
# trees the NATIVE row compiled, for its BLIND check (task #1365): `U <unit>`
# for each named unit whose source differs, `S <path>` for each shared input
# that differs or exists on one side only (going-decompiled/include and
# tools/native but check.sh, which native_scan copies from the tip into both).
# rc 2, with the reason on stdout, when the comparison cannot run (a unit
# missing from either tree, an unreadable file): the caller fails closed.
native_shared_sums() {  # TREE — `<sha1>  <path>` per shared NATIVE input, line-sorted for comm
  (cd "$1" && set -o pipefail && /usr/bin/find going-decompiled/include tools/native -type f ! -path tools/native/check.sh -print0 \
     | xargs -0 shasum | LC_ALL=C sort) 2>/dev/null
}
native_touched() {
  local base=$1 tip=$2 u rc lb lt; shift 2
  for u in "$@"; do
    [ -f "$base/going-decompiled/src/$u" ] && [ -f "$tip/going-decompiled/src/$u" ] || { echo "$u is not a file in both trees"; return 2; }
    rc=0; cmp -s "$base/going-decompiled/src/$u" "$tip/going-decompiled/src/$u" || rc=$?
    case $rc in 0) ;; 1) echo "U $u" ;; *) echo "cmp could not read $u"; return 2 ;; esac
  done
  lb=$(native_shared_sums "$base") || { echo "could not checksum the base's shared NATIVE inputs under $base"; return 2; }
  lt=$(native_shared_sums "$tip") || { echo "could not checksum the tip's shared NATIVE inputs under $tip"; return 2; }
  LC_ALL=C comm -3 <(printf '%s\n' "$lb") <(printf '%s\n' "$lt") | sed 's/^[[:space:]]*[0-9a-f]*  //' | LC_ALL=C sort -u | sed 's/^/S /'
}

# check_native [TIP_TREE] [BASE_TREE] [UPSTREAM] — the NATIVE row (task #923).
# Defaults: the tip is this working tree, the base is extracted from
# merge-base(HEAD, UPSTREAM), UPSTREAM defaulting to origin/master (an argument,
# not an env var: only --selftest's vacuous arm passes it, to reach the
# base == HEAD case through the same default resolution a real run takes).
check_native() {
  local tip=${1:-$ROOT} basetree=${2:-} baseref="(given tree ${2:-})" upstream=${3:-origin/master} vacuous="" samein="" tiprev=""
  NATIVE_NO_C_CHANGE=0
  say "== NATIVE: tools/native/check.sh over every TARGET_NATIVE unit, base-relative — FAIL on a unit failing at the tip that passed at (or is absent from) the base, and on a unit that LEFT the population (in the base's, not the tip's; a byte-identical rename WITHIN a region is paired, not a departure) unless a Native-Left commit trailer names it and a reason. COMPILE-ONLY: green does NOT mean the native build links"
  if [ "$NATIVE_LEFT_FROM_ENV" != 1 ] && [ -n "${LANDING_GATE_NATIVE_LEFT:-}" ]; then
    warn "NATIVE: \$LANDING_GATE_NATIVE_LEFT is set and IGNORED — it is honoured only by --selftest's scratch arms (task #963); excuse a departure with a \`Native-Left: <region>/<path>.c <reason>\` trailer in a commit of the landing, which is reviewable and permanent"
  fi
  if [ -z "$basetree" ]; then
    baseref=${LANDING_GATE_NATIVE_BASE:-$(git merge-base HEAD "$upstream" 2>/dev/null)}
    [ -n "$baseref" ] && baseref=$(git rev-parse --verify -q "$baseref^{commit}")
    [ -n "$baseref" ] || { fail "NATIVE: no base commit (git merge-base HEAD $upstream failed and LANDING_GATE_NATIVE_BASE is unset or not a commit) — the row cannot be base-relative"; return; }
    tiprev=$(git rev-parse --verify -q "${NATIVE_TIP_REV:-HEAD}^{commit}")
    [ -n "$tiprev" ] || { fail "NATIVE: the tip commit ${NATIVE_TIP_REV:-HEAD} does not resolve"; return; }
    # task #992: a base that IS the tip is a control that cannot fire. Only a
    # clean tree is a self-comparison — uncommitted NATIVE inputs are still
    # compared against HEAD, and say so (the DIRTY row fails that tree under
    # --strict, task #1011). A pin equal to HEAD is NOT exempt (task #1011):
    # an explicit pin to the tip is the same self-comparison with extra steps,
    # and it is the easiest mistake a seat told to "pin the base" can make.
    local nd; nd=$(git status --porcelain --no-renames -- going-decompiled/src going-decompiled/include tools/native | wc -l | tr -d ' ')
    if [ "$baseref" = "$tiprev" ]; then
      if [ "$nd" != 0 ]; then
        say "     base == HEAD ($baseref): the row compares only the $nd uncommitted path(s) under going-decompiled/src, going-decompiled/include, tools/native"
      elif [ -n "${LANDING_GATE_NATIVE_BASE:-}" ]; then
        vacuous=1; warn "NATIVE: base == tip, VACUOUS — LANDING_GATE_NATIVE_BASE=$LANDING_GATE_NATIVE_BASE pins the base to HEAD $baseref itself, so both arms compile one tree and neither the regression rule nor the shrink rule can fire; an explicit pin to the tip is the same self-comparison with extra steps — pin the fork point, merge-base(HEAD, $upstream), or on a tip $upstream has already reached, the master the landing was cut from: the parent of the landing's FIRST commit, which is the tip's parent only for a one-commit landing (task #1011, #1116)"
      else
        vacuous=1; warn "NATIVE: base == tip, VACUOUS — the default base merge-base(HEAD, $upstream) is HEAD $baseref itself (the tip has already reached $upstream), so both arms compile one tree and neither the regression rule nor the shrink rule can fire; a post-landing validator pins the master the landing was cut from: LANDING_GATE_NATIVE_BASE=<parent of the landing's FIRST commit> — the tip's parent only for a one-commit landing (task #992, #1116)"
      fi
    # task #1034 (watcher-2's ruling Q1 on FACT ledger-28740): a base with a
    # different sha but the same NATIVE inputs is the same self-comparison. The
    # inputs are what the base arm reads — src/, include/, tools/native/ minus
    # check.sh, which native_scan takes from the tip for both arms. A WARN that
    # is not counted: a tools-only landing has nothing native to regress, so it
    # must pass --strict; the line only stops its row being quoted as a control.
    elif [ "$nd" = 0 ] && git diff --quiet "$baseref" "$tiprev" -- going-decompiled/src going-decompiled/include tools/native ':(exclude)tools/native/check.sh'; then
      samein=1; NATIVE_NO_C_CHANGE=1; say "WARN NATIVE: no C change — the base $baseref and HEAD have identical NATIVE inputs (going-decompiled/src, going-decompiled/include, tools/native but check.sh, which both arms take from the tip), so both arms compile the same C and this row proves nothing; not counted, and not a FAIL under --strict (task #1034, ruling Q1 on FACT ledger-28740) — to exercise the row, pin a base whose C differs"
    fi
    # task #1065 (#1073's spec gap): a pin that is not HEAD can still be the
    # WRONG base. It must be an ancestor of HEAD AND ancestor-or-equal of
    # merge-base(HEAD, upstream) — on the mainline, at or before the fork point,
    # not on the branch's own line and not on an unrelated one. The default
    # base IS that merge-base, so only a pin is checked.
    if [ -n "${LANDING_GATE_NATIVE_BASE:-}" ] && [ "$baseref" != "$tiprev" ]; then
      local mb; mb=$(git merge-base "$tiprev" "$upstream" 2>/dev/null)
      if ! git merge-base --is-ancestor "$baseref" "$tiprev" 2>/dev/null; then
        warn "NATIVE: WRONG BASE — the pinned base $baseref is NOT an ancestor of HEAD $tiprev, so the row compares against an unrelated commit; pin the fork point merge-base(HEAD, $upstream)${mb:+ = $mb} or an ancestor of it — NOT the landing's parent, which on a multi-commit branch sits on the branch's own line and fails the fork-point test below (task #1065, #1116)"
      elif [ -z "$mb" ]; then
        warn "NATIVE: WRONG BASE unverifiable — no merge-base(HEAD, $upstream), so the pinned base $baseref cannot be shown to sit at or before the fork point (task #1065)"
      elif ! git merge-base --is-ancestor "$baseref" "$mb"; then
        warn "NATIVE: WRONG BASE — the pinned base $baseref is an ancestor of HEAD but NOT ancestor-or-equal of merge-base(HEAD, $upstream) $mb: it sits on the branch's own line, so the branch's earlier commits are in neither arm's difference; pin $mb or an ancestor of it (task #1065)"
      else
        say "     pinned base $baseref: an ancestor of HEAD $tiprev and ancestor-or-equal of merge-base(HEAD, $upstream) $mb (task #1065)"
      fi
    fi
    # task #1065: a `no C change` row must be TRUE of the landing, not only of
    # the two trees compared. Any commit of upstream..HEAD touching a NATIVE
    # input (the predicate's own set: check.sh excluded, both arms take it from
    # the tip) means the landing changed C that this row does not see.
    if [ -n "$samein" ]; then
      if ! git rev-parse --verify -q "$upstream^{commit}" >/dev/null; then
        warn "NATIVE: the row says 'no C change' and $upstream does not resolve, so $upstream..HEAD cannot be checked for NATIVE inputs (task #1065)"
      else
        local touched; touched=$(git log --format='@@%h' --name-only "$upstream..$tiprev" -- going-decompiled/src going-decompiled/include tools/native ':(exclude)tools/native/check.sh' \
          | awk '/^@@/ { c = substr($0, 3); next } NF { print $0 " [" c "]" }')
        if [ -n "$touched" ]; then
          warn "NATIVE: FALSE 'no C change' — the row compares identical NATIVE inputs but $upstream..HEAD touches $(printf '%s\n' "$touched" | wc -l | tr -d ' ') NATIVE input path(s), so the landing changed C this row never compiled on both sides: $(printf '%s' "$touched" | tr '\n' ';' | sed 's/;$//; s/;/ ; /g') (task #1065)"
        else
          say "     'no C change' confirmed: $upstream..HEAD touches no NATIVE input (task #1065)"
        fi
      fi
    fi
    basetree="$OUT/native_base"; rm -rf "$basetree"; mkdir -p "$basetree"
    git archive "$baseref" going-decompiled/src going-decompiled/include tools/native | tar -x -C "$basetree" \
      || { fail "NATIVE: git archive of the base $baseref failed"; return; }
  fi
  local t="$OUT/native_tip.txt" b="$OUT/native_base.txt"
  native_scan "$tip" "$t" || { fail "NATIVE: check.sh could not run on the tip ($tip): $(tail -3 "$t.log" | tr '\n' ' ')"; return; }
  native_scan "$basetree" "$b" || { fail "NATIVE: check.sh could not run on the base $baseref: $(tail -3 "$b.log" | tr '\n' ' ')"; return; }
  local tp tf bp bf; tp=$(/usr/bin/grep -c '^PASS ' "$t" || true); tf=$(/usr/bin/grep -c '^FAIL ' "$t" || true)
  bp=$(/usr/bin/grep -c '^PASS ' "$b" || true); bf=$(/usr/bin/grep -c '^FAIL ' "$b" || true)
  say "     tip pass=$tp fail=$tf; base $baseref pass=$bp fail=$bf"
  # per region beside the total (task #1006, FACT ledger-28603): a total can
  # hold still while one region shrinks and the other grows
  say "     NATIVE per region (pass/units): tip $(native_region_counts "$t") = $tp/$((tp+tf)); base $(native_region_counts "$b") = $bp/$((bp+bf))"
  [ "$bp" -gt 0 ] || { fail "NATIVE: the base arm passes 0 of $bf units — every tip failure would read as pre-existing, the row cannot fire"; return; }
  local regress tolerated fixed
  regress=$(LC_ALL=C comm -23 "$t.fail" "$b.fail"); tolerated=$(LC_ALL=C comm -12 "$t.fail" "$b.fail"); fixed=$(LC_ALL=C comm -13 "$t.fail" "$b.fail")
  if [ -n "$regress" ]; then
    fail "NATIVE: $(printf '%s\n' "$regress" | wc -l | tr -d ' ') unit(s) fail to compile at the tip and passed at (or are absent from) the base: $(printf '%s ' $regress)"
    # The unit's own block only: check.sh's `errors: N` count line and its
    # sample of up to 3 (task #1311), at most 4 lines, STOPPING at the next
    # `FAIL:` heading, a `PASS` line or the `--- native compile-check` summary.
    # A fixed -A4 window printed whatever followed a block of fewer than 4
    # lines — the next unit's heading, or the summary (task #1361, seen in
    # #1346). --selftest arms (18x) and (18y) assert the listing.
    local u; for u in $regress; do say "       $u:"; awk -v h="FAIL: going-decompiled/src/$u" 'f && (/^FAIL: / || /^PASS/ || /^--- native compile-check: / || ++n > 4) { exit } f { sub(/^ */, "         "); print } $0 == h { f = 1 }' "$t.log"; done
  else
    ok "NATIVE: no unit fails at the tip that passed at the base ($tp of $((tp+tf)) compile; compile-only, not a link)${vacuous:+ — VACUOUS: base == tip, this measured nothing}${samein:+ — NO C CHANGE: base and tip NATIVE inputs identical, this row proves nothing}"
  fi
  [ -n "$tolerated" ] && say "     failing on BOTH arms (pre-existing, tolerated): $(printf '%s ' $tolerated)"
  [ -n "$fixed" ] && say "     failing at the base only (fixed or removed at the tip): $(printf '%s ' $fixed)"
  # BLIND (task #1365, #1363's measured instance): tolerating a both-arms
  # failure is right for a unit the landing did not touch, but for one it DID
  # touch the comparison sees nothing — failing before and after says nothing
  # about what changed. Touched = differs between the two trees this row
  # compiled (so the same base the arm is pinned to, dirty edits included):
  # the unit's own source, or a shared input every unit reads (include/,
  # tools/native/ but check.sh). WARN naming each unit, FAIL under --strict.
  # Fail closed: if the comparison cannot run, every tolerated unit is BLIND
  # (unverifiable); on a base == tip row the touched set is unknowable, so they
  # are listed under the VACUOUS line that already carries the verdict.
  if [ -n "$tolerated" ]; then
    local tch blind="" shared u
    if [ -n "$vacuous" ]; then
      say "     BLIND (base == tip, the landing's touched set is unknowable): every unit failing on BOTH arms — $(printf '%s ' $tolerated)"
    elif ! tch=$(native_touched "$basetree" "$tip" $tolerated); then
      warn "NATIVE: BLIND unverifiable — the base/tip comparison of the NATIVE inputs could not run: $(printf '%s\n' "$tch" | tail -1), so the row cannot tell whether this landing touched the $(printf '%s\n' $tolerated | wc -l | tr -d ' ') unit(s) failing on BOTH arms; each is treated as BLIND: $(printf '%s ' $tolerated)(task #1365)"
    else
      shared=$(printf '%s\n' "$tch" | sed -n 's/^S //p')
      for u in $tolerated; do
        if printf '%s\n' "$tch" | /usr/bin/grep -qxF "U $u"; then blind="$blind $u (its own source differs base->tip);"
        elif [ -n "$shared" ]; then blind="$blind $u (a shared NATIVE input differs);"; fi
      done
      if [ -n "$blind" ]; then
        warn "NATIVE: BLIND — $(printf '%s\n' "$blind" | tr ';' '\n' | /usr/bin/grep -c . || true) unit(s) fail to compile on BOTH arms AND this landing touched them, so the base-relative comparison cannot see what the landing did to them (a both-arms failure is tolerated; a regression inside it is invisible):${blind%;} — fix the pre-existing failure, or compile the unit's change by other means and say how (task #1365)"
        [ -n "$shared" ] && say "     shared NATIVE input(s) differing base->tip (every unit reads them): $(printf '%s ' $shared)"
      else
        say "     BLIND: none — no unit failing on both arms was touched (its source and the shared NATIVE inputs are identical base->tip) (task #1365)"
      fi
    fi
  fi
  # population shrink (FACT #8359; watcher-2's gate-owner ruling for #945): a
  # unit in the base's population and absent from the tip's is never compiled
  # at the tip, so it FAILS unless a Native-Left override names it and a reason.
  # A RENAME is not a departure (task #963): a base-only unit is paired with a
  # tip-only unit whose bytes are IDENTICAL (one-to-one), so a pure `git mv`
  # passes. Deliberately by content, not by `git diff -M`'s R-status: git's
  # similarity pairing would excuse a unit whose remaining <=50% left the
  # population with it; an edited rename here stays a departure and needs the
  # trailer, and git's own pairing is shown beside it as a hint only.
  # Pairing is per REGION (task #984): an identical arrival in another region
  # does not pair, because the unit still left its own region's population.
  local left ovr="$OUT/native_left_overrides.txt" u excused="" unexcused="" unused pairs renames cross
  left=$(LC_ALL=C comm -23 "$b.all" "$t.all")
  renames=$(native_renames "$basetree" "$tip" "$left" "$(LC_ALL=C comm -13 "$b.all" "$t.all")")
  pairs=$(printf '%s\n' "$renames" | awk '$1 == "=" { print $2, $3 }')
  cross=$(printf '%s\n' "$renames" | awk '$1 == "x" { print $2, $3 }')
  if [ -n "$pairs" ]; then
    say "     renamed, byte-identical (paired, not a departure): $(printf '%s\n' "$pairs" | awk '{ printf "%s -> %s  ", $1, $2 }')"
    left=$(printf '%s\n' $left | LC_ALL=C comm -23 - <(printf '%s\n' "$pairs" | awk '{ print $1 }' | LC_ALL=C sort))
  fi
  native_left_overrides "$([ -z "${2:-}" ] && printf '%s' "$baseref")" "$tiprev" > "$ovr"
  for u in $left; do
    if awk -F'\t' -v u="$u" '$1 == u && $2 != "" { f = 1 } END { exit !f }' "$ovr"; then excused="$excused $u"; else unexcused="$unexcused $u"; fi
  done
  if [ -n "$unexcused" ]; then
    fail "NATIVE: $(printf '%s\n' $unexcused | wc -l | tr -d ' ') unit(s) LEFT the TARGET_NATIVE population (in the base's population, absent from the tip's, not renamed byte-identical within its region) with no Native-Left override naming unit + reason:$unexcused — if intended, add \`Native-Left: <region>/<path>.c <reason>\` to a commit message in the landing"
    [ -n "$cross" ] && say "     byte-identical in ANOTHER region (not paired — a unit leaving src/<region> is that region's shrink whatever lands elsewhere, RULING #5339; task #984): $(printf '%s\n' "$cross" | awk '{ printf "%s -> %s  ", $1, $2 }')"
    if [ -z "${2:-}" ]; then
      local hint; hint=$(git diff -M --name-status "$baseref" -- going-decompiled/src 2>/dev/null \
        | awk -F'\t' -v l="$unexcused" 'BEGIN { n = split(l, a, " "); for (i = 1; i <= n; i++) x["going-decompiled/src/" a[i]] = 1 }
                                         $1 ~ /^R/ && ($2 in x) { sub(/^going-decompiled\/src\//, "", $2); sub(/^going-decompiled\/src\//, "", $3); printf "%s -> %s (%s)  ", $2, $3, $1 }')
      [ -n "$hint" ] && say "     git diff -M pairs these as renames (hint only — an EDITED or CROSS-REGION rename is not paired; name it in a Native-Left trailer): $hint"
    fi
  else
    ok "NATIVE: no unit left the population unexcused ($(printf '%s\n' $left | /usr/bin/grep -c . || true) left, each named by a Native-Left override with a reason; base $((bp+bf)) -> tip $((tp+tf)) units; per region, pass/units: base $(native_region_counts "$b") -> tip $(native_region_counts "$t"))${vacuous:+ — VACUOUS: base == tip, this measured nothing}${samein:+ — NO C CHANGE: base and tip NATIVE inputs identical, this row proves nothing}"
  fi
  for u in $excused; do say "     left the population, overridden: $(awk -F'\t' -v u="$u" '$1 == u && $2 != "" { printf "%s — %s [%s]", $1, $2, $3; exit }' "$ovr")"; done
  unused=$(awk -F'\t' '{ print $1 }' "$ovr" | LC_ALL=C sort -u | LC_ALL=C comm -23 - <(printf '%s\n' $left | LC_ALL=C sort -u))
  [ -n "$unused" ] && say "     Native-Left override(s) naming a unit that did not leave (no effect): $(printf '%s ' $unused)"
  /usr/bin/grep -q $'^[^\t]*\t\t' "$ovr" && say "     Native-Left line(s) with NO reason (ignored — they excuse nothing): $(awk -F'\t' '$2 == "" { printf "%s [%s] ", $1, $3 }' "$ovr")"
  say "     members: $t (tip), $b (base); check.sh output: $t.log, $b.log"
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
  # The dli allowlist's host md5 goes to every asm_unit.sh in build.sh, which
  # verifies its read before the dli pass (task #1205, FACT #8713).
  # EE_CC1_ARGLOG: the CC1ARGS row's input, written fresh by this build only.
  rm -f "$OUT/cc1_args.tsv"
  in_vm "B=$BUILD; rm -rf \$B/going-decompiled \$B/$BASENAME.elf \$B/$BASENAME.lma.elf \$B/$BASENAME.rom \$B/ld.log \$B/ld.lma.log \$B/$BASENAME.map \$B/all_addr_syms.ld;$sync ASM_UNIT_DLISITES_MD5=$(sh "$HERE/mount_sync.sh" md5 "$HERE/ps2eeas_dli_sites.txt") S136OS_FUNCS_MD5=$(sh "$HERE/mount_sync.sh" md5 "$HERE/s136os_functions.txt") EE_CC1_ARGLOG=/work/$OUT/cc1_args.tsv sh tools/ee/build.sh $REGION" > "$OUT/build.log" 2>&1
  say "     build.sh rc=$? ($(wc -l < "$OUT/build.log" | tr -d ' ') log lines -> $OUT/build.log)"
  tail -4 "$OUT/build.log" | sed 's/^/     /'
}

# -------------------------------------------------------------- CC1ARGS ----
# check_cc1args [ARGLOG [TABLE [START_EPOCH]]] — the CC1ARGS row (task #1377):
# tools/ee/cc1_arglog_check.py over the BUILD's arg log. rc 1 (an offender) and
# rc 2 (log absent/empty, table unparsed) are both FAILs: a build that logged
# nothing checked nothing. A log older than START_EPOCH (default: this region's
# build_start) is a stale log from another build and FAILs before it is read.
# --selftest arm (26) passes seeded copies.
check_cc1args() {
  local log=${1:-$OUT/cc1_args.tsv} table=${2:-$HERE/build.sh} start=${3:-$(cat "$OUT/build_start" 2>/dev/null || echo 0)}
  say "== CC1ARGS [$REGION]: every compile's cc1 flags == the flag table's ($table), sdk29 and s136; RULING #9004 floor on usa ($log)"
  if [ -f "$log" ]; then
    local m; m=$(python3 -c 'import os,sys; print(int(os.path.getmtime(sys.argv[1])))' "$log")
    if [ "$m" -lt "$start" ]; then fail "CC1ARGS: $log mtime $m < build start $start — a STALE arg log, not this build's"; return; fi
  fi
  local out rc; out=$(python3 "$HERE/cc1_arglog_check.py" "$REGION" "$log" --table "$table" 2>&1); rc=$?
  say "$out" | sed 's/^/     /'
  case $rc in
    0) ok "CC1ARGS: $(printf '%s\n' "$out" | sed -n 's/^# \([0-9]* lines: .*\); table .*/\1/p') — all as the table assigns" ;;
    1) fail "CC1ARGS: $(printf '%s\n' "$out" | /usr/bin/grep -c '^FAIL ' || true) compile(s)/count(s) differ from the flag table or RULING #9004's floor (listed above)" ;;
    *) fail "CC1ARGS could not run (rc $rc) — fails closed: $(printf '%s\n' "$out" | /usr/bin/grep -m1 '^CNR' || printf '%s' "$out" | head -1)" ;;
  esac
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
# check_dirty [STATUS_FILE] — the DIRTY row (task #1011): every path of `git
# status --porcelain --no-renames` (or of STATUS_FILE, --selftest's seed) is
# listed; any is a WARN, and under --strict a FAIL that sets DIRTY_FAILED so
# the verdict reads `FAIL (dirty)`. Not warn(): the strict FAIL is the ruling
# itself, not a stale baseline, so it must not also count as a warning.
DIRTY_FAILED=0
check_dirty() {
  local st; if [ -n "${1:-}" ]; then st=$(cat "$1"); else st=$(git status --porcelain --no-renames); fi
  local n; n=$(printf '%s' "$st" | /usr/bin/grep -c . || true)
  DIRTY_FAILED=0
  say "== DIRTY: RULING #7208 condition 1 needs a 0-dirty tree — any uncommitted path (tracked, modified or untracked) is a WARN, a FAIL under --strict"
  if [ "$n" = 0 ]; then ok "DIRTY: 0 uncommitted paths"; return; fi
  if [ "$STRICT" = 1 ]; then fail "DIRTY: $n uncommitted path(s) — a landing is gated on a 0-dirty tree at the SHA-named commit; commit or remove them and re-run"; DIRTY_FAILED=1
  else say "WARN DIRTY: $n uncommitted path(s) — this run cannot discharge RULING #7208 condition 1 (FAIL under --strict)"; WARNED=$((WARNED+1)); fi
  local l; while IFS= read -r l; do [ -n "$l" ] && say "       $l"; done <<< "$st"
}

# gate_verdict FAILED WARNED DIRTY_FAILED — the text after `#### landing_gate
# <region>: `. A dirty strict run leads with `FAIL (dirty)` whatever else failed.
gate_verdict() {
  local f=$1 w=$2 d=$3 v
  if [ "$f" = 0 ]; then v=PASS
  elif [ "$d" = 1 ]; then v="FAIL (dirty$([ "$f" -gt 1 ] && echo " + $((f-1)) more"))"
  else v="FAIL ($f)"; fi
  [ "$w" -gt 0 ] && v="$v ($w warning$([ "$w" = 1 ] || echo s)$([ "$STRICT" = 1 ] && echo ', counted as FAIL under --strict'))"
  printf '%s\n' "$v"
}

run_gate() {  # run_gate REGION [--no-build] [--strict]
  region_vars "$1"; shift; local build=1
  while [ $# -gt 0 ]; do case "$1" in --no-build) build=0 ;; --strict) STRICT=1 ;; '') ;; *) say "unknown option $1"; exit 2 ;; esac; shift; done
  FAILED=0; WARNED=0; NATIVE_NO_C_CHANGE=0
  # inside --selftest (arm 14) the run is tagged so it cannot read as a landing
  local tag="#### landing_gate $REGION"; [ "$IN_SELFTEST" = 1 ] && tag="==== selftest inner gate [$REGION] (NOT a landing verdict)"
  local dirty; dirty=$(git status --porcelain --no-renames | wc -l | tr -d ' ')
  say "$tag at $(git rev-parse --short HEAD) ($dirty dirty paths), VM $EE_CTX, $(date -u +%FT%TZ)$([ "$STRICT" = 1 ] && echo ', --strict')"
  say "     RULING #7208 condition 1 is discharged only by the BUILDING form on a 0-dirty tree at the SHA-named landing; this run is $([ $build = 1 ] && echo building || echo '--no-build'), $dirty dirty"
  check_dirty
  [ $build = 1 ] && split_inputs   # first: every check below measures the re-split tree
  check_flags
  check_shadow "$REGION"
  check_orphans "$REGION"
  check_libgcc "$REGION"
  check_gmodel "$REGION"
  check_native
  check_dlisites
  if [ $build = 1 ]; then
    do_build
    check_tree "$OUT/built_tree.txt"   # the tree did not move during the build
  else
    say "== BUILD [$REGION]: skipped (--no-build), start epoch taken from $OUT/build_start"
    check_tree "$OUT/built_tree.txt"
  fi
  check_asmunit
  check_cc1args
  local start; start=$(cat "$OUT/build_start" 2>/dev/null || echo 0)
  measure_row "$BUILD/$BASENAME.rom" "$BUILD/$BASENAME.elf" "$BUILD/ld.log" "$OUT/row.txt"
  check_row "$OUT/row.txt" "$start"
  check_noprovide
  # task #1065 (watcher-2 03:31): the no-C-change annotation rides HERE, on
  # the $tag line, never inside gate_verdict — arms (19)/(20) match that text
  # exactly. LANDING_LINE_RE is prefix-anchored, so the suffix still matches.
  say "$tag: $(gate_verdict "$FAILED" "$WARNED" "$DIRTY_FAILED")$([ "$NATIVE_NO_C_CHANGE" = 1 ] && echo ' (native: no C change)')"
  if [ $build = 0 ] || [ "$dirty" != 0 ]; then say "#### NOTE: RULING #7208 condition 1 is NOT discharged by this run ($([ $build = 0 ] && echo '--no-build')$([ $build = 0 ] && [ "$dirty" != 0 ] && echo ', ')$([ "$dirty" != 0 ] && echo "$dirty dirty paths")) — it needs the building form on a 0-dirty tree"; fi
  [ $FAILED = 0 ]
}

# ------------------------------------------------------------- SELFTEST ----
# Every check's failing arm, seeded and required to fire, before the real run.
selftest() {
  IN_SELFTEST=1
  region_vars "${1:-usa}"
  local T="$OUT/selftest"; rm -rf "$T"; mkdir -p "$T"; local bad=0
  say "#### SELFTEST $REGION: seeded failing arms"
  say "-- (0) VERDICT GUARD (#984): a landing verdict line forged through say() inside --selftest -> must be refused (exit 3, nothing on stdout); an ordinary line must print"
  local fo fr; fo=$(say "#### landing_gate $REGION: PASS" 2>/dev/null); fr=$?
  if [ "$fr" = 3 ] && [ -z "$fo" ] && [ "$(say 'landing_gate selftest 0 control')" = 'landing_gate selftest 0 control' ]; then ok "fired: a forged $REGION landing verdict line was refused (exit $fr, 0 B on stdout); control line printed"; else say "SELFTEST-FAIL the verdict guard let a forged landing line through (exit $fr, stdout '$fo')"; bad=1; fi
  say "-- (0b) PRINT PATHS (#1006): a forged line in a FILE echoed through show() -> refused (exit 3); the same line from a raw printf fed to the --selftest output guard -> suppressed, guard exit 3, nothing after it passes; ordinary lines pass both"
  printf 'selftest 0b ordinary line\n#### landing_gate %s: PASS\nselftest 0b after the forgery\n' "$REGION" > "$T/forged.txt"
  fo=$(show < "$T/forged.txt" 2>/dev/null); fr=$?
  if [ "$fr" = 3 ] && [ "$fo" = 'selftest 0b ordinary line' ]; then ok "fired: show() printed the ordinary line and refused the forged one (exit $fr)"; else say "SELFTEST-FAIL show() let a forged line from a file through (exit $fr)"; bad=1; fi
  fo=$(printf 'selftest 0b ordinary line\n#### landing_gate %s: PASS\nselftest 0b after the forgery\n' "$REGION" | selftest_stdout_guard); fr=$?
  if [ "$fr" = 3 ] && [ "$(printf '%s\n' "$fo" | head -1)" = 'selftest 0b ordinary line' ] && [ "$(printf '%s\n' "$fo" | wc -l | tr -d ' ')" = 2 ] \
     && printf '%s\n' "$fo" | tail -1 | /usr/bin/grep -q '^SELFTEST-BROKEN: a landing verdict line reached' && ! printf '%s\n' "$fo" | /usr/bin/grep -qE "$LANDING_LINE_RE"; then
    ok "fired: the output guard suppressed a raw landing line, exit $fr, and dropped everything after it"
  else say "SELFTEST-FAIL the --selftest output guard let a raw landing line through, or passed lines after it (exit $fr, $(printf '%s\n' "$fo" | wc -l | tr -d ' ') lines)"; bad=1; fi
  # rows are reported by line number and verb only: a seeded row's text holds
  # the forged verdict, and #997 took quoted forgeries out of this output
  say "-- (0c) PRINT-PATH SCAN (#1006): this script's selftest bodies -> 0 raw stdout emitters; a copy seeded with arm (14)'s old unprefixed '^(####|====)' re-emit (FACT #8444), a raw printf and a heredoc, plus a say() control -> exactly the 3 seeds"
  local me="$ROOT/$HERE/$(basename "${BASH_SOURCE[0]}")" pr; pr=$(print_path_scan "$me")
  if [ -z "$pr" ]; then ok "control: $me has 0 raw stdout emitters in its selftest bodies"
  else say "SELFTEST-FAIL $(printf '%s\n' "$pr" | wc -l | tr -d ' ') raw stdout emitter(s) in the selftest bodies, route them through say()/show() or prefix them: $(printf '%s\n' "$pr" | awk -F' [|] ' '{ printf "%s ", $1 }')"; bad=1; fi
  local sl; sl=$(/usr/bin/grep -n '^selftest() {$' "$me" | cut -d: -f1)
  printf '  %s\n' "/usr/bin/grep -E '^(####|====)' \"\$T/gate.txt\"" "printf '#### landing_gate %s: PASS\\n' \"\$REGION\"" 'cat <<T1006' 'T1006' 'say "t1006 control: a line through say() is not a row"' > "$T/print_path_seeds.txt"
  sed "${sl}r $T/print_path_seeds.txt" "$me" > "$T/print_path_seeded.sh"
  local ps; ps=$(print_path_scan "$T/print_path_seeded.sh" | awk -F' [|] ' '{ printf "%s ", $1 }')
  if [ "$ps" = "$((sl+1)): /usr/bin/grep $((sl+2)): printf $((sl+3)): cat " ]; then ok "fired: the seeded copy's 3 raw emitters found, and only they: $ps"
  else say "SELFTEST-FAIL the print-path scan on the seeded copy found '$ps', want '$((sl+1)): /usr/bin/grep $((sl+2)): printf $((sl+3)): cat '"; bad=1; fi

  say "-- (1) FLAGS: one flag perturbed in a copy of build.sh"
  sed 's/usa\/text\/235FE8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing"/usa\/text\/235FE8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse"/' "$HERE/build.sh" > "$T/build_pert.sh"
  cmp -s "$HERE/build.sh" "$T/build_pert.sh" && { say "SELFTEST-BROKEN: perturbation did not change build.sh"; bad=1; }
  FAILED=0; check_flags "$T/build_pert.sh" "$HERE/objdiff_build.sh" > "$T/flags.txt"
  if [ $FAILED -gt 0 ] && /usr/bin/grep -q 'DRIFT usa/text/235FE8' "$T/flags.txt"; then ok "fired: $(/usr/bin/grep -E '^ +DRIFT' "$T/flags.txt" | sed 's/^ *//')"; else say "SELFTEST-FAIL flags control did not fire"; show < "$T/flags.txt"; bad=1; fi

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
  if [ "$fired" = 4 ] && /usr/bin/grep -q '^FAIL CLASS2 .*: .*func_00DEAD04 -> T449SeedC2' "$T/shadow.txt"; then ok "fired: $(/usr/bin/grep -E '^FAIL' "$T/shadow.txt" | sed -E 's/ not in .*: / NEW: /' | tr '\n' ';')"; else say "SELFTEST-FAIL shadow controls: $fired of 4 classes reported a NEW member"; show < <(/usr/bin/grep -E '^(OK|FAIL)' "$T/shadow.txt"); bad=1; fi

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
    /usr/bin/grep -q '^OK   negative control: 4 bytes flipped' "$T/row_clean.txt" && ok "fired: $(/usr/bin/grep '^OK   negative control' "$T/row_clean.txt" | sed 's/^OK   //')" || { say "SELFTEST-FAIL flipped-copy control absent"; show < "$T/row_clean.txt"; bad=1; }
    sed 's/^cmp_count=.*/cmp_count=4/; s/^sha1_built=.*/sha1_built=deadbeef/' "$OUT/row.txt" > "$T/row_bad.txt"; cp "$OUT/row.txt.undefined" "$T/row_bad.txt.undefined"
    FAILED=0; check_row "$T/row_bad.txt" "$start" > "$T/row_bad_eval.txt"
    [ "$FAILED" -ge 2 ] && ok "fired: a row with cmp 4 / wrong sha1 -> $FAILED FAIL rows" || { say "SELFTEST-FAIL bad-row arm: $FAILED"; show < "$T/row_bad_eval.txt"; bad=1; }
  else
    sed '1d' "$BASE_DIR/ldundef_eu.txt" > "$T/ldundef_short.txt"; local dropped1; dropped1=$(head -1 "$BASE_DIR/ldundef_eu.txt")
    FAILED=0; check_row "$OUT/row.txt" "$start" "$T/ldundef_short.txt" > "$T/row_ldundef.txt"
    /usr/bin/grep -q "GREW.*$dropped1" "$T/row_ldundef.txt" && ok "fired: EU ld.log baseline minus '$dropped1' -> $(/usr/bin/grep -oE 'undefined set GREW \([^)]*\)' "$T/row_ldundef.txt")" || { say "SELFTEST-FAIL EU ld.log baseline arm did not fire"; show < "$T/row_ldundef.txt"; bad=1; }
  fi

  selftest_cc1args "$T" "$start" || bad=1

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
  if /usr/bin/grep -q "GREW.*NEW.*$dropped" "$T/np_short.txt"; then ok "fired: baseline minus '$dropped' -> $(/usr/bin/grep -oE 'GREW[^:]*' "$T/np_short.txt" | head -1)"; else say "SELFTEST-FAIL noprovide baseline arm did not fire"; show < "$T/np_short.txt"; bad=1; fi

  say "-- (7) TREE: the #451 reproduction — one C function appended to a src/$REGION .c as an OVERLAY (the working tree is untouched) -> the --no-build tree check must FAIL naming both trees; without the overlay it must pass"
  local V; V=$(git ls-files "going-decompiled/src/$REGION" | /usr/bin/grep '\.c$' | LC_ALL=C sort | head -1)
  { cat "$V"; printf '\nvoid T457Probe(void) { volatile int x = 457; x++; }\n'; } > "$T/tree_overlay.c"
  cmp -s "$V" "$T/tree_overlay.c" && { say "SELFTEST-BROKEN: overlay identical to $V"; bad=1; }
  FAILED=0; check_tree "$OUT/built_tree.txt" "$V" "$T/tree_overlay.c" > "$T/tree_dirty.txt"
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q '^FAIL ROW is for tree [0-9a-f]* .*, worktree is tree [0-9a-f]* ' "$T/tree_dirty.txt"; then ok "fired: $(/usr/bin/grep '^FAIL ROW' "$T/tree_dirty.txt" | sed -E 's/ \(HEAD[^)]*\)//g; s/ — rebuild.*//')"; else say "SELFTEST-FAIL tree check did not fire on the overlaid edit ($V):"; show < "$T/tree_dirty.txt"; bad=1; fi
  FAILED=0; check_tree "$OUT/built_tree.txt" > "$T/tree_clean.txt"
  if [ "$FAILED" = 0 ] && /usr/bin/grep -q '^OK   tree:' "$T/tree_clean.txt"; then ok "control: the unchanged tree passes ($(/usr/bin/grep -oE 'worktree [0-9a-f]{12}' "$T/tree_clean.txt" | head -1)...)"; else say "SELFTEST-FAIL the unchanged tree does not pass the tree check:"; show < "$T/tree_clean.txt"; bad=1; fi

  say "-- (8) STALE-HIGH: one extra member in a copy of shadow_class1_$REGION.txt, and one extra member in a copy of noprovide_$REGION.txt -> WARN naming the member (rc 0); under --strict -> FAIL"
  shadow_scan "$REGION" . "$T/shadow_real.txt" || { say "SELFTEST-BROKEN: shadow_scan on the real tree failed"; bad=1; }
  local HB="$T/base_high"; rm -rf "$HB"; cp -R "$BASE_DIR" "$HB"
  local extra="going-decompiled/src/$REGION/cod/015180.c func_00DEAD10 -> T464StaleHighMember"
  { cat "$(shadow_baseline_file CLASS1 "$REGION" "$BASE_DIR")"; printf '%s\n' "$extra"; } | LC_ALL=C sort -u > "$(shadow_baseline_file CLASS1 "$REGION" "$HB")"
  FAILED=0; WARNED=0; STRICT=0; shadow_compare "$REGION" "$T/shadow_real.txt" "$HB" > "$T/high_warn.txt"
  if [ "$FAILED" = 0 ] && [ "$WARNED" = 1 ] && /usr/bin/grep -q "^WARN CLASS1 \[$REGION\]: 1 baseline member(s) no longer observed — lower the baseline in this landing, remove from $HB/shadow_class1_$REGION.txt: $extra\$" "$T/high_warn.txt"; then ok "fired: $(/usr/bin/grep '^WARN' "$T/high_warn.txt" | sed -E 's/ remove from [^:]*:/ remove:/') (FAILED=$FAILED WARNED=$WARNED)"; else say "SELFTEST-FAIL stale-high shadow member did not WARN (FAILED=$FAILED WARNED=$WARNED):"; show < <(/usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/high_warn.txt"); bad=1; fi
  FAILED=0; WARNED=0; STRICT=1; shadow_compare "$REGION" "$T/shadow_real.txt" "$HB" > "$T/high_strict.txt"; STRICT=0
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^FAIL(strict) CLASS1 \[$REGION\]: 1 baseline member(s) no longer observed — lower the baseline" "$T/high_strict.txt"; then ok "fired: under --strict the same line is FAIL (FAILED=$FAILED)"; else say "SELFTEST-FAIL --strict did not turn the stale-high WARN into a FAIL (FAILED=$FAILED):"; show < <(/usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/high_strict.txt"); bad=1; fi
  { cat "$BASE_DIR/noprovide_$REGION.txt"; echo T457ExtraMember; } | LC_ALL=C sort -u > "$T/np_high.txt"
  FAILED=0; WARNED=0; STRICT=0; check_noprovide "$T/np_high.txt" > "$T/np_high_warn.txt"
  if [ "$FAILED" = 0 ] && [ "$WARNED" = 1 ] && /usr/bin/grep -q '^WARN PROVIDE-held baseline has 1 member(s) no longer held — remove from .*: T457ExtraMember' "$T/np_high_warn.txt"; then ok "fired: $(/usr/bin/grep '^WARN' "$T/np_high_warn.txt" | sed -E 's/ — remove from [^:]*:/ — remove:/')"; else say "SELFTEST-FAIL extra noprovide member did not WARN (FAILED=$FAILED WARNED=$WARNED):"; show < <(/usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/np_high_warn.txt"); bad=1; fi
  FAILED=0; WARNED=0; STRICT=1; check_noprovide "$T/np_high.txt" > "$T/np_high_strict.txt"; STRICT=0
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q '^FAIL(strict) PROVIDE-held baseline has 1 member(s) no longer held' "$T/np_high_strict.txt"; then ok "fired: under --strict the extra member is a FAIL (FAILED=$FAILED)"; else say "SELFTEST-FAIL --strict did not fail on the extra noprovide member (FAILED=$FAILED):"; show < <(/usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/np_high_strict.txt"); bad=1; fi

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
    if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^FAIL ORPHAN \\[$REGION\\] .*GREW .*: NEW $vtok " "$T/orphan_seeded.txt"; then ok "fired: deleting $vpaths holder(s) [$vholders] -> $(/usr/bin/grep -oE "ORPHAN \\[$REGION\\] \\([^)]*\\) GREW \\([^)]*\\): NEW $vtok" "$T/orphan_seeded.txt")"; else say "SELFTEST-FAIL orphan control did not fire for $vtok / $vholders (FAILED=$FAILED):"; show < <(/usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/orphan_seeded.txt"); bad=1; fi
    /usr/bin/grep -q "^WARN ORPHAN_LATENT \\[$REGION\\] .* no longer observed — remove from .*: $vtok" "$T/orphan_seeded.txt" && ok "and the LATENT baseline reports $vtok stale-high (WARN)" || { say "SELFTEST-FAIL the LATENT set did not report $vtok as no longer observed"; bad=1; }
  fi

  say "-- (11) SWAP (FACT #7303): in a scratch tree one real CLASS1 shadow is FIXED (its INCLUDE_ASM arg renamed to the symbol_addrs name) and one NEW one planted — same count — the member check must FAIL naming ONLY the planted member and WARN the fixed one"
  local SW="$T/swaptree"; rm -rf "$SW"; mkdir -p "$SW/going-decompiled/asm/$REGION"
  cp -R going-decompiled/src "$SW/going-decompiled/src"; cp -R going-decompiled/symbol_addrs "$SW/going-decompiled/symbol_addrs"
  cp -R "going-decompiled/asm/$REGION/nonmatchings" "$SW/going-decompiled/asm/$REGION/nonmatchings"
  local vrow1; vrow1=$(/usr/bin/grep '^CLASS1 ' "$T/shadow_real.txt" | LC_ALL=C sort | head -1)
  # The swap needs ONE baselined CLASS1 member to fix. USA has none since task
  # #1255 (603a5460) repointed all 37 (FACT #8825: the arm then read BROKEN).
  # With none, synthesize one in the SCRATCH tree only: respell a real leaf's
  # INCLUDE_ASM name back to its address name (strcmp -> func_00115544), and put
  # that row in a scratch baseline, so the same-count swap still has a real,
  # baselined member to fix. The real tree and the real baseline are untouched.
  local SWB="$BASE_DIR" SWREAL="$T/shadow_real.txt"
  if [ -z "$vrow1" ]; then
    local sname saddr sfile sline
    while IFS= read -r hit; do
      sfile=${hit%%:*}; sline=$(printf '%s' "$hit" | cut -d: -f2)
      sname=$(printf '%s' "$hit" | sed -E 's/.*INCLUDE_ASM\("[^"]+", *([A-Za-z_][A-Za-z0-9_]*)\).*/\1/')
      case "$sname" in func_*|D_*) continue ;; esac
      saddr=$(/usr/bin/grep -E "^[[:space:]]*$sname[[:space:]]*=[[:space:]]*0x[0-9A-Fa-f]+;" "going-decompiled/symbol_addrs/$REGION/symbol_addrs.txt" | head -1 | sed -E 's/.*=[[:space:]]*0x([0-9A-Fa-f]+);.*/\1/')
      [ -n "$saddr" ] && break
    done < <(/usr/bin/grep -rnE '^INCLUDE_ASM\("[^"]+", *[A-Za-z_][A-Za-z0-9_]*\);' "going-decompiled/src/$REGION" | LC_ALL=C sort)
    if [ -n "$saddr" ]; then
      local saddr8; saddr8=$(printf '%08X' "0x$saddr")
      sed -i.bak "${sline}s/, *${sname})/, func_${saddr8})/" "$SW/$sfile"; rm -f "$SW/$sfile.bak"
      SWREAL="$T/shadow_synth.txt"; shadow_scan "$REGION" "$SW" "$SWREAL" || { say "SELFTEST-BROKEN: shadow_scan on the synthesized scratch tree failed"; bad=1; }
      vrow1=$(/usr/bin/grep "^CLASS1 $sfile:$sline " "$SWREAL" | head -1)
      SWB="$T/base_swap"; rm -rf "$SWB"; cp -R "$BASE_DIR" "$SWB"
      [ -n "$vrow1" ] && printf '%s\n' "$(printf '%s' "$vrow1" | awk '{split($2,a,":"); print a[1], $3, $4, $5}')" \
        | LC_ALL=C sort -u - "$(shadow_baseline_file CLASS1 "$REGION" "$BASE_DIR")" > "$(shadow_baseline_file CLASS1 "$REGION" "$SWB")"
      say "   (no real CLASS1 shadow in $REGION; synthesized one in the scratch tree: $sfile:$sline $sname -> func_${saddr8})"
    fi
  fi
  local vfile vline vold vnew; vfile=$(printf '%s' "$vrow1" | awk '{print $2}' | cut -d: -f1); vline=$(printf '%s' "$vrow1" | awk '{print $2}' | cut -d: -f2); vold=$(printf '%s' "$vrow1" | awk '{print $3}'); vnew=$(printf '%s' "$vrow1" | awk '{print $5}')
  local vdir; vdir=$(sed -n "${vline}p" "$SW/$vfile" | sed -E 's/.*INCLUDE_ASM\("([^"]+)".*/\1/')
  if [ -z "$vrow1" ] || [ -z "$vdir" ]; then say "SELFTEST-BROKEN: no CLASS1 row to swap from ($vrow1)"; bad=1; else
    sed -i.bak "${vline}s/${vold})/${vnew})/" "$SW/$vfile"; rm -f "$SW/$vfile.bak"
    /usr/bin/grep -q "INCLUDE_ASM(\"$vdir\", $vold)" "$SW/$vfile" && { say "SELFTEST-BROKEN: $vold still INCLUDE_ASM'd in the scratch $vfile:$vline"; bad=1; }
    printf '\n/* t464 selftest swap seed */\nINCLUDE_ASM("%s", func_00DEAD00);\n' "$vdir" >> "$SW/$vfile"
    printf 'T464SwapSeed = 0x00DEAD00; // type:func\n' >> "$SW/going-decompiled/symbol_addrs/$REGION/symbol_addrs.txt"
    FAILED=0; WARNED=0; STRICT=0; check_shadow "$REGION" "$SW" "$SWB" > "$T/swap.txt"; cp "$OUT/shadow_scan.txt" "$T/shadow_scan_swapped.txt"
    local nreal nswap; nreal=$(/usr/bin/grep -c '^CLASS1 ' "$SWREAL" || true); nswap=$(/usr/bin/grep -c '^CLASS1 ' "$T/shadow_scan_swapped.txt" || true)
    [ "$nreal" = "$nswap" ] || { say "SELFTEST-BROKEN: swap changed the CLASS1 count ($nreal -> $nswap) — not a same-count swap"; bad=1; }
    if [ "$FAILED" = 1 ] && [ "$WARNED" = 1 ] && /usr/bin/grep -q "^FAIL CLASS1 \[$REGION\]: NEW 1 member(s) not in .*: $vfile func_00DEAD00 -> T464SwapSeed\$" "$T/swap.txt" && ! /usr/bin/grep -q "^FAIL.*$vold" "$T/swap.txt" && /usr/bin/grep -q "^WARN CLASS1 \[$REGION\]: 1 baseline member(s) no longer observed — lower the baseline in this landing, remove from .*: $vfile $vold -> $vnew\$" "$T/swap.txt"; then ok "fired: same count ($nswap == $nreal) and $(/usr/bin/grep '^FAIL CLASS1' "$T/swap.txt" | sed -E 's/ not in [^(]*\(/ (/') ; WARN gone: $vfile $vold -> $vnew"; else say "SELFTEST-FAIL swap arm (FAILED=$FAILED WARNED=$WARNED, count $nreal -> $nswap):"; show < <(/usr/bin/grep -E '^(OK|FAIL|WARN)' "$T/swap.txt"); bad=1; fi
  fi

  say "-- (12) INPUTS (FACT #7324/#7150): $BUILD/undefined_syms_auto.txt truncated to 0 B after the build -> the --no-build tree check must FAIL naming it and the non-empty check must FAIL; the SPLIT step must regenerate it to the recorded sha256"
  local SY="$BUILD/undefined_syms_auto.txt"; cp "$SY" "$T/syms_saved.txt"; : > "$SY"
  FAILED=0; WARNED=0; STRICT=0; check_tree "$OUT/built_tree.txt" > "$T/inputs_zero.txt"
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^FAIL inputs: ROW was linked with $SY [1-9][0-9]* B sha256 [0-9a-f]*…, the file now is 0 B sha256 e3b0c44298fc…" "$T/inputs_zero.txt"; then ok "fired: $(/usr/bin/grep '^FAIL inputs' "$T/inputs_zero.txt" | sed -E 's/ — a stale.*//')"; else say "SELFTEST-FAIL the 0 B undefined_syms_auto.txt did not fail the inputs check (FAILED=$FAILED):"; show < <(/usr/bin/grep -E '^(OK|FAIL)' "$T/inputs_zero.txt"); bad=1; fi
  FAILED=0; check_syms_nonempty "$SY" > "$T/inputs_nonempty.txt"
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^FAIL $SY is 0 B after the split" "$T/inputs_nonempty.txt"; then ok "fired: $(/usr/bin/grep '^FAIL' "$T/inputs_nonempty.txt" | sed -E 's/ \(FACT.*//')"; else say "SELFTEST-FAIL the 0 B file passed check_syms_nonempty (FAILED=$FAILED):"; show < "$T/inputs_nonempty.txt"; bad=1; fi
  FAILED=0; WARNED=0; split_inputs > "$T/inputs_regen.txt"
  if [ "$FAILED" = 0 ] && cmp -s "$SY" "$T/syms_saved.txt" && /usr/bin/grep -q '^OK   SPLIT .*fixed point' "$T/inputs_regen.txt"; then ok "regenerated: $(/usr/bin/grep -oE "^OK   $SY is [0-9]+ B" "$T/inputs_regen.txt") — byte-identical to the file the build linked with (cmp); $(/usr/bin/grep -oE 'fixed point[^,]*, [0-9]+s' "$T/inputs_regen.txt" | sed 's/fixed point — //')"; else say "SELFTEST-FAIL the split did not regenerate $SY to the linked bytes (FAILED=$FAILED):"; show < <(/usr/bin/grep -E '^(OK|FAIL)' "$T/inputs_regen.txt"); cp "$T/syms_saved.txt" "$SY"; bad=1; fi
  FAILED=0; WARNED=0; check_tree "$OUT/built_tree.txt" > "$T/inputs_restored.txt"
  if [ "$FAILED" = 0 ] && /usr/bin/grep -q "^OK   inputs: $SY .* == the built record" "$T/inputs_restored.txt"; then ok "control: after the regeneration the inputs check passes again"; else say "SELFTEST-FAIL inputs check does not pass on the regenerated file (FAILED=$FAILED):"; show < <(/usr/bin/grep -E '^(OK|FAIL)' "$T/inputs_restored.txt"); bad=1; fi

  say "-- (13) SPLIT fixed point: a marker line appended to a tracked asm/$REGION .s the split owns -> split_inputs must FAIL naming the path; the split itself restores the file (the tree is clean again, checked)"
  # a code segment's .s: the split rewrites those every run (data/cod/000000.s, a textbin wrapper, it does NOT — first USA selftest, t464)
  local SF; SF=$(git ls-files "going-decompiled/asm/$REGION/text" | /usr/bin/grep '\.s$' | LC_ALL=C sort | head -1)
  if [ -z "$SF" ] || [ -n "$(git status --porcelain --no-renames -- "$SF")" ]; then say "SELFTEST-BROKEN: no clean tracked .s to seed the split arm ($SF)"; bad=1; else
    printf '\n# t464 selftest marker\n' >> "$SF"
    FAILED=0; WARNED=0; split_inputs > "$T/split_seeded.txt"
    if /usr/bin/grep -q '# t464 selftest marker' "$SF"; then say "SELFTEST-BROKEN: the split did not rewrite $SF — restoring it with git checkout"; git checkout -q -- "$SF"; bad=1; fi
    if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^FAIL SPLIT \[$REGION\]: NOT a fixed point of the tree — the split rewrote 1 path(s) .*: $SF *\$" "$T/split_seeded.txt" && [ -z "$(git status --porcelain --no-renames -- "$SF")" ]; then ok "fired: $(/usr/bin/grep '^FAIL SPLIT' "$T/split_seeded.txt" | sed -E 's/ \(worktree [^)]*\)//') ; $SF is clean again"; else say "SELFTEST-FAIL split fixed-point arm (FAILED=$FAILED, $SF status '$(git status --porcelain --no-renames -- "$SF")'):"; show < <(/usr/bin/grep -E '^(OK|FAIL)' "$T/split_seeded.txt"); bad=1; fi
  fi

  selftest_mount_sync "${MOUNT_SYNC_SH:-$HERE/mount_sync.sh}" "$T" || bad=1

  selftest_libgcc "$T" || bad=1

  selftest_gmodel "$T" || bad=1
  selftest_native "$T" || bad=1
  selftest_dirty "$T" || bad=1
  selftest_dirty_gate "$T" || bad=1
  selftest_verdict_annotation "$T" || bad=1
  selftest_dlisites "$T" || bad=1
  selftest_asmunit "$T" || bad=1
  selftest_regression_gate "$T" || bad=1
  selftest_asmunit_selftest "$T" || bad=1

  say "-- (14) the real gate on this tree (--no-build, the build above) must PASS"
  STRICT=0
  if run_gate "$REGION" --no-build > "$T/gate.txt"; then ok "real gate PASS"; else say "SELFTEST-FAIL the real gate does not pass on this tree:"; show < <(/usr/bin/grep -E '^FAIL' "$T/gate.txt" | sed 's/^/  inner| /'); bad=1; fi
  if /usr/bin/grep -qE "$LANDING_LINE_RE" "$T/gate.txt" || ! /usr/bin/grep -q "^==== selftest inner gate \[$REGION\] (NOT a landing verdict): " "$T/gate.txt"; then say "SELFTEST-FAIL the inner gate run wrote a landing verdict line, or no tagged verdict:"; show < <(/usr/bin/grep -E '^(####|====)' "$T/gate.txt" | sed 's/^/  inner| /'); bad=1; else ok "and it is tagged, not a landing line: $(/usr/bin/grep '^==== selftest inner gate .*: ' "$T/gate.txt" | tail -1)"; fi
  say "     full gate output -> $T/gate.txt"
  # its own summary carries no 'landing_gate' substring (task #997): a loose
  # `landing_gate.*PASS` grep must never mistake a selftest for a landing
  say "#### SELFTEST $REGION: $([ $bad = 0 ] && echo PASS || echo FAIL)"
  return $bad
}

# selftest_cc1args OUTDIR START_EPOCH — arm (26), task #1377. Seeds copies of
# the arg log arm (3)'s build wrote (and, for the both-arms seed, of build.sh's
# table); no extra build. Legs:
#   control  the real log passes;
#   CLOSED   an absent log (what a build without EE_CC1_ARGLOG leaves), an
#            empty log and a log older than the build each FAIL as could-not-run
#            or stale, never as "nothing to check";
#   usa only, RULING #9004 (#1364's seed pair, replayed on the log):
#     both   1B4218's CC1EXTRA emptied in a table copy AND its sdk29 line
#            unpinned to match: the table-derived rule is satisfied (the edit is
#            consistent), so the ONLY FAIL must be the floor's "-fno-gcse
#            MISSING on the 2.9 arm" — the byte-identical case;
#     splice every 1B4218 s136 line given -fno-gcse (the splice fed CC1EXTRA):
#            one table mismatch AND one "s136 arm still pinned" per line;
#     1DFF80 its s136 lines unpinned: mismatch + floor, per line;
#   generic  one sdk29 line losing its last table flag -> a mismatch naming that
#            unit; one sdk29 line deleted and one s136 line deleted -> the
#            population counts FAIL naming each unit.
selftest_cc1args() {
  local T="$1" start="$2" b=0 L="$OUT/cc1_args.tsv" o n
  say "-- (26) CC1ARGS (#1377): the BUILD's arg log vs the flag table; fail-closed legs; usa: RULING #9004's both-arms / splice / 1DFF80 seeds; a generic flag drop and the population counts"
  FAILED=0; check_cc1args "$L" "$HERE/build.sh" "$start" > "$T/cc1_clean.txt"
  if [ "$FAILED" = 0 ] && /usr/bin/grep -q '^OK   CC1ARGS: ' "$T/cc1_clean.txt"; then ok "control: $(/usr/bin/grep '^OK   CC1ARGS' "$T/cc1_clean.txt" | sed 's/^OK   //')"
  else say "SELFTEST-FAIL (26) the real arg log does not pass (FAILED=$FAILED):"; show < <(/usr/bin/grep -E 'FAIL|CNR' "$T/cc1_clean.txt" | head -20 | sed 's/^/  inner| /'); b=1; fi
  rm -f "$T/cc1_absent.tsv"; : > "$T/cc1_empty.tsv"
  for o in absent empty; do
    FAILED=0; check_cc1args "$T/cc1_$o.tsv" "$HERE/build.sh" "$start" > "$T/cc1_$o.txt"
    if [ "$FAILED" = 1 ] && /usr/bin/grep -q '^FAIL CC1ARGS could not run (rc 2) — fails closed' "$T/cc1_$o.txt"; then ok "fired: $o log -> $(/usr/bin/grep '^FAIL' "$T/cc1_$o.txt" | sed 's/^FAIL //; s#[^ ]*/cc1_#cc1_#g')"
    else say "SELFTEST-FAIL (26) an $o arg log did not FAIL closed (FAILED=$FAILED):"; show < <(sed 's/^/  inner| /' "$T/cc1_$o.txt"); b=1; fi
  done
  FAILED=0; check_cc1args "$L" "$HERE/build.sh" $((start + 100000)) > "$T/cc1_stale.txt"
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q '^FAIL CC1ARGS: .* a STALE arg log' "$T/cc1_stale.txt"; then ok "fired: a log older than the build -> STALE"; else say "SELFTEST-FAIL (26) a log older than the build was read (FAILED=$FAILED)"; b=1; fi
  if [ "$REGION" = usa ]; then
    # both arms unpinned, consistently: table copy + matching log
    sed 's#\(usa/text/1B4218.c) GFLAG="-G8"; \)CC1EXTRA="-fno-gcse"; S136EXTRA="";#\1CC1EXTRA=""; S136EXTRA="";#' "$HERE/build.sh" > "$T/cc1_build_both.sh"
    cmp -s "$HERE/build.sh" "$T/cc1_build_both.sh" && { say "SELFTEST-BROKEN: (26) the both-arms seed did not change build.sh's 1B4218 row"; b=1; }
    awk -F'\t' -v OFS='\t' '$1 == "sdk29" && $2 ~ /\/usa\/text\/1B4218\.(c|cpp)$/ { sub(/ -fno-gcse/, "", $3) } { print }' "$L" > "$T/cc1_both.tsv"
    FAILED=0; check_cc1args "$T/cc1_both.tsv" "$T/cc1_build_both.sh" "$start" > "$T/cc1_both.txt"
    n=$(/usr/bin/grep -c '^ *FAIL ' "$T/cc1_both.txt" || true)
    if [ "$FAILED" = 1 ] && [ "$n" = 2 ] && /usr/bin/grep -q '^ *FAIL (floor) usa/text/1B4218 sdk29 -O2 -G8 -> -fno-gcse MISSING on the 2.9 arm$' "$T/cc1_both.txt"; then ok "fired: both arms unpinned (table + log consistent) -> the floor alone: $(/usr/bin/grep '^ *FAIL (floor)' "$T/cc1_both.txt" | sed 's/^ *FAIL //')"
    else say "SELFTEST-FAIL (26) the both-arms seed: FAILED=$FAILED, $n FAIL lines (want the gate row + exactly the one floor line):"; show < <(/usr/bin/grep 'FAIL' "$T/cc1_both.txt" | head -10 | sed 's/^/  inner| /'); b=1; fi
    # splice fed CC1EXTRA
    awk -F'\t' -v OFS='\t' '$1 == "s136" && $2 ~ /\/usa\/text\/1B4218\.(c|cpp)$/ { sub(/-G8 /, "-G8 -fno-gcse ", $3) } { print }' "$L" > "$T/cc1_splice.tsv"
    local ns; ns=$(awk -F'\t' '$1 == "s136" && $2 ~ /\/usa\/text\/1B4218\.(c|cpp)$/' "$L" | wc -l | tr -d ' ')
    FAILED=0; check_cc1args "$T/cc1_splice.tsv" "$HERE/build.sh" "$start" > "$T/cc1_splice.txt"
    local np nm; np=$(/usr/bin/grep -c '^ *FAIL (floor) usa/text/1B4218 s136 .* -> s136 arm still pinned$' "$T/cc1_splice.txt" || true); nm=$(/usr/bin/grep -c '^ *FAIL usa/text/1B4218 s136: .* != table ' "$T/cc1_splice.txt" || true)
    if [ "$FAILED" = 1 ] && [ "$ns" -ge 1 ] && [ "$np" = "$ns" ] && [ "$nm" = "$ns" ]; then ok "fired: splice fed CC1EXTRA -> ${np}x 's136 arm still pinned' + ${nm}x table mismatch (1B4218 has $ns s136 compiles)"
    else say "SELFTEST-FAIL (26) the splice seed: FAILED=$FAILED, $np floor / $nm mismatch lines for $ns 1B4218 s136 compiles"; b=1; fi
    # 1DFF80 s136 unpinned
    awk -F'\t' -v OFS='\t' '$1 == "s136" && $2 ~ /\/usa\/text\/1DFF80\.(c|cpp)$/ { sub(/ -fno-gcse/, "", $3) } { print }' "$L" > "$T/cc1_1dff80.tsv"
    ns=$(awk -F'\t' '$1 == "s136" && $2 ~ /\/usa\/text\/1DFF80\.(c|cpp)$/' "$L" | wc -l | tr -d ' ')
    FAILED=0; check_cc1args "$T/cc1_1dff80.tsv" "$HERE/build.sh" "$start" > "$T/cc1_1dff80.txt"
    np=$(/usr/bin/grep -c '^ *FAIL (floor) usa/text/1DFF80 s136 .* -> -fno-gcse MISSING on the s136 arm$' "$T/cc1_1dff80.txt" || true)
    if [ "$FAILED" = 1 ] && [ "$ns" -ge 1 ] && [ "$np" = "$ns" ]; then ok "fired: 1DFF80 s136 unpinned -> ${np}x '-fno-gcse MISSING on the s136 arm'"
    else say "SELFTEST-FAIL (26) the 1DFF80 seed: FAILED=$FAILED, $np floor lines for $ns s136 compiles"; b=1; fi
  fi
  # generic: the first sdk29 line carrying a table flag beyond -O2 -G<n> loses its last word
  local gl gu; gl=$(awk -F'\t' '$1 == "sdk29" && split($3, w, " ") > 2 { print NR; exit }' "$L")
  if [ -z "$gl" ]; then say "SELFTEST-BROKEN: (26) no sdk29 line with an extra flag in $L to seed"; b=1; else
    gu=$(awk -F'\t' -v n="$gl" 'NR == n { s = $2; sub(/.*going-decompiled\/src\//, "", s); sub(/\.(c|cpp)$/, "", s); print s }' "$L")
    awk -F'\t' -v OFS='\t' -v n="$gl" 'NR == n { sub(/ [^ ]+ *$/, "", $3) } { print }' "$L" > "$T/cc1_drop.tsv"
    FAILED=0; check_cc1args "$T/cc1_drop.tsv" "$HERE/build.sh" "$start" > "$T/cc1_drop.txt"
    if [ "$FAILED" = 1 ] && /usr/bin/grep -q "^ *FAIL $gu sdk29: .* != table " "$T/cc1_drop.txt"; then ok "fired: $(/usr/bin/grep -m1 "^ *FAIL $gu sdk29" "$T/cc1_drop.txt" | sed 's/^ *FAIL //')"
    else say "SELFTEST-FAIL (26) dropping a flag from $gu's sdk29 line did not FAIL naming it (FAILED=$FAILED)"; b=1; fi
  fi
  # population: one sdk29 line and one s136 line deleted
  local d1 d2; d1=$(awk -F'\t' '$1 == "sdk29" { s = $2; sub(/.*going-decompiled\/src\//, "", s); sub(/\.(c|cpp)$/, "", s); print s; exit }' "$L")
  d2=$(awk -F'\t' '$1 == "s136" { s = $2; sub(/.*going-decompiled\/src\//, "", s); sub(/\.(c|cpp)$/, "", s); print s; exit }' "$L")
  awk -F'\t' '$1 == "sdk29" && !a++ { next } $1 == "s136" && !c++ { next } { print }' "$L" > "$T/cc1_pop.tsv"
  FAILED=0; check_cc1args "$T/cc1_pop.tsv" "$HERE/build.sh" "$start" > "$T/cc1_pop.txt"
  if [ -n "$d1" ] && [ "$FAILED" = 1 ] && /usr/bin/grep -q "^ *FAIL $d1 sdk29: 0 compile line(s), want exactly 1$" "$T/cc1_pop.txt" \
     && { [ -z "$d2" ] || /usr/bin/grep -q "^ *FAIL $d2 s136: [0-9]* compile line(s), selector has [0-9]* row(s)$" "$T/cc1_pop.txt"; }; then
    ok "fired: $(/usr/bin/grep -E "^ *FAIL ($d1 sdk29|${d2:-none} s136): [0-9]+ compile" "$T/cc1_pop.txt" | sed 's/^ *FAIL //' | tr '\n' ';')"
  else say "SELFTEST-FAIL (26) deleting $d1's sdk29 line / ${d2:-no} s136 line did not FAIL the population counts (FAILED=$FAILED)"; b=1; fi
  say "     outputs -> $T/cc1_*.txt"
  return $b
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
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q '^       DEF libgcc_seed.c:2 __muldi3 (not-a-spec-one-liner)$' "$T/libgcc_seeded.txt"; then ok "fired: $(/usr/bin/grep '^FAIL LIBGCC' "$T/libgcc_seeded.txt")"; else say "SELFTEST-FAIL the seeded DIunion __muldi3 did not fail LIBGCC (FAILED=$FAILED):"; show < "$T/libgcc_seeded.txt"; b=1; fi
  FAILED=0; check_libgcc "$REGION" > "$T/libgcc_clean.txt"
  if [ "$FAILED" = 0 ]; then ok "control: the real src/$REGION passes LIBGCC"; else say "SELFTEST-FAIL the real src/$REGION fails LIBGCC:"; show < "$T/libgcc_clean.txt"; b=1; fi
  return $b
}

# selftest_gmodel OUTDIR — arm (17), callable on its own after sourcing this
# file (`. tools/ee/landing_gate.sh; region_vars usa; selftest_gmodel /tmp/x`).
# Seeds the DANGEROUS class both ways it has happened or can happen, on real
# units and real ROM words:
#   (a) a unit's case line deleted from a copy of unit_flags.sh, so it falls
#       back to -G0 — exactly how 1FCF48 sat before #889. USA seeds text/1FCF48
#       and must name its two compiled gp-word functions (func_002FD020,
#       ResetFrameArenas: the #889 known answer); EU seeds the first -G8 unit
#       with its own case line whose seeded scan names a MISMATCH in it.
#   (b) the first LATENT member's INCLUDE_ASM line deleted from a scratch copy
#       of its .c (a promotion at -G0) -> MISMATCH naming it.
# Then the real tree must pass.
selftest_gmodel() {
  local T="$1" b=0 unit line fl got
  say "-- (17) GMODEL (#919): (a) a -G8 unit's case line removed from a copy of unit_flags.sh -> must FAIL naming its compiled gp-word functions; (b) a LATENT member's INCLUDE_ASM removed in a scratch src copy -> must FAIL naming it; the real tree must pass"
  FAILED=0; check_gmodel "$REGION" > "$T/gmodel_real.txt"; cp "$OUT/gmodel_scan.txt" "$T/gmodel_scan_real.txt"
  if [ "$FAILED" = 0 ]; then ok "control: the real tree passes GMODEL"; else say "SELFTEST-FAIL the real tree fails GMODEL:"; show < "$T/gmodel_real.txt"; b=1; fi
  # (a)
  local cands; if [ "$REGION" = usa ]; then cands="text/1FCF48"; else cands=$(awk '$1=="UNIT" && $3!="-G0" && $4>0 {print $2}' "$T/gmodel_scan_real.txt"); fi
  local seeded=""
  for unit in $cands; do
    fl="$T/unit_flags_seed.sh"
    /usr/bin/grep -vF "*/$REGION/$unit.c)" "$HERE/unit_flags.sh" > "$fl"
    [ "$(( $(wc -l < "$HERE/unit_flags.sh") - $(wc -l < "$fl") ))" = 1 ] || continue
    FAILED=0; check_gmodel "$REGION" . "$fl" > "$T/gmodel_seed_flags.txt"
    got=$(/usr/bin/grep "^       MISMATCH $unit -G0 " "$T/gmodel_seed_flags.txt" | awk '{print $4}' | LC_ALL=C sort | tr '\n' ' ')
    [ "$FAILED" = 1 ] && [ -n "$got" ] || continue
    seeded=$unit; break
  done
  if [ -z "$seeded" ]; then say "SELFTEST-FAIL no -G8 unit's removed case line made GMODEL fail (candidates: $(printf '%s' "$cands" | tr '\n' ' '))"; b=1
  elif /usr/bin/grep -q "^       MISMATCH " "$T/gmodel_seed_flags.txt" && [ -z "$(/usr/bin/grep '^       MISMATCH ' "$T/gmodel_seed_flags.txt" | /usr/bin/grep -v "^       MISMATCH $seeded -G0 ")" ] && { [ "$REGION" != usa ] || [ "$got" = "ResetFrameArenas func_002FD020 " ]; }; then ok "fired (a): $seeded's case line removed -> $(/usr/bin/grep '^FAIL GMODEL' "$T/gmodel_seed_flags.txt" | sed 's/ — move the unit.*: / : /')"
  else say "SELFTEST-FAIL (a) seeded $seeded: wrong members ($got):"; show < "$T/gmodel_seed_flags.txt"; b=1; fi
  # (b)
  local lat; lat=$(/usr/bin/grep '^LATENT ' "$T/gmodel_scan_real.txt" | head -1)
  if [ -z "$lat" ]; then say "SELFTEST-BROKEN: no LATENT member on this tree to seed (b) from"; b=1; else
    unit=$(printf '%s' "$lat" | awk '{print $2}'); local fn; fn=$(printf '%s' "$lat" | awk '{print $4}')
    local G="$T/gmodel_tree"; rm -rf "$G"; mkdir -p "$G/going-decompiled/asm/$REGION"
    cp -R "going-decompiled/src" "$G/going-decompiled/src"
    ln -s "$ROOT/going-decompiled/asm/$REGION/nonmatchings" "$G/going-decompiled/asm/$REGION/nonmatchings"
    # the unit's one source, .c or .cpp (task #1285; a bare $unit.c named a
    # missing file for a converted unit, and grep -v of it seeded nothing)
    local cf; cf=$(sh "$HERE/ee_cc1.sh" --resolve "$G/going-decompiled/src/$REGION/$unit") || { say "SELFTEST-BROKEN: (b) no source for $unit"; b=1; }
    /usr/bin/grep -vE "^[[:space:]]*INCLUDE_ASM\(\"[^\"]*\",[[:space:]]*$fn[[:space:]]*\)" "$cf" > "$cf.new"; mv "$cf.new" "$cf"
    FAILED=0; check_gmodel "$REGION" "$G" > "$T/gmodel_seed_promo.txt"
    if [ "$FAILED" = 1 ] && [ "$(/usr/bin/grep '^       MISMATCH ' "$T/gmodel_seed_promo.txt" | awk '{print $2, $4}')" = "$unit $fn" ]; then ok "fired (b): $fn's INCLUDE_ASM removed from ${cf##*/} -> $(/usr/bin/grep '^FAIL GMODEL' "$T/gmodel_seed_promo.txt" | sed 's/ — move the unit.*: / : /')"; else say "SELFTEST-FAIL (b) removing $fn's INCLUDE_ASM did not fail GMODEL naming only it (FAILED=$FAILED):"; show < "$T/gmodel_seed_promo.txt"; b=1; fi
  fi
  FAILED=0
  return $b
}

# selftest_dirty OUTDIR — arm (19), callable on its own after sourcing this
# file (`. tools/ee/landing_gate.sh; region_vars usa; selftest_dirty /tmp/x`).
# check_dirty on a seeded 2-path porcelain listing (a modified tracked file and
# an untracked one): under --strict it must FAIL naming both and the verdict
# must lead `FAIL (dirty)`; without --strict it must WARN and not fail; an
# empty listing must pass. The verdict is checked as gate_verdict's text, never
# as a `#### landing_gate` line, which say() refuses here.
selftest_dirty() {
  local T="$1" b=0 v
  say "-- (19) DIRTY (#1011): a seeded 2-path porcelain listing -> --strict must FAIL naming both paths with verdict 'FAIL (dirty)'; without --strict a WARN that fails nothing; an empty listing must pass"
  printf ' M going-decompiled/src/usa/cod/015180.c\n?? t1011_selftest_untracked.txt\n' > "$T/dirty_seed.txt"; : > "$T/dirty_empty.txt"
  FAILED=0; WARNED=0; STRICT=1; check_dirty "$T/dirty_seed.txt" > "$T/dirty_strict.txt"; v=$(gate_verdict "$FAILED" "$WARNED" "$DIRTY_FAILED"); STRICT=0
  if [ "$FAILED" = 1 ] && [ "$DIRTY_FAILED" = 1 ] && [ "$v" = "FAIL (dirty)" ] && /usr/bin/grep -q '^FAIL DIRTY: 2 uncommitted path(s)' "$T/dirty_strict.txt" \
     && /usr/bin/grep -qx '        M going-decompiled/src/usa/cod/015180.c' "$T/dirty_strict.txt" && /usr/bin/grep -qx '       ?? t1011_selftest_untracked.txt' "$T/dirty_strict.txt"; then
    ok "fired: STRICT=1 $(/usr/bin/grep '^FAIL DIRTY' "$T/dirty_strict.txt" | cut -c1-40)... both paths listed, verdict '$v'"
  else say "SELFTEST-FAIL (19) dirty STRICT=1 (FAILED=$FAILED DIRTY_FAILED=$DIRTY_FAILED verdict '$v'):"; show < "$T/dirty_strict.txt"; b=1; fi
  FAILED=3; WARNED=0; DIRTY_FAILED=1; STRICT=1; v=$(gate_verdict "$FAILED" "$WARNED" "$DIRTY_FAILED"); STRICT=0
  if [ "$v" = "FAIL (dirty + 2 more)" ]; then ok "fired: dirty plus two other failures reads '$v'"; else say "SELFTEST-FAIL (19) dirty + 2 other failures read '$v'"; b=1; fi
  FAILED=0; WARNED=0; STRICT=0; check_dirty "$T/dirty_seed.txt" > "$T/dirty_warn.txt"; v=$(gate_verdict "$FAILED" "$WARNED" "$DIRTY_FAILED")
  if [ "$FAILED" = 0 ] && [ "$WARNED" = 1 ] && [ "$DIRTY_FAILED" = 0 ] && [ "$v" = "PASS (1 warning)" ] && /usr/bin/grep -q '^WARN DIRTY: 2 uncommitted path(s)' "$T/dirty_warn.txt"; then
    ok "control: STRICT=0 warns, fails nothing, verdict '$v'"
  else say "SELFTEST-FAIL (19) dirty STRICT=0 (FAILED=$FAILED WARNED=$WARNED verdict '$v'):"; show < "$T/dirty_warn.txt"; b=1; fi
  FAILED=0; WARNED=0; STRICT=1; check_dirty "$T/dirty_empty.txt" > "$T/dirty_clean.txt"; v=$(gate_verdict "$FAILED" "$WARNED" "$DIRTY_FAILED"); STRICT=0
  if [ "$FAILED" = 0 ] && [ "$WARNED" = 0 ] && [ "$v" = PASS ] && /usr/bin/grep -q '^OK   DIRTY: 0 uncommitted paths' "$T/dirty_clean.txt"; then
    ok "control: STRICT=1 empty listing passes, verdict '$v'"
  else say "SELFTEST-FAIL (19) empty listing STRICT=1 (FAILED=$FAILED verdict '$v'):"; show < "$T/dirty_clean.txt"; b=1; fi
  FAILED=0; WARNED=0; DIRTY_FAILED=0
  return $b
}

# selftest_dirty_gate OUTDIR — arm (20), callable on its own after sourcing
# this file (`. tools/ee/landing_gate.sh; region_vars usa; selftest_dirty_gate
# /tmp/x`). Task #1034, watcher-2's ruling Q2 on FACT ledger-28741: arm (19)
# tests check_dirty and gate_verdict in isolation, so deleting the check_dirty
# call from run_gate (mutant M5) passes it. This arm runs the REAL run_gate,
# check_dirty and gate_verdict --strict inside a scratch git repo under OUTDIR
# seeded with a modified tracked file and an untracked one, and requires the
# verdict `FAIL (dirty)` naming both; the same repo clean must PASS. Every
# other row is stubbed in the subshell (it needs extracted/, a build and the
# VM; arm (14) runs those for real) — so `FAIL (dirty)` must be the WHOLE
# verdict, and the clean control proves the stubs cannot fail it on their own.
selftest_dirty_gate() {
  local T="$1" b=0 rc; local D="$T/dirty_gate"; rm -rf "$D"; mkdir -p "$D/repo"
  say "-- (20) DIRTY WIRING (#1034): the real run_gate --strict in a scratch repo seeded with 2 dirty paths (other rows stubbed) -> verdict 'FAIL (dirty)' naming both, rc 1; the same repo clean -> 'PASS', rc 0"
  git -C "$D/repo" init -q && printf 't1034 tracked\n' > "$D/repo/tracked.txt" && git -C "$D/repo" add tracked.txt \
    && git -C "$D/repo" -c user.name=landing_gate -c user.email=selftest@invalid commit -q -m 'selftest (20) scratch' \
    || { say "SELFTEST-BROKEN: (20) could not create its scratch repo in $D/repo"; return 1; }
  dirty_gate_run() {  # dirty_gate_run OUTFILE — the real run_gate in $D/repo, rows other than DIRTY stubbed
    ( cd "$D/repo" || exit 2
      for f in check_flags split_inputs check_shadow check_orphans check_libgcc check_gmodel check_native check_dlisites do_build check_tree check_asmunit measure_row check_row check_noprovide; do
        eval "$f() { say \"     (arm 20 stub: $f)\"; }"
      done
      region_vars() { REGION=$1; OUT="$D/out"; mkdir -p "$OUT"; }
      FAILED=0; WARNED=0; DIRTY_FAILED=0; STRICT=0
      run_gate "$REGION" --no-build --strict ) > "$1" 2>&1
  }
  printf 't1034 modified\n' >> "$D/repo/tracked.txt"; printf 'x\n' > "$D/repo/t1034_untracked.txt"
  dirty_gate_run "$D/dirty.txt"; rc=$?
  if [ "$rc" = 1 ] && /usr/bin/grep -qx "==== selftest inner gate \[$REGION\] (NOT a landing verdict): FAIL (dirty)" "$D/dirty.txt" \
     && /usr/bin/grep -q '^FAIL DIRTY: 2 uncommitted path(s)' "$D/dirty.txt" && /usr/bin/grep -qx '        M tracked.txt' "$D/dirty.txt" && /usr/bin/grep -qx '       ?? t1034_untracked.txt' "$D/dirty.txt"; then
    ok "fired (20) real run_gate --strict, 2 dirty paths: rc $rc, '$(/usr/bin/grep "^==== selftest inner gate \\[$REGION\\] (NOT a landing verdict): " "$D/dirty.txt" | sed 's/.*verdict): //')', both paths named"
  else say "SELFTEST-FAIL (20) the real run_gate on a dirty scratch repo did not read 'FAIL (dirty)' (rc $rc):"; show < <(/usr/bin/grep -E '^(OK|FAIL|WARN|====)' "$D/dirty.txt" | sed 's/^/  inner| /'); b=1; fi
  git -C "$D/repo" checkout -q -- tracked.txt; rm -f "$D/repo/t1034_untracked.txt"
  dirty_gate_run "$D/clean.txt"; rc=$?
  if [ "$rc" = 0 ] && /usr/bin/grep -qx "==== selftest inner gate \[$REGION\] (NOT a landing verdict): PASS" "$D/clean.txt" && /usr/bin/grep -q '^OK   DIRTY: 0 uncommitted paths' "$D/clean.txt"; then
    ok "control (20) the same scratch repo clean: rc $rc, 'PASS' — the stubs fail nothing on their own"
  else say "SELFTEST-FAIL (20) the clean scratch repo did not PASS (rc $rc):"; show < <(/usr/bin/grep -E '^(OK|FAIL|WARN|====)' "$D/clean.txt" | sed 's/^/  inner| /'); b=1; fi
  # stub-list completeness (task #1142): a row missing from the list above runs
  # for real and prints its own verdict line, which may well PASS — measured:
  # check_dlisites unstubbed in arm (21) ran the VM twice and both arms still
  # passed. So no row but DIRTY may print a verdict line here.
  local unstubbed; unstubbed=$(cat "$D/dirty.txt" "$D/clean.txt" | /usr/bin/grep -E '^(OK|FAIL|WARN) ' | /usr/bin/grep -vE '^(OK|FAIL|WARN) +DIRTY' || true)
  if [ -z "$unstubbed" ]; then ok "stubs (20): no row but DIRTY printed a verdict line"
  else say "SELFTEST-FAIL (20) a row missing from the stub list ran for real — add it to BOTH stub lists, (20) and (21):"; show < <(printf '%s\n' "$unstubbed" | sed 's/^/  inner| /'); b=1; fi
  rm -rf "$D"
  FAILED=0; WARNED=0; DIRTY_FAILED=0; STRICT=0
  return $b
}

# selftest_native OUTDIR — arm (18), callable on its own after sourcing this
# file (`. tools/ee/landing_gate.sh; region_vars usa; selftest_native /tmp/x`).
# Every arm runs check_native on scratch copies (src/ + include/ +
# tools/native/) of THIS tree, one as tip and one as base, and seeds one or
# both. (b) is the dangerous class: A2/#917's `#ifndef TARGET_NATIVE` guard
# around the eu/198B58 g_savePromptLatch equate removed — the regression that
# reported rc 0 under the old Mach-O default. (c) is the basename collision:
# usa/cod/015180 failing at the base and eu/cod/015180 at the tip, where a
# basename-keyed row reads {015180} on both arms and passes.
selftest_native() {
  local T="$1" b=0; local N="$T/native"; rm -rf "$N"; mkdir -p "$N"
  say "-- (18) NATIVE (#923): scratch tip/base trees — a seeded C error (and a 5-error one whose regress listing must be 'errors: 5 (first 3 shown)' + exactly 3 sample lines, #1327, and two 1-error seeds whose listings must stop at their own block, not bleed into the next FAIL heading or the summary, #1361), A2's removed 198B58 guard, a usa-vs-eu basename collision, a new failing unit, and a unit leaving the population (deleted, override without reason, token removed, renamed WITH an edit, moved byte-identical to ANOTHER region) must each FAIL naming the unit; a failure on both arms (untouched, or beside an edit to a clean unit), a departure with a Native-Left override + reason, a byte-identical rename within its region, a cross-region move with a Native-Left override + reason, and the clean pair must pass; the env override outside a scratch arm must excuse nothing and WARN (FAIL under --strict); the default base resolving to HEAD must print 'NATIVE: base == tip, VACUOUS' as a WARN (FAIL under --strict), and so must a base pinned explicitly to HEAD (#1011); a base pinned to ANOTHER sha with HEAD's NATIVE inputs must print 'WARN NATIVE: no C change', counted nowhere, while a base whose C differs must stay a normal row (#1034), and so must a base differing ONLY in a tools path read 'no C change' (u'', V10); a pin on the branch's own line or off the tip's history must WARN 'WRONG BASE' (FAIL under --strict) while the fork point passes, and a 'no C change' row over a branch that touches a NATIVE input must WARN 'FALSE no C change' (FAIL under --strict) (#1065); a both-arms failure whose own source or a shared header the landing edits must WARN 'BLIND' naming it (FAIL under --strict), and an unreadable input must make it 'BLIND unverifiable' (#1365); the per-region counts must show a usa -1 / eu +1 move under an unchanged total, and sum to the totals on the real tree (task #1006); then the real tree against its real base"
  native_tree() {  # native_tree DIR — a fresh scratch copy of the check.sh inputs
    rm -rf "$1"; mkdir -p "$1/going-decompiled" "$1/tools"
    cp -R going-decompiled/src going-decompiled/include "$1/going-decompiled/"; cp -R tools/native "$1/tools/"
  }
  native_seed() { printf '\n/* t923 selftest seed */\nint t923_seeded_error = ;\n' >> "$1/going-decompiled/src/$2"; }
  native_arm() {  # native_arm NAME TIP BASE WANT_FAILED WANT_REGEX [MUST_NOT_REGEX]
    local out NATIVE_LEFT_FROM_ENV=1; out="$N/$(printf '%s' "$1" | tr -c 'A-Za-z0-9' _).txt"; NATIVE_ARM_OUT=$out; FAILED=0; check_native "$2" "$3" > "$out"
    if [ "$FAILED" = "$4" ] && /usr/bin/grep -qE "$5" "$out" && { [ -z "${6:-}" ] || ! /usr/bin/grep -qE "$6" "$out"; }; then
      ok "$1: $(/usr/bin/grep -E '^(OK|FAIL) ' "$out" | tr '\n' ' ')$(/usr/bin/grep -E '^     (failing|tip)' "$out" | sed 's/^ *//' | tr '\n' ' ')"
    else say "SELFTEST-FAIL native arm $1 (FAILED=$FAILED, want $4):"; show < "$out"; b=1; fi
  }
  native_tree "$N/base"
  # (a) synthetic: one C error appended to usa/cod/015180.c
  native_tree "$N/tip_a"; native_seed "$N/tip_a" usa/cod/015180.c
  native_arm "fired (a) seeded C error" "$N/tip_a" "$N/base" 1 '^FAIL NATIVE: 1 unit\(s\) .*: usa/cod/015180\.c $' 'eu/cod/015180'
  # (x) task #1327 (#1315 q2): the listing under a regress FAIL is check.sh's
  # `errors: N` line plus its sample of up to 3. (a)'s one-error seed cannot
  # tell a 2-line window from a 4-line one, so this seed carries 5 errors (it
  # fills the window; (y) below covers the short block): the listing must be the unit heading, then `errors: 5 (first 3
  # shown)`, then exactly 3 sample lines, each an `error:` in that unit.
  native_tree "$N/tip_x"
  printf '\n/* t1327 selftest seed */\nint t1327_e1 = ;\nint t1327_e2 = ;\nint t1327_e3 = ;\nint t1327_e4 = ;\nint t1327_e5 = ;\n' >> "$N/tip_x/going-decompiled/src/usa/cod/015180.c"
  native_arm "fired (x) 5-error seed" "$N/tip_x" "$N/base" 1 '^FAIL NATIVE: 1 unit\(s\) .*: usa/cod/015180\.c $'
  local xl xn xs; xl=$(awk '/^       usa\/cod\/015180\.c:$/ { f = 1; next } f && /^         / { print; next } f { exit }' "$NATIVE_ARM_OUT")
  xn=$(printf '%s\n' "$xl" | /usr/bin/grep -c . || true)
  xs=$(printf '%s\n' "$xl" | sed -n '2,4p' | /usr/bin/grep -cE '^         going-decompiled/src/usa/cod/015180\.c:[0-9]+:[0-9]+: error: ' || true)
  if [ "$(printf '%s\n' "$xl" | sed -n 1p)" = '         errors: 5 (first 3 shown)' ] && [ "$xn" = 4 ] && [ "$xs" = 3 ]; then ok "  ... and its listing is the count line + 3 samples: $(printf '%s\n' "$xl" | sed -n 1p | sed 's/^ *//'); $xs sample error lines"
  else say "SELFTEST-FAIL (x) the regress listing under the FAIL is not 'errors: 5 (first 3 shown)' + 3 sample error lines ($xn listing line(s), $xs sample(s)):"; show < "$NATIVE_ARM_OUT"; b=1; fi
  # (y) task #1361: (x) is blind to the listing BLEED — with 5 errors its
  # sample fills the window. A unit with FEWER than 3 errors left room in a
  # fixed -A4 window for whatever check.sh printed next: the next unit's
  # `FAIL:` heading and count (#1346, eu/cod/015180.c then 1907F0.cpp), or,
  # for the last failing unit, the `--- native compile-check` summary. Seed ONE
  # error in each of usa/ and eu/cod/015180.c, so one block is followed by the
  # other's FAIL heading and the later one by the summary (both shapes, checked
  # in the tip's check.sh log); each listing must be exactly the count line +
  # 1 sample of that unit.
  native_tree "$N/tip_y"; native_seed "$N/tip_y" usa/cod/015180.c; native_seed "$N/tip_y" eu/cod/015180.c
  native_arm "fired (y) 1-error seeds in two units" "$N/tip_y" "$N/base" 1 '^FAIL NATIVE: 2 unit\(s\) .*: eu/cod/015180\.c usa/cod/015180\.c $'
  local yu yl yn ys yshape; yshape=$(awk 'h && NR == h + 3 { print (/^FAIL: / ? "heading" : /^--- native compile-check: / ? "summary" : "other"); h = 0 } /^FAIL: going-decompiled\/src\/(usa|eu)\/cod\/015180\.c$/ { h = NR }' "$OUT/native_tip.txt.log" | LC_ALL=C sort | tr '\n' ' ')
  if [ "$yshape" = 'heading summary ' ]; then ok "  ... and the tip log has both bleed shapes: one seeded block is followed by another unit's FAIL heading, the other by the summary"
  else say "SELFTEST-BROKEN (y) the tip log does not set up both bleed shapes (lines after the seeded blocks: '$yshape', want 'heading summary ')"; b=1; fi
  for yu in eu usa; do
    yl=$(awk -v h="       $yu/cod/015180.c:" '$0 == h { f = 1; next } f && /^         / { print; next } f { exit }' "$NATIVE_ARM_OUT")
    yn=$(printf '%s\n' "$yl" | /usr/bin/grep -c . || true)
    ys=$(printf '%s\n' "$yl" | sed -n 2p | /usr/bin/grep -cE "^         going-decompiled/src/$yu/cod/015180\\.c:[0-9]+:[0-9]+: error: " || true)
    if [ "$(printf '%s\n' "$yl" | sed -n 1p)" = '         errors: 1 (first 3 shown)' ] && [ "$yn" = 2 ] && [ "$ys" = 1 ]; then ok "  ... and $yu/cod/015180.c's listing stops at its own block: errors: 1 (first 3 shown) + 1 sample"
    else say "SELFTEST-FAIL (y) $yu/cod/015180.c's regress listing bleeds past its own block ($yn line(s), want 2: 'errors: 1 (first 3 shown)' + 1 sample of that unit):"; printf '%s\n' "$yl" | show; b=1; fi
  done
  # (b) the dangerous class: A2's guard removed around the 198B58 equate
  native_tree "$N/tip_b"; local G="$N/tip_b/going-decompiled/src/eu/text/198B58.c"
  local E="going-decompiled/src/eu/text/198B58.c" L
  L=$(/usr/bin/grep -n '^__asm__("g_savePromptLatch = ' "$E" | head -1 | cut -d: -f1)
  if [ -n "$L" ] && [ "$(sed -n "$((L-1))p" "$E")" = '#ifndef TARGET_NATIVE' ] && [ "$(sed -n "$((L+1))p" "$E")" = '#endif' ]; then
    sed "$((L-1))d; $((L+1))d" "$E" > "$G"
  fi
  if /usr/bin/grep -q '^__asm__("g_savePromptLatch = ' "$G" && [ "$(( $(wc -l < "$E") - $(wc -l < "$G") ))" = 2 ]; then
    native_arm "fired (b) A2 guard removed from eu/198B58" "$N/tip_b" "$N/base" 1 '^FAIL NATIVE: 1 unit\(s\) .*: eu/text/198B58\.c $'
    /usr/bin/grep -q 'expected relocatable expression' "$NATIVE_ARM_OUT" && ok "  ... and its error is A2's: expected relocatable expression" || { say "SELFTEST-FAIL (b) fired for a different error"; b=1; }
  else say "SELFTEST-BROKEN: the 198B58 guard was not removed (2 lines) — has A2's guard moved?"; b=1; fi
  # (c) basename collision: usa/cod/015180 fails at the base, eu/cod/015180 at the tip
  native_tree "$N/base_c"; native_seed "$N/base_c" usa/cod/015180.c
  native_tree "$N/tip_c"; native_seed "$N/tip_c" eu/cod/015180.c
  native_arm "fired (c) usa-vs-eu collision" "$N/tip_c" "$N/base_c" 1 '^FAIL NATIVE: 1 unit\(s\) .*: eu/cod/015180\.c $' '^     failing on BOTH'
  /usr/bin/grep -q '^     failing at the base only .*: usa/cod/015180\.c $' "$NATIVE_ARM_OUT" && ok "  ... and usa/cod/015180.c is reported fixed at the tip, not conflated" || { say "SELFTEST-FAIL (c) did not report usa/cod/015180.c as base-only"; b=1; }
  # (d) a NEW unit that fails: absent at the base is not a pass at the base
  native_tree "$N/tip_d"; printf '#ifdef TARGET_NATIVE\nint t923_new_unit = ;\n#endif\n' > "$N/tip_d/going-decompiled/src/usa/t923_new_unit.c"
  native_arm "fired (d) new failing unit" "$N/tip_d" "$N/base" 1 '^FAIL NATIVE: 1 unit\(s\) .*: usa/t923_new_unit\.c $'
  # (e) base-relative: the same unit failing on both arms is tolerated
  native_arm "control (e) failing on both arms" "$N/base_c" "$N/base_c" 0 '^     failing on BOTH arms .*: usa/cod/015180\.c $'
  # (z1)-(z4) task #1365 BLIND: (e)'s both-arms failure is tolerated because
  # the landing did not touch it. Touched — its own source edited (z1), or a
  # shared header every unit reads edited (z2) — the comparison cannot see the
  # change, so the row must WARN BLIND naming it (FAIL under --strict). A
  # landing touching only a CLEAN unit beside it must stay silent (z3), and a
  # comparison that cannot run must fail closed (z4: an unreadable base file).
  local zt zst zout zre zwant
  native_tree "$N/tip_z1"; native_seed "$N/tip_z1" usa/cod/015180.c; printf '\n/* t1365 selftest: an edit inside a both-arms failure */\n' >> "$N/tip_z1/going-decompiled/src/usa/cod/015180.c"
  native_tree "$N/tip_z2"; native_seed "$N/tip_z2" usa/cod/015180.c; printf '\n/* t1365 selftest: a shared header edit */\n' >> "$N/tip_z2/going-decompiled/include/common.h"
  native_tree "$N/tip_z3"; native_seed "$N/tip_z3" usa/cod/015180.c; printf '\n/* t1365 selftest: an edit to a clean unit */\n' >> "$N/tip_z3/going-decompiled/src/eu/cod/015180.c"
  native_tree "$N/base_z4"; native_seed "$N/base_z4" usa/cod/015180.c; : > "$N/base_z4/going-decompiled/include/t1365_unreadable.h"; chmod 000 "$N/base_z4/going-decompiled/include/t1365_unreadable.h"
  for zst in 0 1; do
    for zt in z1 z2 z3 z4; do
      zout="$N/blind_${zt}_$zst.txt"; zre=$([ $zst = 1 ] && echo 'FAIL\(strict\)' || echo WARN)
      case $zt in
        z1) zwant="^$zre NATIVE: BLIND — 1 unit\\(s\\) .*: usa/cod/015180\\.c \\(its own source differs base->tip\\) — " ;;
        z2) zwant="^$zre NATIVE: BLIND — 1 unit\\(s\\) .*: usa/cod/015180\\.c \\(a shared NATIVE input differs\\) — " ;;
        z3) zwant='^     BLIND: none — ' ;;
        z4) zwant="^$zre NATIVE: BLIND unverifiable — .*could not checksum the base's shared NATIVE inputs.*each is treated as BLIND: usa/cod/015180\\.c \\(task #1365\\)" ;;
      esac
      if [ $zt = z4 ] && [ -r "$N/base_z4/going-decompiled/include/t1365_unreadable.h" ]; then say "SELFTEST-BROKEN (z4) the chmod-000 seed is still readable (running as root?) — the fail-closed arm cannot be exercised"; b=1; continue; fi
      FAILED=0; WARNED=0; STRICT=$zst; NATIVE_LEFT_FROM_ENV=1 check_native "$N/$([ $zt = z4 ] && echo base_c || echo "tip_$zt")" "$N/$([ $zt = z4 ] && echo base_z4 || echo base_c)" > "$zout"; STRICT=0
      if [ $zt = z3 ]; then
        if [ "$FAILED" = 0 ] && [ "$WARNED" = 0 ] && /usr/bin/grep -qE "$zwant" "$zout" && /usr/bin/grep -qE '^     failing on BOTH arms .*: usa/cod/015180\.c $' "$zout" && ! /usr/bin/grep -q 'NATIVE: BLIND' "$zout"; then
          ok "control (z3) STRICT=$zst: a landing touching only clean eu/cod/015180.c beside a both-arms failure — tolerated, 'BLIND: none', FAILED=0 WARNED=0"
        else say "SELFTEST-FAIL (z3) STRICT=$zst: touching only a clean unit changed the verdict or fired BLIND (FAILED=$FAILED WARNED=$WARNED):"; show < "$zout"; b=1; fi
      elif [ "$FAILED" = "$zst" ] && [ "$WARNED" = 1 ] && /usr/bin/grep -qE "$zwant" "$zout" \
           && { [ $zt = z4 ] || /usr/bin/grep -qE '^     failing on BOTH arms .*: usa/cod/015180\.c $' "$zout"; } \
           && { [ $zt != z2 ] || /usr/bin/grep -qE '^     shared NATIVE input\(s\) differing base->tip .*: going-decompiled/include/common\.h $' "$zout"; }; then
        ok "fired ($zt) STRICT=$zst: $(/usr/bin/grep -E "^$zre NATIVE: BLIND" "$zout" | cut -c1-200)... (FAILED=$FAILED WARNED=$WARNED)"
      else say "SELFTEST-FAIL ($zt) STRICT=$zst: the BLIND line did not fire as required (FAILED=$FAILED WARNED=$WARNED, want $zst/1):"; show < "$zout"; b=1; fi
    done
  done
  chmod 644 "$N/base_z4/going-decompiled/include/t1365_unreadable.h"
  # (h)-(k) population shrink (FACT #8359, #945): the first usa unit of the
  # population, deleted at the tip or with its TARGET_NATIVE token removed
  local lu; lu=$(cd going-decompiled/src && /usr/bin/grep -rl TARGET_NATIVE usa | LC_ALL=C sort | head -1)
  if [ -z "$lu" ]; then say "SELFTEST-BROKEN: no usa TARGET_NATIVE unit to remove for the shrink arms"; b=1; else
    local lre; lre=$(printf '%s' "$lu" | sed 's/[.]/\\./g')
    native_tree "$N/tip_h"; rm "$N/tip_h/going-decompiled/src/$lu"
    native_arm "fired (h) $lu deleted, no override" "$N/tip_h" "$N/base" 1 "^FAIL NATIVE: 1 unit\\(s\\) LEFT the TARGET_NATIVE population .*: $lre — "
    LANDING_GATE_NATIVE_LEFT="Native-Left: $lu t945 selftest: carved away" \
      native_arm "control (i) $lu deleted, Native-Left names it + reason" "$N/tip_h" "$N/base" 0 "^     left the population, overridden: $lre — t945 selftest: carved away \\[env\\]\$"
    LANDING_GATE_NATIVE_LEFT="Native-Left: $lu" \
      native_arm "fired (j) $lu deleted, Native-Left names it with NO reason" "$N/tip_h" "$N/base" 1 "^FAIL NATIVE: 1 unit\\(s\\) LEFT .*: $lre — " '^     left the population, overridden'
    native_tree "$N/tip_k"; sed 's/TARGET_NATIVE/T945_NO_NATIVE/g' "going-decompiled/src/$lu" > "$N/tip_k/going-decompiled/src/$lu"
    native_arm "fired (k) $lu's TARGET_NATIVE token removed" "$N/tip_k" "$N/base" 1 "^FAIL NATIVE: 1 unit\\(s\\) LEFT .*: $lre — "
    # (l)-(n) task #963: a byte-identical rename is paired and passes; the
    # same rename with one line appended is a departure; and the env override
    # OUTSIDE a scratch arm excuses nothing and is itself a WARN / strict FAIL
    local mu="${lu%.c}_t963_moved.c"; local mre; mre=$(printf '%s' "$mu" | sed 's/[.]/\\./g')
    native_tree "$N/tip_l"; mv "$N/tip_l/going-decompiled/src/$lu" "$N/tip_l/going-decompiled/src/$mu"
    native_arm "control (l) $lu renamed byte-identical" "$N/tip_l" "$N/base" 0 "^     renamed, byte-identical \\(paired, not a departure\\): $lre -> $mre " '^FAIL'
    native_tree "$N/tip_m"; mv "$N/tip_m/going-decompiled/src/$lu" "$N/tip_m/going-decompiled/src/$mu"; printf '\n/* t963 selftest edit */\n' >> "$N/tip_m/going-decompiled/src/$mu"
    native_arm "fired (m) $lu renamed with an edit" "$N/tip_m" "$N/base" 1 "^FAIL NATIVE: 1 unit\\(s\\) LEFT .*: $lre — " '^     renamed, byte-identical'
    # (o)-(p) task #984: the same byte-identical move into ANOTHER region is
    # not paired — the unit left usa's population — so it FAILS without a
    # trailer and passes with one; (l) above is the in-region case, unchanged
    local xu="eu/${lu#usa/}"; xu="${xu%.c}_t984_from_usa.c"; local xre; xre=$(printf '%s' "$xu" | sed 's/[.]/\\./g')
    native_tree "$N/tip_o"; mv "$N/tip_o/going-decompiled/src/$lu" "$N/tip_o/going-decompiled/src/$xu"
    native_arm "fired (o) $lu moved byte-identical to $xu" "$N/tip_o" "$N/base" 1 "^     byte-identical in ANOTHER region \\(not paired .*: $lre -> $xre " '^     renamed, byte-identical'
    /usr/bin/grep -qE "^FAIL NATIVE: 1 unit\\(s\\) LEFT .*: $lre — " "$NATIVE_ARM_OUT" && ok "  ... and the FAIL names $lu as having LEFT" || { say "SELFTEST-FAIL (o) did not name $lu as LEFT"; b=1; }
    LANDING_GATE_NATIVE_LEFT="Native-Left: $lu t984 selftest: moved to eu" \
      native_arm "control (p) $lu moved to $xu, Native-Left names it + reason" "$N/tip_o" "$N/base" 0 "^     left the population, overridden: $lre — t984 selftest: moved to eu \\[env\\]\$" '^FAIL'
    # (s) task #1006, FACT ledger-28603: (p) is the masking case — usa -1, eu
    # +1, an excused PASS, and the total identical on both arms. The per-region
    # line must show both moves while the total reads unchanged.
    local ub ut eb et totb tott; ub=$(native_region_units "$NATIVE_ARM_OUT" base usa); ut=$(native_region_units "$NATIVE_ARM_OUT" tip usa)
    eb=$(native_region_units "$NATIVE_ARM_OUT" base eu); et=$(native_region_units "$NATIVE_ARM_OUT" tip eu)
    totb=$(native_total_units "$NATIVE_ARM_OUT" base); tott=$(native_total_units "$NATIVE_ARM_OUT" tip)
    if [ -n "$ub" ] && [ -n "$ut" ] && [ -n "$eb" ] && [ -n "$et" ] && [ -n "$totb" ] && [ "$totb" = "$tott" ] && [ "$ut" = $((ub-1)) ] && [ "$et" = $((eb+1)) ]; then
      ok "fired (s) the masking case is visible: total $totb -> $tott units (unchanged) while usa $ub -> $ut and eu $eb -> $et"
    else say "SELFTEST-FAIL (s) the per-region line does not show (p)'s usa -1 / eu +1 under an unchanged total (usa '$ub' -> '$ut', eu '$eb' -> '$et', total '$totb' -> '$tott'):"; show < "$NATIVE_ARM_OUT"; b=1; fi
    local st
    for st in 1 0; do
      FAILED=0; WARNED=0; STRICT=$st; LANDING_GATE_NATIVE_LEFT="Native-Left: $lu t963 selftest: env excuse" check_native "$N/tip_h" "$N/base" > "$N/env_real_$st.txt"; STRICT=0
      if [ "$FAILED" = $((1+st)) ] && [ "$WARNED" = 1 ] && /usr/bin/grep -qE "^$([ $st = 1 ] && echo 'FAIL\(strict\)' || echo WARN) NATIVE: \\\$LANDING_GATE_NATIVE_LEFT is set and IGNORED" "$N/env_real_$st.txt" \
         && /usr/bin/grep -qE "^FAIL NATIVE: 1 unit\\(s\\) LEFT .*: $lre — " "$N/env_real_$st.txt" && ! /usr/bin/grep -q 'overridden' "$N/env_real_$st.txt"; then
        ok "fired (n) env override outside a scratch arm, STRICT=$st: excuses nothing, $(/usr/bin/grep -E '^(WARN|FAIL\(strict\)) NATIVE' "$N/env_real_$st.txt" | cut -c1-80)... (FAILED=$FAILED)"
      else say "SELFTEST-FAIL (n) env override outside a scratch arm, STRICT=$st (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/env_real_$st.txt"; b=1; fi
    done
  fi
  # (q)-(r) task #992: the DEFAULT base resolving to HEAD, reached through the
  # real resolution (merge-base(HEAD, HEAD) stands in for origin/master having
  # reached the tip) -> `NATIVE: base == tip, VACUOUS`, a WARN / strict FAIL;
  # the same base pinned explicitly is reported and not counted. With NATIVE
  # inputs dirty the comparison is not a self-comparison: the arm then requires
  # the uncommitted-paths note and no VACUOUS line instead.
  local vst vnd; vnd=$(git status --porcelain --no-renames -- going-decompiled/src going-decompiled/include tools/native | wc -l | tr -d ' ')
  for vst in 1 0; do
    FAILED=0; WARNED=0; STRICT=$vst; LANDING_GATE_NATIVE_LEFT= LANDING_GATE_NATIVE_BASE= check_native "" "" HEAD > "$N/vacuous_$vst.txt"; STRICT=0
    if [ "$vnd" = 0 ]; then
      if [ "$FAILED" = "$vst" ] && [ "$WARNED" = 1 ] && /usr/bin/grep -qE "^$([ $vst = 1 ] && echo 'FAIL\(strict\)' || echo WARN) NATIVE: base == tip, VACUOUS — the default base merge-base\(HEAD, HEAD\) is HEAD $(git rev-parse HEAD) itself" "$N/vacuous_$vst.txt" \
         && [ "$(/usr/bin/grep -c 'VACUOUS: base == tip, this measured nothing' "$N/vacuous_$vst.txt" || true)" = 2 ]; then
        ok "fired (q) default base == HEAD, STRICT=$vst: $(/usr/bin/grep -E '^(WARN|FAIL\(strict\)) NATIVE' "$N/vacuous_$vst.txt" | cut -c1-60)... (FAILED=$FAILED)"
      else say "SELFTEST-FAIL (q) default base == HEAD, STRICT=$vst (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/vacuous_$vst.txt"; b=1; fi
    else
      if [ "$FAILED" = 0 ] && [ "$WARNED" = 0 ] && /usr/bin/grep -q "^     base == HEAD ([0-9a-f]*): the row compares only the $vnd uncommitted path" "$N/vacuous_$vst.txt" && ! /usr/bin/grep -q VACUOUS "$N/vacuous_$vst.txt"; then
        ok "control (q') default base == HEAD with $vnd dirty NATIVE input(s), STRICT=$vst: not a self-comparison, noted, not counted"
      else say "SELFTEST-FAIL (q') default base == HEAD with $vnd dirty NATIVE input(s), STRICT=$vst (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/vacuous_$vst.txt"; b=1; fi
    fi
  done
  # (r) task #1011: a pin EQUAL to HEAD behaves exactly like the unpinned case
  # (this arm used to require FAILED=0 there — it certified the bug).
  if [ "$vnd" = 0 ]; then
    for vst in 1 0; do
      FAILED=0; WARNED=0; STRICT=$vst; LANDING_GATE_NATIVE_LEFT= LANDING_GATE_NATIVE_BASE=HEAD check_native > "$N/vacuous_pinned_$vst.txt"; STRICT=0
      if [ "$FAILED" = "$vst" ] && [ "$WARNED" = 1 ] && /usr/bin/grep -qE "^$([ $vst = 1 ] && echo 'FAIL\(strict\)' || echo WARN) NATIVE: base == tip, VACUOUS — LANDING_GATE_NATIVE_BASE=HEAD pins the base to HEAD $(git rev-parse HEAD) itself" "$N/vacuous_pinned_$vst.txt" \
         && [ "$(/usr/bin/grep -c 'VACUOUS: base == tip, this measured nothing' "$N/vacuous_pinned_$vst.txt" || true)" = 2 ]; then
        ok "fired (r) base pinned explicitly to HEAD, STRICT=$vst: $(/usr/bin/grep -E '^(WARN|FAIL\(strict\)) NATIVE' "$N/vacuous_pinned_$vst.txt" | cut -c1-60)... (FAILED=$FAILED)"
      else say "SELFTEST-FAIL (r) base pinned explicitly to HEAD, STRICT=$vst (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/vacuous_pinned_$vst.txt"; b=1; fi
    done
    # (u)-(w): every probe is a CHAIN of commit objects — no ref, no index, no
    # working-tree edit — `P` with HEAD as parent and one path replaced, then a
    # tip `T` with P as parent and HEAD's own tree, set as NATIVE_TIP_REV. The
    # working tree the tip arm compiles is therefore T's content, and the pin
    # is a genuine ancestor of the tip, which task #1065's ancestry rule needs
    # (a probe hanging off HEAD is not an ancestor of HEAD and would fire it).
    # (u) task #1034, ruling Q1: a pin to ANOTHER sha with HEAD's tree (#1022's
    # probe) is the same self-comparison -> `WARN NATIVE: no C change`, counted
    # nowhere, so it passes --strict; (u') the anti-overreach control: a pin
    # whose C differs by one appended comment line is a normal row, no such line.
    # (u'') task #1065, V10 / FACT ledger-28835: a pin differing from the tip
    # ONLY in a tools path (tools/ee/landing_gate.sh) must ALSO read `no C
    # change` — the predicate compares NATIVE-INPUT trees. A whole-tree compare
    # (mutant V10) sees the tools difference, prints no WARN, and fails here;
    # (u) cannot tell the two apart because its probe is whole-tree identical.
    local syn synt cre cret too toot cblob tblob
    cblob=$({ git cat-file blob "HEAD:going-decompiled/src/usa/cod/015180.c"; printf '\n/* t1034 selftest (u) */\n'; } | git hash-object -w --stdin)
    tblob=$({ git cat-file blob "HEAD:$HERE/landing_gate.sh"; printf '\n# t1065 selftest (u2) tools-only probe\n'; } | git hash-object -w --stdin)
    syn=$(native_probe_commit HEAD "$N/probe_index"); synt=$(native_probe_commit "$syn" "$N/probe_index")
    cre=$(native_probe_commit HEAD "$N/probe_index" going-decompiled/src/usa/cod/015180.c "$cblob"); cret=$(native_probe_commit "$cre" "$N/probe_index")
    too=$(native_probe_commit HEAD "$N/probe_index" "$HERE/landing_gate.sh" "$tblob"); toot=$(native_probe_commit "$too" "$N/probe_index")
    if [ -z "$syn" ] || [ -z "$synt" ] || [ -z "$cre" ] || [ -z "$cret" ] || [ -z "$too" ] || [ -z "$toot" ] \
       || [ "$(git rev-parse "$syn^{tree}")" != "$(git rev-parse 'HEAD^{tree}')" ] || git diff --quiet "$cre" HEAD -- going-decompiled/src \
       || git diff --quiet "$too" HEAD || ! git diff --quiet "$too" HEAD -- going-decompiled/src going-decompiled/include tools/native; then
      say "SELFTEST-BROKEN: (u) could not build its probe chains (syn '$syn', C-change '$cre', tools-only '$too')"; b=1
    else
      for vst in 1 0; do
        FAILED=0; WARNED=0; STRICT=$vst; NATIVE_TIP_REV=$synt LANDING_GATE_NATIVE_LEFT= LANDING_GATE_NATIVE_BASE=$syn check_native "" "" "$syn" > "$N/samein_$vst.txt"; STRICT=0
        if [ "$FAILED" = 0 ] && [ "$WARNED" = 0 ] && /usr/bin/grep -q "^WARN NATIVE: no C change — the base $syn and HEAD have identical NATIVE inputs" "$N/samein_$vst.txt" \
           && [ "$(/usr/bin/grep -c 'NO C CHANGE: base and tip NATIVE inputs identical' "$N/samein_$vst.txt" || true)" = 2 ] && ! /usr/bin/grep -qE 'VACUOUS|WRONG BASE|FALSE' "$N/samein_$vst.txt"; then
          ok "fired (u) base pinned to $(git rev-parse --short "$syn") (another sha, HEAD's tree), STRICT=$vst: 'WARN NATIVE: no C change', not counted (FAILED=0 WARNED=0)"
        else say "SELFTEST-FAIL (u) base pinned to a tree-identical sha, STRICT=$vst (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/samein_$vst.txt"; b=1; fi
      done
      FAILED=0; WARNED=0; STRICT=1; NATIVE_TIP_REV=$cret LANDING_GATE_NATIVE_LEFT= LANDING_GATE_NATIVE_BASE=$cre check_native "" "" "$cre" > "$N/cchange.txt"; STRICT=0
      if [ "$FAILED" = 0 ] && [ "$WARNED" = 0 ] && /usr/bin/grep -q "^     tip pass=[0-9]* fail=0; base $cre pass=[1-9]" "$N/cchange.txt" && ! /usr/bin/grep -qE 'no C change|NO C CHANGE|VACUOUS|WRONG BASE' "$N/cchange.txt"; then
        ok "control (u') base pinned to $(git rev-parse --short "$cre") (usa/cod/015180.c differs by one comment line), STRICT=1: a normal row, no 'no C change' line: $(/usr/bin/grep -E '^     tip ' "$N/cchange.txt" | sed 's/^ *//')"
      else say "SELFTEST-FAIL (u') a base whose C differs was not a normal row (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/cchange.txt"; b=1; fi
      FAILED=0; WARNED=0; STRICT=1; NATIVE_TIP_REV=$toot LANDING_GATE_NATIVE_LEFT= LANDING_GATE_NATIVE_BASE=$too check_native "" "" "$too" > "$N/toolsonly.txt"; STRICT=0
      if [ "$FAILED" = 0 ] && [ "$WARNED" = 0 ] && /usr/bin/grep -q "^WARN NATIVE: no C change — the base $too and HEAD have identical NATIVE inputs" "$N/toolsonly.txt" \
         && [ "$(/usr/bin/grep -c 'NO C CHANGE: base and tip NATIVE inputs identical' "$N/toolsonly.txt" || true)" = 2 ] && ! /usr/bin/grep -qE 'VACUOUS|WRONG BASE|FALSE' "$N/toolsonly.txt"; then
        ok "fired (u'') base pinned to $(git rev-parse --short "$too") (differs from the tip ONLY in $HERE/landing_gate.sh), STRICT=1: 'WARN NATIVE: no C change' — the predicate compares NATIVE inputs, not the whole tree (V10, FACT ledger-28835)"
      else say "SELFTEST-FAIL (u'') a base differing only in a tools path did not read 'no C change' — the predicate is comparing more than the NATIVE inputs (V10, FACT ledger-28835) (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/toolsonly.txt"; b=1; fi
      # (v) task #1065 item 3: the pin must be an ancestor of HEAD AND at or
      # below merge-base(HEAD, upstream). Tip = toot, upstream = HEAD, so the
      # fork point is HEAD. (v) a pin ON the branch's own line (too: an
      # ancestor of the tip, above the fork point) and (v') a pin that is no
      # ancestor at all (syn, off HEAD) must each WARN / FAIL under --strict,
      # naming WRONG BASE; (v'') HEAD itself — the fork point — passes. Every
      # probe here is tools-only against the tip, so (w)'s rule cannot fire.
      for vst in 1 0; do
        FAILED=0; WARNED=0; STRICT=$vst; NATIVE_TIP_REV=$toot LANDING_GATE_NATIVE_LEFT= LANDING_GATE_NATIVE_BASE=$too check_native "" "" HEAD > "$N/wrongbase_$vst.txt"; STRICT=0
        if [ "$FAILED" = "$vst" ] && [ "$WARNED" = 1 ] && /usr/bin/grep -qE "^$([ $vst = 1 ] && echo 'FAIL\(strict\)' || echo WARN) NATIVE: WRONG BASE — the pinned base $too is an ancestor of HEAD but NOT ancestor-or-equal of merge-base\(HEAD, HEAD\) $(git rev-parse HEAD)" "$N/wrongbase_$vst.txt"; then
          ok "fired (v) base pinned on the branch's own line ($(git rev-parse --short "$too"), above the fork point), STRICT=$vst: WRONG BASE (FAILED=$FAILED WARNED=$WARNED)"
        else say "SELFTEST-FAIL (v) a pin above the fork point was accepted, STRICT=$vst (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/wrongbase_$vst.txt"; b=1; fi
      done
      FAILED=0; WARNED=0; STRICT=1; NATIVE_TIP_REV=$toot LANDING_GATE_NATIVE_LEFT= LANDING_GATE_NATIVE_BASE=$syn check_native "" "" HEAD > "$N/unrelated.txt"; STRICT=0
      if [ "$FAILED" = 1 ] && [ "$WARNED" = 1 ] && /usr/bin/grep -q "^FAIL(strict) NATIVE: WRONG BASE — the pinned base $syn is NOT an ancestor of HEAD $toot" "$N/unrelated.txt"; then
        ok "fired (v') base pinned to $(git rev-parse --short "$syn"), not an ancestor of the tip, STRICT=1: WRONG BASE (FAILED=$FAILED)"
      else say "SELFTEST-FAIL (v') a pin that is no ancestor of the tip was accepted (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/unrelated.txt"; b=1; fi
      FAILED=0; WARNED=0; STRICT=1; NATIVE_TIP_REV=$toot LANDING_GATE_NATIVE_LEFT= LANDING_GATE_NATIVE_BASE=HEAD check_native "" "" HEAD > "$N/rightbase.txt"; STRICT=0
      if [ "$FAILED" = 0 ] && [ "$WARNED" = 0 ] && /usr/bin/grep -q "^     pinned base $(git rev-parse HEAD): an ancestor of HEAD $toot and ancestor-or-equal of merge-base(HEAD, HEAD)" "$N/rightbase.txt" \
         && /usr/bin/grep -q "^     'no C change' confirmed: HEAD\.\.HEAD touches no NATIVE input" "$N/rightbase.txt" && ! /usr/bin/grep -qE 'WRONG BASE|FALSE' "$N/rightbase.txt"; then
        ok "control (v'') base pinned to the fork point HEAD, tools-only branch, STRICT=1: accepted, and its 'no C change' confirmed (FAILED=0 WARNED=0)"
      else say "SELFTEST-FAIL (v'') the fork point itself was rejected as a base, or its tools-only branch read as a C change (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/rightbase.txt"; b=1; fi
      # (w) task #1065 item 4: the branch HEAD -> cre (C comment appended) ->
      # cret (HEAD's tree again) nets to no C change, so the row honestly reads
      # `no C change` — but HEAD..tip TOUCHES usa/cod/015180.c twice, and the
      # claim is false of the landing. It must WARN / FAIL under --strict
      # naming the path and both commits. (v'') is its control: the same
      # shape with a tools-only branch is confirmed, not flagged.
      for vst in 1 0; do
        FAILED=0; WARNED=0; STRICT=$vst; NATIVE_TIP_REV=$cret LANDING_GATE_NATIVE_LEFT= LANDING_GATE_NATIVE_BASE=HEAD check_native "" "" HEAD > "$N/falsenoc_$vst.txt"; STRICT=0
        if [ "$FAILED" = "$vst" ] && [ "$WARNED" = 1 ] && /usr/bin/grep -q '^WARN NATIVE: no C change' "$N/falsenoc_$vst.txt" \
           && /usr/bin/grep -qE "^$([ $vst = 1 ] && echo 'FAIL\(strict\)' || echo WARN) NATIVE: FALSE 'no C change' — .* touches 2 NATIVE input path\(s\).*going-decompiled/src/usa/cod/015180\.c \[$(git rev-parse --short "$cret")\] ; going-decompiled/src/usa/cod/015180\.c \[$(git rev-parse --short "$cre")\]" "$N/falsenoc_$vst.txt" \
           && ! /usr/bin/grep -q 'WRONG BASE' "$N/falsenoc_$vst.txt"; then
          ok "fired (w) row 'no C change' while HEAD..tip touches usa/cod/015180.c, STRICT=$vst: FALSE 'no C change' naming both commits (FAILED=$FAILED WARNED=$WARNED)"
        else say "SELFTEST-FAIL (w) a 'no C change' row over a branch that touches a NATIVE input was not flagged, STRICT=$vst (FAILED=$FAILED WARNED=$WARNED):"; show < "$N/falsenoc_$vst.txt"; b=1; fi
      done
      NATIVE_PROBE_TOOLS="$too $toot"; NATIVE_PROBE_C="$cre $cret"
    fi
  fi
  # (f) clean pair; (g) the real tree against its real base
  native_arm "control (f) clean pair" "$N/base" "$N/base" 0 '^OK   NATIVE: no unit fails'
  FAILED=0; check_native > "$N/real.txt"
  # (t) task #1006: the per-region counts are a re-presentation, not a
  # re-measurement — on the real tree they sum to the old total on both arms;
  # and the sum check itself fires on a copy whose tip total is altered
  if native_region_sums_close "$N/real.txt"; then ok "control (t) real tree: per-region counts sum to the totals on both arms: $(sed -n 's/^     NATIVE per region (pass\/units): //p' "$N/real.txt")"
  else say "SELFTEST-FAIL (t) the real tree's per-region counts do not sum to its totals (or the line is missing):"; show < <(/usr/bin/grep -E '^     (tip|NATIVE per region)' "$N/real.txt"); b=1; fi
  sed 's#^\(     NATIVE per region (pass/units): tip [^=]*\) = \([0-9]*\)/#\1 = 9\2/#' "$N/real.txt" > "$N/real_sum_seeded.txt"
  if ! cmp -s "$N/real.txt" "$N/real_sum_seeded.txt" && ! native_region_sums_close "$N/real_sum_seeded.txt"; then ok "fired (t') the sum check rejects a tip total altered to $(sed -n 's/^     NATIVE per region.*: tip .* = \([0-9]*\/[0-9]*\);.*/\1/p' "$N/real_sum_seeded.txt")"
  else say "SELFTEST-FAIL (t') the sum check accepted an altered tip total (or the seed did not apply)"; b=1; fi
  if [ "$FAILED" = 0 ]; then ok "control (g) real tree: $(/usr/bin/grep -E '^     tip ' "$N/real.txt" | sed 's/^ *//')"; else say "SELFTEST-FAIL the real tree fails NATIVE:"; show < "$N/real.txt"; b=1; fi
  return $b
}

# native_probe_commit PARENT INDEXFILE [PATH BLOB] — a commit object (no ref,
# no real index, no working-tree edit) whose tree is HEAD's tree with PATH
# replaced by BLOB, and whose one parent is PARENT. Prints its sha.
native_probe_commit() {
  local t; rm -f "$2"
  GIT_INDEX_FILE="$2" git read-tree HEAD || return 1
  if [ -n "${3:-}" ]; then GIT_INDEX_FILE="$2" git update-index --cacheinfo "100644,$4,$3" || { rm -f "$2"; return 1; }; fi
  t=$(GIT_INDEX_FILE="$2" git write-tree); rm -f "$2"; [ -n "$t" ] || return 1
  git commit-tree "$t" -p "$1" -m 'landing_gate selftest probe (task #1034/#1065)'
}

# selftest_verdict_annotation OUTDIR — arm (21), run after selftest_native
# (it reuses (u')/(u'')'s probe chains, NATIVE_PROBE_C / NATIVE_PROBE_TOOLS).
# Task #1065 item 2: the REAL run_gate --strict, every row but NATIVE stubbed
# and NATIVE the real check_native pinned to a probe, must end its $tag
# verdict line in ` (native: no C change)` when the base differs from the tip
# only in a tools path, and must NOT when the base's C differs. Both
# directions: an annotation that always appears is as bad as one that never
# does. The inner tag is the selftest's, never a landing line.
NATIVE_PROBE_C=""; NATIVE_PROBE_TOOLS=""
selftest_verdict_annotation() {
  local T="$1" b=0 rc; local D="$T/annot"; rm -rf "$D"; mkdir -p "$D"
  say "-- (21) VERDICT ANNOTATION (#1065): the real run_gate --strict (rows other than NATIVE stubbed) -> a tools-only base ends the verdict line ' (native: no C change)'; a base whose C differs -> the bare verdict, no annotation"
  if [ -z "$NATIVE_PROBE_C" ] || [ -z "$NATIVE_PROBE_TOOLS" ]; then say "SELFTEST-BROKEN: (21) needs (u')/(u'')'s probe chains, which arm (18) did not build (NATIVE inputs dirty?)"; return 1; fi
  eval "$(declare -f check_native | sed '1s/^check_native/annot_real_check_native/')"
  annot_run() {  # annot_run OUTFILE BASE TIP — the real run_gate, NATIVE pinned to BASE, tip TIP, upstream BASE
    ( for f in check_dirty check_flags split_inputs check_shadow check_orphans check_libgcc check_gmodel check_dlisites do_build check_tree check_asmunit measure_row check_row check_noprovide; do
        eval "$f() { say \"     (arm 21 stub: $f)\"; }"
      done
      region_vars() { REGION=$1; OUT="$D/out"; mkdir -p "$OUT"; }
      check_native() { annot_real_check_native "" "" "$ANNOT_UP"; }
      FAILED=0; WARNED=0; DIRTY_FAILED=0; STRICT=0
      ANNOT_UP=$2 NATIVE_TIP_REV=$3 LANDING_GATE_NATIVE_LEFT= LANDING_GATE_NATIVE_BASE=$2 run_gate "$REGION" --no-build --strict ) > "$1" 2>&1
  }
  local tag="==== selftest inner gate \[$REGION\] (NOT a landing verdict): "
  set -- $NATIVE_PROBE_TOOLS; annot_run "$D/tools.txt" "$1" "$2"; rc=$?
  if [ "$rc" = 0 ] && /usr/bin/grep -qx "${tag}PASS (native: no C change)" "$D/tools.txt" && /usr/bin/grep -q '^WARN NATIVE: no C change' "$D/tools.txt"; then
    ok "fired (21) tools-only base $(git rev-parse --short "$1"): rc $rc, verdict '$(/usr/bin/grep "^$tag" "$D/tools.txt" | sed 's/.*verdict): //')'"
  else say "SELFTEST-FAIL (21) a tools-only base did not annotate the verdict line (rc $rc):"; show < <(/usr/bin/grep -E '^(OK|FAIL|WARN|====)' "$D/tools.txt" | sed 's/^/  inner| /'); b=1; fi
  set -- $NATIVE_PROBE_C; annot_run "$D/cchange.txt" "$1" "$2"; rc=$?
  if [ "$rc" = 0 ] && /usr/bin/grep -qx "${tag}PASS" "$D/cchange.txt" && ! /usr/bin/grep -qE 'no C change|NO C CHANGE' "$D/cchange.txt"; then
    ok "control (21) base $(git rev-parse --short "$1") whose C differs: rc $rc, verdict 'PASS' with no annotation"
  else say "SELFTEST-FAIL (21) a base whose C differs still annotated the verdict, or did not PASS (rc $rc):"; show < <(/usr/bin/grep -E '^(OK|FAIL|WARN|====)' "$D/cchange.txt" | sed 's/^/  inner| /'); b=1; fi
  # stub-list completeness (task #1142), as in (20): no row but NATIVE may
  # print a verdict line, or a row missing from the list ran unnoticed
  local unstubbed; unstubbed=$(cat "$D/tools.txt" "$D/cchange.txt" | /usr/bin/grep -E '^(OK|FAIL|WARN) ' | /usr/bin/grep -vE '^(OK|FAIL|WARN) +NATIVE' || true)
  if [ -z "$unstubbed" ]; then ok "stubs (21): no row but NATIVE printed a verdict line"
  else say "SELFTEST-FAIL (21) a row missing from the stub list ran for real — add it to BOTH stub lists, (20) and (21):"; show < <(printf '%s\n' "$unstubbed" | sed 's/^/  inner| /'); b=1; fi
  unset -f annot_real_check_native annot_run
  rm -rf "$D"; FAILED=0; WARNED=0; DIRTY_FAILED=0; STRICT=0; NATIVE_NO_C_CHANGE=0
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

# selftest_dlisites OUTDIR — arm (22), callable on its own after sourcing this
# file (`. tools/ee/landing_gate.sh; region_vars usa; selftest_dlisites /tmp/x`).
# The seed is #1105's func_002E5074@0x2E50E4 row: ROM-true, splat-true and
# simulation-true, so ONLY the Ps2EeAs arm can fail it. The host-only checker
# must pass the same copy — if it ever stops doing so, the seed no longer
# isolates the Ps2EeAs arm and the arm says so instead of passing.
#
# The seed's "fired" predicate requires the KNOWN emission, `PS2EEAS emits
# 3c080130` (task #1185): FACT #8645 saw the old predicate, which ended at
# `emits `, pass on an EMPTY emission — the seed's site had been lost from a
# stale sites.s, so the arm fired for a reason unrelated to the Ps2EeAs
# expansion it exists to show. #1142's "any hex word" still accepted
# `24080013`, which real Ps2EeAs emits for a site cut to `dli $8,0x13`
# (FACT #8683, #8691) — a shape check, not a value check. The predicate is
# known-answer checked on all three forms (empty, 3c080130, 24080013) before
# it is used.
#
# Two fault arms (task #1142) run the REAL check_dlisites and the REAL VM
# path with one fault injected by a wrapper around ps2eeas_dli_sites.run_vm
# (written to OUTDIR, never to the tree), and each must read could-not-run,
# neither OK nor a row FAIL:
#   truncate  the last site is cut from the source the container receives —
#             FACT #8645's stale-view shape. The md5 transport check must fire.
#   drop      the last site is removed from Ps2EeAs's objdump output. That
#             row must print `CNR`, not `FAIL ... PS2EEAS emits`.
# A third (task #1185) runs the SEEDED copy with its FIRST site dropped from
# the objdump output: the seed FAILs and a real row is CNR, so the FAIL
# headline must name the CNR row too — absence must not be hidden by another
# row's failure (FACT #8691).
# The wrapper exits 3 when its fault matched nothing, so a fault that stops
# applying reads SELFTEST-BROKEN rather than passing.
DLISITES_SEED_FIRED_RE='^FAIL DLISITES: 1 of [0-9]+ allowlist row\(s\) fail — .*: FAIL line [0-9]+: usa +func_002E5074 +0x002E50E4 .* \| PS2EEAS emits 3c080130( ;|$)'
selftest_dlisites() {
  local T="$1" b=0 rc
  local seed='usa      func_002E5074  0x002E50E4  $8,0x1300000             24080013 00084538'
  say "-- (22) DLISITES (#1116, #1142): the real allowlist must pass; a copy with #1105's ROM-true row func_002E5074@0x2E50E4 appended must FAIL naming ONLY that row with PS2EEAS emits 3c080130; the host-only checker must PASS that copy (the arm it isolates); a source cut in transit and a site absent from Ps2EeAs's output must each be could-not-run; that copy with a real site absent must FAIL naming the seed AND the CNR row"
  local kfail="FAIL DLISITES: 1 of 8 allowlist row(s) fail — asm_unit.sh would expand them as written: FAIL line 50: $seed | PS2EEAS emits "
  if printf '%s\n' "$kfail" | /usr/bin/grep -qE "$DLISITES_SEED_FIRED_RE"; then say "SELFTEST-BROKEN the fired predicate accepts an EMPTY 'PS2EEAS emits' (FACT #8645)"; b=1
  elif ! printf '%s\n' "${kfail}3c080130" | /usr/bin/grep -qE "$DLISITES_SEED_FIRED_RE"; then say "SELFTEST-BROKEN the fired predicate rejects the genuine 'PS2EEAS emits 3c080130'"; b=1
  elif printf '%s\n' "${kfail}24080013" | /usr/bin/grep -qE "$DLISITES_SEED_FIRED_RE"; then say "SELFTEST-BROKEN the fired predicate accepts a WRONG word, 'PS2EEAS emits 24080013' (FACT #8683, #8691)"; b=1
  else ok "predicate: rejects an empty 'PS2EEAS emits' and a wrong 'PS2EEAS emits 24080013', accepts 'PS2EEAS emits 3c080130'"; fi
  FAILED=0; check_dlisites > "$T/dlisites_real.txt"
  if [ "$FAILED" = 0 ] && /usr/bin/grep -q '^OK   DLISITES: all [1-9][0-9]* allowlist row(s)' "$T/dlisites_real.txt"; then ok "control: $(/usr/bin/grep '^OK   DLISITES' "$T/dlisites_real.txt" | sed 's/^OK   //')"; else say "SELFTEST-FAIL the real allowlist does not pass DLISITES:"; show < "$T/dlisites_real.txt"; b=1; fi
  { cat "$HERE/ps2eeas_dli_sites.txt"; printf '%s\n' "$seed"; } > "$T/dlisites_seed.txt"
  FAILED=0; check_dlisites "$T/dlisites_seed.txt" > "$T/dlisites_seeded.txt"
  if [ "$FAILED" = 1 ] && /usr/bin/grep -qE "$DLISITES_SEED_FIRED_RE" "$T/dlisites_seeded.txt"; then ok "fired: $(/usr/bin/grep '^FAIL DLISITES' "$T/dlisites_seeded.txt" | sed 's/ — asm_unit.sh would expand them as written//')"
  else say "SELFTEST-FAIL the seeded func_002E5074 row did not fail DLISITES alone with PS2EEAS emits 3c080130 (FAILED=$FAILED):"; show < "$T/dlisites_seeded.txt"; b=1; fi
  python3 "$HERE/ps2eeas_dli_sites.py" "$T/dlisites_seed.txt" > "$T/dlisites_hostonly.txt" 2>&1; rc=$?
  if [ "$rc" = 0 ] && /usr/bin/grep -qE '^ps2eeas_dli_sites: [0-9]+ rows, 0 failed \(Ps2EeAs NOT run\)$' "$T/dlisites_hostonly.txt"; then ok "isolation: the host-only checker passes the same seeded copy (rc 0, $(tail -1 "$T/dlisites_hostonly.txt" | sed 's/^ps2eeas_dli_sites: //')) — only the Ps2EeAs arm sees it"
  else say "SELFTEST-BROKEN the host-only checker no longer passes the func_002E5074 seed (rc $rc) — it no longer isolates the Ps2EeAs arm; choose a seed only Ps2EeAs rejects:"; show < "$T/dlisites_hostonly.txt"; b=1; fi
  cat > "$T/dlisites_fault.py" <<'PYEOF'
# landing_gate --selftest (22) fault wrapper: argv = checker path, then its args
import os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(sys.argv[1])))
import ps2eeas_dli_sites as m
mode, real = os.environ['DLISITES_FAULT'], m.run_vm
def run_vm(cmd):
    if mode == 'truncate':
        cut, k = re.subn(r'site_\d+:\n\tdli\t[^\n]*\n(PS2EEAS_DLI_EOF\n)', r'\1', cmd)
        if k != 1: sys.exit(3)
        return real(cut)
    out = real(cmd)
    last = re.findall(r'^[0-9a-f]+ <(site_\d+)>:$', out.stdout, re.M)
    if mode not in ('drop', 'dropfirst') or not last: sys.exit(3)
    victim = last[0] if mode == 'dropfirst' else last[-1]
    out.stdout = re.sub(r'^[0-9a-f]+ <%s>:\n(?:\s+[0-9a-f]+:.*\n?)*' % victim, '', out.stdout, flags=re.M)
    return out
m.run_vm = run_vm
sys.argv = sys.argv[1:]
m.main()
PYEOF
  local mode want
  for mode in truncate drop; do
    case $mode in
      truncate) want='^FAIL ps2eeas_dli_sites.py could not run.*\(rc 2, summary .none.\): ps2eeas_dli_sites: could not run: the container assembled a sites.s with md5 [0-9a-f]{32}, the host wrote one with md5 [0-9a-f]{32}' ;;
      drop)     want='^FAIL ps2eeas_dli_sites.py could not run.*\(rc 2, summary .ps2eeas_dli_sites: [0-9]+ rows, 0 failed, 1 could not run \(Ps2EeAs INCOMPLETE\).\): CNR  line [0-9]+: .* \| PS2EEAS site_[0-9]+ is absent from Ps2EeAs.s output: not checked$' ;;
    esac
    FAILED=0; ( python3() { DLISITES_FAULT=$mode command python3 "$T/dlisites_fault.py" "$@"; }; check_dlisites; exit "$FAILED" ) > "$T/dlisites_$mode.txt"; rc=$?
    if /usr/bin/grep -qE '\(rc 3,' "$T/dlisites_$mode.txt"; then say "SELFTEST-BROKEN (22) the $mode fault matched nothing — the arm no longer injects it:"; show < "$T/dlisites_$mode.txt"; b=1
    elif [ "$rc" = 1 ] && /usr/bin/grep -qE "$want" "$T/dlisites_$mode.txt" && ! /usr/bin/grep -qE '^(OK   DLISITES|FAIL DLISITES)' "$T/dlisites_$mode.txt"; then ok "fired ($mode): $(/usr/bin/grep '^FAIL ps2eeas_dli_sites.py' "$T/dlisites_$mode.txt" | cut -c1-260)"
    else say "SELFTEST-FAIL (22) the $mode fault did not read could-not-run (neither OK nor a row FAIL) (rc $rc):"; show < "$T/dlisites_$mode.txt"; b=1; fi
  done
  want='^FAIL DLISITES: 1 of [0-9]+ allowlist row\(s\) fail — .*: FAIL line [0-9]+: usa +func_002E5074 +0x002E50E4 .* \| PS2EEAS emits 3c080130 ; and 1 row\(s\) could not run: CNR  line [0-9]+: .* \| PS2EEAS site_[0-9]+ is absent from Ps2EeAs.s output: not checked$'
  FAILED=0; ( python3() { DLISITES_FAULT=dropfirst command python3 "$T/dlisites_fault.py" "$@"; }; check_dlisites "$T/dlisites_seed.txt"; exit "$FAILED" ) > "$T/dlisites_failcnr.txt"; rc=$?
  if /usr/bin/grep -qE '\(rc 3,' "$T/dlisites_failcnr.txt"; then say "SELFTEST-BROKEN (22) the dropfirst fault matched nothing — the arm no longer injects it:"; show < "$T/dlisites_failcnr.txt"; b=1
  elif [ "$rc" = 1 ] && /usr/bin/grep -qE "$want" "$T/dlisites_failcnr.txt"; then ok "fired (seed FAIL + CNR): $(/usr/bin/grep '^FAIL DLISITES' "$T/dlisites_failcnr.txt" | sed 's/ — asm_unit.sh would expand them as written//' | cut -c1-320)"
  else say "SELFTEST-FAIL (22) with the seed failing and a real site absent, the FAIL headline does not name the CNR row (rc $rc):"; show < "$T/dlisites_failcnr.txt"; b=1; fi
  FAILED=0
  return $b
}

# selftest_asmunit OUTDIR [BUILD_LOG] — arm (23). Seeds are copies of the
# selftest build's own log with one line appended, so the query form is the one
# the real row runs on: FACT #8610's WARNING line (the one it observed reach a
# gate build.log), a REFUSED line and an unknown `asm_unit.sh:` kind. The
# per-unit `asm_unit.sh: dli: N transforms (M allowlist rows for <region>)`
# line (task #1205) is a known kind in that spelling only: the real log must
# carry it, and a reworded or suffixed copy must WARN.
selftest_asmunit() {
  local T="$1" log=${2:-$OUT/build.log} b=0 s
  say "-- (23) ASMUNIT (#1116): this build's log must pass; copies with FACT #8610's WARNING line, or a REFUSED line, appended must FAIL naming it; an unknown asm_unit.sh: kind must WARN (FAIL under --strict); a missing log must FAIL"
  FAILED=0; WARNED=0; STRICT=0; check_asmunit "$log" > "$T/asmunit_real.txt"
  if [ "$FAILED" = 0 ] && [ "$WARNED" = 0 ]; then ok "control: $(/usr/bin/grep '^OK   ASMUNIT' "$T/asmunit_real.txt" | sed 's/^OK   //')"; else say "SELFTEST-FAIL this build's log does not pass ASMUNIT:"; show < "$T/asmunit_real.txt"; b=1; fi
  { cat "$log"; printf 'asm_unit.sh: WARNING: func_00352CE8: `sw\t$2,g_pHeroMoby` in the delay slot of `beql\t$4,$0,1f` NOT hoisted (branch-likely); emitted as lui $at before the branch + the %%lo access in the slot. The ROM has no such site: this function cannot match as written (#979, FACT #8385)\n'; } > "$T/asmunit_warn.log"
  { cat "$log"; printf 'asm_unit.sh: REFUSED: ps2eeas_dli.awk exited 1 on t1116.s; t1116.o removed (RULING #8549)\n'; } > "$T/asmunit_refused.log"
  { cat "$log"; printf 'asm_unit.sh: NOTICE: t1116 selftest, a kind the row does not know\n'; } > "$T/asmunit_unknown.log"
  for s in WARNING:func_00352CE8:warn REFUSED:ps2eeas_dli.awk:refused; do
    FAILED=0; WARNED=0; check_asmunit "$T/asmunit_${s##*:}.log" > "$T/asmunit_${s##*:}.txt"
    if [ "$FAILED" = 1 ] && [ "$WARNED" = 0 ] && /usr/bin/grep -qE "^FAIL ASMUNIT \[$REGION\]: 1 asm_unit.sh WARNING/REFUSED/FAIL line\(s\) in .*: [0-9]+:asm_unit\.sh: $(printf '%s' "$s" | cut -d: -f1): $(printf '%s' "$s" | cut -d: -f2)" "$T/asmunit_${s##*:}.txt"; then ok "fired: $(/usr/bin/grep '^FAIL ASMUNIT' "$T/asmunit_${s##*:}.txt" | sed -E 's/ in [^ ]*: / : /')"
    else say "SELFTEST-FAIL the appended $(printf '%s' "$s" | cut -d: -f1) line did not FAIL ASMUNIT naming it (FAILED=$FAILED WARNED=$WARNED):"; show < "$T/asmunit_${s##*:}.txt"; b=1; fi
  done
  FAILED=0; WARNED=0; STRICT=0; check_asmunit "$T/asmunit_unknown.log" > "$T/asmunit_unknown_warn.txt"
  if [ "$FAILED" = 0 ] && [ "$WARNED" = 1 ] && /usr/bin/grep -q '^WARN ASMUNIT .*: 1 asm_unit.sh: line(s) with none of WARNING/REFUSED/FAIL' "$T/asmunit_unknown_warn.txt"; then ok "fired: an unknown kind WARNs (FAILED=$FAILED WARNED=$WARNED)"; else say "SELFTEST-FAIL an unknown asm_unit.sh: kind did not WARN (FAILED=$FAILED WARNED=$WARNED):"; show < "$T/asmunit_unknown_warn.txt"; b=1; fi
  FAILED=0; WARNED=0; STRICT=1; check_asmunit "$T/asmunit_unknown.log" > "$T/asmunit_unknown_strict.txt"; STRICT=0
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q '^FAIL(strict) ASMUNIT' "$T/asmunit_unknown_strict.txt"; then ok "fired: under --strict the unknown kind is a FAIL"; else say "SELFTEST-FAIL --strict did not fail the unknown asm_unit.sh: kind (FAILED=$FAILED)"; b=1; fi
  # task #1205: the real log must carry asm_unit.sh's per-unit `dli:` lines in
  # the exact spelling (so the control above exercised the exclusion), and a
  # reworded or suffixed copy of one must still WARN: the row knows ONE spelling
  local nd; nd=$(/usr/bin/grep -cE "$ASMUNIT_DLI_RE" "$log" || true)
  if [ "$nd" -ge 1 ]; then ok "control: $nd asm_unit.sh dli: line(s) in the exact spelling, none counted as an unknown kind ($(/usr/bin/grep -E "$ASMUNIT_DLI_RE" "$log" | sort | uniq -c | sed -E 's/^ +//' | tr '\n' ';' | sed 's/;$//; s/;/ ; /g'))"
  else say "SELFTEST-FAIL (23) this build's log carries no 'asm_unit.sh: dli: N transforms (M allowlist rows for $REGION)' line: asm_unit.sh did not emit the interface GATE-F3 (#1158) reads"; b=1; fi
  for s in 'transform (9 allowlist rows for usa)' 'transforms (9 allowlist rows for usa) INERT'; do
    { cat "$log"; printf 'asm_unit.sh: dli: 9 %s\n' "$s"; } > "$T/asmunit_dliword.log"
    FAILED=0; WARNED=0; STRICT=0; check_asmunit "$T/asmunit_dliword.log" > "$T/asmunit_dliword.txt"
    if [ "$FAILED" = 0 ] && [ "$WARNED" = 1 ]; then ok "fired: 'asm_unit.sh: dli: 9 $s' is not the known spelling and WARNs"
    else say "SELFTEST-FAIL (23) 'asm_unit.sh: dli: 9 $s' was read as the known dli line (FAILED=$FAILED WARNED=$WARNED):"; show < "$T/asmunit_dliword.txt"; b=1; fi
  done
  FAILED=0; WARNED=0; check_asmunit "$T/asmunit_absent.log" > "$T/asmunit_absent.txt"
  if [ "$FAILED" = 1 ] && /usr/bin/grep -q '^FAIL ASMUNIT .*: no .*asmunit_absent.log' "$T/asmunit_absent.txt"; then ok "fired: a missing log is a FAIL"; else say "SELFTEST-FAIL a missing build.log did not FAIL ASMUNIT (FAILED=$FAILED)"; b=1; fi
  FAILED=0; WARNED=0
  return $b
}

# selftest_regression_gate OUTDIR — arm (24), task #1327. Runs #1316's
# tools/ee/eetest/regression_gate_selftest.sh, which nothing invoked (#1316
# unresolved 6). Host-only fixture repos: no VM, no emulator, ~2 min a run.
# Fired leg: the same selftest against 3eba641c4, regression_gate.sh before
# #1316, which advanced its baseline on an incomplete validation, must FAIL on
# exactly cap, baseline, nosnap and ilcap. Control: on this tree every case OK.
# 3eba641c4 is an ancestor of master, so the fired leg is reachable from any
# checkout of it.
selftest_regression_gate() {
  local T="$1" b=0 rs="$HERE/eetest/regression_gate_selftest.sh" pre=3eba641c4 rc fl nok
  say "-- (24) REGRESSION_GATE (#1316, #1327): $rs on $pre (regression_gate.sh before #1316) -> rc 1, FAIL on exactly cap baseline nosnap ilcap; on this tree -> rc 0, all 7 cases OK"
  bash "$rs" "$pre" > "$T/rgself_pre.txt" 2>&1; rc=$?
  fl=$(sed -n 's/^FAIL \([a-z]*\):.*/\1/p' "$T/rgself_pre.txt" | tr '\n' ' ')
  if [ "$rc" = 1 ] && [ "$fl" = 'cap baseline nosnap ilcap ' ]; then ok "fired: on $pre rc $rc, FAIL on $fl"
  else say "SELFTEST-FAIL (24) the selftest on $pre did not FAIL exactly cap baseline nosnap ilcap (rc $rc, FAIL on '$fl'):"; show < <(/usr/bin/grep -E '^(OK|FAIL|####)' "$T/rgself_pre.txt" | sed 's/^/  inner| /'); b=1; fi
  bash "$rs" > "$T/rgself_tree.txt" 2>&1; rc=$?
  nok=$(/usr/bin/grep -c '^OK ' "$T/rgself_tree.txt" || true); fl=$(sed -n 's/^FAIL \([a-z]*\):.*/\1/p' "$T/rgself_tree.txt" | tr '\n' ' ')
  if [ "$rc" = 0 ] && [ "$nok" = 7 ] && [ -z "$fl" ]; then ok "control: on this tree rc $rc, $nok of 7 cases OK"
  else say "SELFTEST-FAIL (24) regression_gate_selftest.sh does not pass on this tree (rc $rc, $nok OK, FAIL on '$fl'):"; show < <(/usr/bin/grep -E '^(OK|FAIL|####)' "$T/rgself_tree.txt" | sed 's/^/  inner| /'); b=1; fi
  return $b
}

# selftest_asmunit_selftest OUTDIR — arm (25), task #1366. Runs
# asm_unit_selftest.sh, the seeded controls for asm_unit.sh's refusals and the
# rules they guard, which no gate ran (#1352 unresolved (d)). One container on
# $EE_CTX, ~1.5 min (FACT #9011). The DEFAULT form, no argument: it tests this
# tree's own asm_unit.sh with every sibling it needs in place. The arm count is
# read, not pinned, so a row that adds arms does not have to edit this one.
# Fails CLOSED: a VM that is down or a run that dies before its summary
# (docker rc, no `asm_unit_selftest: N arms, F failed` line) is a SELFTEST-FAIL
# naming the rc, never a skip. That it can fail is shown inside the tool by its
# own seeded copies, and for this arm by #1366's seed (asm_unit.sh's dli-pass
# refusal `exit 2` -> `exit 0`: rc 1, 62 failed in this default form, task
# #1374; FACT #9011's 66 is the argument form, incl. 4 SYNC artefacts).
selftest_asmunit_selftest() {
  local T="$1" b=0 rc sum n f np nf
  say "-- (25) ASM_UNIT_SELFTEST (#1366): $HERE/asm_unit_selftest.sh on this tree's asm_unit.sh in one container on $EE_CTX -> rc 0, 'N arms, 0 failed', N PASS lines and no FAIL line; a VM that cannot run it is a FAIL"
  in_vm "sh $HERE/asm_unit_selftest.sh" > "$T/asmunit_selftest.txt" 2>&1; rc=$?
  sum=$(/usr/bin/grep -E '^asm_unit_selftest: [0-9]+ arms, [0-9]+ failed \(/work/tools/ee/asm_unit\.sh\)$' "$T/asmunit_selftest.txt" | tail -1)
  n=$(printf '%s' "$sum" | sed -nE 's/^asm_unit_selftest: ([0-9]+) arms.*/\1/p'); f=$(printf '%s' "$sum" | sed -nE 's/.* arms, ([0-9]+) failed.*/\1/p')
  np=$(/usr/bin/grep -c '^PASS ' "$T/asmunit_selftest.txt" || true); nf=$(/usr/bin/grep -c '^FAIL ' "$T/asmunit_selftest.txt" || true)
  if [ -z "$sum" ]; then say "SELFTEST-FAIL (25) asm_unit_selftest.sh printed no summary line for /work/tools/ee/asm_unit.sh (docker --context $EE_CTX rc $rc): it did not run, and an arm that cannot run is not coverage. Last lines:"; show < <(tail -5 "$T/asmunit_selftest.txt" | sed 's/^/  inner| /'); b=1
  elif [ "$rc" = 0 ] && [ "$f" = 0 ] && [ "$nf" = 0 ] && [ "$n" -ge 1 ] && [ "$np" = "$n" ]; then ok "control: rc $rc, $sum, $np PASS lines"
  else say "SELFTEST-FAIL (25) asm_unit_selftest.sh does not pass on this tree (rc $rc, $n arms, $f failed, $np PASS / $nf FAIL lines):"; show < <(/usr/bin/grep '^FAIL ' "$T/asmunit_selftest.txt" | head -20 | sed 's/^/  inner| /'); b=1; fi
  say "     full output -> $T/asmunit_selftest.txt"
  return $b
}

# sourceable (`. tools/ee/landing_gate.sh`) for the individual check functions
if [ "${BASH_SOURCE[0]}" = "$0" ]; then
  case "${1:-}" in
    # stderr joins stdout so the guard sees both; the guard's exit 3 wins
    --selftest) selftest "${2:-usa}" 2>&1 | selftest_stdout_guard; st=("${PIPESTATUS[@]}")
                [ "${st[1]}" = 0 ] || exit "${st[1]}"; exit "${st[0]}" ;;
    usa|eu) run_gate "$@" ;;
    *) usage ;;
  esac
fi

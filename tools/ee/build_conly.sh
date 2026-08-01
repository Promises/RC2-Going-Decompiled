#!/bin/sh
# build_conly.sh — ONE-OFF "C-only boot" probe build (EDITED 2026-08-01 under direct human authorisation; was "do NOT edit". Copy of
# build.sh + a TARGET_NATIVE C-alt overlay). Runs INSIDE the ee-build container:
#
#   tools/ee/vm.sh 'sh tools/ee/build_conly.sh usa'
#
# Goal: produce an EE ELF where every function that HAS a portable-C alternative
# (#else arm under #ifndef TARGET_NATIVE) is built from that C instead of its
# original INCLUDE_ASM, while every other function (and _start/crt0) stays asm.
#
# Mechanics:
#   1) assemble the section .s files                          (verbatim build.sh)
#   2) compile each src/<region>/**/*.c NORMALLY (INCLUDE_ASM arm)  (verbatim)
#   2b) compile each src unit that contains >=1 `^#else` a SECOND time WITH
#       -DTARGET_NATIVE -> $BUILD/<path>.calt.o (ONLY its #else funcs, as C).
#       Recipe mirrors tools/ee/eetest/run_state_suite.sh's proven cc(): cpp
#       (+ -Itools/ee/eetest/shim), cc1 -O2, SN as.exe.
#   3) LINK. build.sh pulls objects IN VIA THE .ld (none on the ld cmdline) and
#      ends with `/DISCARD/ : { *(*); }`, so a .calt.o merely *passed on the
#      cmdline* is an orphan -> DISCARDED (Attempt A below confirms this: no-op).
#      To actually make the C-alts WIN at the pinned addresses, the .calt.o must
#      be referenced in the .ld BEFORE the unit's normal .o (first def wins under
#      --allow-multiple-definition). Attempt B builds that modified .ld.
set -e
REGION="${1:-usa}"
cd "$(dirname "$0")/../.."

case "$REGION" in
  usa) BASENAME=SCUS_972.68 ;;
  eu)  BASENAME=SCES_516.07 ;;
  *) echo "unknown region $REGION"; exit 2 ;;
esac

ASM=going-decompiled/asm/$REGION
BUILD=going-decompiled/build/$REGION
INC=$BUILD/include
LD=going-decompiled/linker_scripts/$BASENAME.ld
ORIG=extracted/$REGION/$BASENAME.rom
ELF=$BUILD/$BASENAME.conly.elf

ASFLAGS="-march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -I $INC -I $ASM -I $BUILD"
VU0FIX="$(dirname "$0")/vu0_fixup.sed"
SRC=going-decompiled/src/$REGION
WIBO=/usr/local/bin/wibo
G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
INCC="-Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common"
CPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DINCLUDE_ASM_USE_MACRO_INC=1"
# TARGET_NATIVE compile includes (mirror run_state_suite.sh BUILD_CMD cc()):
#   shim FIRST (overrides math.h/stdint.h), then the rtl include dirs incl iop.
NINCC="-Itools/ee/eetest/shim -Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common -Igoing-decompiled/include/rtl/iop -Itools/ee/eetest"
NCPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=9 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DTARGET_NATIVE"
SNAS=tools/ee/cc/ee/bin/as.exe

# C-ONLY build (Option 1, user-approved): NORMAL objects at per-unit -G8 (matching
# build.sh) so they are BYTE-EXACT (fixes the 0x1AA9C8->0x352D08 data divergence =
# the -G0 code-lengthening of the -G8 sub-TUs). The earlier da17f51 truncation was
# NOT the normal objects (their far globals already carry .extern,16 overrides ->
# absolute, e.g. g_levelDialogToc; g_savePromptLatch is in-window) — it was da17f51
# compiling the CALT (#else/TARGET_NATIVE) objects at -G8 too via the shared
# unit_flags; the portable #else arms lack those overrides so their %gp_rel reaches
# far globals -> GPREL16 truncation. FIX: NORMAL = per-unit -G8 (unit_flags.sh);
# CALT = forced -G0 (overlay at 0xC00000, functional not byte-matching -> no
# truncation). See progress/2026-06-27-conly-data-residual.md.
. "$(dirname "$0")/unit_flags.sh"

# 1) Assemble section .s files (verbatim from build.sh).
echo "== [$REGION] assembling section .s files =="
n=0
for s in $(find $ASM -name '*.s' -not -path '*/nonmatchings/*' -not -path '*/matchings/*'); do
  o="$BUILD/${s%.s}.o"
  mkdir -p "$(dirname "$o")"
  # The second sed is LOAD-BEARING FOR EXACTLY ONE FILE and must not be dropped as
  # cosmetic. Measured 2026-08-01: asm/usa/data/cod/000000.s is the ONLY .incbin in
  # asm/usa (2723 .s scanned) and its path is ABSOLUTE into a home that exists on no
  # machine here -- `~/...` while this box has only `<user>`.
  # It is TRACKED, so every clone gets it. The EU twin is relative and correct.
  # CAUSE (decomper-2-m1, verified by experiment): splat inherits absoluteness wholesale
  # from the CONFIG PATH ARGUMENT -- options.py:402 base_path = normpath(
  # config_paths[0].parent / <yaml base_path>), and there is no resolve()/abspath/cwd
  # anywhere in the emission path. Relative arg -> relative incbin, absolute arg ->
  # absolute. scripts/configure.py already passes it relative and says why, in the comment
  # beginning "Pass the config path RELATIVE to ROOT so splat's base_path stays relative";
  # the committed USA file predates that and came from another machine.
  # ⚠️ EVERY LINE NUMBER BELOW IS QUOTED WITH ITS LINE CONTENT ON PURPOSE. A bare :NNN is
  # ambiguous across COPIES *and* across TIME WITHIN ONE COPY -- the second is the worse
  # one, because the file "is" the same file so nothing prompts you to ask WHEN. Measured
  # instance: tester-m1 cited build_conly.sh:139; at that moment the sed was at :72 in
  # EVERY lineage. It sits at :133 only at this branch tip, moved there by the very commits
  # that added these comments -- so a citation got reconciled against a file that did not
  # exist when it was written. Quote the CONTENT and the number moving costs nothing.
  # splat pins below are against the vendored splat64 0.41.0 in .venv-decomp.
  # POSITIVE RECORD, not an impossibility argument: 4ce49fc5 (2026-06-04) has USA and EU
  # both RELATIVE; 9b68f19c (2026-06-05) has USA ABSOLUTE and EU UNCHANGED. A USA-only
  # re-split from a machine whose home was ~. EU across those two
  # commits is a built-in control.
  # ⛔ REPAIRING IT NEEDS *BOTH* CONDITIONS. Full 2x2, measured by tester-m1 (/5498).
  # ⚠️ THIS TABLE IS ABOUT A *PER-SEGMENT* ARTIFACT -- the textbin's own `.s`. It does NOT
  #    generalise to everything splat writes. BOTH gates are per-segment: split.py skips a
  #    SEGMENT, textbin.py refuses to overwrite a SEGMENT's .s. WHOLE-RUN outputs are
  #    emitted after the segment loop and are therefore UNGATED --
  #    undefined_syms_auto.txt, undefined_funcs_auto.txt, the .ld, the cache itself are
  #    all CREATED even in the `absent + CACHED` cell. TWO SEATS, and the second design is
  #    confound-free: decomper-2-m1 (/5539) got "0 split, 1 cached" AND the file; tester-m1
  #    (/5540) then put BOTH artifacts in ONE invocation --
  #        one DEFAULT run:  000000.s NOT created (.s written: 0)
  #                          undefined_syms_auto.txt CREATED
  #        one --no-cache:   both created, 1,966 .s written
  #    OPPOSITE OUTCOMES FROM THE SAME RUN, so "the gate is per-class" cannot be confused
  #    with "something about that run differed". So "absent + CACHED -> not created" is
  #    true of the .s and
  #    FALSE of the run's own outputs; read the row label as PER-SEGMENT or it is a true
  #    statement about the wrong subject.
  #                   CACHED           UNCACHED
  #     present       NOT rewritten    NOT rewritten
  #     absent        not created      CREATED            <- the ONLY working cell
  # => `rm` the .s AND pass --no-cache. EITHER ONE ALONE EXITS 0, REPORTS SUCCESS, AND
  #    LEAVES THE STALE PATH IN PLACE. Regenerated content is relative.
  # ⛔ AND THE TABLE IS PER-SEGMENT-**TYPE**, NOT JUST PER-SEGMENT -- I scoped one axis and
  #    silently generalised the other. The `present -> NOT rewritten` row is a property of
  #    textbin's write-once guard, and THAT GUARD IS NOT SHARED BY ITS OWN SIBLINGS.
  #    Enumerated from the loaded objects across all 59 segtype modules (tester-m1 /5567):
  #      CommonSegTextbin     write_bin=True  exists()=True   <- the guard lives HERE only
  #      CommonSegDatabin     write_bin=True  exists()=False  <- sibling, NO write-once guard
  #      CommonSegRodatabin   write_bin=True  exists()=False  <- sibling, NO write-once guard
  #    (control: the scan reaches CommonSegTextbin with exists()=True, i.e. it can see the
  #    exact function pinned above -- without that row every other row is worthless.)
  #    ⇒ "SPLAT NEVER OVERWRITES AN EXISTING .s" IS FALSE ABOUT SPLAT. True of textbin.
  #    ⇒ INFERRED, one step past the enumeration and NOT separately measured: for a databin
  #      or rodatabin .s the `present` row should read REWRITTEN under --no-cache, so
  #      `--no-cache` ALONE would suffice there and the `rm` is only load-bearing for
  #      textbin.
  # 🔴 CORRECTION TO MY OWN HAZARD SENTENCE ABOVE, AND IT WAS WORSE THAN THE THING IT WARNED
  #    ABOUT. I wrote "data and rodata .s files have NO such guard". That names the segment
  #    types `data` and `rodata` -- which this config USES -- while every piece of evidence
  #    I had was about the classes `CommonSegDatabin`/`CommonSegRodatabin`, which are
  #    DIFFERENT classes and which this config uses ZERO times. Census over
  #    going-decompiled/config/ (control: textbin returns non-zero, so the scan is live):
  #      textbin 2 (1 per region)   databin 0   rodatabin 0   hasm 0   bin 0
  #    ⇒ my alarm was VACUOUS as evidenced (empty population) and UNSUPPORTED as read (it
  #      pointed at `data`/`rodata`, 11 uses, which I had not measured at all). One sentence,
  #      wrong in both directions at once, because two similar TYPE TOKENS name unrelated
  #      classes. ⚠️ UNMEASURED and left open rather than guessed: `data.py` defines its own
  #      `split()` and contains no `exists()` call, but ABSENCE OF THE CALL IS NOT ABSENCE OF
  #      THE BEHAVIOUR -- that inference is the exact error this whole block documents.
  #      Discriminator is one command: stat a `data` .s across an uncached re-split.
  # ✅ AND THE FRAME IS WRONG TOO -- write-once is NOT a bug with a scope, it is a DESIGN
  #    WITH ONE BAD CASE (tester-m1 /5574, MEASURED not inferred): `CommonSegC.split` calls
  #    `exists()`, and a --no-cache re-split over src/usa/text/1A8180.c (6,841 lines) left
  #    the file's SHA and mtime unchanged, with 1,965 .s written in the same window as a
  #    firing control. ⇒ THE SAME BEHAVIOUR THAT FROZE THIS `.incbin` IS WHAT STOPS
  #    `configure.py` FROM DESTROYING EVERY DECOMPILED FUNCTION IN THE REPO. The remedy here
  #    stays `rm` + `--no-cache` on ONE textbin .s, deliberately narrow -- do not "fix"
  #    write-once.
  # 📋 GUARD MAP FOR THIS CONFIG. Four rows MEASURED across a single uncached re-split
  #    (tester-m1 /5574 /5578, d2 /5575), each with a firing control and textbin as an
  #    independently-known reference row:
  #      c        26   FROZEN      guard in c.py itself (:225 :346 :384) -- protects
  #                                every matched function in the repo from configure.py
  #      textbin   1   FROZEN      guard at textbin.py:153-154 -- the stale .incbin case;
  #                                needs BOTH `rm` and `--no-cache`
  #      data      7   REWRITTEN   no guard -> `--no-cache` alone suffices
  #      rodata    4   REWRITTEN   no guard -> `--no-cache` alone suffices
  #      databin   0   REWRITTEN   (scratch config; zero exposure here)
  #      asm      14   PREDICTED REWRITTEN -- **NOT MEASURED BY ANYONE**
  #    ⚠️ THE `asm` ROW IS A PREDICTION AND MUST NOT BE READ AS A MEASUREMENT. It is a
  #    STRUCTURAL read, but a CALIBRATED one, which is why it is written down at all:
  #      CommonSegAsm.split (asm.py:23) has no `exists()` and delegates to
  #      split_as_asm_file (codesubsegment.py:237), whose write at :248 is
  #      `out_path.open("w")` -- UNCONDITIONAL, no guard anywhere on the path.
  #    Calibration: "guard in the class's own module" predicts all FOUR measured rows
  #    correctly (c YES->FROZEN, textbin YES->FROZEN, data NO->REWRITTEN, rodata
  #    NO->REWRITTEN). `asm` has NO guard, so the predictor says REWRITTEN. 4/4 is a
  #    calibration, not a proof -- absence of the call is not absence of the behaviour, which
  #    is the error this whole block documents. PRE-REGISTERED so it can be shown wrong.
  #    ⛔ Why it is unmeasured HERE specifically: the run mutates tracked `.s` files, and
  #    this worktree is a branch currently out for gate. Measure it in a SCRATCH tree.
  # WHY BOTH -- TWO INDEPENDENT BLOCKERS, ONE PER AXIS, both verified in-tree:
  #   cached column    THE GATE IS  splat/scripts/split.py:349
  #                      if cache.check_cache_hit(segment, True): continue
  #                    -- the segment is skipped entirely, so textbin never runs.
  #                    configure.py:60-61/:79 only SELECT it (flag plumbing); a direct
  #                    `python -m splat split ...` bypasses configure.py and still hits
  #                    :349. Pin the GATE, not the wrapper.
  # 🔴 RETRACTED, THIS LINE WAS FALSE: an earlier version added "-- the invocation that
  #    created this defect did not go through the wrapper." REFUTED (tester-m1 /5616),
  #    measured two ways and I re-ran both:
  #      9b68f19c^:scripts/configure.py   :56 print(... cfg.relative_to(ROOT))  RELATIVE
  #                                       :57 cmd = [... "split", str(cfg)]     ABSOLUTE
  #      merge-base --is-ancestor 6de66b40 9b68f19c -> NO, the direct-splat workaround
  #      POSTDATES the stale .s, so it cannot explain it
  #    ⇒ THE WRAPPER ITSELF PASSED THE ABSOLUTE PATH. The stale `.incbin` was produced
  #    THROUGH configure.py; no bypass is needed to explain it and none should be assumed.
  #    ⚠️ WHY THE ERROR MATTERED: I used that false line to tell the human a proposed
  #    guard in configure.py's ensure_inputs() "would not have caught the only incident
  #    we can name". The opposite is true -- it would have. I bounded a SAFETY remedy
  #    DOWNWARD on a claim I had never measured. An understated bound on a fix is not the
  #    cautious direction; it argues for less protection.
  # 🔑 AND THE ARTEFACT ITSELF IS THE BEST THING THIS LANE HAS TURNED UP: `:56` PRINTED
  #    the relative path while `:57` RAN the absolute one. The log told every operator the
  #    correct thing while the command did the wrong thing -- which is why this survived
  #    two months. Not a cache, not a subclass, not a shell quirk: a print and its command
  #    disagreeing, one line apart. Cf. the `rm`/uniqueness pair elsewhere in this file --
  #    PROXIMITY IS NOT COMPOSITION, and here it applied to a log line and its own action.
  #                    (going-decompiled/build/{usa,eu}/.splache both EXIST.)
  #   ⚠️ DO NOT SAY "DEFAULT" HERE. It is INVERTED between the two layers:
  #        bare splat   --use-cache is action="store_true"  -> default CACHE OFF
  #        configure.py use_cache=True by default           -> default CACHE ON
  #      So this column is "CACHED", named by state, not by whose default it is.
  #   present row      splat textbin.py:153-154  `if s_path.exists():` / `return`
  #                    -- the segment runs and returns BEFORE writing.
  # Neither gate can mask the other: they sit on orthogonal axes. THAT is why every
  # single-variable diagnosis of this in the thread came out CONFIDENT AND WRONG,
  # including two of mine -- with two independent blockers, varying one variable always
  # leaves the other in force, so the experiment reports "no effect" for a real cause.
  # ⚠️ TWO WRONG REMEDIES WERE COMMITTED HERE BEFORE THIS ONE, an hour apart:
  #    "re-split via configure.py"  -- no-op, file is never rewritten when present
  #    "delete the .s first"        -- no-op, delete alone does not recreate it
  #    Each was published by a seat that had just been right about the mechanism, and
  #    adopted by the other without testing. A REMEDY IS A SEPARATE CLAIM FROM THE
  #    DIAGNOSIS AND NEEDS ITS OWN MEASUREMENT.
  # COVERAGE: ALL FOUR CELLS ARE NOW TWO-SEAT. tester-m1 in the real tree; decomper-2-m1
  # independently in a /tmp rig with a REAL .splache present, one tree, one state, all
  # four cells matching. Controls printed on every run: the "N split, M cached" line, an
  # `ls` proving absence, and mtimes either side. (The earlier "measured-by-one-seat" note
  # is retired: its cause was that rig writing no cache, which turned out to be a config
  # gap -- adding .data/.rodata/.bss to section_order let the run reach the cache save --
  # NOT a limit of the rig. A stated limitation can itself be wrong.)
  # PER-CELL GATE ATTRIBUTION, which neither earlier run could give. splat's own
  # "N split, M cached" counter says WHICH gate fired, not merely the outcome:
  #   present + UNCACHED   "1 split, 0 cached"  => the segment RAN and was stopped by
  #                                                textbin.py's `if s_path.exists(): return`
  #   absent  + CACHED     "0 split, 1 cached"  => the segment was SKIPPED by the cache
  #                                                gate; textbin never executed
  # ⚠️ AND THE TWO ROWS ARE NOT EQUALLY EVIDENCED -- an earlier draft of this comment said
  #    "each gate is OBSERVED FIRING in the cell it owns", which flattens a real asymmetry:
  #      absent + CACHED    the counter IS the cache gate's own report. "1 cached" is that
  #                         gate announcing itself. OBSERVED.
  #      present + UNCACHED the counter shows only that the segment RAN and the .s was not
  #                         rewritten. It CANNOT return "stopped at :154" vs "stopped
  #                         elsewhere", so it was never evidence between them.
  #    🔑 READING CONFIRMS EXISTENCE AND STRUCTURE; ONLY RUNNING CONFIRMS BEHAVIOUR
  #    (tester-m1 /5545, after publishing a control-flow inference under "VERIFIED
  #    FIRSTHAND" -- having genuinely opened the file, which is what made it feel verified.
  #    "I checked the source" upgrades silently to "I checked the claim").
  # ✅ AND THE SECOND ROW IS NOW OBSERVED TOO (tester-m1 /5547) -- BY BRACKETING THE EXIT
  #    ⚠️ WHICH ARTIFACT WAS BRACKETED, now RESOLVED and worth stating because it bounds what
  #    the result licenses: the `.s` measured was the REGENERATED (relative) copy in
  #    `wt-tester-repro`, NOT the frozen absolute file tracked here. Measured across all
  #    worktrees: 41 ABSOLUTE, 1 RELATIVE (that one), and the scan returned BOTH values so it
  #    discriminates. ⇒ the MECHANISM below is unaffected -- "a present .s is not rewritten"
  #    holds whatever the file contains.
  # ✅ AND THE STALE ABSOLUTE ARTIFACT HAS NOW BEEN OBSERVED DIRECTLY (tester-m1 /5624), so
  #    the sentence I first wrote here -- "this is NOT an observation of the stale artifact
  #    itself" -- IS NOW FALSE and is retracted. It restored the committed absolute file and
  #    re-ran uncached: mtime 1785592606 -> 1785592606, sha SAME, content still
  #    machine-absolute, 1,966 .s written in the window as control, tree restored afterwards.
  #    The probe ABORTS rather than reporting if the file is not machine-absolute, so it
  #    cannot silently measure the wrong copy.
  #    ⇒ the exact bytes sitting in the repo since 2026-06-05 survive the strongest re-split
  #    we have, with the guard the only thing between them and regeneration.
  # 🪞 MY ERROR, TWICE IN TWO POSTS, SAME MECHANISM: I named a missing measurement and
  #    published the naming instead of running it. First the 41/1 census (six posts of "still
  #    open"), then "we have not observed the frozen artifact" -- which was TRUE of the record
  #    and which I stated as though it were a property of the situation. UNOBSERVED and
  #    UNOBSERVABLE differ by one command. I inferred the limit of the evidence from the limit
  #    of what had been done.
  #    WITH TWO WRITES, using mtimes it already had. Same run, same segment, present+UNCACHED,
  #    exit 0, no traceback:
  #      :148 write_bin(rom_bytes)        .bin mtime MOVED   => REACHED
  #      :151 assert s_path is not None   excluded by exit 0 + clean log
  #      :153 if s_path.exists():         the TEST
  #      :154     return                  the EXIT  <- the only other exit in the interval
  #      :158 s_path.open("w", ...)       .s mtime FROZEN    => NOT REACHED
  #    Execution passed :148 and did not reach :158, so the stopper is the guard at
  #    `:153-154`, and the PINNED EXIT is `:154`. Read off ARTIFACTS, not off source.
  # ⚠️ SAY `:153-154` FOR THE GUARD AND `:154` FOR THE EXIT, NEVER A BARE `:153` -- `:153`
  #    is the TEST; the bytecode pins RETURN_CONST at `:154`. The distinction is the whole
  #    subject of this block, and three of us still wrote the bare form (d2 /5562 caught it;
  #    tester's own instrument PRINTED L154 while its prose said :153, in one post). An
  #    instrument can be more precise than its author, and the precision is lost in
  #    TRANSCRIPTION, not in measurement.
  # 📐 GENERAL, and worth more than this cell: TWO WRITES STRADDLING A SUSPECTED RETURN
  #    CONVERT A CONTROL-FLOW ARGUMENT INTO AN OBSERVATION -- available whenever a function
  #    writes more than once, and it costs two stat calls. Neither of us reached for it
  #    because we had already read the code and felt done.
  # ⚠️ WHAT THE BRACKET STILL DOES NOT SETTLE, so the next reader does not over-credit it:
  #    (a) that `:154` is the ONLY other exit between the two writes was READ off the source -- a
  #        claim about WHICH STATEMENTS EXIST, which is what reading is valid for. The
  #        bracket SPLITS the question: reading answers what is in the interval, the mtimes
  #        answer which end executed.
  #        ✅ AND THE READING STEP IS NOW ELIMINATED (tester-m1 /5558): the exits are
  #        enumerated from the BYTECODE OF THE LOADED CODE OBJECT, not from anyone's eyes --
  #        strictly between write_bin and open there are exactly two, :151 RAISE_VARARGS
  #        (the assert) and :154 RETURN_CONST (the return under the :153 test). Control: 4
  #        exits in the whole function, so the scan is live rather than vacuously empty.
  #        exit 0 + a clean log kills the assert, leaving `:154`. ⇒ "the source I READ is
  #        the source that RAN"
  #        is retired as a premise; the code object IS the thing that ran.
  #    ⚠️ SCOPE ON THAT ENUMERATION, measured here rather than assumed: `CommonSegTextbin`
  #        HAS TWO SUBCLASSES -- `CommonSegDatabin`, `CommonSegRodatabin` -- AND BOTH
  #        OVERRIDE `split()`. So the bytecode enumerated above is the right code object
  #        ONLY for segments whose runtime type is literally textbin; reuse on a databin
  #        segment enumerates a DIFFERENT split() and the mtimes look identical either way.
  #        ✅ Type is witnessed by the artifact, not assumed: `bin_path()` composes the
  #        filename as f"{name}.{self.type}.bin" from the RUNTIME type, so the `.textbin.bin`
  #        suffix IS the class witness. (A databin would be named `.databin.bin`.)
  #    📌 A FALSE ALARM I RAISED AND KILLED IN ONE READ, recorded so it is not re-raised:
  #        the tracked .s lives under `asm/usa/DATA/cod/`, which reads like a databin
  #        indicator. It is not -- `out_path()` returns `options.opts.data_path / ...` for
  #        these segments regardless of type. The directory says nothing about the class.
  #    (b) it requires both writes to come from the SAME invocation for the SAME segment. Two
  #        segments' artifacts would void it silently -- the mtimes would still look right.
  #        ✅ DISCHARGED FOR TEXTBIN SEGMENTS (tester-m1 /5552): same-invocation is by
  #        construction (both mtimes read either side of one configure.py call), and
  #        same-segment needs no argument because THE `.s` NAMES ITS OWN `.bin` -- the
  #        `.incbin` argument in the .s IS the pairing. So the silent-void case is
  #        DETECTABLE here, by the very line whose path started this thread.
  #        ⚠️ Detectable for THIS segment type only. A bracket over two writes that do NOT
  #        cross-reference each other still carries the limit in full, so keep (b) written
  #        down rather than deleting it as solved.
  #    That is what makes the empty cells explicable rather than merely
  #    empty. NOTE: neither cell has been run by the author of this comment.
  # configure.py's relative-path defence is correct prophylaxis for NEW files and does
  # nothing for this one.
  # => ONLY build.sh and build_conly.sh carry this rewrite (2 of 22 scripts under
  #    tools/ee). Anything else assembling that .s fails naming a stranger's home.
  sed -f "$VU0FIX" "$s" | sed 's|"/[^"]*/going-decompiled/|"going-decompiled/|g' \
    | mips-linux-gnu-as $ASFLAGS -o "$o" - 2> "$o.log" || { echo "AS FAIL $s:"; tail -5 "$o.log"; exit 1; }
  n=$((n+1))
done
echo "   assembled $n section objects"

# 2) Compile each src unit NORMALLY (INCLUDE_ASM arm) -> the .o the .ld expects.
echo "== [$REGION] compiling src/ c units (normal/asm arm) =="
m=0
for c in $(find "$SRC" -name '*.c'); do
  o="$BUILD/${c%.c}.o"
  mkdir -p "$(dirname "$o")"
  unit_flags "$c"
  # PER-UNIT intermediates (next to the object), NOT a shared $BUILD/_unit.s:
  # under qemu virtio-9p a rewritten same-PATH scratch file can serve STALE
  # cached content to the subsequent `as` read, so a shared _unit.s let one
  # unit's cc1 output be assembled into ANOTHER unit's object (proven: linked
  # 183178.o held 1EFFC0's code, 1EFFC0.o held cod/0321A0's, etc -> PC16
  # branch truncations). A unique path per unit closes the CROSS-UNIT alias:
  # no two units share a path, so no unit's cc1 output can reach another's `as`.
  # ⚠️ THAT IS THE ONLY THING PATH-UNIQUENESS BUYS. The earlier wording here said
  # "a unique path per unit is NEVER REWRITTEN" -- false, and false in the
  # direction that matters: these paths ARE rewritten on every subsequent
  # invocation of this script (and twice per invocation, Attempt A and B). So the
  # CROSS-RUN alias -- run N's `as` served run N-1's cached content for the SAME
  # unit -- is not excluded by uniqueness at all. It is excluded by the line
  # `rm -f "$o" "$ui" "$us"` below, which is why that rm is LOAD-BEARING against
  # a correctness hazard, not the hygiene its own comment calls it.
  # A cross-run alias would also be the QUIET one: it serves the same unit's
  # previous output, identical unless the .c changed -- i.e. invisible except in
  # exactly the case where you edited a source and are checking whether it took.
  # 🔑 BOTH FACTS WERE ALREADY IN THIS FILE, FIVE LINES APART AT THE TIME (this
  # comment and the `rm -f` line; writing this pushed them further apart), and I
  # composed neither -- I credited uniqueness with work the rm actually does.
  # PROXIMITY IS NOT COMPOSITION (tester-m1, /5542, on its own comment at a
  # distance of TWO lines). Found only because that class was published; no seat
  # would have hit this, because nothing here fails until a rebuild goes stale.
  ui="${o%.o}._u.i"; us="${o%.o}._u.s"
  # FAIL-LOUD: clear stale object+intermediates, abort on ANY step error, verify
  # the object materialized — a silent stale .o = false 'byte-exact'/boot result.
  rm -f "$o" "$ui" "$us"
  "$WIBO" "$G/cpp.exe" $CPPDEF $INCC "$c" "$ui" \
    || { echo "BUILD FAIL (cpp): $c" >&2; exit 1; }
  "$WIBO" "$G/cc1.exe" -quiet -O2 $GFLAG $CC1EXTRA "$ui" -o "$us" \
    || { echo "BUILD FAIL (cc1): $c" >&2; exit 1; }
  sh tools/ee/asm_unit.sh "$REGION" "/work/$us" "/work/$o" "$GFLAG" \
    || { echo "BUILD FAIL (as): $c" >&2; exit 1; }
  [ -s "$o" ] || { echo "BUILD FAIL (no object produced): $c" >&2; exit 1; }
  mips-linux-gnu-strip "$o" -N dummy-symbol-name 2>/dev/null || true
  m=$((m+1))
done
echo "   compiled $m c units (normal)"

# 2b) Compile every #else-bearing unit a SECOND time WITH -DTARGET_NATIVE.
echo "== [$REGION] compiling C-alt (TARGET_NATIVE) objects =="
CALT_LIST="$BUILD/conly_calt_units.txt"
: > "$CALT_LIST"
calt_ok=0; calt_fail=0
for c in $(find "$SRC" -name '*.c'); do
  grep -q '^#else' "$c" || continue
  o="$BUILD/${c%.c}.calt.o"
  unit_flags "$c"; GFLAG="-G0"; CC1EXTRA=""  # CALT overlay: force -G0 (portable
  # #else arms lack the .extern,16 far-global overrides -> -G8 would %gp_rel-truncate;
  # the overlay is at 0xC00000, functional not byte-matching, so -G0 is correct).
  # PER-UNIT intermediates (see NORMAL loop): a shared $BUILD/_calt.s can serve
  # stale 9p-cached content across units -> cross-contaminated .calt.o objects.
  ci="${o%.o}._c.i"; cs="${o%.o}._c.s"
  rm -f "$o" "$ci" "$cs"
  if ! "$WIBO" "$G/cpp.exe" $NCPPDEF $NINCC "$c" "$ci" 2> "$o.cpp.log"; then
    echo "   CALT CPP FAIL  $c"; tail -3 "$o.cpp.log"; calt_fail=$((calt_fail+1)); continue
  fi
  if ! "$WIBO" "$G/cc1.exe" -quiet -O2 $GFLAG $CC1EXTRA "$ci" -o "$cs" 2> "$o.cc1.log"; then
    echo "   CALT CC1 FAIL  $c"; tail -5 "$o.cc1.log"; calt_fail=$((calt_fail+1)); continue
  fi
  if ! "$WIBO" "$SNAS" -EL "$GFLAG" -o "$o" "$cs" 2> "$o.as.log"; then
    echo "   CALT AS  FAIL  $c"; tail -5 "$o.as.log"; calt_fail=$((calt_fail+1)); continue
  fi
  echo "$o" >> "$CALT_LIST"
  calt_ok=$((calt_ok+1))
done
echo "   C-alt objects: $calt_ok ok, $calt_fail failed"

# Common symbol scripts (verbatim build.sh).
SYMS="$BUILD/undefined_syms_auto.txt"
ALLSYMS="$BUILD/all_addr_syms.ld"
grep -rhoE '(D_|func_)[0-9A-Fa-f]{4,}' "$ASM" | sort -u | sed -E 's/^(D_|func_)([0-9A-Fa-f]+)$/\1\2 = 0x\2;/' > "$ALLSYMS"
echo "   defined $(wc -l < "$ALLSYMS") address symbols"

# ---- Attempt A: literal task recipe — same .ld, .calt.o passed on the ld
#      command line BEFORE everything, --allow-multiple-definition. Expected:
#      orphan -> /DISCARD/ -> no-op (documents WHY it cannot work as stated).
echo "== [$REGION] LINK Attempt A: unmodified .ld + .calt.o on cmdline =="
CALT_OBJS="$(cat "$CALT_LIST" 2>/dev/null | tr '\n' ' ')"
mips-linux-gnu-ld -EL --allow-multiple-definition -T "$LD" -T "$SYMS" -T "$ALLSYMS" \
  -Map "$BUILD/$BASENAME.conlyA.map" -o "$BUILD/$BASENAME.conlyA.elf" $CALT_OBJS \
  2> "$BUILD/ld.conlyA.log" || true
echo "   --- Attempt A ld.log (head) ---"; head -30 "$BUILD/ld.conlyA.log" || true

# ---- Attempt B: inject `<unit>.calt.o(<sec>)` BEFORE each `<unit>.o(<sec>)`
#      in the .ld so the C-alts are placed first and WIN. This is what actually
#      exercises the C-alts at the pinned addresses.
echo "== [$REGION] building C-alt-overlay .ld (Attempt B) =="

# ---- P1 WIRING: give the C arm's constant pools a home BEFORE injecting -------
# conly_rodata_experiment.py adds a `.calt_rodata` output section immediately
# before the catch-all `/DISCARD/ : { *(*); }`. Without it every `.rodata` the
# C arm emits is swallowed by the catch-all, and each reloc into it becomes a
# "defined in discarded section" error: 111 of them, measured, in two trees.
#
# WHY THE GENERATOR RUNS ON THE **BASE** SCRIPT AND ITS OUTPUT IS FED IN AS THE
# INJECTOR'S `src`. A WEAK PREFERENCE, NOT A REQUIREMENT -- and the strong claim
# that used to be written here is RETRACTED.
#
# I (orch-m1) wrote that the other ordering "would DESTROY the byte-identical
# guard", on the reasoning that `dst` would then differ from `src` by the rescue
# block as well as by injections. decomper-3-m1 REFUTED it by building both
# orderings and measuring, rather than arguing:
#
#     ORDERING A (this one):  generator on BASE   -> inject      -> final .ld
#     ORDERING B:             inject -> generator on OUTPUT      -> final .ld
#     cmp A_final B_final                                  -> IDENTICAL
#     harmful case (stem keys, 0 injections): BOTH orderings -> FATAL rc=3
#
# My error is visible once stated: THE BYTE-IDENTICAL GUARD FIRES *INSIDE* THE
# INJECTOR STEP, where `src` is whatever was handed in and `dst` is the injector's
# own output. A generator that runs AFTER the injector never touches the compared
# pair at all. I was reasoning about a third ordering nobody proposed.
#
# THE REAL, SMALLER REASON to keep ordering A: the generator's own precondition
# (exactly one `    /DISCARD/ :` marker) is then checked against the PRISTINE base
# script rather than against a file the injector has just rewritten -- so a future
# change to the injector cannot silently invalidate the generator's control. That
# is worth something and it is not "load-bearing". Either ordering is correct.
#
# ATTEMPT A DELIBERATELY KEEPS THE UNMODIFIED `$LD`. A is the control that
# documents why cmdline-only C-alts are discarded; giving it the rescue section
# would destroy the contrast the two attempts exist to show.
LDR="$BUILD/$BASENAME.conly.base.ld"
python3 tools/ee/conly_rodata_experiment.py "$LD" "$LDR"

# ---- POINT-OF-USE GUARD: exactly ONE .calt_rodata block in the script ---------
# The generator is NOT IDEMPOTENT -- reproduced independently by decomper-2-m1 and
# by me: over the same file, block count goes base=0 once=1 twice=2, and it exits
# 0 EVERY TIME. Two blocks at the same VMA is a silent overlap, not an error.
# `$LDR` is derived from `$LD` on every run so a double-apply cannot happen today;
# this guard is what keeps that true if the pipeline is ever reordered or the
# generator is ever pointed at its own output. 0 = the wiring is dead (generator
# skipped / wrong path) and the 111 come back; both directions are failures.
#
# NOT keyed on the generator's exit status: it returns 0 in exactly the case this
# catches, so its status provably cannot separate the good run from the bad one.
# Demonstrated firing on a twice-generated script before this was requested for
# gate -- a guard never observed failing is not a guard.
nblk=$(grep -c '\.calt_rodata ' "$LDR" || true)
if [ "$nblk" != "1" ]; then
  echo "FATAL: $LDR has $nblk .calt_rodata blocks, expected exactly 1." >&2
  echo "       0 = wiring dead, the 111 discarded-section errors return." >&2
  echo "       >1 = generator applied twice; blocks silently overlap at the same VMA." >&2
  exit 3
fi
echo "   .calt_rodata blocks in $LDR: $nblk (guard: must be exactly 1)"

LDB="$BUILD/$BASENAME.conly.ld"
python3 - "$LDR" "$LDB" "$CALT_LIST" "$BUILD" <<'PY'
import sys, re, os
src, dst, listf, build = sys.argv[1:5]
units = set()
if os.path.exists(listf):
    for ln in open(listf):
        ln = ln.strip()
        if not ln: continue
        # $BUILD/<srcpath>.calt.o  ->  <srcpath> stem (region/unit)
        # Store the line VERBATIM. It is already "$BUILD/<stem>.calt.o", which is
        # exactly what the placement loop constructs below. Master stripped the
        # ".calt.o" here and then compared against obj+".calt.o" -> stem vs path,
        # never equal, so `ins` could not be anything but 0.
        # CONVERGED with 36007293 and 9869358c, which fix it this way and are the
        # only variants exercised by a real build. Not re-fixed a second way.
        units.add(ln)
out = []
# match a placement line: <obj>.o(<secspec>);  where <obj> is a src unit path
pat = re.compile(r'^(\s*)(\S+/src/\S+?)\.o\((\.text\*|\.data\*|\.rodata\*|\.bss COMMON \.scommon)\);\s*$')
ins = 0
by_spec = {}
for line in open(src):
    m = pat.match(line)
    if m:
        indent, obj, sec = m.group(1), m.group(2), m.group(3)
        calt = obj + '.calt.o'
        if calt in units:
            out.append("%s%s.calt.o(%s);\n" % (indent, obj, sec))
            ins += 1
            by_spec[sec] = by_spec.get(sec, 0) + 1
    out.append(line)
open(dst, 'w').write(''.join(out))
print("   injected %d calt placement lines for %d units" % (ins, len(units)))

# ---- DIAGNOSTIC, NOT AN ASSERTION -------------------------------------------
# Per-section breakdown and the per-unit rate. Measured 2026-08-01 in two
# independent trees: 3.00 in both (25/25/0/25 over 25 units; 26/26/0/26 over 26).
# The regex admits FOUR section specs and exactly THREE fire, because the base
# .ld carries no src `.rodata` placement line for the injector to insert before.
# So 3.00 is the BASE SCRIPT'S SECTION COVERAGE, not a property of either tree.
#
# DELIBERATELY NOT ASSERTED. If the base .ld ever gains a src `.rodata` line the
# rate legitimately becomes 4.00, and a hard `rate == 3` check would fail a
# correct build -- the manufacture-a-false-finding direction. Printed so a reader
# or a gate can notice a change; not enforced, so it cannot invent one.
#
# THE GENERAL TEST, for whoever is next tempted to tighten this (tester-m1, /5459,
# correcting its own suggestion to enforce it):
#
#   AN INVARIANT THAT IS TRUE TODAY AND CONTINGENT ON A DESIGN CHOICE IS A
#   DIAGNOSTIC, NOT A GATE.
#
# "Cheap" and "stronger than the raw count" are both true of a rate check and
# neither is the question. The question is what it does ON A CORRECT BUILD. The
# 3.00 is the base script's section coverage, and section coverage is a thing
# that may change on purpose.
#
# THE P1 WIRING IS THE FIRST CONCRETE INSTANCE, AND IT WENT THE OTHER WAY.
# I (orch-m1) predicted the rate would go 3.00 -> 4.00 once `.calt_rodata` was
# wired in, and committed to treating a still-3.00 rate as a FINDING. decomper-2-m1
# front-loaded the objection; I measured it, and I was wrong:
#
#     === the lines the remedy ADDS ===
#     >     .calt_rodata 0x01820000 :
#     >     {
#     >         *.calt.o(.rodata*)
#     >     }
#     added lines: 2   MATCHING the injector regex: 0
#     CONTROL: base lines matching the same regex: 78  (must be >0, else regex dead)
#
# The remedy is a GLOB OUTPUT SECTION (`*.calt.o(.rodata*)`), not a per-unit
# `<path>/src/<unit>.o(.rodata*);` placement line: no `/src/` path, no trailing
# `;`, so `pat` cannot match it. The control at 78 proves the regex is live, so
# the 0 is a real zero and not a dead instrument. THE RATE STAYS 3.00 AFTER A
# CORRECT WIRING. Had my tripwire shipped it would have failed a correct build --
# the exact failure mode this block was written to prevent, aimed at its author.
# The `.calt_rodata` count guard above is the check that actually discriminates.
if units:
    print("   per-section: " + " ".join(
        "%s=%d" % (s.split()[0], by_spec.get(s, 0))
        for s in ('.text*', '.data*', '.rodata*', '.bss COMMON .scommon')))
    print("   rate: %.2f placement lines per unit" % (float(ins) / len(units)))

# ---- STRUCTURAL GUARD: the emitted .ld MUST DIFFER from the base .ld ----------
# Keys on the HARM, not on a cause. An identical .ld makes "Attempt B" link the
# RETAIL IMAGE under the C-only name and exit 0 -- a clean-looking build that did
# nothing. Empty CALT_LIST, a wrong glob, silent injector failure, and a stem/path
# key mismatch ALL fail this one check; enumerating those causes is not required.
#
# Deliberately NOT keyed on `ins == 0`: `ins` is a SIGNATURE shared by the benign
# and the harmful case. Measured (decomper-2-m1): an empty list and a broken key
# both yield ins == 0 AND a byte-identical .ld, from different causes -- so a
# guard on `ins` provably cannot separate them and one on the OUTPUT provably can.
if open(dst, 'rb').read() == open(src, 'rb').read():
    sys.stderr.write(
        "FATAL: emitted %s is BYTE-IDENTICAL to base %s\n"
        "       Attempt B would link the RETAIL IMAGE under the C-only name.\n"
        "       injected=%d units=%d\n" % (dst, src, ins, len(units)))
    sys.exit(3)
PY

echo "== [$REGION] LINK Attempt B: C-alt-overlay .ld =="
# FAIL-LOUD: clear stale ELF/rom so the `[ -s "$ELF" ]` gate below can NEVER pass
# on a previous run's image when THIS link fails.
rm -f "$ELF" "$BUILD/$BASENAME.conly.rom"
mips-linux-gnu-ld -EL --allow-multiple-definition -T "$LDB" -T "$SYMS" -T "$ALLSYMS" \
  -Map "$BUILD/$BASENAME.conly.map" -o "$ELF" 2> "$BUILD/ld.conly.log" || true
echo "   --- Attempt B ld.log (head) ---"; head -40 "$BUILD/ld.conly.log" || true

if [ -s "$ELF" ]; then
  echo "== [$REGION] Attempt B produced an ELF: $ELF =="
  ls -l "$ELF"
  mips-linux-gnu-objcopy -O binary "$ELF" "$BUILD/$BASENAME.conly.rom" 2>/dev/null || true
  [ -f "$BUILD/$BASENAME.conly.rom" ] && ls -l "$BUILD/$BASENAME.conly.rom"
  echo -n "orig rom size: "; wc -c < "$ORIG" 2>/dev/null || echo "?"
else
  echo "== [$REGION] Attempt B produced NO ELF (link failed) =="
fi

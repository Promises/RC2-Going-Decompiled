#!/usr/bin/env bash
# objdiff_build.sh — pre-build per-unit TARGET and BASE objects for project-wide
# objdiff (objdiff.json at the repo root reads these pre-built .o files).
#
#   tools/ee/objdiff_build.sh <region> <unit> [base_cfile]
#   e.g. tools/ee/objdiff_build.sh usa cod/015180
#        tools/ee/objdiff_build.sh usa cod/015180 some/override.c   # wrong-base test
#        GUARD_CHECK_ONLY=1 tools/ee/objdiff_build.sh usa cod/015180  # guards only, no VM
#
# TARGET object (immutable original bytes) ->
#     going-decompiled/build/<region>/expected/<unit>.o
#   Assembled from the original splat asm tree: we compile a target-only,
#   all-INCLUDE_ASM copy of the unit C with cc1 so its .s pulls in every
#   per-function asm via `.include`, then assemble that with asm_unit.sh
#   (applies vu0_fixup). splat's `nonmatching` macro tags every function with a
#   `<func>.NON_MATCHING` object symbol; that marker is a *base*-side decomp
#   flag, never meaningful on the target, so we strip it from the target object
#   (objdiff special-cases `.NON_MATCHING` and would otherwise drop the symbol).
#
# BASE object (your decomp under test) ->
#     going-decompiled/build/<region>/obj/<unit>.o
#   Compiled from src/<region>/<unit>.c or .cpp (or the optional override
#   cfile) by tools/ee/ee_cc1.sh, the compile step build.sh also uses. Every
#   function that is still `INCLUDE_ASM` pulls in its asm AND that asm's
#   `<func>.NON_MATCHING` marker, so objdiff correctly EXCLUDES it from the
#   report (not yet decompiled). When you replace a stub with real C, its marker
#   disappears and objdiff starts scoring it — 100% once the bytes match. The
#   ee-gcc placeholder symbols are stripped so the symbol tables line up.
#
# To prove the harness end-to-end on the current all-INCLUDE_ASM unit (where the
# base still carries every marker), tools/ee/objdiff_demo.sh builds a base==target
# variant so the unit reads 100% across all 707 functions.
#
# TWO-COMPILER, FUNCTION-SELECTED. R&C2 is a two-compiler build (diff96.sh
# header, project_cc1_subbuild_lead): the SDK/runtime below the entry point was
# built by ee-gcc 2.9-ee-991111 (16-byte callee-save slots), the game engine
# above it by an ee-gcc 2.96 (8-byte slots), which no 2.9 flag reproduces. The
# base is therefore built by up to TWO arms, and the unit report
# (tools/ee/unit_report.sh) takes each function's row from the arm that owns it:
#   sdk29    — cc1 2.9-ee-991111 (wibo), -O2 $GFLAG $CC1EXTRA, unchanged, into
#              obj/<unit>.o. Owns every function NOT named by a MATCH_ guard.
#   engine96 — the diff96.sh engine pipeline: cc1 2.96-ee-001003 (native i386 via
#              its bundled loader), -O2 -G8 -fno-schedule-insns -fno-strict-aliasing,
#              then engine_swap_fix.py + mtc1_fixup.py on the HOST (python3 is not
#              in the container), then asm_unit.sh at -G8, into
#              obj/<unit>.engine96.o. Every `defined(MATCH_<fn>)` guard in the unit
#              source is defined on this arm, so the engine C parked behind
#              `!defined(MATCH_<fn>)` (INCLUDE_ASM for the 2.9 link in build.sh) is
#              compiled here. Owns exactly the functions those guards name; the
#              list is written to tools/ee/.objdiff/<region>/<unit>/engine_funcs.txt.
#   s136os   — not a third object: the tools/ee/s136os_functions.txt rows of the
#              unit are compiled alone by SN 2.95.3 v1.36 -fopt-stack and spliced
#              into the sdk29 base.s over their S136OS_SLOT lines by
#              tools/ee/s136os_splice.sh, the helper build.sh runs at the same
#              point, so obj/<unit>.o carries the image's bytes for them and
#              their rows are scored from it (task #1257, FACT #8810).
#              The splice's --selftest runs first, in the same step-(1)
#              container as the splice, as build.sh runs it: its refusal check
#              once passed on the host's awk and refused every member in the
#              container's mawk (task #1326), so only the container arm counts.
#              A FAIL exits 1 with the log on stderr. It runs once per
#              invocation (one invocation is one unit) and after the
#              $OBJ/$EXPECTED delete, so a failed selftest leaves no object
#              for the report to read (task #1354).
# WHY per FUNCTION and not per unit: routing a whole engine-region unit through
# the 2.96 arm was measured (t276, 2026-09-13) and it un-matches rows that are
# byte-exact under 2.9 today (text/188858 16->12, text/235FE8 73->65,
# cod/0321A0 34->0, text/1A8180 34->8): the 001003 pipeline and the 2.9 -fno-gcse
# model each reproduce a different subset of the ROM's engine compiler, and a
# unit compiles under one scheduler setting. The MATCH_ guard is the selector
# that already exists (diff96.sh's per-function promote), so it is the routing
# predicate here too.
# WHICH UNITS GET AN ENGINE ARM: those whose lowest function VRAM address (from
# the asm tree) is >= ENGINE_BOUNDARY=0x131D98, the first 8-byte-slot function
# in the ROM (project_cc1_subbuild_lead: 16-byte slots end at 0x131A98). Measured
# over every unit's frozen asm (tools/ee/.t276/slot_census.py): cod/015180
# (0x115200..) is 16-byte apart from 3 members (throw_helper (was func_00120BD0), main, snd_Pump);
# every other unit is 8-byte only. A unit below the boundary never gets an engine
# arm; a unit above it gets one only when it carries at least one MATCH_ guard.
# The TARGET object is pure INCLUDE_ASM and is built once, by the 2.9 pipeline.
# A stale obj/<unit>.engine96.o from an earlier run is deleted whenever the arm
# is not built, so the report can never read an old arm. That delete MUST run
# INSIDE a container (it sits in the step-(1) `docker run`), never on the host:
# $OBJ96 is written by the container, and deleting a just-container-written file
# from the host leaves the container's view of it stale, so the NEXT container
# write fails `Fatal error: can't create .../<unit>.engine96.o`. The script has no
# retry loop, so that surfaces as a bare rc=1 the caller reads as their own edit
# breaking the build. It alternates (a failed run leaves no object, so the run
# after it succeeds) and only shows up when runs are seconds apart — a warm
# ASMFIX_SHARED mirror, the default since #398 — which is why a slow cold run
# used to give a false all-clear.
#
# GUARD ENFORCEMENT (#294). The guard token IS the function symbol; both this
# script and unit_report.sh assume it and neither used to check it, so a guard
# that named nothing was a silent no-op that still exited 0. Three checks now
# exit 3, each listing every offending token:
#   1. the token names no <token>.s in this unit's asm tree (host-only, runs
#      BEFORE any docker run — a mis-named guard costs no VM time);
#   2. the unit is sdk-class yet carries guards, which can never be honoured
#      because no engine arm is built below ENGINE_BOUNDARY (host-only);
#   3. the engine arm compiled but emitted no `.ent <token>` for the guard.
#
# Single ops only; the compound commands are the necessary `docker run`s.
# Requires a colima VM carrying the `ee-build` image (see CLAUDE.md).
#
# MOUNT-SYNC (#542; FACT #7449, NOTE #7430; class FACT #7464 as narrowed by
# #7479). The worktree reaches the container over the VM's fuse.sshfs mount,
# and a file the HOST rewrote LONGER (growth is the trigger, FACT #8713) is
# read TRUNCATED at the VM's CACHED length — the size it last read/stat'ed,
# not necessarily the previous host length (#7479) — by a container started
# while that cached size is under ~20 s old (measured: 224/225 grows stale at
# Δ 0..5 s, 18/20 at 10 s, 0/40 at 20 and 30 s; shrinks and same-length
# rewrites read the current bytes; the view heals on a later open, usually
# the next: 92/95 on try 2, one on try 13 in #7479 —
# tools/ee/.t542/summary*.txt). Every host-write ->
# container-read edge below is therefore guarded by tools/ee/mount_sync.sh: the
# host takes the md5, the container re-reads until its md5sum agrees, and
# aborts rc 9 naming the file after MOUNT_SYNC_TRIES (20) x MOUNT_SYNC_SLEEP
# (0.5 s). The guarded reads: $TGTC and $BASECFILE before the step-(1) cpp,
# $BASECFILE again before (2a), and $W/base96.s — rewritten on the host by the
# (2b) post-passes, and GROWN by every nop mtc1_fixup.py inserts — before the
# (2c) assemble (asm_unit.sh, via ASM_UNIT_S_MD5), and the dli allowlist at
# every assemble (asm_unit.sh, via ASM_UNIT_DLISITES_MD5), and the s136os
# selector before the splice (s136os_splice.sh, via S136OS_FUNCS_MD5). NOT guarded: the include
# tree and the frozen asm .s files (git-written, read through the stamped
# mirror), and the container-write -> host-read edge of (2a)->(2b), which
# sshfs flushes on close before `docker run` returns.
#
# WHICH VM (#401). Two equal build VMs exist, `ee-x86` and `ee-x86-b` (same
# ee-build image ID, docker-save/load'd rather than rebuilt). EE_DOCKER_CONTEXT
# selects the docker context for every `docker run` below; unset it and the
# build goes to `colima-ee-x86` exactly as before. The context is passed
# explicitly on every invocation, never inherited from `docker context use` or
# docker's own DOCKER_CONTEXT, so a slot's builds cannot drift to whichever VM
# someone last selected interactively. A context that does not exist fails the
# first `docker run` (rc 1, "context ... not found") — there is no fallback.
#
# --user: the container runs as the invoking uid:gid, so on native-Linux docker
# its outputs are not root-owned (task #1373; why, in landing_gate.sh in_vm).
set -euo pipefail
REGION="$1"; UNIT="$2"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"
# The unit's source is <unit>.c or, once converted, <unit>.cpp (task #1258).
# ee_cc1.sh --resolve picks the one that exists and refuses both. An override
# (arg 3) may be either; its extension selects the front end.
BASECFILE="${3:-$(sh tools/ee/ee_cc1.sh --resolve "going-decompiled/src/$REGION/$UNIT")}"
EE_CTX="${EE_DOCKER_CONTEXT:-colima-ee-x86}"

EXPECTED="going-decompiled/build/$REGION/expected/$UNIT.o"
OBJ="going-decompiled/build/$REGION/obj/$UNIT.o"
W="tools/ee/.objdiff/$REGION/$UNIT"
ASMDIR="going-decompiled/asm/$REGION/nonmatchings/$UNIT"
mkdir -p "$(dirname "$EXPECTED")" "$(dirname "$OBJ")" "$W"

INC="-Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common"
CPPDEF_COMMON="-D__GNUC__=2 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DINCLUDE_ASM_USE_MACRO_INC=1"
CPPDEF="-D__GNUC_MINOR__=9 $CPPDEF_COMMON"

# Per-unit -G override (BASE compile only; the target is pure INCLUDE_ASM so
# -G is irrelevant there). The cod/0321A0 989snd sub-TU is an original separate
# TU built at nonzero -G: its sdata cluster (0x1A7480..0x1A74F8) is accessed
# uniformly via %gp_rel, which only a nonzero -G reproduces. Everything else
# stays at the proven -O2 -G0. CC1EXTRA holds additional per-unit cc1-only
# flags (NOT passed to the assembler): the gameplay-text TUs were built by a
# later SN cc1 without the load-PRE pass, which -fno-gcse reproduces (proven
# byte-exact on text/1907F0). Keep this list in sync with diff.sh.
# S136EXTRA holds the cc1 flags for the unit's s136os splice compile (SN 1.36);
# an arm that does not set it gets CC1EXTRA, so only a unit whose two
# compilers need different flags names it (RULING #9004: 1B4218; RULING #9070: 191238; RULING #9450: 188858).
GFLAG="-G0"
CC1EXTRA=""
unset S136EXTRA
case "$REGION/$UNIT" in
  usa/cod/0321A0) GFLAG="-G8";;
  eu/cod/0321A0) GFLAG="-G8";; # EU mirror of the 989snd sub-TU (same -G8 model)
  usa/text/183178) GFLAG="-G8";; # scale/round accessor sub-TU (D_1A7910..D_1A792C)
  usa/text/188580) GFLAG="-G8";; # camera-aux sub-TU (D_1A8A60..D_1A8AE0)
  usa/text/188858) GFLAG="-G8"; CC1EXTRA="-fno-gcse"; S136EXTRA="";; # Tier-1-A carve (.text mid 2; later-cc1 gameplay/UI TU model). S136EXTRA: the s136os arm compiles at the -O2 default (RULING #9450, FACTs #9441/#9449); the 2.9 compile keeps -fno-gcse
  usa/text/1907F0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # level-init/screen-fade sub-TU (D_1A9000..D_1A9020)
  usa/text/191238) GFLAG="-G8"; CC1EXTRA="-fno-gcse"; S136EXTRA="";; # Tier-1-B carve (.text mid 3; boot/IRX init + sky render + segment loader + map system). S136EXTRA: the s136os arm compiles at the -O2 default (RULING #9070, FACT #9069); the 2.9 compile keeps -fno-gcse
  usa/text/198FA0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # save/GUI-wrapper unit (g_guiInstance modeled cc1-small/assembler-absolute)
  usa/text/1A00F0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # Tier-1-C carve (.text tail head; moby render/anim/grid + ammo-drop + bestiary)
  usa/text/1A8180) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing"; S136EXTRA="-fno-strict-aliasing";; # S136EXTRA: the s136os arm drops -fno-gcse only (RULING #9336, FACTs #9333/#9334); the 2.9 compile keeps both. game-state cluster (carve pick #1; later-cc1 TU model: sized externs under -G8, no load-PRE). -fno-strict-aliasing: the u16 slot-list reads are ordered after the pointer-global cursor stores and re-read after the slot/moby stores in func_002A9550 (task #1100)
  usa/text/250080) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # segment-tail FMV/debug-stub unit (carve pick #2; same later-cc1 TU model)
  usa/text/16E980) GFLAG="-G8";; # 16E980 head: camera/screen-FX unit (carve pick #6). Plain -G8, NO -fno-gcse: the original KEEPS the %hi CSE in a register across AddScreenSpriteFx (load-PRE present), unlike the gameplay-text TUs
  usa/text/248B50) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # GUI sub-chunk 1 (carve pick #3a; later-cc1 TU model, sized externs under -G8)
  usa/text/235FE8) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing";; # GUI widget-method band (carve pick #3b; same later-cc1 GUI TU model). -fno-strict-aliasing restores the per-store pointer-member reload in GuiElementSetPos/GuiElementSetScale (#75)
  usa/text/1CA080) GFLAG="-G8"; CC1EXTRA="-fno-gcse"; S136EXTRA="-fno-gcse -fstrict-aliasing";; # menu-screens A (carve pick #5; later-cc1 TU model, gp-dense). S136EXTRA: RULING #9235 (task #1540) — the s136os arm adds -fstrict-aliasing; the 2.9 compile is unchanged
  usa/text/1D54C0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # menu-screens B (carve pick #5; later-cc1 TU model, gp-dense)
  usa/text/1B4218) GFLAG="-G8"; CC1EXTRA="-fno-gcse"; S136EXTRA="";; # moby-bind band (carve pick #5/moby-bind; later-cc1 TU model, sized externs under -G8). S136EXTRA: the s136os arm compiles at the -O2 default (RULING #9004, FACT #9003); the 2.9 compile keeps -fno-gcse
  # USA CARVE MEGA-BATCH PHASE A (2026-06-14): 7 new c-units carved from the
  # TILE A/B/C/D asm tiles. 6 are later-cc1 gameplay/UI TUs (-G8 -fno-gcse);
  # text/183558 is the math C-helper band; it was built at the default -O2 -G0
  # until task #889 moved it (and text/1FCF48) to -G8.
  usa/text/178E88) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE A render/draw-2D A
  usa/text/1823B8) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE A render/draw-2D B
  usa/text/1DFF80) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B moby-glow/shrub/sky/sound-emit/cinematic
  usa/text/1EFFC0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B tfrag/tie draw + vendor shop + GS/VIF
  usa/text/1FCF48) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # frame-arena/render-task-list sub-TU: the ROM stores g_sceneArenaCursor, g_frameArenaFlip and g_renderTaskWorkBuf %gp_rel, which cc1 cannot emit at -G0 (task #889)
  usa/text/183558) GFLAG="-G8";; # math C-helper band: -G8 so the ROM's %gp_rel store of g_bProgressiveScan is reachable; measured harmless to the unit's existing C (task #889)
  usa/text/1FFBA0) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE B bolt economy + turret weapon
  usa/text/24D728) GFLAG="-G8"; CC1EXTRA="-fno-gcse";; # TILE D GUI/camera helpers
  # EU TEXT RE-TILE (Phase A, 2026-06-14): EU twins of the 11 USA text c-units.
  # Each EU unit is named text/<eu-fileoff> and mirrors its USA sibling's GFLAG.
  eu/text/16E7B8) GFLAG="-G8";;                          # USA 16E980 twin
  eu/text/183088) GFLAG="-G8";;                          # USA 183178 twin
  eu/text/188470) GFLAG="-G8";;                          # USA 188580 twin
  eu/text/190808) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1907F0 twin
  eu/text/198B58) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 198FA0 twin
  eu/text/1A7D10) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1A8180 twin
  eu/text/1C9F58) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1CA080 twin
  eu/text/1D5488) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1D54C0 twin
  eu/text/236ED8) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing";;    # USA 235FE8 twin (#75 -fsa parity: reload-class unlock, applies when EU carves GuiElementSetPos/Scale etc.)
  eu/text/249FE8) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 248B50 twin
  eu/text/251520) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 250080 twin
  # REGION-AXIS CARVE (2026-06-15): EU twins of the 3 recent USA text carves.
  eu/text/188748) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 188858 twin
  eu/text/191240) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 191238 twin
  eu/text/19FC78) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;    # USA 1A00F0 twin
esac
S136EXTRA="${S136EXTRA-$CC1EXTRA}"

# Generate a PRISTINE all-INCLUDE_ASM unit C for the TARGET, straight from the
# asm tree — one INCLUDE_ASM per <func>.s, emitted in ASCENDING VRAM-ADDRESS order
# (each .s carries its address in the first `/* off addr bytes */` comment). This
# preserves the original section layout (alphabetical glob order would misplace
# named funcs like memset) and keeps the target == the original bytes even after
# the editable src/<unit>.c gains real decompiled C (the base uses that source;
# the target must never drift).
TGTC="$W/target_unit.c"
printf '#include "common.h"\n' > "$TGTC"
python3 - "$ASMDIR" "$W/first_addr" >> "$TGTC" <<'PY'
import sys, re, glob, os
asmdir, first_addr_file = sys.argv[1], sys.argv[2]
items = []
for s in sorted(glob.glob(os.path.join(asmdir, "*.s"))):
    func = os.path.basename(s)[:-2]
    addr = None
    with open(s) as fh:
        for line in fh:
            m = re.search(r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+[0-9A-Fa-f]+\s*\*/", line)
            if m:
                addr = int(m.group(1), 16)
                break
    items.append((addr if addr is not None else (1 << 62), func))
for _, func in sorted(items):
    print(f'INCLUDE_ASM("{asmdir}", {func});')
# The region predicate's input: the unit's lowest function VRAM address. A unit
# with no addressed .s at all writes 0 (classes as sdk — nothing to compile anyway).
real = [a for a, _ in items if a != (1 << 62)]
with open(first_addr_file, "w") as fh:
    fh.write(f"0x{min(real):08X}\n" if real else "0x00000000\n")
PY

# ---- decide whether this unit gets an engine arm (see header) ----
ENGINE_BOUNDARY=0x131D98
FIRST_ADDR="$(cat "$W/first_addr")"
OBJ96="going-decompiled/build/$REGION/obj/$UNIT.engine96.o"
ENGINE_FUNCS="$W/engine_funcs.txt"
if [ $((FIRST_ADDR)) -ge $((ENGINE_BOUNDARY)) ]; then REGION_CLASS=engine; else REGION_CLASS=sdk; fi
MATCHDEFS=""; MATCHFUNCS=""
# Every `defined(MATCH_<fn>)` guard in the unit source. The convention — the
# guard TOKEN IS THE FUNCTION SYMBOL — is assumed by this script AND by
# unit_report.sh, and until #294 nothing enforced it: a guard named differently
# from any function in the unit owned NOTHING, its function was silently built
# by the 2.9 arm and scored DIFFERS, and the run still exited 0. Extracted for
# EVERY unit, not just engine-class ones, so a guard below the boundary (which
# gets no engine arm at all) is caught too.
# (the harvest now runs for sdk units too, where it never used to — so say
# plainly that the base C is missing instead of raising a python traceback.)
[ -f "$BASECFILE" ] || { echo "objdiff_build: FATAL — base C not found: $BASECFILE" >&2; exit 2; }
GUARDS="$(python3 -c 'import re,sys; print(" ".join(sorted(set(re.findall(r"defined\s*\(\s*MATCH_([A-Za-z0-9_]+)\s*\)", open(sys.argv[1]).read())))))' "$BASECFILE")"

# GUARD CHECK 1 (host-only, before any docker run): the token must name a
# function THAT EXISTS IN THIS UNIT. The unit's function inventory is its frozen
# asm tree — the TARGET object is generated from exactly those .s files — so a
# token with no <token>.s can never be scored here whatever it compiles to.
# Every offender is listed, not just the first.
BADGUARDS=""
for f in $GUARDS; do
  [ -f "$ASMDIR/$f.s" ] || BADGUARDS="$BADGUARDS $f"
done
if [ -n "$BADGUARDS" ]; then
  echo "objdiff_build: FATAL — MATCH_ guard names no function in $REGION/$UNIT:" >&2
  for f in $BADGUARDS; do echo "    MATCH_$f   (no $ASMDIR/$f.s)" >&2; done
  echo "  A guard token must equal the function symbol it promotes. This one owns nothing:" >&2
  echo "  its function would be built by the 2.9 arm and scored DIFFERS while the run passed." >&2
  exit 3
fi

# GUARD CHECK 2 (host-only): a guard in a unit BELOW ENGINE_BOUNDARY is inert —
# no engine arm is ever built there, so the guard owns nothing however it is
# spelled. Bound this makes visible rather than fixes: cod/015180 carries
# 8-byte-slot members (throw_helper (was func_00120BD0), main, snd_Pump) below the unit predicate;
# they cannot be engine-gated without a per-function override, and this check
# says so loudly instead of letting such a guard read as effective.
if [ "$REGION_CLASS" != engine ] && [ -n "$GUARDS" ]; then
  echo "objdiff_build: FATAL — $REGION/$UNIT is sdk-class (first_addr=$FIRST_ADDR < boundary=$ENGINE_BOUNDARY) but carries MATCH_ guards:" >&2
  for f in $GUARDS; do echo "    MATCH_$f" >&2; done
  echo "  No engine arm is built below the boundary, so these guards own nothing." >&2
  exit 3
fi

if [ "$REGION_CLASS" = engine ]; then
  MATCHFUNCS="$GUARDS"
  for f in $MATCHFUNCS; do MATCHDEFS="$MATCHDEFS -DMATCH_$f"; done
fi
if [ -n "$MATCHFUNCS" ]; then BUILD96=1; else BUILD96=0; fi

# GUARD_CHECK_ONLY=1 stops here: checks 1 and 2 have run and nothing below this
# line has been touched. No docker, no VM, ~0.2s per unit — so the guard
# convention can be swept over every unit in the tree without an hour of build.
# Checks 1 and 2 are the only ones reachable in this mode; check 3 needs the
# engine arm and is therefore only exercised by a real run.
if [ "${GUARD_CHECK_ONLY:-0}" = 1 ]; then
  echo "[objdiff_build] guard-check-only $REGION/$UNIT ($REGION_CLASS, first_addr=$FIRST_ADDR): ${GUARDS:-(no MATCH_ guards)}"
  exit 0
fi

CC296="${CC296:-tools/ee/cc-296}"
CC1_96="$CC296/lib/gcc-lib/ee/2.96-ee-001003-1/cc1"
# The engine arm must never fall back to 2.9 silently: a missing 2.96 cc1 is a
# hard error, not a quieter run.
if [ "$BUILD96" = 1 ] && [ ! -f "$CC1_96" ]; then
  echo "objdiff_build: engine arm needs $CC1_96 — run scripts/fetch_ee_toolchain.sh" >&2; exit 2
fi
# engine96 flags: the diff96.sh flag string (-fno-strict-aliasing is required,
# -G8 always), and __GNUC_MINOR__=96 as diff96.sh preprocesses.
GFLAG96="-G8"; CC1EXTRA96="-fno-schedule-insns -fno-strict-aliasing"
CPPDEF96="-D__GNUC_MINOR__=96 $CPPDEF_COMMON $MATCHDEFS"

# WARM MIRROR BY DEFAULT (#398): ASMFIX_SHARED, when unset, is defaulted to a
# per-worktree mirror named by tools/ee/asmfix_stamp.sh. The block lives in
# tools/ee/asmfix_default.sh so that build.sh gets the identical default (#438:
# it had none and built every unit cold); that file also aborts this script if
# either consumer stops sourcing it or re-inlines the assignment. Sets
# ASMFIX_SHARED (exported), ASMFIX_PRUNE (run in-container below), ASMFIX_STATE.
. tools/ee/asmfix_default.sh

# Compile the pristine *target* C and the *base* C (your decomp / override).
# Each cc1 emits a .s; asm_unit.sh assembles it (with the VU0 fixup mirror) into
# one object. Then:
#   - target: strip the `.NON_MATCHING` object markers (a base-side flag; objdiff
#     special-cases them, so leaving them on the target would drop those symbols).
#   - base:   strip the ee-gcc placeholder symbols so the symbol tables line up.
#     The base KEEPS the `.NON_MATCHING` markers for functions still on INCLUDE_ASM
#     — that is exactly how objdiff knows they aren't decompiled yet.
# (1) target + sdk29 base — byte-for-byte the pre-t276 gate.
# MOUNT-SYNC: $TGTC was just written on the host; $BASECFILE is the worker's
# edit. Both digests are taken here and verified in the container first.
# The dli allowlist is host-written too and GROWS with every row (FACT #8713;
# ledger-29550 read it truncated in a real run of this script): asm_unit.sh
# verifies it against ASM_UNIT_DLISITES_MD5 before its dli pass (task #1205).
TGTC_MD5="$(sh tools/ee/mount_sync.sh md5 "$TGTC")"
BASE_MD5="$(sh tools/ee/mount_sync.sh md5 "$BASECFILE")"
DLISITES_MD5="$(sh tools/ee/mount_sync.sh md5 tools/ee/ps2eeas_dli_sites.txt)"
# The s136os selector is host-written too; s136os_splice.sh verifies it (task #1257).
S136OS_MD5="$(sh tools/ee/mount_sync.sh md5 tools/ee/s136os_functions.txt)"
# RULING #8915 (task #1308): ee_cc1.sh holds a .cpp unit to
# tools/ee/cpp96_allowlist.txt on the engine96 arm, and a MATCH_-guarded .cpp on
# every arm. The unit is named here (EE_CC1_UNIT), not read from $BASECFILE,
# which may be an override at any path; the allowlist is host-written.
ALLOW96_MD5="$(sh tools/ee/mount_sync.sh md5 tools/ee/cpp96_allowlist.txt)"
# $OBJ and $EXPECTED are deleted in the container first, like $OBJ96 (see the
# header for why in the container): a run that stops early must not leave the
# previous run's object for the report to read (ledger-29550 left a stale obj).
docker --context "$EE_CTX" run --rm --user="$(id -u):$(id -g)" -e HOME=/tmp -e ASMFIX_SHARED -e ASM_UNIT_DLISITES_MD5="$DLISITES_MD5" -e S136OS_FUNCS_MD5="$S136OS_MD5" -v "$ROOT":/work ee-build sh -c "
  set -e; cd /work; WIBO=/usr/local/bin/wibo; G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
  export EE_CC1_UNIT='$REGION $UNIT' CPP96_ALLOWLIST_MD5=$ALLOW96_MD5
  sh tools/ee/mount_sync.sh check $TGTC $TGTC_MD5
  sh tools/ee/mount_sync.sh check $BASECFILE $BASE_MD5
  rm -f $OBJ96 $OBJ $EXPECTED
  sh tools/ee/s136os_splice.sh --selftest > $W/s136os_splice_selftest.log 2>&1 \
    || { cat $W/s136os_splice_selftest.log >&2; echo 'objdiff_build: FATAL (s136os_splice --selftest)' >&2; exit 1; }
  tail -1 $W/s136os_splice_selftest.log
  $ASMFIX_PRUNE
  \$WIBO \$G/cpp.exe $CPPDEF $INC $TGTC $W/target.i
  \$WIBO \$G/cc1.exe -quiet -O2 -G0 $W/target.i -o $W/target.s
  sh tools/ee/asm_unit.sh $REGION /work/$W/target.s /work/$EXPECTED
  sh tools/ee/ee_cc1.sh sdk29 $BASECFILE $W/base.i $W/base.s '$CPPDEF $INC' '-O2 $GFLAG $CC1EXTRA'
  sh tools/ee/s136os_splice.sh $REGION $UNIT $BASECFILE $W/base.s $GFLAG '$S136EXTRA'
  sh tools/ee/asm_unit.sh $REGION /work/$W/base.s /work/$OBJ $GFLAG
  mips-linux-gnu-nm $EXPECTED | awk '/\\.NON_MATCHING\$/{print \"-N\", \$3}' > $W/nmstrip.txt
  test -s $W/nmstrip.txt && mips-linux-gnu-strip $EXPECTED \$(cat $W/nmstrip.txt) || true
  mips-linux-gnu-strip $OBJ -N gcc2_compiled. -N __gnu_compiled_c -N __gnu_compiled_cplusplus -N dummy-symbol-name
"

# (2) engine96 base, only when the unit owns MATCH_-guarded functions.
# $OBJ96 is NOT deleted here — the step-(1) container above does it (see header).
# $ENGINE_FUNCS is written and read by the host only, so it is safe to drop here.
rm -f "$ENGINE_FUNCS"
if [ "$BUILD96" = 1 ]; then
  printf '%s\n' $MATCHFUNCS > "$ENGINE_FUNCS"
  # (2a, container) preprocess with the 2.9 cpp as diff96.sh does, compile with
  # the native 2.96 cc1 (cc1plus for a .cpp unit) through its bundled
  # glibc-2.3.6 loader, via tools/ee/ee_cc1.sh.
  # MOUNT-SYNC: the base C is read a second time; it must still be the file
  # step (1) compiled (an edit mid-run would score two different sources).
  BASE_MD5_2A="$(sh tools/ee/mount_sync.sh md5 "$BASECFILE")"
  if [ "$BASE_MD5_2A" != "$BASE_MD5" ]; then
    echo "objdiff_build: FATAL — $BASECFILE changed on the host between step (1) and (2a) (md5 $BASE_MD5 -> $BASE_MD5_2A); rerun" >&2; exit 2
  fi
docker --context "$EE_CTX" run --rm --user="$(id -u):$(id -g)" -e HOME=/tmp -v "$ROOT":/work ee-build sh -c "
  set -e; cd /work
  sh tools/ee/mount_sync.sh check $BASECFILE $BASE_MD5
  EE_CC1_UNIT='$REGION $UNIT' CPP96_ALLOWLIST_MD5=$ALLOW96_MD5 CC296=$CC296 sh tools/ee/ee_cc1.sh engine96 $BASECFILE $W/base96.i $W/base96.s '$CPPDEF96 $INC' '-O2 $GFLAG96 $CC1EXTRA96'
"
  # (2b, host) the engine post-passes, in diff96.sh's order.
  python3 tools/ee/engine_swap_fix.py "$W/base96.s"
  python3 tools/ee/mtc1_fixup.py "$W/base96.s"
  # MOUNT-SYNC: base96.s was written by the (2a) container a moment ago and has
  # just been rewritten on the host — the exact stale-read shape. Its digest
  # goes to asm_unit.sh, which verifies the container's read before assembling.
  S96_MD5="$(sh tools/ee/mount_sync.sh md5 "$W/base96.s")"
  # (2c, container) assemble at -G8, same placeholder strip as the sdk29 base.
docker --context "$EE_CTX" run --rm --user="$(id -u):$(id -g)" -e HOME=/tmp -e ASMFIX_SHARED -e ASM_UNIT_S_MD5="$S96_MD5" -e ASM_UNIT_DLISITES_MD5="$DLISITES_MD5" -v "$ROOT":/work ee-build sh -c "
  set -e; cd /work
  sh tools/ee/asm_unit.sh $REGION /work/$W/base96.s /work/$OBJ96 $GFLAG96
  mips-linux-gnu-strip $OBJ96 -N gcc2_compiled. -N __gnu_compiled_c -N __gnu_compiled_cplusplus -N dummy-symbol-name
"
  # GUARD CHECK 3: the token names a real function of this unit (check 1 passed)
  # but the engine arm never EMITTED it — the guarded C is missing, or the
  # function is still INCLUDE_ASM on both arms. The engine arm would then claim
  # a function it has no row for and unit_report.sh silently falls back to the
  # 2.9 row. This was a WARNING at exit 0 until #294.
  NOENT=""
  for f in $MATCHFUNCS; do
    /usr/bin/grep -qE "^[[:space:]]*\.ent[[:space:]]+$f([[:space:]]|\$)" "$W/base96.s" || NOENT="$NOENT $f"
  done
  if [ -n "$NOENT" ]; then
    echo "objdiff_build: FATAL — guarded in $BASECFILE but no '.ent' in the engine arm ($W/base96.s):" >&2
    for f in $NOENT; do echo "    MATCH_$f" >&2; done
    echo "  The guard owns a function the 2.96 arm never compiled; its row would come from the 2.9 arm." >&2
    exit 3
  fi
fi

echo "[objdiff_build] region -> $REGION_CLASS (first_addr=$FIRST_ADDR, boundary=$ENGINE_BOUNDARY)"
if [ "$BUILD96" = 1 ]; then
  echo "[objdiff_build] engine96 -> $OBJ96 owns: $MATCHFUNCS"
elif [ "$REGION_CLASS" = engine ]; then
  echo "[objdiff_build] engine96 -> (not built: engine unit with no MATCH_ guard)"
else
  echo "[objdiff_build] engine96 -> (not built: sdk unit)"
fi
echo "[objdiff_build] asmfix mirror -> $ASMFIX_SHARED ($ASMFIX_STATE)"
echo "[objdiff_build] target -> $EXPECTED"
echo "[objdiff_build] sdk29  -> $OBJ"

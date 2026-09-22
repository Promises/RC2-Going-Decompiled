#!/usr/bin/env bash
# diff96.sh — per-function matching loop for ENGINE functions, using
# ee-gcc 2.96 + -fno-schedule-insns + the engine post-pass (commutative-swap +
# internal-alignment fix), reproducing R&C2's SN-ProDG engine compiler.
#
#   tools/ee/diff96.sh <region> <unit> <func> <cfile>
#
# WHY (memory project_cc1_subbuild_lead): R&C2 is a TWO-compiler build. SDK/runtime
# (<~0x131B, 16-byte slots) = ee-gcc 2.9-991111 (diff.sh). The game ENGINE
# (>~0x131D, 8-byte slots, ~879 save-walled fns) = ee-gcc 2.96, which no 2.9 flag
# reproduces. 2.96 build 001003 (decomp.me) differs from R&C2's by exactly two
# engine-only codegen rules — commutative-operand precedence and internal code
# alignment — both reproduced by tools/ee/engine_swap_fix.py. PROVEN: func_002A0480
# walled -> 100% through this pipeline. The post-pass is engine-2.96-only and
# cannot affect the 2.9 SDK matches.
#
# Setup: scripts/fetch_2.96.sh (-> tools/ee/cc-296/, gitignored). Requires the
# colima ee-x86 VM + ee-build image. The 2.96 cc1 is a native i386 ELF, run via
# its bundled glibc-2.3.6 loader so it doesn't clobber the container libc.
set -euo pipefail
REGION="$1"; UNIT="$2"; FUNC="$3"; CFILE="$4"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"
# GRIND_SCRATCH lets a concurrent user (e.g. the authoritative-gate tester) build into
# an ISOLATED per-submission scratch instead of the shared .diff96 — closes hazard #1's
# false-PASS path when >1 agent builds on the same worktree. Default = .diff96 (workers
# unaffected). asm_unit.sh's FIXROOT derives from the output path, so it isolates too.
W="${GRIND_SCRATCH:-tools/ee/.diff96}"; mkdir -p "$W"
OBJDIFF=tools/objdiff-cli-macos-arm64
CC="${CC296:-tools/ee/cc-296}"   # CC296 env = isolated (e.g. split-address patched) cc1 dir for gate validation
CC1="$CC/lib/gcc-lib/ee/2.96-ee-001003-1/cc1"
[ -f "$CC1" ] || { echo "2.96 toolchain missing — run scripts/fetch_2.96.sh" >&2; exit 2; }
INC="-Igoing-decompiled/include -Igoing-decompiled/include/rtl/ee -Igoing-decompiled/include/rtl/common"
ASF="-march=r5900 -mabi=eabi -no-pad-sections -EL -G0 -Igoing-decompiled/build/$REGION/include"
# MATCH_<func> promotes exactly the target function to real engine C (siblings stay
# INCLUDE_ASM), so objdiff scopes the diff to this one function. See its guard in the
# unit .c. This is the per-function grind selector — and it STAYS after the match:
# the unit gate (objdiff_build.sh + unit_report.sh) defines every MATCH_ guard on
# its engine96 arm and scores exactly those functions from it, while build.sh
# (cc1 2.9) keeps them INCLUDE_ASM. Dropping the guard would hand the function to
# the 2.9 arm, where an engine-2.96 body cannot match (t276).
CPPDEF="-D__GNUC__=2 -D__GNUC_MINOR__=96 -D__mips__ -D__mips=3 -D__R5900 -D__LANGUAGE_C -D_LANGUAGE_C -D__EE__ -DINCLUDE_ASM_USE_MACRO_INC=1 -DMATCH_$FUNC"
# Engine flag string: -fno-strict-aliasing is REQUIRED — at -O2 the 001003 cc1 has
# strict-aliasing ON, which CSEs a double-deref pointer load (self->p->x; self->p->y)
# into one; R&C2 reloads, so it was built -fno-strict-aliasing (d2's reload-class find,
# corpus-non-regression validated: no-op where irrelevant, required for the reload class).
GFLAG="-G8"; CC1EXTRA="${CC1EXTRA_OVERRIDE:--fno-schedule-insns -fno-strict-aliasing}"   # override for split-address validation

sed -f "$(dirname "$0")/vu0_fixup.sed" \
  "going-decompiled/asm/$REGION/nonmatchings/$UNIT/$FUNC.s" > "$W/$FUNC.s"
cat > "$W/target.s" <<EOF
.include "macro.inc"
.section .text, "ax"
.set noat
.set noreorder
.include "$W/$FUNC.s"
.set reorder
.set at
EOF

# (1) assemble the original asm + compile the C with the native 2.96 cc1 (-> .s).
docker --context colima-ee-x86 run --rm -v "$ROOT":/work ee-build sh -c "
  set -e; cd /work
  WIBO=/usr/local/bin/wibo; G29=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
  LD96='$CC/ld-2.3.6.so --library-path $CC'
  mips-linux-gnu-as $ASF -o $W/target.o $W/target.s
  \$WIBO \$G29/cpp.exe $CPPDEF $INC $CFILE $W/base.i
  \$LD96 $CC1 -quiet -O2 $GFLAG $CC1EXTRA $W/base.i -o $W/base.s
"
# (2) HOST: apply the engine post-passes (python3 not in the container).
python3 "$(dirname "$0")/engine_swap_fix.py" "$W/base.s"
# mtc1_fixup: restore the R5900 COP1 move-to-use hazard nop the 001003 cc1 leaves
# COMMENTED (#nop) under -fno-schedule-insns. Validated (d2, raw verify_match): converts
# func_0034A7F8 90->100, generalizes to the float-store class, no corpus regression
# (correctly skips non-hazard cases). Assembler-compat, same class as move_fixup.sed.
python3 "$(dirname "$0")/mtc1_fixup.py" "$W/base.s"
# MOUNT-SYNC (#542): base.s was container-written in (1) and host-rewritten in
# (2) — the sshfs stale-read shape (FACT #7449). asm_unit.sh verifies the
# container's read against this host md5 before assembling (rc 9 names the file).
S_MD5="$(sh "$(dirname "$0")/mount_sync.sh" md5 "$W/base.s")"
# (3) assemble the post-passed .s.
docker --context colima-ee-x86 run --rm -e ASM_UNIT_S_MD5="$S_MD5" -v "$ROOT":/work ee-build sh -c "
  set -e; cd /work; sh tools/ee/asm_unit.sh $REGION /work/$W/base.s /work/$W/base.o $GFLAG
"

"$OBJDIFF" diff -1 "$W/target.o" -2 "$W/base.o" "$FUNC" -o - --format json-pretty \
  -c mips.instrCategory=r5900 -c mips.abi=eabi32 \
  | python3 -c "import sys,json; d=json.load(sys.stdin); s=[x for x in d['left']['sections'] if x['name']=='.text'][0]; pct=s.get('match_percent',0); print(f'$FUNC (ee-gcc 2.96 engine pipeline): {pct:.2f}% match' + (' ✅ MATCH' if pct==100 else ''))"

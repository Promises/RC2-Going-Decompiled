#!/bin/sh
# asm_unit.sh — assemble a whole compiled unit (cc1 output .s that pulls in
# per-function asm via INCLUDE_ASM's `.include "going-decompiled/asm/.../F.s"`)
# into a single object, applying the VU0 fixup to the included asm.
#
# Runs INSIDE the ee-build container (colima x86 VM):
#   docker --context colima-ee-x86 run --rm -v "$PWD":/work ee-build \
#       sh tools/ee/asm_unit.sh <region> <unit.s> <out.o>
#   e.g. ... sh tools/ee/asm_unit.sh usa /work/build/cod_015180.s out.o
#
# WHY the mirror+cd dance:
#   INCLUDE_ASM hardcodes a *source-relative* include path
#   (`going-decompiled/asm/<region>/nonmatchings/<unit>/<func>.s`). GNU as
#   resolves a relative `.include` against the CWD before any -I dir, so we
#   cannot redirect it with -I alone. Instead we build a filtered MIRROR of the
#   asm tree (each .s passed through tools/ee/vu0_fixup.sed, which only rewrites
#   VU0 Q/ACC operands and is a no-op everywhere else) plus macro.inc, then run
#   `as` with its CWD at the mirror root so every `.include` resolves to the
#   fixed-up copy. Encoding is byte-identical to the original (the fixup only
#   adds the `$` prefix GNU as requires on the Q/ACC special registers).
set -e
REGION="$1"; UNIT_S="$2"; OUT_O="$3"; GFLAG="${4:--G0}"
# GFLAG: optional -G<N> for the assembler (default -G0). cc1 references small
# externs by plain name + `.extern sym,size`, and GNU as decides gp-relativity
# from its own -G threshold - so a base unit compiled at -G8 (cod/0321A0) must
# also be ASSEMBLED at -G8 or the gp_rel accesses macro-expand to lui/lw.
# Explicit %gp_rel/%hi/%lo in the included original asm is unaffected by -G.
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
# MOUNT-SYNC (#542): a caller that wrote $UNIT_S on the HOST (objdiff_build.sh's
# engine arm rewrites base96.s with python there) passes its md5 as
# ASM_UNIT_S_MD5. The container's read of the file is then verified — retried
# while the VM's sshfs view is stale, rc 9 naming the file if it never agrees —
# BEFORE anything is assembled; a stale .s would assemble to a wrong object with
# no error. Unset (a container-written .s, as in build.sh) it checks nothing.
if [ -n "${ASM_UNIT_S_MD5:-}" ]; then
  sh "$ROOT/tools/ee/mount_sync.sh" check "$UNIT_S" "$ASM_UNIT_S_MD5"
fi

# LAYOUT GATE (#1103, FACT #8551, FACT #8553). Every -G8 rule below matches
# cc1's exact line layout - a TAB indent, the mnemonic, a TAB, operands with no
# whitespace (`/^\t(bnel|...)\t/`, `/^\t(sw|lw|...)\t\$r,sym$/`). An input in
# any other layout matches nothing, prints nothing and exits 0, which reads
# exactly like a clean subject, and its -G0 run reads 0 as well, so the two
# dead numbers agree and look like confirmation. That is a hand-written control
# seed's failure, never cc1's, so it is refused here with no object and no
# assembler output (a warning inside a readout is still a readout, #1037).
#   - input missing or unreadable, at any -G: `as` would read an empty stdin
#     and write an empty object at rc 0 (a seed outside the container mount);
#   - -G8, instruction lines present but none in cc1 layout (the whole seed is
#     dead). An input with no instruction lines at all is not refused: a unit
#     whose every function is INCLUDE_ASM is exactly that (usa cod/0213D0);
#   - -G8, any branch the delay-slot guard holds, or any integer load/store of
#     a bare symbol, written outside cc1 layout (that line is dead even when
#     the rest of the seed is live). A bare-symbol memop counts as live only
#     when it matches MEMOP_RE, the one spelling the slot rule keys on, so a
#     trailing comment or space on one is refused (FACT #8616): the slot rule
#     cannot see it. MEMOP_RE is therefore required to cover every spelling
#     cc1 itself emits for such a memop, and that is a measured property, not
#     a definition: the first version missed cc1's `name.N` function-static
#     (`lw $3,s_count.3`) and refused real cc1 output (FACT #8652). The known
#     cc1 spellings are listed at MEMOP_RE. Inline asm
#     reaches the unit .s verbatim and outside cc1 layout, but none of it is a
#     held branch or a bare-symbol memop: lq/sq/cvt.w.s in -G8 units, `la` in
#     the 2.96 arm, syscall/COP0 shims in -G0 cod/015180 (every USA/EU image
#     and objdiff .s, #1103).
# Exit 2 and the `asm_unit.sh: FAIL:` prefix are deliberately NOT the delay-slot
# guard's `REFUSED` (rc 1 from `.error`): a control that counts WARNING/REFUSED
# lines must not read this refusal as its seed firing.
layout_fail() {
  echo "asm_unit.sh: FAIL: $1" >&2
  echo "  input: $UNIT_S (-G: $GFLAG)" >&2
  [ -n "${2:-}" ] && printf '%s\n' "$2" | sed 's/^/  dead line: /' >&2
  echo "  No object written." >&2
  [ "${3:-}" = layout ] && {
    echo "  cc1 layout is: TAB, mnemonic, TAB, operands with no whitespace" >&2
    echo "  (e.g. '\\tbnel\\t\$4,\$0,\$L1' then '\\tlw\\t\$3,g_hx')." >&2
  }
  rm -f "$OUT_O"
  exit 2
}
# The one spelling of a bare-symbol integer memop that the -G8 delay-slot rule
# below can see. The layout scan keys such memops on this SAME regex, so a line
# the scan counts as live is a line the matcher matches (FACT #8616: a scan
# wider than the matcher passed a slot memop with a trailing comment or space,
# and the rule then silently skipped it). It must also cover every spelling cc1
# emits, or the scan refuses real cc1 output (FACT #8652). cc1 2.9's spellings:
#   `\tlw\t$3,g_hx`        an extern or file-scope symbol;
#   `\tsw\t$0,sym+4`       the same plus a decimal offset;
#   `\tlw\t$3,s_count.3`   a function-static, cc1's private `name.N`, which it
#                         emits bare only when the static is small data
#                         (.sbss/.sdata); a larger one is %hi/%lo. The slot
#                         rule leaves these to GNU as (see there);
#   `\tsw\t$4,s_pair.3+4`  the same plus a decimal offset (a member of a
#                         small static struct or array).
# The mnemonics are MEMOP_MN, which the scan's `mem` test also uses, so the
# list is spelled once. `[$]`/`[+]`/`[.]`, not `\$`/`\+`/`\.`: it reaches awk
# as a dynamic regex through -v.
MEMOP_MN='sw|sh|sb|sd|lw|lh|lhu|lb|lbu|ld'
MEMOP_RE="^\t($MEMOP_MN)\t[\$][a-z0-9]+,[A-Za-z_][A-Za-z0-9_]*([.][0-9]+)?([+][0-9]+)?\$"
[ -f "$UNIT_S" ] && [ -r "$UNIT_S" ] \
  || layout_fail "input .s missing or unreadable (a path outside the container mount reads as missing here)"
if [ "$GFLAG" = "-G8" ]; then
  LAYOUT="$(tr -d '\r' < "$UNIT_S" | awk -v memre="$MEMOP_RE" -v memmn="$MEMOP_MN" '
    BEGIN {
      br = "^(j|jal|jalr|b|beq|bne|beql|bnel|blez|bgez|bgtz|bltz|blezl|bgezl|bgtzl|bltzl|bgezal|bltzal|bc1f|bc1t|bgezall|bltzall|bc1fl|bc1tl)$"
      mem = "^(" memmn ")$"
    }
    {
      line = $0
      # an instruction after a label on the same line is never cc1 layout
      lab = sub(/^[ \t]*[A-Za-z0-9_$.]+:[ \t]*/, "", line)
      if (line !~ /^[ \t]*[a-z][a-z0-9.]*([ \t]|$)/) next
      mn = line; sub(/^[ \t]+/, "", mn); ops = mn
      sub(/[ \t].*$/, "", mn); sub(/^[a-z0-9.]+[ \t]*/, "", ops)
      symop = (mn ~ mem && ops ~ /^\$[a-z0-9]+[ \t]*,[ \t]*[A-Za-z_]/)
      # a BARE symbol: no `(base)` outside a comment (`lbu $2,sym($4)` is not)
      opsnc = ops; sub(/[ \t]*#.*$/, "", opsnc)
      bare = (symop && opsnc !~ /\(/)
      # cc1 appends `# high`-style comments to some lines; they stay live. A
      # bare-symbol memop is live only when it matches memre, the spelling the
      # slot rule keys on: with a trailing comment or space it cannot (#8616).
      if (!lab && (bare ? line ~ memre : line ~ /^\t[a-z][a-z0-9.]*(\t[^ \t#]+)?([ \t]*(#.*)?)$/)) { live++; next }
      dead++
      if (mn ~ br || symop) {
        key++; if (key <= 5) keys = keys (keys == "" ? "" : "\n") NR ": " $0
      }
    }
    END {
      if (key) { print "KEY " key; print keys }
      else if (dead && !live) print "ALLDEAD " dead
      else print "OK"
    }')" || LAYOUT="scan exited non-zero"
  case "$LAYOUT" in
    OK) ;;
    KEY*) layout_fail "$(printf '%s\n' "$LAYOUT" | sed -n '1s/^KEY //p') branch/symbolic-memop line(s) outside cc1 layout; the -G8 delay-slot guard cannot see them" \
                      "$(printf '%s\n' "$LAYOUT" | sed 1d)" layout ;;
    ALLDEAD*) layout_fail "no instruction line in cc1 layout ($(printf '%s' "$LAYOUT" | sed 's/^ALLDEAD //') in another layout); no -G8 rule can match this input" "" layout ;;
    *) layout_fail "layout scan produced no verdict ('$LAYOUT')" ;;
  esac
fi
VU0FIX="$ROOT/tools/ee/vu0_fixup.sed"
MOVEFIX="$ROOT/tools/ee/move_fixup.sed"   # cc1 `move` pseudo -> `daddu` (0x2d) for EE
ASMSRC="$ROOT/going-decompiled/asm/$REGION/nonmatchings"
MACINC="$ROOT/going-decompiled/build/$REGION/include/macro.inc"

# Build the filtered mirror in a scratch dir next to the output object.
# Suffixed with the object name so concurrent unit builds that share an output
# dir (e.g. expected/cod/015180.o and expected/cod/0321A0.o) cannot race on one
# mirror (one run's rm -rf would yank the tree out from under the other's
# mkdir/sed, failing with ENOENT).
# OPT-IN shared mirror (ASMFIX_SHARED): the mirror is the SAME whole-tree copy
# for every unit, but rebuilding it per-unit over a slow 9p mount dominates the
# build (minutes/unit). When ASMFIX_SHARED names a path, build the mirror ONCE
# (guarded by a .built marker) and reuse it for every subsequent unit. Safe only
# for SEQUENTIAL unit builds (one asm_unit.sh at a time) — which build_conly is.
# Default (env unset) keeps the per-unit race-safe behaviour verbatim.
if [ -n "${ASMFIX_SHARED:-}" ]; then
  FIXROOT="$ASMFIX_SHARED"
  if [ ! -f "$FIXROOT/.built" ]; then
    rm -rf "$FIXROOT"
    mkdir -p "$FIXROOT/going-decompiled/asm/$REGION/nonmatchings" "$FIXROOT/include"
    find "$ASMSRC" -name '*.s' | while read -r s; do
      rel="${s#"$ROOT"/}"
      mkdir -p "$FIXROOT/$(dirname "$rel")"
      sed -f "$VU0FIX" "$s" > "$FIXROOT/$rel"
    done
    cp "$MACINC" "$FIXROOT/include/macro.inc"
    : > "$FIXROOT/.built"
  fi
else
  FIXROOT="$(dirname "$OUT_O")/.asmfix-$REGION-$(basename "$OUT_O" .o)"
  rm -rf "$FIXROOT"
  mkdir -p "$FIXROOT/going-decompiled/asm/$REGION/nonmatchings" "$FIXROOT/include"
  # Mirror every nonmatching .s through the VU0 fixup, preserving subdirs.
  find "$ASMSRC" -name '*.s' | while read -r s; do
    rel="${s#"$ROOT"/}"
    mkdir -p "$FIXROOT/$(dirname "$rel")"
    sed -f "$VU0FIX" "$s" > "$FIXROOT/$rel"
  done
  cp "$MACINC" "$FIXROOT/include/macro.inc"
fi

# Assemble with CWD at the mirror so source-relative `.include`s resolve there.
cd "$FIXROOT"
# Apply the cc1 `move`->`daddu` fixup to the (cc1-emitted) unit asm before
# assembling. The .include'd original asm is read from the mirror by `as` and is
# untouched (it has explicit `daddu`, never the `move` pseudo).
#
# At -G8 also fix the `la` pseudo (bare `la $r,SYM` and the `la $r,SYM+OFF`
# form cc1 emits for address-plus-constant, e.g. text/198FA0 func_0029C418).
# The SN ee-as expands both with 32-bit adds (proven by the original bytes):
# `addiu $r,$gp,%gp_rel(SYM)` for a small-data symbol, `lui $r,%hi(SYM);
# addiu $r,$r,%lo(SYM)` for an absolute one. GNU as uses the 64-bit `daddiu`
# in both cases, so we expand the pseudo ourselves, deciding smallness exactly
# like the assembler does - from the `.extern SYM, SIZE` directives in the
# same unit .s, keyed on the BASE symbol for the +OFF form (first directive
# wins, matching observed GAS behaviour; a file-scope __asm__(".extern SYM,
# 16") in the C overrides cc1's own size, which is how a cc1-small but
# assembler-absolute original symbol is reproduced). Not applied at -G0,
# where cc1 never relies on gp-relative `la`.
#
# Also reproduce the SN ee-as COP1 load-delay flush (proven by the original
# bytes in text/183178): when a `.set noreorder` region begins directly after
# a cop1 load macro (`l.s`/`lwc1`), the SN assembler conservatively pads the
# load delay with a `nop` before entering the region (it can no longer reorder
# inside it). GNU as treats r5900 cop1 loads as interlocked and emits nothing,
# so we insert the nop ourselves. (That holds for a LOAD: a register lwc1 and a
# symbolic l.s assemble with no pad at the boundary, while an mtc1 there does
# get one - the mtc1 rule below is the other case; FACT #8641.) Not
# applied at -G0: no currently-matched -G0 function has a cop1-load/noreorder
# boundary, and the -G0 units' matches were proven WITHOUT the pad.
#
# At -G8 also expand the `li.s` float-constant pseudo when its IEEE bits need
# a 2-insn materialisation (low half nonzero). The SN ee-as always expands
# `li.s $fN,<c>` inline as `lui $at,hi[; ori $at,$at,lo]; mtc1 $at,$fN`
# (proven by the original bytes in text/1907F0 func_00290EF8); GNU as does
# the same at -G0 (which is why the -G0 units never hit this) and for
# lui-only constants at any -G, but with a nonzero -G it places an
# ori-needing constant in a gp-relative `.lit4` pool instead. We pre-expand
# exactly like the SN assembler so no `.lit4` is emitted. Not applied at -G0
# (GNU as is already byte-identical there).
#
# And the inverse hazard of the COP1 flush above: when a `.set noreorder`
# region begins directly after an `mfc1`, GNU as pads the cop1-move hazard
# with a nop at the region boundary; the SN ee-as does not (the r5900
# interlocks, proven by the original bytes in text/1907F0 func_00290EF8:
# `mfc1 $a3,$f1` directly followed by `beqz $a3`). We hold the mfc1 and emit
# it just inside the region, where GNU as adds no hazard padding. Not applied
# at -G0 (no matched -G0 function has an mfc1/noreorder boundary).
#
# At -G8 also hoist a multi-insn symbolic memory macro out of a branch delay
# slot (proven by the original bytes in text/250080 func_00350F78): cc1
# treats a cc1-small symbol's `sw $0,SYM` as one insn and schedules it into
# the branch delay slot; when the `.extern SYM,16` override makes the
# assembler expand it absolutely (lui $at / sw), the SN ee-as places the
# expansion BEFORE the branch and fills the slot with a nop, while GNU as
# splits it across the branch ("macro expanded into multiple instructions in
# a branch delay slot" warning + the store half landing dead after the jump
# - outright broken code, so this can never affect an already-matched
# function). We reproduce the SN placement: macro first, then the branch,
# then a nop in the slot. Only inside `.set noreorder` regions and only for
# symbols the .extern size map does NOT class as small.
#
# The hoist is only a legal reordering when the slot insn is independent of
# the branch (task #979, FACT #8385). Two shapes are not, and hoisting them
# assembles a different program from cc1's: a BRANCH-LIKELY slot (annulled
# when the branch falls through; hoisted, it runs unconditionally) and a slot
# LOAD whose destination the branch reads or links into (hoisted, it replaces
# the value the branch tests). A linking branch writes its link register
# before the slot runs, so a slot load into it or a slot store of it is the
# same class (#993, FACT #8423); for `jalr` that register is the rd operand,
# $31 only by default. The ROM has neither next to an absolute macro,
# in either placement (USA SCUS_972.68, all branches: 0 of the 61 `lui $at;
# op; branch; nop` sites and 0 of the 3 `lui $at; branch; op` sites), so what
# the SN ee-as did there is unobservable and a function carrying one cannot
# match as written. For those we emit cc1's semantics instead - `lui $at`
# before the branch, the %lo access in the slot - and say so on stderr naming
# the function. A branch that reads $at itself has no correct placement for
# a $at expansion: that is refused with an `.error`, so the unit fails loudly.
#
# At -G8 also honor cc1's `#.set volatile` markers at branch boundaries
# (proven by the original bytes in text/250080 func_00352B90): when cc1
# declines to fill a delay slot itself (volatile memop directly before a
# reorder-mode branch), the SN ee-as left the slot as a nop, while GNU as
# 2.40 reorders the volatile store INTO the slot (the `.set volatile`
# directive is emitted commented-out and ignored). We pin the branch in a
# noreorder/nop wrapper exactly when the directly preceding instruction
# carried the novolatile marker.
#
# At -G8 also keep a 128-bit lq/sq out of a reorder-mode `j $31` slot
# (proven by the original bytes of text/1A8180 func_002A9A68 / func_002A8948,
# text/183558 func_00284028 / func_002839D8, text/16E980 func_00270EB8 and
# cod/015180 func_0012B0D8: `sq; jr $31; nop` in all seven such return tails
# in the USA asm tree, versus two `jr $31; lq/sq` slots, both in hand-written
# asm). cc1 leaves the return unfilled and in reorder mode after an lq/sq;
# the SN ee-as left the slot empty, while GNU as 2.40 swaps the lq/sq into
# it. We pin the return in a noreorder/nop wrapper exactly when the directly
# preceding instruction is an lq/sq.
#
# The same pin holds a `mflo`/`mfhi` out of the return slot (task #919, FACT
# #8243, FACT #8203): cc1 emits `mflo $2; #nop; j $31` in reorder mode and GNU
# as 2.40 swaps the mflo into the slot, while the ROM keeps `mflo; jr $31; nop`
# in all three such tails (USA text/178E88 func_0027A0D0, cod/0321A0
# func_00133988; EU cod/0321A0 func_001339E8) and has no `jr $31; mflo/mfhi`
# anywhere (0 in USA and EU, counted on the decoded ROM words). Not extended
# to other branches or to mflo1/mfhi1: the ROM has no mf* in any branch slot
# either, but only the return tail has a measured function behind it.
#
# And a `cvt.s.w` out of ANY reorder-mode branch slot (task #1053, FACT #8499,
# FACT #8507): cc1 emits `mtc1; cvt.s.w; <branch>` with the slot unfilled and
# GNU as 2.40 swaps the cvt.s.w into it, while the ROM has 0 cvt.s.w in any
# branch delay slot (0 of 598 USA, 0 of 602 EU, every branch kind, against an
# 18-28% slot rate for mov.s/mul.s/add.s/sub.s) and keeps `cvt.s.w; <branch>;
# nop` at 9 USA / 7 EU sites (b, jal, jr - e.g. text/235FE8
# GuiListSetScrollPos). We pin the branch in a noreorder/nop wrapper exactly
# when the directly preceding instruction is a cvt.s.w with no label between
# them. Where GNU as could not have swapped (a label between), the wrapper
# would assemble the same bytes; the reset only keeps the rule's scope exact.
# Not extended to div.s/cvt.w.s: the ROM keeps those out of slots too, but
# that is observed, not tested as a rule.
#
# And the small half of the atomic `la` macro (RULING #9966, NOTE #9963 §3,
# task #1965). The `la` pre-expansion below pins its ABSOLUTE form in a
# noreorder bracket, but prints the small form as a bare 1-word
# `addiu $r,$gp,%gp_rel(SYM)`, which GNU as 2.40 may swap into the slot of a
# following reorder-mode branch (cod/0321A0 snd_FlushCommandRing: `la $16,
# D_001A74A8; b` became `b; addiu`). The SN ee-as never did: the ROM has a
# gp-relative addiu in 0 of 43,315 USA and 0 of 43,497 EU branch delay slots,
# against 617 / 623 gp-relative lw on the same census (FACT #9962), and keeps
# `addiu; <branch>; nop` at 9 sites per region. So when that addiu is DIRECTLY
# followed (no label, directive or comment line between) by a reorder-mode
# non-likely branch, jump or call, the BRANCH is pinned in a noreorder/nop
# wrapper, the form of the pins above. The addiu itself is never bracketed: a
# `.set noreorder` directly after an l.s/lwc1 triggers the COP1 load-delay
# flush below and would add a nop at unrelated sites. Likely branches are
# left alone (GNU as does not swap into an annulled slot, and cc1 never emits
# one in reorder mode). Unscoped at -G8 by ruling, not allowlisted: the census
# is an assembler property. Each pin leaves a `# asm_unit.sh la-slot pin`
# comment, counted into the per-unit line described below.
# (cc1 is a Win32 PE - its .s lines end in CRLF, hence the \r-stripping.)
#
# Last, at every -G, the scoped Ps2EeAs `dli` expansion (RULING #8549, task
# #1105). It runs as tools/ee/ps2eeas_dli.awk, the final pass before `as`. A cc1
# `dli` is replaced by SN Ps2EeAs.exe's expansion only at a site listed in
# tools/ee/ps2eeas_dli_sites.txt: same region, same enclosing `.ent` function,
# same operands. The expansion comes from the row's words, which the ROM carries
# at that address; tools/ee/ps2eeas_dli_sites.py re-derives every row. Every
# other `dli` stays GNU as's: tree-wide, Ps2EeAs's form is wrong in at least 11
# engine chains per region (FACT #8518). Splat's asm carries no `dli`, so an
# INCLUDE_ASM body is never touched. A malformed allowlist row emits an `.error`.
# A listed `dli` directly before a non-likely reorder-mode branch is REFUSED,
# because GNU as slots the expansion's last word where Ps2EeAs does not (FACT
# #8623, FACT #8653; the awk's header has the measured branch lists). So is a
# listed `dli` under `.set nomacro`, which Ps2EeAs rejects (FACT #8698, task
# #1170). Each prints its own `asm_unit.sh: FAIL:` line naming the site.
#
# `as` fed an empty stream still writes a valid-looking object at rc 0, so the
# pass is checked from outside as well as by its own status. Each of these is
# `asm_unit.sh: FAIL:`, exit 2, no object, naming its condition and the
# allowlist's row count:
#   (a) the allowlist has no valid row - 0 bytes, comments only, or every row
#       malformed. A 0-byte file used to make the pass swallow the whole unit
#       and exit 0 (FACT #8640); a comment-only one transformed nothing, so every
#       dli control measured GNU's output while reporting success. Each of the
#       three prints its own line (task #1170): a truncated file and a
#       deliberately emptied one are different repairs;
#   (b) the pass (or the rule pass before it, at -G8) exits non-zero;
#   (c) the pass prints nothing, or fewer lines than it read. It only ever
#       replaces a line with more lines, so fewer means input was lost. (A
#       relative <unit.s> path is one way to get there: the file is read after
#       the `cd` above.)
#   (m) the caller passed the allowlist's host md5 as ASM_UNIT_DLISITES_MD5 and
#       the container's read never agreed with it (see below).
#
# MOUNT-SYNC OF THE ALLOWLIST (task #1205). The allowlist is host-written, and
# every row added GROWS it. A host write that grows a file is read TRUNCATED at
# the old length by a container that opens it within ~20 s of an earlier VM read
# (FACT #8713, which NARROWS #8705; reader-dependent, FACT #8683; the class is
# FACT #7449/#7464, tools/ee/mount_sync.sh). A cut that lands mid-row is refused
# as a malformed row, but a cut on a row boundary is a well-formed SHORTER
# allowlist: the row count cannot see it, and the dropped site assembles as GNU
# as's with no error. It has fired on a real build (ledger-29550: an
# objdiff_build run read the grown allowlist truncated). So a caller that can
# take the host md5 passes it as ASM_UNIT_DLISITES_MD5 (objdiff_build.sh,
# landing_gate.sh do_build), and mount_sync.sh's check re-reads until the
# container agrees, the #542 form used for $UNIT_S above. The verified bytes are
# then COPIED into a container-local file, and every read below (the count and
# the pass) reads that copy, so the two cannot see different allowlists. Unset,
# the copy is still taken but nothing can verify it against the host: a caller
# that writes the allowlist and builds within ~20 s must pass the md5.
#
# Every assembled unit prints one line on stderr (task #1205):
#   asm_unit.sh: dli: N transforms (M allowlist rows for <region>)
# N is the number of dli SITES the pass substituted in this unit, counted from
# its substitution markers in what reaches `as`; M is the allowlist's valid rows
# for <region>. That spelling is an INTERFACE: landing_gate.sh's ASMUNIT row
# knows it, and GATE-F3 (#1158) sums it per region. The words never inflect
# (`1 transforms`), so a consumer needs one pattern. N and M are different
# quantities and N != M is normal: a row substitutes every matching dli in its
# function (func_0027C020's row covers two sites), and a row whose function is
# still INCLUDE_ASM substitutes nothing, since splat's asm has no dli. EU has 0
# rows, so every EU unit prints `0 transforms (0 allowlist rows for eu)`: the
# pass is inert there by construction, which is not the same as clean. A unit
# that is refused prints no such line.
# It is followed by a second line, the small-`la` slot pin's (RULING #9966):
#   asm_unit.sh: la-slot: N pins
# N is the number of branches pinned in this unit, counted from the pin's
# comment markers in what reaches `as`. It is 0 at -G0 by construction (the
# rule lives in the -G8 pass), which is not the same as a -G8 unit with no
# site. landing_gate.sh's ASMUNIT row knows this spelling too.
# And a third, R1's (RULING #10043, task #2004; the pass is described where it
# runs, below the dli pass):
#   asm_unit.sh: noapp-slot: N pins in <unit>[: <fn> <fn> ...]
# <unit> is $UNIT_S relative to src/<region>/ without `._u.s` (cod/015180), or
# the whole path, likewise trimmed, when it is not under src/<region>/. One
# function name per pin, so a function pinned twice is named twice. N is 0 at
# a unit with no inline-asm return, at every -G. landing_gate.sh's ASMUNIT row
# knows this spelling too.
# The selftest is tools/ee/asm_unit_selftest.sh.
DLIAWK="$ROOT/tools/ee/ps2eeas_dli.awk"
DLISRC="$ROOT/tools/ee/ps2eeas_dli_sites.txt"
DLITMP=""; DLISITES=""
dli_fail() {
  echo "asm_unit.sh: FAIL: RULING #8549 dli pass, condition $1 (allowlist $DLISRC: ${DLIROWS:-?} valid row(s))" >&2
  echo "  input: $UNIT_S (-G: $GFLAG)" >&2
  echo "  No object written." >&2
  rm -f "$DLIIN" "$DLIOUT" "$OUT_O"
  [ -z "$DLITMP" ] || rm -rf "$DLITMP"
  exit 2
}
DLIIN=""; DLIOUT=""
for f in "$DLIAWK" "$DLISRC"; do
  [ -r "$f" ] || dli_fail "(a): cannot read $f"
done
if [ -n "${ASM_UNIT_DLISITES_MD5:-}" ]; then
  sh "$ROOT/tools/ee/mount_sync.sh" check "$DLISRC" "$ASM_UNIT_DLISITES_MD5" \
    || dli_fail "(m): the container's read of the allowlist never matched the host md5 $ASM_UNIT_DLISITES_MD5 (MOUNT-SYNC line above; FACT #8713)"
fi
DLITMP="$(mktemp -d)"; DLISITES="$DLITMP/ps2eeas_dli_sites.txt"
cp "$DLISRC" "$DLISITES" || dli_fail "(a): cannot copy $DLISRC"
if [ -n "${ASM_UNIT_DLISITES_MD5:-}" ]; then
  [ "$(md5sum < "$DLISITES" | cut -d' ' -f1)" = "$ASM_UNIT_DLISITES_MD5" ] \
    || dli_fail "(m): the allowlist matched the host md5 $ASM_UNIT_DLISITES_MD5, then its copy did not (FACT #8713)"
fi
# `<valid> <usa> <eu> <bad>` (the awk's count mode). Both regions' rows count:
# EU has none yet, and a region with no row is a normal unit.
DLICOUNT="$(awk -v count=1 -v sites="$DLISITES" -f "$DLIAWK" < /dev/null)" \
  || dli_fail "(b): the allowlist count exited non-zero"
DLIROWS="${DLICOUNT%% *}"
case "$DLIROWS" in
  ''|*[!0-9]*) dli_fail "(a): the allowlist count printed '$DLICOUNT', not a row count" ;;
  0) DLIBAD="$(echo "$DLICOUNT" | awk '{ print $4 }')"
     if [ ! -s "$DLISITES" ]; then dli_fail "(a): the allowlist is 0 bytes (truncated or emptied)"
     elif [ "$DLIBAD" = 0 ]; then dli_fail "(a): the allowlist holds no row, only comments or blank lines"
     else dli_fail "(a): no valid row ($DLIBAD malformed, duplicate or unspellable)"; fi ;;
esac
DLIIN="$(mktemp)"; DLIOUT="$(mktemp)"
PRERC=0
if [ "$GFLAG" = "-G8" ]; then
  sed -E -f "$MOVEFIX" "$UNIT_S" | tr -d '\r' | awk -v memre="$MEMOP_RE" '
    NR==FNR {
      if ($0 ~ /^[ \t]*\.extern[ \t]/) {
        line=$0; sub(/^[ \t]*\.extern[ \t]+/,"",line)
        n=split(line,a,/[, \t]+/)
        if (n>=2 && !(a[1] in sz)) sz[a[1]]=a[2]+0
      }
      next
    }
    function f32bits(v,    s, e, m) {
      s = 0; if (v < 0) { s = 1; v = -v }
      if (v == 0) return s * 2147483648
      e = 0
      while (v >= 2) { v /= 2; e++ }
      while (v < 1)  { v *= 2; e-- }
      m = int((v - 1) * 8388608 + 0.5)
      if (m == 8388608) { m = 0; e++ }
      return s * 2147483648 + (e + 127) * 8388608 + m
    }
    # `$name` -> `$number` for a GPR operand, so one register spelled two ways
    # compares equal (FACT #8471); anything else comes back unchanged. The
    # o32 names GNU as uses under -mabi=eabi: $t0-$t7 are $8-$15, and $fp and
    # $s8 are both $30.
    function gprnum(s,    i, nm) {
      if (!gprinit) {
        split("zero at v0 v1 a0 a1 a2 a3 t0 t1 t2 t3 t4 t5 t6 t7 s0 s1 s2 s3 s4 s5 s6 s7 t8 t9 k0 k1 gp sp fp ra", nm, " ")
        for (i = 1; i <= 32; i++) gpr["$" nm[i]] = "$" (i - 1)
        gpr["$s8"] = "$30"; gprinit = 1
      }
      return (s in gpr) ? gpr[s] : s
    }
    # why hoisting slot insn `ins` above branch `br` would change the program
    # (see header, #979): "" when it is a legal reordering. Registers are
    # compared by number; the caller quotes the operands as written.
    function dslot_hazard(br, ins,    mn, ops, reads, link, n, r, reg, i) {
      mn = br; sub(/^\t/, "", mn); sub(/\t.*/, "", mn)
      ops = br; sub(/^\t[a-z0-9.]+\t/, "", ops)
      if (ops ~ /\$(1|at)([^0-9a-z]|$)/) return "at"
      if (mn ~ /^(beql|bnel|blezl|bgezl|bgtzl|bltzl|bgezall|bltzall|bc1fl|bc1tl)$/) return "branch-likely"
      # The link register is written before the slot runs, so a slot insn
      # touching it sees the return address in place and the old value when
      # hoisted (#993, FACT #8423). It is an OPERAND of the register forms:
      # `jalr $rs` / `jal $rs` link $31, `jalr $rd,$rs` / `jal $rd,$rs` link
      # $rd (and read only $rs); every other linking branch links $31.
      reads = ops; link = ""
      if (mn == "jalr" || (mn == "jal" && ops ~ /^\$/)) {
        n = split(ops, r, ",")
        reads = r[n]; link = (n >= 2) ? r[1] : "$31"
      } else if (mn ~ /^(jal|bgezal|bltzal)$/) link = "$31"
      link = gprnum(link)
      if (link == "$0") link = ""
      n = split(reads, r, ","); reads = gprnum(r[1])
      for (i = 2; i <= n; i++) reads = reads "," gprnum(r[i])
      reg = ins; sub(/^\t[a-z]+\t/, "", reg); sub(/,.*/, "", reg); reg = gprnum(reg)
      if (ins ~ /^\tl/) {
        if (reg == "$0") return ""
        if (("," reads ",") ~ ("," "\\" reg ",")) return "load writes a register the branch reads"
        if (link != "" && reg == link) return "load writes the link register"
      } else if (link != "" && reg == link) {
        return "store reads the link register"
      }
      return ""
    }
    # the small-`la` slot pin (see header) looks at exactly one line: the one
    # directly after the addiu. Any line at all closes the window.
    { lapin = lapend; lapend = 0 }
    /^[ \t]*\.ent[ \t]/ { curfn = $2 }
    # flush a held mfc1 unless the next line opens a noreorder region (or is
    # a comment-only line, which we let pass while still holding)
    {
      if (pend != "" && $0 !~ /^[ \t]*\.set[ \t]+noreorder/ && $0 !~ /^[ \t]*#/) {
        print pend; pend = ""
      }
    }
    # delay-slot macro handling (see header), keyed on the .extern size map:
    #   size <= 8   true small data - GNU as expands the macro to the 1-insn
    #               %gp_rel form itself; the line stays in the slot untouched.
    #   size 9..15  gp-addressable but assembler-absolute (the marker for SN
    #               cc1-small symbols whose non-delay-slot accesses are the
    #               absolute lui/$at macro): SN-as keeps the DELAY-SLOT access
    #               as the 1-insn %gp_rel form (proven by the original bytes
    #               of func_002AC9E0 / func_002A9468 in text/1A8180), so we
    #               rewrite the operand explicitly; GNU as expands every
    #               other access absolutely (size > G threshold).
    #   size >= 16 / unknown: truly absolute - SN-as hoists the 2-insn
    #               expansion above the branch and fills the slot with a nop.
    /^[ \t]*\.set[ \t]+reorder/ { nore = 0 }
    {
      if (pendbr != "") {
        msym = ""; mfull = ""
        if ($0 ~ memre) {
          mfull = $0; sub(/^\t[a-z]+\t\$[a-z0-9]+,/, "", mfull)
          msym = mfull; sub(/\+[0-9]+$/, "", msym)
        }
        # A cc1 `name.N` function-static is emitted bare only as small data it
        # defines in this unit (.sbss/.sdata; see MEMOP_RE), so GNU as already
        # assembles the 1-insn %gp_rel form in the slot, as it did before
        # MEMOP_RE could match it (FACT #8652). It has no .extern size, and the
        # unknown-size branch below would hoist it: leave it untouched.
        if (msym ~ /[.][0-9]+$/) msym = ""
        if (msym != "" && !((msym in sz) && sz[msym] <= 8)) {
          if ((msym in sz) && sz[msym] <= 15) {
            line = $0
            sub(/,[A-Za-z_][A-Za-z0-9_+]*$/, ",%gp_rel(" mfull ")($28)", line)
            print pendbr; print line
          } else if ((why = dslot_hazard(pendbr, $0)) == "at") {
            printf "asm_unit.sh: REFUSED: %s: `%s` in the delay slot of `%s`, which reads $at - no placement of the $at expansion keeps the program\n", curfn, substr($0, 2), substr(pendbr, 2) | "cat 1>&2"
            print "\t.error \"asm_unit.sh: absolute macro in the slot of a branch reading $at (" curfn ")\""
          } else if (why != "") {
            printf "asm_unit.sh: WARNING: %s: `%s` in the delay slot of `%s` NOT hoisted (%s); emitted as lui $at before the branch + the %%lo access in the slot. The ROM has no such site: this function cannot match as written (#979, FACT #8385)\n", curfn, substr($0, 2), substr(pendbr, 2), why | "cat 1>&2"
            op = $0; sub(/^\t/, "", op); sub(/\t.*/, "", op)
            reg = $0; sub(/^\t[a-z]+\t/, "", reg); sub(/,.*/, "", reg)
            print "\t.set\tnoat"
            print "\tlui\t$1,%hi(" mfull ")"
            print pendbr
            print "\t" op "\t" reg ",%lo(" mfull ")($1)"
            print "\t.set\tat"
          } else {
            print $0; print pendbr; print "\tnop"
          }
          pendbr = ""; prevcop = ""
          next
        }
        print pendbr; pendbr = ""
      }
    }
    # The likely branches missing from the list below are held for the
    # slot-macro check only (#993): nothing else sees them, so GNU as split an
    # absolute slot macro across them with no asm_unit.sh line. cc1 emits
    # bc1fl/bc1tl; bgezall/bltzall only reach here from hand-written asm. The
    # reorder-mode pins below are measured on other branches and are not
    # extended to these.
    # A held branch is a non-FPU insn after any mtc1, so it ends the mtc1
    # hazard window here: its `next` skips the reset in the mtc1 rule below,
    # which otherwise padded the SLOT insn, putting a nop between the branch
    # and its slot (FACT #8063, task #1352; the ROM has 100 `mtc1; jump;
    # slot reads $fN` sites and none with a nop, NOTE #8972).
    /^\t(bgezall|bltzall|bc1fl|bc1tl)\t/ { if (nore && pendmov == "") { pendbr = $0; lastmtc = ""; next } }
    /^\t(j|jal|jalr|b|beq|bne|beql|bnel|blez|bgez|bgtz|bltz|blezl|bgezl|bgtzl|bltzl|bgezal|bltzal|bc1f|bc1t)\t/ {
      if (nore && pendmov == "") { pendbr = $0; lastmtc = ""; next }
      # small-`la` slot pin (see header, RULING #9966): SN-as never swapped
      # its 1-word gp-relative `la` into a following branch slot.
      if (lapin && !nore && $0 ~ /^\t(j|jal|jalr|b|beq|bne|blez|bgez|bgtz|bltz|bgezal|bltzal|bc1f|bc1t)\t/) {
        print "\t# asm_unit.sh la-slot pin (RULING #9966)"
        print "\t.set\tnoreorder"; print; print "\tnop"; print "\t.set\treorder"
        lapin = 0; prevcop = 0
        next
      }
      # volatile-marker pin (see header): the insn directly before this
      # reorder-mode branch was volatile - SN-as left the slot empty.
      if (volpend) {
        print "\t.set\tnoreorder"; print; print "\tnop"; print "\t.set\treorder"
        volpend = 0; prevcop = 0
        next
      }
      # 128-bit store/load and mflo/mfhi pin (see header): SN-as never
      # swapped an lq/sq or an mflo/mfhi into a reorder-mode return slot.
      if (qpend && $0 ~ /^\tj\t\$31[ \t]*$/) {
        print "\t.set\tnoreorder"; print; print "\tnop"; print "\t.set\treorder"
        qpend = 0; prevcop = 0
        next
      }
      # cvt.s.w pin (see header): SN-as never swapped a cvt.s.w into any
      # reorder-mode branch slot.
      if (cvtpend) {
        print "\t.set\tnoreorder"; print; print "\tnop"; print "\t.set\treorder"
        cvtpend = 0; prevcop = 0
        next
      }
    }
    /^[ \t]*#\.set[ \t]+novolatile/ { print; volpend = 1; next }
    # GNU as 2.40 refuses to fill a reorder-mode delay slot with a MIPS4
    # conditional move; the SN ee-as moved it in like any other insn (proven
    # by the original bytes of text/1A8180 CountSkillPointsCompleted /
    # CountPlatinumBolts: `bnez ...; movn` in the slot where cc1 emitted
    # movn-then-branch). When a movn/movz directly precedes a reorder-mode
    # branch that does not read its destination, pin the SN placement.
    {
      if (pendmov != "") {
        if (pmst == 0 && $0 ~ /^[ \t]*\.set[ \t]+noreorder/) {
          # the branch is opening its own noreorder bracket - keep holding
          print; nore = 1; pmst = 1; next
        }
        if (pmst == 1 && ($0 ~ /^[ \t]*\.set[ \t]+nomacro/ || $0 ~ /^[ \t]*#/)) {
          print; next
        }
        if (pmst == 1 && $0 ~ /^\t(j|b|beq|bne|blez|bgez|bgtz|bltz)[a-z]*\t/) {
          dst = pendmov; sub(/^\tmov[nz]\t/, "", dst); sub(/,.*/, "", dst)
          gsub(/\$/, "\\$", dst)
          # HOLD the branch too (do not print yet): pmst==2 decides from the
          # delay slot whether the cond-move fills an empty (nop) slot or must
          # stay BEFORE a slot cc1 already filled (the `movz; jr $31; store(delay)`
          # return tail). Printing the branch here orphaned the store.
          if ($0 !~ (dst "([^0-9a-z]|$)")) { heldbr = $0; pmst = 2; next }
        }
        if (pmst == 0 && nore == 0 && $0 ~ /^\t(j|b|beq|bne|blez|bgez|bgtz|bltz)[a-z]*\t/) {
          # bare reorder-mode branch directly after the cond-move
          dst = pendmov; sub(/^\tmov[nz]\t/, "", dst); sub(/,.*/, "", dst)
          gsub(/\$/, "\\$", dst)
          if ($0 !~ (dst "([^0-9a-z]|$)")) {
            print "\t.set\tnoreorder"; print; print pendmov; print "\t.set\treorder"
            pendmov = ""; pmst = 0
            next
          }
        }
        if (pmst == 2) {
          if ($0 ~ /^\tnop[ \t]*$/) {
            # cc1 left an empty (nop) slot in its noreorder bracket: SN-as fills
            # it with the cond-move (branch printed, then the move in the slot;
            # the nop is dropped).
            print heldbr; print pendmov; heldbr = ""; pendmov = ""; pmst = 0; next
          }
          # cc1 ALREADY filled the delay slot with a real insn (the
          # `movz; jr $31; store(delay)` return tail): the cond-move is an
          # ordinary insn BEFORE the branch, not a slot filler. Emit move, then
          # branch, then fall through to print the real slot insn ($0).
          print pendmov; print heldbr; heldbr = ""; pendmov = ""; pmst = 0
        } else {
          # any other shape: flush the held cond-move in its original place.
          print pendmov; pendmov = ""; pmst = 0
        }
      }
    }
    /^\tmov[nz]\t\$/ { pendmov = $0; pmst = 0; next }
    /^\t/ {
      if ($0 !~ /^\t\.|^\t#/) volpend = 0
      if ($0 !~ /^\t\.|^\t#|^\t[ \t]*$/) {
        qpend = ($0 ~ /^\t(lq|sq|mflo|mfhi)[ \t]/)
        cvtpend = ($0 ~ /^\tcvt\.s\.w[ \t]/)
      }
    }
    # a label ends the cvt.s.w pin: GNU as never swaps across one
    /^[A-Za-z0-9_$.]+:/ { cvtpend = 0 }
    # SN-as mtc1 write-back hazard (proven by the original bytes of
    # text/1A8180 func_002A8600 / func_002A87A8): an mtc1 directly followed
    # by an FPU op that READS the just-written register gets one padding nop
    # from the SN ee-as. GNU as 2.40 inserts the same nop itself when the pair
    # is assembled in reorder mode (FACT #8606, FACT #8622), so there this rule
    # changes no byte; it is live only inside a `.set noreorder` region, where
    # gas does not pad (the #8622 noreorder control). Whether any cc1 output has
    # a dependent mtc1 pair inside noreorder is not measured (#1113). Track the
    # last mtc1 destination and pad when the very next instruction is a
    # dependent FPU op. (mtc1 followed by a non-FPU insn or an independent FPU
    # op is NOT padded - proven by the same functions.)
    {
      if (lastmtc != "" && $0 ~ /^\t/ && $0 !~ /^\t\.|^\t#/) {
        if ($0 ~ /^\t(add|sub|mul|div|abs|neg|mov|sqrt|max|min)\.s\t/ || $0 ~ /^\tcvt\.[a-z.]+\t/ || $0 ~ /^\tc\.[a-z]+\.s\t/) {
          if ($0 ~ ("\\$f" lastmtc "([^0-9]|$)")) print "\tnop"
        }
        lastmtc = ""
      }
    }
    /^\tmtc1\t/ {
      lastmtc = $0; sub(/^\tmtc1\t\$[a-z0-9]+,\$f/, "", lastmtc)
      print; next
    }
    /^\tmfc1\t/ { pend = $0; prevcop = ""; next }
    /^\tli\.s\t\$f[0-9]+,/ {
      s = $0; sub(/^\tli\.s\t/, "", s)
      split(s, q, ","); r = q[1]; bits = f32bits(q[2] + 0)
      hi = int(bits / 65536); lo = bits % 65536
      if (lo != 0)
        printf "\tlui\t$1,0x%x\n\tori\t$1,$1,0x%x\n\tmtc1\t$1,%s\n", hi, lo, r
      else
        printf "\tlui\t$1,0x%x\n\tmtc1\t$1,%s\n", hi, r
      # (the lui-only expansion is byte-identical to what GNU as emits; we
      # expand it here too so the mtc1 hazard rule below can see it)
      lastmtc = r; sub(/^\$f/, "", lastmtc)
      prevcop = ""
      next
    }
    /^[ \t]*\.set[ \t]+noreorder/ {
      nore = 1
      # cop1 load-delay flush (see header; refined 2026-06-12): SN-as pads
      # the load delay at a noreorder boundary after a SYMBOLIC cop1 load
      # macro (l.s/lwc1 of a symbol, which it expands itself - proven by
      # text/183178), but NOT after a plain register-offset lwc1 (proven by
      # text/1A8180 func_002B1348: lwc1 $f12,0(sp) directly before a jal
      # region, no pad).
      if (prevcop) print "\tnop"
      print
      if (pend != "") { print pend; pend = "" }
      next
    }
    /^\tla\t\$[0-9]+,[A-Za-z_][A-Za-z0-9_]*(\+[0-9]+)?$/ {
      s=$0; sub(/^\tla\t/,"",s)
      split(s,p,","); r=p[1]; sym=p[2]
      base=sym; sub(/\+[0-9]+$/,"",base)   # smallness is decided by the BASE symbol
      if ((base in sz) && sz[base]<=8) {
        printf "\taddiu\t%s,$gp,%%gp_rel(%s)\n", r, sym
        lapend = 1   # the slot pin window opens on the next line
      } else {
        # the SN ee-as la macro is atomic: GNU as must not steal its second
        # half into a following branch delay slot (proven by the original
        # bytes of text/1A8180 func_002B11C8: lui/addiu adjacent, nop in the
        # jal slot), so the expansion is pinned in a noreorder bracket.
        printf "\t.set\tnoreorder\n\tlui\t%s,%%hi(%s)\n\taddiu\t%s,%s,%%lo(%s)\n\t.set\treorder\n", r, sym, r, r, sym
      }
      prevcop=""
      next
    }
    { print; prevcop = ($0 ~ /^\t(l\.s|lwc1)\t\$f[0-9]+,[A-Za-z_]/) }
    END {
      if (pendmov != "") print pendmov
      if (heldbr != "") print heldbr
      if (pendbr != "") print pendbr
      if (pend != "") print pend
    }
  ' "$UNIT_S" - > "$DLIIN" || PRERC=$?
else
  sed -E -f "$MOVEFIX" "$UNIT_S" > "$DLIIN" || PRERC=$?
fi
[ "$PRERC" = 0 ] || dli_fail "(b): the pass before it exited $PRERC, so its input is not the unit"
DLIRC=0
awk -v region="$REGION" -v sites="$DLISITES" -f "$DLIAWK" < "$DLIIN" > "$DLIOUT" || DLIRC=$?
case "$DLIRC" in
  0) ;;
  3) # an adjacency or nomacro refusal: the pass has printed `asm_unit.sh: FAIL:` itself
    echo "  input: $UNIT_S (-G: $GFLAG)" >&2
    echo "  No object written." >&2
    rm -f "$DLIIN" "$DLIOUT" "$OUT_O"; rm -rf "$DLITMP"; exit 2 ;;
  *) dli_fail "(b): ps2eeas_dli.awk exited $DLIRC" ;;
esac
NIN=$(($(wc -l < "$DLIIN"))); NOUT=$(($(wc -l < "$DLIOUT")))
[ -s "$DLIOUT" ] || dli_fail "(c): ps2eeas_dli.awk printed nothing ($NIN line(s) in)"
[ "$NOUT" -ge "$NIN" ] || dli_fail "(c): ps2eeas_dli.awk printed $NOUT line(s) for $NIN in"
# the per-unit transform line (see above): N from the pass's markers, M from
# the count mode's per-region field (`<valid> <usa> <eu> <bad>`)
DLIN=$(awk '/^\t# ps2eeas_dli_sites\.txt [^ ]+: dli / { n++ } END { print n + 0 }' "$DLIOUT")
DLIM=$(echo "$DLICOUNT" | awk -v r="$REGION" '{ print (r == "usa") ? $2 : (r == "eu") ? $3 : 0 }')
LAPINS=$(awk '/^\t# asm_unit\.sh la-slot pin / { n++ } END { print n + 0 }' "$DLIOUT")
# R1, the no-fill after an inline-asm block (RULING #10043, task #2004). The
# held SN assembler, Ps2EeAs.exe 1.9.25.758, never fills a reorder-mode delay
# slot (FACT #10035, confirmed by FACT #10040). cc1 fills its own C return
# slots, so the only slot it hands the assembler at a return is one directly
# after an inline-asm block, where GNU as 2.40 moves the block's last
# instruction into it and Ps2EeAs leaves a nop. So a reorder-mode `j $31` /
# `jr $31` whose previous non-blank line is cc1's `#NO_APP` is printed
# `.set noreorder` / the return / `nop` / `.set reorder`. Nothing else is
# touched: a BROAD no-fill (every reorder-mode branch) breaks 10 matched -G0
# cod/ functions (FACT #10042) and is not granted. It runs at every -G, after
# every pass above (the -G8 lq/sq return pin already brackets func_002A9A68's
# return, so the line before it is `.set noreorder` and R1 does not see it).
# Measured price on master 4a8c55b15: 27 sites in 25 matched functions, 0
# bytes changed - GNU as moves neither a `syscall` nor an `mtc1` into the slot
# (FACT #10042). Each pin leaves a `# asm_unit.sh noapp-slot pin` comment
# naming its function, counted into the per-unit line below. CRLF (the -G0
# input is not \r-stripped) is ignored when matching and kept when printing.
R1OUT="$(mktemp)"
R1RC=0
awk '
  { s = $0; sub(/\r$/, "", s) }
  s ~ /^[ \t]*\.ent[ \t]/ { fn = s; sub(/^[ \t]*\.ent[ \t]+/, "", fn); sub(/[ \t,].*/, "", fn) }
  s ~ /^[ \t]*\.set[ \t]+noreorder([ \t#]|$)/ { nore = 1 }
  s ~ /^[ \t]*\.set[ \t]+reorder([ \t#]|$)/ { nore = 0 }
  !nore && prev == "#NO_APP" && s ~ /^\tj(r)?\t\$31[ \t]*$/ {
    print "\t# asm_unit.sh noapp-slot pin (RULING #10043) " fn
    print "\t.set\tnoreorder"; print; print "\tnop"; print "\t.set\treorder"
    prev = s; next
  }
  { print; if (s !~ /^[ \t]*$/) { t = s; sub(/^[ \t]+/, "", t); sub(/[ \t]+$/, "", t); prev = t } }
' "$DLIOUT" > "$R1OUT" || R1RC=$?
[ "$R1RC" = 0 ] || { rm -f "$R1OUT"; dli_fail "(b): the RULING #10043 noapp-slot pass exited $R1RC"; }
NR1=$(($(wc -l < "$R1OUT")))
[ "$NR1" -ge "$NOUT" ] || { rm -f "$R1OUT"; dli_fail "(c): the RULING #10043 noapp-slot pass printed $NR1 line(s) for $NOUT in"; }
# the per-unit line names the unit (the gap #1982 found in #9966's la-slot
# line) and lists one function per pin, so the count and the members agree
R1UNIT=$(printf '%s\n' "$UNIT_S" | sed -E "s#^.*/src/$REGION/##; s#(\._u)?\.s\$##")
R1PINS=$(awk '/^\t# asm_unit\.sh noapp-slot pin / { n++ } END { print n + 0 }' "$R1OUT")
R1FNS=$(awk '/^\t# asm_unit\.sh noapp-slot pin / { printf " %s", $NF }' "$R1OUT")
mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL "$GFLAG" -I. -o "$OUT_O" - < "$R1OUT"
echo "asm_unit.sh: dli: $DLIN transforms ($DLIM allowlist rows for $REGION)" >&2
echo "asm_unit.sh: la-slot: $LAPINS pins" >&2
echo "asm_unit.sh: noapp-slot: $R1PINS pins in $R1UNIT${R1FNS:+:$R1FNS}" >&2
rm -f "$DLIIN" "$DLIOUT" "$R1OUT"; rm -rf "$DLITMP"

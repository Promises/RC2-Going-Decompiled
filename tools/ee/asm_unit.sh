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
#     the rest of the seed is live). Inline asm reaches the unit .s verbatim
#     and outside cc1 layout, but in the image it is only lq/sq/la/cvt.w.s and
#     -G0 syscall shims (every USA/EU image, objdiff base and base96 .s, #1103).
# Exit 2 and the `asm_unit.sh: FAIL:` prefix are deliberately NOT the delay-slot
# guard's `REFUSED` (rc 1 from `.error`): a control that counts WARNING/REFUSED
# lines must not read this refusal as its seed firing.
layout_fail() {
  echo "asm_unit.sh: FAIL: $1" >&2
  echo "  input: $UNIT_S (-G: $GFLAG)" >&2
  [ -n "${2:-}" ] && printf '%s\n' "$2" | sed 's/^/  dead line: /' >&2
  echo "  No object written. cc1 layout is: TAB, mnemonic, TAB, operands with no" >&2
  echo "  whitespace (e.g. '\\tbnel\\t\$4,\$0,\$L1' then '\\tlw\\t\$3,g_hx')." >&2
  rm -f "$OUT_O"
  exit 2
}
[ -f "$UNIT_S" ] && [ -r "$UNIT_S" ] \
  || layout_fail "input .s missing or unreadable (a path outside the container mount reads as missing here)"
if [ "$GFLAG" = "-G8" ]; then
  LAYOUT="$(tr -d '\r' < "$UNIT_S" | awk '
    BEGIN {
      br = "^(j|jal|jalr|b|beq|bne|beql|bnel|blez|bgez|bgtz|bltz|blezl|bgezl|bgtzl|bltzl|bgezal|bltzal|bc1f|bc1t|bgezall|bltzall|bc1fl|bc1tl)$"
      mem = "^(sw|sh|sb|sd|lw|lh|lhu|lb|lbu|ld)$"
    }
    {
      line = $0
      # an instruction after a label on the same line is never cc1 layout
      lab = sub(/^[ \t]*[A-Za-z0-9_$.]+:[ \t]*/, "", line)
      if (line !~ /^[ \t]*[a-z][a-z0-9.]*([ \t]|$)/) next
      # cc1 appends `# high`-style comments to some lines; they stay live
      if (!lab && line ~ /^\t[a-z][a-z0-9.]*(\t[^ \t#]+)?([ \t]*(#.*)?)$/) { live++; next }
      dead++
      mn = line; sub(/^[ \t]+/, "", mn); ops = mn
      sub(/[ \t].*$/, "", mn); sub(/^[a-z0-9.]+[ \t]*/, "", ops)
      if (mn ~ br || (mn ~ mem && ops ~ /^\$[a-z0-9]+[ \t]*,[ \t]*[A-Za-z_]/)) {
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
                      "$(printf '%s\n' "$LAYOUT" | sed 1d)" ;;
    ALLDEAD*) layout_fail "no instruction line in cc1 layout ($(printf '%s' "$LAYOUT" | sed 's/^ALLDEAD //') in another layout); no -G8 rule can match this input" ;;
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
# so we insert the nop ourselves. Not applied at -G0: no currently-matched
# -G0 function has a cop1-load/noreorder boundary, and the -G0 units' matches
# were proven WITHOUT the pad.
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
# (cc1 is a Win32 PE - its .s lines end in CRLF, hence the \r-stripping.)
if [ "$GFLAG" = "-G8" ]; then
  sed -E -f "$MOVEFIX" "$UNIT_S" | tr -d '\r' | awk '
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
        if ($0 ~ /^\t(sw|sh|sb|sd|lw|lh|lhu|lb|lbu|ld)\t\$[a-z0-9]+,[A-Za-z_][A-Za-z0-9_]*(\+[0-9]+)?$/) {
          mfull = $0; sub(/^\t[a-z]+\t\$[a-z0-9]+,/, "", mfull)
          msym = mfull; sub(/\+[0-9]+$/, "", msym)
        }
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
    /^\t(bgezall|bltzall|bc1fl|bc1tl)\t/ { if (nore && pendmov == "") { pendbr = $0; next } }
    /^\t(j|jal|jalr|b|beq|bne|beql|bnel|blez|bgez|bgtz|bltz|blezl|bgezl|bgtzl|bltzl|bgezal|bltzal|bc1f|bc1t)\t/ {
      if (nore && pendmov == "") { pendbr = $0; next }
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
    # from the SN ee-as; GNU as treats the r5900 as interlocked and emits
    # nothing. Track the last mtc1 destination and pad when the very next
    # instruction is a dependent FPU op. (mtc1 followed by a non-FPU insn or
    # an independent FPU op is NOT padded - proven by the same functions.)
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
      if ((base in sz) && sz[base]<=8)
        printf "\taddiu\t%s,$gp,%%gp_rel(%s)\n", r, sym
      else {
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
  ' "$UNIT_S" - \
    | mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL "$GFLAG" -I. -o "$OUT_O" -
else
  sed -E -f "$MOVEFIX" "$UNIT_S" \
    | mips-linux-gnu-as -march=r5900 -mabi=eabi -no-pad-sections -EL "$GFLAG" -I. -o "$OUT_O" -
fi

#!/usr/bin/env bash
# verify_match_unit.sh — RAW byte+reloc gate that scopes ONE function out of a
# WHOLE-UNIT object. diff96.sh's base.o is the whole compiled unit (the MATCH
# function as real cc1 code, every sibling INCLUDE_ASM'd) built through
# asm_unit.sh's full SN-parity fixup pipeline. This slices the target function's
# `objdump -dr` block from both the whole-unit base.o and the single-function
# target.o, normalizes away instruction addresses AND reloc offsets (position-
# dependent, differ because the function sits at a unit offset in base.o but at 0
# in target.o), and diffs the remaining instruction WORDS + R_MIPS_* reloc
# type/symbol lines. Same authority as verify_match.sh (NEVER objdiff fuzzy%),
# just unit-scoped.
#
#   verify_match_unit.sh <func> <whole_unit_base.o> <single_func_target.o>
# Exit 0 + "BYTE+RELOC IDENTICAL" = confirmed engine-2.96 match.
set -u
FN="${1:?func}"; BASE="${2:?base.o (whole-unit, your compiled)}"; TGT="${3:?tgt.o (original single-func)}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"  # script-relative = worktree-portable
docker --context colima-ee-x86 run --rm -v "$ROOT":/work ee-build sh -c "
  cd /work
  slice() {  # \$1=.o  \$2=func-name(empty=whole file)
    # Compare instruction ENCODING WORDS (= exact bytes, position-invariant:
    # branch/jump relative offsets encode identically regardless of the fn's
    # placement in the unit) + R_MIPS_* reloc type/symbol lines. Drop the
    # disassembled operand text (objdump renders branch targets as absolute
    # addresses that differ func-relative vs unit-relative) and pure-nop words
    # (trailing alignment padding), matching verify_match.sh's nop handling.
    mips-linux-gnu-objdump -dr \"\$1\" 2>/dev/null | awk -v f=\"\$2\" '
      /^[0-9a-f]+ <.*>:/ { n=\$2; gsub(/[<>:]/,\"\",n); emit=(f==\"\"||n==f); next }
      emit { print }
    ' | sed -E 's/^ *[0-9a-f]+:\t//; s/^[ \t]+[0-9a-f]+: /RELOC /; s/^([0-9a-f]{8}) .*/\1/' \
      | grep -vE 'file format|Disassembly|^\$|^00000000\$'
  }
  slice '$TGT' ''    > /tmp/vm_t.txt
  slice '$BASE' '$FN' > /tmp/vm_b.txt
  if [ ! -s /tmp/vm_b.txt ]; then echo '$FN: NOT FOUND in base.o (still INCLUDE_ASM?)'; exit 2; fi
  if diff -q /tmp/vm_t.txt /tmp/vm_b.txt >/dev/null; then
    echo '$FN: BYTE+RELOC IDENTICAL ✅ (relocs:'\$(grep -c RELOC /tmp/vm_b.txt)')'
  else
    echo '$FN: DIFFERS ❌ — NOT a confirmed match'; diff /tmp/vm_t.txt /tmp/vm_b.txt | head -40; exit 1
  fi"

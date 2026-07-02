#!/usr/bin/env python3
# engine_swap_fix.py — reproduce R&C2's commutative-operand order on ee-gcc 2.96
# (001003) assembly output. ENGINE-2.96 PIPELINE ONLY (post-cc1, pre-assemble) ->
# cannot touch the 2.9 SDK matches (different compiler).
#
# THE RULE (PRODUCER-CLASS precedence, empirically derived — see
# docs/engine_2.96_cc1_patch.md + project_cc1_subbuild_lead): commutative operand
# order is gcc's RTL CANONICALIZATION, not source order (writing a+b vs b+a folds
# identically) and not recency (50/50 in the corpus). R&C2 ranks a commutative op's
# two register operands by the *class of the instruction that produced each* and
# puts the higher-class operand first (rs):
#     mflo/mfhi (mult/div result) > load > other > shift
# (corpus leanings: muldiv->rs 85%, load->rs 63%, shift ~51%/neutral). 001003 uses a
# different comparison, hence the swap. We track each register's producer within the
# basic block and, for a commutative `op rd,rs,rt`, swap rs<->rt when the rt operand
# was produced by a higher-class op than the rs operand. STATISTICAL (~85% on the
# strong class), so objdiff-GATE every function: keep the swap only if it lands 100%.
# Replaces the earlier broken guess-direction ("derived->rt") rule (no-op 3/4,
# wrong-dir 1/4 on the validation sample); this rule gets both func_002A0480 and
# func_002A8688 right.
import sys, re

COMM = {'addu','add','and','or','xor','nor','daddu','dadd'}   # commutative; NOT subu
# producer classes -> precedence (higher goes to rs)
MULDIV = {'mflo','mfhi'}
LOAD   = {'lw','lbu','lhu','lh','lb','lwu','ld','ldl','ldr','lwl','lwr'}
SHIFT  = {'sll','srl','sra','sllv','srlv','srav',
          'dsll','dsrl','dsra','dsll32','dsrl32','dsra32'}
# ops that do NOT write a GPR (must not be recorded as a register's producer)
NOWRITE = {'sw','sd','sb','sh','swl','swr','sdl','sdr','swc1','sdc1','cache',
           'beq','bne','beqz','bnez','bgez','bltz','blez','bgtz','j','jr','jal','jalr',
           'bc1f','bc1t','b'}

def prec(op):
    if op in MULDIV: return 3
    if op in LOAD:   return 2
    if op in SHIFT:  return 0
    return 1   # other / unknown / cross-block (None)

# dest = first operand of any instruction (reg). Handles imm/mem trailing operands.
DEST  = re.compile(r'^\s*(\w[\w.]*)\s+(\$\w+)\b')
# the 3-register commutative form we may rewrite.
COMM3 = re.compile(r'^(\s*)(\w[\w.]*)\s+(\$\w+)\s*,\s*(\$\w+)\s*,\s*(\$\w+)\s*$')
BRANCH = re.compile(r'^\s*(b|beq|bne|bgez|bltz|blez|bgtz|j|jr|jal|jalr|bc1)\w*\b')

def fix(lines):
    out=[]; producers={}; seen_insn=False
    for l in lines:
        body=l.rstrip('\n'); s=body.strip()
        # Strip INTERNAL code alignment: R&C2's engine compiler did not align branch
        # targets, but 2.96 emits `.p2align 3,,7` before $L labels -> the assembler
        # pads a nop. Drop these once the body has started; keep the leading align.
        if seen_insn and re.match(r'^\s*\.(p2align|align)\b', body):
            continue
        if re.match(r'^\s*\.ent\b', body):  # next function -> re-enable leading align
            seen_insn=False
        # basic-block boundary (label or branch/jump) resets producer tracking
        if s.endswith(':') or BRANCH.match(body):
            out.append(l); producers={}; continue
        m3=COMM3.match(body)
        if m3:
            ind,op,rd,rs,rt=m3.groups()
            if op in COMM and rs!=rt and prec(producers.get(rt)) > prec(producers.get(rs)):
                out.append(f"{ind}{op}\t{rd},{rt},{rs}\n")
            else:
                out.append(l)
            producers[rd]=op; seen_insn=True
            continue
        d=DEST.match(body)
        if d:
            op,rd=d.groups()
            out.append(l)
            if op not in NOWRITE:
                producers[rd]=op
            seen_insn=True
        else:
            out.append(l)   # directive/blank/comment: keep tracking across it
    return out

def main():
    if len(sys.argv)>1:
        p=sys.argv[1]
        lines=open(p).readlines()
        res=fix(lines)
        open(p,'w').writelines(res)
    else:
        sys.stdout.writelines(fix(sys.stdin.readlines()))

if __name__=='__main__': main()

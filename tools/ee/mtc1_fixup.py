#!/usr/bin/env python3
# mtc1_fixup.py — restore the R5900 COP1 move-to-use hazard nop the 001003 cc1 leaves
# COMMENTED. After `mtc1 $gpr,$fN` the FPR isn't readable for 1 cycle; the cc1 reserves
# the slot but emits it as `#nop` (commented, unfilled under -fno-schedule-insns), while
# the original asm (SN's assembler) has a real `nop` there. Missing nop -> branch offsets
# shift -> byte-match fails (d2's func_0034A7F8 codegen-gap find). This post-pass uncomments
# the `#nop` slot when it sits between an `mtc1 $fN` and the next real op that reads $fN
# (the genuine hazard); if no #nop slot exists it inserts one. Assembler-compat, same class
# as move_fixup.sed. Idempotent. Runs on the HOST (cpp/cc1 in the VM, this on host).
#   python3 mtc1_fixup.py <file.s>   (in place)
import sys, re
p = sys.argv[1]
lines = open(p).read().split('\n')
# GPR->FPR moves that incur the COP1 move-to-use hazard. mtc1 writes the 2nd operand;
# li.s (float-immediate pseudo -> lui;mtc1) writes the 1st operand.
MTC1 = re.compile(r'^(\s*)mtc1\s+\$[A-Za-z0-9]+\s*,\s*\$(f\d+)\b')
LIS  = re.compile(r'^(\s*)li\.s\s+\$(f\d+)\b')
def fpr_write(c):
    m = MTC1.match(c) or LIS.match(c)
    return (m.group(1), m.group(2)) if m else None
def code(s): return s.split('#', 1)[0]
def is_real(s):                       # a real instruction line (not blank/comment/.set/label)
    c = code(s).strip()
    return c != '' and not c.startswith('.') and not c.endswith(':')
def is_cnop(s): return re.match(r'^\s*#\s*nop\b', s) is not None

i = 0
while i < len(lines):
    w = fpr_write(code(lines[i]))
    if w:
        indent, fpr = w
        # walk forward: optional #nop slot(s) / blanks, to the first real instruction
        slot = None; j = i + 1
        while j < len(lines):
            if is_cnop(lines[j]): slot = j; j += 1; continue
            if lines[j].strip() == '': j += 1; continue
            if is_real(lines[j]): break
            j += 1
        if j < len(lines) and re.search(r'\$' + fpr + r'\b', code(lines[j])):
            # genuine hazard: ensure a real nop sits in the slot
            if slot is not None:
                lines[slot] = indent + 'nop'          # uncomment the reserved slot
            elif not re.match(r'^\s*nop\b', code(lines[i+1] if i+1 < len(lines) else '')):
                lines.insert(i + 1, indent + 'nop')   # no slot -> insert
    i += 1
open(p, 'w').write('\n'.join(lines))

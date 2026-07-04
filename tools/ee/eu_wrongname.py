#!/usr/bin/env python3
"""eu_wrongname.py — EU +0x80 data-lane wrong-name gate for cod/015180.

The EU build places its cod/015180 SDK *data* globals at USA_addr + 0x80.
EU C bodies were spliced region-agnostically from USA and therefore reference
the USA/unshifted D_<addr> name where the EU original asm (ground truth, as
disassembled+symbol-resolved by splat into the nonmatchings .s) references the
+0x80 name.  Words are identical; only the reloc target symbol is -0x80 wrong.
objdiff's fuzzy% normalises the reloc and HIDES this (see
project_eu_plus80_data_lane) — so this is a dedicated raw gate.

For every function that has a REAL C body (i.e. is NOT INCLUDE_ASM'd in the .c)
this compares the D_ symbols referenced by the C body against the D_ symbols
referenced by that function's ground-truth .s:

  wrong-name  <=>  C references D_<X>, .s does NOT, but .s DOES reference D_<X+0x80>

Exit non-zero (and list them) if any wrong names remain — intended as a
mandatory EU-promotion gate.
"""
import os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
C_FILE  = os.path.join(ROOT, "going-decompiled/src/eu/cod/015180.c")
ASM_DIR = os.path.join(ROOT, "going-decompiled/asm/eu/nonmatchings/cod/015180")

D_RE = re.compile(r"\bD_([0-9A-Fa-f]{6,8})\b")

def d_refs(text):
    return set(int(m, 16) for m in D_RE.findall(text))

def load_c():
    with open(C_FILE) as f:
        src = f.read()
    include_asm = set(re.findall(r"INCLUDE_ASM\([^,]+,\s*func_([0-9a-fA-F]+)\)", src))
    # split into top-level function bodies by brace matching
    bodies = {}   # func_addr -> body text (incl signature)
    def_re = re.compile(r"(?:^|\n)[ \t]*[A-Za-z_][\w\s\*]*?\bfunc_([0-9a-fA-F]+)\s*\([^;{]*\)\s*\{")
    for m in def_re.finditer(src):
        addr = m.group(1)
        i = src.index("{", m.start())
        depth = 0
        j = i
        while j < len(src):
            c = src[j]
            if c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    break
            j += 1
        bodies[addr] = src[m.start():j+1]
    return include_asm, bodies

def s_refs(addr):
    p = os.path.join(ASM_DIR, "func_%s.s" % addr)
    if not os.path.exists(p):
        return None
    with open(p) as f:
        return d_refs(f.read())

def main():
    include_asm, bodies = load_c()
    wrong = {}      # func -> [(old_addr, new_addr)]
    anomalies = {}  # func -> [addr] (referenced in C, absent from .s, no +0x80 twin)
    for addr, body in sorted(bodies.items()):
        if addr in include_asm:
            continue
        s = s_refs(addr)
        if s is None:
            continue
        c = d_refs(body)
        for cd in sorted(c):
            if cd in s:
                continue
            if (cd + 0x80) in s:
                wrong.setdefault(addr, []).append((cd, cd + 0x80))
            else:
                anomalies.setdefault(addr, []).append(cd)
    for addr in sorted(wrong):
        for old, new in wrong[addr]:
            print("func_%s  D_%08X -> D_%08X" % (addr, old, new))
    nfns = len(wrong)
    nswaps = sum(len(v) for v in wrong.values())
    print("---")
    print("WRONG-NAME functions: %d   (swaps: %d)" % (nfns, nswaps))
    if anomalies:
        print("ANOMALIES (C D_ ref absent from .s and no +0x80 twin):")
        for addr in sorted(anomalies):
            print("  func_%s: %s" % (addr, ", ".join("D_%08X" % a for a in anomalies[addr])))
    sys.exit(1 if nfns else 0)

if __name__ == "__main__":
    main()

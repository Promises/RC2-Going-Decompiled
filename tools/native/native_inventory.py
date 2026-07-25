#!/usr/bin/env python3
"""native_inventory.py - honest "how close is the native target" gauge.

Counts bare INCLUDE_ASM functions (USA text units) that have NEITHER a byte-match
NOR a TARGET_NATIVE #else body, and splits them into:

  correctly-bare (stay INCLUDE_ASM forever, NOT a coverage gap):
    - FRAGMENT : splat over-split stub (no real `addiu $sp,-N` prologue; lone
                 epilogue/`jr`/store tails) - not a callable function.
    - HLE      : hardware / VU0 glue (lq/sq 128-bit VIF-DMA packets, COP2/VU0
                 vector ops, or 0x10xx-0x13xx MMIO lui) - no portable-C semantics.

  genuinely-#else-able portable logic (the REAL native gap = (b)):
    - MATCH-CAND : 0-1 GPR callee-save, no switch -> attempt byte-match FIRST.
    - WALLED     : 2+ GPR saves (8-byte-pack wall) -> typed #else.

The (b) total is the honest "remaining native portable-#else work" number.
Run from repo root:  .venv-decomp/bin/python tools/native/native_inventory.py
"""
import os, re, glob, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from guard_blocks import walled_and_bodied

SRC = "going-decompiled/src/usa/text"
ASM = "going-decompiled/asm/usa/nonmatchings/text"

inc_re   = re.compile(r'INCLUDE_ASM\([^,]*,\s*(\w+)\)')
save_re  = re.compile(r'\b(sd|sw|sq)\s+\$(1[6-9]|2[0-3]|30|s[0-7]|fp)\b')
neg_prol = re.compile(r'addiu\s+\$(sp|29),\s*\$(sp|29),\s*-')   # frame setup (prologue)
pos_prol = re.compile(r'addiu\s+\$(sp|29),\s*\$(sp|29),\s*0x')  # frame teardown (epilogue)
jr_ra_re = re.compile(r'\bjr\s+\$(31|ra)\b')                    # genuine function return
# hardware/VU0 signals -> HLE (correctly bare)
hw_re    = re.compile(r'\b(lq|sq|qmfc2|qmtc2|cfc2|ctc2|vcallms|vcallmsr|'
                      r'v(mul|add|sub|madd|msub|move|opmula|opmsub|div|sqrt|rsqrt|'
                      r'mula|adda|suba|ftoi[0-9]*|itof[0-9]*|clip|mr32|nop|waitq|'
                      r'max|mini|abs|maxx|minii|maxi))\b')
mmio_re  = re.compile(r'lui\s+\$\w+,\s*0x1[0-3][0-9a-fA-F][0-9a-fA-F]\b')

def bare_funcs(cfile):
    """INCLUDE_ASM funcs with NO portable #else body in the same guard.

    Delegates to the shared guard scanner. The previous version tested
    `startswith('#ifndef TARGET_NATIVE')`, which is 1 of the 5 opener spellings
    in use, and had no `#else` handling at all - so under `#ifdef TARGET_NATIVE`
    (109 blocks, body in the FIRST arm and INCLUDE_ASM in the second) it read the
    guard inside out and reported the function as BARE.

    MEASURED: 17 USA functions were reported bare while having a portable body.
    That direction is the dangerous one - it inflates work REMAINING, so the
    error can never surface as a failure and nobody re-opens it.
    """
    walled, _bodied = walled_and_bodied(open(cfile, errors='replace').read())
    return sorted(walled)

def classify(unit, func):
    s = os.path.join(ASM, unit, func + ".s")
    if not os.path.exists(s):
        return "no_s"
    txt = open(s, errors='replace').read()
    # FRAGMENT = splat over-split stub, NOT a standalone callable function:
    #   - no genuine return (`jr $ra`/`jr $31`), OR
    #   - epilogue/teardown sliver: a positive `addiu $sp,+N` with NO matching
    #     negative prologue (the tail of the previous function, mis-split off).
    # (A real framed fn has both -N prologue and +N epilogue; a real leaf has a
    #  jr $ra and neither.) These are uncallable - they are not a coverage gap.
    if not jr_ra_re.search(txt):
        return "fragment"
    if pos_prol.search(txt) and not neg_prol.search(txt):
        return "fragment"
    if hw_re.search(txt) or mmio_re.search(txt):
        return "hle"                 # hardware / VU0 glue
    saves = len(set(save_re.findall(txt)))
    return "match_cand" if saves <= 1 else "walled"

KEYS = ["match_cand", "walled", "hle", "fragment", "no_s"]
tot = {k: 0 for k in KEYS}
rows = []
for cfile in sorted(glob.glob(os.path.join(SRC, "*.c"))):
    unit = os.path.basename(cfile)[:-2]
    c = {k: 0 for k in KEYS}
    for f in bare_funcs(cfile):
        k = classify(unit, f); c[k] += 1; tot[k] += 1
    b = c["match_cand"] + c["walled"]
    if b or c["hle"] or c["fragment"]:
        rows.append((b, unit, c))

rows.sort(reverse=True)
print(f"{'unit':14}{'#else-able(b)':>14}{'match':>7}{'walled':>7}{'HLE':>6}{'frag':>6}")
for b, unit, c in rows:
    print(f"{unit:14}{b:>14}{c['match_cand']:>7}{c['walled']:>7}{c['hle']:>6}{c['fragment']:>6}")
print("-" * 54)
correctly_bare = tot["hle"] + tot["fragment"]
b_total = tot["match_cand"] + tot["walled"]
print(f"(a) CORRECTLY-BARE (stay INCLUDE_ASM): {correctly_bare}  "
      f"(HLE={tot['hle']} + fragments={tot['fragment']})")
print(f"(b) GENUINELY-#ELSE-ABLE portable logic: {b_total}  "
      f"(match-candidates={tot['match_cand']} + walled={tot['walled']})")
print(f"==> Honest native gap = (b) = {b_total} functions still needing a body "
      f"(match-first, then typed #else).")

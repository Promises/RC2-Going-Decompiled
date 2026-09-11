#!/usr/bin/env python3
"""fragment_census.py - classify the bare INCLUDE_ASM leaves that are NOT functions.

The USA `.text` progress figures count every bare `INCLUDE_ASM` site as one
unmatched function. A large share of the small ones are not functions at all:
splat/spimdisasm pinned a symbol on an orphaned epilogue tail, an inter-function
fill word, or a dead mid-function remnant. They carry bytes the image needs (the
build reproduces the ROM through them) but they can never be matched from C, so
counting them as "remaining work" inflates the denominator.

This tool classifies every bare leaf by the CONTENT of its `.s` (never by name,
size or comment), so the classes can be re-derived and the tree can be linted.

Classes (a leaf gets exactly one):
  PAD_ZERO     every word is 0x00000000                        -> debris
  PAD_CDFILL   every word is 0xCDCDCDCD or 0x00000000          -> debris (SDK debug fill)
  SP_TAIL      first insn is a POSITIVE `addiu $29,$29,+N`     -> debris (orphaned epilogue)
  UNDECODED    any `.word` that is not a pad value             -> not classified (splat emitted data)
  NO_RET       no `jr $31`, no `j`, no `jal`, no `syscall`     -> debris (dead remnant)
  SDK_SYSCALL  contains `syscall`                              -> real, handwritten SDK stub
  JR_NO_SLOT   a `jr $31` that is the LAST word of the body    -> debris (its delay slot is not its own)
  EPI_RET      has `jr $31` but restores from a frame it never built -> debris (epilogue reached by branch)
  REAL         everything else                                  -> a genuine function

DEBRIS = PAD_ZERO, PAD_CDFILL, SP_TAIL, NO_RET, JR_NO_SLOT, EPI_RET. Those sites belong under
`INCLUDE_ASM_FRAGMENT(...)` (include/include_asm.h) instead of `INCLUDE_ASM(...)`.
The two macros expand identically, so the marker is byte-neutral by construction;
its only effect is on counting tools, which match the `INCLUDE_ASM(` token.

Usage (from the repo root):
  tools/ee/fragment_census.py [--region usa] [--max-bytes 16]          census TSV to stdout
  tools/ee/fragment_census.py --lint                                    exit 1 on any mismatch

--lint checks BOTH directions and can therefore fail:
  * an INCLUDE_ASM_FRAGMENT site whose .s is NOT debris   (a real function marked as debris)
  * a plain INCLUDE_ASM site <= --max-bytes whose .s IS debris (unmarked debris)
Bare-vs-portable is decided by tools/guard_blocks.py, the shared guard scanner.
"""
import argparse, glob, os, re, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from guard_blocks import walled_and_bodied  # noqa: E402

RE_SITE = re.compile(r'\b(INCLUDE_ASM|INCLUDE_ASM_FRAGMENT)\s*\(\s*"([^"]+)"\s*,\s*([A-Za-z_]\w*)\s*\)')
RE_SIZE = re.compile(r'^nonmatching\s+(\S+),\s*0x([0-9A-Fa-f]+)', re.M)
RE_INSN = re.compile(r'^\s*/\* [0-9A-Fa-f]+ [0-9A-Fa-f]+ ([0-9A-Fa-f]{8}) \*/\s+(\S+)\s*(.*?)\s*$')
DEBRIS = ('PAD_ZERO', 'PAD_CDFILL', 'SP_TAIL', 'NO_RET', 'JR_NO_SLOT', 'EPI_RET')


def read_body(spath, name):
    """(size_bytes, [(word_hex, mnemonic, operands)]) for the glabel..endlabel of NAME."""
    size = -1
    insns = []
    inside = False
    with open(spath, errors='replace') as fh:
        for line in fh:
            m = RE_SIZE.match(line)
            if m and m.group(1) == name:
                size = int(m.group(2), 16)
            if line.startswith('glabel ' + name):
                inside = True
                continue
            if line.startswith('endlabel'):
                inside = False
                continue
            if inside:
                m = RE_INSN.match(line)
                if m:
                    insns.append((m.group(1).upper(), m.group(2), m.group(3).replace(' ', '')))
    return size, insns


def sp_positive(mn, ops):
    if mn != 'addiu' or not ops.startswith('$29,$29,'):
        return False
    return not ops.split(',')[2].startswith('-')


def classify(insns):
    words = [w for w, _, _ in insns]
    if words and all(w == '00000000' for w in words):
        return 'PAD_ZERO'
    if words and all(w in ('CDCDCDCD', '00000000') for w in words):
        return 'PAD_CDFILL'
    if insns and sp_positive(insns[0][1], insns[0][2]):
        return 'SP_TAIL'
    if any(mn == '.word' for _, mn, _ in insns):
        return 'UNDECODED'
    if any(mn == 'syscall' for _, mn, _ in insns):
        return 'SDK_SYSCALL'
    has_jr = any(mn == 'jr' and ops == '$31' for _, mn, ops in insns)
    has_jump = any(mn in ('j', 'jal') for _, mn, _ in insns)
    if not has_jr and not has_jump:
        return 'NO_RET'
    if insns[-1][1] == 'jr' and insns[-1][2] == '$31':
        return 'JR_NO_SLOT'
    restores = any((mn in ('lw', 'ld', 'lq', 'lwc1', 'ldc1') and ops.endswith('($29)')) or sp_positive(mn, ops)
                   for _, mn, ops in insns)
    builds = any(mn == 'addiu' and ops.startswith('$29,$29,-') for _, mn, ops in insns)
    if has_jr and restores and not builds:
        return 'EPI_RET'
    return 'REAL'


def sites(region):
    """Yield (unit, cfile, lineno, macro, folder, name, bare) for every asm site."""
    src = os.path.join('going-decompiled', 'src', region)
    for cfile in sorted(glob.glob(os.path.join(src, '*', '*.c'))):
        text = open(cfile, errors='replace').read()
        # guard_blocks only knows the INCLUDE_ASM token; feed it a view where the
        # fragment marker is spelled as the plain macro so bare-vs-portable is
        # decided the same way for both.
        walled, _ = walled_and_bodied(text.replace('INCLUDE_ASM_FRAGMENT(', 'INCLUDE_ASM('))
        unit = os.path.relpath(cfile, src)[:-2]
        for ln, line in enumerate(text.splitlines(), 1):
            m = RE_SITE.search(line)
            if m:
                yield unit, cfile, ln, m.group(1), m.group(2), m.group(3), m.group(3) in walled


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--region', default='usa')
    ap.add_argument('--max-bytes', type=int, default=16, help='size ceiling of the class (default 16)')
    ap.add_argument('--all-sizes', action='store_true', help='ignore --max-bytes')
    ap.add_argument('--lint', action='store_true')
    a = ap.parse_args()
    ceiling = None if a.all_sizes else a.max_bytes

    rows = []
    for unit, cfile, ln, macro, folder, name, bare in sites(a.region):
        spath = os.path.join(folder, name + '.s')
        if not os.path.exists(spath):
            rows.append((unit, name, macro, 'NO_S', -1, bare, cfile, ln, ''))
            continue
        size, insns = read_body(spath, name)
        cls = classify(insns) if bare else 'PORTABLE'
        body = ' ; '.join(f'{mn} {ops}'.strip() for _, mn, ops in insns)
        rows.append((unit, name, macro, cls, size, bare, cfile, ln, body))

    if not a.lint:
        print('\t'.join(['unit', 'name', 'macro', 'class', 'size', 'bare', 'cfile', 'line', 'body']))
        for r in rows:
            if not r[5]:
                continue
            if ceiling is not None and (r[4] < 0 or r[4] > ceiling):
                continue
            print('\t'.join(str(x) for x in r))
        return 0

    bad = 0
    for unit, name, macro, cls, size, bare, cfile, ln, body in rows:
        if macro == 'INCLUDE_ASM_FRAGMENT' and cls not in DEBRIS:
            print(f'FAIL {cfile}:{ln} {name} is marked INCLUDE_ASM_FRAGMENT but classifies {cls}: {body}')
            bad += 1
        elif macro == 'INCLUDE_ASM' and bare and cls in DEBRIS and (ceiling is None or 0 <= size <= ceiling):
            print(f'FAIL {cfile}:{ln} {name} ({size} B) is unmarked debris ({cls}): {body}')
            bad += 1
    print(f'lint: {bad} finding(s) over {sum(1 for r in rows if r[5])} bare sites')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())

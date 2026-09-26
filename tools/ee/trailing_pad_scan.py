#!/usr/bin/env python3
"""Static trailing-pad screen (FACT #7982).

For a git rev range BASE..TIP, find every function whose INCLUDE_ASM line is
REMOVED from a going-decompiled/src/usa .c (i.e. no longer in the 2.9/build.sh
link), read its nonmatchings .s at TIP, and report the words that sit AFTER
`endlabel` at an address >= roundup8(end of body). `.align 3` (every .s and
every cc1 function starts with it) regenerates words below that point, so only
words at/after it are LOST when the .s stops being included.

Also reports whether TIP's copy of the .c carries a `.word` asm directive
(the cod/015180.c:5910 compensation construct).

usage: padscan.py BASE TIP        (branch range)
       padscan.py --tree REV      (every dead .s under a usa unit at REV)
"""
import re, subprocess, sys

ROW = re.compile(r'/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s*\*/')
INC = re.compile(r'^\s*INCLUDE_ASM(?:_FRAGMENT)?\(\s*"([^"]+)"\s*,\s*(\w+)\s*\)', re.M)


def git(*a):
    return subprocess.run(['git', *a], capture_output=True, text=True).stdout


def show(rev, path):
    r = subprocess.run(['git', 'show', f'{rev}:{path}'], capture_output=True, text=True)
    return r.stdout if r.returncode == 0 else None


def includes(text):
    return {(f, n) for f, n in INC.findall(text or '')}


def scan_s(text):
    """-> (fend, [(vaddr, word) post-endlabel], [(vaddr, word) lost])"""
    body, post, seen_end = [], [], False
    for ln in text.splitlines():
        if ln.startswith('endlabel'):
            seen_end = True
            continue
        m = ROW.search(ln)
        if not m:
            continue
        v, w = int(m.group(1), 16), m.group(2)
        (post if seen_end else body).append((v, w))
    if not body or not seen_end:
        return None
    fend = body[-1][0] + 4
    lim = (fend + 7) & ~7
    lost = [(v, w) for v, w in post if v >= lim]
    return fend, post, lost


def report(rev, cpath, folder, name):
    s = show(rev, f'{folder}/{name}.s')
    if s is None:
        return f'  {name}: NO .s at {rev[:8]} ({folder})'
    r = scan_s(s)
    if r is None:
        return f'  {name}: unparsed (no endlabel/body)'
    fend, post, lost = r
    c = show(rev, cpath) or ''
    pads = len(re.findall(r'__asm__\s*\(\s*"\s*\.word', c))
    tag = 'LOST' if lost else 'ok'
    return (f'  {tag:4} {name} fend=0x{fend:08X} post={len(post)} lost={len(lost)} '
            f'{" ".join(f"{v:08X}:{w}" for v, w in lost)} [.word-asm in unit at tip: {pads}] ({cpath})')


def branch(base, tip):
    files = [f for f in git('diff', '--name-only', f'{base}..{tip}').split()
             if f.startswith('going-decompiled/src/usa/') and f.endswith('.c')]
    out = []
    for f in files:
        gone = includes(show(base, f)) - includes(show(tip, f))
        for folder, name in sorted(gone):
            out.append(report(tip, f, folder, name))
    return out


def tree(rev):
    files = [f for f in git('ls-tree', '-r', '--name-only', rev, 'going-decompiled/src/usa').split()
             if f.endswith('.c')]
    out = []
    for f in files:
        c = show(rev, f)
        live = includes(c)
        folders = {fo for fo, _ in live}
        for fo in sorted(folders):
            for p in git('ls-tree', '--name-only', rev, fo + '/').split():
                n = p.rsplit('/', 1)[-1][:-2]
                if p.endswith('.s') and (fo, n) not in live:
                    line = report(rev, f, fo, n)
                    if 'LOST' in line:
                        out.append(line)
    return out


if __name__ == '__main__':
    if sys.argv[1] == '--tree':
        print('\n'.join(tree(sys.argv[2])))
    else:
        print('\n'.join(branch(sys.argv[1], sys.argv[2])))

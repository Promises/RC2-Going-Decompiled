#!/usr/bin/env python3
"""Static trailing-pad screen (FACT #7982).

For a git rev range BASE..TIP, find every function whose INCLUDE_ASM line is
REMOVED from a going-decompiled/src/usa .c (i.e. no longer in the 2.9/build.sh
link), read its nonmatchings .s at TIP, and report the words that sit AFTER
`endlabel` at an address >= roundup8(end of body). `.align 3` (every .s and
every cc1 function starts with it) regenerates words below that point, so only
words at/after it are LOST when the .s stops being included.

Each loss is classified against TIP's copy of the .c:
  FILL   a live INCLUDE_ASM of a filler .s starting at the first lost address
         (06333c5 idiom, e.g. text/1B4218 func_002B58D0);
  WORD   a top-level __asm__(".word ...") right after the C definition carrying
         as many words as were lost (cod/015180.c:5910 idiom); WORD? = count
         differs;
  LOST   neither: every later function in the unit lands low. Exit 1.

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
    if not lost:
        return f'  ok   {name} fend=0x{fend:08X} post={len(post)} lost=0 ({cpath})'
    c = show(rev, cpath) or ''
    words = ' '.join(f'{v:08X}:{w}' for v, w in lost)
    # Compensation 1: a live INCLUDE_ASM of a filler .s whose first word sits at
    # the first lost address (the 06333c5 idiom, e.g. text/1B4218 func_002B58D0).
    for fo, n in includes(c):
        m = ROW.search(show(rev, f'{fo}/{n}.s') or '')
        if m and int(m.group(1), 16) == lost[0][0]:
            return f'  FILL {name} lost={len(lost)} {words} -> filler {n} ({cpath})'
    # Compensation 2: a top-level __asm__(".word ...") after the C definition of
    # NAME and before the next INCLUDE_ASM or top-level definition (the
    # cod/015180.c:5910 idiom).
    lines = c.splitlines()
    defn = next((i for i, l in enumerate(lines)
                 if re.match(r'^[A-Za-z_].*\b%s\s*\(' % re.escape(name), l)
                 and not l.rstrip().endswith(';')), None)
    if defn is not None:
        depth, started = 0, False
        for j in range(defn, len(lines)):
            l = lines[j]
            if started and depth == 0:
                if re.match(r'^\s*__asm__\s*\(\s*"\s*\.word', l):
                    nw = l.count('.word')
                    tag = 'WORD' if nw == len(lost) else 'WORD?'
                    return f'  {tag:5}{name} lost={len(lost)} {words} -> {nw} .word at {cpath}:{j + 1}'
                if re.match(r'^(INCLUDE_ASM|[A-Za-z_][\w\s\*]*\w+\s*\()', l):
                    break
            depth += l.count('{') - l.count('}')
            started = started or '{' in l
    return (f'  LOST {name} fend=0x{fend:08X} post={len(post)} lost={len(lost)} {words} '
            f'UNCOMPENSATED (no filler .s at 0x{lost[0][0]:08X}, no .word after the definition'
            f'{"" if defn is not None else "; definition NOT FOUND"}) ({cpath})')


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
                    if not line.startswith('  ok'):
                        out.append(line)
    return out


if __name__ == '__main__':
    rows = tree(sys.argv[2]) if sys.argv[1] == '--tree' else branch(sys.argv[1], sys.argv[2])
    print('\n'.join(rows))
    sys.exit(1 if any(r.startswith('  LOST') for r in rows) else 0)

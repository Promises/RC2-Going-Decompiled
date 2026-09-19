#!/usr/bin/env python3
"""blanket_orphans.py — D_/func_ tokens that compiled C references and that
only build.sh's asm/ blanket keeps linkable (task #457 ENFORCE-2, FACT #7301;
grown from tools/ee/.t452/12_blanket_orphans.sh on preserve/evidence/t452).

    blanket_orphans.py <root> <region> [-v] [-D MACRO ...]

build.sh defines every `D_X`/`func_X` token it can grep out of asm/<region>
(`(D_|func_)[0-9A-Fa-f]{4,}`, build.sh "ALLSYMS") at the address in its name.
A token that compiled C references is therefore linkable only while SOME .s
under asm/<region> still spells it — and a `nonmatchings/**/func_*.s` is a
file a rename deletes (#452 deleted 278 of them and the USA link lost four
tokens, FACT #7301). One row per referenced token, sorted:

    ORPHAN         <token> <file:line>[+N] holders=none
        no .s under asm/<region> mentions it, no compiled C defines it, and
        symbol_addrs has no line under that exact spelling: the link is
        undefined on it NOW (build.sh's ld.log names it).
    ORPHAN_LATENT  <token> <file:line>[+N] holders=<f1.s>,<f2.s>...
        every .s that mentions it is a nonmatchings/**/func_*.s — INCLUDE_ASM'd
        or not: the write-once split never rewrites either, and a class-1 or
        class-2 rename deletes it. Delete the last holder and the row becomes
        ORPHAN. (The task's literal predicate — "not INCLUDE_ASM'd" only —
        would MISS FACT #7301's four at 8b536f0c: their six Mc* holders were
        INCLUDE_ASM'd there. So every func_*.s counts as a holder that can go.)
    HELD           <token> ...  (only with -v: a holder outside those files,
        a compiled C definition, or a symbol_addrs line — the common case)

A REFERENCE is a token on a line the EE build compiles (preprocessor state
tracked by shadow_cdef_scan.PP: build.sh defines none of TARGET_NATIVE,
NON_MATCHING, MATCH_*), outside comments and strings, that is not an
INCLUDE_ASM(...) argument, not a brace-depth-0 `extern` declaration, and not
a brace-depth-0 definition (a definition marks the token C-DEFINED instead).
Line-based: a declaration or definition split across lines is classified by
its first line.

Failure observable: a member absent from the output. The gate's selftest
deletes one holder in a scratch copy and requires its token to move to ORPHAN;
the real control is 8b536f0c with #452's leftovers removed, which must print
exactly FACT #7301's four as ORPHAN.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from shadow_cdef_scan import PP  # noqa: E402  (the same arm tracker as CLASS2)

TOKEN_RE = re.compile(r'\b(?:D_|func_)[0-9A-Fa-f]{4,}\b')   # == build.sh's blanket grep
INCLUDE_ASM_RE = re.compile(r'INCLUDE_ASM\s*\(')
KEYWORDS = {'return', 'if', 'while', 'for', 'else', 'goto', 'case', 'sizeof', 'switch', 'do'}


def strip_code(s, state):
    """Return the code text of one line with comments and string/char literals
    removed; state['in_block'] carries a /* */ comment across lines."""
    code = ''
    i = 0
    while i < len(s):
        if state['in_block']:
            j = s.find('*/', i)
            if j < 0:
                return code
            state['in_block'] = False
            i = j + 2
            continue
        if s.startswith('/*', i):
            state['in_block'] = True
            i += 2
            continue
        if s.startswith('//', i):
            break
        if s[i] in '"\'':
            q = s[i]
            j = i + 1
            while j < len(s) and s[j] != q:
                if s[j] == '\\':
                    j += 1
                j += 1
            code += ' " '
            i = j + 1
            continue
        code += s[i]
        i += 1
    return code


def scan_c(path, rel, defined, warn, refs, cdefs):
    """Collect compiled-arm token references into refs[token] -> [loc...] and
    compiled-arm definitions into cdefs (a set of tokens)."""
    pp = PP(defined, warn)
    depth = 0
    state = {'in_block': False}
    with open(path, encoding='utf-8', errors='replace') as f:
        for ln, raw in enumerate(f, 1):
            code = strip_code(raw.rstrip('\n'), state)
            if re.match(r'\s*#', code):
                pp.directive(code, '%s:%d' % (rel, ln))
                continue
            toks = TOKEN_RE.findall(code)
            if toks and pp.active() and not INCLUDE_ASM_RE.search(code):
                head = code.lstrip()
                first = re.findall(r'[A-Za-z_]\w*', head)
                is_decl = depth == 0 and head.startswith('extern')
                is_def = depth == 0 and not is_decl and not head.startswith('typedef') \
                    and not (first and first[0] in KEYWORDS) \
                    and re.match(r'[A-Za-z_]', head) is not None
                if is_def:
                    # `type NAME(...) {`, `type NAME[...] = {`, `type NAME;` at depth 0 define
                    # the FIRST token; any later token on the line (an initialiser) is a use.
                    cdefs.add(toks[0])
                    toks = toks[1:]
                elif is_decl:
                    toks = []
                for t in toks:
                    refs.setdefault(t, []).append('%s:%d' % (rel, ln))
            depth += code.count('{') - code.count('}')


def main():
    args = [a for a in sys.argv[1:]]
    if len(args) < 2:
        sys.exit(__doc__)
    root, region = args[0], args[1]
    verbose = '-v' in args
    defined = []
    i = 2
    while i < len(args):
        if args[i] == '-D' and i + 1 < len(args):
            defined.append(args[i + 1])
            i += 2
        elif args[i].startswith('-D'):
            defined.append(args[i][2:])
            i += 1
        else:
            i += 1
    src = os.path.join(root, 'going-decompiled', 'src', region)
    asm = os.path.join(root, 'going-decompiled', 'asm', region)
    sa = os.path.join(root, 'going-decompiled', 'symbol_addrs', region, 'symbol_addrs.txt')
    for d in (src, asm):
        if not os.path.isdir(d):
            sys.exit('blanket_orphans: no %s' % d)
    warn = lambda m: print('WARN ' + m, file=sys.stderr)

    # holders: token -> set of asm files (relative to root) that mention it
    holders = {}
    nonmatch = os.path.join(asm, 'nonmatchings') + os.sep
    for dp, _, fns in os.walk(asm):
        for fn in fns:
            if not fn.endswith('.s'):
                continue
            p = os.path.join(dp, fn)
            with open(p, encoding='utf-8', errors='replace') as f:
                toks = set(TOKEN_RE.findall(f.read()))
            if not toks:
                continue
            deletable = p.startswith(nonmatch) and fn.startswith('func_')
            rel = os.path.relpath(p, root)
            for t in toks:
                holders.setdefault(t, {})[rel] = deletable

    # symbol_addrs lines under the exact token spelling become PROVIDE(token = addr)
    provided = set()
    if os.path.isfile(sa):
        rx = re.compile(r'^\s*((?:D_|func_)[0-9A-Fa-f]{4,})\s*=\s*0x[0-9A-Fa-f]+')
        with open(sa, encoding='utf-8', errors='replace') as f:
            for line in f:
                m = rx.match(line)
                if m:
                    provided.add(m.group(1))

    refs, cdefs = {}, set()
    for dp, _, fns in os.walk(src):
        for fn in sorted(fns):
            if fn.endswith('.c'):
                p = os.path.join(dp, fn)
                scan_c(p, os.path.relpath(p, root), defined, warn, refs, cdefs)

    rows = []
    for t in sorted(refs):
        locs = refs[t]
        where = locs[0] + ('+%d' % (len(locs) - 1) if len(locs) > 1 else '')
        h = holders.get(t, {})
        if t in cdefs or t in provided:
            cls, hold = 'HELD', ('cdef' if t in cdefs else 'symbol_addrs')
        elif not h:
            cls, hold = 'ORPHAN', 'none'
        elif all(h.values()):
            cls, hold = 'ORPHAN_LATENT', ','.join(sorted(os.path.basename(x) for x in h))
        else:
            cls, hold = 'HELD', ','.join(sorted(os.path.basename(x) for x in h if not h[x])[:3])
        if cls != 'HELD' or verbose:
            rows.append((cls, t, where, hold))
    order = {'ORPHAN': 0, 'ORPHAN_LATENT': 1, 'HELD': 2}
    for cls, t, where, hold in sorted(rows, key=lambda r: (order[r[0]], r[1])):
        print('%s %s %s holders=%s' % (cls, t, where, hold))


if __name__ == '__main__':
    main()

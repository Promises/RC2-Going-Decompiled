#!/usr/bin/env python3
"""shadow_cdef_scan.py — class-2 shadow scan: C function DEFINITIONS named
func_XXXXXXXX whose address symbol_addrs names differently (task #448, FACT #7291).

    shadow_cdef_scan.py <root> <region> [--all-arms] [-D MACRO ...]

Walks <root>/going-decompiled/src/<region>/**/*.c, tracks the preprocessor
state the EE build sees (build.sh defines none of TARGET_NATIVE, NON_MATCHING,
MATCH_*; pass -D to change that) and prints one row per definition:

    CLASS2  <file>:<line>  func_ADDR -> Name        (compiled arm)
    CLASS2X <file>:<line>  func_ADDR -> Name        (only with --all-arms: a
            definition the EE build does NOT compile, e.g. the #else arm of an
            INCLUDE_ASM'd function)

A DEFINITION is: at brace depth 0, outside comments and strings, a statement
that contains `func_XXXXXXXX (` and reaches `{` before `;` (a prototype ends in
`;`). INCLUDE_ASM(...) lines cannot match because `"` precedes the name.
Unevaluable `#if` expressions are reported on stderr and assumed compiled.
Row shape on failure: a missing member is an absent row; the seeded control in
shadow_scan2_selftest.sh (a planted `s32 func_00DEAD00(void) {` plus a fake
symbol_addrs line) must print exactly one CLASS2 row for it.
"""
import os
import re
import sys

DEF_RE = re.compile(r'\bfunc_([0-9A-Fa-f]{8})\s*\(')
KEYWORDS = {'return', 'if', 'while', 'for', 'else', 'goto', 'case', 'sizeof', 'switch', 'do'}


def load_symbol_addrs(path):
    table = {}
    rx = re.compile(r'^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;')
    with open(path, encoding='utf-8', errors='replace') as f:
        for line in f:
            m = rx.match(line)
            if m:
                table.setdefault('%08X' % int(m.group(2), 16), m.group(1))
    return table


class PP:
    """Minimal preprocessor state tracker: a stack of (active, taken, parent_active)."""

    def __init__(self, defined, warn):
        self.defined = set(defined)
        self.stack = []
        self.warn = warn

    def active(self):
        return all(a for a, _, _ in self.stack)

    def eval_if(self, expr, where):
        e = expr.strip()
        e = re.sub(r'/\*.*?\*/', ' ', e)
        e = re.sub(r'//.*$', '', e)
        e = re.sub(r'defined\s*\(\s*(\w+)\s*\)', lambda m: 'True' if m.group(1) in self.defined else 'False', e)
        e = re.sub(r'defined\s+(\w+)', lambda m: 'True' if m.group(1) in self.defined else 'False', e)
        e = e.replace('&&', ' and ').replace('||', ' or ')
        e = re.sub(r'!(?!=)', ' not ', e)
        e = re.sub(r'\b(\d+)[uUlL]+\b', r'\1', e)
        # any remaining identifier is an undefined macro -> 0 (C semantics)
        e = re.sub(r'\b(?!(?:and|or|not|True|False)\b)[A-Za-z_]\w*\b', '0', e)
        try:
            return bool(eval(e, {'__builtins__': {}}, {}))
        except Exception:
            self.warn('%s: cannot evaluate #if %s -> assuming TRUE' % (where, expr.strip()))
            return True

    def directive(self, line, where):
        m = re.match(r'\s*#\s*(\w+)\s*(.*)$', line)
        if not m:
            return False
        d, rest = m.group(1), m.group(2)
        parent = self.active()
        if d == 'ifdef':
            c = rest.split()[0] in self.defined if rest.split() else False
            self.stack.append((c, c, parent))
        elif d == 'ifndef':
            c = rest.split()[0] not in self.defined if rest.split() else True
            self.stack.append((c, c, parent))
        elif d == 'if':
            c = self.eval_if(rest, where) if parent else False
            self.stack.append((c, c, parent))
        elif d == 'elif':
            if not self.stack:
                self.warn('%s: #elif without #if' % where)
                return True
            a, taken, p = self.stack.pop()
            c = (not taken) and p and self.eval_if(rest, where)
            self.stack.append((c, taken or c, p))
        elif d == 'else':
            if not self.stack:
                self.warn('%s: #else without #if' % where)
                return True
            a, taken, p = self.stack.pop()
            self.stack.append((not taken and p, True, p))
        elif d == 'endif':
            if self.stack:
                self.stack.pop()
            else:
                self.warn('%s: #endif without #if' % where)
        return True


def scan_file(path, defined, warn):
    """Yield (line, addr, compiled) for every func_XXXXXXXX definition."""
    pp = PP(defined, warn)
    depth = 0
    in_block = False
    pending = None      # (line, addr, compiled) awaiting '{' or ';'
    with open(path, encoding='utf-8', errors='replace') as f:
        lines = f.readlines()
    for ln, raw in enumerate(lines, 1):
        where = '%s:%d' % (path, ln)
        # strip comments to get the code text of this line
        code = ''
        i = 0
        s = raw.rstrip('\n')
        while i < len(s):
            if in_block:
                j = s.find('*/', i)
                if j < 0:
                    i = len(s)
                else:
                    in_block = False
                    i = j + 2
                continue
            if s.startswith('/*', i):
                in_block = True
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
                code += ' " '   # keep a marker so INCLUDE_ASM("..", func_X) stays non-matching
                i = j + 1
                continue
            code += s[i]
            i += 1
        if re.match(r'\s*#', code):
            pp.directive(code, where)
            continue
        if pending is None and depth == 0:
            m = DEF_RE.search(code)
            if m:
                head = code[:m.start()]
                first = re.findall(r'[A-Za-z_]\w*', head)
                ok = '"' not in head and '(' not in head and '=' not in head \
                    and not (first and first[0] in KEYWORDS) \
                    and not head.lstrip().startswith('extern') \
                    and not head.lstrip().startswith('typedef')
                if ok:
                    pending = (ln, m.group(1).upper(), pp.active())
                    # examine the rest of this line for '{' / ';'
                    tail = code[m.end():]
                    r = _resolve(tail)
                    if r is not None:
                        yield pending[0], pending[1], pending[2], r
                        pending = None
                    depth += code.count('{') - code.count('}')
                    continue
        elif pending is not None:
            r = _resolve(code)
            if r is not None:
                yield pending[0], pending[1], pending[2], r
                pending = None
        depth += code.count('{') - code.count('}')


def _resolve(text):
    """True if '{' comes before ';' in text, False if ';' first, None if neither."""
    b, sc = text.find('{'), text.find(';')
    if b < 0 and sc < 0:
        return None
    if b < 0:
        return False
    if sc < 0:
        return True
    return b < sc


def main():
    args = sys.argv[1:]
    if len(args) < 2:
        sys.exit(__doc__)
    root, region = args[0], args[1]
    all_arms = '--all-arms' in args
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
    sa = os.path.join(root, 'going-decompiled', 'symbol_addrs', region, 'symbol_addrs.txt')
    src = os.path.join(root, 'going-decompiled', 'src', region)
    table = load_symbol_addrs(sa)
    warn = lambda m: print('WARN ' + m, file=sys.stderr)
    rows = []
    for dp, _, fns in os.walk(src):
        for fn in sorted(fns):
            if not fn.endswith(('.c', '.cpp')):  # .cpp: a converted unit (#1258)
                continue
            p = os.path.join(dp, fn)
            rel = os.path.relpath(p, root)
            for ln, addr, compiled, is_def in scan_file(p, defined, warn):
                if not is_def:
                    continue
                new = table.get(addr)
                old = 'func_' + addr
                if new is None or new == old:
                    continue
                if compiled:
                    rows.append(('CLASS2', rel, ln, old, new))
                elif all_arms:
                    rows.append(('CLASS2X', rel, ln, old, new))
    for cls, rel, ln, old, new in sorted(rows, key=lambda r: (r[0], r[1], r[2])):
        print('%s %s:%d %s -> %s' % (cls, rel, ln, old, new))


if __name__ == '__main__':
    main()

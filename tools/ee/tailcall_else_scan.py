#!/usr/bin/env python3
"""
tailcall_else_scan.py — read-only scan for the C-only OVERLAY sibling-call class bug.

Flags every TARGET_NATIVE #else function whose LAST EXECUTED statement is a
tail-position `void` call lacking the function-scope guard `__asm__ __volatile__("")`.
cc1 sibling-call-lowers such a tail call to `j <callee>` WITHOUT restoring the frame
-> $sp left low -> caller resumes corrupted -> wild pointer (the boot trap).

Detection = backward scan from the function's closing brace: skip `}` / blank /
comment lines; the FIRST real statement is the tail. Candidate iff that statement is
a bare call `name(...);` (not `return ...`, not an assignment `... = ...`) AND it is
not the guard. (Matches func_00294C48; excludes SelectSceneSubChunk [ends in a store]
and func_002956F8 [ends in `return prev`].)

NO build / NO VM — pure source analysis.
Usage: tailcall_else_scan.py <file.c> [<file.c> ...]
"""
import sys, re

CALL = re.compile(r'^[A-Za-z_]\w*\s*\(.*\)\s*;\s*$')


def strip_comments(text):
    """remove /* */ (incl. multi-line) and // comments, preserving line count + newlines."""
    out = []
    i, n = 0, len(text)
    in_block = False
    in_line = False
    in_str = None
    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ''
        if in_block:
            if c == '*' and nxt == '/':
                in_block = False; i += 2; continue
            out.append('\n' if c == '\n' else ' '); i += 1; continue
        if in_line:
            if c == '\n':
                in_line = False; out.append('\n')
            i += 1; continue
        if in_str:
            out.append(c)
            if c == '\\' and nxt:
                out.append(nxt); i += 2; continue
            if c == in_str:
                in_str = None
            i += 1; continue
        if c == '/' and nxt == '*':
            in_block = True; i += 2; continue
        if c == '/' and nxt == '/':
            in_line = True; i += 2; continue
        if c in ('"', "'"):
            in_str = c; out.append(c); i += 1; continue
        out.append(c); i += 1
    return ''.join(out).splitlines()


def _guard_polarity(s):
    """('native'|'other', first_arm_is_native) for a preprocessor conditional.

    Unlike the block-level scanner in tools/guard_blocks.py, this tool walks line
    by line and must decide polarity AT THE OPENER, before it has seen either
    arm - so it cannot use the "arm holding INCLUDE_ASM is the asm arm" rule and
    has to read the condition. It therefore tests for a NEGATED mention rather
    than matching whole spellings:

        #ifndef TARGET_NATIVE                            -> first arm not native
        #if !defined(TARGET_NATIVE) && !defined(MATCH_x) -> first arm not native
        #ifdef TARGET_NATIVE                             -> first arm IS native
        #if defined(MATCH_x) || defined(TARGET_NATIVE)   -> first arm IS native

    Previously the two compound forms fell through to 'other' and their bodies
    were never scanned at all - 7 blocks corpus-wide.

    LIMIT, stated because it is invisible otherwise: a condition mixing a negated
    and a non-negated TARGET_NATIVE would be classified by the negation. None
    exists today; if one appears this returns the wrong arm rather than skipping,
    so re-check here if the corpus grows a mixed condition.
    """
    if not re.search(r'\bTARGET_NATIVE\b', s):
        return 'other', False
    negated = (re.match(r'#\s*ifndef\s+TARGET_NATIVE\b', s)
               or re.search(r'!\s*defined\s*\(\s*TARGET_NATIVE\s*\)', s)
               or re.search(r'!\s*TARGET_NATIVE\b', s))
    return 'native', not bool(negated)


def target_native_regions(lines):
    """yield (start,end) line-index ranges where TARGET_NATIVE code is ACTIVE
       (the #else arm of #ifndef TARGET_NATIVE, or the #if arm of #ifdef TARGET_NATIVE)."""
    stack = []        # each: ('native'|'other', this_arm_is_native)
    regions = []
    active_start = None
    for i, ln in enumerate(lines):
        s = ln.strip()
        if s.startswith('#if'):
            kind, first_arm_is_native = _guard_polarity(s)
            stack.append([kind, first_arm_is_native])
        elif s.startswith('#else') and stack:
            stack[-1][1] = (not stack[-1][1]) if stack[-1][0] == 'native' else stack[-1][1]
        elif s.startswith('#endif') and stack:
            stack.pop()
        # active iff ANY frame on the stack is a TARGET_NATIVE frame currently active
        cur_active = any(fr[0] == 'native' and fr[1] for fr in stack)
        if cur_active and active_start is None:
            active_start = i
        elif not cur_active and active_start is not None:
            regions.append((active_start, i)); active_start = None
    if active_start is not None:
        regions.append((active_start, len(lines)))
    return regions


def funcs_in(lines, lo, hi):
    """yield (name, body_start_idx, body_end_idx) for top-level func defs in [lo,hi)."""
    i = lo
    defhdr = re.compile(r'^\s*(?:static\s+)?[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\([^;]*\)\s*\{')
    while i < hi:
        m = defhdr.match(lines[i])
        if m and not lines[i].lstrip().startswith(('#', '//', '*')):
            # brace-match from the { on this line
            depth = lines[i].count('{') - lines[i].count('}')
            j = i + 1
            while j < hi and depth > 0:
                depth += lines[j].count('{') - lines[j].count('}')
                j += 1
            yield (m.group(1), i, j)
            i = j
        else:
            i += 1


def _kw_before_brace(text, bi):
    """controlling keyword of the construct whose '{' is at text[bi]."""
    j = bi - 1
    while j >= 0 and text[j] in ' \t\n':
        j -= 1
    if j < 0:
        return 'block'
    if text[j] == ')':                         # for/while/switch/if (...) {
        depth = 1; j -= 1
        while j >= 0 and depth:
            if text[j] == ')': depth += 1
            elif text[j] == '(': depth -= 1
            j -= 1
        while j >= 0 and text[j] in ' \t\n':
            j -= 1
    e = j
    while j >= 0 and (text[j].isalnum() or text[j] == '_'):
        j -= 1
    return text[j + 1:e + 1] or 'block'


def _loop_depth_per_line(lines, lo, hi):
    """for each line index in [lo,hi), how many enclosing LOOP/switch braces are open."""
    text = "\n".join(lines[lo:hi])
    # map char offset -> line index
    line_of = []
    li = lo
    for ch in text:
        line_of.append(li)
        if ch == '\n':
            li += 1
    line_of.append(li)
    stack = []                                  # is_loop bools
    depth_at = {}
    cur_line = lo
    for idx, ch in enumerate(text):
        ln = line_of[idx]
        if ln != cur_line:
            cur_line = ln
        depth_at[ln] = sum(1 for x in stack if x)
        if ch == '{':
            kw = _kw_before_brace(text, idx)
            stack.append(kw in ('for', 'while', 'do', 'switch'))
        elif ch == '}':
            if stack:
                stack.pop()
    return depth_at


def tail_is_unguarded_void_call(lines, lo, hi):
    """tail-position void call lacking the guard, AND not enclosed by a loop/switch."""
    depth = _loop_depth_per_line(lines, lo, hi)
    for k in range(hi - 1, lo - 1, -1):
        s = lines[k].strip()
        if s.strip('{}\t ') == '':            # blank or brace-only -> skip
            continue
        if '__asm__ __volatile__("")' in s:
            return (False, s)                 # already guarded
        if s.startswith('return'):
            return (False, s)                 # value/void return, not a bare call
        if CALL.match(s):
            if depth.get(k, 0) > 0:           # inside a loop/switch -> NOT tail position
                return (False, s + "   [loop-enclosed: NOT tail]")
            return (True, s)                  # genuine tail-position void call
        return (False, s)                     # assignment / store / other
    return (False, '')


def main():
    grand = []
    for path in sys.argv[1:]:
        # strip comments first: trailing /* */ on a call line, and '*'-leading
        # pointer stores, were corrupting the backward scan.
        lines = strip_comments(open(path, errors='ignore').read())
        hits = []
        for lo, hi in target_native_regions(lines):
            for name, fs, fe in funcs_in(lines, lo, hi):
                cand, tail = tail_is_unguarded_void_call(lines, fs, fe)
                if cand:
                    hits.append((name, fs + 1, tail))
        if hits:
            print(f"\n=== {path} : {len(hits)} unguarded tail-call #else ===")
            for name, line, tail in hits:
                print(f"   L{line:<5} {name:32} tail: {tail}")
            grand.extend((path, n, l) for n, l, _ in hits)
    print(f"\nTOTAL unguarded tail-call #else candidates: {len(grand)}")


if __name__ == '__main__':
    main()

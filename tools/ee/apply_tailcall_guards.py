#!/usr/bin/env python3
"""
apply_tailcall_guards.py — insert the proven sibling-call-suppression guard
`__asm__ __volatile__("");` as the FUNCTION-SCOPE last statement of named
TARGET_NATIVE #else functions (fixes cc1's frame-dropping `j <callee>` lowering).

Reuses tailcall_else_scan's region/function finders so the guard lands exactly
before the function's closing brace (function scope, NOT nested). Idempotent:
skips a function that already has the guard as its last statement.

Usage: apply_tailcall_guards.py <file.c> <funcName> [<funcName> ...]
"""
import sys, importlib.util, os

_spec = importlib.util.spec_from_file_location(
    "tcs", os.path.join(os.path.dirname(__file__), "tailcall_else_scan.py"))
tcs = importlib.util.module_from_spec(_spec); _spec.loader.exec_module(tcs)

GUARD = '    __asm__ __volatile__("");'


def main():
    path = sys.argv[1]
    wanted = set(sys.argv[2:])
    raw = open(path).read().splitlines(keepends=True)
    code = tcs.strip_comments("".join(raw))           # comment-stripped, 1:1 line map
    # locate each wanted func's closing-brace line (in TARGET_NATIVE regions)
    inserts = {}                                       # line_idx_of_closing_brace -> name
    for lo, hi in tcs.target_native_regions(code):
        for name, fs, fe in tcs.funcs_in(code, lo, hi):
            if name in wanted:
                close = fe - 1                         # the function's final '}'
                # already guarded? (last real stmt before close)
                already = any('__asm__ __volatile__("")' in code[k]
                              for k in range(fs, close))
                last_real = None
                for k in range(close - 1, fs, -1):
                    if code[k].strip().strip('{}\t ') != '':
                        last_real = code[k]; break
                if last_real is not None and '__asm__ __volatile__("")' in last_real:
                    print(f"  SKIP {name} (already guarded)"); wanted.discard(name); continue
                inserts[close] = name
                wanted.discard(name)
    if wanted:
        print(f"  WARN not found in TARGET_NATIVE regions: {sorted(wanted)}")
    # insert from the bottom up so earlier indices stay valid
    out = list(raw)
    for close in sorted(inserts, reverse=True):
        name = inserts[close]
        out.insert(close, GUARD + "\n")
        print(f"  guarded {name} (before closing brace at line {close + 1})")
    open(path, "w").write("".join(out))
    print(f"applied {len(inserts)} guard(s) to {path}")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Verify (not merely predict) the C-only overlay sibling-call frame-drop bug.

Reads the ACTUAL cc1 -DTARGET_NATIVE output (.s) produced by calt_undef_scan.sh
and reports, per unit, the functions that BOTH allocate a stack frame
(`subu $sp,$sp,N`) AND leave via a bare `j <symbol>` sibling call with NO
`addu $sp,$sp,N` frame restore before it. Those resume the CALLER with $sp
still low -> stack corruption (the documented boot trap).

This is machine evidence from emitted code, unlike the source-level candidate
screen in tailcall_else_scan.py.

Usage: calt_sibling_verify.py <dir-of-.s> [...]
"""
import os
import re
import sys

ENT = re.compile(r"\.ent\s+(\w+)(.*?)\.end\s+\1", re.S)
FRAME = re.compile(r"subu\s+\$sp,\$sp,(\d+)")
RESTORE = re.compile(r"addu\s+\$sp,\$sp,(\d+)")
SIBLING = re.compile(r"^\s+j\s+([A-Za-z_]\w*)\s*$", re.M)


def scan(path):
    text = open(path, errors="replace").read()
    out = []
    for m in ENT.finditer(text):
        name, body = m.group(1), m.group(2)
        if not FRAME.search(body):
            continue
        sib = list(SIBLING.finditer(body))
        if not sib:
            continue
        # look at the code from the last frame-restore (if any) to the end
        last_restore = None
        for r in RESTORE.finditer(body):
            last_restore = r.end()
        for s in sib:
            if last_restore is None or s.start() > last_restore:
                out.append((name, s.group(1)))
                break
    return out


def main():
    total = 0
    for d in sys.argv[1:]:
        for fn in sorted(os.listdir(d)):
            if not fn.endswith(".s"):
                continue
            hits = scan(os.path.join(d, fn))
            if hits:
                print("%s: %d framed+unrestored sibling tails" % (fn, len(hits)))
                for name, callee in hits:
                    print("    %-34s j %s" % (name, callee))
                total += len(hits)
    print("TOTAL verified framed+unrestored sibling tails: %d" % total)


if __name__ == "__main__":
    main()

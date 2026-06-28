#!/usr/bin/env python3
"""bisect_toggle.py — enable/disable a calt #else function body for per-function
overlay bisection. "disable" rewrites the `#else` directive immediately before
`<ret> <func>(` to `#elif 0` (so under -DTARGET_NATIVE the body is excluded ->
the overlay falls back to the base asm for that function). "enable" restores it.

Usage:
  bisect_toggle.py <file.c> disable <func> [<func> ...]
  bisect_toggle.py <file.c> enable  <func> [<func> ...]
  bisect_toggle.py <file.c> status
"""
import sys, re

def main():
    f = sys.argv[1]; op = sys.argv[2]
    funcs = sys.argv[3:]
    lines = open(f).read().split("\n")
    # locate each function definition line
    defre = lambda name: re.compile(r'^[A-Za-z_].*\b' + re.escape(name) + r'\s*\(')
    if op == "status":
        for i, ln in enumerate(lines):
            if re.match(r'^#elif 0 /\* BISECT', ln):
                # find following function
                for j in range(i, min(i+3, len(lines))):
                    m = re.match(r'^[A-Za-z_].*?\b(\w+)\s*\(', lines[j])
                    if m: print("DISABLED:", m.group(1)); break
        return
    changed = 0
    for name in funcs:
        # find the def line, then walk up to the nearest #else / #elif 0
        di = next((i for i, ln in enumerate(lines) if defre(name).match(ln)), None)
        if di is None:
            print("  ?? not found:", name); continue
        gi = next((i for i in range(di-1, max(di-20, -1), -1)
                   if lines[i].startswith("#else") or lines[i].startswith("#elif 0")), None)
        if gi is None:
            print("  ?? no guard above:", name); continue
        if op == "disable":
            if lines[gi].startswith("#else"):
                lines[gi] = "#elif 0 /* BISECT */"; changed += 1; print("  disabled", name)
            else:
                print("  already disabled", name)
        elif op == "enable":
            if lines[gi].startswith("#elif 0"):
                lines[gi] = "#else"; changed += 1; print("  enabled", name)
            else:
                print("  already enabled", name)
    open(f, "w").write("\n".join(lines))
    print("changed %d guard(s)" % changed)

if __name__ == "__main__":
    main()

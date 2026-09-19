#!/usr/bin/env python3
"""flagdiff.py — the per-unit cc1 flag tables (GFLAG / CC1EXTRA `case` arms) of
tools/ee/build.sh, tools/ee/objdiff_build.sh, tools/ee/diff.sh and
tools/ee/unit_flags.sh must agree. Every one of them carries a "keep in sync"
comment and nothing executed it: lever 2 (-fno-strict-aliasing on text/235FE8)
and WARM-1 both drifted between build.sh and objdiff_build.sh under that
comment (task #412, FACT #7164). The link (build.sh) and the unit gate
(objdiff_build.sh) then measured different compiles of the same C.

    tools/ee/flagdiff.py                      # the four tables in the tree
    tools/ee/flagdiff.py A.sh B.sh [C.sh ...] # explicit files; the FIRST is the reference

Exit 0 = every table equals the reference on every unit; 1 = a DRIFT line was
printed (unit, and each side's value NAMED by file); 2 = a table parsed to
zero units, which is a broken parser or a rewritten script, never "no drift".

Failure observable (on a mutated copy): `DRIFT usa/text/235FE8: build.sh=('-G8',
'-fno-gcse') objdiff_build.sh=('-G8', '-fno-gcse -fno-strict-aliasing')`, rc 1.
Bound: only `case` arms of the form `<pattern>) GFLAG="…"; CC1EXTRA="…";;` are
parsed. A flag set outside such an arm (an env override, a default assignment
changed in one script only) is invisible here. The diff96 engine arm's fixed
flags are not a per-unit table and are not compared.
"""
import os
import re
import sys

DEFAULTS = ["tools/ee/build.sh", "tools/ee/objdiff_build.sh", "tools/ee/diff.sh", "tools/ee/unit_flags.sh"]

# `  */usa/text/183178.c) GFLAG="-G8";;`, `  usa/text/183178) GFLAG="-G8";; # …`,
# `  */cod/0321A0.c) GFLAG="-G8";;` (region-less: applies to both regions)
ARM = re.compile(
    r'^\s*(?:\*/)?((?:usa/|eu/)?(?:cod|text)/[0-9A-Fa-f]{6})(?:\.c)?\)\s*(.*?)\s*;;'
)


def table(path):
    t = {}
    with open(path) as fh:
        for line in fh:
            m = ARM.match(line)
            if not m:
                continue
            unit, body = m.group(1), m.group(2)
            g = re.search(r'GFLAG="([^"]*)"', body)
            c = re.search(r'CC1EXTRA="([^"]*)"', body)
            val = (g.group(1) if g else "-G0", c.group(1) if c else "")
            units = [unit] if unit.startswith(("usa/", "eu/")) else ["usa/" + unit, "eu/" + unit]
            for u in units:
                if u in t and t[u] != val:
                    print(f"DUPLICATE {u} in {os.path.basename(path)}: {t[u]} then {val}")
                t[u] = val
    return t


def main(argv):
    files = argv[1:] or DEFAULTS
    if len(files) < 2:
        print("flagdiff: need at least two files (the first is the reference)", file=sys.stderr)
        return 2
    tables = {}
    for f in files:
        tables[f] = table(f)
        if not tables[f]:
            print(f"flagdiff: {f} parsed to ZERO units — parser or script rewritten, not 'no drift'")
            return 2
    ref = files[0]
    default = ("-G0", "")
    drift = 0
    for other in files[1:]:
        units = sorted(set(tables[ref]) | set(tables[other]))
        for u in units:
            a = tables[ref].get(u, default)
            b = tables[other].get(u, default)
            if a != b:
                drift += 1
                print(f"DRIFT {u}: {os.path.basename(ref)}={a} {os.path.basename(other)}={b}")
    summary = ", ".join(f"{os.path.basename(f)}={len(tables[f])}" for f in files)
    print(f"# units per table: {summary}; differing: {drift}")
    return 1 if drift else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

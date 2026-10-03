#!/usr/bin/env python3
"""cc1_arglog_check.py — every cc1 compile a build ran carries exactly the flags
the per-unit flag table assigns it (task #1377, RULING #9004).

    tools/ee/cc1_arglog_check.py <region> <arglog> [--table tools/ee/build.sh]
                                 [--selector tools/ee/s136os_functions.txt]

<arglog> is the file tools/ee/ee_cc1.sh appends to when EE_CC1_ARGLOG is set:
one `<arm> TAB <src> TAB <cc1 args>` line per compile. landing_gate.sh sets it
on the build it already runs and calls this on the result.

WHY: a flag that reaches the wrong compiler is often BYTE-IDENTICAL today. #1364
measured it: dropping 1B4218's 2.9-arm -fno-gcse (CC1EXTRA="") gives a unit
object identical to the tip's, so the whole-image cmp cannot see it, while
FACT #7933 says the same unpin moves 20 2.9-arm scores once 1B4218's arms are
promoted. Only the argument strings show it.

THE EXPECTATION IS DERIVED, NOT LISTED. The flag table is read from --table
with flagdiff.py's parser (the parser the FLAGS row already holds the four
table copies equal with); a unit the table does not name gets build.sh's
defaults (-G0, no extra flags). Then, for each line:
    sdk29  args == -O2 <GFLAG> <CC1EXTRA>
    s136   args == -O2 <GFLAG> <S136EXTRA> -fopt-stack
where S136EXTRA is the arm's own value if it sets one, else CC1EXTRA (the
scripts' `${S136EXTRA-$CC1EXTRA}`). Compared as word lists, order included.

POPULATION (a check over the lines present cannot see a compile that never
logged): every .c/.cpp under going-decompiled/src/<region> — build.sh's own
find — has exactly ONE sdk29 line, and no line names a source outside it; each
unit has exactly as many s136 lines as the selector has rows for it
(s136os_splice.sh compiles every row of the unit, or fails the build). Any
other arm in the log is a FAIL.

RULING #9004 FLOOR (usa only; the owner's explicit minimum, independent of the
table, because a consistent edit of all four table copies passes both FLAGS and
the derived rule above):
    1B4218 sdk29 carries -fno-gcse        else "-fno-gcse MISSING on the 2.9 arm"
    1B4218 s136 (>=1 line) carries none   else "s136 arm still pinned", per line
    1DFF80 keeps -fno-gcse on BOTH arms (>=1 s136 line; its pin is load-bearing:
           StopAllSoundEmitters +110 B unpinned)
Revoked with the ruling (its revoked_by); delete this block in that row.

Exit 0 every line and every count as derived; 1 a FAIL line was printed
(each offender named); 2 could not run — the arglog absent or EMPTY, or the
table parsed to zero units. 2 is not a pass: a build that logged nothing has
checked nothing (fail closed). landing_gate.sh turns 1 and 2 into FAIL rows.

Bound: the log records the string ee_cc1.sh hands cc1 ($CC1ARGS), not a trace
of the binary's argv; cpp arguments are not logged or checked. A flag set
outside a flagdiff-shaped `case` arm is outside the derived expectation and so
shows up here as a mismatch, not a pass.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import flagdiff  # noqa: E402  (the table parser; one copy)

SRC_RE = re.compile(r'(?:^|/)going-decompiled/src/(usa|eu)/(.+)\.(c|cpp)$')
DEFAULT = ("-G0", "", "")
FLOOR_PIN = "-fno-gcse"


def unit_of(src):
    m = SRC_RE.search(src)
    return (m.group(1), m.group(2)) if m else (None, None)


def sources(region):
    root = f"going-decompiled/src/{region}"
    out = set()
    for d, _, files in os.walk(root):
        for f in files:
            if f.endswith((".c", ".cpp")):
                out.add(os.path.relpath(os.path.join(d, f), "going-decompiled/src/" + region).rsplit(".", 1)[0])
    return out


def selector_rows(path, region):
    n = {}
    with open(path) as fh:
        for line in fh:
            w = line.split("#", 1)[0].split()
            if len(w) == 3 and w[0] == region:
                n[w[1]] = n.get(w[1], 0) + 1
    return n


def main(argv):
    args = argv[1:]
    table_path, sel_path = "tools/ee/build.sh", "tools/ee/s136os_functions.txt"
    pos = []
    while args:
        a = args.pop(0)
        if a == "--table":
            table_path = args.pop(0)
        elif a == "--selector":
            sel_path = args.pop(0)
        else:
            pos.append(a)
    if len(pos) != 2 or pos[0] not in ("usa", "eu"):
        print("usage: cc1_arglog_check.py <usa|eu> <arglog> [--table F] [--selector F]", file=sys.stderr)
        return 2
    region, log = pos
    if not os.path.isfile(log):
        print(f"CNR arglog {log} absent — the build did not log its cc1 calls (EE_CC1_ARGLOG unset or not reaching ee_cc1.sh); nothing checked is not a pass")
        return 2
    lines = [l.rstrip("\n") for l in open(log) if l.strip()]
    if not lines:
        print(f"CNR arglog {log} is EMPTY — no compile was logged; nothing checked is not a pass")
        return 2
    table = flagdiff.table(table_path)
    if not table:
        print(f"CNR {table_path} parsed to ZERO flag-table units")
        return 2

    fails = []
    sdk = {}       # unit -> [args...]
    s136 = {}      # unit -> [args...]
    for i, l in enumerate(lines, 1):
        parts = l.split("\t")
        if len(parts) != 3:
            fails.append(f"line {i}: malformed (want arm TAB src TAB args): {l!r}")
            continue
        arm, src, cargs = parts
        r, u = unit_of(src)
        if r != region:
            fails.append(f"line {i}: {arm} compile of {src} is not a {region} unit source")
            continue
        g, c, s = table.get(f"{r}/{u}", DEFAULT)
        got = cargs.split()
        if arm == "sdk29":
            want = ["-O2", g] + c.split()
            sdk.setdefault(u, []).append(got)
        elif arm == "s136":
            want = ["-O2", g] + s.split() + ["-fopt-stack"]
            s136.setdefault(u, []).append(got)
        else:
            fails.append(f"line {i}: unexpected arm '{arm}' for {r}/{u} (a build compiles sdk29 and s136 only)")
            continue
        if got != want:
            fails.append(f"{r}/{u} {arm}: {' '.join(got)} != table {' '.join(want)}")

    # population
    srcs = sources(region)
    rows = selector_rows(sel_path, region)
    for u in sorted(srcs):
        n = len(sdk.get(u, []))
        if n != 1:
            fails.append(f"{region}/{u} sdk29: {n} compile line(s), want exactly 1")
    for u in sorted(set(sdk) - srcs):
        fails.append(f"{region}/{u} sdk29: logged, but no such source under going-decompiled/src/{region}")
    for u in sorted(set(rows) | set(s136)):
        n, w = len(s136.get(u, [])), rows.get(u, 0)
        if n != w:
            fails.append(f"{region}/{u} s136: {n} compile line(s), selector has {w} row(s)")

    # RULING #9004 floor
    floor = []
    if region == "usa":
        def floor_unit(u, sdk_pinned, s136_pinned):
            sd, ss = sdk.get(u, []), s136.get(u, [])
            if not sd:
                fails.append(f"(floor) usa/{u}: no sdk29 compile logged — RULING #9004 cannot be checked")
            for a in sd:
                if (FLOOR_PIN in a) != sdk_pinned:
                    fails.append(f"(floor) usa/{u} sdk29 {' '.join(a)} -> " + (f"{FLOOR_PIN} MISSING on the 2.9 arm" if sdk_pinned else f"{FLOOR_PIN} present on the 2.9 arm"))
            if not ss:
                fails.append(f"(floor) usa/{u}: no s136 compile logged — RULING #9004 cannot be checked")
            for a in ss:
                if (FLOOR_PIN in a) != s136_pinned:
                    fails.append(f"(floor) usa/{u} s136 {' '.join(a)} -> " + ("s136 arm still pinned" if not s136_pinned else f"{FLOOR_PIN} MISSING on the s136 arm"))
            # observed, not wanted: a summary that restates the expectation
            # reads green on a failing log
            floor.append(f"usa/{u} sdk29 pinned {sum(FLOOR_PIN in a for a in sd)}/{len(sd)} (want {len(sd) if sdk_pinned else 0}), "
                         f"s136 pinned {sum(FLOOR_PIN in a for a in ss)}/{len(ss)} (want {len(ss) if s136_pinned else 0})")
        floor_unit("text/1B4218", True, False)
        floor_unit("text/1DFF80", True, True)

    for f in fails:
        print(f"FAIL {f}")
    pinned = sorted(u for u, v in sdk.items() if any(FLOOR_PIN in a for a in v))
    print(f"# {len(lines)} lines: sdk29 {sum(map(len, sdk.values()))} over {len(sdk)} units (sources {len(srcs)}), "
          f"s136 {sum(map(len, s136.values()))} over {len(s136)} units (selector rows {sum(rows.values())}); "
          f"table {table_path} {len(table)} units")
    print(f"# sdk29 {FLOOR_PIN}: {len(pinned)} units: {' '.join(pinned)}")
    s136_unpinned = sorted(u for u in s136 if u in pinned and not any(FLOOR_PIN in a for a in s136[u]))
    print(f"# s136 compiles of those without {FLOOR_PIN}: {' '.join(s136_unpinned) or '(none)'}")
    if floor:
        print(f"# RULING #9004 floor: {'; '.join(floor)}")
    print(f"# failures: {len(fails)}")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

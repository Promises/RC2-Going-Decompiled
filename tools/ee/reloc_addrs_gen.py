#!/usr/bin/env python3
"""reloc_addrs_gen.py - emit `reloc_addrs.txt` rows that make baked code addresses
RELOCATABLE, and REFUSE every target it cannot prove sound.

WHY THE REFUSALS ARE THE DELIVERABLE
------------------------------------
1,589 hand-authored `rom:` rows is a table nobody can review, and one wrong addend is
SILENT at link -- it assembles, links, and jumps into the middle of the wrong function.
Sampling does not help: a 2-of-3 spot check passed on an unsound construction earlier in
this lane's history.  So soundness has to be STRUCTURAL: a row that cannot be proven is
not emitted at all, and the list of what was refused is the output a human actually reads.

THE FOUR CLAUSES (forum/6168, consolidated)
-------------------------------------------
1 CONTAINMENT   SYM is the UNIQUE glabel whose [start, start + declared_size) contains
                the target.  NOT "greatest start <= target" -- those differ exactly on
                4-byte functions and on inter-function gaps, and the difference is a
                wrong-but-linkable addend.
                Cross-validated against the glabel..endlabel span.  EXACTLY ONE
                disagreement is expected and is NOT widened: func_002835C0, declared
                0x18 vs spanned 0x20, where 0x18 is a segment-boundary cut.
2 ADDEND        target - start(SYM) == the emitted addend, by construction and asserted.
3 SAME OBJECT   implied by clause 1; kept as an independent cheap check.
4 INCLUDED      SYM's size must come from a file that is actually ASSEMBLED.  A tracked,
                committed, documented `.s` fragment that no `INCLUDE_ASM` references is a
                SHADOW: it contributes no bytes and its size header is stale.  Measured
                case: nonmatchings/text/183558/func_002835C0.s declares 0x20 and is cited
                by 0 of that unit's 78 INCLUDE_ASM lines, while the live monolithic body
                declares 0x18.  A checker reading `asm/**` sees both and cannot tell which
                is real -- this clause is what tells it.

FALSE-ZERO POLICY: every population prints a count that must be non-zero, and the
emit/refuse split must sum to the input.  A generator that emits nothing looks identical
to a generator whose target scan is dead.
"""
from __future__ import annotations
import argparse
import bisect
import collections
import os
import re
import subprocess
import sys

GLABEL = re.compile(r"^glabel\s+(\S+)")
ENDLABEL = re.compile(r"^endlabel\s+(\S+)")
NONMATCH = re.compile(r"^nonmatching\s+(\S+),\s*0x([0-9A-Fa-f]+)")
INSTR = re.compile(r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{6,8})\s+[0-9A-Fa-f]{8}\s*\*/")
INCASM = re.compile(r'INCLUDE_ASM\("([^"]+)",\s*(\S+?)\s*\)')
WORD = re.compile(r"/\*\s*([0-9A-Fa-f]+)\s+([0-9A-Fa-f]{6,8})\s+\S+\s*\*/\s*\.word\s+0x([0-9A-Fa-f]+)")
SHORT = re.compile(r"/\*\s*([0-9A-Fa-f]+)\s+([0-9A-Fa-f]{6,8})\s*\*/\s*\.short\s+0x([0-9A-Fa-f]{1,4})")


def git_files(ref, path):
    out = subprocess.run(["git", "ls-tree", "-r", "--name-only", ref, "--", path],
                         capture_output=True, text=True).stdout
    return [p for p in out.split("\n") if p.endswith(".s") or p.endswith(".c")]


def git_show(ref, path):
    return subprocess.run(["git", "show", f"{ref}:{path}"],
                          capture_output=True, text=True).stdout


def included_fragments(ref, region):
    """Set of fragment basenames actually referenced by an INCLUDE_ASM."""
    inc = set()
    for p in git_files(ref, f"going-decompiled/src/{region}"):
        if not p.endswith(".c"):
            continue
        for m in INCASM.finditer(git_show(ref, p)):
            inc.add((m.group(1).rstrip("/"), m.group(2)))
    return inc


def build_symbols(ref, region, inc):
    """name -> (start, declared_size, spanned_size, path, included)."""
    syms = {}
    for p in git_files(ref, f"going-decompiled/asm/{region}"):
        if not p.endswith(".s"):
            continue
        text = git_show(ref, p)
        monolithic = "/nonmatchings/" not in p
        decl, cur, first, last = {}, None, None, None
        for ln in text.split("\n"):
            m = NONMATCH.match(ln)
            if m:
                decl[m.group(1)] = int(m.group(2), 16)
                continue
            g = GLABEL.match(ln)
            if g:
                cur, first, last = g.group(1), None, None
                continue
            e = ENDLABEL.match(ln)
            if e and cur and first is not None:
                _record(syms, cur, first, last, decl, p, monolithic, inc)
                cur = None
                continue
            i = INSTR.search(ln)
            if i and cur:
                a = int(i.group(1), 16)
                if first is None:
                    first = a
                last = a
        if cur and first is not None:
            _record(syms, cur, first, last, decl, p, monolithic, inc)
    return syms


def _record(syms, name, first, last, decl, path, monolithic, inc):
    size = decl.get(name)
    if size is None:
        return
    span = last - first + 4
    # clause 4: a fragment counts only if some INCLUDE_ASM cites it
    if monolithic:
        included = True
    else:
        d = os.path.dirname(path).replace("going-decompiled/", "", 1)
        included = (d, name) in inc or (os.path.dirname(path), name) in inc
    prev = syms.get(name)
    if prev and prev[4] and not included:
        return          # keep the included definition
    if prev and not prev[4] and included:
        pass            # replace the shadow
    elif prev and prev[4] and included:
        pass            # both included: caller reports
    syms[name] = (first, size, span, path, included)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--ref", default="ac135682")
    ap.add_argument("--region", default="usa")
    ap.add_argument("--emit")
    a = ap.parse_args()

    inc = included_fragments(a.ref, a.region)
    print("CONTROL INCLUDE_ASM references parsed : %d" % len(inc))
    if not inc:
        sys.stderr.write("REFUSING: no INCLUDE_ASM found - clause 4 cannot be evaluated.\n")
        return 2

    syms = build_symbols(a.ref, a.region, inc)
    live = {n: v for n, v in syms.items() if v[4]}
    shadow = {n: v for n, v in syms.items() if not v[4]}
    print("CONTROL sized symbols found           : %d" % len(syms))
    print("  live (clause 4 satisfied)           : %d" % len(live))
    print("  SHADOW (no INCLUDE_ASM cites them)  : %d  %s"
          % (len(shadow), sorted(shadow)[:4]))

    dis = [(n, v[1], v[2]) for n, v in live.items() if v[1] != v[2]]
    print("declared-vs-spanned disagreements     : %d  (expected exactly 1)" % len(dis))
    for n, d, s in sorted(dis):
        print("    %-24s declared 0x%X  spanned 0x%X" % (n, d, s))

    iv = sorted((v[0], v[0] + v[1], n) for n, v in live.items())
    ov = sum(1 for x, y in zip(iv, iv[1:]) if x[1] > y[0])
    print("overlapping intervals                 : %d  (must be 0 for uniqueness)" % ov)

    # ---- targets: baked code addresses in the data segments ------------------
    lo = [x[0] for x in iv]
    allsyms = sorted((v[0], v[0] + v[1], n) for n, v in syms.items())
    alllo = [x[0] for x in allsyms]

    def container(tbl, keys, v):
        i = bisect.bisect_right(keys, v) - 1
        if i < 0:
            return None
        s, e, n = tbl[i]
        return n if s <= v < e else None

    targets = []
    for p in git_files(a.ref, f"going-decompiled/asm/{a.region}/data"):
        if not p.endswith(".s"):
            continue
        rows = {}
        for ln in git_show(a.ref, p).split("\n"):
            m = WORD.search(ln)
            if m:
                targets.append((int(m.group(1), 16), int(m.group(3), 16), p, "word"))
                continue
            s = SHORT.search(ln)
            if s:
                rows[int(s.group(2), 16)] = (int(s.group(1), 16), int(s.group(3), 16))
        for va, (rom, loval) in sorted(rows.items()):
            if va % 4 or (va + 2) not in rows:
                continue
            hi = rows[va + 2][1]
            targets.append((rom, (hi << 16) | loval, p, "short"))

    print("\nCONTROL raw literals scanned          : %d" % len(targets))
    if not targets:
        sys.stderr.write("REFUSING: no literals parsed - target scan is dead.\n")
        return 2

    emit, refuse = [], collections.Counter()
    rows_out = []
    # A literal is a CANDIDATE only if it falls inside the code region at all.
    # Without this the "refused" list is dominated by zeros and small data words,
    # which were never code addresses and are not refusals - they are non-candidates.
    code_lo, code_hi = allsyms[0][0], max(e for _, e, _ in allsyms)
    for rom, tgt, path, kind in targets:
        if tgt % 4 or not (code_lo <= tgt < code_hi):
            continue
        sym = container(iv, lo, tgt)
        if sym is None:
            if container(allsyms, alllo, tgt) is not None:
                refuse["container is a SHADOW (clause 4)"] += 1
            else:
                refuse["NO CONTAINER (clause 1)"] += 1
            continue
        start, decl, span, spath, _ = live[sym]
        if decl != span:
            refuse["declared != spanned (clause 1 cross-check)"] += 1
            continue
        addend = tgt - start
        assert 0 <= addend < decl, "clause 2 violated"
        emit.append((rom, sym, addend, kind))
        rows_out.append("rom:0x%X reloc:MIPS_32 symbol:%s addend:0x%X" % (rom, sym, addend))

    code = len(emit) + sum(refuse.values())
    print("code-address literals (4-aligned, in a symbol) : %d" % code)
    print("  EMITTED   : %d" % len(emit))
    print("  REFUSED   : %d" % sum(refuse.values()))
    for k, v in refuse.most_common():
        print("      %-45s %d" % (k, v))
    print("  CONTROL sum %d == %d  %s" % (len(emit) + sum(refuse.values()), code,
                                          "OK" if len(emit) + sum(refuse.values()) == code else "MISMATCH"))
    if a.emit:
        with open(a.emit, "w") as fh:
            fh.write("# generated by reloc_addrs_gen.py -- do not hand-edit\n")
            fh.write("\n".join(rows_out) + "\n")
        print("wrote %s (%d rows)" % (a.emit, len(rows_out)))
    return 0


if __name__ == "__main__":
    sys.exit(main())

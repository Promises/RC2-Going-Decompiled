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
5 IS A POINTER  Clauses 1-4 all ask WHICH SYMBOL OWNS THE VALUE.  None asks whether the
                value is an ADDRESS at all -- and a packed pair of 16-bit fields read as a
                word passes every one of them, because it really does land inside a real
                function.  Measured at SHA 9ae6cf33 (gate FAIL): 35 such rows were EMITTED,
                e.g. `0x00120000 -> __divdi3 + 0x398`, relocating into the middle of a
                division helper for an integer that was never a pointer.

                THE CLAIM IS "WE CANNOT PROVE THIS VALUE IS A POINTER" -- not "this value
                looks suspicious".  A real code pointer CAN be 64KB-aligned and CAN have a
                zero low half; nothing here says otherwise.  This is the same refuse-what-
                you-cannot-prove design as clauses 1-4, applied one axis over.
                THE ASYMMETRY THAT JUSTIFIES REFUSING ON UNPROVABILITY:
                  false REFUSAL  -> a row we could have relocated is not relocated.  Costs
                                    coverage, and the loss is COUNTABLE -- it prints below.
                  false EMISSION -> a relocation against a symbol the value does not point
                                    into.  Assembles, links, and is silently wrong once the
                                    section moves.  There is no link error for this.
                Those costs are not symmetric, so the tie does not go to emitting.

                DISCRIMINATOR: how many DISTINCT addresses share one low half.  A genuine
                popular jump target is ONE address referenced many times (low half 0x9248:
                37 rows, 1 distinct).  A packed `(counter, constant)` table is MANY
                addresses sharing a constant low short (0x206C: 6 distinct,
                0x0030206C..0x0035206C, stride 0x10000).
                REJECTED, each measured:
                  "low half == 0"  catches 0x0000, misses 0x206C entirely.
                  "64KB-aligned"   0x206C is not.
                  "concentration"  the MOST concentrated low half in the set is an ordinary
                                   jump target, so frequency alone is no evidence.
                  "both halves small"  DO NOT -- 81 members / 29 distinct, far too broad.

                ORDERING IS PART OF THE CLAUSE, AND IS ASSERTED IN CODE BELOW, NOT COMMENTED.
                Computed before containment the same threshold flags 7 classes instead of 2,
                one of which (0x001C) has zero surviving members -- a refusal pointing at
                nothing.  A comment survives a refactor; the ordering does not, so the
                assertion is the only durable form of this paragraph.

FALSE-ZERO POLICY: every population prints a count that must be non-zero, and the
emit/refuse split must sum to the input.  A generator that emits nothing looks identical
to a generator whose target scan is dead.
"""
from __future__ import annotations
import argparse
import bisect
import collections
import math
import os
import re
import subprocess
import sys

# A 4-aligned low half has 2^16 / 4 possible values.  Not a tuning knob -- it is what the
# `& 0xFFFF` in the discriminator means.
LOWHALF_BINS = 16384

# Clause 5 threshold, in DISTINCT addresses sharing one low half.
# NOTHING IS FROZEN HERE ON PURPOSE.  Two constants were published for this clause and both
# were withdrawn as artifacts of how they were produced rather than facts about the data:
#   "null max = 4"      a TRIAL-COUNT artifact; the simulated max is 5 at 30k and at 100k.
#   "P = 2.95e-03"      a POPULATION artifact, computed at a pre-containment N the clause
#                       never runs at.
# So the probability is COMPUTED AT RUNTIME, from the population the clause is actually
# holding (see `chance_of_class`).  The threshold below is the only number, and the
# distribution it cuts is bimodal -- observed classes are [15, 6, 3, 3, 2 ...], so any value
# in [4, 6] yields identical output.  That margin is a SANITY CHECK, not the justification;
# the justification is the unprovability argument in the module docstring.
LOWHALF_MIN_DISTINCT = 4


def chance_of_class(n_distinct, k, code_lo, code_hi):
    """P(SOME low-half class reaches k distinct addresses) under a no-structure null.

    Computed, never cached: every input is a property of the run.  Real addresses are
    DISTINCT, so a bin holding `s` candidate slots fills k of them C(s,k) ways rather than
    s**k/k! -- the product below is that correction, C(s,k)/(s**k/k!) = prod(1 - i/s).
    Dropping it would overstate collisions and make the clause look better justified than
    it is, so it is applied rather than quoted.
    """
    slots_per_bin = ((code_hi - code_lo) // 4) / LOWHALF_BINS
    if slots_per_bin < k:                    # cannot fit k distinct addresses in a bin
        return 0.0
    lam = n_distinct / LOWHALF_BINS
    p = math.exp(-lam) * lam ** k / math.factorial(k)
    for i in range(k):
        p *= (1.0 - i / slots_per_bin)
    return 1.0 - (1.0 - p) ** LOWHALF_BINS

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

    # Reported, not enforced, and the count is 0 by CONSTRUCTION rather than by luck: the
    # one known disagreement (func_002835C0, declared 0x18 vs spanned 0x20) lives in a
    # SHADOW fragment, and clause 4 removes shadows before this runs.  An earlier revision
    # annotated this "expected exactly 1" and printed 0 on every run -- a stated expectation
    # contradicted by its own output, which nothing noticed because nothing checked it.
    # A non-zero here now means a LIVE symbol disagrees, which clause 1's cross-check refuses
    # per row below; it is a signal to read, not a stop condition.
    dis = [(n, v[1], v[2]) for n, v in live.items() if v[1] != v[2]]
    print("declared-vs-spanned disagreements     : %d  (0 expected: the known one is a "
          "shadow, removed by clause 4)" % len(dis))
    for n, d, s in sorted(dis):
        print("    %-24s declared 0x%X  spanned 0x%X" % (n, d, s))

    # ENFORCED, not printed.  Clause 1 says SYM is the UNIQUE symbol containing the target,
    # and `container()` implements that as "greatest start <= v, then check v < end" -- which
    # is only equivalent to uniqueness if the intervals do not overlap.  If two live symbols
    # overlap, bisect silently picks one and every addend derived from the other is wrong in
    # exactly the way this tool exists to prevent.  Printing "must be 0" while continuing
    # anyway states the premise without testing it.
    iv = sorted((v[0], v[0] + v[1], n) for n, v in live.items())
    overlaps = [(x[2], y[2]) for x, y in zip(iv, iv[1:]) if x[1] > y[0]]
    print("overlapping intervals                 : %d  (0 required -- clause 1 uniqueness)"
          % len(overlaps))
    if overlaps:
        for a, b in overlaps[:8]:
            sys.stderr.write("  OVERLAP %s / %s\n" % (a, b))
        sys.stderr.write("REFUSING: live symbol intervals overlap, so containment is not "
                         "unique and every addend is unsafe.\n")
        return 2

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
    # NOTE: a RUN-membership filter was tried here and REVERTED. It looked principled
    # (/6022 measured 54 of 55 runs homogeneous by owner) and it is refuted by this
    # lane's own partition: of the 595 `.word` targets, 81 are ISOLATED single
    # code-pointing words under ordinary `D_` labels - callback slots and stored
    # handlers, not table entries (/5892). Filtering to runs discarded those 81 AND
    # silently removed every clause-4 refusal, because the shadow-container hits are
    # themselves isolated. A discriminator that deletes the findings is not a filter.
    incode = [t for t in targets
              if t[1] % 4 == 0 and code_lo <= t[1] < code_hi]
    print("  in-code-region, 4-aligned            : %d" % len(incode))
    # ---- clauses 1-4: per-ROW soundness --------------------------------------
    survivors = []
    for rom, tgt, path, kind in incode:
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
        survivors.append((rom, tgt, sym, start, decl, kind))

    # ---- clause 5: a POPULATION statistic, over the rows that survived 1-4 ----
    # ORDERING ASSERTION.  The clause is only defined on a post-containment population, and
    # a refactor that moves it earlier would still compute a plausible-looking answer -- 7
    # classes instead of 2, one of them with no surviving members.  Every row here must have
    # a live container; a pre-containment population contains rows that do not, so feeding
    # one in fails loudly instead of silently changing the output.
    for _, tgt, _, _, _, _ in survivors:
        assert container(iv, lo, tgt) is not None, (
            "clause 5 received a pre-containment population (0x%08X has no live container) "
            "-- it must run AFTER clauses 1-4" % tgt)

    # Keyed on DISTINCT addresses, not rows: one jump target referenced 37 times is one
    # address and must not look like a table.
    by_lowhalf = collections.defaultdict(set)
    for _, tgt, _, _, _, _ in survivors:
        by_lowhalf[tgt & 0xFFFF].add(tgt)
    packed = {h for h, s in by_lowhalf.items() if len(s) >= LOWHALF_MIN_DISTINCT}
    undet = {h: len(s) for h, s in by_lowhalf.items()
             if 1 < len(s) < LOWHALF_MIN_DISTINCT}
    n_distinct = sum(len(s) for s in by_lowhalf.values())

    for rom, tgt, sym, start, decl, kind in survivors:
        if (tgt & 0xFFFF) in packed:
            refuse["NOT PROVABLY A POINTER (clause 5)"] += 1
            continue
        addend = tgt - start
        assert 0 <= addend < decl, "clause 2 violated"
        emit.append((rom, sym, addend, kind))
        rows_out.append("rom:0x%X reloc:MIPS_32 symbol:%s addend:0x%X" % (rom, sym, addend))

    print("\nsurvived clauses 1-4                  : %d rows, %d distinct addresses"
          % (len(survivors), n_distinct))
    print("clause 5, DISTINCT addresses per low half (threshold %d)"
          % LOWHALF_MIN_DISTINCT)
    print("    P(some class reaches %d | no structure) = %.2e   <- computed at runtime"
          % (LOWHALF_MIN_DISTINCT,
             chance_of_class(n_distinct, LOWHALF_MIN_DISTINCT, code_lo, code_hi)))
    for h in sorted(packed, key=lambda x: -len(by_lowhalf[x])):
        vals = sorted(by_lowhalf[h])
        print("    REFUSED 0x%04X  %2d distinct  %s%s"
              % (h, len(vals), " ".join("0x%08X" % v for v in vals[:4]),
                 " ..." if len(vals) > 4 else ""))
    # NOT REFUSED IS NOT "SHOWN TO BE CLEAN".  These classes are individually below
    # threshold; they are not individually innocent.  A per-item threshold cannot see a
    # COLLECTIVE excess, and the residual (these plus the 2-distinct classes) sits above
    # chance by an amount that is unresolved in BOTH directions on two seats' measurements
    # -- and is not testable by any spatial null, because the low-half statistic is a
    # function of the offset multiset, so conditioning on the structure conditions on the
    # answer.  Settling it needs an independent axis (is the target a real branch target?),
    # not a better null.
    near = {h: n for h, n in undet.items() if n > 2}
    print("    KEPT, below threshold, NOT shown to be noise : %s"
          % (", ".join("0x%04X(%d)" % (h, n) for h, n in sorted(near.items())) or "none"))
    print("    plus %d classes at 2 distinct (chance alone gives P=%.2f for some 2-class)"
          % (sum(1 for n in undet.values() if n == 2),
             chance_of_class(n_distinct, 2, code_lo, code_hi)))
    # Per-kind breakdown, because a clause that appears not to fire on a subpopulation reads
    # as broken.  NOTE: the largest .short class is 0x0000 -- the SAME class as .word's, not
    # a separate .short phenomenon -- and it is refused via the combined population, which is
    # the only population the clause is defined on.  The largest .short-ONLY class is 0x1FD8
    # at 2 distinct, below threshold.  (A published spec described .short as having a single
    # >=2 class at 0x1FD8; that holds for a low-half-FILTERED .short population and not for
    # the one this clause sees, where 0x0000 contributes 4 distinct .short targets.)
    for kind in ("word", "short"):
        sub = collections.defaultdict(set)
        for _, tgt, _, _, _, k in survivors:
            if k == kind:
                sub[tgt & 0xFFFF].add(tgt)
        top = sorted(((len(s), h) for h, s in sub.items()), reverse=True)[:2]
        print("    %-5s classes >=2: %-3d largest: %s"
              % (kind, sum(1 for s in sub.values() if len(s) >= 2),
                 " ".join("0x%04X(%d)" % (h, n) for n, h in top) or "none"))

    print("\nEMITTED   : %d" % len(emit))
    print("REFUSED   : %d" % sum(refuse.values()))
    for k, v in refuse.most_common():
        print("      %-45s %d" % (k, v))
    # CONTROL against the INPUT, not against a total derived from the outputs: comparing
    # emit+refuse to a variable *defined* as emit+refuse is an identity and cannot fail.
    total = len(emit) + sum(refuse.values())
    ok = total == len(incode)
    print("CONTROL   emit + refuse == candidates : %d == %d  %s"
          % (total, len(incode), "OK" if ok else "MISMATCH"))
    if not ok:
        sys.stderr.write("REFUSING: rows lost between candidate scan and emission.\n")
        return 2
    if a.emit:
        with open(a.emit, "w") as fh:
            fh.write("# generated by reloc_addrs_gen.py -- do not hand-edit\n")
            fh.write("\n".join(rows_out) + "\n")
        print("wrote %s (%d rows)" % (a.emit, len(rows_out)))
    return 0


if __name__ == "__main__":
    sys.exit(main())

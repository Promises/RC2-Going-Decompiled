#!/usr/bin/env python3
"""Put every compiled C function of the boot's final unit asm in its own section.

    python3 -I tools/levels/boot_sections.py <boot build dir> <cc1_args.tsv> <out dir>

Our own tool (task #2516, RULING #13219 condition 1, NOTE #13218 option (c)).

Input: the boot build's OWN final unit assembly, `<boot build>/going-decompiled/
src/usa/text/<unit>._u.s` (written by tools/ee/build.sh after the s136os/sn2952
splices and jtbl_bind, i.e. exactly what the boot's asm_unit.sh assembled into
the gated boot image). Boot sources are never compiled again.

For each unit it writes <out>/<unit>.s, a copy in which every function cc1
emitted (a `.ent F` ... `.end F` block) sits in its own section `.text.F`:
  * the block starts at the `.ent` line, extended back over the lines cc1 and
    the splices put before it (`.align`, `.p2align`, `.globl`, `.text`,
    `.section .text`, `.file`, `#S136OS_BEGIN`, `#SN2952_BEGIN`), and ends at
    `.end F`;
  * a `.section .text.F,"ax",@progbits` line goes before the block and a
    `.section .text` line after it, so the unit's other code stays in .text;
  * inside the block, a return to `.text` (after a `.rdata` switch, say)
    becomes a return to `.text.F`. Other section switches are left as they are.
A function is NOT sectioned, and is listed as refused with its reason, when:
  * its name has not exactly one `.ent` and one `.end` line, in order;
  * a `.L`/`$L` label defined in its block is referenced outside it, or the
    block references one defined outside it (a branch across the cut).
INCLUDE_ASM functions have no `.ent` (they are `.include`d asm), so only C is
sectioned.

It also writes:
  <out>/units.tsv     unit <TAB> -G flag <TAB> source   (one row per unit; -G is
                      read from the gated build's cc1_args.tsv sdk29 row for that
                      source, never from a copy of build.sh's flag table)
  <out>/sections.tsv  unit <TAB> function <TAB> align bytes (2^N of the block's
                      first `.align N` / `.p2align N`; a block with neither is
                      given 8 and counted in the summary line)
  <out>/refused.tsv   unit <TAB> function <TAB> reason
Nothing under tools/ee/ or going-decompiled/src is written.
"""
import os
import re
import sys

LEAD = re.compile(r"^\s*(\.align|\.p2align|\.globl|\.file\s|\.text\s*$|\.section\s+\.text\s*$|#S136OS_BEGIN|#SN2952_BEGIN)")
LBL_DEF = re.compile(r"^\s*(\$L\w+|\.L\w+):")
LBL_USE = re.compile(r"(\$L\w+|\.L\w+)")
TO_TEXT = re.compile(r"^\s*(\.text|\.section\s+\.text)\s*$")


def code(l):
    """A line without its # comment (cc1 writes `#nop`, splices write `#S136OS_...`)."""
    return l.split("#", 1)[0]


def section_unit(lines):
    ents, endl = {}, {}
    for i, l in enumerate(lines):
        m = re.match(r"^\s*\.ent\s+(\S+)\s*$", l)
        if m:
            ents.setdefault(m.group(1), []).append(i)
        m = re.match(r"^\s*\.end\s+(\S+)\s*$", l)
        if m:
            endl.setdefault(m.group(1), []).append(i)
    blocks, refused = [], []
    for fn in ents:
        e, d = ents[fn], endl.get(fn, [])
        if len(e) != 1 or len(d) != 1 or d[0] < e[0]:
            refused.append((fn, "ent/end %d/%d" % (len(e), len(d))))
            continue
        s = e[0]
        while s > 0 and LEAD.match(lines[s - 1]):
            s -= 1
        blocks.append([s, d[0], fn])
    blocks.sort()
    # labels: defined in a block and used outside it, or used in a block and
    # defined outside it, refuse the block
    defs = {}
    for b in blocks:
        for l in lines[b[0]:b[1] + 1]:
            m = LBL_DEF.match(l)
            if m:
                defs[m.group(1)] = b[2]
    owner = [None] * len(lines)
    for s, d, fn in blocks:
        for i in range(s, d + 1):
            owner[i] = fn
    cross = {}
    alldefs = {}
    for i, l in enumerate(lines):
        m = LBL_DEF.match(l)
        if m:
            alldefs[m.group(1)] = owner[i]
    for i, l in enumerate(lines):
        for lab in LBL_USE.findall(code(l)):
            if LBL_DEF.match(l) and l.strip().startswith(lab + ":"):
                continue
            where = alldefs.get(lab, "<undefined>")
            if where != owner[i]:
                for f in (where, owner[i]):
                    if f and f != "<undefined>":
                        cross.setdefault(f, "label crosses block: %s (line %d)" % (lab, i + 1))
    keep = []
    for b in blocks:
        if b[2] in cross:
            refused.append((b[2], cross[b[2]]))
        else:
            keep.append(b)
    out = []
    starts = {s: fn for s, d, fn in keep}
    ends = {d: fn for s, d, fn in keep}
    cur = None
    for i, l in enumerate(lines):
        if i in starts:
            cur = starts[i]
            out.append('\t.section\t.text.%s,"ax",@progbits' % cur)
        if cur and TO_TEXT.match(l):
            out.append('\t.section\t.text.%s,"ax",@progbits' % cur)
        else:
            out.append(l)
        if i in ends:
            out.append("\t.section\t.text")
            cur = None
    aligns = []
    for s, d, fn in keep:
        al = None
        for l in lines[s:d + 1]:
            m = re.match(r"^\s*\.(?:p2)?align\s+(\d+)", l)
            if m:
                al = 1 << int(m.group(1))
                break
        aligns.append((fn, al))
    return out, aligns, refused


def gflags(cc1_args):
    g = {}
    for l in open(cc1_args):
        f = l.rstrip("\n").split("\t")
        if len(f) >= 3 and f[0] == "sdk29":
            m = re.search(r"(?:^|\s)(-G\d+)(?:\s|$)", f[2])
            g[f[1]] = m.group(1) if m else "-G0"
    return g


def main():
    if len(sys.argv) != 4:
        sys.exit("usage: boot_sections.py <boot build dir> <cc1_args.tsv> <out dir>")
    bdir, cc1_args, out = sys.argv[1:4]
    src = os.path.join(bdir, "going-decompiled", "src", "usa", "text")
    G = gflags(cc1_args)
    os.makedirs(out, exist_ok=True)
    ut = open(os.path.join(out, "units.tsv"), "w")
    st = open(os.path.join(out, "sections.tsv"), "w")
    rf = open(os.path.join(out, "refused.tsv"), "w")
    nsec = nref = nunit = nnoalign = 0
    for f in sorted(os.listdir(src)):
        if not f.endswith("._u.s"):
            continue
        unit = f[:-len("._u.s")]
        cands = [s for s in G if re.search(r"/usa/text/%s\.(c|cpp)$" % re.escape(unit), s)]
        if len(cands) != 1:
            sys.exit("boot_sections: unit %s: %d sdk29 rows in %s (want 1)" % (unit, len(cands), cc1_args))
        lines = open(os.path.join(src, f)).read().split("\n")
        o, aligns, refused = section_unit(lines)
        open(os.path.join(out, unit + ".s"), "w").write("\n".join(o))
        ut.write("%s\t%s\t%s\n" % (unit, G[cands[0]], cands[0]))
        for fn, al in aligns:
            if al is None:
                nnoalign += 1
                al = 8
            st.write("%s\t%s\t%d\n" % (unit, fn, al))
        for fn, why in sorted(refused):
            rf.write("%s\t%s\t%s\n" % (unit, fn, why))
            print("boot_sections: REFUSED %s %s: %s" % (unit, fn, why))
        nsec += len(aligns)
        nref += len(refused)
        nunit += 1
    print("boot_sections: %d units, %d functions sectioned, %d refused (members above), %d with no .align (given 8)"
          % (nunit, nsec, nref, nnoalign))
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Second-spelling check for level C (task #2617; census NOTE #13409, FACT #13410).

    second_names.py check --level NN --root <boot tree> --level-dir <LNN dir>
                          --allow <tsv> [--complete] <fn>=<object.o> ...

Our own code; host `python3 -I`, no ELF library.

The class it stops: a level C TU that reaches a datum through an ADDRESS-NAMED
extern (`D_001395E3`, `D_L09_001B1578`: level_elf.py's ADDR_NAME, valued by
`level_elf.py undef` from the address in its name) when a DIFFERENT name
already sits at EXACTLY that address. The link is byte-identical either way,
which is why no byte gate sees it; what breaks is the name: the tree then
spells one object two ways (D_001395B8 / D_1395B8, task #2606; D_001395E3 /
dlabel D_1395E3, FACT #13403), and a later rename of one spelling misses the
other.

Names checked: every UNDEFINED global symbol of each C object (the ELF symtab,
so comments, unused declarations and macros do not count, and an asm-label view
`__asm__("D_x")` is checked under the name it links against), that matches
ADDR_NAME. It is valued at the address in its name and looked up in the scope
below. It is SAME (no second spelling) only if it IS one of the scope's names AT
THAT ADDRESS (task #2635, FACT #13438): being a name somewhere is not enough.
The level splits spell some core addresses short (`D_13A148`, 47 `func_` short
forms) where the boot's own name is `dlabel D_0013A148` / `glabel
func_0011A930`; those spellings are level-only, not in scope at their address,
and FAIL naming the boot's spelling.

  core (address < 0x1A7C80, every level's .lit dest), the boot's names:
    boot-label  going-decompiled/asm/usa/**/*.s glabel/dlabel/alabel/jlabel,
                addressed from the next spimdisasm `/* off ADDR ... */` comment
    boot-opref  an ADDR_NAME-shaped OPERAND name in that asm (%hi/%lo/.word),
                valued from its name: D_1395B8 has no dlabel (it lies inside
                dlabel D_139599) yet it is the boot's spelling of 0x1395B8 -
                the label-only first census missed exactly that class
    boot-sa     going-decompiled/symbol_addrs/usa/symbol_addrs.txt
  level space (>= 0x1A7C80), the level's own split:
    lvl-label / lvl-opref   the same two, over <LNN dir>/asm/**/*.s
    lvl-bootmap             <LNN dir>/symbol_addrs/LNN_boot_names.txt entries
                            at or above 0x1A7C80: the names this level's own
                            link gives boot objects (RULING #13219), valued per
                            level (g_mobyTableBase is 0x1B359C in L02, 0x1B29DC
                            in L08). Task #2646: an address-named extern sitting
                            exactly on one of them (the A2 class of NOTE #13409,
                            D_L02_001B359C = g_mobyTableBase) is a second
                            spelling and FAILs naming the map's name. The file
                            is required: a level without it cannot be checked.
  The boot's own view of overlay memory (boot names >= 0x1A7C80) is NOT in
  scope: a boot-view name there need not hold for a level (0x1BA3C0 is named
  differently by the boot split and by L02's map; the A3 class of NOTE #13409).
  Base+offset names INSIDE a larger label, with no name at their exact address
  in the split or the map, pass (RULING #13291 section 2).
  Not checked (unruled, NOTE #13423 finding 2 and the #2646 addendum): a name
  that IS a level-split name at its address passes as SAME even when the map
  gives the address a different name (split dlabel D_L02_001AA850 vs map
  g_pLevelOrder; split glabel func_L08_002FAC20 vs map DistXYVu0).

A name with a different name at its exact address is SECOND-NAME. It passes
only on a row of the allowlist (tools/levels/second_names_allow.tsv) keyed by
level + function + name whose `existing` column equals the names found:
  ACCEPTED  an accepted second name (#2580's D_001B1578, the #8549 precedent:
            small, per site, documented);
  PENDING   a member already on master when this check landed, awaiting its
            respell row. Never added to: a respell row deletes its row in the
            same landing (RULING #7317: baselines are only lowered).
With --complete (the gate passes every C row of the level), an allowlist row of
this level that matches no SECOND-NAME found this run is STALE and FAILs, so a
respell that leaves its PENDING row behind cannot pass either.

Prints one line per member and `#### second_names LNN: PASS|FAIL (...)`.
Exit 0 PASS, 1 FAIL, 2 usage/input error.
"""
import os
import re
import struct
import sys

ADDR_NAME = re.compile(r"^(?:func|D|jtbl|B|RO|T|FLT|DBL|STR|PSTR)_(?:L\d\d_)?([0-9A-F]{1,8})$")
IDENT = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
LABEL = re.compile(r"^\s*(glabel|dlabel|alabel|jlabel)\s+([A-Za-z_][\w.$]*)")
SKIP = re.compile(r"^\s*(nonmatching|endlabel|enddlabel)\b")
CMT = re.compile(r"/\*\s*([^*]*?)\s*\*/")
SA = re.compile(r"^\s*([A-Za-z_][\w.$]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
H8 = re.compile(r"[0-9A-F]{8}")
CORE_TOP = 0x1A7C80
STATUSES = ("ACCEPTED", "PENDING")


def scan_asm(path, rel, kind, out):
    """(addr, name, kind-label|kind-opref, rel) for labels and ADDR_NAME operand names."""
    pend = []
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = LABEL.match(line)
            if m:
                pend.append(m.group(2))
                continue
            if SKIP.match(line):
                continue
            c = CMT.search(line)
            if pend and c:
                toks = c.group(1).split()
                addr = None
                if len(toks) == 1 and H8.fullmatch(toks[0]):
                    addr = int(toks[0], 16)          # bss: /* ADDR */
                elif len(toks) >= 2 and H8.fullmatch(toks[1]):
                    addr = int(toks[1], 16)          # /* off ADDR [word] */
                if addr is not None:
                    for n in pend:
                        out.append((addr, n, kind + "-label", rel))
                    pend = []
            if "_" not in line:
                continue
            for t in IDENT.findall(CMT.sub(" ", line)):
                mm = ADDR_NAME.match(t)
                if mm:
                    out.append((int(mm.group(1), 16), t, kind + "-opref", rel))


def scan_tree(top, rel_base, kind, out):
    n = 0
    for d, _, fs in sorted(os.walk(top)):
        for fn in sorted(fs):
            if fn.endswith(".s"):
                p = os.path.join(d, fn)
                scan_asm(p, os.path.relpath(p, rel_base), kind, out)
                n += 1
    return n


def undefined_globals(obj):
    """Undefined GLOBAL/WEAK symbol names of an ELF32 LE relocatable."""
    b = open(obj, "rb").read()
    if b[:4] != b"\x7fELF" or b[4] != 1 or b[5] != 1:
        raise ValueError("%s is not an ELF32 LE object" % obj)
    shoff, = struct.unpack_from("<I", b, 0x20)
    shentsize, shnum = struct.unpack_from("<HH", b, 0x2E)
    shs = [struct.unpack_from("<10I", b, shoff + i * shentsize) for i in range(shnum)]
    names = set()
    for s in shs:
        if s[1] != 2:  # SHT_SYMTAB
            continue
        strs = shs[s[6]][4]
        for i in range(s[5] // 16):
            n, v, sz, info, oth, shn = struct.unpack_from("<IIIBBH", b, s[4] + 16 * i)
            if shn == 0 and n and info >> 4 in (1, 2):
                names.add(b[strs + n:b.index(b"\0", strs + n)].decode())
    return names


def evidence(kinds_files):
    """'boot-label a.s; boot-opref b.s (+3 files)' from {'kind file', ...}."""
    by = {}
    for kf in kinds_files:
        k, w = kf.split(" ", 1)
        by.setdefault(k, []).append(w)
    return "; ".join("%s %s%s" % (k, sorted(ws)[0], " (+%d files)" % (len(ws) - 1) if len(ws) > 1 else "")
                     for k, ws in sorted(by.items()))


def read_allow(path, level):
    rows = {}
    for ln, line in enumerate(open(path), 1):
        if not line.strip() or line.startswith("#"):
            continue
        p = line.rstrip("\n").split("\t")
        if len(p) < 6 or p[3] not in STATUSES:
            raise ValueError("%s:%d: want level/function/name/status(%s)/existing/reason, got %r"
                             % (path, ln, "|".join(STATUSES), line.rstrip("\n")))
        if p[0] == level:
            key = (p[1], p[2])
            if key in rows:
                raise ValueError("%s:%d: duplicate row %s %s %s" % (path, ln, level, p[1], p[2]))
            rows[key] = (p[3], p[4], p[5], ln)
    return rows


def main(argv):
    a = argv[1:]
    if a[:1] != ["check"]:
        print(__doc__.strip().split("\n\n")[0], file=sys.stderr)
        return 2
    opts, objs, i = {"--complete": False}, [], 1
    while i < len(a):
        if a[i] == "--complete":
            opts["--complete"] = True
            i += 1
        elif a[i] in ("--level", "--root", "--level-dir", "--allow"):
            opts[a[i]] = a[i + 1]
            i += 2
        elif "=" in a[i]:
            objs.append(tuple(a[i].split("=", 1)))
            i += 1
        else:
            print("second_names: unknown argument %r" % a[i], file=sys.stderr)
            return 2
    try:
        lv = "L%02d" % int(opts["--level"], 10)
        root, ldir, allowp = opts["--root"], opts["--level-dir"], opts["--allow"]
    except (KeyError, ValueError):
        print("second_names: need --level NN --root R --level-dir D --allow TSV", file=sys.stderr)
        return 2
    try:
        allow = read_allow(allowp, lv)
    except (OSError, ValueError) as e:
        print("second_names: allowlist: %s" % e)
        print("#### second_names %s: FAIL (allowlist unreadable)" % lv)
        return 2

    boot = []
    nb = scan_tree(os.path.join(root, "going-decompiled/asm/usa"), root, "boot", boot)
    sap = os.path.join(root, "going-decompiled/symbol_addrs/usa/symbol_addrs.txt")
    for line in open(sap, encoding="utf-8", errors="replace"):
        m = SA.match(line)
        if m:
            boot.append((int(m.group(2), 16), m.group(1), "boot-sa", os.path.relpath(sap, root)))
    lvl = []
    nl = scan_tree(os.path.join(ldir, "asm"), ldir, "lvl", lvl)
    bmp = os.path.join(ldir, "symbol_addrs", "%s_boot_names.txt" % lv)
    if os.path.isfile(bmp):
        for line in open(bmp, encoding="utf-8", errors="replace"):
            m = SA.match(line)
            if m and int(m.group(2), 16) >= CORE_TOP:
                lvl.append((int(m.group(2), 16), m.group(1), "lvl-bootmap", os.path.relpath(bmp, ldir)))
    nmap = sum(1 for e in lvl if e[2] == "lvl-bootmap")
    # an empty scope is a broken query, never a clean result
    if nb == 0 or nl == 0 or nmap == 0 or not any(k == "boot-label" for _, _, k, _ in boot):
        print("second_names: scope empty: %d boot .s, %d level .s under %s, %d boot-name map entries in %s"
              % (nb, nl, ldir, nmap, bmp))
        print("#### second_names %s: FAIL (no scope)" % lv)
        return 2
    scope = [e for e in boot if e[0] < CORE_TOP] + [e for e in lvl if e[0] >= CORE_TOP]
    byaddr = {}
    for addr, n, k, w in scope:
        byaddr.setdefault(addr, {}).setdefault(n, set()).add("%s %s" % (k, w))
    print("second_names %s: scope %d boot .s + symbol_addrs.txt (%d entries below 0x%X), %d level .s + boot-name map (%d entries at/above, %d of them map names); %d C objects"
          % (lv, nb, sum(1 for e in boot if e[0] < CORE_TOP), CORE_TOP, nl,
             sum(1 for e in lvl if e[0] >= CORE_TOP), nmap, len(objs)))

    cnt = {"same": 0, "free": 0, "ACCEPTED": 0, "PENDING": 0, "fail": 0, "stale": 0}
    hit = set()
    for fn, obj in objs:
        try:
            und = undefined_globals(obj)
        except (OSError, ValueError, struct.error) as e:
            print("SECOND-NAME FAIL %s %s: cannot read object %s: %s" % (lv, fn, obj, e))
            cnt["fail"] += 1
            continue
        for n in sorted(und):
            m = ADDR_NAME.match(n)
            if not m:
                continue
            addr = int(m.group(1), 16)
            # SAME only when the scope itself spells this address this way: the
            # boot's own name for a core address, the level's for level space
            if n in byaddr.get(addr, {}):
                cnt["same"] += 1
                continue
            others = {o: v for o, v in byaddr.get(addr, {}).items() if o != n}
            if not others:
                cnt["free"] += 1
                print("   free     %s %s: %s = 0x%08X (no name at that address: base+off, RULING #13291 section 2)" % (lv, fn, n, addr))
                continue
            existing = ",".join(sorted(others))
            where = "; ".join("%s [%s]" % (o, evidence(v)) for o, v in sorted(others.items()))
            row = allow.get((fn, n))
            if row and row[1] == existing:
                hit.add((fn, n))
                cnt[row[0]] += 1
                print("   %-8s %s %s: %s = 0x%08X is also %s (allowlist line %d: %s)" % (row[0].lower(), lv, fn, n, addr, existing, row[3], row[2]))
                continue
            cnt["fail"] += 1
            print("SECOND-NAME FAIL %s %s: %s = 0x%08X is already named %s: %s -> spell it %s%s"
                  % (lv, fn, n, addr, existing, where, sorted(others)[0],
                     "" if not row else " (allowlist line %d names existing '%s', found '%s')" % (row[3], row[1], existing)))
    if opts["--complete"]:
        for (fn, n), (st, ex, why, ln) in sorted(allow.items(), key=lambda kv: kv[1][3]):
            if (fn, n) not in hit:
                cnt["stale"] += 1
                print("SECOND-NAME STALE %s: allowlist line %d (%s %s %s %s) matches no second name this run: delete it in this landing (baselines only lowered, RULING #7317)"
                      % (lv, ln, st, fn, n, ex))
    ok = cnt["fail"] == 0 and cnt["stale"] == 0
    print("#### second_names %s: %s (%d objects; %d same-name, %d base+off/free, %d accepted, %d pending, %d FAIL, %d stale allowlist rows)"
          % (lv, "PASS" if ok else "FAIL", len(objs), cnt["same"], cnt["free"], cnt["ACCEPTED"], cnt["PENDING"], cnt["fail"], cnt["stale"]))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))

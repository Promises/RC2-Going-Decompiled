#!/usr/bin/env python3
"""The level gate's EXTERNAL anchor: each level program's sha1/size/entry read
straight off the disc (task #2458, NOTE #13098 row L5; addendum from validator #2490).

    python3 -I tools/levels/disc_programs.py table --iso <USA v2.00 iso> [--names <extracted/usa_levels>] > tools/levels/expected_programs.tsv
    python3 -I tools/levels/disc_programs.py read  --iso <iso> --level 24
    python3 -I tools/levels/disc_programs.py crosscheck tools/levels/expected_programs.tsv <OpenRAC config/programs.tsv> [--version us2]

Why it exists: the level build's section cmp compares against blobs that
extract_levels.py cut from the disc, and the asm was split from those same
blobs, so that comparison is a ROUND TRIP: it cannot see an extractor error.
This reader shares no code with extract_levels.py (its own ISO9660 walk, its
own WAD header and slot read) and hashes the program's RAW byte range as the
disc stores it -- no record parsing, no repack. Its table is committed as
tools/levels/expected_programs.tsv, and OpenRAC's programs.tsv (an independent
implementation, read as data with `git show`) is a CROSS-CHECK of it only.

Disc layout used (all little-endian, 2048-byte sectors):
  ISO9660 primary volume descriptor at sector 16, root directory record at +156;
  G/LEVELn.WAD (n = 0..26); WAD +0x08 = level number, +0x10 = (level-data
  sector offset within the WAD, sector count); level data begins with
  (byte offset, byte size) slots, slot 0 = the program; the program's first
  16-byte record carries the entry at +12.

table: one row per WAD, `level wad program sha1 size entry`, tab-separated.
`program` is a LABEL only (the NN_<name> directory name extract_levels.py gives
the level under --names, else LNN); the anchor is level + sha1/size/entry.
read: prints `level wad sha1 size entry` for one level.
crosscheck: joins the table to the TSV by program name for --version and prints
every member EQUAL/DIFFERS/MISSING; exit 1 unless every table row is EQUAL.
Their rows with no table row (us2 `boot`, the boot program) are listed, not failed.
"""
import hashlib
import os
import struct
import sys

SECTOR = 2048


class Disc:
    def __init__(self, path):
        self.f = open(path, "rb")
        pvd = self.sec(16, 1)
        if pvd[0] != 1 or pvd[1:6] != b"CD001":
            sys.exit("disc_programs: %s: no ISO9660 primary volume descriptor at sector 16" % path)
        self.root = self.rec(pvd, 156)

    def sec(self, lba, count):
        self.f.seek(lba * SECTOR)
        b = self.f.read(count * SECTOR)
        if len(b) != count * SECTOR:
            sys.exit("disc_programs: short read at sector %d" % lba)
        return b

    @staticmethod
    def rec(b, o):
        n = b[o]
        lba = struct.unpack_from("<I", b, o + 2)[0]
        size = struct.unpack_from("<I", b, o + 10)[0]
        flags = b[o + 25]
        name = b[o + 33:o + 33 + b[o + 32]].decode("ascii", "replace")
        return n, name, lba, size, flags

    def find(self, path):
        cur = self.root
        for part in path.upper().split("/"):
            _, _, lba, size, flags = cur
            if not flags & 2:
                sys.exit("disc_programs: %s: not a directory on the way to %s" % (cur[1], path))
            buf = self.sec(lba, (size + SECTOR - 1) // SECTOR)
            o, hit = 0, None
            while o < size:
                if buf[o] == 0:
                    o = (o // SECTOR + 1) * SECTOR
                    continue
                r = self.rec(buf, o)
                if r[1].split(";")[0] == part:
                    hit = r
                    break
                o += r[0]
            if hit is None:
                sys.exit("disc_programs: %s not on the disc" % path)
            cur = hit
        return cur


def read_level_program(disc, n):
    wad = "G/LEVEL%d.WAD" % n
    _, _, wlba, wsize, _ = disc.find(wad)
    hdr = disc.sec(wlba, 1)
    level = struct.unpack_from("<I", hdr, 0x08)[0]
    dsec, dcnt = struct.unpack_from("<II", hdr, 0x10)
    poff, psize = struct.unpack_from("<II", disc.sec(wlba + dsec, 1), 0)
    if dsec * SECTOR + poff + psize > wsize:
        sys.exit("disc_programs: %s: program range beyond the WAD" % wad)
    disc.f.seek((wlba + dsec) * SECTOR + poff)
    prog = disc.f.read(psize)
    if len(prog) != psize:
        sys.exit("disc_programs: %s: short program read" % wad)
    entry = struct.unpack_from("<I", prog, 12)[0]
    return level, wad, hashlib.sha1(prog).hexdigest(), psize, "%08X" % entry


def arg(name, default=None):
    if name in sys.argv:
        return sys.argv[sys.argv.index(name) + 1]
    return default


def main():
    if len(sys.argv) < 2 or sys.argv[1] not in ("table", "read", "crosscheck"):
        print(__doc__.strip().split("\n\n")[0], file=sys.stderr)
        return 2
    cmd = sys.argv[1]
    if cmd == "crosscheck":
        table, tsv, ver = sys.argv[2], sys.argv[3], arg("--version", "us2")
        theirs = {}
        for line in open(tsv):
            p = line.rstrip("\n").split("\t")
            if len(p) >= 5 and p[0] == ver:
                theirs[p[1]] = (p[2], int(p[3]), p[4].upper())
        ok = n = 0
        for line in open(table):
            if line.startswith("#") or not line.strip():
                continue
            lv, wad, prog, sha, size, entry = line.rstrip("\n").split("\t")
            n += 1
            t = theirs.pop(prog, None)
            mine = (sha, int(size), entry.upper())
            v = "MISSING" if t is None else ("EQUAL" if t == mine else "DIFFERS %s %d %s" % t)
            ok += v == "EQUAL"
            print("crosscheck: L%02d %-22s %s %d %s  %s" % (int(lv), prog, sha, int(size), entry, v))
        for prog in sorted(theirs):
            print("crosscheck: %s row %s not in the table (not a level WAD; informational)" % (ver, prog))
        print("crosscheck: %d of %d table rows EQUAL to %s %s rows (%d of theirs unmatched)" % (ok, n, os.path.basename(tsv), ver, len(theirs)))
        return 0 if ok == n and n > 0 else 1
    disc = Disc(arg("--iso"))
    if cmd == "read":
        n = int(arg("--level"))
        for w in range(27):
            r = read_level_program(disc, w)
            if r[0] == n:
                print("%d\t%s\t%s\t%d\t%s" % r)
                return 0
        print("disc_programs: no WAD holds level %d" % n, file=sys.stderr)
        return 1
    names = arg("--names")
    print("# level\twad\tprogram\tsha1\tsize\tentry -- tools/levels/disc_programs.py table: raw slot-0 range of each G/LEVELn.WAD on the USA v2.00 disc (sha256 eda31b45...ec6e72)")
    for w in range(27):
        level, wad, sha, size, entry = read_level_program(disc, w)
        label = "L%02d" % level
        if names:
            hit = [d for d in sorted(os.listdir(names)) if d.startswith("%02d_" % level)]
            label = hit[0] if hit else label
        print("%d\t%s\t%s\t%s\t%d\t%s" % (level, wad, label, sha, size, entry))
    return 0


if __name__ == "__main__":
    sys.exit(main())

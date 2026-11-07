#!/usr/bin/env python3
"""Extract the 27 level programs of Ratchet & Clank: Going Commando (USA v2.00).

Our own tool, written from the format description in claudiverse NOTE #13098 §1
(itself read from OpenRAC as data) and checked against the disc. No third-party
code was copied.

    extract_levels.py extract --iso <iso> [--out extracted/usa_levels] [--only N,...]
    extract_levels.py repack  [--out extracted/usa_levels] [--expect <tsv> --version us2]

`extract` writes, per level, `extracted/usa_levels/NN_<name>/`:
    lit.bin bss.bin data.bin vtbl.bin camvtbl.bin sndvtbl.bin text.bin
    manifest.json   (per section: dest, size, type, sha1; entry; level number; ...)
    LNN.elf         (synthesized ELF32 MIPS executable, one PT_LOAD per section)

`repack` rebuilds each program's archive form (the record list exactly as the
disc stores it) from the .bin files plus the manifest only, and prints
sha1 / size / entry. With --expect it compares against rows of a TSV
`version<TAB>program<TAB>sha1<TAB>size<TAB>entry` and exits 1 on any mismatch
or on a missing / extra program.

Format (all little-endian, sectors of 2048 bytes):
  * ISO9660: G/LEVELn.WAD, n = 0..26.
  * WAD header: +0x08 = the level number (the archive's own; LEVEL21.WAD holds
    level 30); +0x10 = (level-data sector offset from the WAD, sector count).
  * Level data: 12 (byte offset, byte size) slots; slot 0 is the program.
  * Program: records {u32 dest, u32 size, u32 type, u32 entry}, each followed by
    `size` bytes (a NOBITS section's bytes are stored too). No compression, no
    relocation: an absolute overlay.
  * Stop rule: the list ends where `entry` changes or the program extent ends.
    There must then be EXACTLY 7 sections and the records must consume the
    program extent exactly. No size threshold is used anywhere: NOTE #10140
    records that a size heuristic silently truncated 10 of 27 levels.
"""
import argparse
import hashlib
import json
import os
import struct
import sys

SECTOR = 2048
NSLOTS = 12
LIT_BASE = 0x1A7C80
SHT_PROGBITS, SHT_NOBITS = 1, 8

# Order of the 7 sections in every program: (file stem, ELF name, sh_type,
# sh_flags, sh_addralign). Flags/alignment mirror the boot ELF's sections of
# the same names (SCUS_972.68's level slot, 0x1A7C80 upward).
SECTIONS = [
    ("lit",     ".lit",        SHT_PROGBITS, 0x10000003, 64),
    ("bss",     ".bss",        SHT_NOBITS,   0x10000003, 64),
    ("data",    ".data",       SHT_PROGBITS, 0x3,        64),
    ("vtbl",    "lvl.vtbl",    SHT_PROGBITS, 0x2,        1),
    ("camvtbl", "lvl.camvtbl", SHT_PROGBITS, 0x2,        1),
    ("sndvtbl", "lvl.sndvtbl", SHT_PROGBITS, 0x2,        1),
    ("text",    ".text",       SHT_PROGBITS, 0x6,        8),
]

# Level number (the archive's +0x08 word) -> directory name.
LEVEL_NAMES = {
    0: "aranos_tutorial", 1: "oozla", 2: "maktar_nebula", 3: "endako",
    4: "barlow", 5: "feltzin_system", 6: "notak", 7: "siberius", 8: "tabora",
    9: "dobbo", 10: "hrugis_cloud", 11: "joba", 12: "todano", 13: "boldan",
    14: "aranos_prison", 15: "gorn", 16: "snivelak", 17: "smolg",
    18: "damosel", 19: "grelbin", 20: "yeedil", 22: "dobbo_orbit",
    23: "damosel_orbit", 24: "ship_shack", 25: "wupash_nebula",
    26: "jamming_array", 30: "insomniac_museum",
}

ELF_FLAGS = 0x20924001  # the boot ELF's e_flags (R5900, EABI)


class FormatError(Exception):
    pass


# ---------------------------------------------------------------- ISO9660 ----
class Iso:
    def __init__(self, path):
        self.f = open(path, "rb")
        pvd = self.read(16, SECTOR)
        if pvd[0] != 1 or pvd[1:6] != b"CD001":
            raise FormatError("no ISO9660 primary volume descriptor at sector 16")
        self.root = self._dirent(pvd, 156)

    def read(self, lba, n):
        self.f.seek(lba * SECTOR)
        b = self.f.read(n)
        if len(b) != n:
            raise FormatError("short read at sector %d" % lba)
        return b

    @staticmethod
    def _dirent(buf, o):
        ln = buf[o]
        lba, = struct.unpack_from("<I", buf, o + 2)
        size, = struct.unpack_from("<I", buf, o + 10)
        flags = buf[o + 25]
        nlen = buf[o + 32]
        name = buf[o + 33:o + 33 + nlen]
        return ln, name, lba, size, flags

    def listdir(self, ent):
        _, _, lba, size, flags = ent
        if not flags & 2:
            raise FormatError("not a directory")
        buf = self.read(lba, size)
        o, out = 0, {}
        while o < len(buf):
            if buf[o] == 0:  # records never straddle a sector: skip to the next
                o = (o // SECTOR + 1) * SECTOR
                continue
            e = self._dirent(buf, o)
            if e[1] not in (b"\x00", b"\x01"):
                out[e[1].split(b";")[0].decode("ascii")] = e
            o += e[0]
        return out

    def lookup(self, path):
        ent = self.root
        for part in path.split("/"):
            d = self.listdir(ent)
            if part not in d:
                raise FormatError("not on the disc: %s" % path)
            ent = d[part]
        return ent  # (len, name, lba, size, flags)


# ---------------------------------------------------------------- program ----
def parse_program(prog):
    """Split a program into its records. Raises FormatError on any deviation."""
    o, recs, entry = 0, [], None
    while o < len(prog):
        if o + 16 > len(prog):
            raise FormatError("truncated record header at +0x%x" % o)
        dest, size, typ, ent = struct.unpack_from("<4I", prog, o)
        if entry is None:
            entry = ent
        if ent != entry:
            break  # stop rule: the entry point changed
        if o + 16 + size > len(prog):
            raise FormatError("record at +0x%x overruns the program (size 0x%x)" % (o, size))
        recs.append((dest, size, typ, ent, prog[o + 16:o + 16 + size]))
        o += 16 + size
    if len(recs) != len(SECTIONS):
        raise FormatError("%d sections, expected exactly %d" % (len(recs), len(SECTIONS)))
    if o != len(prog):
        raise FormatError("records end at +0x%x but the program is 0x%x bytes" % (o, len(prog)))
    for (stem, _, want, _, _), (dest, size, typ, _, _) in zip(SECTIONS, recs):
        if typ != want:
            raise FormatError("section %s has type %d, expected %d" % (stem, typ, want))
    if recs[0][0] != LIT_BASE:
        raise FormatError(".lit at 0x%x, expected 0x%x" % (recs[0][0], LIT_BASE))
    return recs, entry


def repack(recs):
    out = bytearray()
    for dest, size, typ, ent, body in recs:
        assert len(body) == size
        out += struct.pack("<4I", dest, size, typ, ent) + body
    return bytes(out)


# -------------------------------------------------------------------- ELF ----
def write_elf(path, recs, entry):
    nsec = len(recs)
    shstr = b"\0"
    name_off = []
    for _, name, _, _, _ in SECTIONS:
        name_off.append(len(shstr))
        shstr += name.encode() + b"\0"
    shstr_name = len(shstr)
    shstr += b".shstrtab\0"

    ehsize, phentsize, shentsize = 52, 32, 40
    off = ehsize + phentsize * nsec
    blobs, offs = [], []
    for (stem, _, typ, _, align), (dest, size, _, _, body) in zip(SECTIONS, recs):
        off = (off + 15) & ~15
        offs.append(off)
        if typ != SHT_NOBITS:
            blobs.append((off, body))
            off += size
    shstr_off = off
    off += len(shstr)
    shoff = (off + 3) & ~3

    img = bytearray(shoff + shentsize * (nsec + 2))
    struct.pack_into("<4sBBBB8xHHIIIIIHHHHHH", img, 0, b"\x7fELF", 1, 1, 1, 0,
                     2, 8, 1, entry, ehsize, shoff, ELF_FLAGS,
                     ehsize, phentsize, nsec, shentsize, nsec + 2, nsec + 1)
    for i, ((stem, _, typ, flags, align), (dest, size, _, _, _)) in enumerate(zip(SECTIONS, recs)):
        pflags = 5 if flags & 4 else 6  # R+X for .text, R+W otherwise
        filesz = 0 if typ == SHT_NOBITS else size
        struct.pack_into("<8I", img, ehsize + phentsize * i,
                         1, offs[i], dest, dest, filesz, size, pflags, 0x10)
    for o, body in blobs:
        img[o:o + len(body)] = body
    img[shstr_off:shstr_off + len(shstr)] = shstr
    for i, ((stem, _, typ, flags, align), (dest, size, _, _, _)) in enumerate(zip(SECTIONS, recs)):
        struct.pack_into("<10I", img, shoff + shentsize * (i + 1),
                         name_off[i], typ, flags, dest, offs[i], size, 0, 0, align, 0)
    struct.pack_into("<10I", img, shoff + shentsize * (nsec + 1),
                     shstr_name, 3, 0, 0, shstr_off, len(shstr), 0, 0, 1, 0)
    with open(path, "wb") as f:
        f.write(img)


# ------------------------------------------------------------------- main ----
def sha1(b):
    return hashlib.sha1(b).hexdigest()


def level_dirname(level):
    return "%02d_%s" % (level, LEVEL_NAMES.get(level, "level%02d" % level))


def cmd_extract(a):
    iso = Iso(a.iso)
    files = range(27) if not a.only else [int(x) for x in a.only.split(",")]
    os.makedirs(a.out, exist_ok=True)
    bad = 0
    for n in files:
        wadname = "G/LEVEL%d.WAD" % n
        try:
            _, _, wlba, wsize, _ = iso.lookup(wadname)
            hdr = iso.read(wlba, 0x18)
            level, = struct.unpack_from("<I", hdr, 0x08)
            dsec, dcnt = struct.unpack_from("<II", hdr, 0x10)
            if (dsec + dcnt) * SECTOR > ((wsize + SECTOR - 1) // SECTOR) * SECTOR:
                raise FormatError("level data (%d+%d sectors) beyond the WAD" % (dsec, dcnt))
            slots = list(struct.iter_unpack("<II", iso.read(wlba + dsec, 8 * NSLOTS)))
            poff, psize = slots[0]
            if poff + psize > dcnt * SECTOR:
                raise FormatError("slot 0 beyond the level data")
            base = (wlba + dsec) * SECTOR + poff
            iso.f.seek(base)
            prog = iso.f.read(psize)
            if len(prog) != psize:
                raise FormatError("short program read")
            recs, entry = parse_program(prog)
            if repack(recs) != prog:
                raise FormatError("repack is not byte-identical to the stored program")
        except FormatError as e:
            print("%-14s FAIL %s" % (wadname, e))
            bad += 1
            continue

        d = os.path.join(a.out, level_dirname(level))
        os.makedirs(d, exist_ok=True)
        sections = []
        for (stem, name, typ, _, _), (dest, size, rtyp, _, body) in zip(SECTIONS, recs):
            with open(os.path.join(d, stem + ".bin"), "wb") as f:
                f.write(body)
            s = {"name": name, "file": stem + ".bin", "dest": "0x%08X" % dest,
                 "size": size, "type": rtyp, "sha1": sha1(body)}
            if typ == SHT_NOBITS:
                nz = [i for i, c in enumerate(body) if c]
                s["nonzero_bytes"] = len(nz)
                if nz:
                    s["first_nonzero_offset"] = "0x%x" % nz[0]
            sections.append(s)
        elf = "L%02d.elf" % level
        write_elf(os.path.join(d, elf), recs, entry)
        man = {
            "level": level, "wad": wadname, "wad_lba": wlba, "wad_size": wsize,
            "level_data_sector": dsec, "level_data_sectors": dcnt,
            "slots": [[o, s] for o, s in slots],
            "program_offset": poff, "program_size": psize,
            "program_sha1": sha1(prog),
            "entry": "0x%08X" % entry, "sections": sections, "elf": elf,
        }
        with open(os.path.join(d, "manifest.json"), "w") as f:
            json.dump(man, f, indent=1)
            f.write("\n")
        bss = sections[1]
        print("%-14s level %02d -> %-22s size %8d entry %08X sha1 %s bss_nonzero %d" % (
            wadname, level, level_dirname(level), psize, entry, sha1(prog), bss["nonzero_bytes"]))
    print("extracted %d, failed %d" % (len(files) - bad, bad))
    return 1 if bad else 0


def cmd_repack(a):
    got = {}
    for dn in sorted(os.listdir(a.out)):
        mp = os.path.join(a.out, dn, "manifest.json")
        if not os.path.isfile(mp):
            continue
        man = json.load(open(mp))
        entry = int(man["entry"], 16)
        recs = []
        for s in man["sections"]:
            body = open(os.path.join(a.out, dn, s["file"]), "rb").read()
            recs.append((int(s["dest"], 16), len(body), s["type"], entry, body))
        prog = repack(recs)
        got[dn] = (sha1(prog), len(prog), "%08X" % entry)

    exp = {}
    if a.expect:
        for line in open(a.expect):
            if line.startswith("#") or not line.strip():
                continue
            p = line.rstrip("\n").split("\t")
            if p[0] == a.version and p[1] != "boot":
                exp[p[1]] = (p[2], int(p[3]), p[4].upper())

    ok = bad = 0
    for dn in sorted(set(got) | set(exp)):
        g, e = got.get(dn), exp.get(dn)
        if not a.expect:
            print("%-22s ours %s %8d %s" % (dn, g[0], g[1], g[2]))
            continue
        verdict = "OK" if g == e else "DIFFERS"
        ok += g == e
        bad += g != e
        fmt = lambda t: "%s %8d %s" % t if t else "%-40s %8s %8s" % ("(missing)", "-", "-")
        print("%-22s ours %s | theirs %s  %s" % (dn, fmt(g), fmt(e), verdict))
    if a.expect:
        print("equal %d/%d, differ %d" % (ok, len(set(got) | set(exp)), bad))
        return 1 if bad else 0
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    e = sub.add_parser("extract")
    e.add_argument("--iso", required=True)
    e.add_argument("--out", default="extracted/usa_levels")
    e.add_argument("--only", help="comma-separated WAD file numbers (default 0..26)")
    r = sub.add_parser("repack")
    r.add_argument("--out", default="extracted/usa_levels")
    r.add_argument("--expect", help="TSV: version, program, sha1, size, entry")
    r.add_argument("--version", default="us2")
    a = ap.parse_args()
    return cmd_extract(a) if a.cmd == "extract" else cmd_repack(a)


if __name__ == "__main__":
    sys.exit(main())

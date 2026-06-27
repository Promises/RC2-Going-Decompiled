#!/usr/bin/env python3
"""elf_contiguous_repack.py — repackage a linked EE ELF so each address REGION is
ONE contiguous PT_LOAD with bss MATERIALIZED as file zeros (FileSiz==MemSiz),
matching the original boot ELF. Fixes the boot fault where split LOAD segments
leave .cod_bss globals in an unloaded gap (garbage -> kernel fault -> pc=0x0).

Reads the input ELF's LOAD segments, groups them into regions (gap > GAP starts a
new region), zero-fills each region from min vaddr to max vaddr+memsz, overlays
each segment's file bytes, and writes a fresh ELF (entry preserved) with one
PT_LOAD per region. 32-bit big/little-endian EE ELF.

Usage: elf_contiguous_repack.py <in.elf> <out.elf>
"""
import sys, struct

GAP = 0x200000  # segments farther apart than this start a new region

def main():
    src, dst = sys.argv[1], sys.argv[2]
    d = open(src, "rb").read()
    assert d[:4] == b"\x7fELF", "not an ELF"
    ei_class, ei_data = d[4], d[5]            # 1=32bit, 1=LE/2=BE
    assert ei_class == 1, "expect ELF32"
    en = "<" if ei_data == 1 else ">"
    # ELF32 header
    (e_type, e_machine, e_version, e_entry, e_phoff, e_shoff, e_flags,
     e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx
     ) = struct.unpack(en + "HHIIIIIHHHHHH", d[16:52])
    segs = []
    for i in range(e_phnum):
        off = e_phoff + i * e_phentsize
        p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags, p_align = \
            struct.unpack(en + "IIIIIIII", d[off:off + 32])
        if p_type == 1 and p_memsz > 0:       # PT_LOAD
            segs.append((p_vaddr, p_filesz, p_memsz, p_flags,
                         d[p_offset:p_offset + p_filesz]))
    segs.sort()
    # group into regions by GAP
    regions = []
    for v, fz, mz, fl, data in segs:
        if regions and v - (regions[-1]["end"]) <= GAP:
            regions[-1]["segs"].append((v, fz, mz, fl, data))
            regions[-1]["end"] = max(regions[-1]["end"], v + mz)
            regions[-1]["flags"] |= fl
        else:
            regions.append({"start": v, "end": v + mz, "flags": fl,
                            "segs": [(v, fz, mz, fl, data)]})
    # materialize each region: contiguous, bss as zeros, FileSiz==MemSiz
    blobs = []
    for r in regions:
        size = r["end"] - r["start"]
        buf = bytearray(size)
        for v, fz, mz, fl, data in r["segs"]:
            buf[v - r["start"]: v - r["start"] + len(data)] = data
        blobs.append((r["start"], r["flags"] | 0x4, bytes(buf)))  # ensure R
    # write new ELF: EHDR + PHDRs + page-padded blobs
    ehsize, phentsize = 52, 32
    phoff = ehsize
    nseg = len(blobs)
    data_start = phoff + nseg * phentsize
    out = bytearray()
    phdrs = bytearray()
    cur = data_start
    cur = (cur + 0xF) & ~0xF
    placed = []
    for vaddr, flags, blob in blobs:
        foff = cur
        placed.append((foff, blob))
        phdrs += struct.pack(en + "IIIIIIII", 1, foff, vaddr, vaddr,
                             len(blob), len(blob), flags, 0x10)
        cur += len(blob)
        cur = (cur + 0xF) & ~0xF
    ehdr = d[:16] + struct.pack(en + "HHIIIIIHHHHHH",
        e_type, e_machine, e_version, e_entry, phoff, 0, e_flags,
        ehsize, phentsize, nseg, 0, 0, 0)
    out += ehdr
    out += phdrs
    while len(out) < placed[0][0]:
        out += b"\x00"
    for foff, blob in placed:
        while len(out) < foff:
            out += b"\x00"
        out += blob
    open(dst, "wb").write(out)
    print("repacked %d region(s):" % nseg)
    for vaddr, flags, blob in blobs:
        print("  PT_LOAD vaddr=0x%08X size=0x%X flags=%d (FileSiz==MemSiz)"
              % (vaddr, len(blob), flags))
    print("entry=0x%08X  out=%s (%d bytes)" % (e_entry, dst, len(out)))

if __name__ == "__main__":
    main()

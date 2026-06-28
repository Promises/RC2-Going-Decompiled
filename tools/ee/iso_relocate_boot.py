#!/usr/bin/env python3
"""iso_relocate_boot.py — inject a boot ELF that is LARGER than the original
disc slot by RELOCATING the boot file's data to the end of the ISO and
repointing its ISO9660 directory record, instead of overwriting in place.

The original boot ELF (SCUS_972.68;1) sits in a fixed 1280-sector slot at LBA
1201; the file immediately after it is the IOP ROM image (RESET/ROMDIR), so a
2-sector-larger overlay image cannot be dd'd in place without corrupting it.
This tool appends the new ELF at the (sector-aligned) end of the disc image and
edits the directory record's extent LBA + data length (both-endian ISO9660
fields) so the PS2 BIOS loads the bigger ELF from the new location. The original
slot bytes are left untouched (harmless dead data).

Usage:
  iso_relocate_boot.py <pristine_iso> <elf> <out_iso> [boot_name_substr=SCUS]
"""
import sys, struct, shutil, os

SECTOR = 2048

def both_le_be(buf, off, val, width):
    """Write a value as ISO9660 both-endian (LE at off, BE at off+width)."""
    if width == 4:
        struct.pack_into("<I", buf, off, val)
        struct.pack_into(">I", buf, off + 4, val)
    else:
        raise ValueError(width)

def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    pristine, elf, out = sys.argv[1:4]
    namesub = (sys.argv[4] if len(sys.argv) > 4 else "SCUS").upper()

    if os.path.abspath(pristine) != os.path.abspath(out):
        print("copying %s -> %s ..." % (pristine, out))
        shutil.copyfile(pristine, out)

    elf_bytes = open(elf, "rb").read()
    f = open(out, "r+b")

    # PVD at sector 16; root dir record at PVD offset 156.
    f.seek(16 * SECTOR); pvd = bytearray(f.read(SECTOR))
    root = pvd[156:156 + 34]
    root_lba = struct.unpack("<I", root[2:6])[0]
    root_len = struct.unpack("<I", root[10:14])[0]

    # Walk the root directory for the boot record.
    f.seek(root_lba * SECTOR); d = bytearray(f.read(root_len))
    i = 0; rec_off = None; rec_len = None; name = None
    while i < len(d):
        L = d[i]
        if L == 0:
            i = (i // SECTOR + 1) * SECTOR; continue
        nlen = d[i + 32]; nm = d[i + 33:i + 33 + nlen].decode("latin1")
        if namesub in nm.upper():
            rec_off = i; rec_len = L; name = nm
        i += L
    if rec_off is None:
        sys.exit("boot record matching %r not found in root dir" % namesub)
    old_lba = struct.unpack("<I", d[rec_off + 2:rec_off + 6])[0]
    old_sz = struct.unpack("<I", d[rec_off + 10:rec_off + 14])[0]
    print("found %s: lba=%d size=%d (rec_off %d in root dir)" % (name, old_lba, old_sz, rec_off))

    # Append the ELF at the sector-aligned end of the disc image.
    f.seek(0, 2); end = f.tell()
    if end % SECTOR:
        pad = SECTOR - (end % SECTOR)
        f.write(b"\x00" * pad); end += pad
    new_lba = end // SECTOR
    f.write(elf_bytes)
    # pad the appended file to a sector boundary
    if len(elf_bytes) % SECTOR:
        f.write(b"\x00" * (SECTOR - (len(elf_bytes) % SECTOR)))
    new_total_sectors = f.tell() // SECTOR + (1 if f.tell() % SECTOR else 0)
    print("appended ELF: %d bytes at LBA %d (was LBA %d)" % (len(elf_bytes), new_lba, old_lba))

    # Patch the directory record: extent LBA + data length (both-endian).
    both_le_be(d, rec_off + 2, new_lba, 4)
    both_le_be(d, rec_off + 10, len(elf_bytes), 4)
    f.seek(root_lba * SECTOR); f.write(d)

    # Update PVD volume space size (both-endian at offset 80) so the image is
    # internally consistent.
    both_le_be(pvd, 80, new_total_sectors, 4)
    f.seek(16 * SECTOR); f.write(pvd)

    f.close()
    print("OK: %s now boots %s from LBA %d (size %d), vol=%d sectors"
          % (out, name, new_lba, len(elf_bytes), new_total_sectors))

if __name__ == "__main__":
    main()

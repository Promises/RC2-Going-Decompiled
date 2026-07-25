#!/usr/bin/env python3
"""
Locate SCE SDK libc/libm functions that appear *verbatim* in the Going Commando
binaries, by matching each library object's WHOLE function body.

This is the "the SDK has code" shortcut: functions linked from the SDK static
libraries (libc.a, libm.a, ...) are byte-identical in the game, so finding them
both *locates* and *validates* them (and pins the toolchain via known source).

MATCHING RULES (deliberately strict -- see the history note below):

1. The unit of comparison is a **symbol**, not an object file. Each `STT_FUNC`
   symbol in `.text` is carved out by `st_value`/`st_size` and reported under
   *its own name from the symbol table*. An object may define several functions
   (`rename.o` defines `_rename_r` AND `rename`) and the defined symbol need not
   equal the filename stem (`s_isnan.o` defines `isnan`).
2. A hit requires the **entire body** to agree -- every word, over the full
   `st_size`. A leading run that agrees is NOT a hit.
3. Words covered by a relocation are compared *masked*: the linker rewrites the
   target field, so only the non-relocated bits (opcode/registers) are required
   to agree. Everything outside a reloc must be exactly equal.

History / why this is strict: the original version fingerprinted only the
leading relocation-free run (capped at 96B) and then labelled the hit with the
*object filename stem*. That produced two independent classes of false
identification, both measured:
  - naming: `s_isnan.o` defines `isnan`; `rename.o`'s first body is `_rename_r`.
  - prefix-only: the 28B leading run of `_rename_r` coincides with an unrelated
    game function at 0x0011D3A0 (first 7 words equal, word 8 diverges; game body
    0xB0 vs 0x8C) -- reported for years as a verbatim `rename`. Neither SDK body
    from `rename.o` is linked into either boot ELF.
Note also that the vendored libc.a's r5900 string routines (`strcmp`, `strlen`,
`strncpy`) are a DIFFERENT REVISION from the ROM's -- same source file, but the
SIMD mask constants are built with different `li` expansions. Whole-body
matching is what makes that visible; a prefix match would have hidden it.

Prereq: extract the SDK objects first (run in the colima x86 VM):
    docker --context colima-ee-x86 run --rm -v "$PWD":/work ee-build sh -c \
      'cd /work/tools/ee/sdk-lib && mkdir -p obj && cd obj && ar x ../libc.a'

Then (host, in the decomp venv):
    python tools/ee/find_libc_funcs.py            # report
    python tools/ee/find_libc_funcs.py --verbose  # also list rejected near-misses
    python tools/ee/find_libc_funcs.py --emit-symbols  # append to symbol_addrs
"""
from __future__ import annotations
import glob, struct, sys
from pathlib import Path
from elftools.elf.elffile import ELFFile

ROOT = Path(__file__).resolve().parent.parent.parent
OBJ_DIRS = [ROOT / "tools/ee/sdk-lib/obj", ROOT / "tools/ee/sdk-lib/obj-m"]
SEG1_DELTA = 0xFF080  # file offset -> vram for the main PT_LOAD segment
TARGETS = {
    "usa": ROOT / "extracted/usa/SCUS_972.68",
    "eu": ROOT / "extracted/eu/SCES_516.07",
}
# A body must be at least this long to be trustworthy on its own. Short bodies
# (a couple of instructions) collide with unrelated code by chance.
MIN_BODY = 28

# A body must also carry this many DISTINCTIVE words -- words that are neither
# relocated nor generic frame boilerplate. Without this, every trivial
# "set up frame / jal something / tear down frame" thunk matches every other
# one once the jal target is masked: 12 unrelated libc objects all "matched"
# the same 28B address (0x0027C0A8) that way.
MIN_DISTINCT = 4

# Generic prologue/epilogue words that carry no identifying information.
def _is_boilerplate(w: int) -> bool:
    op = w >> 26
    if w in (0x00000000, 0x03E00008):          # nop, jr $ra
        return True
    if (w & 0xFFFF0000) == 0x27BD0000:          # addiu $sp, $sp, imm
        return True
    if op in (0x3F, 0x37) and (w >> 21 & 0x1F) == 29:   # sd/ld off($sp)
        return True
    if op in (0x2B, 0x23) and (w >> 21 & 0x1F) == 29:   # sw/lw off($sp)
        return True
    if (w & 0xFFE0FFFF) == 0x0000102D:          # daddu $2, rs, $0  (move)
        return True
    return False

# MIPS reloc types -> mask of the bits the LINKER may rewrite in that word.
# Words under a reloc are compared with these bits cleared on both sides.
RELOC_WRITE_MASK = {
    "R_MIPS_26": 0x03FFFFFF,
    "R_MIPS_HI16": 0x0000FFFF,
    "R_MIPS_LO16": 0x0000FFFF,
    "R_MIPS_GPREL16": 0x0000FFFF,
    "R_MIPS_LITERAL": 0x0000FFFF,
    "R_MIPS_PC16": 0x0000FFFF,
    "R_MIPS_GOT16": 0x0000FFFF,
    "R_MIPS_CALL16": 0x0000FFFF,
    "R_MIPS_32": 0xFFFFFFFF,
}


def carve_symbols(path: Path):
    """Yield (symbol_name, body_bytes, {word_index: write_mask}) per .text FUNC."""
    with path.open("rb") as fh:
        elf = ELFFile(fh)
        text = elf.get_section_by_name(".text")
        if text is None:
            return
        data = bytes(text.data())
        tidx = next(
            (i for i, s in enumerate(elf.iter_sections()) if s.name == ".text"), None
        )

        # offset -> write mask, for every relocation applying to .text
        relmask: dict[int, int] = {}
        for sname in (".rel.text", ".rela.text"):
            rs = elf.get_section_by_name(sname)
            if rs is None:
                continue
            for r in rs.iter_relocations():
                rtype = r["r_info_type"]
                name = next(
                    (k for k, v in _MIPS_RTYPE.items() if v == rtype), None
                )
                mask = RELOC_WRITE_MASK.get(name, 0xFFFFFFFF)
                off = r["r_offset"]
                relmask[off] = relmask.get(off, 0) | mask

        symtab = elf.get_section_by_name(".symtab")
        if symtab is None:
            return
        for sym in symtab.iter_symbols():
            if sym["st_info"]["type"] != "STT_FUNC" or sym["st_shndx"] != tidx:
                continue
            base, size = sym["st_value"], sym["st_size"]
            if size < 8 or base + size > len(data):
                continue
            body = data[base : base + size]
            masks = {
                (o - base) // 4: m
                for o, m in relmask.items()
                if base <= o < base + size
            }
            yield sym.name, body, masks


_MIPS_RTYPE = {
    "R_MIPS_NONE": 0, "R_MIPS_16": 1, "R_MIPS_32": 2, "R_MIPS_REL32": 3,
    "R_MIPS_26": 4, "R_MIPS_HI16": 5, "R_MIPS_LO16": 6, "R_MIPS_GPREL16": 7,
    "R_MIPS_LITERAL": 8, "R_MIPS_GOT16": 9, "R_MIPS_PC16": 10,
    "R_MIPS_CALL16": 11, "R_MIPS_GPREL32": 12,
}


def _words(buf: bytes) -> list[int]:
    return list(struct.unpack_from("<%dI" % (len(buf) // 4), buf, 0))


def find_body(blob: bytes, body: bytes, masks: dict[int, int]) -> int | None:
    """Return the file offset where the WHOLE body matches (reloc-masked), else None."""
    ow = _words(body)
    if not ow:
        return None
    # Anchor the scan on the longest run of non-relocated words, so the fast
    # bytes.find() path stays exact; then verify every word under the mask.
    best_start, best_len, run = 0, 0, 0
    for i in range(len(ow)):
        run = 0 if i in masks else run + 1
        if run > best_len:
            best_len, best_start = run, i - run + 1
    if best_len == 0:
        return None
    anchor = body[best_start * 4 : (best_start + best_len) * 4]

    pos = 0
    while True:
        hit = blob.find(anchor, pos)
        if hit < 0:
            return None
        start = hit - best_start * 4
        pos = hit + 4
        if start < 0 or start + len(body) > len(blob):
            continue
        cand = _words(blob[start : start + len(body)])
        if all(
            (a & ~masks.get(i, 0)) == (b & ~masks.get(i, 0))
            for i, (a, b) in enumerate(zip(ow, cand))
        ):
            return start


def main() -> None:
    emit = "--emit-symbols" in sys.argv
    verbose = "--verbose" in sys.argv
    bins = {k: v.read_bytes() for k, v in TARGETS.items()}

    rows, rejected = [], []
    seen: set[tuple] = set()
    for d in OBJ_DIRS:
        for o in sorted(glob.glob(str(d / "*.o"))):
            for name, body, masks in carve_symbols(Path(o)):
                # libc.a and libm.a share members, and obj/ + obj-m/ are both
                # scanned -- collapse identical (symbol, bytes) pairs.
                key = (name, body)
                if key in seen:
                    continue
                seen.add(key)
                iu = find_body(bins["usa"], body, masks)
                if iu is None:
                    continue
                ie = find_body(bins["eu"], body, masks)
                distinct = sum(
                    1
                    for i, w in enumerate(_words(body))
                    if i not in masks and not _is_boilerplate(w)
                )
                row = (
                    iu + SEG1_DELTA,
                    name,
                    Path(o).name,
                    (ie + SEG1_DELTA) if ie is not None else None,
                    len(body),
                    len(masks),
                    distinct,
                )
                ok = len(body) >= MIN_BODY and distinct >= MIN_DISTINCT
                (rows if ok else rejected).append(row)
    rows.sort()
    rejected.sort()

    print(f"# {len(rows)} SDK library functions located verbatim (whole body, USA)")
    for v, name, obj, ve, n, nr, nd in rows:
        eu = "EU 0x%08x" % ve if ve else "EU --"
        rel = f", {nr} reloc" if nr else ""
        print(f"  0x{v:08x}  {name:16} {eu:17} ({n}B{rel})  [{obj}]")

    if verbose:
        print(
            f"\n# {len(rejected)} whole-body hits REJECTED "
            f"(need >={MIN_BODY}B and >={MIN_DISTINCT} distinctive words)"
        )
        for v, name, obj, ve, n, nr, nd in rejected:
            print(f"  0x{v:08x}  {name:16} ({n}B, {nd} distinctive)  [{obj}]")

    if emit:
        for region in ("usa", "eu"):
            path = ROOT / f"going-decompiled/symbol_addrs/{region}/symbol_addrs.txt"
            existing = path.read_text() if path.exists() else ""
            lines = []
            for v, name, obj, ve, n, nr, nd in rows:
                addr = v if region == "usa" else ve
                if addr is None or name in existing:
                    continue
                lines.append(
                    f"{name} = 0x{addr:08X}; // {obj} (verbatim, whole {n}B body verified)"
                )
            if lines:
                with path.open("a") as f:
                    f.write("\n" + "\n".join(lines) + "\n")
                print(f"emitted {len(lines)} symbols -> {path}")


if __name__ == "__main__":
    main()

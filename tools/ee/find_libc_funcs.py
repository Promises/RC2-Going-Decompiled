#!/usr/bin/env python3
"""
Locate SCE SDK libc/libm functions that appear *verbatim* in the Going Commando
binaries, by matching each library object's relocation-free leading bytes.

This is the "the SDK has code" shortcut: functions linked from the SDK static
libraries (libc.a, libm.a, ...) are byte-identical in the game, so finding them
both *locates* and *validates* them (and pins the toolchain via known source).

Prereq: extract the SDK objects first (run in the colima x86 VM):
    docker --context colima-ee-x86 run --rm -v "$PWD":/work ee-build sh -c \
      'cd /work/tools/ee/sdk-lib && mkdir -p obj && cd obj && ar x ../libc.a'

Then (host, in the decomp venv):
    python tools/ee/find_libc_funcs.py            # report
    python tools/ee/find_libc_funcs.py --emit-symbols  # append to symbol_addrs

Short prefixes (<28B) are reported but not emitted (risk of false positives).
"""
from __future__ import annotations
import glob, sys
from pathlib import Path
from elftools.elf.elffile import ELFFile

ROOT = Path(__file__).resolve().parent.parent.parent
OBJ_DIR = ROOT / "tools/ee/sdk-lib/obj"
SEG1_DELTA = 0xFF080  # file offset -> vram for the main PT_LOAD segment
TARGETS = {
    "usa": ROOT / "extracted/usa/SCUS_972.68",
    "eu": ROOT / "extracted/eu/SCES_516.07",
}
EMIT_MIN = 28  # only emit symbols whose verbatim prefix is at least this long


def leading_relocfree(path: Path) -> bytes | None:
    elf = ELFFile(path.open("rb"))
    text = elf.get_section_by_name(".text")
    if not text:
        return None
    data = bytes(text.data())
    if len(data) < 16:
        return None
    rs = elf.get_section_by_name(".rel.text") or elf.get_section_by_name(".rela.text")
    first = min((r["r_offset"] for r in rs.iter_relocations()), default=len(data)) if rs else len(data)
    lim = min(first, 96, len(data))
    return data[:lim] if lim >= 16 else None


def main() -> None:
    emit = "--emit-symbols" in sys.argv
    bins = {k: v.read_bytes() for k, v in TARGETS.items()}
    rows = []
    for o in sorted(glob.glob(str(OBJ_DIR / "*.o"))):
        p = leading_relocfree(Path(o))
        if not p:
            continue
        iu = bins["usa"].find(p)
        if iu < 0:
            continue
        ie = bins["eu"].find(p)
        rows.append((iu + SEG1_DELTA, Path(o).stem, (ie + SEG1_DELTA) if ie >= 0 else None, len(p)))
    rows.sort()
    print(f"# {len(rows)} SDK libc functions located verbatim (USA)")
    for v, name, ve, n in rows:
        tag = "" if n >= EMIT_MIN else "  [short/uncertain]"
        print(f"  0x{v:08x}  {name:16} {'EU 0x%08x'%ve if ve else 'EU --':17} ({n}B){tag}")

    if emit:
        for region, off_key in (("usa", 0), ("eu", 1)):
            path = ROOT / f"going-decompiled/symbol_addrs/{region}/symbol_addrs.txt"
            existing = path.read_text() if path.exists() else ""
            lines = []
            for v, name, ve, n in rows:
                if n < EMIT_MIN:
                    continue
                addr = v if region == "usa" else ve
                if addr is None:
                    continue
                line = f"{name} = 0x{addr:08X}; // libc.a (verbatim, {n}B-verified)"
                if name not in existing:
                    lines.append(line)
            if lines:
                with path.open("a") as f:
                    f.write("\n" + "\n".join(lines) + "\n")
                print(f"emitted {len(lines)} symbols -> {path}")


if __name__ == "__main__":
    main()

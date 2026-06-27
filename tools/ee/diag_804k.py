#!/usr/bin/env python3
"""diag_804k.py — DISCRIMINATOR (classification, not a fix) for the C-only boot's
0x1AA9C8->0x352D08 data divergence. Repacks a linked ELF, diffs its main segment
vs the original boot ELF over the region, and reports:
  1) HISTOGRAM of (orig-ours) deltas: structured small const / displaced block
     -> LAYOUT/SIZE symptom; scattered/random -> CONTENT.
  2) CLASSIFY each diverging word: looks like a code/data POINTER (0x00100000..
     0x01900000 or the 0x00C00000 calt-overlay) vs a small literal/flag/non-ptr
     -> all-pointer = size-symptom (baked linked addrs); mixed/non-ptr = genuine
     missing/wrong static data.
  3) calt-overlay (->0x00C00000) pointers are EXPECTED for the C-only image.
Usage: diag_804k.py <linked.elf>   (repacks via elf_contiguous_repack.py)
"""
import sys, struct, subprocess, os, collections, tempfile
ORIG = "extracted/usa/SCUS_972.68"
LO, HI = 0x1AA9C8, 0x352D08
BASE = 0x100080

def seg0(path):
    from elftools.elf.elffile import ELFFile
    e = ELFFile(open(path, "rb"))
    for s in e.iter_segments():
        if s["p_type"] == "PT_LOAD" and s["p_vaddr"] == BASE:
            f = open(path, "rb"); f.seek(s["p_offset"]); return f.read(s["p_filesz"])
    raise SystemExit("no 0x100080 PT_LOAD in " + path)

def is_ptr(w):
    return 0x00100000 <= w < 0x01900000

def main():
    elf = sys.argv[1]
    rp = tempfile.mktemp(suffix=".elf")
    subprocess.run([sys.executable, "tools/ee/elf_contiguous_repack.py", elf, rp],
                   check=True, capture_output=True)
    orig, ours = seg0(ORIG), seg0(rp)
    n = min(len(orig), len(ours))
    deltas = collections.Counter()
    ptr_diff = nonptr_diff = calt = 0
    total = 0
    examples = []
    for v in range(LO, HI, 4):
        i = v - BASE
        if i + 4 > n:
            break
        a = struct.unpack("<I", orig[i:i+4])[0]
        b = struct.unpack("<I", ours[i:i+4])[0]
        if a == b:
            continue
        total += 1
        if 0x00C00000 <= b < 0x00C30000:
            calt += 1
        elif is_ptr(a) and is_ptr(b):
            ptr_diff += 1; deltas[a-b] += 1
        else:
            nonptr_diff += 1; deltas[("nonptr", a-b if abs(a-b) < 0x10000 else "big")] += 1
        if len(examples) < 12:
            examples.append((v, a, b))
    print("region 0x%X-0x%X: %d differing words" % (LO, HI, total))
    print("  POINTER->POINTER shifts : %d" % ptr_diff)
    print("  ->calt-overlay (EXPECTED): %d" % calt)
    print("  NON-pointer / content    : %d" % nonptr_diff)
    print("  --- delta histogram (top 15; +const = ours low / -const = ours high) ---")
    for d, c in deltas.most_common(15):
        ds = ("%+#x" % d) if isinstance(d, int) else str(d)
        print("    %-12s : %d" % (ds, c))
    print("  --- examples (vaddr: orig -> ours) ---")
    for v, a, b in examples:
        tag = "calt" if 0xC00000 <= b < 0xC30000 else ("ptr" if is_ptr(a) and is_ptr(b) else "NONPTR")
        print("    0x%X: %08x -> %08x  [%s]" % (v, a, b, tag))
    # verdict
    real = ptr_diff + nonptr_diff
    print("  === VERDICT ===")
    if real == 0:
        print("  CLEAN (only calt-overlay) -> base static is byte-exact; 804K is C-only "
              "overlay pointers (expected) or RUNTIME-init (needs tester's compare method).")
    elif nonptr_diff == 0:
        struct_frac = sum(c for d, c in deltas.items() if isinstance(d, int)) / max(real, 1)
        print("  ALL real diffs are POINTER->POINTER, %.0f%% structured const shifts -> "
              "LAYOUT/SIZE symptom (unit-size shift baking wrong linked addrs), NOT content."
              % (100*struct_frac))
    else:
        print("  %d NON-pointer diffs present -> GENUINE missing/wrong static CONTENT "
              "(not just a size symptom)." % nonptr_diff)
    os.unlink(rp)

if __name__ == "__main__":
    main()

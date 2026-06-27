#!/usr/bin/env python3
"""
conly_overlay_ld.py — generate the C-only-boot overlay linker script with the
calt (TARGET_NATIVE) objects ISOLATED into a fixed high-VRAM catch-all, so the
overlay adds ZERO bytes to the base flowing layout.

WHY (the gp-window perturbation bug):
  The raw-asm %gp_rel set fits entirely in the +-32KB gp window (43KB span, 0
  out-of-window) and the BASE link is clean — but the low window edge has only
  ~400 bytes of slack (lowest gp_rel sym 0x1A7180 vs floor _gp-0x8000=0x1A6FF0).
  The earlier overlay injected each `<unit>.calt.o(<sec>)` INLINE before its base
  `<unit>.o(<sec>)`, and routed orphan calt .rodata into a *flowing* catch-all.
  Those inline/flowing bytes slid the base small-data/bss layout, pushing the
  edge gp-window globals past the window -> R_MIPS_GPREL16 truncations. It was
  never the gp ceiling; it was overlay-induced placement drift of a layout that
  already fits.

THE FIX (this script):
  Emit ONE loadable `.calt_overlay` block + one NOLOAD `.calt_overlay_bss` block,
  placed FIRST in SECTIONS{} (so calt's func_<addr> wins symbol resolution under
  --allow-multiple-definition: script-order-first wins — verified) but at a FIXED
  high VRAM ABOVE the main image and below the 0x01800000 DVP/VU overlay region.
  The rest of the base .ld is copied VERBATIM — no inline injection — so every
  base section (and thus every gp_rel symbol) keeps its exact clean-link address.
  calt code is compiled -G0 (absolute lui/%hi-%lo addressing), so it runs
  correctly from any address and its `jal func_<addr>` targets stay in-region.

  CHURN-FREE for the matching build: this only rewrites the C-only overlay .ld;
  build.sh and the per-unit objdiff matching path are untouched.

Usage:
  conly_overlay_ld.py <base.ld> <calt_list.txt> <out.ld> [calt_vram_hex]
    base.ld       the region's matching linker script (verbatim base layout)
    calt_list.txt one `$BUILD/.../<unit>.calt.o` path per line (build_conly output)
    out.ld        overlay .ld to write
    calt_vram_hex optional VMA for the catch-all (default 0x00C00000) — must be
                  above the main image's text_VRAM_END and below 0x01800000.
"""
import sys, os, re

LOAD_SECS = (".text* .rodata* .rodata.* .data* .sdata* .sdata2* "
             ".lit8 .lit4 .gcc_except_table*")
BSS_SECS = ".sbss* .scommon .bss COMMON"


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    base_ld, listf, out_ld = sys.argv[1:4]
    calt_vram = int(sys.argv[4], 16) if len(sys.argv) > 4 else 0x00C00000

    calt_objs = []
    if os.path.exists(listf):
        for ln in open(listf):
            ln = ln.strip()
            if ln.endswith(".calt.o"):
                calt_objs.append(ln)

    load_lines = "\n".join("        %s(%s);" % (o, LOAD_SECS) for o in calt_objs)
    bss_lines = "\n".join("        %s(%s);" % (o, BSS_SECS) for o in calt_objs)

    block = []
    block.append("    /* ---- C-only-boot calt isolation (conly_overlay_ld.py) ---- */")
    block.append("    /* FIRST in script order -> calt func_<addr> wins symbol")
    block.append("       resolution (--allow-multiple-definition: script-order-first");
    block.append("       wins). Fixed high VRAM -> the base low layout is preserved")
    block.append("       byte-for-byte, so every gp_rel symbol keeps its address. */")
    block.append("    .calt_overlay 0x%08X : {" % calt_vram)
    block.append("        __calt_overlay_start = .;")
    block.append(load_lines if load_lines else "        /* (no calt objects) */")
    block.append("        . = ALIGN(., 16);")
    block.append("        __calt_overlay_end = .;")
    block.append("    }")
    block.append("    .calt_overlay_bss (NOLOAD) : {")
    block.append(bss_lines if bss_lines else "        /* (no calt bss) */")
    block.append("        . = ALIGN(., 16);")
    block.append("    }")
    block.append("    /* ---- end calt isolation ---- */")
    block_txt = "\n".join(block) + "\n"

    # Inject the calt blocks immediately after the opening `SECTIONS {` and the
    # leading `_gp` assignment line, so they precede the first base section.
    src = open(base_ld).read()
    lines = src.splitlines(keepends=True)

    # ---- Pin .cod_bss to its true VRAM anchor ------------------------------
    # .cod_bss is NOLOAD and FLOWS from cod_RODATA_END, but the cod text/data
    # layout comes up 0x720 short, so the flowing base lands the whole bss block
    # (and the gp-window globals DEFINED-with-storage inside it:
    # g_nSaveLoadStatusCode/g_vramDynamicBase/...) ~0x720 below their real
    # addresses — past the gp window floor -> R_MIPS_GPREL16 truncations (the
    # symbol_addrs PROVIDE never fires because the symbols are defined here, not
    # undefined). The anchor is encoded in the cod bss object name
    # `data/cod/<HEX>.bss.o` (USA 0x0013C080, EU 0x0013C100); 0x<HEX> +
    # cod_BSS_SIZE lands exactly on core_lit. Pinning .cod_bss to it puts every
    # in-bss global back at its real address. This is the C-only overlay .ld
    # only — churn-free for the matching build (matching is per-unit objdiff and
    # never uses the full .ld). NOTE: the upstream 0x720 cod text/data deficit is
    # a separate latent splat-layout issue; it doesn't affect matching (functions
    # are absolute-pinned).
    cod_bss_anchor = None
    for ln in lines:
        m = re.search(r"/cod/0*([0-9A-Fa-f]+)\.bss\.o", ln)
        if m:
            cod_bss_anchor = int(m.group(1), 16)
            break
    if cod_bss_anchor is not None:
        pinned = 0
        for i, ln in enumerate(lines):
            m = re.match(r"^(\s*)\.cod_bss\s+\(NOLOAD\)\s*:(.*)$", ln)
            if m:
                lines[i] = "%s.cod_bss 0x%08X (NOLOAD) :%s\n" % (
                    m.group(1), cod_bss_anchor, m.group(2).rstrip())
                pinned += 1
        print("   pinned .cod_bss to 0x%08X (%d header)" % (cod_bss_anchor, pinned))

    # ---- Match the original ROM's 8-byte .text unit packing -----------------
    # The per-unit `.o` .text sections are gas-default 2**4 (16)-aligned, but
    # cc1 emits `.p2align 3` (8) and the ORIGINAL ROM packs text units at 8:
    # all 22 text units start 8-aligned, but only 10 are 16-aligned -- the 12
    # that are 8-but-not-16 (e.g. 1740A8 @ 0x274128, 176168 @ 0x2761E8) are
    # impossible under 16-byte alignment, proving the original granularity is 8.
    # With no SUBALIGN the linker force-aligns each unit's .text to 16, inserting
    # +8 inter-unit padding the original lacks (8 such boundaries before 1D54C0
    # = +0x40, masked by -0x30 of splat-short units -> net +0x10). That floats
    # every downstream unit up, so the baked `.word <func>` pointer tables in
    # .lit/.data resolve to wrong (too-high) addresses -- the ~764K C-only data
    # residual; nm confirms func_002D6248 links at 0x2D6258 (+0x10). SUBALIGN(8)
    # forces 8-byte input alignment so units pack at their original positions.
    # C-only overlay .ld ONLY -- the matching full-ROM link script is untouched.
    # (The .cod text segment is already byte-exact: its 3 units happen to be
    # 16-aligned, so this is scoped to the .text segment that actually drifts.)
    for i, ln in enumerate(lines):
        m = re.match(r"^(\s*\.text\s+0x[0-9A-Fa-f]+\s*:\s*AT\([^)]*\))(.*)$", ln)
        if m:
            lines[i] = "%s SUBALIGN(8)%s\n" % (m.group(1), m.group(2).rstrip())
            print("   set .text SUBALIGN(8) (match original 8-byte unit packing)")
            break

    inj = None
    for i, ln in enumerate(lines):
        if "_gp" in ln and "=" in ln:
            inj = i + 1
            break
    if inj is None:  # fall back to right after `SECTIONS {`
        for i, ln in enumerate(lines):
            if ln.strip().startswith("SECTIONS"):
                inj = i + 1
                break
    if inj is None:
        sys.exit("conly_overlay_ld: could not find SECTIONS{/_gp injection point")

    out = "".join(lines[:inj]) + block_txt + "".join(lines[inj:])
    open(out_ld, "w").write(out)
    print("   wrote %s: %d calt objects isolated at 0x%08X (base layout verbatim)"
          % (out_ld, len(calt_objs), calt_vram))


if __name__ == "__main__":
    main()

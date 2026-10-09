# Credits

## References

- **Made in Slovakia — Ratchet & Clank PCSX2 cheat codes**
  <https://github.com/Made-in-Slovakia/rac>

  Their cheat codes for *Going Commando* USA v2.00 (CRC `B3A71D10`) gave this
  project its first naming anchors. Each cheat writes to an exact address with a
  known meaning (bolts, health, ammo, unlock flags), which named many globals
  before any code around them had been read. `tools/ee/pnach_to_symbols.py`
  turned them into naming hypotheses, and each one was confirmed against the
  code that writes it before entering `going-decompiled/symbol_addrs/`.

  The cheat files themselves are not redistributed here. See the link above.

- **OpenRAC — `games/rac2/ntsc`** (MIT, © 2026 llesieur99)
  <https://github.com/OpenRAC/OpenRAC/tree/421126411e3453176c85eb7dcde8190c5f044315/games/rac2/ntsc>
  at commit `421126411e3453176c85eb7dcde8190c5f044315`

  Their decompilation of the USA v1.01 boot gave this project identities for
  SDK functions, and their research notes are references for the level
  overlays. Every item was re-derived on our own USA v2.00 ROM before use; their
  name is a hypothesis here, never the evidence. Their C is read as a semantics
  reference only. No OpenRAC code, text or table is copied into this tree.

  Used so far:
  - **Identities for three SDK functions**, which went into the doc comments of
    functions that were already matched: `FlushCache` (`0x0011AEA0`, which this
    project already held from the ROM), `SetD3Chcr` (`0x00130AB0`) and
    `SetD4Chcr` (`0x00130B18`). The last two spellings appear in OpenRAC as
    externs in a file that OpenRAC credits to Lombyte.
  - **References for the level overlays** (not yet in scope here): their
    level-archive format, their code-reuse families, and the `.DVP.ovlytab`
    record layout.

  Each adopted item carries a `Source:` line in the code naming the file and
  commit it came from. The licence is pinned to the commit above because the
  OpenRAC repository mixes licences: its top level is GPL-3.0-or-later, and
  `games/rac2/ntsc/src/libgcc` is GPLv2. Nothing from those paths is used here.

- **Lombyte** (MIT, © 2026 Mateusz Kłysz)
  <https://github.com/mateuszklysz/Lombyte> at commit `2c4452dd03f5f7ebb2868dbe073ccdb1afb27f61`

  The spellings `SetD3Chcr` and `SetD4Chcr` that OpenRAC uses come from
  Lombyte's symbol names. Only those names were taken; both identities were
  re-derived from the DMA channel registers these functions write in our ROM.
  No Lombyte code is copied into this tree.

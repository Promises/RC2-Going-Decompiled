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
  - **The name `sceIpuRestartDMA`** (`0x00130C68`), adopted when that function
    was promoted (task #2048). It came from their boot function catalogue and
    was re-derived on our ROM: the function restarts DMA channels 3 and 4
    through `SetD3Chcr` and `SetD4Chcr`.
  - **References for the level overlays** (not yet in scope here): their
    level-archive format, their code-reuse families, and the `.DVP.ovlytab`
    record layout.

  Each adopted item carries a `Source:` line in the code naming the file and
  commit it came from. The licence is pinned to the commit above because the
  OpenRAC repository mixes licences: its top level is GPL-3.0-or-later, and
  `games/rac2/ntsc/src/libgcc` is GPLv2. Nothing from those paths is used here.

- **OpenRAC — `rac2-gc-decomp`, later commit** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `d43c3b99b984fb63556c844a21ee99e8e31d2deb`

  Three more libmpeg C bodies from the same repository, adopted with the
  authors' permission (held by the operator, RULING #10405) and re-checked
  byte-exact against our v2.00 ROM (task #2206). Contributors: Louis-Philippe
  Le Sieur (llesieur99) and Gianni. Each doc comment carries the `Source:`
  line, which names every change beyond extern spellings.

  Used so far:
  - **`func_001296A0`** (`0x001296A0`) in `going-decompiled/src/usa/cod/022FA8.c`,
    from `src/core/00129450.c` (author commit `4b02f2de`, Louis-Philippe Le
    Sieur); upstream of that, rac1-decomp `src/core/00125630.c` `func_00125880`.
  - **`func_0012DAC0`** (`0x0012DAC0`) in the same file, from
    `src/core/0012DA98.c` (author commit `3451db40`, Gianni), carried from
    rac1-decomp `src/core/00125630.c` `func_00129CA0`.
  - **`func_0012DD60`** (`0x0012DD60`) in the same file, adapted, from
    `src/core/0012DD60.c` (author commit `962455ea`, Louis-Philippe Le Sieur),
    which their header says is adapted from **Lombyte** (MIT)
    `src/sdk/library/disprefimagefield.c`; see the Lombyte entry below.

- **OpenRAC — `rac2-gc-decomp`** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `974302ba936bb455acd78034b58ad4b98bafcc6b`
  (five bodies, task #2159) and at commit `8b490f56b8e46a1b749b54fd898946a4f09c1cca`
  (two bodies, tasks #2163 and #2164)

  Their decompilation of the RAC2 v1.01 boot. Seven C bodies are adopted, with
  the authors' permission (held by the operator, RULING #10405), each
  re-checked byte-exact against our v2.00 ROM and with their v1.01 extern
  spellings replaced by our symbols. Five are verbatim apart from spellings
  (task #2159); `IpuGetBits` takes our shared prototype (task #2163) and
  `func_00293760` is adapted (`long` changed to `s64`, task #2164). Each doc
  comment carries the `Source:` line, which names the commit that body came
  from and what was changed.

  Used so far:
  - **`func_00131850` and `func_00131908`** (`0x00131850`, `0x00131908`) in
    `going-decompiled/src/usa/cod/0314C0.c`, from `src/core/00131780.c`. These
    two are rac1-decomp's own C, carried into rac2-gc-decomp (rac1-decomp
    `src/core/0012CC90.c`: `func_0012D5D0`, `func_0012D688`).
  - **`MatrixToEulerAngles`** (`0x002AC540`) in
    `going-decompiled/src/usa/text/1A8180.c`, from `src/game/002AA800.c`
    (`func_002AB6D8`).
  - **`InitMenuBgImageBuffers`** (`0x002DAF90`) and **`func_002DCDC0`**
    (`0x002DCDC0`) in `going-decompiled/src/usa/text/1D54C0.cpp`, from
    `src/game/002D5E98.c` (`func_002DA238`, `func_002DC090`).
  - **`IpuGetBits`** (`0x0012C878`) in `going-decompiled/src/usa/cod/022FA8.c`,
    from `src/core/0012C508.c` (`func_0012C878`) at the later commit
    `8b490f56b8e46a1b749b54fd898946a4f09c1cca`. This is rac1-decomp's own C
    (`src/core/00125630.c`: `func_00128A58`), carried into rac2-gc-decomp; the
    signature takes our shared prototype (task #2163).
  - **`func_00293760`** (`0x00293760`) in
    `going-decompiled/src/usa/text/191238.cpp`, from `src/game/00291758.c`
    (`func_00292EB0`) at `8b490f56`. This is rac1-decomp's C
    (`src/game/loaders.c`: `func_00203808`), carried into rac2-gc-decomp.
    Adapted: every `long` is changed to `s64` (the same type on the EE).

- **OpenRAC — `rac2-gc-decomp`, commit `d43c3b99`, `_exitThunk` and `func_0027F348`** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `d43c3b99b984fb63556c844a21ee99e8e31d2deb`

  Two C bodies, adopted with the authors' permission (held by the operator,
  RULING #10405) and re-checked byte-exact against our v2.00 ROM: one core body
  on the plain sdk29 arm (task #2208) and one engine body on the s136os arm
  (task #2216). Contributors: Gianni and Flavius. Each doc comment carries the
  `Source:` line.

  Used so far:
  - **`_exitThunk`** (`0x00131CA8`) in `going-decompiled/src/usa/cod/0314C0.c`,
    from `src/core/00131780.c` (`func_00131CA8`, author commit `302ea406`).
  - **`func_0027F348`** (`0x0027F348`, DrawShadowedBoxOutline) in
    `going-decompiled/src/usa/text/178E88.cpp`, from `src/game/0027B948.c`
    (`func_0027ECC8`, author commit `4a78f7b7`, Flavius), compiled on the
    s136os arm (task #2216).

- **OpenRAC — `rac2-gc-decomp`, commit `d43c3b99`, s136os bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `d43c3b99b984fb63556c844a21ee99e8e31d2deb`

  Two engine C bodies, adopted with the authors' permission (held by the
  operator, RULING #10405) and re-checked byte-exact against our v2.00 ROM on
  the s136os arm (task #2207). Contributor: ThorpenCodes. Each doc comment
  carries the `Source:` line, which names every change beyond extern spellings.

  Used so far:
  - **`BuildAttractReelPlaylist`** (`0x0026EC58`) in
    `going-decompiled/src/usa/text/16E980.c`, from `src/game/0026EA00.c`
    (`func_0026EC58`, author commit `8850b66a`).
  - **`ApplyMobyLocalTransformDelta`** (`0x002B4FA8`) in
    `going-decompiled/src/usa/text/1B4218.cpp`, adapted, from
    `src/game/002B3868.c` (`func_002B4090`, author commit `019bc7a7`).

- **Lombyte** (MIT, © 2026 Mateusz Kłysz)
  <https://github.com/mateuszklysz/Lombyte> at commit `2c4452dd03f5f7ebb2868dbe073ccdb1afb27f61`

  The spellings `SetD3Chcr`, `SetD4Chcr` and `sceIpuRestartDMA` that OpenRAC
  uses come from Lombyte (`src/sdk/dma/sce_ipu_restart_dma.c` for the last).
  Each identity was re-derived from what the function does in our ROM.

  Used so far:
  - **The C body and name of `_sysbitFlush`** (`0x0012E8E8`), from
    `src/sdk/debug/sysbit_flush.c`, adopted when that function was promoted
    (task #2134) under the operator's ruling that Lombyte C may be used
    verbatim when it is byte-exact on our ROM. Two unused permuter temporaries
    were dropped; the body is otherwise Lombyte's. Its doc comment in
    `going-decompiled/src/usa/cod/022FA8.c` carries the `Source:` line.

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

  Four engine C bodies, adopted with the authors' permission (held by the
  operator, RULING #10405) and re-checked byte-exact against our v2.00 ROM on
  the s136os arm (tasks #2207, #2217 and #2218). Contributors: ThorpenCodes and
  Flavius. Each doc comment
  carries the `Source:` line, which names every change beyond extern spellings.

  Used so far:
  - **`BuildAttractReelPlaylist`** (`0x0026EC58`) in
    `going-decompiled/src/usa/text/16E980.c`, from `src/game/0026EA00.c`
    (`func_0026EC58`, author commit `8850b66a`).
  - **`ApplyMobyLocalTransformDelta`** (`0x002B4FA8`) in
    `going-decompiled/src/usa/text/1B4218.cpp`, adapted, from
    `src/game/002B3868.c` (`func_002B4090`, author commit `019bc7a7`).
  - **`func_00352058`** (`0x00352058`, the FMV DMA-add slot-ring trim) in
    `going-decompiled/src/usa/text/250080.cpp`, from `src/game/003512B8.c`
    (`func_00351310`, author commit `455cde53`, Flavius), with its
    `Transfer_3518F0` typedef and `ring_mod` helper. ADAPTED: two `long long`
    are respelled `long` (128-bit on s136, 64-bit upstream), the upstream
    forward declaration is dropped, and the signature takes our shared
    prototype (task #2218).
  - **`func_003011C0`** (`0x003011C0`, the turret-registry target ranking) in
    `going-decompiled/src/usa/text/1FFBA0.cpp`, from `src/game/002FC8D0.c`
    (`func_00300330`, author commit `f06fefe2`, ThorpenCodes), with its
    `State00300330` typedef. ADAPTED: the callee and the table-origin spelling
    are ours (their `D_00190000 - 0x6160` origin is our symbol, so the reloc
    symbol and addend move; linked bytes identical) (task #2217).

- **OpenRAC — `rac2-gc-decomp`, commit `96927f87`, s136os bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  Eighteen engine C bodies, two in text/16E980, one in text/1A00F0, one in
  text/1A8180, three in text/1B4218, one in text/1D54C0, two in text/1DFF80,
  two in text/188858, two in cod/0321A0, one in text/235FE8, one in
  text/248B50 and two in text/250080, adopted with the authors' permission
  (held by the operator, RULING #10405) and re-checked byte-exact against our
  v2.00 ROM on the s136os arm (tasks #2232, #2234, #2235, #2248, #2251, #2252,
  #2262, #2294, #2304, #2305, #2306 and #2320, from audit NOTE #12089 rows 3,
  4, 5, 6, 8 and 9, NOTE #12161, NOTE #12173 and NOTE #12202).
  Contributors: bigbeno37, Flavius, Louis-Philippe Le Sieur, ThorpenCodes and Lynder063. Each doc comment carries the `Source:`
  line, which names every change beyond extern spellings.

  Used so far:
  - **`ApplyCameraShakeAxis`** (`0x00271968`) in
    `going-decompiled/src/usa/text/16E980.c`, from `src/game/00271958-g8.c`
    (`func_00271958`, v1.01 address, author commit `d4ac4036`, bigbeno37).
    ADAPTED: their camera block `D_001B5200` is our `g_cameraState`, with our
    field names (a `matrix` field added to `CameraSysState` in its padding),
    our signature and our callee prototypes.
  - **`SampleCameraFogZone`** (`0x002735B0`) in
    `going-decompiled/src/usa/text/16E980.c`, from `src/game/0026EA00.c`
    (`func_002735A0`, v1.01 address, author commit `69034e31`, Flavius), with
    its `CameraEnvironmentRegion` and `FogPresetView` typedefs and its
    `MACRO_ADDR` ".sdata" declaration device. Its upstream comment says the
    arithmetic follows RAC1's `update_camera_environment_from_regions`
    (`src/game/effects.c`, Lombyte/OpenRAC, GPL-3.0); no file of that project
    is carried. ADAPTED: the fog block and region array are reached through
    our `g_blobShadowCount` and `g_collTriBuffer`, with our signature and
    callee spellings.
  - **`func_002AD590`** (`0x002AD590`) in
    `going-decompiled/src/usa/text/1A8180.c`, from `src/game/002AA800.c`
    (`func_002AC728`, v1.01 address, author commit `8f3538bb`, bigbeno37),
    task #2234, from audit NOTE #12089 row 5. ADAPTED: our callee and data
    spellings (the colour palettes read through a struct view of our
    `D_1A9F20`/`D_1A9F38`), and EE-only callee prototypes in upstream's
    argument order except SpawnShockwaveRingMoby, whose f32 is first in
    ours and last upstream (EABI-neutral), with the native arm keeping ours.
  - **`func_002D75D8`** (`0x002D75D8`) in
    `going-decompiled/src/usa/text/1D54C0.cpp`, from `src/game/002D6970-g32.c`
    (`func_002D6970`, v1.01 address, author commit `0eb05a7c`, Flavius), with
    its `MenuRoot`, `PadState`, `GridMenu`, `GridEntry` and `WeaponInfo`
    typedefs. ADAPTED: their menu and pad blocks are views of our
    `g_menuScreenBlock` and `D_138180`; their tables and callee are our
    `g_inventoryOwned` (through the unit's `g_inventoryOwnedAbs` alias),
    `g_itemEquippedSlot`, `g_weaponTable` and `func_002DFFA0`; one `!(...)`
    test goes through a local so the C++ front end keeps the ROM's `xori`.
    The unit's s136os compile is unpinned for it (`S136EXTRA=""`).
  - **`UpdateMobyMotionVelocity`** (`0x002B61C0`) in
    `going-decompiled/src/usa/text/1B4218.cpp`, from `src/game/002B52A0-g8.c`
    (`func_002B52A0`, author commit `e2e1a29f`, Louis-Philippe Le Sieur), with
    its `Self_002B52A0`/`Ctl_002B52A0` layouts. ADAPTED: callee spellings and
    the signature are ours (task #2235).
  - **`StartTertiaryVoice`** (`0x002B7FA8`) in
    `going-decompiled/src/usa/text/1B4218.cpp`, from
    `src/game/002B6120-keep-block-order.c` (`func_002B7088`, author commit
    `8f3538bb`, bigbeno37), whose comment names RAC1's `music_Transition` as
    the shape. ADAPTED: symbol spellings are ours and the sample start is
    narrowed to `s32` at the call (task #2235).
  - **`UpdateMobyAnimLoopSound`** (`0x002A0288`) in
    `going-decompiled/src/usa/text/1A00F0.cpp`, from `src/game/0029DE38.c`
    (`func_0029F450`, v1.01 address, author commit `4a78f7b7`, Flavius),
    task #2251 (audit NOTE #12089 row 9). It closes only with gcse on, so
    text/1A00F0's s136os compile was unpinned with it. ADAPTED: our
    signature, local names, symbol spellings (`g_listenerPosHistory`,
    `StopSoundEmitter`, `PlayMobySound`) and prototypes; the control flow
    and every test are theirs.
  - **`UpdateDialogVoiceManager`** (`0x002B8498`) in
    `going-decompiled/src/usa/text/1B4218.cpp`, from `src/game/002B7578.c`
    (`func_002B7578`, v1.01 address, author commit `8f3538bb`, bigbeno37),
    whose header says it is adapted from RAC1's `src/game/music.c`
    `music_Update` (GPL-3.0), with its `State`/`Channel` layouts. Task #2262;
    its switch table is bound to the ROM's `jtbl_0026CBF0_text` by
    `tools/ee/jtbl_bind.sh`. ADAPTED: our symbol spellings, the unit's
    prototypes with casts at the call (StartDialogVoice through a
    void-returning cast), and func_00132BC0 declared with upstream's
    parameter types.
  - **`func_002E0010`** (`0x002E0010`) in
    `going-decompiled/src/usa/text/1DFF80.cpp`, from
    `src/game/002DF1B8-g8.c` (`func_002DF1F8`, v1.01 address, author commit
    `04cbc7e5`, Flavius). Task #2262; its switch table is bound to the ROM's
    `jtbl_0026CEB0_text` by `tools/ee/jtbl_bind.sh`. ADAPTED: our symbol
    spellings (`sprintf`, `GetLocalizedString`, `D_1AB9B8`, `D_1ABA38`,
    `g_levelSelectEntries` as a `LevelSelectEntry` array).
  - **`UpgradeWeaponToMax`** (`0x002891D8`) in
    `going-decompiled/src/usa/text/188858.c`, from `src/game/002887B0.c`
    (`func_002887B0`, v1.01 address, author commit `768f3964`, Flavius, from
    Louis-Philippe Le Sieur's draft `df59088a`), task #2248 (audit NOTE
    #12089 row 4). Its switch's jump table is bound to the ROM's
    `jtbl_0026C6D0_text` by `tools/ee/jtbl_bind.sh`. ADAPTED: our signature,
    local names and types, and our symbols (`g_weaponTable`,
    `g_itemEquippedSlot`, `g_weaponXp`, `SetWeaponUpgradeSlot`).
  - **`func_002897B8`** (`0x002897B8`) in
    `going-decompiled/src/usa/text/188858.c`, from `src/game/00288D88.c`
    (`func_00288D90`, v1.01 address, author commit `be9f428d`,
    Louis-Philippe Le Sieur; the file is headed "Qwen draft"), task #2248
    (audit NOTE #12089 row 4). Its jump table is bound to
    `jtbl_0026C780_text`. ADAPTED: their subtitle block `D_00254E78` is our
    `g_subtitleState`, with our struct and field names.
  - **`snd_QueueCommandToRing`** (`0x00132DF0`, 989snd's
    snd_SendIOPCommandNoWait) in `going-decompiled/src/usa/cod/0321A0.c`,
    from `src/core/00132DF0-g8.c` (`func_00132DF0`, author commit
    `fddb10bf`, Flavius), task #2294 (audit NOTE #12161). ADAPTED: our
    signature (an `s32` return where theirs is void; the 64-bit last
    parameter kept), our callee and descriptor spellings, and their `sda`
    declarations dropped for the unit's own (`.extern ,16` on the RPC client
    and receive block, RULING #8620).
  - **`OnVblankInterrupt`** (`0x001339B0`) in
    `going-decompiled/src/usa/cod/0321A0.c`, from `src/core/001339B0.c`
    (`func_001339B0`, author commit `962455ea`, Louis-Philippe Le Sieur,
    whose note credits rac1-decomp `src/core/permcb.c` `func_0012F308`),
    task #2294 (audit NOTE #12161). ADAPTED: our names (`g_vblankCount`,
    `g_hblankTicksAccum`), their `NOT_SDA` declarations replaced by
    `.extern ,16` lines and their `MACRO_ADDR` dropped (RULING #8620).
  - **`GuiIconScreenInit2`** (`0x00343028`) in
    `going-decompiled/src/usa/text/235FE8.c`, from `src/game/00341C20-g8.c`
    (`func_00342118`, v1.01 address, author commit `19fe7638`,
    ThorpenCodes), task #2304 (audit NOTE #12173, FACT #12175). It closes
    under the unit's current s136os flags, the `-fno-gcse` pin kept.
    ADAPTED: our callee names and prototypes (element pointers cast to
    `GuiElement *`, tag and atlas pointers to `s32`), our v2.00 data names
    (`D_1ADBE8`, `D_1ADBF8`, `D_1ADF98`, `D_1AE098`, `D_1AE110`, and a plain
    `g_guiInstance` for their `sda` array `D_001A8CFC`), memset called
    through an asm-label alias so cc1 does not treat it as the builtin, and
    our name and signature.
  - **`func_0034BA50`** (`0x0034BA50`) in
    `going-decompiled/src/usa/text/248B50.cpp`, from `src/game/00345358.c`
    (`func_0034AD10`, v1.01 address, author commit `5e993d01`, bigbeno37),
    task #2306 (audit NOTE #12173 / FACT #12175). It closes only with gcse
    on, so text/248B50's s136os compile was unpinned first. ADAPTED: our
    callee and data names and prototypes (the widget arguments cast to
    `GuiWidget *`, `func_002AA3F0` declared with `s32` parameters as in
    188858.c), and their `section(".sdata")` on the GUI instance pointer
    dropped for the unit's `.extern g_guiInstance, 16`; the control flow and
    expressions are theirs.
  - **`func_00350910`** (`0x00350910`) in
    `going-decompiled/src/usa/text/250080.cpp`, from `src/game/0034CEA0.c`
    (`func_0034FBC8`, v1.01 address, author commit `3f207b09`, Lynder063),
    task #2320 (NOTE #12202). ADAPTED: our callee names and prototypes
    (`func_00350868` takes the decode buffer as `u8 *`), their `.sdata`
    decode-buffer word read as our `u8 *D_001B2354`, and an `(s32 *)` cast
    for the C++ front end. The EE forward declaration func_00350840 forwards
    through stays `(void)` on the 2.9 arm only.
  - **`func_003526A8`** (`0x003526A8`) in
    `going-decompiled/src/usa/text/250080.cpp`, from `src/game/003512B8.c`
    (`func_00351960`, v1.01 address, author commit `455cde53`, Flavius), with
    its `Header_351960`/`Packet_351960` typedefs, task #2320 (NOTE #12202).
    ADAPTED: our callee names and prototypes, two of them called through
    void-returning casts as upstream declares them, their `D_001AE860` as our
    `D_1AE818` with a `section(".data")` addressing device, and their `sda`
    arena word as the unit's `g_pFmvArenaBase`.
  - **`func_002E07F8`** (`0x002E07F8`) in
    `going-decompiled/src/usa/text/1DFF80.cpp`, from
    `src/game/002DF1B8-clear.c` (`func_002DF9E0`, v1.01 address, author
    commit `5ae008de`, bigbeno37), task #2305 (re-audit NOTE #12173, FACT
    #12175). Its two `dli` sites are RULING #8549 rows in
    `tools/ee/ps2eeas_dli_sites.txt`. ADAPTED: their `D_001A6480` is our
    `g_gsScreenContext` read through the `GsScreenDims` view, their
    `D_001B22A8` our `g_frameDmaCursor` through the `.sdata` alias
    `g_frameDmaCursorWords` (`MACRO_ADDR` dropped), `int`/`long` spelled
    `s32`/`s64`, and our local names.

- **RC2 decomp contributions — `src/game/002E4528-rc2.c`, `src/game/002B7380-rc2.c` and `src/game/00297BB0-rc2.c` in OpenRAC `rac2-gc-decomp`, commit `96927f87`** (MIT, © 2026 RC2 decomp contributions)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  Three engine C bodies, adopted with permission (held by the operator, RULINGs
  #10405 and #12108) and re-checked byte-exact against our v2.00 ROM on the
  s136os arm. `002E4528-rc2.c` was carried into rac2-gc-decomp by atlasbez-dot
  (commit `ebf90a4c`, from atlasbez-dot/rac2-gc-decomp at `921a4dee`), as was
  `00297BB0-rc2.c` (commit `bc1dd02f`); `002B7380-rc2.c` came in with commit
  `fe9859d7` (Sergio Rodmauser). All three files carry the same MIT notice,
  which is kept in each doc comment and in `THIRD-PARTY.md`. Each doc comment
  carries the `Source:` line, which names every change beyond extern spellings.

  Used so far:
  - **`CastEmitterOcclusionRay`** (`0x002E5340`) in
    `going-decompiled/src/usa/text/1DFF80.cpp`, from `func_002E4528` (v1.01
    address). ADAPTED: our callee and data spellings and prototypes, our
    signature with the `SoundEmitterSlot` fields for its +0x20/+0x18, and a
    `Vec4` probe for their 4-byte `s32 sp[1]` (written 16 bytes by every
    callee). Returning `CollLine`'s result, as upstream does, is load-bearing.
    Task #2239, from audit NOTE #11286 §(b) row 3.
  - **`StepDialogVoiceChannel`** (`0x002B82A0`) in
    `going-decompiled/src/usa/text/1B4218.cpp`, from `func_002B7380` (v1.01
    address). ADAPTED: their statements unchanged, under our `(u8 *ch)`
    signature with `s32 arg0 = (s32)ch;`; `FastDecTimer` for their
    `func_00282978`; our three callback functions for their `s8[]` labels;
    the unit's prototypes; and their `long long` u64/s64 as our 64-bit
    `long`, which the s136 compiler requires. Task #2296, from audit NOTE
    #12163 row 17.
  - **`func_002984E0`** (`0x002984E0`) in
    `going-decompiled/src/usa/text/191238.cpp`, from `src/game/00297BB0-rc2.c`
    (`func_00297BB0`, v1.01 address, author commit `bc1dd02f`,
    atlasbez-dot), task #2299. ADAPTED: `FillMemory32` and `D_1A9718` for
    their `func_00282A60` / `D_001A9720`, `(&g_mapBitmapBuffer)[1]` for their
    `D_001C4FAC`, and the RULING #8620 `section(".data")` device on the
    `g_mapBitmapBuffer` extern (EE arm only). Without that device the -G8 unit
    reads the scalar gp-relative, where the ROM uses lui/lw.
- **OpenRAC — `rac2-gc-decomp`, commit `96927f87`, core bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  Eight core C bodies, adopted with the authors' permission (held by the
  operator, RULING #10405) and re-checked byte-exact against our v2.00 ROM:
  four on the plain sdk29 arm (tasks #2230, #2261 and #2308) and four on the
  per-function SN 2.95.2 arm (RULING #12157, task #2311). Contributors:
  Flavius, bigbeno37, Louis-Philippe Le Sieur, and the OpenRAC commit authored
  as "agent-core-04". The doc comment carries the `Source:` line, which names
  every change beyond extern spellings.

  Used so far:
  - **`func_0011DE08`** (`0x0011DE08`, the file-I/O RPC library init) in
    `going-decompiled/src/usa/cod/015180.c`, from `src/core/0011DDC8.c`
    (`func_0011DE08`, author commit `c8887184`, Flavius). This is rac1-decomp's
    C (`src/core/00119D88.c`), carried into rac2-gc-decomp. ADAPTED: their
    extern prototypes are dropped for this unit's own, which adds a
    `(SifRpcClient *)` cast on the bind client and `(char *)` casts on two
    `u8[]` buffers.
  - **`func_0011C1F8`** (`0x0011C1F8`, the Kprintf/vfprintf formatting core)
    in `going-decompiled/src/usa/cod/015180.c`, from `src/core/0011BF18.c`
    (`func_0011C1F8`, author commit `c8887184`, Flavius), rac1-decomp's C
    carried into rac2-gc-decomp. ADAPTED: our spellings of their asm-label
    aliases, the float conversion spelled as an explicit
    `func_0011C090(func_001234F0(f))`, and `va_list ap` as `char *ap`. Its
    switch's jump table is bound to the ROM's `jtbl_0013A9F0` by
    `tools/ee/jtbl_bind.sh` (task #2261).
  - **`func_0011D9C0`** (`0x0011D9C0`, the file-I/O RPC reply handler) in
    `going-decompiled/src/usa/cod/015180.c`, from `src/core/0011D950.c`
    (`func_0011D9C0` and the `Word`/`FsRcv*` typedefs, author commit
    `c8887184`, Flavius), rac1-decomp's C carried into rac2-gc-decomp.
    ADAPTED: our extern spellings, and the one store into their
    `volatile s32[]` semaphore table spelled through a `(volatile s32 *)`
    cast on this unit's `s32 D_001346A8[32]`. Its switch's jump table is bound
    to the ROM's `jtbl_0013AB20` by `tools/ee/jtbl_bind.sh` (task #2261).
  - **`WaitVblankStartIntc`** (`0x0011B0B0`, polls INTC_STAT for the next
    VBLANK_START) in `going-decompiled/src/usa/cod/015180.c`, from the draft
    `nonmatching/core/func_0011B0B0.c` (`func_0011B0B0`; its only commit
    `df6a5a10`, author "agent-core-04", filed it as a near miss for its two
    barriers). ADAPTED: our name, and each `__sync_synchronize()` (a call on
    this cc1) written as an inline `sync` behind `#ifndef TARGET_NATIVE`.
    Admitted because it is byte-exact on our ROM; its two `sync` are the
    hardware barriers RULING #12168 allows (task #2308).
  - **`IntegerToAscii`** (`0x00118548`, itoa), **`BufferedWrite128`**
    (`0x00118BC0`) and **`BufferedWrite128Secondary`** (`0x00119AC0`, the two
    128-byte TTY staging writers) in `going-decompiled/src/usa/cod/015180.c`,
    from `src/core/00118548.c`, `src/core/00118BC0.c` and
    `src/core/00119AC0.c` (author commit `d4ac4036`, bigbeno37), on the
    sn2952 arm (task #2311). ADAPTED: our symbol names, `int`/`unsigned int`
    written `s32`/`u32`, and the writers' `func_0011B1E8` extern dropped for
    the unit's own prototype.
  - **`VfprintfDispatch`** (`0x00118CC8`, newlib's split-vfprintf engine
    selector) in `going-decompiled/src/usa/cod/015180.c`, from
    `src/core/00118CC8.c` (author commit `cd02daa5`, Louis-Philippe Le
    Sieur), on the sn2952 arm (task #2311). ADAPTED: our symbol names for it
    and its two engines, `int` written `s32`, and the stream's `+0x54` read
    through a `PrintfStream` view. Its switch's jump table is bound to the
    ROM's `jtbl_0013A4B0` by `tools/ee/jtbl_bind.sh`.

- **OpenRAC — `rac2-decomp`, commit `b2a35115`, core body** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-decomp> at commit `b2a35115e0cf580a1fdab8dd1071b157799e590e`

  One core C body, adopted with the authors' permission (held by the
  operator, RULINGs #10405 and #12108) and re-checked byte-exact against our
  v2.00 ROM on the per-function SN 2.95.2 arm (RULING #12157), task #2465,
  from Window X audit NOTE #13099 row A1. Contributor: bigbeno37. The doc
  comment carries the `Source:` line, which names every change beyond extern
  spellings.

  Used so far:
  - **`VfprintfInteger`** (`0x00119BF8`, the integer-only engine of the split
    vfprintf) in `going-decompiled/src/usa/cod/015180.c`, from
    `src/core/00119BF8.c` (`func_00119BF8`, author commit `bf067513`,
    bigbeno37), whose header says it was reconstructed from retail control
    flow and the newlib 1999 implementation in rac1-decomp
    `src/core/00116FA0.c`. ADAPTED: our name, `s32` return; their `PrintFile`
    view merged into the unit's `PrintfStream`; callees respelled by address
    to our `_mbtowc_r`, `BufferedWrite128Secondary`, `func_00115250` (their
    `memchr` label), `strlen` and `memcpy`; `(void *)D_00133E74` for the
    unit's `s32` extern; `const` dropped from two pad-string externs. Its
    switch's jump table is bound to the ROM's `jtbl_0013A7B0` by
    `tools/ee/jtbl_bind.sh`.

- **OpenRAC — `rac2-gc-decomp`, commit `96927f87`, boundary-split bodies** (GPL-3.0, and MIT for one carried file)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  Two bodies at real entries that our split had fused onto a few words of the
  previous function's debris; task #2240 split them off (symbol_addrs pins)
  and adopted these bodies, re-checked byte-exact against our v2.00 ROM (from
  audit NOTE #12089 rows 10 and 11). Each doc comment carries the `Source:`
  line, which names every change.

  Used so far:
  - **`func_002907C0`** (`0x002907C0`) in
    `going-decompiled/src/usa/text/188858.c`, from `src/game/0028FF50.c`
    (`func_0028FF50`, v1.01 address, author commit `d9e7028d`, Flavius), on
    the 2.9 arm, with the authors' permission (RULING #10405). ADAPTED: their
    `Gauge` view of the element is our `HudElement` by raw offset, and the
    signature is ours.
  - **`func_00336B48`** (`0x00336B48`) in
    `going-decompiled/src/usa/text/235FE8.c`, from `src/game/00335D30-rc2.c`
    (`func_00335D30`, v1.01 address), on the s136os arm. That file came into
    OpenRAC in commit `fe9859d7` (Sergio Rodmauser) from an unnamed RC2 fork
    and is MIT-licensed, © 2026 Forja contributors; the notice is kept
    verbatim in the doc comment and in THIRD-PARTY.md. ADAPTED: callee spelled
    and prototyped as our `func_00336678`, so the call passes `(p, flag)` on
    explicitly; name and signature ours.

- **OpenRAC — `rac2-gc-decomp`, commit `96927f87`, text/1CA080 menu-input bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  Five engine C bodies, adopted with the authors' permission (held by the
  operator, RULING #10405) and re-checked byte-exact against our v2.00 ROM,
  four on the s136os arm (task #2236) and one on the 2.9 arm (task #2272).
  Contributors: bigbeno37, Flavius and Louis-Philippe Le Sieur. Each doc
  comment carries the `Source:` line, which names every respelling. The first
  four are ADAPTED in the same way: our symbol names per the ROM's relocations,
  our `PlayGlobalSound` prototype, the menu-screen block read through a view
  of `g_menuScreenBlock`, upstream's `sda` attribute dropped, and its
  `MACRO_ADDR` spelled as the EE-only `.sdata` extern device (RULING #8620) or
  this unit's `.extern` model.

  Used so far:
  - **`UpdateBestiaryMenuInput`** (`0x002CEAB8`) in
    `going-decompiled/src/usa/text/1CA080.cpp`, from `src/game/002CDB30-g8.c`
    (`func_002CDB30`, author commit `14fd82f2`, bigbeno37; its back-button
    block carries over from upstream's `src/pal/game/002D3DE8.c`, `20d9186d`,
    Flavius).
  - **`UpdateSkillPointsMenu`** (`0x002D0B48`), from
    `src/game/002CFBE8-g32.c` (`func_002CFBE8`, author commit `0eb05a7c`,
    Flavius).
  - **`UpdateHelpTopicMenuInput`** (`0x002D2CD0`), from
    `src/game/002D1D20-g32.c` (`func_002D1D20`, author commit `0eb05a7c`,
    Flavius).
  - **`func_002D2FC8`** (`0x002D2FC8`, the options sub-screen input), from
    `src/game/002D2018-g32.c` (`func_002D2018`, author commits `0eb05a7c` and
    `0f3403f8`, Flavius).
  - **`func_002CC7D8`** (`0x002CC7D8`, a five-way menu action dispatcher), on
    the plain 2.9 arm, not s136os, from `src/game/002C7748-g8.c`
    (`func_002CB820`, author commit `4ce2e79b`, Louis-Philippe Le Sieur; two
    externs from `455cde53`, Flavius). ADAPTED: our symbol names per the ROM's
    relocations, this unit's `func_002D67A0` prototype, `sda` dropped. Its
    switch table is bound to the ROM's `jtbl_0026CC80_text` by
    `tools/ee/jtbl_bind.sh` (task #2272).

- **OpenRAC — `rac2-gc-decomp`, commit `96927f87`, text/1D54C0 Window W bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  Six engine C bodies in text/1D54C0, adopted with the authors' permission
  (held by the operator, RULING #10405) and re-checked byte-exact against our
  v2.00 ROM, five on the s136os arm and one on the plain 2.9 arm (task #2295,
  from re-audit NOTE #12163 / FACT #12162). Upstream compiles these files at
  -G0; in our -G8 unit the same addressing comes from aggregate views,
  `ROM_SPLIT` and the unit's `.extern , 16` model. Contributors:
  ThorpenCodes, Flavius and Louis-Philippe Le Sieur. Each doc comment carries
  the `Source:` line, which names every change beyond extern spellings.

  Used so far:
  - **`func_002D7AE0`** (`0x002D7AE0`, galactic-map streaming-slot teardown)
    in `going-decompiled/src/usa/text/1D54C0.cpp`, on the s136os arm, from
    `src/game/002D5E98.c` (`func_002D6E78`, v1.01 address, author commit
    `66978bb7`, ThorpenCodes). ADAPTED: our symbols as label aliases of their
    struct views (`g_fileLoadVoiceState`, `g_mapVertexData`,
    `g_menuScreenBlock`, `StopFileLoad`), their byte `D_001F291B` written as
    the menu view's field at the same address, and `ROM_SPLIT` (RULING #8620)
    on the file-load view.
  - **`func_002D8DD0`** (`0x002D8DD0`, the planet-row rebuild), on the s136os
    arm, from `src/game/002D8120.c` (`func_002D8120`, v1.01 address, author
    commit `8daa0d46`, Flavius). ADAPTED: our symbols (`D_1A7C0C`,
    `D_18D0E8`, and `D_254E48` / `D_0025AD90` as label aliases with their
    types); `MACRO_ADDR` dropped for the unit's `.extern D_1A7C0C, 16`.
  - **`func_002DA358`** (`0x002DA358`, the per-frame level-entry scan), on the
    s136os arm, from `src/game/002D5E98.c` (`func_002D9600`, v1.01 address,
    author commit `e54ed6d5`, Louis-Philippe Le Sieur). Its switch table is
    bound to the ROM's `jtbl_0026CDA0_text` by `tools/ee/jtbl_bind.sh`.
    ADAPTED: `unsigned long long` / `ULL` written `unsigned long` / `UL`
    (64-bit on the EE; SN 1.36 cc1 makes `long long` 128-bit), our
    `D_261900`, and `func_002DFF68` under this unit's prototype.
  - **`func_002DAE70`** (`0x002DAE70`, the streamed full-screen image draw),
    on the plain 2.9 arm, not s136os, from `src/game/002D5E98.c`
    (`func_002DA118`, v1.01 address, author commit `b413de09`, Flavius).
    ADAPTED: `unsigned long long` written `u64`, our `DrawGlyphQuad`
    prototype, `g_gsScreenContext` and `g_mapTextureWidth` (its `+0x28`
    word) as label aliases.
  - **`func_002DCBF0`** (`0x002DCBF0`, a 30-entry selector input) and
    **`func_002DCCC8`** (`0x002DCCC8`, a 12-entry selector input), on the
    s136os arm, from `src/game/002D5E98.c` (`func_002DBEC0` and
    `func_002DBF98`, v1.01 addresses, author commits `7cb06451` and
    `331fbb0a`, ThorpenCodes). ADAPTED: the pad block `D_138180` as an `int`
    label alias, their `D_001F2974` read as `g_menuScreenBlock+0x134`, and
    `func_002DFFA0`.

- **OpenRAC — `rac2-gc-decomp`, commit `96927f87`, libcdvd/libgraph/libmpeg bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  Ten core C bodies, adopted with the authors' permission (held by the
  operator, RULING #10405) and re-checked byte-exact against our v2.00 ROM on
  the plain sdk29 arm (tasks #2231, #2237, #2273 and #2297). Contributors:
  Flavius, bigbeno37, and the OpenRAC commit authored as "agent" with
  Louis-Philippe Le Sieur's move of it to `nonmatching/`. Eight are rac1-decomp's
  C carried into rac2-gc-decomp; `sceCdInit` and `func_00130A90` are OpenRAC's own. Each doc comment carries the `Source:`
  line, which names every change beyond extern spellings.

  Used so far, all in `going-decompiled/src/usa/cod/022FA8.c`:
  - **`func_001246E0`** (`0x001246E0`, sceCdRead's RPC end callback), adapted,
    from `src/core/00124568.c` (`func_001246D0`, author commit `15ac0fe3`,
    Flavius); rac1-decomp `src/core/001208E8.c`.
  - **`sceCdInit`** (`0x00124E08`), adapted, from `src/core/00124C98.c`
    (`func_00124E08`, author commit `6a0dbf0b`, bigbeno37).
  - **`BuildGsDispEnv`** (`0x00125A20`), adapted (`unsigned long` written
    `u64`), from `src/core/00125970.c` (`func_00125A20`, author commit
    `c8887184`, Flavius); rac1-decomp `src/core/00121D18.c`.
  - **`func_0012D4B0`** (`0x0012D4B0`), adapted (`long` written `s64`), from
    `src/core/0012D100.c` (author commit `c8887184`, Flavius); rac1-decomp
    `src/core/00125630.c`.
  - **`func_0012E378`** (`0x0012E378`), adapted, from `src/core/0012E110.c`
    (author commit `de08d596`, Flavius); rac1-decomp `src/core/0012A2F0.c`.
  - **`func_001306D0`** (`0x001306D0`), verbatim, from `src/core/00130288.c`
    (author commit `c8887184`, Flavius); rac1-decomp `src/core/0012AC80.c`.
  - **`func_00130A90`** (`0x00130A90`), adapted, from `src/core/00130890.c`
    (`func_00130A8C`, author commit `d9e7028d`, Flavius).
  - **`func_0012FBF0`** (`0x0012FBF0`, libmpeg's _getpic), adapted (callee
    spellings with casts to our prototypes, our MpegCtx/IpuDecoder fields for
    their raw offsets), from `src/core/0012FB80.c` (author commit `c8887184`,
    Flavius); rac1-decomp `src/core/0012AC80.c`. Its switch's jump table is
    bound to the ROM's `jtbl_0013BDB0` by `tools/ee/jtbl_bind.sh` (task #2273).
  - **`func_0012EB30`** (`0x0012EB30`, libmpeg's sceMpegDemuxPssRing),
    adapted (callee spellings, with casts to our prototypes), from
    `src/core/0012EAA0.c` (`func_0012EB28`, author commit `c8887184`,
    Flavius); rac1-decomp `src/core/0012AC80.c`; their header says it is
    adapted from **Lombyte** (GPL-3.0) `src/sdk/dma/sce_mpeg_demux_pss_ring.c`
    (task #2237).
  - **`BuildGsDrawEnvPacket`** (`0x00125F20`, libgraph's sceGsSetDefDrawEnv),
    adapted (`unsigned long`/`long` written `u64`/`s64`, our struct and callee
    names, the `sync` behind `#ifndef TARGET_NATIVE`), from the draft
    `nonmatching/core/func_00125F20.c` (`func_00125F20`; origin commit
    `72cfdb68`, author "agent", in `src/core/00125E58.c`, moved to
    `nonmatching/` by `d67ee8c0`, Louis-Philippe Le Sieur, for its inline
    `sync`); rac1-decomp `src/core/00122200.c`. Admitted because it is
    byte-exact on our ROM; its one inline `sync` is the hardware barrier
    RULING #12168 allows (task #2297).

- **OpenRAC — `rac2-decomp`, commit `b2a35115`, Window X bodies (text/235FE8 and level program 24)** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-decomp> at commit `b2a35115e0cf580a1fdab8dd1071b157799e590e`
  (the same history as `rac2-gc-decomp` above: `96927f87` is an ancestor)

  C bodies from upstream's `src/us2/` (their USA v2.00 port) and
  `src/levels/` (their level programs), adopted with the authors' permission
  (held by the operator, RULINGs #10405 and #12108) and re-checked byte-exact
  against our v2.00 ROM (the boot ELF, or the level program off the disc) on
  the s136os arm.
  Each doc comment carries the `Source:` line, which names every respelling.

  Used so far:
  - **`func_00346878`** (`0x00346878`, a grid-cursor input handler) in
    `going-decompiled/src/usa/text/235FE8.c`, from
    `src/us2/game/00346878-g8.c` (`func_US2_00346878`, author commit
    `37ee1b19`, Flavius). ADAPTED: our symbol names per the ROM's
    relocations, our signature and `PlayGlobalSound` prototype, their owner
    struct as our `GuiGridCursor` view, their `.sdata` `g_inventoryOwned` and
    their typed grid array as EE-only asm-label aliases, `sda` dropped. Per-
    function `S136EXTRA=-fno-strict-aliasing` override (RULING #12178,
    task #2467).
  - **`func_00346CD8`** (`0x00346CD8`, the map screen's state-1 row input) in
    `going-decompiled/src/usa/text/235FE8.c`, from
    `src/us2/game/00346CD8-pad-g8.c` (`func_US2_00346CD8`, author commit
    `13388073`, Flavius; the file carries no notice of its own). ADAPTED: our
    symbols, `D_1AA8B8` and `g_inventoryOwned` through EE-only asm-label
    aliases (a typed row array, RULING #13168, and the unit's
    `g_inventoryOwnedLa` `la` alias, RULING #8620) in place of their typed
    array and `section(".sdata")`, their `sda` dropped, their struct views
    renamed and their material record read as the unit's
    `GuiWeaponTableEntry`; our signature with a local cast. Per-function
    gcse-ON override (RULING #12178, task #2468, from audit NOTE #13103 /
    FACT #13097).
  - **`func_L24_0034D598`** (level program 24, Ship Shack, at `0x0034D598`:
    builds the level-ordered menu rows and selects the current owner's entry)
    in `going-decompiled/levels/usa/L24/src/func_L24_0034D598.c`, the first
    level function built from C. From
    `src/levels/24_ship_shack/L24_0034C450-init-g8.c` (`func_L24_0034C450`,
    author commit `a96d3126`, Louis-Philippe Le Sieur), read at the head
    `5833be5b`, where the blob is identical to `b2a35115`'s. ADAPTED: our
    name at the v2.00 place, the boot's `g_anAvailableLevelOrder` for
    `D_00139948`, four level symbols renamed to the v2.00 ROM's relocation
    targets, `volatile` and `sda` dropped (both measured inert),
    `include_asm.h` dropped. s136os arm at -G8, no device (task #2495).

- **OpenRAC — `rac2-decomp`, commit `b2a35115`, text/178E88 Window X bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-decomp> at commit `b2a35115e0cf580a1fdab8dd1071b157799e590e`

  Engine C bodies in text/178E88, adopted with the authors' permission
  (held by the operator, RULING #10405/#12108) and re-checked byte-exact
  against our v2.00 ROM on the s136os arm (task #2471, from the Window X
  boot audit NOTE #13122 / FACT #13116). The doc comment carries the
  `Source:` line, which names every change beyond symbol spellings.

  Used so far:
  - **`func_00280EC8`** (`0x00280EC8`, the offscreen render-target bind for
    menu-widget render-to-texture) in `going-decompiled/src/usa/text/178E88.cpp`,
    on the s136os arm with one RULING #8549 Ps2EeAs dli row (0x280F88), from
    `src/game/0027F128.c` (`func_00280510`, author commit `bf067513e`,
    bigbeno37, 2026-11-01). ADAPTED: our callee spellings (their
    func_00285030 / func_0027B2F8 / func_002FC560 = our func_002859E0 /
    func_0027B858 / AppendGsRegPacket), their `D_001A65EE` read as
    `*(s16 *)(g_gsScreenContext + 0x16E)` (same address), and `int`/`float`
    spelled `s32`/`f32`.
  - **`func_00281020`** (`0x00281020`, the scene-actor occlusion visibility
    ratio) in `going-decompiled/src/usa/text/178E88.cpp`, on the s136os arm
    with gcse ON via the per-function `S136EXTRA=` override (RULING #12178),
    from `src/game/0027F128.c` (`func_00280670`, author commit `bf067513e`,
    bigbeno37), with its `CopyRegion` typedef as `CopyRegion_280670`.
    ADAPTED: their `extern CopyRegion D_001B8A10[]` and `D_001B8A10[index]`
    written as the cast view of our `g_sceneActorMobys + 0x44` (0x1B894C +
    0x44 = 0x1B8990, the address the ROM relocates, which their name does not
    spell), `int`/`float`/`short` written `s32`/`f32`/`s16` (task #2472).

- **Forja contributors** (MIT, © 2026 Forja contributors), via OpenRAC
  `rac2-gc-decomp` at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  OpenRAC commit `fe9859d7` (Sergio Rodmauser) carried 129 functions over from
  an RC2 fork it does not name, each `-rc2.c` file under that fork's MIT
  notice. Two are adopted, with the operator's 2026-10-31 "adopt all" (RULING #12108, tasks #2237 and #2272);
  the notice is reproduced in [THIRD-PARTY.md](THIRD-PARTY.md) and in each
  function's doc comment.

  Used so far:
  - **`func_001248B8`** (`0x001248B8`, libcdvd's SIF command 0x80000012
    handler) in `going-decompiled/src/usa/cod/022FA8.c`, adapted (name at
    our split address), from `src/core/001248B0-rc2.c` (`func_001248B0`).
  - **`func_002CC858`** (`0x002CC858`, the seven-way map-cell descriptor
    dispatch) in `going-decompiled/src/usa/text/1CA080.cpp`, EE arm, from
    `src/game/002CB8A0-rc2.c` (`func_002CB8A0`, v1.01 address), adapted:
    upstream's computed `goto` through the resident jump table is written as
    the `switch` it emulates (the verbatim form does not compile in this C++
    unit), whose table `tools/ee/jtbl_bind.sh` binds to the ROM's
    `jtbl_0026CCA0_text`; signature and symbol spellings are ours (task #2272).

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
  - **Six SDK C bodies** (tasks #2158, #2243), with the authors' permission
    (RULING #10405), each byte-exact on our ROM. Every doc comment carries the
    `Source:` line naming its respellings and edits:
    - **`sceSifInitRpc`** (`0x0011CF78`, `going-decompiled/src/usa/cod/015180.c`),
      adapted, from `src/sdk/rpc/sce_sif_init_rpc.c`.
    - **`sceSifRebootIop`** (`0x0011EED8`, `cod/015180.c`), verbatim apart
      from extern spellings, from `src/sdk/rpc/sce_sif_reboot_iop.c`.
    - **`sceDmaPutEnv`** (`0x00127040`, `cod/022FA8.c`, splat `func_00127040`
      until task #2158), verbatim apart from extern spellings, from
      `src/sdk/dma/sce_dma_put_env.c`.
    - **`sceDbcInit`** (`0x001282A8`, `cod/022FA8.c`), adapted, from
      `src/sdk/library/sceDbcInit.c`.
    - **`func_0012FD60`** (`0x0012FD60`, `cod/022FA8.c`), adapted, from
      `src/sdk/library/_decodeOrSkipFrame.c` (`_decodeOrSkipFrame`).
    - **`func_0012FEC0`** (`0x0012FEC0`, `cod/022FA8.c`), adapted, from
      `src/sdk/library/_decodeOrSkipField.c` (`_decodeOrSkipField`).
  - **`func_00129DA8`** (`0x00129DA8`, `cod/022FA8.c`, libmpeg's `_getRef0`),
    VERBATIM (no edits), taken via OpenRAC `rac2-decomp` at
    `b2a35115e0cf580a1fdab8dd1071b157799e590e` (`src/core/00129450.c`, author
    commit `bf067513e`, bigbeno37, 2026-11-01), whose file notice reads
    "func_00129DA8 is Lombyte (MIT) src/assembly/sdk/library/_getRef0.c"; their
    removed draft `nonmatching/core/func_00129DA8.c` names that Lombyte source
    at commit `5ccfa1fb314e9cb43f6ff9a43249523e1c4a5310`. Adopted byte-exact on
    our ROM with the operator's "adopt all" (RULING #12108, task #2475); the
    doc comment carries the notice verbatim and the `Source:` line.
  - **`func_00130428`** (`0x00130428`, libmpeg `mpeg_init_seq`: the
    reference-image resize after a sequence header) in
    `going-decompiled/src/usa/cod/022FA8.c`, from `src/core/00130288.c`
    (`func_00130428`, author commit `00ebb44e`, bigbeno37; from rac1-decomp
    `src/core/0012AC80.c`). Lombyte-origin: its earlier upstream draft says it
    is transcribed from Lombyte (MIT) `src/assembly/sdk/library/sequenceheader.c`
    (`func_0012C4C8`, present at Lombyte `2c4452dd`); the MIT notice is
    reproduced in [THIRD-PARTY.md](THIRD-PARTY.md) and in the doc comment.
    ADAPTED: `s32 *` parameter for our prototype, casts to our callee
    prototypes, a `func_001306D0` prototype ahead of it. Plain sdk29
    `-O2 -G0`, no selector row; upstream's never-taken branch is kept
    verbatim as the RULING #13126 codegen device (task #2476).

- **OpenRAC — `rac2-decomp`, commit `b2a35115`, us2 body** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-decomp> at commit `b2a35115e0cf580a1fdab8dd1071b157799e590e`

  One C body, adopted with the authors' permission (held by the operator,
  RULING #10405) and the operator's 2026-10-31 "adopt all" (RULING #12108),
  byte-exact on our v2.00 ROM on the s136os arm (task #2460). Contributor:
  Flavius (author commit `c4928ea3`). The doc comment carries the `Source:`
  line, which names every respelling. ADAPTED: our symbol names per the ROM's
  relocations, upstream's `sda` attribute spelled as the EE-only `.sdata`
  extern device (RULING #8620), typedef names ours.

  Used so far:
  - **`BuildCheatMenuItemList`** (`0x002CD510`) in
    `going-decompiled/src/usa/text/1CA080.cpp`, from `src/us2/game/002CD510.c`
    (`func_US2_002CD510`, author commit `c4928ea3`, Flavius).

- **OpenRAC — `rac2-decomp`, nonmatching draft, possible prior art** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-decomp> at commit `3e72fa2d`

  Our `FmvStreamFeedLoop` (`0x0034FD90`, `going-decompiled/src/usa/text/24D728.cpp`,
  task #2429) was decompiled from our ROM. OpenRAC had published a nonmatching
  draft of the same function, `nonmatching/game/func_0034F048.c` (author
  "Fromnowhere", 2026-10-30), about 16 hours before our first draft. The two
  share shapes that are largely forced by the assembly; whether the draft was
  consulted is not recorded, so it is credited here as possible prior art
  (validator #2512).

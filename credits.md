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

- **RC2 decomp contributions — `src/game/002E4528-rc2.c`, `src/game/002B7380-rc2.c`, `src/game/00297BB0-rc2.c` and `src/game/003409C8-rc2.c` in OpenRAC `rac2-gc-decomp`, commit `96927f87`** (MIT, © 2026 RC2 decomp contributions)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  Four engine C bodies, adopted with permission (held by the operator, RULINGs
  #10405 and #12108) and re-checked byte-exact against our v2.00 ROM on the
  s136os arm. `002E4528-rc2.c` was carried into rac2-gc-decomp by atlasbez-dot
  (commit `ebf90a4c`, from atlasbez-dot/rac2-gc-decomp at `921a4dee`), as was
  `00297BB0-rc2.c` (commit `bc1dd02f`) and `003409C8-rc2.c` (commit
  `13cb3d0b`); `002B7380-rc2.c` came in with commit
  `fe9859d7` (Sergio Rodmauser). All four files carry the same MIT notice,
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
  - **`func_003418D8`** (`0x003418D8`, the weapon grid's region-0 d-pad
    handler) in `going-decompiled/src/usa/text/235FE8.c`, from
    `src/game/003409C8-rc2.c` (`func_003409C8`, v1.01 address, author commit
    `13cb3d0b`, atlasbez-dot), read at OpenRAC `rac2-decomp` commit
    `b2a35115e0cf580a1fdab8dd1071b157799e590e`, where the file is
    blob-identical. ADAPTED: our callee and data spellings and prototypes,
    our `(void *w, s32 flags)` signature with their `struct Obj` as our
    `GuiWeaponGridCursor` view, and their typed `s16[][5]` table as an EE-only
    asm-label view of `D_00259F38` (RULING #13168). Per-function
    `S136EXTRA=-fno-strict-aliasing` override (RULING #12178), task #2469.
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

- **OpenRAC — `rac2-decomp`, commit `b2a35115`, cod/015180 core bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-decomp> at commit `b2a35115e0cf580a1fdab8dd1071b157799e590e`
  (the four Newlib stdio files are byte-identical at `8b26d8adeb0b5515a962f83230df4bf909fd0cf5`, which task #2497 read)

  Five core C bodies in `going-decompiled/src/usa/cod/015180.c`, adopted with
  the authors' permission (held by the operator, RULINGs #10405 and #12108)
  and re-checked byte-exact against our v2.00 ROM on the per-function SN 2.95.2
  arm (RULING #12157): `VfprintfInteger` (task #2465, from Window X audit NOTE
  #13099 row A1; contributor bigbeno37) and four Newlib stdio functions under
  the operator's Newlib decision (RULING #13179; tasks #2497 and #2518;
  contributor Louis-Philippe Le Sieur). All five reconstruct Newlib C-library
  code, so Newlib is credited below as well. Each doc comment carries the
  `Source:` line, which names every change beyond extern spellings.

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
    `tools/ee/jtbl_bind.sh`. Its `strlen` declaration sits inside
    `#ifndef TARGET_NATIVE` (the native arm uses `<string.h>`'s). Newlib is
    credited for the code it reconstructs: see the Newlib entry below.
  - **`func_00119BC8`** (`0x00119BC8`, Newlib `vfiprintf`), from
    `src/core/00119BC8.c` (`func_00119BC8`, author commit `7b023993`).
    ADAPTED: callee func_00119BF8 spelled VfprintfInteger with the unit's
    prototype, the +0x54 read as the `PrintfStream` view's `data`, `int`
    parameters typed.
  - **`sprintf`** (`0x00115DA8`, Newlib `sprintf`), from `src/core/00115DA8.c`
    (`func_00115DA8`, author commit `ab9b31a8`). ADAPTED: callee
    func_00118CC8 spelled VfprintfDispatch with the unit's prototype, their
    FILE struct named `SprintfFile` with `s32`/`s16`, `_impure_ptr` read
    through the unit's `s32 D_00133E74`, and a `TARGET_NATIVE` branch of ours
    that builds the va_list with `__builtin_va_start` (clang has no
    `__builtin_next_arg`).
  - **`func_00115CF0`** (`0x00115CF0`, Newlib `fiprintf`), from
    `src/core/00115CF0.c` (`func_00115CF0`, author commit `ab9b31a8`).
    ADAPTED: parameters typed `void *stream, const char *fmt`, the callee's
    prototype the unit's, and the same `TARGET_NATIVE` va_list branch.
  - **`snd_PrintError`** (`0x00115D48`, Newlib `printf`), from
    `src/core/00115D48.c` (`func_00115D48`, author commit `ab9b31a8`).
    ADAPTED: our name, callee func_00118CC8 spelled VfprintfDispatch with the
    unit's prototype, `_impure_ptr` read through the unit's `s32 D_00133E74`,
    locals named, and the same `TARGET_NATIVE` va_list branch.

- **Newlib — `libc/stdio`** (BSD, © 1990 The Regents of the University of
  California; `vfprintf.c` derived from software contributed to Berkeley by
  Chris Torek)
  <https://sourceware.org/git/newlib-cygwin.git>, notices quoted at HEAD
  `5aa0a116d1a3e4bf3be150e5eaaa8e2ed07637f5`; the ROM's build is nearest tag
  `newlib-2000-02-17` (commit `061afe83dc04a43321d69b5f04860e5de799665c`)

  The ROM links a Newlib build; the five bodies above reconstruct Newlib's own
  C functions, adopted under RULING #13179. Each origin file's notice is
  reproduced verbatim in [THIRD-PARTY.md](THIRD-PARTY.md), with its history
  since that tag.

  Used so far:
  - `newlib/libc/stdio/vfprintf.c` (built `-DINTEGER_ONLY`): `VfprintfInteger`
    (newlib's `_vfiprintf_r`, with Sony's output layer in place of newlib's
    `__sprint`/uio; task #2500) and `func_00119BC8` (newlib's `vfiprintf`).
  - `newlib/libc/stdio/sprintf.c` (`sprintf`): `sprintf`.
  - `newlib/libc/stdio/fiprintf.c` (`fiprintf`): `func_00115CF0`.
  - `newlib/libc/stdio/printf.c` (`printf`): `snd_PrintError`.

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

- **OpenRAC — `rac2-decomp`, commit `b2a35115`, Window X bodies (text/235FE8, text/1DFF80, text/1D54C0, text/250080, text/188858, text/191238, text/198FA0, text/1B4218, cod/015180 and level program 24)** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-decomp> at commit `b2a35115e0cf580a1fdab8dd1071b157799e590e`
  (the same history as `rac2-gc-decomp` above: `96927f87` is an ancestor)

  C bodies from upstream's `src/us2/` (their USA v2.00 port), `src/game/`
  (their v1.01 sources), `src/levels/` (their level programs) and `src/core/`
  (their boot core), adopted with the
  authors' permission (held by the operator, RULINGs #10405 and #12108) and
  re-checked byte-exact against our v2.00 ROM (the boot ELF, or the level
  program off the disc) on the s136os arm, or for the five cod/015180 malloc
  bodies on the sn2952 arm (RULING #12157). Contributors so far: bigbeno37,
  Flavius, Louis-Philippe Le Sieur, ReaperOfVeriod and ThorpenCodes.
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
  - **`func_00345298`** (`0x00345298`, the weapon-select screen's stat
    lookup), **`func_00346AF8`** (`0x00346AF8`, the map screen's mode-3
    two-entry input), **`func_00338F88`** (`0x00338F88`, the framed caption
    box), **`func_0033D070`** (`0x0033D070`, the progressive-scan option
    screen) and **`func_003380B8`** (`0x003380B8`, the pause-menu item grid)
    in `going-decompiled/src/usa/text/235FE8.c`, from
    `src/us2/game/003448C0.c` (`func_US2_00345298`, author commit
    `cb63d202`, bigbeno37), `src/us2/game/00346AF8-g8.c`
    (`func_US2_00346AF8`, `37ee1b19`, Flavius), `src/us2/game/00338F88-cc.c`
    (`func_US2_00338F88`, `ff791d1e`, bigbeno37),
    `src/us2/game/0033C958-qcopy.c` (`func_US2_0033D070`, `c4928ea3`,
    Flavius; no qcopy in this body) and `src/game/00337210.c`
    (`func_00337210`, `2e9774a0`, Louis-Philippe Le Sieur; the file notes it
    was adapted from the public RAC1 pause-menu grid renderer, GPL-3.0). All
    five blobs are identical at `b2a35115`, `5776dcc3` (where the sda
    re-screen, NOTE #13181, found them) and the head `f3cc4dbf`; none carries
    a notice of its own. ADAPTED: our symbols per the ROM's relocations, our
    signatures with local typed views, upstream's `sda`/`nosda` carried as
    EE-only RULING #8620 `.sdata`/`.data` devices (and RULING #13168 typed
    views where measured), types spelled `s32`/`u64`/etc. func_00345298
    is on the unit's plain cc1 2.9 arm (no selector row); func_00338F88 and
    func_0033D070 on the s136os arm as the unit is; func_00346AF8 and
    func_003380B8 with the per-function `S136EXTRA=-fno-strict-aliasing`
    override (RULING #12178). Task #2501.
  - **`func_00337DC8`** (`0x00337DC8`, the colours of one item's cell in
    the pause-menu item grid) in `going-decompiled/src/usa/text/235FE8.c`,
    from `src/game/00336A78-g16.c` (`func_00336F20`, v1.01 address, author
    commit `77eabfa9`, bigbeno37). The blob is identical at `b2a35115`,
    `5776dcc3` and the head `fe6adf9b`; it carries no notice of its own.
    ADAPTED: our symbols per the ROM's relocations, our prototype, their
    `ItemRow` array as a cast view of our `u8[]` `g_weaponTable`, their
    `sda` dropped (the unit's -G8 and its existing 8-byte `D_1ADAD8`
    declaration keep both tables gp-relative). Per-function
    `S136EXTRA=-fno-strict-aliasing` override (RULING #12178); its three
    switch tables are bound to the ROM's by `tools/ee/jtbl_bind.sh`, once
    that tool counts a function's tables separately (task #2550).
  - **`func_00339740`** (`0x00339740`, the fade and fit-to-width layout of a
    measured text box) in `going-decompiled/src/usa/text/235FE8.c`, from
    `src/us2/game/00339740-g8.c` (`func_US2_00339740`, author commit
    `22fbd608`, bigbeno37; audit NOTE #13103 / FACT #13097). ADAPTED: our
    callee names and prototypes (`GuiElementGetColor`, `GuiElementSetAlpha`,
    `func_0027F900`, `func_003396E8`, with `GuiElement *`, `const char *`
    and `s32` casts) and our data names `D_1ADBAC` / `D_1ADBB0` for their
    `sda` floats; the unit's -G8 makes those gp-relative without the
    attribute. Per-function `S136EXTRA=-fno-strict-aliasing` override
    (RULING #12178, RULING #13082, task #2466).
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
  - **`PatchShrubPacketTex0`** (`0x002E1ED0`, patches the TEX0 TBP fields
    of the visible shrubs' draw packets) in
    `going-decompiled/src/usa/text/1DFF80.cpp`, from `src/game/002E10B8.c`
    (`func_002E10B8`, v1.01 address, author commit `7fccf735`, Flavius; upstream's root commit, which squashed the earlier RAC2 history).
    ADAPTED: our symbol names by address, their {short, short} table entry
    as our `ShrubTexVramPair` view, `int`/`short`/`unsigned char` spelled
    `s32`/`s16`/`u8`, an `(s32 *)` cast on `g_shrubClassTable[*list]` (ours
    is `u8 *[]`), their `nosda` attribute not carried, an unused local
    dropped, local names ours. Per-function `S136EXTRA=` override (gcse ON
    in a `-fno-gcse` unit, RULING #12178, task #2470).
  - **`func_002DC6C8`** (`0x002DC6C8`, the level-select list widget's init
    handler) in `going-decompiled/src/usa/text/1D54C0.cpp`, from
    `src/game/002DB988-init-g8.c` (`func_002DB988`, author commit
    `a96d3126`, Louis-Philippe Le Sieur). ADAPTED: our symbol names per the
    ROM's relocations, our signature and record types, their struct-typed
    map cache as a cast view of our `g_mapVertexData`, `volatile` and `sda`
    on the list pointer dropped. Unit s136os arm (S136EXTRA "", task #2479),
    after splitting the 4-word debris head `func_002DC6B8` off it.
  - **`func_00350F28`** (`0x00350F28`, the vblank field-sync waiter that arms
    the FMV vblank flip handler) in `going-decompiled/src/usa/text/250080.cpp`,
    from `src/game/0034CEA0.c` (`func_003501E0`, author commit `1c58727d`,
    Louis-Philippe Le Sieur), read at `5776dcc3`, where the body is identical
    to `b2a35115`'s. ADAPTED: our spellings by address, their volatile `sda`
    flag arrays as volatile stores to our `s32` flags, `g_bProgressiveScan`
    through an EE-only gp equate (RULING #9073 under #8620), the loop
    written to break on progressive scan so the unit's RULING #8435
    short-loop pad sits before its branch, `int` spelled `s32`. Unit s136os
    arm (S136EXTRA "", task #2542, from audit NOTE #13181 / FACT #13187).
  - **The newlib malloc layer of `going-decompiled/src/usa/cod/015180.c`**
    (task #2498, sn2952 arm, no device; read at upstream's head `8b26d8ad`,
    where every blob is identical to `b2a35115`'s), five bodies:
    - **`func_00115C90`** (`0x00115C90`, newlib `_sbrk_r`), from
      `src/core/00115C90.c` (author commit `399c57d5`, Louis-Philippe Le
      Sieur). ADAPTED: `int` written `s32`, parameters named.
    - **`malloc`** (`0x00115F28`, newlib `malloc` plus SCE's lock pair), from
      `src/core/00115F28.c` (`func_00115F28`, author commit `399c57d5`,
      Louis-Philippe Le Sieur). ADAPTED: our name, newlib's `void *` /
      unsigned types for their `int`s, the unit's `s32` model for
      `D_00133E74`.
    - **`func_00115F78`** (`0x00115F78`, newlib `free` plus SCE's lock pair),
      from `src/core/00115F78.c` (author commit `0f740805`, Louis-Philippe Le
      Sieur). ADAPTED: `_free_r` for their `func_00116E10`, the shared lock
      prototypes for their asm-label ones, the unit's `s32` model.
    - **`func_00116300`** (`0x00116300`, SCE's recursive semaphore lock behind
      newlib's `__malloc_lock` hook), from `src/core/00115200.c` (author
      commit `4541c4ce`, ReaperOfVeriod). ADAPTED: newlib's unused `reent`
      parameter added, `int` written `s32`. RULING #13221.
    - **`func_00116360`** (`0x00116360`, the matching `__malloc_unlock`), from
      `src/core/00116360.c` (author commit `0f740805`, Louis-Philippe Le
      Sieur). ADAPTED: `SignalSema` for their `func_0011AC40`, newlib's
      unused `reent` parameter added, `int` written `s32`. RULING #13221.
  - **`func_002DEEF8`** (`0x002DEEF8`, the per-frame tick of a streamed
    moby-animation job: decompress, install three animation tables, then
    step the moby's idle/dialog sequence) in
    `going-decompiled/src/usa/text/1D54C0.cpp`, from
    `src/game/002DDEC8-animation.c` (`func_002DE0D0`, author commit
    `b073e803`, bigbeno37; the file carries no notice of its own; its
    comment names RAC1's `update_streamed_moby_animation`). ADAPTED: our
    symbol names per the ROM's relocations, their struct views renamed,
    their `sda` byte array as our scalar `g_miscExtras` behind an EE-only
    `.extern g_miscExtras, 16` (RULING #12316), `g_fileLoadState` through
    the unit's `section(".data")` declaration (RULING #8620). Unit s136os
    arm (S136EXTRA "", task #2480), after splitting the 4-word debris head
    `func_002DEEE8` off it.
  - **`func_00339F98`** (`0x00339F98`, a camera-projection override around a
    HUD list update) in `going-decompiled/src/usa/text/235FE8.c`, from
    `src/game/00338F58.c` (`func_00338F58`, v1.01 address, author commit
    `4afceda4`, Louis-Philippe Le Sieur). ADAPTED: our callee spellings and
    declarations, and their v1.01 `D_L00_001BD2C0` array as the EE-only
    assembler equate `g_cameraProjScratch = g_sceneActorMobys + 0x674`
    (RULING #9574). Per-function `S136EXTRA=-fno-strict-aliasing` override
    (RULING #12178), task #2469.
  - **`func_00341A80`** (`0x00341A80`, the weapon grid's region-1 d-pad
    handler) in `going-decompiled/src/usa/text/235FE8.c`, from
    `src/game/0033F030.c` (`func_00340B70`, v1.01 address, body author commit
    `aac730b3`, Flavius). ADAPTED: our callee and data spellings and
    prototypes, our `(void *w, s32 buttons)` signature with their
    `MenuState`/`MenuEntry` as our `GuiWeaponGridCursor`/`GuiWeaponGridEntry`,
    and their typed table as an EE-only asm-label view of `D_259CC0`
    (RULING #13168). Per-function `S136EXTRA=-fno-strict-aliasing` override
    (RULING #12178), task #2469.
  - **`func_00342520`** (`0x00342520`, a multi-column screen's column
    select) in `going-decompiled/src/usa/text/235FE8.c`, from
    `src/us2/game/003424C8-g8.c` (`func_US2_00342520`, author commit
    `5800c586`, Flavius). ADAPTED: our callee and data spellings and
    declarations with `GuiElement *` casts, and their `sda` scalar
    `D_001AE0C8` as our `D_1AE0A0[idx]`, declared `s32[2]` on EE (RULING
    #12316 Q3) and `s32[3]` on native. Per-function
    `S136EXTRA=-fno-strict-aliasing` override (RULING #12178), task #2469.
  - **`SetupMemoryArenaTable`** (`0x002913C0`, fills the 0x9C-byte EE
    memory-region table) in `going-decompiled/src/usa/text/191238.cpp`, from
    `src/game/00290A40.c` (`func_00290B48`, author commit `7dc80be1`,
    bigbeno37; the blob is identical at `5776dcc3` and at the head
    `d36c267a`). ADAPTED: our symbol names per the ROM's relocations, their
    end-of-text and top-of-RAM symbol arithmetic read as the build's
    absolute names `D_356D07` / `D_1FF8000` / `D_1FFC000` (numbers on
    native), their `sda` cursor as an EE-only `.sdata` asm-label view
    (RULINGs #8620/#13168), `s32`/`u32` spellings. Unit s136os arm
    (S136EXTRA "", task #2507).
  - **`SaveLoadStateMachine`** (`0x00299CF0`, the memory-card transaction
    state machine) in `going-decompiled/src/usa/text/198FA0.cpp`, from
    `src/us2/game/00299CF0.c` (`func_US2_00299CF0`, author commit
    `3b7195d7`, Flavius; the blob is identical at `b2a35115`, `5776dcc3`,
    where the sda re-screen NOTE #13181 found it, and the head `71af8fdc`;
    the file carries no notice of its own). ADAPTED: our symbols per the
    ROM's relocations, their state block, disc-TOC record and directory
    buffer as EE-only typed views (RULING #13168), two `section(".data")`
    scalars (RULING #8620), their `D_001A7A00[8]` as an EE-only size-16
    equate view of `D_1A7A28`, their `void`/`int` prototypes for two callees
    kept through EE-only asm-label declarations, `(char *)` casts on the
    unit's `u8[]` string buffers. s136os arm as the unit is (S136EXTRA "");
    its six switch tables are bound to the ROM's by `tools/ee/jtbl_bind.sh`.
    Task #2551.
  - **The display-model loaders and texture-upload queue of
    `going-decompiled/src/usa/text/191238.cpp`** (task #2540, s136os arm at
    the unit's flags; the six upstream blobs are identical at `b2a35115`,
    `5776dcc3` and the head `fe6adf9b`). The ROM's 64-bit `dli` constants
    use RULING #8549 allowlist rows (one per function). Six bodies:
    - **`LoadPlayerDisplayTextures`** (`0x00292928`), from
      `src/us2/game/00292928.c` (`func_US2_00292928`, author commit
      `f0f860b7`, bigbeno37). ADAPTED: our names by address; their
      fixed-address macros defined over EE-only `.sdata` views of our
      symbols (`g_pPlayerModelBuffer` [0]/[1], `g_cheatFlags` +2/+9/+0xA/+0xB,
      the VRAM block words); `ULL` written `UL`; one empty `"+m"`/`"m"` asm
      fence (RULINGs #13208 and #13379).
    - **`LoadHeldItemDisplayModel`** (`0x00292CB8`), from
      `src/game/00292408.c` (`func_00292408`, author commit `e14fe0f3`,
      bigbeno37). ADAPTED: our names by address, `long` written `s64`,
      EE-only views for the buffer pointer (`.data`), the VRAM words and
      the descriptor table.
    - **`LoadShipDisplayModel`** (`0x00292EA0`), from
      `src/game/002925F0-g8.c` (`func_002925F0`, author commit `34e66dc9`,
      Flavius; their header says it was carried from rac3-decomp). ADAPTED:
      our names by address, the unit's callee prototypes with two casts,
      EE-only views of `g_shipModelBufferBase` and `D_1A91E0`.
    - **`LoadShipDisplayTexture`** (`0x00292F68`), from
      `src/game/00291758.c` (`func_002926B8`, author commit `e14fe0f3`,
      bigbeno37). ADAPTED: our names by address, `long` written `s64`,
      EE-only views of the frame-arena pair and the VRAM words.
    - **`func_002954F0`** (`0x002954F0`), from `src/game/00294BC8.c`
      (`func_00294C40`, author commit `ec07f377`, bigbeno37). ADAPTED: our
      names by address, `long` written `s64`, the unit's
      `g_vramAllocCursorAbs` equate, their queue record as our
      `GsTextureUploadSlot`.
    - **`QueueGsTextureUpload`** (`0x00295630`), from `src/game/00294D80.c`
      (`func_00294D80`, author commit `8daa0d46`, Flavius). ADAPTED: our
      names by address, `ULL` written `UL`, the record's fields renamed,
      the count declared `s32`.
    Each function's doc comment carries the full `Source:` line and the
    removal probe for each device. None of the six source files carries a
    notice of its own.

  - **`UpdateSubtitleStateMachine`** (`0x00289BB8`) in
    `going-decompiled/src/usa/text/188858.c`, from `src/game/00289178.c`
    (`func_00289178`, v1.01 address, author commit `6a0dbf0b`, bigbeno37,
    2026-10-28). Task #2505; its switch table is bound to the ROM's
    `jtbl_0026C7A0_text` by `tools/ee/jtbl_bind.sh`. Read at `5776dcc3`; the body
    is identical at `b2a35115` and at head `fe6adf9b`. ADAPTED: our symbols per
    the ROM's relocations, the audit's z2 addressing devices (RULINGs
    #8620/#8036/#12316) minimised by removal probes, the state-8 latch
    increment pinned to `$2` by an EE-only asm register local, and the
    `RECORD_SUBTITLE_TIME` macro's opening brace moved off the `#define`
    line for the ORPHAN_LATENT scanner.
  - **`func_0028E680`** (`0x0028E680`, the Giant Clank health bar's draw
    callback) in `going-decompiled/src/usa/text/188858.c`, from
    `src/game/0028BDB0.c` (`func_0028DDD8`, v1.01 address, author commit
    `6a0dbf0b`, bigbeno37; the body is identical at `96927f87`), on the
    s136os arm, with the authors' permission (RULING #10405). ADAPTED: our
    callee and data spellings by address, a RULING #8620 `section(".data")`
    device on the `D_18B69A` declaration (EE arm only), `char *` written
    `u8 *` with `HudElement *` casts, and this unit's prototypes for their
    extern block.
  - **`MenuScreenDoAction`** (`0x002D67C0`, the front-end/pause screen-action
    dispatcher) in `going-decompiled/src/usa/text/1D54C0.cpp`, from
    `src/game/002D5540.c` (`func_002D5B58`, author commit `08722f0c`,
    bigmak94, 2026-10-21, which replaced the `INCLUDE_ASM` that `7fccf735`
    carried; the body is unchanged after it through `71af8fdc`). ADAPTED:
    our symbol names per the ROM's relocations (incl.
    the block fields upstream reads as `root->f18`/`root->f14`, which the
    ROM's own `.s` names `g_pNextMenuScreen`/`g_pCurrentMenuScreen`); their
    `Func002D5B58Root` as our `MenuScreenActionState`; the third parameter as
    this unit's existing `void *outFlag` prototype; `PlayGlobalSound`'s
    file-scope prototype corrected to `s32` (the ROM-matched definition and
    this unit's other decls already return `s32`; the `void` spelling flips
    op 5's register allocation, 6/183 words); RULING #12316 `.extern ,16`
    sizing for `g_gameStateFlags` (`g_currentLanguage`'s directive
    pre-existing); declaration-only
    asm-label aliases with offset views for the caption table
    (`g_collTriBuffer+0x1020`) and popup word (`g_pTextTableLoadBuf+0x48`);
    their jump-table name bound to `jtbl_0026CD40_text`; the s136os selector
    row. Two of the audit's three devices carried (probe figures at the task
    FACT); the section(".data") device probed inert and dropped.

  - **`SoundOptionsMenuInput`** (`0x002D87C8`, the sound-options screen input
    handler) in `going-decompiled/src/usa/text/1D54C0.cpp`, from
    `src/game/002D5E98.c` (`func_002D7B18`, author commit `5ae008de`,
    bigbeno37, 2026-10-29, which replaced the `INCLUDE_ASM` that `3cd2f593`
    carried; the body is unchanged after it through `71af8fdc`). ADAPTED:
    our symbols per the ROM's relocations (upstream v1.01 names in the doc
    comment); upstream's
    `Settings2D7B18` settings block respelled BY ADDRESS onto the ROM's own
    symbols as u8-cast offset views over the middle word (base
    `g_audioStereoMode`, +4 `g_sfxVolume`, +8 `g_musicVolume`; the PAL-flag
    first word is never referenced here); two `volatile` accesses, which are
    ours and not upstream's, are CODEGEN DEVICES (RULINGs #8404 / #12316
    Q2): they carry the ROM's merge asymmetry (the sfx word re-read after the
    music store; 166 words without it) and the X-toggle shape (a volatile
    read cannot fill the branch-likely delay, so the ROM's plain `bne`+nop
    stays; 169 words without it). The writer census in the doc comment finds
    no concurrent writer of either word. The RULING #8620-class file-scope
    `.extern g_audioStereoMode, 16` (without it this body assembles 154
    words vs 168): it is one line of func_002D8A68's existing three-line
    `.extern ,16` block, which moved above this arm's guard. The other two
    lines, `g_musicVolume, 16` and `g_sfxVolume, 16`, are not load-bearing
    for this body (still 168 without either). They serve only
    func_002D8A68 (217 vs 218 words without either) and are not credited
    here. Also adapted: a declaration-only asm-label array alias for the
    `g_pTextTableLoadBuf+0x1C` offset view (RULING #13168); the s136os selector row; the mix callee
    spelled under its current in-tree name `func_002E5698` (the former alias
    `ComputeAudioChannelMix` no longer resolves — caught by the link at gate
    run 4). Byte-exact 168=168 words, 0 diffs; splice gate-green after one
    real-stream device fix (device ordering — splice ADDRESSING), which no
    solo harness can see. A `.extern g_pLevelOrder, 4` line once carried for a
    LENGTH refusal is not load-bearing at the landed body and was removed
    (task #2586: 168 = 168 without it, unit object cmp-identical).
  - **`CalcMobyTargetThreatDist`** (`0x002B47E0`, the lock-on threat scorer:
    planar distance, a facing-misalignment penalty, and +42 toward the hero
    moby) in `going-decompiled/src/usa/text/1B4218.cpp`, from
    `src/game/002B3868.c` (`func_002B38C8`, v1.01 address, author commit
    `e1ef4e67`, ThorpenCodes; the blob is identical at `b2a35115`, `5776dcc3`
    — where the sda re-screen NOTE #13181 found it — and the head
    `71af8fdc`). ADAPTED: our symbol names per the ROM's relocations, our
    `(Moby *, Moby *)` signature with `&moby->facingTarget` /
    `moby->moveSpeed` reads, `float` as `f32`, their `sda` dropped for the
    file-scope `.extern g_altGravityEnabled, 16` sizing (RULING #12316 Q1)
    and their `.data` `g_pHeroMoby` carried by the unit's existing
    `ABSOLUTE_GLOBAL` declaration (RULING #8620); the unit's local alias
    prototypes `Dist3DVu0`/`WrapAngleAbsDiff` renamed to the canonical
    `Vec3DistVu0`/`AngleAbsDiffPi`. s136os arm as the unit is (task #2506,
    from audit NOTE #13181 row 10 / FACT #13187).
- **Newlib** (<https://sourceware.org/git/newlib-cygwin.git>, tag
  `newlib-2000-02-17`, commit `061afe83dc04a43321d69b5f04860e5de799665c`;
  Red Hat / Cygnus Solutions and contributors)

  The C library the game links. Bodies reconstructed from it are adopted under
  RULING #13179 (operator, 2026-11-06); THIRD-PARTY.md carries each origin
  file's notice, or for a file with none, the `COPYING.NEWLIB` default.

  Used so far:
  - `func_00115C90` (`_sbrk_r`, `newlib/libc/reent/sbrkr.c`), `malloc` and
    `func_00115F78` (`malloc`, `free`, `newlib/libc/stdlib/malloc.c`) in
    `going-decompiled/src/usa/cod/015180.c` (task #2498). `func_00116300` /
    `func_00116360` implement newlib's `__malloc_lock` / `__malloc_unlock`
    hook (`newlib/libc/stdlib/mlock.c`) but carry SCE's body, not newlib's.

- **OpenRAC — `rac2-decomp`, commit `b2a35115`, text/178E88 and text/16E980 Window X bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-decomp> at commit `b2a35115e0cf580a1fdab8dd1071b157799e590e`

  Engine C bodies in text/178E88 and text/16E980, adopted with the authors'
  permission (held by the operator, RULING #10405/#12108) and re-checked
  byte-exact against our v2.00 ROM on the s136os arm (tasks #2471 and #2472,
  from the Window X boot audit NOTE #13122 / FACT #13116; task #2459, from
  NOTE #13117 / FACT #13102; tasks #2503, #2504 and #2541, from the Window X
  re-screen NOTE #13181 / FACT #13187, read at `5776dcc3`, where their files
  are identical to `b2a35115`'s). Each doc comment carries the `Source:` line,
  which names every change beyond symbol spellings.

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
  - **`DrawLensFlare`** (`0x002725F0`) in
    `going-decompiled/src/usa/text/16E980.c`, from `src/game/0026EA00.c`
    (`func_002725E0`, author commit `d4ac4036`, bigbeno37; the body is
    unchanged in that repo since commit `96927f87`). ADAPTED: our data and
    callee spellings (the flare block read through a struct view of our
    `g_pLensFlareSource`, `g_cameraPos` through an unaligned view, and
    upstream's `sda` screen-centre array as one RULING #8620 + #12316
    `.sdata` extern device spanning our four screen scalars, EE arm only,
    with the native arm reading the true `g_nGsPixelOffsetX`/`g_nGsPixelOffsetY`,
    RULING #13197); the 64-bit `long` written `s64` (RULING #13197); struct
    fields renamed (timer->intensity, texture->texId,
    interpolation->offset); the statements and locals are upstream's.
  - **`InitScreenGeometry`** (`0x0027AF50`, 83 words, one-time screen-geometry
    and projection-viewport init) in `going-decompiled/src/usa/text/178E88.cpp`,
    on the s136os arm at the unit's pinned S136EXTRA, from
    `src/game/00274128.c` (`func_0027A9F0`, author commit `7dc80be1`,
    bigbeno37, 2026-10-29, body unchanged to the pin). ADAPTED: their
    `D_001A6480` view as `g_gsScreenContextDims` over `g_gsScreenContext`,
    their `D_001A7340` as the `g_screenExtent` view over `g_screenWidth`
    with sda respelled `section(".sdata")` (the x arm), their `D_001B9040`
    as the cast `(ScreenState *)(g_sceneActorMobys + 0x674)` (same address
    0x1B8FC0), their `func_00283CE0` as `IntToFloat`, and
    `int`/`short`/`float` spelled `s32`/`s16`/`f32`.
  - **`func_0027B858`** (`0x0027B858`, 76 words, configure the viewport /
    camera extents and rebuild the projection; the v2.00 copy of v1.01's
    `func_0027B2F8`) in `going-decompiled/src/usa/text/178E88.cpp`, on the
    s136os arm at the unit's pinned S136EXTRA, from
    `src/us2/game/0027B858.c` (`func_US2_0027B858`, author commit
    `ac7e1a2e`, bigbeno37, 2026-11-06, body unchanged to the pin). ADAPTED:
    their `D_001B9040` as the cast `(ScreenState *)(g_sceneActorMobys +
    0x674)` (same address 0x1B8FC0), their `D_001A7340` as the
    `g_screenExtent` view over `g_screenWidth` with sda respelled
    `section(".sdata")` (the x arm), their `func_00283CE0` as `IntToFloat`,
    their `func_0027AB40` as `BuildCameraProjection`, and `int`/`float`
    spelled `s32`/`f32`.
  - **`RecomputeScreenViewportFromGsContext`** (`0x0027B988`, 70 words,
    re-derive the display geometry / 2D draw viewport from the GS screen
    context pixel dims, then run `func_0027A550`) in
    `going-decompiled/src/usa/text/178E88.cpp`, on the s136os arm at the
    unit's pinned S136EXTRA, from `src/us2/game/0027B858.c`
    (`func_US2_0027B988`, author commit `ac7e1a2e`, bigbeno37, 2026-11-06,
    body unchanged to the pin). ADAPTED: their `D_001A6480` as
    `&g_gsScreenContextDims` over `g_gsScreenContext`, their `D_001A7340`
    as the `g_screenExtent` view over `g_screenWidth` with sda respelled
    `section(".sdata")` (the x arm), their `D_001B9040` as the cast
    `(ScreenState *)(g_sceneActorMobys + 0x674)` (same address 0x1B8FC0),
    their `func_00283CE0` as `IntToFloat`, their `func_00279FF0` as
    `func_0027A550`, and `short`/`int`/`float` spelled `s16`/`s32`/`f32`.
  - **`func_0027DB38`** (`0x0027DB38`, 66 words, draw the pulsing "notice"
    text line — localized string `0x2DB6` at a fixed x, y shifted when the
    suppression flag is set, color-cycled on a cosine pulse driven by the
    frame counter) in `going-decompiled/src/usa/text/178E88.cpp`, on the
    s136os arm at the unit's pinned S136EXTRA, from
    `src/us2/game/0027DB38.c` (`func_US2_0027DB38`, author commit
    `cde7e4dd`, Flavius, 2026-10-21, body unchanged to the pin). ADAPTED:
    their `D_001A7BB1` is stale (the ROM reads `0x1A7BB9`) — respelled as
    the `g_noticeFadeSuppress` view over `D_1A7BB9`, carried with the
    file-scope `.extern D_1A7BB9, 16` sizing device (probe-measured
    load-bearing); their `D_L00_001BCAF4` is stale (the ROM reads
    `0x1B87F4`) — respelled as the `g_noticePulseFrame` `.data` view over
    `g_sceneFrame`; their `func_00283180` as `func_00283B30` (cosine),
    `func_00283D38` as `ColorLerpPacked`, `func_00288FB8` as
    `GetLocalizedString` (`u8 *`), `func_0027F5B8` as `DrawDebugString`
    prototyped to its definition's five-`s32` shape with a `(s32)text`
    cast, their int-valued `long` (64-bit on the EE) written `s32` (RULING
    #13282), and `unsigned char *` written `u8 *`.
  - **`func_00279F08`** (`0x00279F08`, advance the text-run cursor to the
    next run group) in `going-decompiled/src/usa/text/178E88.cpp`, on the
    s136os arm with gcse ON via the per-function `S136EXTRA=` override
    (RULING #12178), from `src/game/00274128.c` (`func_002799A8`, author
    commit `65ef3209`, ThorpenCodes, 2026-10-21; read at `5776dcc3`, the file
    blob identical at `b2a35115`). ADAPTED: their index holder, item table
    and per-level `sda` state word as our `g_textRunCursor`
    (`g_cameraSlotActive + 0xD0`), `g_textRunTable` and a sized equate
    `g_textSequenceMode = g_cameraCallbackCount + 0x80` (EE arm, RULING
    #8620); `func_002798A0` as our `func_00279E00`; struct and local names;
    `int`/`short` spelled `s32`/`s16` (task #2504, from the Window X
    re-screen NOTE #13181 / FACT #13187).
  - **`func_0027DC40`** (`0x0027DC40`, draw this frame's timed subtitle) in
    `going-decompiled/src/usa/text/178E88.cpp`, on the s136os arm with gcse
    ON via the per-function `S136EXTRA=` override, from
    `src/game/0027B948.c` (`func_0027D5C0`, author commit `bf067513`,
    bigbeno37, 2026-11-01; read at `5776dcc3`, where the body is unchanged
    from `b2a35115`; the file credits the layout to OpenRAC `rac1-decomp`
    `src/game/draw.c` `draw_subtitles`, GPL-3.0). ADAPTED: data and callees
    by address to our names; their `sda` / `volatile` externs as EE-only
    RULING #8620 / #12316 / #13168 / #8404 devices; two EE-only asm-label
    prototype views of `func_00280C98` (int parameters) and `func_00280BB8`
    (u64 colour); `long` / `unsigned long` written `s64` / `u64` (RULING
    #13197) (task #2504).
  - **`SetupGsDisplayBuffers`** (`0x0027BAA0`, VRAM layout and GS display
    environment by video mode, optional VRAM clear) in
    `going-decompiled/src/usa/text/178E88.cpp`, on the s136os arm with gcse
    ON via the per-function `S136EXTRA=` override, from
    `src/game/00274128.c` (`func_0027B440`, author commit `00ebb44e`,
    bigbeno37, 2026-11-03; read at `5776dcc3`, the file blob identical at
    `b2a35115`; upstream credits it as adapted from RAC1 `SetPalMode` and
    from Lombyte's `src/video/display/set_pal_mode.c`, both GPL-3.0).
    ADAPTED: their `long long` u64/s64 as our `long`-based u64/s64 (RULING
    #13197); every scalar through an EE-only sized zero-offset alias and
    `.data` on the two u64 ZBUF images (RULING #8620); their `ScreenExtent`
    struct spelled as its eight fields' own symbols; struct and parameter
    names (task #2504).
  - **`DrawRotatedSprite2d`** (`0x0027EB28`, the rotated/mirrored HUD-icon
    quad) in `going-decompiled/src/usa/text/178E88.cpp`, on the s136os arm
    with two RULING #8549 Ps2EeAs dli rows (0x27ED6C, 0x27ED78), from
    `src/game/0027B948.c` (`func_0027E4A8`, author commit `7dc80be1`,
    bigbeno37, 2026-10-29), which upstream says is adapted from rac1-decomp
    `src/game/draw.c`'s `append_rotated_sprite_quad`, itself adapted from
    Lombyte `src/rendering/append_rotated_sprite_quad.c` (GPL-3.0). ADAPTED:
    our callee and data spellings by address (the cursor through a RULING
    #13168 typed view of `g_frameDmaCursor`, the pixel offsets as the unit's
    `g_gsPixelOffsetXAbs`/`YAbs`), `long` written `s64`/`u64` (RULING
    #13197), `int`/`float`/`unsigned char` written `s32`/`f32`/`u8`; the
    parameter order and statements are upstream's (task #2541).

- **OpenRAC — `rac2-decomp`, commit `b2a35115`, cod/022FA8 Window X body** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-decomp> at commit `b2a35115e0cf580a1fdab8dd1071b157799e590e`

  One C body, adopted with the authors' permission (held by the operator,
  RULING #10405) and the operator's 2026-10-31 "adopt all" (RULING #12108),
  byte-exact on our v2.00 ROM on the unit's plain sdk29 `-O2 -G0` arm
  (task #2474). Contributor: Louis-Philippe Le Sieur (author commit
  `a96d3126e`), whose file credits the packet layout to OpenRAC `rac1-decomp`
  `src/core/00122818.c`, revision `bbb526cb`. The doc comment carries the
  `Source:` line, which names every respelling.

  Used so far:
  - **`func_00126470`** (`0x00126470`, libgraph's `sceGsSetDefStoreImage`;
    FACT #5541 names it BuildGsStoreImagePacket) in
    `going-decompiled/src/usa/cod/022FA8.c`, from `src/core/0012646C-store.c`
    (`func_0012646C` — their name at our address; our 0xCD fill FRAG at
    `0x0012646C` owns that name). ADAPTED: name at our address, `int` spelled
    `s32`, `long`/`unsigned long` written `s64`/`u64` (RULING #13197), the
    unused `u_long_sd` typedef dropped, `u128_sd` with the unit's
    TARGET_NATIVE mode(TI) stand-in, their `CpuSync()` as the RULING #12168
    HARDWARE BARRIER inline asm (ROM `0x12659C`) (task #2474).

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

- **OpenRAC — `rac2-decomp` at `5833be5b`, Newlib-identity bodies in cod/015180** (GPL-3.0; the bodies reconstruct Newlib code, whose notice is carried in THIRD-PARTY.md's section "Newlib, origin files with no in-file notice")
  <https://github.com/OpenRAC/rac2-decomp> at commit `5833be5b` (read at
  HEAD; `src/core/00115200.c` and `src/core/00115E68.c` are identical at the
  audit pin `b2a35115e0cf580a1fdab8dd1071b157799e590e`)

  Three Newlib-identity C bodies, adopted per RULING #13179 and re-checked
  byte-exact against our v2.00 ROM on the per-function SN 2.95.2 arm
  (RULING #12157, task #2499). Contributor: Louis-Philippe Le Sieur
  (llesieur99). Newlib's notice for the reconstructed code is carried in
  THIRD-PARTY.md ("Newlib C bodies adopted via OpenRAC rac2-decomp at
  5833be5b — notice-less Newlib origin files"): the origin files carry no
  in-file notice, so COPYING.NEWLIB's default section (1) applies. Each
  doc comment carries the `Source:` line, which names every change.

  Used so far:
  - **`abort`** (`0x00115E28`, Newlib `abort`, SCE-modified: the raise-loop
    replaced by one call to `_exitThunk`) and **`AssertFail`**
    (`0x00115E38`, Newlib `__assert`), both in
    `going-decompiled/src/usa/cod/015180.c`, from `src/core/00115200.c`
    (`func_00115E28` / `func_00115E38`, author commit `2e9774a0`,
    Louis-Philippe Le Sieur). ADAPTED: `func_00131CA8` spelled
    `_exitThunk` and `func_00115E28` spelled `abort` (our symbols), their
    `(char *)` pointer arithmetic kept in this unit's `s32` model, `int`
    written `s32`.
  - **`func_00115E68`** (`0x00115E68`, Newlib `atoi`) in
    `going-decompiled/src/usa/cod/015180.c`, from `src/core/00115E68.c`
    (`func_00115E68`, author commit `7b023993`, Louis-Philippe Le Sieur).
    ADAPTED: callee `func_001175F0` keeps its name (this unit's own
    symbol), `long` written `s64`, `int` written `s32`, `NULL` written 0.

- **OpenRAC — `rac2-decomp`, commit `fe6adf9b`, level programs** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-decomp> at commit `fe6adf9baddcd31323c35adf102d078aa250852d`

  C bodies of the USA v2.00 level programs, adopted per RULINGs #13082 and
  #13291 from the audit of task #2539 (NOTE #13290) and re-checked in a full
  level build against the disc (`tools/levels/level_gate.sh`). Each doc
  comment carries the `Source:` line, which names every change; the licence
  per row is in THIRD-PARTY.md ("OpenRAC rac2-decomp at fe6adf9b, level
  programs").

  Used so far:
  - **`func_L24_0037AE10`** (`0x0037AE10`, level program 24) in
    `going-decompiled/levels/usa/L24/src/func_L24_0037AE10.c`, from
    `src/us2/levels/24_ship_shack/L24_0037AE10-g8.c`
    (`func_US2_L24_0037AE10`, author commit `c4928ea3`, Flavius), task
    #2552. ADAPTED: our name, `sda`/`nosda` as section attributes
    (RULING #8620), the boot's `g_soundBankHandlesBlk`, two level data names
    and eight callees renamed to the v2.00 disc's targets; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L11_003417D0`** (`0x003417D0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003417D0.c`, from
    `src/levels/11_joba/L11_002DE9E0.c` (`func_L11_00340500`, author commit `c7d9e30a`, bigbeno37),
    task #2553. VERBATIM but for names: our name at the v2.00 place; callees func_00282CF0 and func_L02_00333480 as L11's copies func_L11_00301210 and func_L11_003415E8.
  - **`func_L11_0036A7F8`** (`0x0036A7F8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0036A7F8.c`, from
    `src/levels/11_joba/L11_00369700-init-g8.c` (`func_L11_00369700`, author commit `a96d3126`, Louis-Philippe Le Sieur),
    task #2553. VERBATIM but for names: our name at the v2.00 place; the boot's `g_anAvailableLevelOrder`; five level data names renamed to the v2.00 ROM's relocation targets, three of them address-named inside larger labels (RULING #13291 §2); `include_asm.h` dropped.
  - **`func_L11_003F4310`** (`0x003F4310`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F4310.c`, from
    `src/levels/11_joba/L11_003D6270.c` (`func_L11_003F2E00`, author commit `ee3fee45`, llesieur99),
    task #2553. ADAPTED: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); `common.h` dropped, its typedefs written out.
  - **`func_L11_00414290`** (`0x00414290`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00414290.c`, from
    `src/levels/11_joba/L11_00412B20-g8.c` (`func_L11_00412B20`, author commit `c7d9e30a`, bigbeno37),
    task #2553. ADAPTED: our name at the v2.00 place; `sda`/`NOT_SDA` as `section(".sdata")`/`section(".data")` (RULING #8620); the boot's `g_heroPos`; four callees as L11's copies; two data names renamed to the v2.00 ROM's relocation targets, address-named (RULING #13291 §2); `common.h` dropped.
  - **`func_L11_00427310`** (`0x00427310`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00427310.c`, from
    `src/levels/11_joba/L11_00420C88.c` (`func_L11_00425AF8`, author commit `ba383701`, bigbeno37),
    task #2553. ADAPTED: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); two data names renamed to the v2.00 ROM's relocation targets, address-named (RULING #13291 §2); `include_asm.h` dropped.
  - **`func_L11_0042BCF8`** (`0x0042BCF8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0042BCF8.c`, from
    `src/levels/11_joba/L11_00425F10.c` (`func_L11_0042A4C8`, author commit `ba383701`, bigbeno37),
    task #2553. ADAPTED: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); two data names renamed to the v2.00 ROM's relocation targets, address-named (RULING #13291 §2); `common.h` dropped.
  - **`func_L11_00439600`** (`0x00439600`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00439600.c`, from
    `src/levels/11_joba/L11_00432628.c` (`func_L11_00437DC0`, author commit `fa8d6672`, llesieur99),
    task #2553. ADAPTED: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); the record base renamed to the v2.00 target, address-named (RULING #13291 §2); one callee as L11's copy; `common.h` dropped.
  - **`func_L11_004396A8`** (`0x004396A8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004396A8.c`, from
    `src/levels/11_joba/L11_00432628.c` (`func_L11_00437E68`, author commit `6b943916`, Louis-Philippe Le Sieur),
    task #2553. ADAPTED: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); the record base renamed to the v2.00 target, address-named (RULING #13291 §2); one callee as L11's copy; `common.h` dropped.
  - **`func_L11_0043B430`** (`0x0043B430`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0043B430.c`, from
    `src/levels/11_joba/L11_00432628.c` (`func_L11_00439BF0`, author commit `431ac503`, llesieur99),
    task #2553. ADAPTED: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); the boot's `g_listenerPosHistory`; the record base renamed to the v2.00 target, address-named (RULING #13291 §2); three callees as L11's copies; `common.h` dropped.
  - **`func_L11_00442A18`** (`0x00442A18`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00442A18.c`, from
    `src/levels/11_joba/L11_0043E1A0.c` (`func_L11_004411C8`, author commit `f6353318`, DrLegitamate),
    task #2553. ADAPTED: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); their asm-label view `D_L11_001B359C_base` written as the one direct declaration `D_L11_001B361C` (no asm label); two data names renamed to the v2.00 targets, address-named (RULING #13291 §2); nine handlers renamed to their v2.00 places; `common.h` dropped.
  - **`func_L11_00443038`** (`0x00443038`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00443038.c`, from
    `src/levels/11_joba/L11_0043E1A0.c` (`func_L11_004417E8`, author commit `264cb2be`, llesieur99),
    task #2553. ADAPTED: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); the table base renamed to the v2.00 target, address-named (RULING #13291 §2); `common.h` dropped.
  - **`func_L11_00443E18`** (`0x00443E18`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00443E18.c`, from
    `src/levels/11_joba/L11_004425C8.c` (`func_L11_004425C8`, author commit `3cd8af2d`, bigbeno37),
    task #2553. ADAPTED: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620) on two externs; the boot's `g_heroPos`; two data names renamed to the v2.00 targets, address-named (RULING #13291 §2); ten callees as L11's own functions or copies.
  - **`func_L11_00456538`** (`0x00456538`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00456538.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00454AE0`, author commit `91e04732`, llesieur99),
    task #2553. ADAPTED: our name at the v2.00 place; two callees as L11's copies; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L11_004565B8`** (`0x004565B8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004565B8.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00454B60`, author commit `5964c620`, Flavius),
    task #2553. ADAPTED: our name at the v2.00 place; three callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L11_004566D0`** (`0x004566D0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004566D0.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00454C78`, author commit `daf0e4a1`, Flavius),
    task #2553. ADAPTED: our name at the v2.00 place; six callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L11_00456830`** (`0x00456830`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00456830.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00454DD8`, author commit `daf0e4a1`, Flavius),
    task #2553. ADAPTED: our name at the v2.00 place; the boot's `g_listenerPosHistory`; seven callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L11_00457A68`** (`0x00457A68`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00457A68.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00456010`, author commit `91e04732`, llesieur99),
    task #2553. ADAPTED: our name at the v2.00 place; one callee as L11's copy; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L11_00457DD0`** (`0x00457DD0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00457DD0.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00456378`, author commit `5964c620`, Flavius),
    task #2553. ADAPTED: our name at the v2.00 place; two callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L11_00457F98`** (`0x00457F98`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00457F98.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00456540`, author commit `431ac503`, llesieur99),
    task #2553. ADAPTED: our name at the v2.00 place; four callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L11_00458678`** (`0x00458678`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00458678.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00456C20`, author commit `3cd8af2d`, bigbeno37),
    task #2553. ADAPTED: our name at the v2.00 place; twelve callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L11_0045A840`** (`0x0045A840`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0045A840.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00458D88`, author commit `7fccf735`, Flavius; upstream's root commit, which squashed the earlier RAC2 history),
    task #2553. ADAPTED: our name at the v2.00 place; one callee as L11's copy; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L11_0045AA10`** (`0x0045AA10`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0045AA10.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00458F58`, author commit `91e04732`, llesieur99),
    task #2553. ADAPTED: our name at the v2.00 place; one callee as L11's copy; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L11_00461AD8`** (`0x00461AD8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00461AD8.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00460038`, author commit `7fccf735`, Flavius; upstream's root commit, which squashed the earlier RAC2 history),
    task #2553. ADAPTED: our name at the v2.00 place; one callee as L11's copy; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped.
  - **`func_L02_003DCEB8`** (`0x003DCEB8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003DCEB8.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003DB950`, author commit `773eafc2`, llesieur99),
    task #2559. VERBATIM but for names: our name at the v2.00 place; no other symbol.
  - **`func_L02_003E8B18`** (`0x003E8B18`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003E8B18.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003E75A8`, author commit `b7374531`, bigmak94),
    task #2559. VERBATIM but for names: our name at the v2.00 place; four callees as L02's copies; the track table D_L02_002A53B0 as the v2.00 label D_L02_002A5470; `u16` written out.
  - **`func_L02_003EB9F8`** (`0x003EB9F8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003EB9F8.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003EA488`, author commit `ee3fee45`, llesieur99),
    task #2559. VERBATIM but for names: our name at the v2.00 place; callee func_31E6A0 as L02's copy func_L02_0031F998.
  - **`func_L02_003F01C8`** (`0x003F01C8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003F01C8.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003EEC58`, author commit `eef580e5`, Flavius),
    task #2559. VERBATIM but for names: our name at the v2.00 place; the boot's `g_heroPos`; one callee as L02's copy; `D_L02_001FF1C0` kept, address-named (RULING #13291 §2).
  - **`func_L02_003F0528`** (`0x003F0528`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003F0528.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003EEFB8`, author commit `246b78db`, bigmak94),
    task #2559. VERBATIM but for names: our name at the v2.00 place; the boot's `g_heroPos`; two callees at their v2.00 places; `D_L02_001FF1C0` kept, address-named (RULING #13291 §2).
  - **`func_L02_003F2AE8`** (`0x003F2AE8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003F2AE8.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003F1578`, author commit `adcbd51e`, llesieur99),
    task #2559. VERBATIM but for names: our name at the v2.00 place; two callees as L02's copies.
  - **`func_L02_003F2C60`** (`0x003F2C60`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003F2C60.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003F16F0`, author commit `e80a585a`, Flavius),
    task #2559. VERBATIM but for names: our name at the v2.00 place; two callees at their v2.00 places.
  - **`func_L02_003F2D20`** (`0x003F2D20`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003F2D20.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003F17B0`, author commit `e80a585a`, Flavius),
    task #2559. VERBATIM but for names: our name at the v2.00 place; three callees as L02's copies; `u8`/`u16` written out.
  - **`func_L02_003F2E58`** (`0x003F2E58`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003F2E58.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003F18E8`, author commit `ee3fee45`, llesieur99),
    task #2559. VERBATIM but for names: our name at the v2.00 place; nine callees at their v2.00 places.
  - **`func_L02_003F30E0`** (`0x003F30E0`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003F30E0.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003F1B70`, author commit `773eafc2`, llesieur99),
    task #2559. VERBATIM but for names: our name at the v2.00 place; one callee as L02's copy.
  - **`func_L02_003FC4C8`** (`0x003FC4C8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003FC4C8.c`, from
    `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FAF28`, author commit `ee3fee45`, llesieur99),
    task #2559. VERBATIM but for names: our name at the v2.00 place; three callees as L02's copies; the callback D_L02_004004D8 as its v2.00 glabel func_L02_00401C20 (upstream's `char []` type kept).
  - **`func_L02_003FCC60`** (`0x003FCC60`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003FCC60.c`, from
    `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FB6C0`, author commit `455cde53`, Flavius),
    task #2559. VERBATIM but for names: our name at the v2.00 place; two callees at their v2.00 places.
  - **`func_L02_003FCFF8`** (`0x003FCFF8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003FCFF8.c`, from
    `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FBA58`, author commit `adcbd51e`, llesieur99),
    task #2559. VERBATIM but for names: our name at the v2.00 place; two callees at their v2.00 places.
  - **`func_L02_003FD5C0`** (`0x003FD5C0`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003FD5C0.c`, from
    `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FC020`, author commit `773eafc2`, llesieur99),
    task #2559. VERBATIM but for names: our name at the v2.00 place; one callee at its v2.00 place; `D_L02_001BA3C0` kept, address-named (RULING #13291 §2).
  - **`func_L02_003FD658`** (`0x003FD658`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003FD658.c`, from
    `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FC0B8`, author commit `adcbd51e`, llesieur99),
    task #2559. VERBATIM but for names: our name at the v2.00 place; no other symbol.
  - **`func_L02_003FD670`** (`0x003FD670`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003FD670.c`, from
    `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FC0D0`, author commit `455cde53`, Flavius),
    task #2559. VERBATIM but for names: our name at the v2.00 place; no other symbol.
  - **`func_L02_00401D98`** (`0x00401D98`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_00401D98.c`, from
    `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_00400650`, author commit `ee3fee45`, llesieur99),
    task #2559. VERBATIM but for names: our name at the v2.00 place; the boot's `g_fileLoadVoiceState`.
  - **`func_L02_00402410`** (`0x00402410`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_00402410.c`, from
    `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_00400CC8`, author commit `597e1ea6`, ReaperOfVeriod),
    task #2559. VERBATIM but for names: our name at the v2.00 place; `D_0017A80` as the v2.00 relocation target `D_L02_001CAA80`, address-named (RULING #13291 §2).
  - **`func_L11_0043A758`** (level program 11, Joba, at `0x0043A758`) in
    `going-decompiled/levels/usa/L11/src/func_L11_0043A758.c`, from `src/levels/11_joba/L11_00432628.c`
    (`func_L11_00438F18`, v1.01 address, author commit `b4b0fe9b`, bigbeno37). VERBATIM but for names: our name, `D_L11_001FF580` as the address-named `D_L11_001FF600`, two boot-copy callees by their L11 places, `common.h` dropped, its u8/u16/s16 typedefs written in the TU. Task #2555.
  - **`func_L11_0043B5A0`** (level program 11, Joba, at `0x0043B5A0`) in
    `going-decompiled/levels/usa/L11/src/func_L11_0043B5A0.c`, from `src/levels/11_joba/L11_00432628.c`
    (`func_L11_00439D60`, v1.01 address, author commit `fa8d6672`, llesieur99). VERBATIM but for names: our name, `common.h` dropped. Task #2555.
  - **`func_L11_0043BE60`** (level program 11, Joba, at `0x0043BE60`) in
    `going-decompiled/levels/usa/L11/src/func_L11_0043BE60.c`, from `src/levels/11_joba/L11_00432628.c`
    (`func_L11_0043A620`, v1.01 address, author commit `fa8d6672`, llesieur99). VERBATIM but for names: our name, `func_32E480` as `func_L11_0032F698`, `common.h` dropped. Task #2555.
  - **`func_L11_0043C040`** (level program 11, Joba, at `0x0043C040`) in
    `going-decompiled/levels/usa/L11/src/func_L11_0043C040.c`, from `src/levels/11_joba/L11_00432628.c`
    (`func_L11_0043A800`, v1.01 address, author commit `fa8d6672`, llesieur99). VERBATIM but for names: our name, `common.h` dropped. Task #2555.
  - **`func_L11_0043C368`** (level program 11, Joba, at `0x0043C368`) in
    `going-decompiled/levels/usa/L11/src/func_L11_0043C368.c`, from `src/levels/11_joba/L11_00432628.c`
    (`func_L11_0043AB28`, v1.01 address, author commit `5acf9869`, Flavius). VERBATIM but for names: our name, ten boot-copy callees by their L11 places, `common.h` dropped. Task #2555.
  - **`func_L11_004419B0`** (level program 11, Joba, at `0x004419B0`) in
    `going-decompiled/levels/usa/L11/src/func_L11_004419B0.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_00440160`, v1.01 address, author commit `264cb2be`, llesieur99). VERBATIM but for names: our name, `D_L11_004465F8` as our label `func_L11_00448020` (type kept), three boot-copy callees by their L11 places, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_004420B8`** (level program 11, Joba, at `0x004420B8`) in
    `going-decompiled/levels/usa/L11/src/func_L11_004420B8.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_00440868`, v1.01 address, author commit `96f56db3`, bigbeno37). VERBATIM but for names: our name, `D_L11_001FC420`/`D_L11_001B359C` as the address-named `D_L11_001FC4A0`/`D_L11_001B361C` (upstream's own `section(".sdata")` kept), two level callees by their v2.00 places, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_004424D0`** (level program 11, Joba, at `0x004424D0`) in
    `going-decompiled/levels/usa/L11/src/func_L11_004424D0.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_00440C80`, v1.01 address, author commit `431ac503`, llesieur99). VERBATIM but for names: our name, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_004424E0`** (level program 11, Joba, at `0x004424E0`) in
    `going-decompiled/levels/usa/L11/src/func_L11_004424E0.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_00440C90`, v1.01 address, author commit `c7d9e30a`, bigbeno37). VERBATIM but for names: our name, their three `func_L02_…` callees as L11's own copies at the v2.00 places, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_004428E0`** (level program 11, Joba, at `0x004428E0`) in
    `going-decompiled/levels/usa/L11/src/func_L11_004428E0.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_00441090`, v1.01 address, author commit `ee3fee45`, llesieur99). VERBATIM but for names: our name, two callees by their L11 places, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_00442FF8`** (level program 11, Joba, at `0x00442FF8`) in
    `going-decompiled/levels/usa/L11/src/func_L11_00442FF8.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_004417A8`, v1.01 address, author commit `264cb2be`, llesieur99). VERBATIM but for names: our name, `D_L11_001BA780` as the address-named `D_L11_001BA800`, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_00443058`** (level program 11, Joba, at `0x00443058`) in
    `going-decompiled/levels/usa/L11/src/func_L11_00443058.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_00441808`, v1.01 address, author commit `7fccf735`, Flavius; upstream's root commit, which squashed the earlier RAC2 history). VERBATIM but for names: our name, `D_L11_001FF580` as the address-named `D_L11_001FF600`, `func_002A8AF0` as `func_L11_0032F328`, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_004430A8`** (level program 11, Joba, at `0x004430A8`) in
    `going-decompiled/levels/usa/L11/src/func_L11_004430A8.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_00441858`, v1.01 address, author commit `431ac503`, llesieur99). VERBATIM but for names: our name, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_004430B8`** (level program 11, Joba, at `0x004430B8`) in
    `going-decompiled/levels/usa/L11/src/func_L11_004430B8.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_00441868`, v1.01 address, author commit `7fccf735`, Flavius; upstream's root commit, which squashed the earlier RAC2 history). VERBATIM but for names: our name, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_004431C8`** (level program 11, Joba, at `0x004431C8`) in
    `going-decompiled/levels/usa/L11/src/func_L11_004431C8.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_00441978`, v1.01 address, author commit `7fccf735`, Flavius; upstream's root commit, which squashed the earlier RAC2 history). VERBATIM but for names: our name, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_00447750`** (level program 11, Joba, at `0x00447750`) in
    `going-decompiled/levels/usa/L11/src/func_L11_00447750.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_00445D28`, v1.01 address, author commit `4541c4ce`, ReaperOfVeriod). VERBATIM but for names: our name, `D_001A63A8` as the boot's `g_fileLoadVoiceState`, `include_asm.h`/`common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_00447BE8`** (level program 11, Joba, at `0x00447BE8`) in
    `going-decompiled/levels/usa/L11/src/func_L11_00447BE8.c`, from `src/levels/11_joba/L11_0043E1A0.c`
    (`func_L11_004461C0`, v1.01 address, author commit `6b943916`, Louis-Philippe Le Sieur). VERBATIM but for names: our name, `func_L00_003206B8` as L11's own copy `func_L11_00333F00`, `include_asm.h`/`common.h` dropped, typedef written in the TU. Task #2555.
  - **`func_L11_00441A88`** (level program 11, Joba, at `0x00441A88`) in
    `going-decompiled/levels/usa/L11/src/func_L11_00441A88.c`, from `src/levels/11_joba/L11_00440238-g8.c`
    (`func_L11_00440238`, v1.01 address, author commit `68795b5b`, Gianni). VERBATIM but for names: our name, `D_L11_001B359C`/`D_L11_001FC420` as the address-named `D_L11_001B361C`/`D_L11_001FC4A0`, `D_00189E20` as the boot's `g_soundBankHandlesBlk` (upstream's `sda`/`nosda` text kept, ignored by cc1), `include_asm.h` dropped. Task #2555.
  - **`func_L11_00441FE0`** (level program 11, Joba, at `0x00441FE0`) in
    `going-decompiled/levels/usa/L11/src/func_L11_00441FE0.c`, from `src/levels/11_joba/L11_00440238-g8.c`
    (`func_L11_00440790`, v1.01 address, author commit `68795b5b`, Gianni). VERBATIM but for names: our name, the two address-named bank externs as above, two level callees by their v2.00 places (one is our C `func_L11_004424E0`), `include_asm.h` dropped. Task #2555.
  - **`func_L11_00447A48`** (level program 11, Joba, at `0x00447A48`) in
    `going-decompiled/levels/usa/L11/src/func_L11_00447A48.c`, from `src/levels/11_joba/L11_00446020-g8.c`
    (`func_L11_00446020`, v1.01 address, author commit `3cd8af2d`, bigbeno37). VERBATIM but for names: our name, `D_00189E20` as `g_soundBankHandlesBlk`, `D_L11_001B1870` as our label `D_L11_001B18C8`, two callees by their v2.00 places (one is our C `func_L11_00447750`), `common.h` dropped. Task #2555.
  - **`func_L11_0044A750`** (level program 11, Joba, at `0x0044A750`) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044A750.c`, from `src/levels/11_joba/L11_004465F8-g8.c`
    (`func_L11_00448D28`, v1.01 address, author commit `352d5dad`, Louis-Philippe Le Sieur). VERBATIM but for names: our name, the two address-named bank externs `D_L11_001FC4A0`/`D_L11_001B361C`, `common.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_0044A270`** (level program 11, Joba, at `0x0044A270`) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044A270.c`, from `src/levels/11_joba/L11_004465F8.c`
    (`func_L11_00448848`, v1.01 address, author commit `fa8d6672`, llesieur99). ADAPTED: our name, four boot-copy callees by their L11 places, the `(long)` pointer cast written `(s64)` with `typedef long s64` (RULING #13197; the same type), `common.h`/`include_asm.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L11_0044A330`** (level program 11, Joba, at `0x0044A330`) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044A330.c`, from `src/levels/11_joba/L11_004465F8.c`
    (`func_L11_00448908`, v1.01 address, author commit `fa8d6672`, llesieur99). VERBATIM but for names: our name, `func_32C9E0`/`func_32C700` as `func_L11_0032DBF8`/`func_L11_0032D918`, `common.h`/`include_asm.h` dropped, typedefs written in the TU. Task #2555.
  - **`func_L14_0032A188`** (`0x0032A188`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_0032A188.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_00328E90`, author commit `db93b56d`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; three callees as L14's copies.
  - **`func_L14_00392440`** (`0x00392440`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_00392440.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_00390FE8`, author commit `adcbd51e`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; one callee at its v2.00 place; `D_L14_00237B00` as the v2.00 relocation target `D_L14_00237A80`, address-named (RULING #13291 §2).
  - **`func_L14_00392FB0`** (`0x00392FB0`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_00392FB0.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_00391B58`, author commit `6b943916`, Louis-Philippe Le Sieur),
    task #2563. VERBATIM but for names: our name at the v2.00 place; two callees as L14's copies.
  - **`func_L14_003CAA18`** (`0x003CAA18`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003CAA18.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_003C9548`, author commit `db93b56d`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; no other symbol.
  - **`func_L14_003DF838`** (`0x003DF838`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003DF838.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_003DE360`, author commit `913bd1bc`, Gianni),
    task #2563. VERBATIM but for names: our name at the v2.00 place; six callees as L14's copies.
  - **`func_L14_003DF960`** (`0x003DF960`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003DF960.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_003DE488`, author commit `db93b56d`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; one callee as L14's copy.
  - **`func_L14_003DFDC0`** (`0x003DFDC0`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003DFDC0.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_003DE8E8`, author commit `db93b56d`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; no other symbol.
  - **`func_L14_003DFDD0`** (`0x003DFDD0`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003DFDD0.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_003DE8F8`, author commit `db93b56d`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; no other symbol.
  - **`func_L14_003DFE68`** (`0x003DFE68`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003DFE68.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_003DE990`, author commit `db93b56d`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; one callee as L14's copy.
  - **`func_L14_003E0618`** (`0x003E0618`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003E0618.c`, from
    `src/levels/14_aranos_prison/L14_00304190-g8.c` (`func_L14_003DF100`, author commit `ee3fee45`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; three callees as L14's copies / v2.00 places.
  - **`func_L14_003E0B68`** (`0x003E0B68`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003E0B68.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_003DF650`, author commit `db93b56d`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; two callees at their v2.00 places.
  - **`func_L14_003E0BB8`** (`0x003E0BB8`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003E0BB8.c`, from
    `src/levels/14_aranos_prison/L14_003DF6A0-g8.c` (`func_L14_003DF6A0`, author commit `bf60a47f`, Louis-Philippe Le Sieur),
    task #2581. VERBATIM but for names: our name at the v2.00 place; five callees as L14's copies; the boot's `g_listenerPosHistory`; `D_L14_001B268C` as the v2.00 relocation target `D_L14_001B260C`, address-named (RULING #13291 §2); `sda` kept as written (ignored by this cc1); the file's include not carried; asm_unit route, s136 -G8.
  - **`func_L14_003E0D10`** (`0x003E0D10`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003E0D10.c`, from
    `src/levels/14_aranos_prison/L14_00304190-g8.c` (`func_L14_003DF7F8`, author commit `ee3fee45`, llesieur99),
    task #2581. VERBATIM but for names: our name at the v2.00 place; fourteen callees as L14's copies / v2.00 places; three `.sdata` floats as our labels `D_L14_001B0338`/`033C`/`0340`; `sda` kept as written; a repeated declaration carried once; the file's include, other functions and their externs not carried; asm_unit route, s136 -G8.
  - **`func_L14_003E3BA0`** (`0x003E3BA0`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003E3BA0.c`, from
    `src/levels/14_aranos_prison/L14_00304190-g8.c` (`func_L14_003E2688`, author commit `bde5ccd6`, DrLegitamate),
    task #2581. VERBATIM but for names: our name at the v2.00 place; six callees as L14's copies / v2.00 places; three `.sdata` floats as our labels `D_L14_001B0460`/`0464`/`0468`; `sda` kept as written; the file's include, other functions and their externs not carried; asm_unit route, s136 -G8.
  - **`func_L14_003E5F58`** (`0x003E5F58`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_003E5F58.c`, from
    `src/levels/14_aranos_prison/L14_00304190.c` (`func_L14_003E4A40`, author commit `913bd1bc`, Gianni),
    task #2581. ADAPTED: built at -G8 where upstream builds the file at -G0 (a tsv gflag setting; at -G0 .text is 0x154, not 0x158); our name at the v2.00 place; five callees as L14's copies; the boot's `g_listenerPosHistory`; a repeated declaration carried once; the file's include, other functions and their externs not carried; asm_unit route, s136.
  - **`func_L14_0040B080`** (`0x0040B080`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_0040B080.c`, from
    `src/levels/14_aranos_prison/L14_003F4F98-g8.c` (`func_L14_00409B20`, author commit `daf0e4a1`, Flavius),
    task #2581. VERBATIM but for names: our name at the v2.00 place; eight callees as L14's copies; `D_L14_001B259C` as the v2.00 relocation target `D_L14_001B251C`, address-named (RULING #13291 §2); `sda` kept as written; a repeated declaration carried once; the file's include, other functions and their externs not carried; asm_unit route, s136 -G8.
  - **`func_L14_00415288`** (`0x00415288`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_00415288.c`, from
    `src/levels/14_aranos_prison/L14_003F4F98.c` (`func_L14_00413CD8`, author commit `7fccf735`, Flavius; upstream's root commit, which squashed the earlier RAC2 history),
    task #2563. VERBATIM but for names: our name at the v2.00 place; four callees as L14's copies; `D_L14_001BF980` as the v2.00 relocation target `D_L14_001BF900`, address-named (RULING #13291 §2); the audit's `long long`->`long` respelling touches no line carried.
  - **`func_L14_004220D0`** (`0x004220D0`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_004220D0.c`, from
    `src/levels/14_aranos_prison/L14_003F4F98.c` (`func_L14_00420B10`, author commit `4912fc03`, bigbeno37),
    task #2563. ADAPTED: our name at the v2.00 place; two callees as L14's copies; `D_L14_001B259C` as the v2.00 relocation target `D_L14_001B251C`, address-named (RULING #13291 §2), its `sda` attribute written `section(".sdata")` (RULING #8620); the audit's `long long`->`long` respelling touches no line carried.
  - **`func_L14_00424560`** (`0x00424560`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_00424560.c`, from
    `src/levels/14_aranos_prison/L14_003F4F98.c` (`func_L14_00422FA0`, author commit `91e04732`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; five callees at their v2.00 places; the audit's `long long`->`long` respelling touches no line carried.
  - **`func_L14_00424800`** (`0x00424800`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_00424800.c`, from
    `src/levels/14_aranos_prison/L14_003F4F98-g8.c` (`func_L14_00423240`, author commit `daf0e4a1`, Flavius),
    task #2581. VERBATIM but for names: our name at the v2.00 place; five callees as L14's copies; `D_L14_001B16B0` as our label `D_L14_001B1650`; `sda` kept as written; the file's include, other functions and their externs not carried; asm_unit route, s136 -G8.
  - **`func_L14_00425380`** (`0x00425380`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_00425380.c`, from
    `src/levels/14_aranos_prison/L14_003F4F98.c` (`func_L14_00423DC0`, author commit `91e04732`, llesieur99),
    task #2563. VERBATIM but for names: our name at the v2.00 place; four callees as L14's copies; the audit's `long long`->`long` respelling touches no line carried.
  - **`func_L14_004283B8`** (`0x004283B8`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_004283B8.c`, from
    `src/levels/14_aranos_prison/L14_003F4F98.c` (`func_L14_00426DE8`, author commit `431ac503`, llesieur99),
    task #2563. ADAPTED: our name at the v2.00 place; the boot's `g_gsScreenContext` (upstream's `short []` type kept); two callees as L14's copies; one `unsigned long` parameter written `u64` (RULING #13197), typedef written out.
  - **`func_L14_00429CD8`** (`0x00429CD8`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_00429CD8.c`, from
    `src/levels/14_aranos_prison/L14_003F4F98.c` (`func_L14_004286F8`, author commit `4541c4ce`, ReaperOfVeriod),
    task #2581. ADAPTED: built at -G8 where upstream builds the file at -G0 (at -G0 .text is 0x84, not 0x7C) and with the row's cc1flags `-fno-gcse` (without it 0x80); our name at the v2.00 place; `D_L14_001B259C_1` and `D_L14_001FB060` as the v2.00 relocation targets `D_L14_001B251C` and `D_L14_001FAFE0`, address-named (RULING #13291 §2); `sda` kept as written; the file's include, other functions and their externs not carried; sdk29, plain as.
  - **`func_L14_0042E5F8`** (`0x0042E5F8`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_0042E5F8.c`, from
    `src/levels/14_aranos_prison/L14_003F4F98.c` (`func_L14_0042D018`, author commit `431ac503`, llesieur99),
    task #2563. ADAPTED: our name at the v2.00 place; the boot's `sqrt`, `dpmul`, `dpdiv`, `dptofp` for upstream's 0x80000000-based spellings; `func_801234F0` as `func_001234F0`, address-named; one callee as L14's copy; `long long` written `s64` (RULING #13197), typedef written out.
  - **`func_L14_004310F0`** (`0x004310F0`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_004310F0.c`, from
    `src/levels/14_aranos_prison/L14_0042FB10-effect-g8.c` (`func_L14_0042FB10`, author commit `4912fc03`, bigbeno37),
    task #2581. ADAPTED: its two `sda` attributes written `section(".sdata")` (RULING #8620; with `sda`, 12 bytes differ); `NOT_SDA` written out as its `section(".data")` expansion; our name at the v2.00 place; ten callees as L14's copies; the boot's `g_listenerPosHistory` and `g_soundBankHandlesBlk`; `D_001395E3` respelled `D_1395E3`, the boot's existing dlabel at that address (task #2621), address-named; `D_L14_001B259C` / `D_L14_001B259C_1` as `D_L14_001B251C` / `D_001B251C`, two address-named spellings of the one v2.00 target (RULING #13291 §2; C cannot give one name the two types upstream declares); the file's include not carried; asm_unit route, s136 -G8.
  - **`func_L14_004337B0`** (`0x004337B0`, level program 14) in
    `going-decompiled/levels/usa/L14/src/func_L14_004337B0.c`, from
    `src/levels/14_aranos_prison/L14_003F4F98.c` (`func_L14_004321D0`, author commit `312b7eea`, Louis-Philippe Le Sieur),
    task #2563. VERBATIM but for names: our name at the v2.00 place; the boot's `g_listenerPosHistory`; eight callees as L14's copies / v2.00 places, one of them through upstream's asm label; the audit's `long long`->`long` respelling touches no line carried.
  - **`func_L02_002EE5A0`** (level program 02, Maktar Nebula: shifts the records of two tables by one period toward the side of a limit) from `src/levels/02_maktar_nebula/L02_002ED7B0.c` (`func_L02_002ED7B0`, v1.01 address, author commit `6e138d9a`, bigmak94). ADAPTED: `sda` -> `section(".sdata")` on four extern declarations (RULING #8620); the four address-named externs keep their upstream names (RULING #13291 §2); includes and unused declarations dropped. Task #2557.
  - **`func_L02_0035CF30`** (level program 02, Maktar Nebula: builds the level-ordered menu rows and selects the current owner's entry; level 24's func_L24_0034D598 is the same function) from `src/levels/02_maktar_nebula/L02_0035BD60-init-g8.c` (`func_L02_0035BD60`, v1.01 address, author commit `a96d3126`, Louis-Philippe Le Sieur). VERBATIM but for names: the boot's `g_anAvailableLevelOrder`; three level symbols to our split labels and `D_L02_0021E630` to the address-named `D_L02_0021E660`; `volatile` and `sda` kept; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003CC348`** (level program 02, Maktar Nebula: runs an actor's move query and passes its result on) from `src/levels/02_maktar_nebula/L02_002ED7B0.c` (`func_L02_003CADF0`, v1.01 address, author commit `91e04732`, llesieur99). VERBATIM but for names: two callees to our split names; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003CEB08`** (level program 02, Maktar Nebula: finds the first entity of a group whose state byte is 0x17) from `src/levels/02_maktar_nebula/L02_002ED7B0.c` (`func_L02_003CD5B0`, v1.01 address, author commit `ac1ee013`, bigmak94). ADAPTED: `sda` -> `section(".sdata")` on two extern declarations (RULING #8620); three address-named externs keep their upstream names; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003D2988`** (level program 02, Maktar Nebula: updates an actor and the sub-object its state holds) from `src/levels/02_maktar_nebula/L02_002ED7B0.c` (`func_L02_003D1430`, v1.01 address, author commit `91e04732`, llesieur99). VERBATIM but for names: four callees to our split names; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003D2A50`** (level program 02, Maktar Nebula: picks an actor's mode by whether its state holds a sub-object) from `src/levels/02_maktar_nebula/L02_002ED7B0.c` (`func_L02_003D14F8`, v1.01 address, author commit `91e04732`, llesieur99). VERBATIM but for names: two callees to our split names; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003D2DD8`** (level program 02, Maktar Nebula: predicate: is the actor in an engaged state while holding a sub-object) from `src/levels/02_maktar_nebula/L02_002ED7B0.c` (`func_L02_003D1880`, v1.01 address, author commit `91e04732, last changed 863719df`, llesieur99; Louis-Philippe Le Sieur). VERBATIM but for names: none besides the function name (no relocations); includes and unused declarations dropped. Task #2557.
  - **`func_L02_003D2E30`** (level program 02, Maktar Nebula: answers whether the object's group holds an armed 0x9CA entity) from `src/levels/02_maktar_nebula/L02_002ED7B0-g8.c` (`func_L02_003D18D8`, v1.01 address, author commit `ac1ee013`, bigmak94). VERBATIM but for names: two address-named externs keep their upstream names; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003D37A0`** (level program 02, Maktar Nebula: brings an actor into its armed state and re-registers its sub-object) from `src/levels/02_maktar_nebula/L02_002ED7B0.c` (`func_L02_003D2248`, v1.01 address, author commit `431ac503`, llesieur99). VERBATIM but for names: six callees to our split names; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003DC2F0`** (level program 02, Maktar Nebula: runs six set-up steps on the entity the actor's state names) from `src/levels/02_maktar_nebula/L02_003D26B0.c` (`func_L02_003DAD88`, v1.01 address, author commit `431ac503`, llesieur99). ADAPTED: four callees to our split names; `sda` -> `section(".sdata")` on the `D_L02_001B359C` extern declaration (RULING #8620), which keeps its address name; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003DCC40`** (level program 02, Maktar Nebula: switches an actor off) from `src/levels/02_maktar_nebula/L02_003DB310.c` (`func_L02_003DB6D8`, v1.01 address, author commit `0a1d634f`, bigmak94). ADAPTED: the boot's `g_listenerPosHistory` for `D_00188660`; six callees to our split names; `sda` -> `section(".sdata")` on the `D_L02_001B359C` extern declaration (RULING #8620), which keeps its address name; the trailing `#undef` dropped; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003F69A0`** (level program 02, Maktar Nebula: rebuilds an actor's bit mask of answering records) from `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003F5430`, v1.01 address, author commit `adcbd51e`, llesieur99). ADAPTED: one callee to our split name; `sda` -> `section(".sdata")` on the `D_L02_001B3AF8` extern declaration (RULING #8620), which keeps its address name; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003FCA78`** (level program 02, Maktar Nebula: claims the first idle entity of a kind in the actor's group) from `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FB4D8`, v1.01 address, author commit `9e5c365f`, bigmak94). ADAPTED: one callee to our split name; built at -G8 where upstream's file is -G0 (an `L02_c.tsv` gflag); two address-named externs keep their upstream names; `sda` kept (our cc1 ignores it); includes and unused declarations dropped. Task #2557.
  - **`func_L02_003FCB40`** (level program 02, Maktar Nebula: claims and starts the first idle entity of a kind in the actor's group) from `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FB5A0`, v1.01 address, author commit `7a7816b9`, bigmak94). ADAPTED: two callees to our split names; `sda` -> `section(".sdata")` on the `D_L02_001B359C` extern declaration (RULING #8620); two address-named externs keep their upstream names; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003FD130`** (level program 02, Maktar Nebula: runs the per-kind handler of every entity in the actor's group) from `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FBB90`, v1.01 address, author commit `455cde53`, Flavius). ADAPTED: six callees to our split names; `sda` -> `section(".sdata")` on the `D_L02_001B359C` extern declaration (RULING #8620); two address-named externs keep their upstream names; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003FD638`** (level program 02, Maktar Nebula: returns the address of the vector in the actor's 128-byte record) from `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FC098`, v1.01 address, author commit `adcbd51e`, llesieur99). ADAPTED: `sda` -> `section(".sdata")` on the `D_L02_001B368C` extern declaration (RULING #8620), which keeps its address name; includes and unused declarations dropped. Task #2557.
  - **`func_L02_003FE020`** (level program 02, Maktar Nebula: starts the actor) from `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FCA80`, v1.01 address, author commit `24d977dd`, bigmak94). ADAPTED: the boot's `g_heroPos` for `D_00189EA0`; thirteen callees to our split names; `sda` -> `section(".sdata")` on three extern declarations (RULING #8620); the address-named externs keep their upstream names, except `D_001395B8`, respelled `D_1395B8` (the boot's and the L02 split asm's existing name for that address, task #2601); includes and unused declarations dropped. Task #2557.
  - **`func_L02_00402258`** (level program 02, Maktar Nebula: runs the per-kind handler of every object in a list) from `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_00400B10`, v1.01 address, author commit `6eae4d38`, bigmak94). ADAPTED: nine callees to our split names; `sda` -> `section(".sdata")` on the `D_L02_001B35A8` extern declaration (RULING #8620), which keeps its address name; includes and unused declarations dropped. Task #2557.
  - **`func_L02_004112C8`** (level program 02, Maktar Nebula: steps an actor by its kind byte) from `src/us2/levels/02_maktar_nebula/L02_004112C8.c` (`func_US2_L02_004112C8`, v2.00 address, author commit `50c79a7f`, Flavius). ADAPTED: the boot's `g_heroPos` for `D_00189EA0`; `D_001A7BCB` to `D_1A7BD3` (the address the ROM's relocations imply; the boot's existing spelling, its own dlabel, task #2621) and `D_L02_001395B8` to `D_1395B8` (a core address; the boot's and the L02 split asm's existing name, an operand name with no label of its own, task #2601), both address-named; eight callees to our split names; `sda` -> `section(".sdata")` on the `D_1A7BD3` extern declaration (RULING #8620); includes and unused declarations dropped. Task #2557.
  - **`func_L02_00425010`** (level program 02, Maktar Nebula: throws twenty sparks from an actor) from `src/levels/02_maktar_nebula/L02_0040A4A8.c` (`func_L02_00423838`, v1.01 address, author commit `11c21414`, bigmak94). ADAPTED: two colour-pair symbols and eight callees to our split names; `sda` -> `section(".sdata")` on the two colour-pair extern declarations (RULING #8620); includes and unused declarations dropped. Task #2557.
  - **`func_L11_003983E0`** (`0x003983E0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003983E0.c`, from
    `src/levels/11_joba/L11_002DE9E0-g8.c`
    (`func_L11_00397068`, author commit `daf0e4a1`, Flavius), task #2578.
    Verbatim but for names: our name, 1 data name, 6 callees renamed to
    the v2.00 disc's own relocation targets; `common.h` dropped,
    typedefs written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_003F3178`** (`0x003F3178`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F3178.c`, from
    `src/levels/11_joba/L11_003D6270.c`
    (`func_L11_003F1C68`, author commit `91e04732`, llesieur99), task #2578.
    Adapted: built at -G8 (upstream -G0), text verbatim but for names;
    our name, 1 data name, 8 callees renamed to the v2.00 disc's own
    relocation targets; `common.h` dropped, typedefs written; assembled
    through `tools/ee/asm_unit.sh`.
  - **`func_L11_003F4448`** (`0x003F4448`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F4448.c`, from
    `src/levels/11_joba/L11_003F2F38-g8.c`
    (`func_L11_003F2F38`, author commit `eabc4759`, Flavius), task #2578.
    Verbatim but for names: our name, 2 data names, 8 callees renamed to
    the v2.00 disc's own relocation targets; `common.h` dropped,
    typedefs written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_00418AB0`** (`0x00418AB0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00418AB0.c`, from
    `src/levels/11_joba/L11_003D6270-g8.c`
    (`func_L11_00417340`, author commit `aaa28c3b`, Flavius), task #2578.
    Verbatim but for names: our name, 2 data names
    (`g_soundBankHandlesBlk` the boot's), 11 callees renamed to the
    v2.00 disc's own relocation targets; `common.h` dropped, typedefs
    written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_00426FA0`** (`0x00426FA0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00426FA0.c`, from
    `src/levels/11_joba/L11_00420C88.c`
    (`func_L11_00425788`, author commit `aaa28c3b`, Flavius), task #2578.
    Adapted: built at -G8 (upstream -G0), text verbatim but for names;
    our name, 10 callees renamed to the v2.00 disc's own relocation
    targets; `common.h` dropped, typedefs written; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L11_0042B988`** (`0x0042B988`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0042B988.c`, from
    `src/levels/11_joba/L11_00425F10.c`
    (`func_L11_0042A158`, author commit `76d5f55b`, Flavius), task #2578.
    Adapted: built at -G8 (upstream -G0), text verbatim but for names;
    our name, 10 callees renamed to the v2.00 disc's own relocation
    targets; `common.h` dropped, typedefs written; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L11_00436338`** (`0x00436338`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00436338.c`, from
    `src/levels/11_joba/L11_00432628-g8.c`
    (`func_L11_00434AF8`, author commit `9ad4d690`, Flavius), task #2578.
    Adapted: `sda` as `section(".sdata")` (RULING #8620); our name, 5
    data names, 6 callees renamed to the v2.00 disc's own relocation
    targets; `common.h` dropped, typedefs written; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L11_00437B90`** (`0x00437B90`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00437B90.c`, from
    `src/levels/11_joba/L11_00436350-g8.c`
    (`func_L11_00436350`, author commit `644c5b4d`, bigbeno37), task #2578.
    Verbatim but for names: our name, 1 data name, 20 callees renamed to
    the v2.00 disc's own relocation targets; `common.h` dropped,
    typedefs written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_0043AE08`** (`0x0043AE08`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0043AE08.c`, from
    `src/levels/11_joba/L11_00432628-g8.c`
    (`func_L11_004395C8`, author commit `4541c4ce`, ReaperOfVeriod), task #2578.
    Verbatim but for names: our name, 1 data name renamed to the v2.00
    disc's own relocation targets; `common.h` dropped, typedefs written;
    assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_0043BC38`** (`0x0043BC38`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0043BC38.c`, from
    `src/levels/11_joba/L11_00432628.c`
    (`func_L11_0043A3F8`, author commit `5acf9869`, Flavius), task #2578.
    Adapted: built at -G8 (upstream -G0), text verbatim but for names;
    our name, 1 data name, 9 callees renamed to the v2.00 disc's own
    relocation targets; `common.h` dropped, typedefs written; assembled
    through `tools/ee/asm_unit.sh`.
  - **`func_L11_0043C218`** (`0x0043C218`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0043C218.c`, from
    `src/levels/11_joba/L11_00432628-g8.c`
    (`func_L11_0043A9D8`, author commit `47360109`, Flavius), task #2578.
    Verbatim but for names: our name, 1 data name, 5 callees renamed to
    the v2.00 disc's own relocation targets; `common.h` dropped,
    typedefs written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_00442668`** (`0x00442668`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00442668.c`, from
    `src/levels/11_joba/L11_0043E1A0-g8.c`
    (`func_L11_00440E18`, author commit `d932dae8`, bigmak94), task #2578.
    Adapted: `sda` as `section(".sdata")` (RULING #8620); our name, 5
    data names (`g_heroPos` the boot's), 3 callees renamed to the v2.00
    disc's own relocation targets; `common.h` dropped, typedefs written;
    assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_00443C48`** (`0x00443C48`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00443C48.c`, from
    `src/levels/11_joba/L11_0043E1A0-g8.c`
    (`func_L11_004423F8`, author commit `c7d9e30a`, bigbeno37), task #2578.
    Verbatim but for names: our name, 16 data names
    (`g_soundBankHandlesBlk` the boot's), 4 callees renamed to the v2.00
    disc's own relocation targets; `common.h` dropped, typedefs written;
    assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_00443F60`** (`0x00443F60`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00443F60.c`, from
    `src/levels/11_joba/L11_0043E1A0-g8.c`
    (`func_L11_00442710`, author commit `daf0e4a1`, Flavius), task #2578.
    Verbatim but for names: our name, 1 data name renamed to the v2.00
    disc's own relocation targets; `common.h` dropped, typedefs written;
    assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_00444BD0`** (`0x00444BD0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00444BD0.c`, from
    `src/levels/11_joba/L11_0043E1A0-g8.c`
    (`func_L11_00443380`, author commit `264cb2be`, llesieur99), task #2578.
    Verbatim but for names: our name, 4 data names, 5 callees renamed to
    the v2.00 disc's own relocation targets; `common.h` dropped,
    typedefs written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_00444CE8`** (`0x00444CE8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00444CE8.c`, from
    `src/levels/11_joba/L11_00443498-g8.c`
    (`func_L11_00443498`, author commit `3cd8af2d`, bigbeno37), task #2578.
    Verbatim but for names: our name, 5 data names (`g_heroState`,
    `g_itemEquippedSlot` the boot's), 15 callees renamed to the v2.00
    disc's own relocation targets; `common.h` dropped, typedefs written;
    assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_00449ED0`** (`0x00449ED0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00449ED0.c`, from
    `src/levels/11_joba/L11_004465F8-g8.c`
    (`func_L11_004484A8`, author commit `3cd8af2d`, bigbeno37), task #2578.
    Verbatim but for names: our name, 2 data names, 16 callees renamed
    to the v2.00 disc's own relocation targets; `common.h` dropped,
    typedefs written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_0044A4D0`** (`0x0044A4D0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044A4D0.c`, from
    `src/levels/11_joba/L11_00448AA8.c`
    (`func_L11_00448AA8`, author commit `d282effa`, Flavius), task #2578.
    Adapted: built at -G8 (upstream -G0), text verbatim but for names;
    our name, 1 data name (`g_heroPos` the boot's), 9 callees renamed to
    the v2.00 disc's own relocation targets; `common.h` dropped,
    typedefs written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_0044AAC0`** (`0x0044AAC0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044AAC0.c`, from
    `src/levels/11_joba/L11_004465F8.c`
    (`func_L11_00449098`, author commit `5acf9869`, Flavius), task #2578.
    Adapted: built at -G8 (upstream -G0), text verbatim but for names;
    our name, 7 callees renamed to the v2.00 disc's own relocation
    targets; `common.h` dropped, typedefs written; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L11_0044AD58`** (`0x0044AD58`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044AD58.c`, from
    `src/levels/11_joba/L11_00449330.c`
    (`func_L11_00449330`, author commit `72a6596f`, Flavius), task #2578.
    Adapted: built at -G8 (upstream -G0); `sda` as `section(".sdata")`
    (RULING #8620); our name, 4 callees renamed to the v2.00 disc's own
    relocation targets; `common.h` dropped, typedefs written; assembled
    through `tools/ee/asm_unit.sh`.
  - **`func_L11_0044D7A0`** (`0x0044D7A0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044D7A0.c`, from
    `src/levels/11_joba/L11_0044BD68-g8.c`
    (`func_L11_0044BD68`, author commit `644c5b4d`, bigbeno37), task #2578.
    Adapted: `sda` as `section(".sdata")` (RULING #8620); our name, 2
    data names, 17 callees renamed to the v2.00 disc's own relocation
    targets; `common.h` dropped, typedefs written; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L11_00453EA8`** (`0x00453EA8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00453EA8.c`, from
    `src/levels/11_joba/L11_004465F8-g8.c`
    (`func_L11_00452450`, author commit `96f56db3`, bigbeno37), task #2578.
    Verbatim but for names: our name, 2 data names, 19 callees renamed
    to the v2.00 disc's own relocation targets; `common.h` dropped,
    typedefs written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_00455F60`** (`0x00455F60`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00455F60.c`, from
    `src/levels/11_joba/L11_00453358-g8.c`
    (`func_L11_00454508`, author commit `4014e0b6`, Flavius), task #2578.
    Verbatim but for names: our name, 2 data names
    (`g_listenerPosHistory` the boot's), 27 callees renamed to the v2.00
    disc's own relocation targets; no `long long` in this body, so no
    respelling (FACT #13321); `common.h` dropped, typedefs written;
    assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_0045A6F0`** (`0x0045A6F0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0045A6F0.c`, from
    `src/levels/11_joba/L11_00458C38-g8.c`
    (`func_L11_00458C38`, author commit `5f3b0386`, Louis-Philippe Le Sieur), task #2578.
    Verbatim but for names: our name, 4 data names, 4 callees renamed to
    the v2.00 disc's own relocation targets; upstream's `SDATA()` macro
    written out as its section attribute; `common.h` dropped, typedefs
    written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L11_0045B800`** (`0x0045B800`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0045B800.c`, from
    `src/levels/11_joba/L11_00453358-g8.c`
    (`func_L11_00459D48`, author commit `431ac503`, llesieur99), task #2578.
    Verbatim but for names: our name, 1 data name, 8 callees renamed to
    the v2.00 disc's own relocation targets; no `long long` in this
    body, so no respelling (FACT #13321); `common.h` dropped, typedefs
    written; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L09_002B5E70`** (`0x002B5E70`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_002B5E70.c`, from
    `src/levels/09_dobbo/L09_002AFC90.c` (`func_L09_002B5CE8`, author commit
    `aac730b3`, Flavius), task #2580. ADAPTED: our name at the v2.00 place;
    `D_00189E20` as the boot's `g_soundBankHandlesBlk`; three callees
    renamed to their L09 places; built at -G8 against upstream's -G0;
    `include_asm.h` dropped; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L09_003FCC18`** (`0x003FCC18`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003FCC18.c`, from
    `src/levels/09_dobbo/L09_003FB788.c` (`func_L09_003FB788`, author commit
    `757a3e46`, bigbeno37), task #2580. ADAPTED: our name at the v2.00
    place; four data names as address-named externs (RULING #13291 section
    2); one callee renamed to its L09 place; built at -G8 against upstream's
    -G0; upstream's `MACRO_ADDR` (`section(".sdata")`, RULING #8620) kept
    and defined in the TU; `common.h` dropped; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L09_003FCCF0`** (`0x003FCCF0`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003FCCF0.c`, from
    `src/levels/09_dobbo/L09_003FB860.c` (`func_L09_003FB860`, author commit
    `88c5f521`, Flavius), task #2580. ADAPTED: our name at the v2.00 place;
    four data names as address-named externs (RULING #13291 section 2); one
    callee renamed to its L09 place; built at -G8 against upstream's -G0;
    upstream's `MACRO_ADDR` (`section(".sdata")`, RULING #8620) kept and
    defined in the TU; `include_asm.h`/`common.h` dropped; brace moved
    (whitespace only); assembled through `tools/ee/asm_unit.sh`.
  - **`func_L09_003FF768`** (`0x003FF768`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003FF768.c`, from
    `src/levels/09_dobbo/L09_002F8BA0-g8.c` (`func_L09_003FE2D8`, author
    commit `ba383701`, bigbeno37), task #2580. VERBATIM but for names: our
    name at the v2.00 place; one data label and one callee renamed to their
    L09 targets (`sda` text kept, ignored by cc1); `common.h` dropped;
    assembled through `tools/ee/asm_unit.sh`.
  - **`func_L09_003FFF78`** (`0x003FFF78`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003FFF78.c`, from
    `src/levels/09_dobbo/L09_003FEAE8-g8.c` (`func_L09_003FEAE8`, author
    commit `1794e699`, Louis-Philippe Le Sieur), task #2580. VERBATIM but
    for names: our name at the v2.00 place; seven data labels and seven
    callees renamed to their L09 targets (`sda` text kept, ignored by cc1);
    `common.h` dropped, its typedefs written in the TU; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L09_00406020`** (`0x00406020`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_00406020.c`, from
    `src/levels/09_dobbo/L09_003FEA48-g8.c` (`func_L09_00404B90`, author
    commit `264cb2be`, llesieur99), task #2580. VERBATIM but for names: our
    name at the v2.00 place; one data label and one callee renamed to their
    L09 targets (`sda` text kept, ignored by cc1); `common.h` dropped, its
    typedefs written in the TU; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L09_00406080`** (`0x00406080`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_00406080.c`, from
    `src/levels/09_dobbo/L09_003FEA48-g8.c` (`func_L09_00404BF0`, author
    commit `264cb2be`, llesieur99), task #2580. VERBATIM but for names: our
    name at the v2.00 place; one data label and one callee renamed to their
    L09 targets (`sda` text kept, ignored by cc1); `common.h` dropped, its
    typedefs written in the TU; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L09_0040C5B8`** (`0x0040C5B8`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_0040C5B8.c`, from
    `src/levels/09_dobbo/L09_003FEA48-g8.c` (`func_L09_0040B0C8`, author
    commit `27b929ce`, Lynder063), task #2580. VERBATIM but for names: our
    name at the v2.00 place; five data labels and nine callees renamed to
    their L09 targets (`sda` text kept, ignored by cc1; `func_L09_002EB958`
    still undeclared, as upstream); upstream's second name
    `D_L09_001B15B8_4` for 0x001B1578 as the unprefixed address name
    `D_001B1578`, since `D_L09_001B1578` is the scalar's; `common.h`
    dropped, its typedefs and `MACRO_ADDR` written in the TU; assembled
    through `tools/ee/asm_unit.sh`.
  - **`func_L09_0041EB58`** (`0x0041EB58`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_0041EB58.c`, from
    `src/levels/09_dobbo/L09_0041D650-g8.c` (`func_L09_0041D650`, author
    commit `913f6380`, bigbeno37), task #2580. VERBATIM but for names: our
    name at the v2.00 place; four data labels and thirteen callees renamed
    to their L09 targets (`sda` text kept, ignored by cc1); `common.h`
    dropped; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L20_00441690`** (`0x00441690`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00441690.c`, from
    `src/levels/20_yeedil/L20_0043ECB0-g8.c`
    (`func_L20_00440150`, author commit `ee3fee45`, llesieur99), task #2566. Verbatim but for names: our name, 1 level data name, 1 callee renamed to the v2.00 disc's own relocation targets; upstream's per-file `-mno-split-addresses` not passed (measured inert); `include_asm.h` dropped; plain `as`.
  - **`func_L20_004419A0`** (`0x004419A0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_004419A0.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00440460`, author commit `db93b56d`, llesieur99), task #2566. Verbatim but for names: our name, 1 callee renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_004419F8`** (`0x004419F8`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_004419F8.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_004404B8`, author commit `ee3fee45`, llesieur99), task #2566. Verbatim but for names: our name, 5 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_00441AB0`** (`0x00441AB0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00441AB0.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00440570`, author commit `db93b56d`, llesieur99), task #2566. Verbatim but for names: our name, 1 callee renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_00443D10`** (`0x00443D10`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00443D10.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_004427C8`, author commit `adcbd51e`, llesieur99), task #2566. Verbatim but for names: our name, 4 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_00444158`** (`0x00444158`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00444158.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00442C10`, author commit `adcbd51e`, llesieur99), task #2566. Verbatim but for names: our name, 4 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_00444478`** (`0x00444478`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00444478.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00442F30`, author commit `db93b56d`, llesieur99), task #2566. Verbatim but for names: our name, 3 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_00444A50`** (`0x00444A50`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00444A50.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00443508`, author commit `765097bb`, ReaperOfVeriod), task #2566. Verbatim but for names: our name, 4 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_00444E50`** (`0x00444E50`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00444E50.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00443908`, author commit `3b12a35b`, Louis-Philippe Le Sieur), task #2566. Verbatim but for names: our name, 7 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_004450A8`** (`0x004450A8`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_004450A8.c`, from
    `src/levels/20_yeedil/L20_00443B60-helpers.c`
    (`func_L20_00443B60`, author commit `bf60a47f`, Louis-Philippe Le Sieur), task #2566. Verbatim but for names: our name, 6 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_00445238`** (`0x00445238`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00445238.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00443CF0`, author commit `c17a465e`, Fromnowhere), task #2566. Verbatim but for names: our name, 2 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_004452A0`** (`0x004452A0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_004452A0.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00443D58`, author commit `c17a465e`, Fromnowhere), task #2566. Verbatim but for names: our name, 3 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_004461B0`** (`0x004461B0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_004461B0.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00444C68`, author commit `db93b56d`, llesieur99), task #2566. Adapted, no device: our name, 3 callees renamed to the v2.00 disc's own relocation targets; one extern prototype added for `func_L20_00441AB0`, defined earlier in the upstream file; `common.h` dropped; plain `as`.
  - **`func_L20_00448E50`** (`0x00448E50`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00448E50.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00447908`, author commit `db93b56d`, llesieur99), task #2566. Verbatim but for names: our name, 1 callee renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`.
  - **`func_L20_0044AFC0`** (`0x0044AFC0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0044AFC0.c`, from
    `src/levels/20_yeedil/L20_00449178.c`
    (`func_L20_00449A78`, author commit `ee3fee45`, llesieur99), task #2566. Verbatim but for names: our name, 1 callee renamed to the v2.00 disc's own relocation targets; `include_asm.h` dropped; plain `as`.
  - **`func_L20_0044B020`** (`0x0044B020`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0044B020.c`, from
    `src/levels/20_yeedil/L20_00449178.c`
    (`func_L20_00449AD8`, author commit `ee3fee45`, llesieur99), task #2566. Verbatim but for names: our name (no relocations); `include_asm.h` dropped; plain `as`.
  - **`func_L20_0044B030`** (`0x0044B030`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0044B030.c`, from
    `src/levels/20_yeedil/L20_00449178-g8.c`
    (`func_L20_00449AE8`, author commit `e0711884`, ReaperOfVeriod), task #2566. Verbatim but for names: our name, 2 level data names, 5 callees renamed to the v2.00 disc's own relocation targets; `include_asm.h` dropped; plain `as`.
  - **`func_L20_0044B650`** (`0x0044B650`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0044B650.c`, from
    `src/levels/20_yeedil/L20_00449178.c`
    (`func_L20_0044A108`, author commit `ee3fee45`, llesieur99), task #2566. Verbatim but for names: our name, 3 callees renamed to the v2.00 disc's own relocation targets; `include_asm.h` dropped; plain `as`.
  - **`func_L20_0044C0B0`** (`0x0044C0B0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0044C0B0.c`, from
    `src/levels/20_yeedil/L20_00449178.c`
    (`func_L20_0044AB68`, author commit `3b12a35b`, Louis-Philippe Le Sieur), task #2566. Verbatim but for names: our name, 4 callees renamed to the v2.00 disc's own relocation targets; `include_asm.h` dropped; plain `as`.
  - **`func_L23_003C1278`** (`0x003C1278`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_003C1278.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_003BFE38`, author commit `91e04732`, llesieur99), task #2567.
    VERBATIM but for names.
  - **`func_L23_003C1400`** (`0x003C1400`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_003C1400.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_003BFFC0`, author commit `91e04732`, llesieur99), task #2567.
    VERBATIM but for names.
  - **`func_L23_003C1468`** (`0x003C1468`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_003C1468.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_003C0028`, author commit `91e04732`, llesieur99), task #2567.
    VERBATIM but for names.
  - **`func_L23_003C2238`** (`0x003C2238`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_003C2238.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_003C0DF8`, author commit `91e04732`, llesieur99), task #2567.
    ADAPTED: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; `unsigned long long` written `u64` (RULING #13197).
  - **`func_L23_003C22D8`** (`0x003C22D8`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_003C22D8.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_003C0E98`, author commit `ee3fee45`, llesieur99), task #2567.
    ADAPTED: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; `sda` as `section(".sdata")` (RULING #8620); `long long` written `s64` (RULING #13197); three address-named externs (RULING #13291 section 2).
  - **`func_L23_003C2388`** (`0x003C2388`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_003C2388.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_003C0F48`, author commit `91e04732`, llesieur99), task #2567.
    VERBATIM but for names.
  - **`func_L23_003C23C8`** (`0x003C23C8`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_003C23C8.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_003C0F88`, author commit `fa31bfc3`, bigmak94), task #2567.
    ADAPTED: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; two `sda` as `section(".sdata")` (RULING #8620); `long long` written `s64`/`u64` (RULING #13197); three address-named externs (RULING #13291 section 2); the boot's `g_heroPos` for `D_00189EA0`.
  - **`func_L23_003C2DA0`** (`0x003C2DA0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_003C2DA0.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_003C1960`, author commit `91e04732`, llesieur99), task #2567.
    ADAPTED: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; `sda` as `section(".sdata")` (RULING #8620); an address-named extern (RULING #13291 section 2).
  - **`func_L23_003C2DC8`** (`0x003C2DC8`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_003C2DC8.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_003C1988`, author commit `91e04732`, llesieur99), task #2567.
    VERBATIM but for names.
  - **`func_L23_003C2EF0`** (`0x003C2EF0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_003C2EF0.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_003C1AB0`, author commit `91e04732`, llesieur99), task #2567.
    ADAPTED: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; `sda` as `section(".sdata")` (RULING #8620); an address-named extern (RULING #13291 section 2).
  - **`func_L23_004174C8`** (`0x004174C8`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_004174C8.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90-g8.c`
    (`func_L23_00415FC8`, author commit `91e04732`, llesieur99), task #2567.
    Adapted: names, and inert `sda` attributes dropped at -G8.
  - **`func_L23_00417590`** (`0x00417590`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_00417590.c`, from
    `src/levels/23_damosel_orbit/L23_0035CE90.c`
    (`func_L23_00416090`, author commit `91e04732`, llesieur99), task #2567.
    VERBATIM but for names.
  - **`func_L23_004195F0`** (`0x004195F0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_004195F0.c`, from
    `src/levels/23_damosel_orbit/L23_00416388.c`
    (`func_L23_004180F0`, author commit `773eafc2`, llesieur99), task #2567.
    VERBATIM but for names.
  - **`func_L23_0041A430`** (`0x0041A430`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041A430.c`, from
    `src/levels/23_damosel_orbit/L23_00416388.c`
    (`func_L23_00418F30`, author commit `adcbd51e`, llesieur99), task #2567.
    ADAPTED: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; the file's `u64` typedef written `unsigned long` (RULING #13197).
  - **`func_L23_0041A4D0`** (`0x0041A4D0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041A4D0.c`, from
    `src/levels/23_damosel_orbit/L23_00416388.c`
    (`func_L23_00418FD0`, author commit `d2e11c19`, bigmak94), task #2567.
    ADAPTED: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; the file's `u64` typedef written `unsigned long` (RULING #13197); two address-named externs (RULING #13291 section 2).
  - **`func_L23_0041CDF8`** (`0x0041CDF8`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041CDF8.c`, from
    `src/levels/23_damosel_orbit/L23_00416388-g8.c`
    (`func_L23_0041B8F8`, author commit `773eafc2`, llesieur99), task #2567.
    Adapted: names, and inert `sda` attributes dropped at -G8.
  - **`func_L23_0041CE48`** (`0x0041CE48`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041CE48.c`, from
    `src/levels/23_damosel_orbit/L23_00416388-g8.c`
    (`func_L23_0041B948`, author commit `773eafc2`, llesieur99), task #2567.
    Adapted: names, and inert `sda` attributes dropped at -G8.
  - **`func_L23_0041CE60`** (`0x0041CE60`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041CE60.c`, from
    `src/levels/23_damosel_orbit/L23_00416388-g8.c`
    (`func_L23_0041B960`, author commit `adcbd51e`, llesieur99), task #2567.
    Adapted: names, and inert `sda` attributes dropped at -G8.
  - **`func_L23_0041D540`** (`0x0041D540`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041D540.c`, from
    `src/levels/23_damosel_orbit/L23_00416388-g8.c`
    (`func_L23_0041C040`, author commit `adcbd51e`, llesieur99), task #2567.
    Adapted: names, and inert `sda` attributes dropped at -G8.
  - **`func_L23_0041D5B0`** (`0x0041D5B0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041D5B0.c`, from
    `src/levels/23_damosel_orbit/L23_00416388-g8.c`
    (`func_L23_0041C0B0`, author commit `ee3fee45`, llesieur99), task #2567.
    Adapted: names, and inert `sda` attributes dropped at -G8.
  - **`func_L23_0041DCD8`** (`0x0041DCD8`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041DCD8.c`, from
    `src/levels/23_damosel_orbit/L23_00416388-g8.c`
    (`func_L23_0041C7D8`, author commit `babfe411`, bigmak94), task #2567.
    ADAPTED: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; `nosda` as `section(".data")` on the boot's `g_pHeroMoby` for `D_0018C0B0` (RULING #8620); an address-named extern (RULING #13291 section 2).
  - **`func_L02_003D38E8`** (`0x003D38E8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D38E8.c`, from
    `src/levels/02_maktar_nebula/L02_003D2390.c`
    (`func_L02_003D2390`, author commit `291c840e`, Flavius), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D5C20`** (`0x003D5C20`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D5C20.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D46B8`, author commit `e80a585a`, Flavius), task #2558.
    VERBATIM but for names; `D_0018C2BC` respelled `D_18C2BC`, the boot
    asm's existing operand name for that address (task #2621); it stays an
    address-named extern (RULING #13291 §2).
  - **`func_L02_003D5E68`** (`0x003D5E68`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D5E68.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D4900`, author commit `431ac503`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D5EE8`** (`0x003D5EE8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D5EE8.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D4980`, author commit `431ac503`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D5EF8`** (`0x003D5EF8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D5EF8.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D4990`, author commit `ac1ee013`, bigmak94), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D5FB0`** (`0x003D5FB0`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D5FB0.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D4A48`, author commit `773eafc2`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D5FC8`** (`0x003D5FC8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D5FC8.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D4A60`, author commit `773eafc2`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D6268`** (`0x003D6268`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D6268.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D4D00`, author commit `773eafc2`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D6538`** (`0x003D6538`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D6538.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D4FD0`, author commit `ee3fee45`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D6BC8`** (`0x003D6BC8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D6BC8.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D5660`, author commit `431ac503`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D70F8`** (`0x003D70F8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D70F8.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D5B90`, author commit `773eafc2`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D7188`** (`0x003D7188`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D7188.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D5C20`, author commit `e80a585a`, Flavius), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D7858`** (`0x003D7858`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D7858.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D62F0`, author commit `773eafc2`, llesieur99), task #2558.
    Adapted: names, and a prototype for func_L02_003D7F88, which upstream
    calls undeclared (byte-neutral).
  - **`func_L02_003D78B0`** (`0x003D78B0`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D78B0.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D6348`, author commit `773eafc2`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D7AA0`** (`0x003D7AA0`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D7AA0.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D6538`, author commit `773eafc2`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003D8118`** (`0x003D8118`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D8118.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003D6BB0`, author commit `773eafc2`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003DC858`** (`0x003DC858`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003DC858.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c`
    (`func_L02_003DB2F0`, author commit `773eafc2`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003DCDD8`** (`0x003DCDD8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003DCDD8.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c`
    (`func_L02_003DB870`, author commit `adcbd51e`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L02_003DCDE8`** (`0x003DCDE8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003DCDE8.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c`
    (`func_L02_003DB880`, author commit `a2d495f1`, Flavius), task #2558.
    VERBATIM but for names.
  - **`func_L02_003DCEA0`** (`0x003DCEA0`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003DCEA0.c`, from
    `src/levels/02_maktar_nebula/L02_003DB310.c`
    (`func_L02_003DB938`, author commit `773eafc2`, llesieur99), task #2558.
    VERBATIM but for names.
  - **`func_L23_0041A588`** (`0x0041A588`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041A588.c`, from
    `src/levels/23_damosel_orbit/L23_00416388.c`
    (`func_L23_00419088`, author commit `ee3fee45`, llesieur99), task
    #2568. VERBATIM but for names: our name at the v2.00 place.
  - **`func_L23_0041B1E0`** (`0x0041B1E0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041B1E0.c`, from
    `src/levels/23_damosel_orbit/L23_00416388.c`
    (`func_L23_00419CE0`, author commit `86ff805f`, bigmak94), task
    #2568. VERBATIM but for names: our name and 1 callee renamed to the v2.00 disc's own targets.
  - **`func_L23_0041C978`** (`0x0041C978`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041C978.c`, from
    `src/us2/levels/23_damosel_orbit/L23_0041C978.c`
    (`func_US2_L23_0041C978`, author commit `cde7e4dd`, Flavius), task
    #2568. VERBATIM but for names: our name and 1 callee renamed to the v2.00 disc's own targets.
  - **`func_L23_0041CAB0`** (`0x0041CAB0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041CAB0.c`, from
    `src/levels/23_damosel_orbit/L23_00416388.c`
    (`func_L23_0041B5B0`, author commit `773eafc2`, llesieur99), task
    #2568. VERBATIM but for names: our name at the v2.00 place.
  - **`func_L23_0041D5C0`** (`0x0041D5C0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041D5C0.c`, from
    `src/levels/23_damosel_orbit/L23_00416388-g8.c`
    (`func_L23_0041C0C0`, author commit `773eafc2`, llesieur99), task
    #2568. VERBATIM but for names: our name and 2 data names renamed to the v2.00 disc's own targets.
  - **`func_L23_0041EE30`** (`0x0041EE30`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041EE30.c`, from
    `src/levels/23_damosel_orbit/L23_00416388-g8.c`
    (`func_L23_0041D930`, author commit `adcbd51e`, llesieur99), task
    #2568. VERBATIM but for names: our name, 1 data name and 3 callees renamed to the v2.00 disc's own targets.
  - **`func_L23_0041EEB0`** (`0x0041EEB0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041EEB0.c`, from
    `src/us2/levels/23_damosel_orbit/L23_0041EEB0-g8.c`
    (`func_US2_L23_0041EEB0`, author commit `cde7e4dd`, Flavius), task
    #2568. VERBATIM but for names: our name, 2 data names and 3 callees renamed to the v2.00 disc's own targets, upstream's boot address `D_001395B8` respelled `D_1395B8`, the boot's existing spelling, address-named (RULING #13291 §2; task #2606).
  - **`func_L23_0041F720`** (`0x0041F720`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041F720.c`, from
    `src/levels/23_damosel_orbit/L23_00416388.c`
    (`func_L23_0041E220`, author commit `86ff805f`, bigmak94), task
    #2568. VERBATIM but for names: our name at the v2.00 place.
  - **`func_L23_0041F760`** (`0x0041F760`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041F760.c`, from
    `src/levels/23_damosel_orbit/L23_00416388-g8.c`
    (`func_L23_0041E260`, author commit `773eafc2`, llesieur99), task
    #2568. VERBATIM but for names: our name and 1 data name renamed to the v2.00 disc's own targets.
  - **`func_L23_0041F918`** (`0x0041F918`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041F918.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418-g8.c`
    (`func_L23_0041E418`, author commit `7a91db15`, bigmak94), task
    #2568. VERBATIM but for names: our name, the boot's `g_soundBankHandlesBlk`, 3 data names and 1 callee renamed to the v2.00 disc's own targets, `D_L23_001B1F10` address-named inside a larger label (RULING #13291 §2).
  - **`func_L23_0041FA48`** (`0x0041FA48`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0041FA48.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418-g8.c`
    (`func_L23_0041E548`, author commit `264cb2be`, llesieur99), task
    #2568. VERBATIM but for names: our name and 1 data name renamed to the v2.00 disc's own targets.
  - **`func_L23_00426EE0`** (`0x00426EE0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_00426EE0.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418-g8.c`
    (`func_L23_004259E0`, author commit `00641a1b`, bigmak94), task
    #2568. VERBATIM but for names: our name, 6 data names and 1 callee renamed to the v2.00 disc's own targets.
  - **`func_L23_004275F0`** (`0x004275F0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_004275F0.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418.c`
    (`func_L23_004260F0`, author commit `ac1d508f`, bigmak94), task
    #2568. VERBATIM but for names: our name, 1 data name and 8 callees renamed to the v2.00 disc's own targets, `D_L23_001BA5C0` address-named inside a larger label (RULING #13291 §2).
  - **`func_L23_00428CB8`** (`0x00428CB8`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_00428CB8.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418.c`
    (`func_L23_004277B8`, author commit `97da1aea`, Louis-Philippe Le Sieur), task
    #2568. VERBATIM but for names: our name and 2 callees renamed to the v2.00 disc's own targets.
  - **`func_L23_00428CE8`** (`0x00428CE8`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_00428CE8.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418-g8.c`
    (`func_L23_004277E8`, author commit `d2e11c19`, bigmak94), task
    #2568. VERBATIM but for names: our name, 1 data name and 2 callees renamed to the v2.00 disc's own targets, `D_L23_001B367C` address-named inside a larger label (RULING #13291 §2).
  - **`func_L23_00428DB0`** (`0x00428DB0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_00428DB0.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418.c`
    (`func_L23_004278B0`, author commit `264cb2be`, llesieur99), task
    #2568. VERBATIM but for names: our name at the v2.00 place.
  - **`func_L23_0042B4F8`** (`0x0042B4F8`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0042B4F8.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418.c`
    (`func_L23_00429FF8`, author commit `264cb2be`, llesieur99), task
    #2568. VERBATIM but for names: our name at the v2.00 place.
  - **`func_L23_0042C170`** (`0x0042C170`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0042C170.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418-g8.c`
    (`func_L23_0042AC70`, author commit `264cb2be`, llesieur99), task
    #2568. VERBATIM but for names: our name, 2 data names and 1 callee renamed to the v2.00 disc's own targets, `D_L23_001B1B44` address-named inside a larger label (RULING #13291 §2).
  - **`func_L23_0042C290`** (`0x0042C290`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_0042C290.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418.c`
    (`func_L23_0042AD90`, author commit `264cb2be`, llesieur99), task
    #2568. VERBATIM but for names: our name at the v2.00 place.
  - **`func_L23_00440DF0`** (`0x00440DF0`, level program 23) in
    `going-decompiled/levels/usa/L23/src/func_L23_00440DF0.c`, from
    `src/levels/23_damosel_orbit/L23_0041E418.c`
    (`func_L23_0043F930`, author commit `264cb2be`, llesieur99), task
    #2568. VERBATIM but for names: our name and 2 callees renamed to the v2.00 disc's own targets.
  - **`func_L02_003D2BF0`** (`0x003D2BF0`, level program 02, Maktar Nebula:
    starts a close-range attack on a near target in front) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D2BF0.c`, from
    `src/levels/02_maktar_nebula/L02_002ED7B0.c` (`func_L02_003D1698`, v1.01
    address, author commit `24d977dd`, bigmak94), task #2579. ADAPTED: our
    name at the v2.00 place; one callee to its v2.00 split name and six to
    L02's copies of boot code, each the v2.00 disc's own target; built at
    -G8 against upstream's -G0; `common.h` dropped (its `u8` typedef copied)
    and unused declarations not carried; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L02_003D3190`** (`0x003D3190`, level program 02, Maktar Nebula:
    the hit window of a swing between two joints) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D3190.c`, from
    `src/levels/02_maktar_nebula/L02_002ED7B0.c` (`func_L02_003D1C38`, v1.01
    address, author commit `455cde53`, Flavius), task #2579. ADAPTED: our
    name at the v2.00 place; seven callees to L02's copies of boot code, the
    v2.00 disc's own targets; built at -G8 against upstream's -G0;
    `common.h` and unused declarations dropped; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L02_003D3428`** (`0x003D3428`, level program 02, Maktar Nebula:
    lets go of the held object with a burst effect) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D3428.c`, from
    `src/levels/02_maktar_nebula/L02_002ED7B0.c` (`func_L02_003D1ED0`, v1.01
    address, author commit `ac1ee013`, bigmak94), task #2579. ADAPTED: our
    name at the v2.00 place; four callees to L02's copies of boot code;
    `sda` -> `section(".sdata")` on the `D_L02_001A8F20` extern declaration
    (RULING #8620); built at -G8 against upstream's -G0; `common.h` and
    unused declarations dropped; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L02_003D8E68`** (`0x003D8E68`, level program 02, Maktar Nebula:
    places three spawned actors and starts their behaviours) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D8E68.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0.c` (`func_L02_003D7900`, v1.01
    address, author commit `9980854e`, Flavius), task #2579. ADAPTED: our
    name at the v2.00 place; the boot's `g_soundBankHandlesBlk` for
    `D_00189E20`; four callees to their v2.00 split names and four to L02's
    copies of boot code; built at -G8 against upstream's -G0; `common.h` and
    unused declarations dropped; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L02_003FE5A0`** (`0x003FE5A0`, level program 02, Maktar Nebula:
    is joint 0 far from the hero) in
    `going-decompiled/levels/usa/L02/src/func_L02_003FE5A0.c`, from
    `src/levels/02_maktar_nebula/L02_003F5430.c` (`func_L02_003FD000`, v1.01
    address, author commit `773eafc2`, llesieur99), task #2579. ADAPTED: our
    name at the v2.00 place; the boot's `g_heroPos` for `D_00189EA0`; two
    callees to L02's copies of boot code; built at -G8 against upstream's
    -G0; `common.h` and unused declarations dropped; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L02_0040C840`** (`0x0040C840`, level program 02, Maktar Nebula:
    per-frame update of an enemy) in
    `going-decompiled/levels/usa/L02/src/func_L02_0040C840.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c` (`func_L02_0040B0F8`, v1.01
    address, author commit `11c21414`, bigmak94), task #2579. ADAPTED: our
    name at the v2.00 place; one callee to its v2.00 split name and fourteen
    to L02's copies of boot code; `D_L02_001BA500` kept as an address-named
    extern (RULING #13291 section 2); built at -G8 against upstream's -G0;
    `common.h` and unused declarations dropped; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L02_00446A48`** (`0x00446A48`, level program 02, Maktar Nebula:
    a collectible with a 3-bit per-planet counter) in
    `going-decompiled/levels/usa/L02/src/func_L02_00446A48.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c` (`func_L02_00445200`, v1.01
    address, author commit `ce9897f7`, bigmak94), task #2579. ADAPTED: our
    name at the v2.00 place; the boot's `g_playerProgress` for `D_001A79F0`
    (the disc-implied 0x001A79F8); `D_0019B4A8` as the L02 split's existing
    `D_19B4A8`; `D_L02_0021F4F0` as the address-named `D_L02_0021F520`
    (RULING #13291 section 2); seven callees to L02's copies of boot code;
    `sda` -> `section(".sdata")` on two extern declarations (RULING #8620);
    built at -G8 against upstream's -G0; `common.h`, unused declarations and
    one duplicate declaration dropped; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L02_003D62D0`** (`0x003D62D0`, level program 02, Maktar Nebula:
    has the actor waited long enough to act) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D62D0.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0-g8.c` (`func_L02_003D4D68`,
    v1.01 address, author commit `773eafc2`, llesieur99), task #2579.
    BYTE-IDENTICAL, verbatim but for names: our name at the v2.00 place; one
    level data name to our label at the v2.00 address; two callees to L02's
    copies of boot code; upstream's inert `sda` kept; `common.h` and unused
    declarations dropped; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L02_003D7F58`** (`0x003D7F58`, level program 02, Maktar Nebula:
    resets three small-data globals) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D7F58.c`, from
    `src/levels/02_maktar_nebula/L02_003D26B0-g8.c` (`func_L02_003D69F0`,
    v1.01 address, author commit `431ac503`, llesieur99), task #2579.
    BYTE-IDENTICAL, verbatim but for names: our name at the v2.00 place;
    three level data names to our labels at the v2.00 addresses; one callee
    to L02's copy of boot code; upstream's inert `sda` kept; `common.h` and
    unused declarations dropped; assembled through `tools/ee/asm_unit.sh`.
  - **`func_L02_003D5870`** (`0x003D5870`, level program 02, Maktar Nebula:
    per-frame update of an enemy with damage bookkeeping) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D5870.c`, from
    `src/levels/02_maktar_nebula/L02_003D4308-g8.c` (`func_L02_003D4308`,
    v1.01 address, author commit `13442ffb`, bigbeno37), task #2579.
    BYTE-IDENTICAL, verbatim but for names: our name at the v2.00 place; one
    level data name to our label; `D_L02_001FF6B8` and `D_L02_001BA500` kept
    as address-named externs (RULING #13291 section 2); three callees to
    their v2.00 split names and seven to L02's copies of boot code;
    upstream's inert `sda` kept; `common.h` dropped; assembled through
    `tools/ee/asm_unit.sh`.
  - **`func_L02_003FEE58`** (`0x003FEE58`, level program 02, Maktar Nebula:
    input handler of the level's selection menu) in
    `going-decompiled/levels/usa/L02/src/func_L02_003FEE58.c`, from
    `src/levels/02_maktar_nebula/L02_003FD8B8-g8.c` (`func_L02_003FD8B8`,
    v1.01 address, author commit `9bbea36d`, Flavius), task #2579.
    BYTE-IDENTICAL, verbatim but for names: our name at the v2.00 place; the
    boot's `g_itemEquippedSlot`/`g_heroState` for `D_00139568`/`D_0018C0B4`;
    `D_00138180`/`D_001395B8` as the L02 split's existing
    `D_138180`/`D_1395B8` (FACT #13369); one level data name to our label,
    `D_L02_002004A4` kept address-named; three callees to their v2.00 split
    names and twelve to L02's copies of boot and L00 code; upstream's inert
    `sda` kept; `common.h` dropped (its `u8` typedef copied); assembled
    through `tools/ee/asm_unit.sh`.
  - **`func_L11_003F3228`** (`0x003F3228`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F3228.c`, from
    `src/levels/11_joba/L11_003D6270.c`
    (`func_L11_003F1D18`, author commit `91e04732`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_003F32C0`** (`0x003F32C0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F32C0.c`, from
    `src/levels/11_joba/L11_003D6270-g8.c`
    (`func_L11_003F1DB0`, author commit `bcafb24e`, Gianni), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets; `D_L11_001FFA78` as the address-named extern `D_L11_001FFAF8`, RULING #13291 §2.
  - **`func_L11_003F4228`** (`0x003F4228`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F4228.c`, from
    `src/levels/11_joba/L11_003D6270.c`
    (`func_L11_003F2D18`, author commit `91e04732`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callee respelled to the v2.00 level split name at its disc-implied target.
  - **`func_L11_003F4248`** (`0x003F4248`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F4248.c`, from
    `src/levels/11_joba/L11_003D6270-g8.c`
    (`func_L11_003F2D38`, author commit `91e04732`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees and the two .lit thresholds respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_003F42D8`** (`0x003F42D8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F42D8.c`, from
    `src/levels/11_joba/L11_003D6270.c`
    (`func_L11_003F2DC8`, author commit `91e04732`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callee respelled to the v2.00 level split name at its disc-implied target.
  - **`func_L11_003F4410`** (`0x003F4410`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F4410.c`, from
    `src/levels/11_joba/L11_003D6270.c`
    (`func_L11_003F2F00`, author commit `91e04732`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; no other symbol appears.
  - **`func_L11_003F45A8`** (`0x003F45A8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F45A8.c`, from
    `src/levels/11_joba/L11_003D6270.c`
    (`func_L11_003F3098`, author commit `91e04732`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_003F5570`** (`0x003F5570`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_003F5570.c`, from
    `src/levels/11_joba/L11_003F4060-g8.c`
    (`func_L11_003F4060`, author commit `b4b0fe9b`, bigbeno37), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names and the .lit length to the v2.00 split dlabel at their disc-implied targets.
  - **`func_L11_00422438`** (`0x00422438`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00422438.c`, from
    `src/levels/11_joba/L11_003D6270.c`
    (`func_L11_00420C38`, author commit `91e04732`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callee respelled to the v2.00 level split name at its disc-implied target.
  - **`func_L11_00427268`** (`0x00427268`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00427268.c`, from
    `src/levels/11_joba/L11_00420C88.c`
    (`func_L11_00425A50`, author commit `ee3fee45`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_004276D8`** (`0x004276D8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004276D8.c`, from
    `src/levels/11_joba/L11_00420C88.c`
    (`func_L11_00425EC0`, author commit `ee3fee45`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callee respelled to the v2.00 level split name at its disc-implied target.
  - **`func_L11_0042BC50`** (`0x0042BC50`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0042BC50.c`, from
    `src/levels/11_joba/L11_00425F10.c`
    (`func_L11_0042A420`, author commit `ee3fee45`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_004307C0`** (`0x004307C0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004307C0.c`, from
    `src/levels/11_joba/L11_00425F10.c`
    (`func_L11_0042EF90`, author commit `ee3fee45`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callee respelled to the v2.00 level split name at its disc-implied target.
  - **`func_L11_00433590`** (`0x00433590`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00433590.c`, from
    `src/levels/11_joba/L11_00425F10.c`
    (`func_L11_00431D50`, author commit `ee3fee45`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_004335F0`** (`0x004335F0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004335F0.c`, from
    `src/levels/11_joba/L11_00425F10-g8.c`
    (`func_L11_00431DB0`, author commit `ee3fee45`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; both table symbols as address-named externs at their disc-implied targets, RULING #13291 §2.
  - **`func_L11_00433670`** (`0x00433670`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00433670.c`, from
    `src/levels/11_joba/L11_00425F10.c`
    (`func_L11_00431E30`, author commit `ee3fee45`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_004362E8`** (`0x004362E8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004362E8.c`, from
    `src/levels/11_joba/L11_00432628.c`
    (`func_L11_00434AA8`, author commit `431ac503`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_00437FA8`** (`0x00437FA8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00437FA8.c`, from
    `src/levels/11_joba/L11_00432628.c`
    (`func_L11_00436768`, author commit `b4b0fe9b`, bigbeno37; the text is Gianni's nonmatching draft from `77639a79`, moved verbatim), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_00438090`** (`0x00438090`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00438090.c`, from
    `src/levels/11_joba/L11_00432628.c`
    (`func_L11_00436850`, author commit `5964c620`, Flavius), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets; their `func_002B3BA8_6850` second-prototype spelling is the plain `func_L11_0033DAC8` with their prototype.
  - **`func_L11_00438150`** (`0x00438150`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00438150.c`, from
    `src/levels/11_joba/L11_00432628.c`
    (`func_L11_00436910`, author commit `5acf9869`, Flavius), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_004382B8`** (`0x004382B8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004382B8.c`, from
    `src/levels/11_joba/L11_00432628.c`
    (`func_L11_00436A78`, author commit `431ac503`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_00439B60`** (`0x00439B60`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00439B60.c`, from
    `src/levels/11_joba/L11_00432628.c`
    (`func_L11_00438320`, author commit `431ac503`, llesieur99; body changed in `6b943916`, Louis-Philippe Le Sieur, the same account), task #2554.
    Verbatim but for names: our name at the v2.00 place; the callees respelled to the v2.00 level split names at their disc-implied targets.
  - **`func_L11_0043A518`** (`0x0043A518`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0043A518.c`, from
    `src/levels/11_joba/L11_00432628.c`
    (`func_L11_00438CD8`, author commit `fa8d6672`, llesieur99), task #2554.
    Verbatim but for names: our name at the v2.00 place; no other symbol appears.
  - **`func_L20_002DC618`** (`0x002DC618`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_002DC618.c`, from
    `src/levels/20_yeedil/L20_002D6488.c`
    (`func_L20_002DC390`, author commit `ee3fee45`, llesieur99), task
    #2565. VERBATIM but for names: our name, the boot's `g_bPlayerMode` and 1 callee renamed to the v2.00 disc's own targets.
  - **`func_L20_00331CD8`** (`0x00331CD8`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00331CD8.c`, from
    `src/levels/20_yeedil/L20_002DEBE0.c`
    (`func_L20_00330A68`, author commit `71f1dc28`, ReaperOfVeriod), task
    #2565. VERBATIM but for names: our name, 2 data names and 2 callees renamed to the v2.00 disc's own targets.
  - **`func_L20_0036BFB0`** (`0x0036BFB0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0036BFB0.c`, from
    `src/levels/20_yeedil/L20_0036AE68-g8.c`
    (`func_L20_0036AE68`, author commit `7c3ea917`, Fromnowhere), task
    #2565. VERBATIM but for names: our name, the boot's `g_anAvailableLevelOrder` and 4 data names renamed to the v2.00 disc's own targets.
  - **`func_L20_0036F938`** (`0x0036F938`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0036F938.c`, from
    `src/levels/20_yeedil/L20_002DEBE0.c`
    (`func_L20_0036E720`, author commit `71f1dc28`, ReaperOfVeriod), task
    #2565. ADAPTED: our name, 2 data names and 2 callees renamed to the v2.00 disc's own targets; their `sda` as `section(".sdata")` (RULING #8620).
  - **`func_L20_0039BE60`** (`0x0039BE60`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0039BE60.c`, from
    `src/levels/20_yeedil/L20_002DEBE0.c`
    (`func_L20_0039AA98`, author commit `5acf9869`, Flavius), task
    #2565. ADAPTED: our name, 1 data name and 7 callees renamed to the v2.00 disc's own targets; their `sda` as `section(".sdata")` (RULING #8620).
  - **`func_L20_0039DC90`** (`0x0039DC90`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0039DC90.c`, from
    `src/levels/20_yeedil/L20_0039C8C8.c`
    (`func_L20_0039C8C8`, author commit `8821a1ae`, Flavius), task
    #2565. VERBATIM but for names: our name, the boot's `g_heroFacingDir`, 1 data name and 11 callees renamed to the v2.00 disc's own targets.
  - **`func_L20_0039EB78`** (`0x0039EB78`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0039EB78.c`, from
    `src/levels/20_yeedil/L20_0039C448.c`
    (`func_L20_0039D7B0`, author commit `7f907424`, ReaperOfVeriod), task
    #2565. ADAPTED: our name, 1 data name and 5 callees renamed to the v2.00 disc's own targets; their `sda` as `section(".sdata")` (RULING #8620).
  - **`func_L20_0039EF18`** (`0x0039EF18`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0039EF18.c`, from
    `src/levels/20_yeedil/L20_0039C448.c`
    (`func_L20_0039DB50`, author commit `fa8d6672`, llesieur99), task
    #2565. VERBATIM but for names: our name and 1 callee renamed to the v2.00 disc's own targets.
  - **`func_L20_0039F250`** (`0x0039F250`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0039F250.c`, from
    `src/levels/20_yeedil/L20_0039C448.c`
    (`func_L20_0039DE88`, author commit `431ac503`, llesieur99), task
    #2565. VERBATIM but for names: our name and 2 callees renamed to the v2.00 disc's own targets.
  - **`func_L20_0039F2E0`** (`0x0039F2E0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0039F2E0.c`, from
    `src/levels/20_yeedil/L20_0039C448.c`
    (`func_L20_0039DF18`, author commit `9826f942`, ReaperOfVeriod), task
    #2565. VERBATIM but for names: our name and 9 callees renamed to the v2.00 disc's own targets.
  - **`func_L20_0039F5B0`** (`0x0039F5B0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_0039F5B0.c`, from
    `src/levels/20_yeedil/L20_0039C448.c`
    (`func_L20_0039E1E8`, author commit `1da2d1f3`, Fromnowhere), task
    #2565. VERBATIM but for names: our name, the boot's `g_soundBankHandlesBlk`, 1 data name and 9 callees renamed to the v2.00 disc's own targets.
  - **`func_L20_003A05F8`** (`0x003A05F8`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_003A05F8.c`, from
    `src/levels/20_yeedil/L20_0039C448.c`
    (`func_L20_0039F230`, author commit `fa8d6672`, llesieur99), task
    #2565. VERBATIM but for names: our name and 3 callees renamed to the v2.00 disc's own targets.
  - **`func_L20_003C12C0`** (`0x003C12C0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_003C12C0.c`, from
    `src/levels/20_yeedil/L20_0039C448-g8.c`
    (`func_L20_003BFEB8`, author commit `8422a726`, Fromnowhere), task
    #2565. VERBATIM but for names: our name, the boot's `g_soundBankHandlesBlk`, `g_playerProgress`, 4 data names (one the boot's own `D_19B2E8`) and 4 callees renamed to the v2.00 disc's own targets.
  - **`func_L20_003C16D8`** (`0x003C16D8`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_003C16D8.c`, from
    `src/levels/20_yeedil/L20_0039C448.c`
    (`func_L20_003C02D0`, author commit `7f907424`, ReaperOfVeriod), task
    #2565. ADAPTED: our name and 3 data names renamed to the v2.00 disc's own targets; their `sda` as `section(".sdata")` (RULING #8620).
  - **`func_L20_003C1758`** (`0x003C1758`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_003C1758.c`, from
    `src/levels/20_yeedil/L20_0039C448.c`
    (`func_L20_003C0350`, author commit `7f907424`, ReaperOfVeriod), task
    #2565. ADAPTED: our name, 3 data names and 2 callees renamed to the v2.00 disc's own targets; their `sda` as `section(".sdata")` (RULING #8620).
  - **`func_L20_00436710`** (`0x00436710`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00436710.c`, from
    `src/levels/20_yeedil/L20_0039C448.c`
    (`func_L20_00435178`, author commit `7f907424`, ReaperOfVeriod), task
    #2565. VERBATIM but for names: our name, 1 data name and 2 callees renamed to the v2.00 disc's own targets.
  - **`func_L20_00440AA8`** (`0x00440AA8`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00440AA8.c`, from
    `src/levels/20_yeedil/L20_0043ECB0-g8.c`
    (`func_L20_0043F510`, author commit `adcbd51e`, llesieur99), task
    #2565. VERBATIM but for names: our name, 1 data name and 2 callees renamed to the v2.00 disc's own targets.
  - **`func_L20_004418D8`** (`0x004418D8`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_004418D8.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00440398`, author commit `1da2d1f3`, Fromnowhere), task
    #2565. ADAPTED: our name and 3 data names renamed to the v2.00 disc's own targets; their `sda` as `section(".sdata")` (RULING #8620).
  - **`func_L20_004443B0`** (`0x004443B0`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_004443B0.c`, from
    `src/levels/20_yeedil/L20_00442E68-g8.c`
    (`func_L20_00442E68`, author commit `7b31e91f`, Fromnowhere), task
    #2565. ADAPTED: our name, 3 data names and 1 callee renamed to the v2.00 disc's own targets; their `sda` as `section(".sdata")` (RULING #8620).
  - **`func_L20_00444870`** (`0x00444870`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00444870.c`, from
    `src/levels/20_yeedil/L20_0043ECB0.c`
    (`func_L20_00443328`, author commit `765097bb`, ReaperOfVeriod), task
    #2565. ADAPTED: our name, 1 data name and 3 callees renamed to the v2.00 disc's own targets; their `sda` as `section(".sdata")` (RULING #8620).
  - **`func_L20_00448B88`** (`0x00448B88`, level program 20) in
    `going-decompiled/levels/usa/L20/src/func_L20_00448B88.c`, from
    `src/levels/20_yeedil/L20_00447640-g8.c`
    (`func_L20_00447640`, author commit `8422a726`, Fromnowhere), task
    #2565. ADAPTED: our name, 3 data names and 2 callees renamed to the v2.00 disc's own targets; their `sda` as `section(".sdata")` (RULING #8620).
  - **`func_L11_0044A708`** (`0x0044A708`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044A708.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_00448CE0`, author commit `fa8d6672`, llesieur99),
    task #2556. VERBATIM but for names: our name at the v2.00 place; no other symbol referenced (0 relocations).
  - **`func_L11_0044B0C8`** (`0x0044B0C8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044B0C8.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_004496A0`, author commit `5acf9869`, Flavius),
    task #2556. VERBATIM but for names: our name at the v2.00 place; nine names respelled: eight callees plus the sibling func_L11_0044ACC8 (defined in the same upstream TU, declared here as an extern).
  - **`func_L11_0044B2A0`** (`0x0044B2A0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044B2A0.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_00449878`, author commit `fa8d6672`, llesieur99; two callee spellings later changed in `1d296136`, Louis-Philippe Le Sieur),
    task #2556. VERBATIM but for names: our name at the v2.00 place; seven callees as L11’s v2.00 names; the file-scope `TI128` typedef written in the TU.
  - **`func_L11_0044B398`** (`0x0044B398`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044B398.c`, from
    `src/us2/levels/11_joba/L11_0044B398.c` (`func_US2_L11_0044B398`, author commit `ac7e1a2e`, bigbeno37, which carried the text from `func_L11_00449970` in `src/levels/11_joba/L11_004465F8.c`, renaming it and adding one line; that v1.01 body was written in `7119fabe`, Gianni, with seven callee spellings changed in `1d296136`, Louis-Philippe Le Sieur),
    task #2556. VERBATIM but for names: our name at the v2.00 place; nine names respelled (callees plus func_L11_0044B2A0 of row L11-D); the file-scope `TI128` typedef written in the TU; the body’s `(char *)(long)` kept verbatim (EE `long` is 64-bit).
  - **`func_L11_0044DB28`** (`0x0044DB28`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044DB28.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_0044C0F0`, author commit `fa8d6672`, llesieur99),
    task #2556. VERBATIM but for names: our name at the v2.00 place; two callees as L11’s v2.00 names.
  - **`func_L11_0044DCA0`** (`0x0044DCA0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044DCA0.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_0044C268`, author commit `a2d495f1`, Flavius),
    task #2556. VERBATIM but for names: our name at the v2.00 place; two callees as L11’s v2.00 names.
  - **`func_L11_0044DD60`** (`0x0044DD60`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044DD60.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_0044C328`, author commit `5acf9869`, Flavius),
    task #2556. VERBATIM but for names: our name at the v2.00 place; seven callees as L11’s v2.00 names.
  - **`func_L11_0044DF28`** (`0x0044DF28`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044DF28.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_0044C4F0`, author commit `5acf9869`, Flavius),
    task #2556. VERBATIM but for names: our name at the v2.00 place; nine callees as L11’s v2.00 names.
  - **`func_L11_0044E250`** (`0x0044E250`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0044E250.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_0044C818`, author commit `431ac503`, llesieur99),
    task #2556. VERBATIM but for names: our name at the v2.00 place; one callee as L11’s v2.00 name.
  - **`func_L11_00452180`** (`0x00452180`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00452180.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_00450748`, author commit `7fccf735`, Flavius; upstream's root commit, which squashed the earlier RAC2 history),
    task #2556. VERBATIM but for names: our name at the v2.00 place; the vector helper as L11’s func_L11_00301280.
  - **`func_L11_00454268`** (`0x00454268`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00454268.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_00452810`, author commit `fa8d6672`, llesieur99),
    task #2556. VERBATIM but for names: our name at the v2.00 place; two callees as L11’s v2.00 names.
  - **`func_L11_004542F8`** (`0x004542F8`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004542F8.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_004528A0`, author commit `fa8d6672`, llesieur99),
    task #2556. VERBATIM but for names: our name at the v2.00 place; the shared helper as L11’s func_L11_00454430.
  - **`func_L11_004546B0`** (`0x004546B0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004546B0.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_00452C58`, author commit `5acf9869`, Flavius),
    task #2556. VERBATIM but for names: our name at the v2.00 place; seven callees as L11’s v2.00 names.
  - **`func_L11_00454850`** (`0x00454850`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00454850.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_00452DF8`, author commit `5acf9869`, Flavius),
    task #2556. VERBATIM but for names: our name at the v2.00 place; eight callees as L11’s v2.00 names.
  - **`func_L11_00454B18`** (`0x00454B18`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00454B18.c`, from
    `src/levels/11_joba/L11_004465F8.c` (`func_L11_004530C0`, author commit `fa8d6672`, llesieur99),
    task #2556. VERBATIM but for names: our name at the v2.00 place; one callee as L11’s v2.00 name.
  - **`func_L11_0045B958`** (`0x0045B958`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0045B958.c`, from
    `src/levels/11_joba/L11_00453358.c` (`func_L11_00459EA0`, author commit `7fccf735`, Flavius; upstream's root commit, which squashed the earlier RAC2 history),
    task #2556. VERBATIM but for names: our name at the v2.00 place; no other symbol referenced (0 relocations); the body’s `long long` stores kept verbatim (sdk29 arm).
  - **`func_L11_00471C58`** (`0x00471C58`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_00471C58.c`, from
    `src/levels/11_joba/L11_004618C0.c` (`func_L11_004701A0`, author commit `daf0e4a1`, Flavius),
    task #2556. VERBATIM but for names: our name at the v2.00 place; three callees as L11’s v2.00 names.
  - **`func_L11_004748A0`** (`0x004748A0`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_004748A0.c`, from
    `src/levels/11_joba/L11_004618C0.c` (`func_L11_00472DE8`, author commit `ee3fee45`, llesieur99),
    task #2556. VERBATIM but for names: our name at the v2.00 place; no other symbol referenced (0 relocations).
  - **`func_L11_0048A068`** (`0x0048A068`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0048A068.c`, from
    `src/levels/11_joba/L11_004618C0.c` (`func_L11_004885A0`, author commit `76651091`, Flavius),
    task #2556. VERBATIM but for names: our name at the v2.00 place; the datum as the address-named extern `D_L11_001BA990` (g_blobShadowQueue+0x3D0, a base+off target: RULING #13291 section 2).
  - **`func_L11_0048A080`** (`0x0048A080`, level program 11) in
    `going-decompiled/levels/usa/L11/src/func_L11_0048A080.c`, from
    `src/levels/11_joba/L11_004618C0.c` (`func_L11_004885B8`, author commit `ee3fee45`, llesieur99),
    task #2556. VERBATIM but for names: our name at the v2.00 place; three callees as L11’s v2.00 names.
  - **`func_L02_003D39E0`** (`0x003D39E0`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003D39E0.c`, from
    `src/us2/levels/02_maktar_nebula/L02_003D39E0.c`
    (`func_US2_L02_003D39E0`, author commit `ac7e1a2e`, bigbeno37), task #2560.
    Verbatim but for names.
  - **`func_L02_003FC5A0`** (`0x003FC5A0`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_003FC5A0.c`, from
    `src/levels/02_maktar_nebula/L02_003FB000-g8.c`
    (`func_L02_003FB000`, author commit `b887d976`, Louis-Philippe Le Sieur), task #2560.
    Verbatim but for names.
  - **`func_L02_0040CB68`** (`0x0040CB68`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0040CB68.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_0040B420`, author commit `91e04732`, llesieur99), task #2560.
    Verbatim but for names.
  - **`func_L02_0040CBF8`** (`0x0040CBF8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0040CBF8.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_0040B4B0`, author commit `91e04732`, llesieur99), task #2560.
    Verbatim but for names.
  - **`func_L02_0040CFB0`** (`0x0040CFB0`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0040CFB0.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_0040B868`, author commit `91e04732`, llesieur99), task #2560.
    Verbatim but for names.
  - **`func_L02_0040D0C8`** (`0x0040D0C8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0040D0C8.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_0040B980`, author commit `e80a585a`, Flavius), task #2560.
    Verbatim but for names.
  - **`func_L02_0040D2E8`** (`0x0040D2E8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0040D2E8.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_0040BBA0`, author commit `ee3fee45`, llesieur99), task #2560.
    Verbatim but for names.
  - **`func_L02_0040E630`** (`0x0040E630`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0040E630.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_0040CEE8`, author commit `91e04732`, llesieur99), task #2560.
    Verbatim but for names.
  - **`func_L02_0040E6B0`** (`0x0040E6B0`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0040E6B0.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_0040CF68`, author commit `7a7816b9`, bigmak94), task #2560.
    Verbatim but for names.
  - **`func_L02_0040E7C8`** (`0x0040E7C8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0040E7C8.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_0040D080`, author commit `91e04732`, llesieur99), task #2560.
    Verbatim but for names.
  - **`func_L02_0040E8C8`** (`0x0040E8C8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0040E8C8.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_0040D180`, author commit `24d977dd`, bigmak94), task #2560.
    Verbatim but for names.
  - **`func_L02_00413F50`** (`0x00413F50`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_00413F50.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_00412808`, author commit `ce9897f7`, bigmak94), task #2560.
    Verbatim but for names.
  - **`func_L02_0043A360`** (`0x0043A360`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0043A360.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_00438B28`, author commit `91e04732`, llesieur99), task #2560.
    Verbatim but for names; `D_L02_001BE200` kept address-named, RULING #13291 §2.
  - **`func_L02_0043F998`** (`0x0043F998`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_0043F998.c`, from
    `src/levels/02_maktar_nebula/L02_0043E150.c`
    (`func_L02_0043E150`, author commit `b8fd9b4c`, Flavius, which moved bigmak94's nonmatching draft from `20ad739c` (validator #2622, RULING #13427)), task #2560.
    Verbatim but for names.
  - **`func_L02_00457A90`** (`0x00457A90`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_00457A90.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_00456258`, author commit `76651091`, Flavius), task #2560.
    Verbatim but for names; upstream credits rac1-decomp `src/core/00118A40.c` `func_00119100`.
  - **`func_L02_004582F8`** (`0x004582F8`, level program 02) in
    `going-decompiled/levels/usa/L02/src/func_L02_004582F8.c`, from
    `src/levels/02_maktar_nebula/L02_0040A4A8.c`
    (`func_L02_00456AC0`, author commit `91e04732`, llesieur99), task #2560.
    Verbatim but for names.
  - **`func_L09_003F17E8`** (`0x003F17E8`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_003F17E8.c`, from
    `src/levels/09_dobbo/L09_003F0378-rc2.c` (`func_L09_003F0378`, author commit `fe9859d7`, Sergio Rodmauser),
    task #2562. VERBATIM but for names: our name at the v2.00 place; nine names respelled: upstream's ref_ cross-reference callees to our split names at the v2.00 places.
  - **`func_L09_003FFED8`** (`0x003FFED8`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_003FFED8.c`, from
    `src/levels/09_dobbo/L09_003FEA48-g8.c` (`func_L09_003FEA48`, author commit `09484b1f`, Louis-Philippe Le Sieur),
    task #2562. VERBATIM but for names: our name at the v2.00 place; the boot's g_playerProgress; the record table as the address-named extern D_L09_001B27DC (D_001B2354+0x488, a base+off target: RULING #13291 section 2); three callees at their v2.00 places.
  - **`func_L09_00404668`** (`0x00404668`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_00404668.c`, from
    `src/levels/09_dobbo/L09_003FEA48-g8.c` (`func_L09_004031D8`, author commit `431ac503`, llesieur99),
    task #2562. VERBATIM but for names: our name at the v2.00 place; the frame-counter union as D_L09_001B1370 (our label); func_2EA110 as L09's copy func_L09_002EAD98.
  - **`func_L09_00404808`** (`0x00404808`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_00404808.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c` (`func_L09_00403378`, author commit `264cb2be`, llesieur99),
    task #2562. VERBATIM but for names: our name at the v2.00 place; no other symbol referenced (0 relocations).
  - **`func_L09_00404B90`** (`0x00404B90`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_00404B90.c`, from
    `src/levels/09_dobbo/L09_00403700.c` (`func_L09_00403700`, author commit `3a4945e2`, Flavius, which finished the near-miss draft `nonmatching/levels/func_L09_00403700.c` by `96fa46bb`, Louis-Philippe Le Sieur, and `1d0b1d4c`, ReaperOfVeriod; 5 of the 9 definition lines are the draft's, RULING #13427),
    task #2562. VERBATIM but for names: our name at the v2.00 place; one callee as L09's copy func_L09_003195F0.
  - **`func_L09_00406420`** (`0x00406420`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_00406420.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c` (`func_L09_00404F90`, author commit `264cb2be`, llesieur99; later body changes `a541245e` and `3638916f`, Louis-Philippe Le Sieur),
    task #2562. VERBATIM but for names: our name at the v2.00 place; two callees as L09's copies at their v2.00 places.
  - **`func_L09_00406458`** (`0x00406458`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_00406458.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c` (`func_L09_00404FC8`, author commit `264cb2be`, llesieur99),
    task #2562. VERBATIM but for names: our name at the v2.00 place; three callees as L09's copies at their v2.00 places.
  - **`func_L09_00406570`** (`0x00406570`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_00406570.c`, from
    `src/levels/09_dobbo/L09_004050E0.c` (`func_L09_004050E0`, author commit `3dc9b1fa`, Louis-Philippe Le Sieur),
    task #2562. VERBATIM but for names: our name at the v2.00 place; the boot's g_heroPos; nine callees as L09's copies at their v2.00 places; include_asm.h dropped (unused).
  - **`func_L09_00408088`** (`0x00408088`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_00408088.c`, from
    `src/levels/09_dobbo/L09_00406BF8-rc2.c` (`func_L09_00406BF8`, author commit `fe9859d7`, Sergio Rodmauser),
    task #2562. VERBATIM but for names: our name at the v2.00 place; the boot's g_heroPos; two names respelled: upstream's ref_ cross-reference callees to our split names.
  - **`func_L09_004080F8`** (`0x004080F8`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_004080F8.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c` (`func_L09_00406C68`, author commit `264cb2be`, llesieur99),
    task #2562. VERBATIM but for names: our name at the v2.00 place; the callee as L09's copy func_L09_00318D10 (upstream named it after level 08).
  - **`func_L09_004094E8`** (`0x004094E8`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_004094E8.c`, from
    `src/levels/09_dobbo/L09_00408058-rc2.c` (`func_L09_00408058`, author commit `fe9859d7`, Sergio Rodmauser),
    task #2562. VERBATIM but for names: our name at the v2.00 place; no other symbol referenced (0 relocations).
  - **`func_L09_004095C8`** (`0x004095C8`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_004095C8.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c` (`func_L09_00408138`, author commit `ee3fee45`, llesieur99),
    task #2562. VERBATIM but for names: our name at the v2.00 place; the shared body as func_L09_004094E8 (adopted in this row).
  - **`func_L09_004096E0`** (`0x004096E0`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_004096E0.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c` (`func_L09_00408250`, author commit `264cb2be`, llesieur99),
    task #2562. VERBATIM but for names: our name at the v2.00 place; the group id-list table as the address-named extern D_L09_001FB4E0 (g_pTextTableLoadBuf+0x8C08, a base+off target: RULING #13291 section 2); one callee at its v2.00 place.
  - **`func_L09_0040A6B8`** (`0x0040A6B8`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_0040A6B8.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c` (`func_L09_004091C8`, author commit `913f6380`, bigbeno37),
    task #2562. VERBATIM but for names: our name at the v2.00 place; four callees as L09's copies at their v2.00 places.
  - **`func_L09_0040C4F8`** (`0x0040C4F8`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_0040C4F8.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c` (`func_L09_0040B008`, author commit `431ac503`, llesieur99),
    task #2562. VERBATIM but for names: our name at the v2.00 place; the scale datum as the address-named extern D_L09_001B9BB0 (g_uiTextureCache+0x4F0, a base+off target: RULING #13291 section 2); three callees as L09's copies.
  - **`func_L09_00423260`** (`0x00423260`, level program 9) in
    `going-decompiled/levels/usa/L09/src/func_L09_00423260.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c` (`func_L09_00421D60`, author commit `27b929ce`, Lynder063),
    task #2562. VERBATIM but for names: our name at the v2.00 place; four callees as L09's copies at their v2.00 places.
  - **`func_L08_0032B038`** (`0x0032B038`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_0032B038.c`, from
    `src/levels/08_tabora/L08_002D90C8.c`
    (`func_L08_00329EC0`, author commit `b31a91e7`, MantraAU), task #2627.
    Verbatim but for names: our name at the v2.00 place; no other symbol appears.
  - **`func_L08_0032FBD8`** (`0x0032FBD8`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_0032FBD8.c`, from
    `src/levels/08_tabora/L08_002D90C8.c`
    (`func_L08_0032EA60`, author commit `ee3fee45`, llesieur99), task #2627.
    Verbatim but for names: our name at the v2.00 place; callees func_2F9E60, func_2F9DF0 and func_2F9FD0 as L08's copies func_L08_002FAAB0, func_L08_002FAA40 and func_L08_002FAC20.
  - **`func_L08_00330050`** (`0x00330050`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_00330050.c`, from
    `src/levels/08_tabora/L08_002D90C8.c`
    (`func_L08_0032EED8`, author commit `b31a91e7`, MantraAU), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_001FE7C0` kept address-named inside the split label `D_L08_001F0000` (RULING #13291 §2); func_00282E80 as L08's copy func_L08_002FAC20, the v2.00 disc's own target for the call.
  - **`func_L08_003300F8`** (`0x003300F8`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003300F8.c`, from
    `src/levels/08_tabora/L08_002D90C8.c`
    (`func_L08_0032EF80`, author commit `b31a91e7`, MantraAU), task #2627.
    Adapted (byte-identical): our name at the v2.00 place; the four callees respelled to the v2.00 level split names at their disc-implied targets, func_L08_00330050 among them (adopted in this same row); the return type written `f32` for upstream's `float`, and a `func_L08_00330050` prototype added (both byte-neutral; validator #2637).
  - **`func_L08_003CE178`** (`0x003CE178`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003CE178.c`, from
    `src/levels/08_tabora/L08_002D90C8-g8.c`
    (`func_L08_003CCD28`, author commit `886754b0`, Louis-Philippe Le Sieur), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_001B29DC` kept address-named inside the split label `D_L08_001B2200` (RULING #13291 §2), upstream's `sda` attribute kept verbatim, our stock cc1 ignores it, byte-inert; `common.h` dropped.
  - **`func_L08_003CE1A8`** (`0x003CE1A8`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003CE1A8.c`, from
    `src/levels/08_tabora/L08_002D90C8-g8.c`
    (`func_L08_003CCD58`, author commit `b31a91e7`, MantraAU), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_001B29DC` kept address-named inside `D_L08_001B2200` (RULING #13291 §2), upstream's `sda` attribute kept verbatim, byte-inert; `common.h` dropped.
  - **`func_L08_003D1440`** (`0x003D1440`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003D1440.c`, from
    `src/levels/08_tabora/L08_002D90C8.c`
    (`func_L08_003CFFE8`, author commit `b31a91e7`, MantraAU), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_002A0D00` as the split label `D_L08_002A0D80`; func_L04_003A21F8 as L08's copy func_L08_0038E260; `include_asm.h` dropped.
  - **`func_L08_003D1468`** (`0x003D1468`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003D1468.c`, from
    `src/levels/08_tabora/L08_002D90C8.c`
    (`func_L08_003D0010`, author commit `b31a91e7`, MantraAU), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_002A0D00` as the split label `D_L08_002A0D80`; func_L04_003A02A0 as L08's copy func_L08_0038C308; `include_asm.h` dropped.
  - **`func_L08_003D1498`** (`0x003D1498`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003D1498.c`, from
    `src/levels/08_tabora/L08_002D90C8.c`
    (`func_L08_003D0040`, author commit `b31a91e7`, MantraAU), task #2627.
    Verbatim but for names: our name at the v2.00 place; func_0029FCC0 as L08's copy func_L08_0031F7C8; `include_asm.h` dropped.
  - **`func_L08_003D14F0`** (`0x003D14F0`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003D14F0.c`, from
    `src/levels/08_tabora/L08_002D90C8-g8.c`
    (`func_L08_003D0098`, author commit `886754b0`, Louis-Philippe Le Sieur), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_001B29DC` kept address-named inside `D_L08_001B2200` (RULING #13291 §2), upstream's `sda` attribute kept verbatim, byte-inert; `common.h` dropped.
  - **`func_L08_003D9D70`** (`0x003D9D70`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003D9D70.c`, from
    `src/levels/08_tabora/L08_003D8348-g8.c`
    (`func_L08_003D8918`, author commit `91e04732`, llesieur99), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_001B29DC` kept address-named inside `D_L08_001B2200` (RULING #13291 §2), upstream's `sda` attribute kept verbatim, byte-inert; `include_asm.h` dropped.
  - **`func_L08_003D9DA0`** (`0x003D9DA0`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003D9DA0.c`, from
    `src/levels/08_tabora/L08_003D8348-g8.c`
    (`func_L08_003D8948`, author commit `91e04732`, llesieur99), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_001B29DC` kept address-named inside `D_L08_001B2200` (RULING #13291 §2), upstream's `sda` attribute kept verbatim, byte-inert; `include_asm.h` dropped.
  - **`func_L08_003D9DE8`** (`0x003D9DE8`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003D9DE8.c`, from
    `src/levels/08_tabora/L08_003D8348-g8.c`
    (`func_L08_003D8990`, author commit `91e04732`, llesieur99), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_001B29DC` kept address-named inside `D_L08_001B2200` (RULING #13291 §2), upstream's `sda` attribute kept verbatim, byte-inert; `include_asm.h` dropped.
  - **`func_L08_003D9E30`** (`0x003D9E30`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_003D9E30.c`, from
    `src/levels/08_tabora/L08_003D8348-g8.c`
    (`func_L08_003D89D8`, author commit `91e04732`, llesieur99), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_001B29DC` kept address-named inside `D_L08_001B2200` (RULING #13291 §2), upstream's `sda` attribute kept verbatim, byte-inert; `include_asm.h` dropped.
  - **`func_L08_00423FD0`** (`0x00423FD0`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_00423FD0.c`, from
    `src/levels/08_tabora/L08_003D8348-g8.c`
    (`func_L08_004229C8`, author commit `91e04732`, llesieur99), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_001B29DC` kept address-named inside `D_L08_001B2200` (RULING #13291 §2), upstream's `sda` attribute kept verbatim, byte-inert; `include_asm.h` dropped.
  - **`func_L08_00424000`** (`0x00424000`, level program 08) in
    `going-decompiled/levels/usa/L08/src/func_L08_00424000.c`, from
    `src/levels/08_tabora/L08_003D8348-g8.c`
    (`func_L08_004229F8`, author commit `91e04732`, llesieur99), task #2627.
    Verbatim but for names: our name at the v2.00 place; `D_L08_001B29DC` kept address-named inside `D_L08_001B2200` (RULING #13291 §2), upstream's `sda` attribute kept verbatim, byte-inert; `include_asm.h` dropped.
  - **`func_L01_003A79B0`** (`0x003A79B0`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003A79B0.c`, from
    `src/levels/01_oozla/L01_003835D8-g8.c` (`func_L01_003A65B0`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; D_L01_001AF87C as our .lit label D_L01_001AF83C; the suffix table kept address-named as D_L01_001BF930 (g_collHitEventRing+0x7B0, RULING #13291 section 2); four callees as L01's copies at their v2.00 places; the file's other declarations dropped.
  - **`func_L01_003B6698`** (`0x003B6698`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003B6698.c`, from
    `src/levels/01_oozla/L01_003835D8-g8.c` (`func_L01_003B5290`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; both tables kept address-named as D_L01_001B255C (D_001B2354+0x208) and D_L01_001FB020 (g_pTextTableLoadBuf+0x8748) (RULING #13291 section 2); the file's other declarations dropped.
  - **`func_L01_003BC3B8`** (`0x003BC3B8`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003BC3B8.c`, from
    `src/levels/01_oozla/L01_003835D8-g8.c` (`func_L01_003BAFB0`, author commit `ee3fee45`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; D_L01_001AFE18 as our .lit label D_L01_001AFDD8; the file's other declarations dropped.
  - **`func_L01_003B59E0`** (`0x003B59E0`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003B59E0.c`, from
    `src/levels/01_oozla/L01_003835D8.c` (`func_L01_003B45D8`, author commit `c9a5add7`, Flavius; one body line from `ec778f01`, Flavius),
    task #2626. VERBATIM but for names: our name at the v2.00 place; func_00115484 as the boot's memset (0x00115484, codegen-neutral on this arm); func_00277F48 as L01's copy func_L01_002D6C20; the entity table kept address-named as D_L01_001C3980 (g_pointLights+0xEC0, RULING #13291 section 2); the file's other declarations dropped. common.h's typedefs written in the TU.
  - **`func_L01_003B5AD0`** (`0x003B5AD0`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003B5AD0.c`, from
    `src/levels/01_oozla/L01_003835D8.c` (`func_L01_003B46C8`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; the body references no other symbol (0 relocations); the file's other declarations dropped.
  - **`func_L01_003B5FE0`** (`0x003B5FE0`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003B5FE0.c`, from
    `src/levels/01_oozla/L01_003835D8.c` (`func_L01_003B4BD8`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; func_00282948 as L01's copy func_L01_002E3DE8 (upstream's prototype-less declaration kept); the file's other declarations dropped.
  - **`func_L01_003B6028`** (`0x003B6028`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003B6028.c`, from
    `src/levels/01_oozla/L01_003835D8.c` (`func_L01_003B4C20`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; the body references no other symbol (0 relocations); the file's other declarations dropped.
  - **`func_L01_003B6718`** (`0x003B6718`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003B6718.c`, from
    `src/levels/01_oozla/L01_003835D8.c` (`func_L01_003B5310`, author commit `cd8c62bc`, Louis-Philippe Le Sieur, keeping 13 of 18 lines of the same author's draft `b18262d1`),
    task #2626. Adapted (byte-identical): our name at the v2.00 place; func_002A7820, func_00282948 (prototype-less declaration kept) and func_002B0990 as L01's copies func_L01_00311748, func_L01_002E3DE8 and func_L01_0031CFA0; the file's other declarations dropped; func_L01_0031CFA0's prototype types written void */int/float for upstream's u8 */s32/f32 (byte-neutral); common.h's typedefs written in the TU.
  - **`func_L01_003C1548`** (`0x003C1548`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003C1548.c`, from
    `src/levels/01_oozla/L01_003BB9D0-g8.c` (`func_L01_003C0140`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; D_L01_001AFEFC as our .lit label D_L01_001AFEBC; func_002AB8C0, func_00282978 and func_002B0990 as L01's copies func_L01_00316288, func_L01_002E3E18 and func_L01_0031CFA0; include_asm.h and the file's other declarations dropped.
  - **`func_L01_003C1AF0`** (`0x003C1AF0`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003C1AF0.c`, from
    `src/levels/01_oozla/L01_003BB9D0-g8.c` (`func_L01_003C06E8`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; func_001163B0 as the boot's rand (0x001163B0, codegen-neutral on this arm); D_L01_001AFED4 as our .lit label D_L01_001AFE94; func_002A7820, func_002A7750 and func_L01_003BFDC8 as L01's func_L01_00311748, func_L01_00311638 and func_L01_003C11D0; include_asm.h and the file's other declarations dropped.
  - **`func_L01_003C1B68`** (`0x003C1B68`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003C1B68.c`, from
    `src/levels/01_oozla/L01_003BB9D0-g8.c` (`func_L01_003C0760`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; D_L01_001AFED4 as our .lit label D_L01_001AFE94; func_002A7820, func_002A7750 and func_L01_003BFDC8 as L01's func_L01_00311748, func_L01_00311638 and func_L01_003C11D0; include_asm.h and the file's other declarations dropped.
  - **`func_L01_003C1BD0`** (`0x003C1BD0`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003C1BD0.c`, from
    `src/levels/01_oozla/L01_003BB9D0-g8.c` (`func_L01_003C07C8`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; D_L01_001AFED8 as our .lit label D_L01_001AFE98; func_002A7820, func_002A7750 and func_L01_003BFDC8 as L01's func_L01_00311748, func_L01_00311638 and func_L01_003C11D0; include_asm.h and the file's other declarations dropped.
  - **`func_L01_003C1C38`** (`0x003C1C38`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003C1C38.c`, from
    `src/levels/01_oozla/L01_003BB9D0-g8.c` (`func_L01_003C0830`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; D_L01_001AFEDC as our .lit label D_L01_001AFE9C; func_002A7820, func_002A7750 and func_L01_003BFDC8 as L01's func_L01_00311748, func_L01_00311638 and func_L01_003C11D0; include_asm.h and the file's other declarations dropped.
  - **`func_L01_003BEF30`** (`0x003BEF30`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003BEF30.c`, from
    `src/levels/01_oozla/L01_003BB9D0.c` (`func_L01_003BDB28`, author commit `4541c4ce`, ReaperOfVeriod),
    task #2626. VERBATIM but for names: our name at the v2.00 place; func_002A8598 as L01's copy func_L01_003127B8; common.h and the file's other declarations dropped.
  - **`func_L01_003BEFB8`** (`0x003BEFB8`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003BEFB8.c`, from
    `src/levels/01_oozla/L01_003BB9D0.c` (`func_L01_003BDBB0`, author commit `91e04732`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; func_00282948 as L01's copy func_L01_002E3DE8; common.h and the file's other declarations dropped.
  - **`func_L01_003BF330`** (`0x003BF330`, level program 01) in
    `going-decompiled/levels/usa/L01/src/func_L01_003BF330.c`, from
    `src/levels/01_oozla/L01_003BB9D0.c` (`func_L01_003BDF28`, author commit `ee3fee45`, llesieur99),
    task #2626. VERBATIM but for names: our name at the v2.00 place; D_00189EA0 as the boot's g_heroPos; func_00283180, func_00283198, func_002A7878, func_00282CC0, func_00282CF0, func_00282F20 and func_L01_003BDBE8 as L01's copies func_L01_002E4750, func_L01_002E4768, func_L01_003117A0, func_L01_002E4178, func_L01_002E41A8, func_L01_002E4440 and func_L01_003BEFF0; common.h and the file's other declarations dropped.
  - **`func_L09_003CA110`** (`0x003CA110`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003CA110.c`, from
    `src/levels/09_dobbo/L09_002F8BA0.c`
    (`func_L09_003C8CD8`, author commit `431ac503`, llesieur99), task #2561.
    Adapted: our name at the v2.00 place, three boot-copy callees by their L09 places, `*(long long *)` written `*(s64 *)` with `typedef long s64` (RULING #13197), `common.h` dropped.
  - **`func_L09_003DEF10`** (`0x003DEF10`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003DEF10.c`, from
    `src/levels/09_dobbo/L09_002F8BA0.c`
    (`func_L09_003DDAA0`, author commit `264cb2be`, llesieur99), task #2561.
    Verbatim but for names: our name at the v2.00 place, two boot-copy callees by their L09 places, `common.h` dropped.
  - **`func_L09_003DEFB0`** (`0x003DEFB0`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003DEFB0.c`, from
    `src/levels/09_dobbo/L09_002F8BA0.c`
    (`func_L09_003DDB40`, author commit `264cb2be`, llesieur99), task #2561.
    Verbatim but for names: our name at the v2.00 place, two boot-copy callees by their L09 places, `common.h` dropped.
  - **`func_L09_003E4630`** (`0x003E4630`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003E4630.c`, from
    `src/levels/09_dobbo/L09_002F8BA0.c`
    (`func_L09_003E31C0`, author commit `264cb2be`, llesieur99), task #2561.
    Verbatim but for names: our name at the v2.00 place, three boot-copy callees by their L09 places, `common.h` dropped.
  - **`func_L09_003F1740`** (`0x003F1740`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003F1740.c`, from
    `src/levels/09_dobbo/L09_002F8BA0.c`
    (`func_L09_003F02D0`, author commit `d5fd91e5`, Louis-Philippe Le Sieur), task #2561.
    Verbatim but for names: our name at the v2.00 place, one boot-copy callee by its L09 place, `common.h` dropped.
  - **`func_L09_003F17A0`** (`0x003F17A0`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003F17A0.c`, from
    `src/levels/09_dobbo/L09_002F8BA0.c`
    (`func_L09_003F0330`, author commit `264cb2be`, llesieur99), task #2561.
    Verbatim but for names: our name at the v2.00 place, two boot-copy callees by their L09 places, `common.h` dropped.
  - **`func_L09_003F7F48`** (`0x003F7F48`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003F7F48.c`, from
    `src/levels/09_dobbo/L09_002F8BA0-g8.c`
    (`func_L09_003F6AB8`, author commit `431ac503`, llesieur99), task #2561.
    Verbatim but for names: our name at the v2.00 place, `D_L09_001B0EE8` as our label `D_L09_001B0E98` (upstream's `sda` text kept, ignored by cc1), three boot-copy callees by their L09 places, `common.h` dropped, its f32 typedef written in the TU.
  - **`func_L09_003FF0E8`** (`0x003FF0E8`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003FF0E8.c`, from
    `src/levels/09_dobbo/L09_002F8BA0.c`
    (`func_L09_003FDC58`, author commit `ee3fee45`, llesieur99), task #2561.
    Adapted: our name at the v2.00 place, `D_L09_001FE6C0` as the address-named `D_L09_001FE640`, two boot-copy callees by their L09 places, `common.h` dropped.
  - **`func_L09_003FF368`** (`0x003FF368`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003FF368.c`, from
    `src/levels/09_dobbo/L09_002F8BA0.c`
    (`func_L09_003FDED8`, author commit `ee3fee45`, llesieur99), task #2561.
    Verbatim but for names: our name at the v2.00 place, six boot-copy callees by their L09 places, `common.h` dropped.
  - **`func_L09_003FFDA8`** (`0x003FFDA8`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003FFDA8.c`, from
    `src/levels/09_dobbo/L09_002F8BA0.c`
    (`func_L09_003FE918`, author commit `264cb2be`, llesieur99), task #2561.
    Verbatim but for names: our name at the v2.00 place, two boot-copy callees by their L09 places, `common.h` dropped.
  - **`func_L09_003FFE08`** (`0x003FFE08`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_003FFE08.c`, from
    `src/levels/09_dobbo/L09_002F8BA0-g8.c`
    (`func_L09_003FE978`, author commit `132e23b5`, bigmak94), task #2561.
    Adapted: our name at the v2.00 place, `D_L09_001B285C_1`/`D_L09_001B2878`/`D_L09_001FB560` as the address-named `D_L09_001B27DC`/`D_L09_001B27F8`/`D_L09_001FB4E0`, `sda` respelled `section(".sdata")` on both data externs (RULING #8620), `common.h` dropped.
  - **`func_L09_00404510`** (`0x00404510`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_00404510.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c`
    (`func_L09_00403080`, author commit `264cb2be`, llesieur99), task #2561.
    Adapted: our name at the v2.00 place, `D_L09_001B285C`/`D_L09_001FFFB560` as the address-named `D_L09_001B27DC`/`D_L09_001FB4E0` (upstream's `sda` text kept, ignored by cc1), built at -G8 against upstream's -G0, `common.h` dropped, typedefs written in the TU.
  - **`func_L09_00404590`** (`0x00404590`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_00404590.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c`
    (`func_L09_00403100`, author commit `bcc9f7db`, bigbeno37), task #2561.
    Adapted: our name at the v2.00 place, `D_L09_001B285C`/`D_L09_001FFFB560` as the address-named `D_L09_001B27DC`/`D_L09_001FB4E0`, `sda` respelled `section(".sdata")` (RULING #8620), callees `func_002A8508` and `func_L09_00403080` as `func_L09_00318B08` and our C `func_L09_00404510` (declared extern), `common.h` dropped, typedefs written in the TU.
  - **`func_L09_004064F0`** (`0x004064F0`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_004064F0.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c`
    (`func_L09_00405060`, author commit `264cb2be`, llesieur99), task #2561.
    Adapted: our name at the v2.00 place, `D_L09_001B285C`/`D_L09_001FFFB560` as the address-named `D_L09_001B27DC`/`D_L09_001FB4E0` (upstream's `sda` text kept, ignored by cc1), built at -G8 against upstream's -G0, `common.h` dropped, typedefs written in the TU.
  - **`func_L09_00408030`** (`0x00408030`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_00408030.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c`
    (`func_L09_00406BA0`, author commit `264cb2be`, llesieur99), task #2561.
    Adapted: our name at the v2.00 place, `D_L09_001B285C` as the address-named `D_L09_001B27DC`, `sda` respelled `section(".sdata")` (RULING #8620), `common.h` dropped, typedefs written in the TU.
  - **`func_L09_00408120`** (`0x00408120`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_00408120.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c`
    (`func_L09_00406C90`, author commit `264cb2be`, llesieur99), task #2561.
    Adapted: our name at the v2.00 place, `D_L09_001B285C` as the address-named `D_L09_001B27DC` (upstream's `sda` text kept, ignored by cc1), built at -G8 against upstream's -G0, `common.h` dropped, typedefs written in the TU.
  - **`func_L09_00409680`** (`0x00409680`, level program 09) in
    `going-decompiled/levels/usa/L09/src/func_L09_00409680.c`, from
    `src/levels/09_dobbo/L09_003FEA48.c`
    (`func_L09_004081F0`, author commit `96fa46bb`, Louis-Philippe Le Sieur (validator #2585: at `264cb2be` it was still INCLUDE_ASM)), task #2561.
    Adapted: our name at the v2.00 place, `D_L09_001B285C` as the address-named `D_L09_001B27DC`, `sda` respelled `section(".sdata")` (RULING #8620), `func_L09_00408138` as `func_L09_004095C8`, `common.h` dropped, typedefs written in the TU.

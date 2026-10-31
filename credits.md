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

  Eleven engine C bodies, two in text/16E980, one in text/1A00F0, one in
  text/1A8180, three in text/1B4218, one in text/1D54C0, one in text/1DFF80
  and two in text/188858, adopted with the authors' permission (held by the
  operator, RULING #10405) and re-checked byte-exact against our v2.00 ROM on
  the s136os arm (tasks #2232, #2234, #2235, #2248, #2251, #2252 and #2262,
  from audit NOTE #12089 rows 3, 4, 5, 6, 8 and 9).
  Contributors: bigbeno37, Flavius and Louis-Philippe Le Sieur. Each doc comment carries the `Source:`
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

- **RC2 decomp contributions — `src/game/002E4528-rc2.c` in OpenRAC `rac2-gc-decomp`, commit `96927f87`** (MIT, © 2026 RC2 decomp contributions)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  One engine C body, adopted with permission (held by the operator, RULING
  #10405) and re-checked byte-exact against our v2.00 ROM on the s136os arm
  (task #2239, from audit NOTE #11286 §(b) row 3). The file was carried into
  rac2-gc-decomp by atlasbez-dot (commit `ebf90a4c`, from
  atlasbez-dot/rac2-gc-decomp at `921a4dee`) under its own MIT notice, which
  is kept in the doc comment and in `THIRD-PARTY.md`. The doc comment carries
  the `Source:` line, which names every change beyond extern spellings.

  Used so far:
  - **`CastEmitterOcclusionRay`** (`0x002E5340`) in
    `going-decompiled/src/usa/text/1DFF80.cpp`, from `func_002E4528` (v1.01
    address). ADAPTED: our callee and data spellings and prototypes, our
    signature with the `SoundEmitterSlot` fields for its +0x20/+0x18, and a
    `Vec4` probe for their 4-byte `s32 sp[1]` (written 16 bytes by every
    callee). Returning `CollLine`'s result, as upstream does, is load-bearing.
- **OpenRAC — `rac2-gc-decomp`, commit `96927f87`, core bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  Three core C bodies, adopted with the authors' permission (held by the
  operator, RULING #10405) and re-checked byte-exact against our v2.00 ROM on
  the plain sdk29 arm (tasks #2230 and #2261). Contributor: Flavius. The doc
  comment carries the `Source:` line, which names every change beyond extern
  spellings.

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

  Four engine C bodies, adopted with the authors' permission (held by the
  operator, RULING #10405) and re-checked byte-exact against our v2.00 ROM on
  the s136os arm (task #2236). Contributors: bigbeno37 and Flavius. Each doc
  comment carries the `Source:` line, which names every respelling. All four
  are ADAPTED in the same way: our symbol names per the ROM's relocations,
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

- **OpenRAC — `rac2-gc-decomp`, commit `96927f87`, libcdvd/libgraph/libmpeg bodies** (GPL-3.0)
  <https://github.com/OpenRAC/rac2-gc-decomp> at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  Nine core C bodies, adopted with the authors' permission (held by the
  operator, RULING #10405) and re-checked byte-exact against our v2.00 ROM on
  the plain sdk29 arm (tasks #2231, #2237 and #2273). Contributors: Flavius and
  bigbeno37. Seven are rac1-decomp's C carried into rac2-gc-decomp; `sceCdInit` and
  `func_00130A90` are OpenRAC's own. Each doc comment carries the `Source:`
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

- **Forja contributors** (MIT, © 2026 Forja contributors), via OpenRAC
  `rac2-gc-decomp` at commit `96927f872c709e13aaa0f32265bb81a5531bf7ed`

  OpenRAC commit `fe9859d7` (Sergio Rodmauser) carried 129 functions over from
  an RC2 fork it does not name, each `-rc2.c` file under that fork's MIT
  notice. One is adopted, with the operator's 2026-10-31 "adopt all" (RULING #12108, task #2237);
  the notice is reproduced in [THIRD-PARTY.md](THIRD-PARTY.md) and in the
  function's doc comment.

  Used so far:
  - **`func_001248B8`** (`0x001248B8`, libcdvd's SIF command 0x80000012
    handler) in `going-decompiled/src/usa/cod/022FA8.c`, adapted (name at
    our split address), from `src/core/001248B0-rc2.c` (`func_001248B0`).

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

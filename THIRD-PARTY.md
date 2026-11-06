# Licensing

## This project

SPDX-License-Identifier: `GPL-2.0-or-later`

The project's own work is licensed under the GNU General Public License,
version 2 or (at your option) any later version. That covers the decompiled C
and C++, headers, splat configuration, symbol names, build and verification
tools, and documentation. The full text is in [LICENSE](LICENSE).

The licence covers only that work. It grants no rights in **Ratchet & Clank:
Going Commando** itself. The game's code, data, assets, names and trademarks
belong to their owners. This repository contains none of them, and nothing
derived from your copy of the game (the disassembly, extracted assets, the
built image) is tracked.

## Bundled third-party source

These directories hold upstream source, unmodified unless stated. Each keeps
its own licence, and its notices are preserved in the files.

| Path | Origin | Licence |
|---|---|---|
| `going-decompiled/libgcc/` | GCC's `libgcc2.c`, `fp-bit.c` and `longlong.h` from the GCC repository; `cygnus-ee/` from the Cygnus EE toolchain tree (see `PROVENANCE` there) | GPL-2.0-or-later with the GCC runtime exception (stated in each file); text in `going-decompiled/libgcc/COPYING` |
| `going-decompiled/libc/newlib/` | newlib's R5900 string and memory routines (`*.S`), Copyright (C) 1999 Cygnus Solutions | The Cygnus permission notice in each file: use, copy, modify and distribute freely, provided the notice is kept |
| `going-decompiled/libm/newlib/` | newlib's libm `fdlibm.h`, `s_isnan.c` and `w_sqrt.c`, Copyright (C) 1993 Sun Microsystems, Inc. | The SunPro permission notice in each file: use, copy, modify and distribute freely, provided the notice is kept |

The `shim/` directories under `libgcc/` and `libm/` are written by this project
and fall under its own licence.

## Not included

The SN Systems ProDG compiler and the Sony PS2 SDK headers and libraries are
proprietary and are not part of this repository. The `scripts/fetch_*.sh`
scripts download them for a local build.

## Acknowledgements

Names and research adopted from other projects are listed, with commit and
licence, in [credits.md](credits.md). Their code is not copied into this tree,
with the exception below.

### OpenRAC rac2-gc-decomp at d43c3b99 (GPL-3.0)

Three C function bodies are taken from OpenRAC's `rac2-gc-decomp`,
<https://github.com/OpenRAC/rac2-gc-decomp> at commit
`d43c3b99b984fb63556c844a21ee99e8e31d2deb` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405), task #2206:

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_001296A0` in `going-decompiled/src/usa/cod/022FA8.c` | `src/core/00129450.c` (from rac1-decomp `src/core/00125630.c`) | `func_001296A0` (rac1-decomp `func_00125880`) |
| `func_0012DAC0` in `going-decompiled/src/usa/cod/022FA8.c` | `src/core/0012DA98.c` (from rac1-decomp `src/core/00125630.c`) | `func_0012DAC0` (rac1-decomp `func_00129CA0`) |
| `func_0012DD60` in `going-decompiled/src/usa/cod/022FA8.c` (adapted) | `src/core/0012DD60.c`, itself adapted from Lombyte (MIT) `src/sdk/library/disprefimagefield.c` | `func_0012DD60` (Lombyte `_dispRefImageField`) |

The Lombyte-derived body is also covered by the MIT notice reproduced under
"Lombyte (MIT)" below. The GPL-2.0-or-later note in the next section applies to
these bodies too.

### OpenRAC rac2-gc-decomp (GPL-3.0)

Seven C function bodies are taken from OpenRAC's `rac2-gc-decomp`,
<https://github.com/OpenRAC/rac2-gc-decomp>, five at commit
`974302ba936bb455acd78034b58ad4b98bafcc6b` and two at commit
`8b490f56b8e46a1b749b54fd898946a4f09c1cca` (root `LICENSE` at both: GNU General
Public License, version 3), with the authors' permission (RULING #10405):

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_00131850`, `func_00131908` in `going-decompiled/src/usa/cod/0314C0.c` | `src/core/00131780.c` (from rac1-decomp `src/core/0012CC90.c`) | same names (v1.01 addresses equal v2.00 here) |
| `MatrixToEulerAngles` in `going-decompiled/src/usa/text/1A8180.c` | `src/game/002AA800.c` | `func_002AB6D8` |
| `InitMenuBgImageBuffers`, `func_002DCDC0` in `going-decompiled/src/usa/text/1D54C0.cpp` | `src/game/002D5E98.c` | `func_002DA238`, `func_002DC090` |
| `IpuGetBits` in `going-decompiled/src/usa/cod/022FA8.c` | `src/core/0012C508.c` at commit `8b490f56b8e46a1b749b54fd898946a4f09c1cca` (from rac1-decomp `src/core/00125630.c`) | `func_0012C878` (rac1-decomp `func_00128A58`) |
| `func_00293760` in `going-decompiled/src/usa/text/191238.cpp` | `src/game/00291758.c` at `8b490f56` (from rac1-decomp `src/game/loaders.c`) | `func_00292EB0` (rac1-decomp `func_00203808`) |

This project is `GPL-2.0-or-later`; the GPL-3.0 text of these bodies is
combined into it under the "or later" grant. Any wider licensing question is
the operator's (RULING #8304 defers it until the repository goes public).

### OpenRAC rac2-gc-decomp at d43c3b99, further bodies (GPL-3.0)

Six C function bodies are taken from OpenRAC's `rac2-gc-decomp`,
<https://github.com/OpenRAC/rac2-gc-decomp> at commit
`d43c3b99b984fb63556c844a21ee99e8e31d2deb` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405), tasks #2207, #2208, #2216, #2217 and #2218:

| Here | Upstream file | Upstream name |
|---|---|---|
| `_exitThunk` in `going-decompiled/src/usa/cod/0314C0.c` | `src/core/00131780.c` | `func_00131CA8` |
| `func_0027F348` in `going-decompiled/src/usa/text/178E88.cpp` | `src/game/0027B948.c` | `func_0027ECC8` |
| `BuildAttractReelPlaylist` in `going-decompiled/src/usa/text/16E980.c` | `src/game/0026EA00.c` | `func_0026EC58` |
| `ApplyMobyLocalTransformDelta` in `going-decompiled/src/usa/text/1B4218.cpp` (adapted) | `src/game/002B3868.c` | `func_002B4090` (v1.01 address) |
| `func_00352058` in `going-decompiled/src/usa/text/250080.cpp` (adapted: two `long long` respelled `long`, upstream forward declaration dropped, signature takes our shared prototype) | `src/game/003512B8.c` | `func_00351310` |
| `func_003011C0` in `going-decompiled/src/usa/text/1FFBA0.cpp` (adapted: callee and table-origin spelling are ours) | `src/game/002FC8D0.c` | `func_00300330` |

The GPL-2.0-or-later note in the section above applies to these bodies too.

### OpenRAC rac2-gc-decomp at 96927f87 (GPL-3.0)

Thirty-seven C function bodies are taken from OpenRAC's `rac2-gc-decomp`,
<https://github.com/OpenRAC/rac2-gc-decomp> at commit
`96927f872c709e13aaa0f32265bb81a5531bf7ed` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405), tasks #2230, #2232, #2234, #2235, #2236, #2248, #2251, #2252, #2261, #2262, #2272, #2294, #2295, #2304, #2305, #2306, #2308, #2311 and #2320. Each is adapted, with every respelling named in the
function's `Source:` line.

| Here | Upstream file | Upstream name |
|---|---|---|
| `ApplyCameraShakeAxis` in `going-decompiled/src/usa/text/16E980.c` (adapted: camera block, signature and callee prototypes are ours) | `src/game/00271958-g8.c` | `func_00271958` (v1.01 address) |
| `SampleCameraFogZone` in `going-decompiled/src/usa/text/16E980.c` (adapted: fog block and region array reached through our symbols; signature and callees ours) | `src/game/0026EA00.c` | `func_002735A0` (v1.01 address) |
| `func_0011DE08` in `going-decompiled/src/usa/cod/015180.c` (adapted: the unit's own prototypes, with a `(SifRpcClient *)` and two `(char *)` casts) | `src/core/0011DDC8.c` (from rac1-decomp `src/core/00119D88.c`) | `func_0011DE08` |
| `func_0011C1F8` in `going-decompiled/src/usa/cod/015180.c` (adapted: our alias spellings, the float conversion spelled explicitly, `char *ap` for `va_list ap`; task #2261) | `src/core/0011BF18.c` (from rac1-decomp `src/core/00119D88.c`) | `func_0011C1F8` |
| `func_0011D9C0` in `going-decompiled/src/usa/cod/015180.c` (adapted: our extern spellings, one `(volatile s32 *)` cast for their `volatile` table; task #2261) | `src/core/0011D950.c` (from rac1-decomp `src/core/00119D88.c`) | `func_0011D9C0` |
| `WaitVblankStartIntc` in `going-decompiled/src/usa/cod/015180.c` (adapted: our name; each `__sync_synchronize()` written as an inline `sync` behind `#ifndef TARGET_NATIVE`, the RULING #12168 hardware barrier), task #2308 | `nonmatching/core/func_0011B0B0.c`, a near-miss draft (its only commit `df6a5a10`, author "agent-core-04") | `func_0011B0B0` |
| `IntegerToAscii` in `going-decompiled/src/usa/cod/015180.c` (adapted: our name, `s32`/`u32` for `int`/`unsigned int`; sn2952 arm, task #2311) | `src/core/00118548.c` | `func_00118548` |
| `BufferedWrite128` in `going-decompiled/src/usa/cod/015180.c` (adapted: our name, `s32`/`u32`, the unit's `func_0011B1E8` prototype; sn2952 arm, task #2311) | `src/core/00118BC0.c` | `func_00118BC0` |
| `VfprintfDispatch` in `going-decompiled/src/usa/cod/015180.c` (adapted: our names for it and its two engines, `s32`, the `+0x54` read through a `PrintfStream` view; its switch table bound to the ROM's by `tools/ee/jtbl_bind.sh`; sn2952 arm, task #2311) | `src/core/00118CC8.c` | `func_00118CC8` |
| `BufferedWrite128Secondary` in `going-decompiled/src/usa/cod/015180.c` (adapted: as `BufferedWrite128`; sn2952 arm, task #2311) | `src/core/00119AC0.c` | `func_00119AC0` |
| `func_002AD590` in `going-decompiled/src/usa/text/1A8180.c` (adapted: callee and data spellings ours; EE-only callee prototypes, task #2234) | `src/game/002AA800.c` | `func_002AC728` (v1.01 address) |
| `UpdateBestiaryMenuInput` in `going-decompiled/src/usa/text/1CA080.cpp` (adapted) | `src/game/002CDB30-g8.c` | `func_002CDB30` (v1.01 address) |
| `UpdateSkillPointsMenu` in `going-decompiled/src/usa/text/1CA080.cpp` (adapted) | `src/game/002CFBE8-g32.c` | `func_002CFBE8` (v1.01 address) |
| `UpdateHelpTopicMenuInput` in `going-decompiled/src/usa/text/1CA080.cpp` (adapted) | `src/game/002D1D20-g32.c` | `func_002D1D20` (v1.01 address) |
| `func_002D2FC8` in `going-decompiled/src/usa/text/1CA080.cpp` (adapted) | `src/game/002D2018-g32.c` | `func_002D2018` (v1.01 address) |
| `func_002CC7D8` in `going-decompiled/src/usa/text/1CA080.cpp` (adapted: symbol spellings and callee prototype ours; its switch table bound to the ROM's by `tools/ee/jtbl_bind.sh`), task #2272 | `src/game/002C7748-g8.c` | `func_002CB820` (v1.01 address) |
| `func_002D75D8` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: menu block, pad block, tables and callee are our symbols; one test split through a local for the C++ front end) | `src/game/002D6970-g32.c` | `func_002D6970` (v1.01 address) |
| `UpdateMobyMotionVelocity` in `going-decompiled/src/usa/text/1B4218.cpp` (adapted: callee spellings and signature are ours, `sda` attribute dropped, one callee in its definition's argument order, a native-only call order for another) | `src/game/002B52A0-g8.c` | `func_002B52A0` (v1.01 address) |
| `StartTertiaryVoice` in `going-decompiled/src/usa/text/1B4218.cpp` (adapted: symbol spellings are ours, the sample start narrowed to `s32` at the call for our `s64` prototype, their `u8` typedef dropped) | `src/game/002B6120-keep-block-order.c` | `func_002B7088` (v1.01 address) |
| `UpdateMobyAnimLoopSound` in `going-decompiled/src/usa/text/1A00F0.cpp` (adapted: signature, symbol spellings and prototypes ours; control flow theirs), task #2251 | `src/game/0029DE38.c` | `func_0029F450` (v1.01 address) |
| `UpdateDialogVoiceManager` in `going-decompiled/src/usa/text/1B4218.cpp` (adapted: symbol spellings ours, the unit's prototypes with casts at the call, `func_00132BC0` declared with their parameter types; their header: adapted from RAC1 `src/game/music.c` `music_Update`, GPL-3.0), task #2262 | `src/game/002B7578.c` | `func_002B7578` (v1.01 address) |
| `func_002E0010` in `going-decompiled/src/usa/text/1DFF80.cpp` (adapted: symbol spellings ours, their element struct named `LevelSelectEntry`), task #2262 | `src/game/002DF1B8-g8.c` | `func_002DF1F8` (v1.01 address) |
| `UpgradeWeaponToMax` in `going-decompiled/src/usa/text/188858.c` (adapted: signature, local names, symbol spellings and types ours; control flow theirs), task #2248 | `src/game/002887B0.c` | `func_002887B0` (v1.01 address) |
| `func_002897B8` in `going-decompiled/src/usa/text/188858.c` (adapted: the subtitle-state symbol, struct and field names are ours), task #2248 | `src/game/00288D88.c` | `func_00288D90` (v1.01 address) |
| `func_002D7AE0` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: our symbols as label aliases of their struct views, their byte `D_001F291B` written as the menu view's field at the same address, `ROM_SPLIT` on the file-load view), task #2295 | `src/game/002D5E98.c` | `func_002D6E78` (v1.01 address) |
| `func_002D8DD0` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: our symbols, two as label aliases with their types; `MACRO_ADDR` dropped for the unit's `.extern , 16`), task #2295 | `src/game/002D8120.c` | `func_002D8120` (v1.01 address) |
| `func_002DA358` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: `unsigned long long` written `unsigned long`, 64-bit on the EE; our symbols and callee prototype; its switch table bound to the ROM's by `tools/ee/jtbl_bind.sh`), task #2295 | `src/game/002D5E98.c` | `func_002D9600` (v1.01 address) |
| `func_002DAE70` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: `unsigned long long` written `u64`; our symbols, two as label aliases, and our `DrawGlyphQuad` prototype), task #2295 | `src/game/002D5E98.c` | `func_002DA118` (v1.01 address) |
| `func_002DCBF0` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: our symbols, the pad block as an `int` label alias, their `D_001F2974` read as `g_menuScreenBlock+0x134`), task #2295 | `src/game/002D5E98.c` | `func_002DBEC0` (v1.01 address) |
| `func_002DCCC8` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: as `func_002DCBF0`), task #2295 | `src/game/002D5E98.c` | `func_002DBF98` (v1.01 address) |
| `snd_QueueCommandToRing` in `going-decompiled/src/usa/cod/0321A0.c` (adapted: `s32` return for their void, callee and descriptor spellings ours, their `sda` declarations dropped for the unit's `.extern ,16` ones), task #2294 | `src/core/00132DF0-g8.c` | `func_00132DF0` |
| `OnVblankInterrupt` in `going-decompiled/src/usa/cod/0321A0.c` (adapted: our symbol names; their `NOT_SDA`/`MACRO_ADDR` declarations replaced by `.extern ,16` lines), task #2294 | `src/core/001339B0.c` (their note: from rac1-decomp `src/core/permcb.c` `func_0012F308`) | `func_001339B0` |
| `GuiIconScreenInit2` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our callee names and prototypes with `GuiElement *`/`s32` casts, our v2.00 data names, a plain `g_guiInstance` for their `sda` array, memset through an asm-label alias; signature ours), task #2304 | `src/game/00341C20-g8.c` | `func_00342118` (v1.01 address) |
| `func_0034BA50` in `going-decompiled/src/usa/text/248B50.cpp` (adapted: our callee and data names and prototypes, widget arguments cast to `GuiWidget *`; their `section(".sdata")` on the GUI instance pointer dropped for the unit's `.extern g_guiInstance, 16`), task #2306 | `src/game/00345358.c` | `func_0034AD10` (v1.01 address) |
| `func_00350910` in `going-decompiled/src/usa/text/250080.cpp` (adapted: our callee names and prototypes, their `.sdata` decode-buffer word read as our `u8 *D_001B2354`, one `(s32 *)` cast for the C++ front end), task #2320 | `src/game/0034CEA0.c` | `func_0034FBC8` (v1.01 address) |
| `func_003526A8` in `going-decompiled/src/usa/text/250080.cpp` (adapted: our callee names and prototypes, two called through void-returning casts; their `D_001AE860` as our `D_1AE818` with a `section(".data")` addressing device; their `sda` arena word as the unit's `g_pFmvArenaBase`), task #2320 | `src/game/003512B8.c` | `func_00351960` (v1.01 address) |
| `func_002E07F8` in `going-decompiled/src/usa/text/1DFF80.cpp` (adapted: data spellings ours through an `__asm__`-label struct view and an `.sdata` cursor alias, `MACRO_ADDR` dropped, `int`/`long` spelled `s32`/`s64`, local names ours), task #2305 | `src/game/002DF1B8-clear.c` | `func_002DF9E0` (v1.01 address) |

The GPL-2.0-or-later note in the section "OpenRAC rac2-gc-decomp (GPL-3.0)" above applies to these bodies too.

### OpenRAC rac2-gc-decomp at 96927f87, boundary-split bodies (GPL-3.0)

One C function body is taken from OpenRAC's `rac2-gc-decomp`,
<https://github.com/OpenRAC/rac2-gc-decomp> at commit
`96927f872c709e13aaa0f32265bb81a5531bf7ed` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405), task #2240:

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_002907C0` in `going-decompiled/src/usa/text/188858.c` (adapted: their `Gauge` view is our `HudElement` by raw offset; signature ours) | `src/game/0028FF50.c` | `func_0028FF50` (v1.01 address) |

The GPL-2.0-or-later note in the section "OpenRAC rac2-gc-decomp (GPL-3.0)" above applies to this body too.

### OpenRAC rac2-decomp at b2a35115 (GPL-3.0)

C function bodies are taken from OpenRAC's `rac2-decomp`,
<https://github.com/OpenRAC/rac2-decomp> at commit
`b2a35115e0cf580a1fdab8dd1071b157799e590e` (root `LICENSE`: GNU General Public
License, version 3; the repository carries `rac2-gc-decomp`'s history, with
`96927f87` an ancestor), with the authors' permission (RULINGs #10405 and
#12108). Each is adapted, with every respelling named in the function's
`Source:` line. None of the source files below carries a notice of its own;
the newlib origin of the `cod/015180` malloc bodies is noticed in the section
"Newlib, origin files with no in-file notice" below, and the notices of the
four Newlib stdio bodies in the section "Newlib `libc/stdio`, cod/015180
bodies" below.

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_00346878` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol names, signature and `PlayGlobalSound` prototype; their owner struct as our `GuiGridCursor` view; their `.sdata` `g_inventoryOwned` and their typed grid array as EE-only asm-label aliases; `sda` dropped), task #2467 | `src/us2/game/00346878-g8.c` | `func_US2_00346878` (v2.00 address) |
| `func_00346CD8` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol spellings and the unit's declaration types, `s32`/`u8` for their `int`/`unsigned char`; two through EE-only asm-label aliases, a typed view (RULING #13168) replacing their typed array and the `la` alias (RULING #8620) replacing their `section(".sdata")`; their `sda` dropped; struct views renamed and retyped, their material record as our `GuiWeaponTableEntry`; signature ours with a local cast), task #2468 | `src/us2/game/00346CD8-pad-g8.c` (author commit `13388073`, Flavius) | `func_US2_00346CD8` (v2.00 address) |
| `func_00345298` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol names and signature with a local `GuiWeaponSelectCursor` view of their `DobboGridState312`; their `Entry224` as our `GuiWeaponTableEntry` and their `Ids_00344378` object as the unit's `GuiStringIdTable` copied from `D_1AE238`; their `sda` `g_inventoryOwned` as an EE-only `.sdata` asm-label alias (RULING #8620); the unit's plain cc1 2.9 arm, no selector row; blob identical at `5776dcc3` and `f3cc4dbf`), task #2501 | `src/us2/game/003448C0.c` (author commit `cb63d202`, bigbeno37) | `func_US2_00345298` (v2.00 address) |
| `func_00346AF8` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol names, signature and `PlayGlobalSound` prototype; their `I(off)` word macro as our `GuiSelectPairCursor` view; their typed `InputEntry` array as an EE-only typed view of `D_1AA7F8` (RULING #13168) and their `InputRecord` as our `GuiWeaponTableEntry`; their `.sdata` `g_inventoryOwned` as an EE-only asm-label alias (RULING #8620), their `sda` on `D_001ADB10` dropped; per-function `S136EXTRA=-fno-strict-aliasing`, RULING #12178; blob identical at `5776dcc3` and `f3cc4dbf`), task #2501 | `src/us2/game/00346AF8-g8.c` (author commit `37ee1b19`, Flavius) | `func_US2_00346AF8` (v2.00 address) |
| `func_00338F88` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol names and the unit's callee prototypes, their `unsigned long` style parameters as the definitions' `s32` except `func_00280B48`'s `u64` (a `DECL-LEVER`); their `sda` `D_001A7340[1]` read through an EE-only unsized `.sdata` view of `g_screenWidth` (RULINGs #8620/#13168), native reading `g_screenHeight`; their `func_US2_00339398` called through the unit's own float-first prototype (same registers); s136os arm as the unit is; blob identical at `5776dcc3` and `f3cc4dbf`), task #2501 | `src/us2/game/00338F88-cc.c` (author commit `ff791d1e`, bigbeno37) | `func_US2_00338F88` (v2.00 address) |
| `func_0033D070` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol names and signature; their `struct SolIds` as the unit's `CaptionIdPair`; `GetLocalizedString`'s `s32` return cast to `char *`; `unsigned long` as `u64`; their `sda` scalars through an EE-only unsized `.sdata` view of `g_screenWidth` (RULINGs #8620/#13168) and the unit's existing `g_timerHudYScale` alias, `.extern g_bProgressiveScan, 16` and `D_1ADDA8` `.data` devices; upstream's identical-arm branch kept; the file is named `-qcopy` but this body has no qcopy construct; s136os arm as the unit is; blob identical at `5776dcc3` and `f3cc4dbf`), task #2501 | `src/us2/game/0033C958-qcopy.c` (author commit `c4928ea3`, Flavius) | `func_US2_0033D070` (v2.00 address) |
| `func_003380B8` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol names and signature with a local `grid`; their `Grid*` types renamed `GuiItemGrid*`/`GuiGrid*`, layouts unchanged; element pointers passed to the unit's `GuiElement *` prototypes; their `sda` `g_inventoryOwned` as an EE-only `.sdata` asm-label alias and `D_1ADB08` with `section(".data")` (RULING #8620); per-function `S136EXTRA=-fno-strict-aliasing`, RULING #12178; the upstream file notes it was adapted from the public RAC1 pause-menu grid renderer (GPL-3.0); blob identical at `5776dcc3` and `f3cc4dbf`), task #2501 | `src/game/00337210.c` (author commit `2e9774a0`, Louis-Philippe Le Sieur) | `func_00337210` (v1.01 address) |
| `func_L24_0034D598` in `going-decompiled/levels/usa/L24/src/func_L24_0034D598.c`, level program 24 (adapted: our name at the v2.00 place, the boot's `g_anAvailableLevelOrder`, four level symbols renamed to the v2.00 ROM's relocation targets, `volatile` and `sda` dropped as measured inert, `include_asm.h` dropped; the blob is identical at the repository's head `5833be5b716983046be737d85080b96bc9f1435a`), task #2495 | `src/levels/24_ship_shack/L24_0034C450-init-g8.c` | `func_L24_0034C450` (v1.01 address) |
| `VfprintfInteger` in `going-decompiled/src/usa/cod/015180.c` (adapted: our name, `s32` return, `reent` for their `data`; their `PrintFile` view merged into the unit's `PrintfStream`; callees respelled by address to `_mbtowc_r`, `BufferedWrite128Secondary`, `func_00115250` (their `memchr` label), `strlen` and `memcpy`; `(void *)D_00133E74` for the unit's `s32` extern; `const` dropped from the pad-string externs; its switch table bound to the ROM's by `tools/ee/jtbl_bind.sh`; sn2952 arm), task #2465 | `src/core/00119BF8.c` (author commit `bf067513`, bigbeno37) | `func_00119BF8` |
| `PatchShrubPacketTex0` in `going-decompiled/src/usa/text/1DFF80.cpp` (adapted: our symbol names; their {short, short} table entry as our `ShrubTexVramPair` view; `int`/`short`/`unsigned char` spelled `s32`/`s16`/`u8`; an `(s32 *)` cast on `g_shrubClassTable[*list]`; their `nosda` not carried; an unused local dropped; local names ours; s136os arm, gcse ON via the per-function `S136EXTRA=` override, RULING #12178), task #2470 | `src/game/002E10B8.c` (author commit `7fccf735`, Flavius) | `func_002E10B8` (v1.01 address) |
| `func_002DC6C8` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: our symbol names, signature and record types; their struct-typed `D_001C4FA0` as a cast view of our `g_mapVertexData`; `volatile` and `sda` on the list pointer dropped), task #2479 | `src/game/002DB988-init-g8.c` | `func_002DB988` (v1.01 address) |
| `func_00115C90` in `going-decompiled/src/usa/cod/015180.c`, newlib `_sbrk_r` (adapted: `int` written `s32`, parameters named; sn2952 arm), task #2498 | `src/core/00115C90.c`, author commit `399c57d5`, Louis-Philippe Le Sieur | `func_00115C90` |
| `malloc` in `going-decompiled/src/usa/cod/015180.c`, newlib `malloc` with SCE's lock pair (adapted: our name, newlib's `void *`/unsigned types for their `int`s, the unit's `s32` model for `D_00133E74`; sn2952 arm), task #2498 | `src/core/00115F28.c`, author commit `399c57d5`, Louis-Philippe Le Sieur | `func_00115F28` |
| `func_00115F78` in `going-decompiled/src/usa/cod/015180.c`, newlib `free` with SCE's lock pair (adapted: `_free_r` for their `func_00116E10`, shared lock prototypes for their asm-label ones, the unit's `s32` model; sn2952 arm), task #2498 | `src/core/00115F78.c`, author commit `0f740805`, Louis-Philippe Le Sieur | `func_00115F78` |
| `func_00116300` in `going-decompiled/src/usa/cod/015180.c`, newlib's `__malloc_lock` hook with SCE's recursive semaphore-lock body (adapted: newlib's unused `reent` parameter added, `int` written `s32`; sn2952 arm; RULING #13221), task #2498 | `src/core/00115200.c`, author commit `4541c4ce`, ReaperOfVeriod | `func_00116300` |
| `func_00116360` in `going-decompiled/src/usa/cod/015180.c`, newlib's `__malloc_unlock` hook with SCE's body (adapted: `SignalSema` for their `func_0011AC40`, newlib's unused `reent` parameter added, `int` written `s32`; sn2952 arm; RULING #13221), task #2498 | `src/core/00116360.c`, author commit `0f740805`, Louis-Philippe Le Sieur | `func_00116360` |
| `func_00119BC8` in `going-decompiled/src/usa/cod/015180.c` (Newlib `vfiprintf`, notice in the Newlib section below; adapted: callee func_00119BF8 spelled `VfprintfInteger` with the unit's prototype, `int` parameters typed `PrintfStream *`/`char *`/`void *`, the +0x54 read as the `PrintfStream` view's `data`; sn2952 arm), tasks #2497 and #2518 | `src/core/00119BC8.c`, author commit `7b023993` (Louis-Philippe Le Sieur) | `func_00119BC8` |
| `sprintf` in `going-decompiled/src/usa/cod/015180.c` (Newlib `sprintf`, notice below; adapted: our symbol name, callee func_00118CC8 spelled `VfprintfDispatch` with the unit's prototype, their `File_00115DA8` named `SprintfFile` with `s32`/`s16`, `_impure_ptr` read through the unit's `s32 D_00133E74`, parameters typed; a `TARGET_NATIVE` branch of ours builds the va_list with `__builtin_va_start`, as clang has no `__builtin_next_arg`; sn2952 arm), tasks #2497 and #2518 | `src/core/00115DA8.c`, author commit `ab9b31a8` (Louis-Philippe Le Sieur) | `func_00115DA8` |
| `func_00115CF0` in `going-decompiled/src/usa/cod/015180.c` (Newlib `fiprintf`, notice below; adapted: parameters typed `void *stream, const char *fmt`, callee prototype the unit's; a `TARGET_NATIVE` `__builtin_va_start` branch of ours; sn2952 arm), tasks #2497 and #2518 | `src/core/00115CF0.c`, author commit `ab9b31a8` (Louis-Philippe Le Sieur) | `func_00115CF0` |
| `snd_PrintError` in `going-decompiled/src/usa/cod/015180.c` (Newlib `printf`, notice below; adapted: our symbol name, callee func_00118CC8 spelled `VfprintfDispatch` with the unit's prototype, `_impure_ptr` read through the unit's `s32 D_00133E74`, parameter typed, locals named; a `TARGET_NATIVE` `__builtin_va_start` branch of ours; sn2952 arm), tasks #2497 and #2518 | `src/core/00115D48.c`, author commit `ab9b31a8` (Louis-Philippe Le Sieur) | `func_00115D48` |
| `func_002DEEF8` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: our symbol names; their struct views renamed; their `sda` byte array `D_001A7A0A[0]` as our scalar `g_miscExtras` behind an EE-only `.extern g_miscExtras, 16` (RULING #12316); `g_fileLoadState` through the unit's `section(".data")` declaration (RULING #8620); `s32`/`s16`/`u8` spellings; its switch table bound to the ROM's by `tools/ee/jtbl_bind.sh`), task #2480 | `src/game/002DDEC8-animation.c` (author commit `b073e803`, bigbeno37) | `func_002DE0D0` (v1.01 address) |

The four `cod/015180` files are byte-identical between `b2a35115` and
`8b26d8adeb0b5515a962f83230df4bf909fd0cf5`, the commit task #2497 read.

`VfprintfInteger`'s upstream file is named by neither upstream's
`THIRD_PARTY_NOTICES.md` nor its `CREDITS.md`; its header says it was
reconstructed from retail control flow and the newlib 1999 implementation in
rac1-decomp `src/core/00116FA0.c`.

The GPL-2.0-or-later note in the section "OpenRAC rac2-gc-decomp (GPL-3.0)" above applies to these bodies too.

### Newlib, origin files with no in-file notice

C bodies in `going-decompiled/src/usa/cod/015180.c`, taken from OpenRAC
(section "OpenRAC rac2-decomp at b2a35115" above), reconstruct newlib C-library
code as SCE built it (RULING #13179, task #2498): `func_00115C90` is newlib's
`_sbrk_r` (`newlib/libc/reent/sbrkr.c`), `malloc` and `func_00115F78` are
newlib's `malloc` and `free` cover functions (`newlib/libc/stdlib/malloc.c`),
to which SCE added a `__malloc_lock`/`__malloc_unlock` pair. The origin files
were read in a fresh clone of <https://sourceware.org/git/newlib-cygwin.git> at
the release tag nearest the ROM's "newlib 1999", `newlib-2000-02-17` (commit
`061afe83dc04a43321d69b5f04860e5de799665c`, the earliest newlib tag; no 1999
tag exists), and again at HEAD `5aa0a116d1a3e4bf3be150e5eaaa8e2ed07637f5`.
**`sbrkr.c` and `malloc.c` carry no in-file copyright notice at either ref**
(`newlib/libc/stdlib/strtol.c` at the same tag carries one, so the absence is
real; the same holds for `newlib/libc/stdlib/abort.c`, `newlib/libc/stdlib/assert.c`
and `newlib/libc/stdlib/atoi.c`, the origins of `abort`, `AssertFail` (newlib
`__assert`) and `func_00115E68` (newlib `atoi`), also in `cod/015180.c`, adopted
via OpenRAC `rac2-decomp` at `5833be5b`, task #2499; `abort` is SCE-modified,
the raise-loop replaced by one call to the exit thunk). `COPYING.NEWLIB` is absent from the tag's tree (it was first added in
2001), so the text below is quoted from HEAD. Its preamble says that, unless
otherwise noted in the body of a source file, the notices that follow apply;
for a notice-less file that is section (1), recorded here as newlib's default
for notice-less files (watcher-2, 2026-11-06). A newlib origin file that
carries its own in-file notice uses that notice instead. Verbatim, preamble
and section (1):

```
The newlib subdirectory is a collection of software from several sources.

Each file may have its own copyright/license that is embedded in the source 
file.  Unless otherwise noted in the body of the source file(s), the following copyright
notices will apply to the contents of the newlib subdirectory:

(1) Red Hat Incorporated

Copyright (c) 1994-2009  Red Hat, Inc. All rights reserved.

This copyrighted material is made available to anyone wishing to use,
modify, copy, or redistribute it subject to the terms and conditions
of the BSD License.   This program is distributed in the hope that 
it will be useful, but WITHOUT ANY WARRANTY expressed or implied, 
including the implied warranties of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE.  A copy of this license is available at 
http://www.opensource.org/licenses. Any Red Hat trademarks that are
incorporated in the source code or documentation are not subject to
the BSD License and may only be used or replicated with the express
permission of Red Hat, Inc.
```

`func_00116300` and `func_00116360` implement newlib's `__malloc_lock` /
`__malloc_unlock` hook (`newlib/libc/stdlib/mlock.c`, which ships only empty
stubs) with SCE's own recursive semaphore lock; they contain no newlib text,
so no newlib notice applies to them (RULING #13221).

### Newlib `libc/stdio`, cod/015180 bodies (BSD, University of California)

Four bodies in `going-decompiled/src/usa/cod/015180.c`, taken from OpenRAC
`rac2-decomp` (rows in "OpenRAC rac2-decomp at b2a35115" above), reconstruct
functions of Newlib's C library. They are adopted under the operator's RULING
#13179, with each origin file's notice reproduced verbatim below as it stands
at Newlib's current HEAD, <https://sourceware.org/git/newlib-cygwin.git> at
commit `5aa0a116d1a3e4bf3be150e5eaaa8e2ed07637f5` (read 2026-11-06). The ROM's
build is nearest the tag `newlib-2000-02-17` (commit
`061afe83dc04a43321d69b5f04860e5de799665c`, the newlib 1.8.2 release), the
earliest snapshot in that repository. Tasks #2497 and #2518.

| Here | Newlib origin file: function |
|---|---|
| `func_00119BC8` | `newlib/libc/stdio/vfprintf.c`: `vfprintf`, built with `-DINTEGER_ONLY` as `vfiprintf` |
| `sprintf` | `newlib/libc/stdio/sprintf.c`: `sprintf` |
| `func_00115CF0` | `newlib/libc/stdio/fiprintf.c`: `fiprintf` |
| `snd_PrintError` | `newlib/libc/stdio/printf.c`: `printf` |

#### Newlib `libc/stdio/vfprintf.c` (origin of `func_00119BC8`)

`func_00119BC8` shares its origin file with `VfprintfInteger`. That file's
notice at the same HEAD `5aa0a116`, and its history since the tag, are quoted
once, in the section "Newlib `libc/stdio/vfprintf.c` (BSD, University of
California)" below.

#### Newlib `libc/stdio/sprintf.c` (origin of `sprintf`)

At HEAD `5aa0a116`, `newlib/libc/stdio/sprintf.c`:

```
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
```

The `newlib-2000-02-17` copy of this notice still carried the advertising
clause (it read "any documentation, advertising materials, and other materials
related to such distribution and use acknowledge that the software was
developed by the University of California, Berkeley"), which the University of
California rescinded in 1999 for all Regents-copyrighted code; Newlib removed
it from this file in the same commit, `9042d0ce65533a26fc3264206db5828d5692332c`
(2020).

#### Newlib `libc/stdio/fiprintf.c` (origin of `func_00115CF0`)

At HEAD `5aa0a116`, `newlib/libc/stdio/fiprintf.c`:

```
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
```

This notice was absent from the file at the `newlib-2000-02-17` tag (commit
`061afe83`), which carries no copyright notice in it at all; Newlib added it
in commit `05b315770a3925648b1a3b52781a75232809198d` (2004-04-23), in the form
with "advertising materials", which commit `9042d0ce6` (2020) removed as for
`sprintf.c`.

#### Newlib `libc/stdio/printf.c` (origin of `snd_PrintError`)

At HEAD `5aa0a116`, `newlib/libc/stdio/printf.c` (byte-identical to the
`fiprintf.c` notice above):

```
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
```

This notice was absent from the file at the `newlib-2000-02-17` tag (commit
`061afe83`); Newlib added it in commit `05b315770` (2004-04-23), and commit
`9042d0ce6` (2020) removed its advertising wording, as for `fiprintf.c`.

### Forja contributors (MIT), carried by OpenRAC rac2-gc-decomp

The C body of `func_00336B48` in `going-decompiled/src/usa/text/235FE8.c` is
taken from OpenRAC's `rac2-gc-decomp` at commit
`96927f872c709e13aaa0f32265bb81a5531bf7ed`, `src/game/00335D30-rc2.c`
(`func_00335D30`, v1.01 address), a file OpenRAC carried over from an RC2 fork
in commit `fe9859d7` and which keeps its own MIT notice. It was adopted on the
operator's instruction (task #2240), adapted (callee spelled and prototyped as
ours, so the call passes our `(p, flag)` on explicitly; our name and
signature). Neither OpenRAC's CREDITS.md nor its notices file names the fork or
a URL for it. The notice, which the doc comment also carries verbatim:

    MIT License

    Copyright (c) 2026 Forja contributors

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.


### RC2 decomp contributions, via OpenRAC rac2-gc-decomp (MIT)

Three C bodies are adapted from OpenRAC's `rac2-gc-decomp`,
<https://github.com/OpenRAC/rac2-gc-decomp> at commit
`96927f872c709e13aaa0f32265bb81a5531bf7ed`, from files that carry their own
MIT notice, not the repository's GPL-3.0. The notice is the same, word for
word, in all three files. It is reproduced verbatim here and in each function's doc
comment (RULINGs #10405 and #12108):

- `CastEmitterOcclusionRay` in `going-decompiled/src/usa/text/1DFF80.cpp`,
  from `src/game/002E4528-rc2.c`, `func_002E4528` (v1.01 address; the file is
  identical at `d43c3b99b984fb63556c844a21ee99e8e31d2deb`). That file was
  carried over from atlasbez-dot/rac2-gc-decomp at `921a4dee` (upstream commit
  `ebf90a4c`), task #2239.
- `StepDialogVoiceChannel` in `going-decompiled/src/usa/text/1B4218.cpp`,
  from `src/game/002B7380-rc2.c`, `func_002B7380` (v1.01 address). That file
  came in with upstream commit `fe9859d7` (Sergio Rodmauser, "carry over 129
  verified RC2 fork functions"), task #2296.
- `func_002984E0` in `going-decompiled/src/usa/text/191238.cpp`,
  from `src/game/00297BB0-rc2.c`, `func_00297BB0` (v1.01 address; adapted:
  callee and data spellings ours, the word after `g_mapBitmapBuffer` for their
  `D_001C4FAC`, the RULING #8620 `section(".data")` device on that extern,
  parameter and row-counter names). That file was carried over by
  atlasbez-dot (upstream commit `bc1dd02f`), task #2299.

    SPDX-License-Identifier: MIT
    MIT License

    Copyright (c) 2026 RC2 decomp contributions

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

### OpenRAC rac2-gc-decomp at 96927f87, cod/022FA8 core bodies (GPL-3.0)

Nine C function bodies are taken from OpenRAC's `rac2-gc-decomp`,
<https://github.com/OpenRAC/rac2-gc-decomp> at commit
`96927f872c709e13aaa0f32265bb81a5531bf7ed` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405), tasks #2231,
#2273 and #2297.
All are in `going-decompiled/src/usa/cod/022FA8.c`:

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_001246E0` (adapted: extern spellings, name at our split address) | `src/core/00124568.c` (from rac1-decomp `src/core/001208E8.c`) | `func_001246D0` |
| `sceCdInit` (adapted: extern spellings, volatile aliases written as casts) | `src/core/00124C98.c` | `func_00124E08` |
| `BuildGsDispEnv` (adapted: extern spellings, our GsGParam, `unsigned long` written `u64`) | `src/core/00125970.c` (from rac1-decomp `src/core/00121D18.c`) | `func_00125A20` |
| `func_0012D4B0` (adapted: `long` written `s64`) | `src/core/0012D100.c` (from rac1-decomp `src/core/00125630.c`) | `func_0012D4B0` |
| `func_0012E378` (adapted: extern spellings, our shared prototype) | `src/core/0012E110.c` (from rac1-decomp `src/core/0012A2F0.c`) | `func_0012E378` |
| `func_001306D0` | `src/core/00130288.c` (from rac1-decomp `src/core/0012AC80.c`) | `func_001306D0` |
| `func_00130A90` (adapted: callee spelling, name at our split address) | `src/core/00130890.c` | `func_00130A8C` |
| `func_0012FBF0` (adapted: callee spellings with casts to our prototypes, our MpegCtx/IpuDecoder fields for their raw offsets; its switch table bound to the ROM's `jtbl_0013BDB0` by `tools/ee/jtbl_bind.sh`), task #2273 | `src/core/0012FB80.c` (from rac1-decomp `src/core/0012AC80.c`) | `func_0012FBF0` |
| `BuildGsDrawEnvPacket` (adapted: callee and struct names, `unsigned long`/`long` written `u64`/`s64`, its inline `sync` behind `#ifndef TARGET_NATIVE` as the RULING #12168 hardware barrier), task #2297 | `nonmatching/core/func_00125F20.c`, a draft upstream moved out of `src/core/00125E58.c` (origin commit `72cfdb68`, moved by `d67ee8c0`; from rac1-decomp `src/core/00122200.c`) | `func_00125F20` (rac1-decomp `func_001222C8`) |

The GPL-2.0-or-later note in the section "OpenRAC rac2-gc-decomp (GPL-3.0)"
above applies to these bodies too.

### OpenRAC rac2-decomp at b2a35115, text/178E88 and text/16E980 Window X bodies (GPL-3.0)

C function bodies are taken from OpenRAC's `rac2-decomp`,
<https://github.com/OpenRAC/rac2-decomp> at commit
`b2a35115e0cf580a1fdab8dd1071b157799e590e` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405/#12108), tasks
#2471, #2472, #2459 and #2503 (task #2503 read its four files at `5776dcc3`;
they are identical at `b2a35115`). Each is adapted, with every respelling named in the function's `Source:`
line.

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_00280EC8` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: our callee spellings func_002859E0 / func_0027B858 / AppendGsRegPacket for their func_00285030 / func_0027B2F8 / func_002FC560, their `D_001A65EE` read as `*(s16 *)(g_gsScreenContext + 0x16E)`, `int`/`float` spelled `s32`/`f32`; s136os arm with one RULING #8549 Ps2EeAs dli row at 0x280F88) | `src/game/0027F128.c` | `func_00280510` (author commit `bf067513e`, bigbeno37) |
| `func_00281020` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: their `extern CopyRegion D_001B8A10[]` and `D_001B8A10[index]` written as the cast view of our `g_sceneActorMobys + 0x44` — 0x1B894C + 0x44 = 0x1B8990, the address the ROM relocates, which their name does not spell; their `CopyRegion` typedef as `CopyRegion_280670`; `int`/`float`/`short` written `s32`/`f32`/`s16`; s136os arm, gcse ON via the per-function `S136EXTRA=` override, RULING #12178), task #2472 | `src/game/0027F128.c` | `func_00280670` (author commit `bf067513e`, bigbeno37) |
| `DrawLensFlare` in `going-decompiled/src/usa/text/16E980.c` (adapted: our data and callee spellings; the flare block read through a struct view of our `g_pLensFlareSource`, `g_cameraPos` through an unaligned view, upstream's `sda` screen-centre array as one RULING #8620 + #12316 `.sdata` extern device spanning our four screen scalars, EE arm only; `long` written `s64` and the native arm reading the true `g_nGsPixelOffsetX`/`g_nGsPixelOffsetY` (RULING #13197); struct fields renamed; tasks #2459, #2510) | `src/game/0026EA00.c` (author commit `d4ac4036`, bigbeno37) | `func_002725E0` |
| `InitScreenGeometry` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: their `D_001A6480` view as `g_gsScreenContextDims` over `g_gsScreenContext`, their `D_001A7340` as the `g_screenExtent` view over `g_screenWidth` with sda respelled `section(".sdata")`, their `D_001B9040` as the cast `(ScreenState *)(g_sceneActorMobys + 0x674)`, their `func_00283CE0` as `IntToFloat`, `int`/`short`/`float` spelled `s32`/`s16`/`f32`; s136os arm at the unit's pinned S136EXTRA) | `src/game/00274128.c` | `func_0027A9F0` (author commit `7dc80be1`, bigbeno37) |
| `func_0027B858` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: their `D_001B9040` as the cast `(ScreenState *)(g_sceneActorMobys + 0x674)`, their `D_001A7340` as the `g_screenExtent` view over `g_screenWidth` with sda respelled `section(".sdata")`, their `func_00283CE0` as `IntToFloat`, their `func_0027AB40` as `BuildCameraProjection`, `int`/`float` spelled `s32`/`f32`; s136os arm at the unit's pinned S136EXTRA) | `src/us2/game/0027B858.c` | `func_US2_0027B858` (author commit `ac7e1a2e`, bigbeno37) |
| `RecomputeScreenViewportFromGsContext` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: their `D_001A6480` as `&g_gsScreenContextDims` over `g_gsScreenContext`, their `D_001A7340` as the `g_screenExtent` view over `g_screenWidth` with sda respelled `section(".sdata")`, their `D_001B9040` as the cast `(ScreenState *)(g_sceneActorMobys + 0x674)`, their `func_00283CE0` as `IntToFloat`, their `func_00279FF0` as `func_0027A550`, `short`/`int`/`float` spelled `s16`/`s32`/`f32`; s136os arm at the unit's pinned S136EXTRA) | `src/us2/game/0027B858.c` | `func_US2_0027B988` (author commit `ac7e1a2e`, bigbeno37) |
| `func_0027DB38` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: their `D_001A7BB1` is stale — the ROM reads `0x1A7BB9` — respelled as the `g_noticeFadeSuppress` view over `D_1A7BB9` with the file-scope `.extern D_1A7BB9, 16` sizing device; their `D_L00_001BCAF4` is stale — the ROM reads `0x1B87F4` — respelled as the `g_noticePulseFrame` `.data` view over `g_sceneFrame`; their `func_00283180` as `func_00283B30` (cosine), `func_00283D38` as `ColorLerpPacked`, `func_00288FB8` as `GetLocalizedString` (`u8 *`), `func_0027F5B8` as `DrawDebugString` prototyped to its definition's five-`s32` shape with a `(s32)text` cast; their int-valued `long` (64-bit on the EE) spelled `s32` (RULING #13282) and `unsigned char *` spelled `u8 *`; s136os arm at the unit's pinned S136EXTRA) | `src/us2/game/0027DB38.c` | `func_US2_0027DB38` (author commit `cde7e4dd`, Flavius) |

### OpenRAC rac2-decomp at b2a35115, cod/022FA8 Window X bodies (GPL-3.0)

One C function body is taken from OpenRAC's `rac2-decomp`,
<https://github.com/OpenRAC/rac2-decomp> at commit
`b2a35115e0cf580a1fdab8dd1071b157799e590e` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405/#12108), task
#2475. It is VERBATIM: no edits, because every spelling the body uses was
already this file's (landed with task #2206's `func_001296A0`).

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_00129DA8` in `going-decompiled/src/usa/cod/022FA8.c` (verbatim, plain sdk29 `-O2 -G0` arm, no devices) | `src/core/00129450.c` | `func_00129DA8` (author commit `bf067513e`, bigbeno37, 2026-11-01) |
| `func_00130428` in `going-decompiled/src/usa/cod/022FA8.c` (adapted: `s32 *` parameter for our prototype, casts to our callee prototypes, a `func_001306D0` prototype ahead of it; upstream's never-taken branch kept verbatim as the RULING #13126 device), task #2476 | `src/core/00130288.c` (from rac1-decomp `src/core/0012AC80.c`), whose earlier draft says it is transcribed from Lombyte (MIT) `src/assembly/sdk/library/sequenceheader.c` | `func_00130428` (rac1-decomp `mpeg_init_seq`, `func_0012C608`; Lombyte `func_0012C4C8`) |

The GPL-2.0-or-later note in the section "OpenRAC rac2-gc-decomp (GPL-3.0)"
above applies to this body too. The body is Lombyte-origin: the upstream file's
notice reads verbatim "func_00129DA8 is Lombyte (MIT)
src/assembly/sdk/library/_getRef0.c", and upstream's removed draft
`nonmatching/core/func_00129DA8.c` names that Lombyte source at commit
`5ccfa1fb314e9cb43f6ff9a43249523e1c4a5310`. The MIT notice reproduced under
"Lombyte (MIT)" below covers it (RULING #12108); the doc comment carries the
notice verbatim.

### OpenRAC rac2-gc-decomp at 96927f87, Lombyte-origin body (GPL-3.0)

One more C function body is taken from OpenRAC's `rac2-gc-decomp` at commit
`96927f872c709e13aaa0f32265bb81a5531bf7ed` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405) and the
operator's 2026-10-31 "adopt all" (RULING #12108), task #2237:

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_0012EB30` in `going-decompiled/src/usa/cod/022FA8.c` (adapted: callee spellings with casts to our prototypes, name at our address) | `src/core/0012EAA0.c` (from rac1-decomp `src/core/0012AC80.c`), whose header says it is adapted from Lombyte (GPL-3.0) `src/sdk/dma/sce_mpeg_demux_pss_ring.c` | `func_0012EB28` (Lombyte `sceMpegDemuxPssRing`) |

Upstream names no Lombyte commit. The Lombyte file exists both at
`2c4452dd` (MIT, notice under "Lombyte (MIT)" below) and after Lombyte's
GPL-3.0 relicence at `a1379f5`. The GPL-2.0-or-later note in the section
"OpenRAC rac2-gc-decomp (GPL-3.0)" above applies to this body too.

### Forja contributors (MIT), via OpenRAC rac2-gc-decomp at 96927f87

Two C function bodies are taken from OpenRAC's `rac2-gc-decomp` at commit
`96927f872c709e13aaa0f32265bb81a5531bf7ed`, from `-rc2.c` files added there by
commit `fe9859d7961703d78264e8475922636440266093` ("carry over 129 verified RC2
fork functions"). Those files carry a fork's MIT notice, not OpenRAC's GPL-3.0;
upstream does not name the fork, its URL or a commit. Adopted with the
operator's 2026-10-31 "adopt all" (RULING #12108):

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_001248B8` in `going-decompiled/src/usa/cod/022FA8.c` (adapted: name at our split address, upstream's `NULL` fallback dropped), task #2237 | `src/core/001248B0-rc2.c` | `func_001248B0` |
| `func_002CC858` in `going-decompiled/src/usa/text/1CA080.cpp`, EE arm (adapted: upstream's computed `goto` through the resident table written as the `switch` it emulates, whose table `tools/ee/jtbl_bind.sh` binds to the ROM's; signature and symbol spellings ours), task #2272 | `src/game/002CB8A0-rc2.c` | `func_002CB8A0` (v1.01 address) |

Both files carry this notice, reproduced verbatim:

    SPDX-License-Identifier: MIT
    MIT License

    Copyright (c) 2026 Forja contributors

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

### Lombyte (MIT)

Seven C function bodies are taken from Lombyte,
<https://github.com/mateuszklysz/Lombyte> at commit
`2c4452dd03f5f7ebb2868dbe073ccdb1afb27f61` (before Lombyte's GPL-3.0 relicence
at `a1379f5`), with the authors' permission (RULING #10405) and the
operator's 2026-10-31 "adopt all" (RULING #12108). Each function's
`Source:` line names every respelling and edit.

| Here | Upstream file | Upstream name |
|---|---|---|
| `_sysbitFlush` in `going-decompiled/src/usa/cod/022FA8.c` (two inert permuter temporaries dropped, task #2134) | `src/sdk/debug/sysbit_flush.c` | `_sysbitFlush` |
| `sceSifInitRpc` in `going-decompiled/src/usa/cod/015180.c` (adapted: our spellings and struct, `(s32)` casts for the unit's prototypes, task #2243) | `src/sdk/rpc/sce_sif_init_rpc.c` | `sceSifInitRpc` |
| `sceSifRebootIop` in `going-decompiled/src/usa/cod/015180.c` (verbatim apart from extern spellings, task #2243) | `src/sdk/rpc/sce_sif_reboot_iop.c` | `sceSifRebootIop` |
| `sceDmaPutEnv` in `going-decompiled/src/usa/cod/022FA8.c` (verbatim apart from extern spellings and the typedef's name, task #2243) | `src/sdk/dma/sce_dma_put_env.c` | `sceDmaPutEnv` |
| `sceDbcInit` in `going-decompiled/src/usa/cod/022FA8.c` (adapted: our spellings, `&` on our struct-typed work area and a `(s32)` cast, task #2243) | `src/sdk/library/sceDbcInit.c` | `sceDbcInit` |
| `func_0012FD60` in `going-decompiled/src/usa/cod/022FA8.c` (adapted: our struct, field and callee names, redundant casts dropped, task #2243) | `src/sdk/library/_decodeOrSkipFrame.c` | `_decodeOrSkipFrame` |
| `func_0012FEC0` in `going-decompiled/src/usa/cod/022FA8.c` (adapted: inert permuter leftovers removed, our names, task #2243) | `src/sdk/library/_decodeOrSkipField.c` | `_decodeOrSkipField` |

They are used under this notice (the repository's root `LICENSE` at that
commit):

    MIT License

    Copyright (c) 2026 Mateusz Kłysz

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

### OpenRAC rac2-decomp at b2a35115, us2 body (GPL-3.0)

One C function body is taken from OpenRAC's `rac2-decomp`,
<https://github.com/OpenRAC/rac2-decomp> at commit
`b2a35115e0cf580a1fdab8dd1071b157799e590e` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405) and the
operator's 2026-10-31 "adopt all" (RULING #12108), task #2460:

| Here | Upstream file | Upstream name |
|---|---|---|
| `BuildCheatMenuItemList` in `going-decompiled/src/usa/text/1CA080.cpp` (adapted: our symbol names per the ROM's relocations, upstream's `sda` attribute as the EE-only `section(".sdata")` extern device (RULING #8620), typedef names ours) | `src/us2/game/002CD510.c`, author commit `c4928ea3` (Flavius) | `func_US2_002CD510` |

The GPL-2.0-or-later note in the section "OpenRAC rac2-gc-decomp (GPL-3.0)"
above applies to this body too.

### Newlib `libc/stdio/vfprintf.c` (BSD, University of California)

`VfprintfInteger` in `going-decompiled/src/usa/cod/015180.c` (taken from
OpenRAC `rac2-decomp`, see "OpenRAC rac2-decomp at b2a35115" above) is the
game's copy of newlib's `_vfiprintf_r`: newlib's `libc/stdio/vfprintf.c`
compiled with `-DINTEGER_ONLY` (newlib's own `libc/stdio/Makefile.am` rule for
`vfiprintf.o`), with Sony's output layer in place of newlib's `__sprint`/uio.
The format scanning, conversion and padding logic derive from that file, so its
notice is reproduced here verbatim in its current form, as published at
newlib-cygwin (<https://sourceware.org/git/newlib-cygwin.git>) commit
`5aa0a116d1a3` (read 2026-11-06), path `newlib/libc/stdio/vfprintf.c`,
RULING #13179, task #2500:

```
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
```

The code itself derives from the older copy at tag `newlib-2000-02-17` (commit
`061afe83dc04a43321d69b5f04860e5de799665c`), the earliest newlib snapshot in the
repository (its `newlib/README` is the newlib-1.8.2 release's), whose notice
still carried a clause 3 requiring advertising materials to acknowledge the
University of California. The University rescinded that advertising clause for
all software copyrighted by its Regents in 1999, and newlib removed it from the
file in commit `9042d0ce65533a26fc3264206db5828d5692332c` (Keith Packard,
2020-01-29, "Use remove-advertising-clause script to edit BSD licenses"). The
notice that applies is therefore the 3-clause form above, compatible with this
project's GPL-2.0-or-later. The head file also carries a 2002 Red Hat notice for
positional-argument support, code that is not in the tag's file and not in this
body.

The same notice covers `func_00119BC8` (newlib's `vfiprintf` wrapper, built
from this file; section "Newlib `libc/stdio`, cod/015180 bodies" above, task
#2518).


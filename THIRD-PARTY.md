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
| `func_00339740` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our callee names and prototypes with `GuiElement *`/`const char *`/`s32` casts; our `D_1ADBAC`/`D_1ADBB0` for their `sda` floats, gp-relative under the unit's -G8; RULING #13082), task #2466 | `src/us2/game/00339740-g8.c` (author commit `22fbd608`, bigbeno37) | `func_US2_00339740` (v2.00 address) |
| `func_L24_0034D598` in `going-decompiled/levels/usa/L24/src/func_L24_0034D598.c`, level program 24 (adapted: our name at the v2.00 place, the boot's `g_anAvailableLevelOrder`, four level symbols renamed to the v2.00 ROM's relocation targets, `volatile` and `sda` dropped as measured inert, `include_asm.h` dropped; the blob is identical at the repository's head `5833be5b716983046be737d85080b96bc9f1435a`), task #2495 | `src/levels/24_ship_shack/L24_0034C450-init-g8.c` | `func_L24_0034C450` (v1.01 address) |
| `VfprintfInteger` in `going-decompiled/src/usa/cod/015180.c` (adapted: our name, `s32` return, `reent` for their `data`; their `PrintFile` view merged into the unit's `PrintfStream`; callees respelled by address to `_mbtowc_r`, `BufferedWrite128Secondary`, `func_00115250` (their `memchr` label), `strlen` and `memcpy`; `(void *)D_00133E74` for the unit's `s32` extern; `const` dropped from the pad-string externs; its switch table bound to the ROM's by `tools/ee/jtbl_bind.sh`; sn2952 arm), task #2465 | `src/core/00119BF8.c` (author commit `bf067513`, bigbeno37) | `func_00119BF8` |
| `PatchShrubPacketTex0` in `going-decompiled/src/usa/text/1DFF80.cpp` (adapted: our symbol names; their {short, short} table entry as our `ShrubTexVramPair` view; `int`/`short`/`unsigned char` spelled `s32`/`s16`/`u8`; an `(s32 *)` cast on `g_shrubClassTable[*list]`; their `nosda` not carried; an unused local dropped; local names ours; s136os arm, gcse ON via the per-function `S136EXTRA=` override, RULING #12178), task #2470 | `src/game/002E10B8.c` (author commit `7fccf735`, Flavius) | `func_002E10B8` (v1.01 address) |
| `func_002DC6C8` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: our symbol names, signature and record types; their struct-typed `D_001C4FA0` as a cast view of our `g_mapVertexData`; `volatile` and `sda` on the list pointer dropped), task #2479 | `src/game/002DB988-init-g8.c` | `func_002DB988` (v1.01 address) |
| `func_00350F28` in `going-decompiled/src/usa/text/250080.cpp` (adapted: our spellings by address, `WaitVblankGetField` / `g_bProgressiveScan` / `D_1AE788` / `D_1AE78C` for their `func_001261F0` / `D_001A7BB8` / `D_001AE7D0` / `D_001AE7D4`; their `extern volatile int D[]` `sda` objects written as volatile stores to our `s32` objects, `sda` not carried; `g_bProgressiveScan` read through an EE-only offset-0 gp equate (RULING #9073 under #8620); the loop written to break on progressive scan so the unit's RULING #8435 short-loop pad sits before its branch; `void` as upstream, `int` spelled `s32`; s136os arm at the unit's S136EXTRA ""; read at `5776dcc3`, identical at `b2a35115`), task #2542 | `src/game/0034CEA0.c` (author commit `1c58727d`, Louis-Philippe Le Sieur) | `func_003501E0` |
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
| `func_00339F98` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our callee spellings and declarations; their `D_L00_001BD2C0` array as the EE-only equate `g_cameraProjScratch = g_sceneActorMobys + 0x674`), task #2469 | `src/game/00338F58.c` | `func_00338F58` (v1.01 address) |
| `func_00341A80` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol names, signature and prototypes; their `MenuState`/`MenuEntry` as our `GuiWeaponGridCursor`/`GuiWeaponGridEntry`; their typed table as an EE-only asm-label view of `D_259CC0`), task #2469 | `src/game/0033F030.c` | `func_00340B70` (v1.01 address) |
| `func_00342520` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol names and declarations, `GuiElement *` casts; their `sda` scalar `D_001AE0C8` as our `D_1AE0A0[idx]`, `s32[2]` on EE and `s32[3]` on native; `char[]` as `u8[]`), task #2469 | `src/us2/game/003424C8-g8.c` | `func_US2_00342520` (v2.00 address) |
| `UpdateSubtitleStateMachine` in `going-decompiled/src/usa/text/188858.c` (adapted: every global respelled by address to ours — `D_00254E78` → our `g_subtitleState` struct and field names, `D_001B1840` → `g_pActiveTextTable`, `D_0018C958` → the `g_subtitleHistory` rows, `D_001A63A8` → `g_fileLoadVoiceState` fields / `g_dialogVoiceId`, `D_0014B540` → `g_discToc`, `D_001A7B94` → `D_1A7B9C`, `D_001A79F0` → `g_playerProgress`, `D_001A8BB0` → `g_nGameState`, `D_00138320`/`D_00138324` → `D_138320`/`D_138324`, `D_001A7390` → `D_1A7390`; callee auto-labels to our names (`TickCountdownTimer`, `BeginSubtitleDisplay`, `func_0028AA70`, `func_002B1B48`, `StopDialogVoice`, `FindTextTableEntry`); the playing id respelled per site as the ROM has it; case order left as upstream; the audit's z2 addressing devices minimised by removal probes (RULING #8620 `.sdata`/`.data` typed views, #8036/#12316 alias externs); the state-8 latch increment's temp register pinned to `$2` by an EE-only asm register local (`SUBTITLES_LATCH_INC`; the native arm spells the plain `++`); the `RECORD_SUBTITLE_TIME` macro's opening brace moved off the `#define` line so the line-based ORPHAN_LATENT scanner's brace count stays balanced; the switch table bound to `jtbl_0026C7A0_text` by `tools/ee/jtbl_bind.sh`; s136os arm, `S136EXTRA` empty) | `src/game/00289178.c` | `func_00289178` (v1.01 address, author commit `6a0dbf0b`, bigbeno37, 2026-10-28) |
| `SetupMemoryArenaTable` in `going-decompiled/src/usa/text/191238.cpp` (adapted: our symbol names; their end-of-text `+ 0x3FFF` and their top-of-RAM `- 0x8000` / `- 0x4000` read as the build's absolute names `D_356D07` / `D_1FF8000` / `D_1FFC000` on EE, numbers on native; their `sda` cursor as an EE-only `section(".sdata")` asm-label view, RULINGs #8620/#13168; `int`/`unsigned int` spelled `s32`/`u32`; the blob is identical at `5776dcc3` and at the head `d36c267a`), task #2507 | `src/game/00290A40.c` (author commit `7dc80be1`, bigbeno37) | `func_00290B48` (v1.01 address) |
| `MenuScreenDoAction` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: our symbol names per the ROM's relocations; their `Func002D5B58Root` as our `MenuScreenActionState`; the third parameter as this unit's existing `void *outFlag` prototype; `PlayGlobalSound`'s file-scope prototype corrected to `s32`; RULING #12316 `.extern ,16` sizing for `g_gameStateFlags` (`g_currentLanguage`'s directive pre-existing); declaration-only asm-label aliases with offset views for two far data refs; jump-table name bound to `jtbl_0026CD40_text`; s136os selector row), task #2502 | `src/game/002D5540.c` (the file is identical at `5776dcc35045fb2320610edd1d2db4fa7660ef37`, the snapshot fetched for this adoption; the body's author commit is `08722f0c`, bigmak94) | `func_002D5B58` (v1.01 address) |
| `SoundOptionsMenuInput` in `going-decompiled/src/usa/text/1D54C0.cpp` (adapted: our symbol names per the ROM's relocations; upstream's `Settings2D7B18` settings block respelled by address as u8-cast offset views over `g_audioStereoMode` (+4 `g_sfxVolume`, +8 `g_musicVolume`); two `volatile` accesses of our own (not upstream's) for the ROM's merge asymmetry and the X-toggle shape, CODEGEN DEVICES under RULINGs #8404 / #12316 Q2 with a writer census; RULING #8620-class file-scope `.extern g_audioStereoMode, 16` sizing (one line of func_002D8A68's existing `.extern ,16` block, moved above this arm; the block's `g_musicVolume`/`g_sfxVolume` lines are not load-bearing for this body); declaration-only asm-label array alias for `g_pTextTableLoadBuf+0x1C` (RULING #13168); the mix callee spelled under its current in-tree name `func_002E5698`; s136os selector row), task #2502 | `src/game/002D5E98.c` (the file is identical at `5776dcc35045fb2320610edd1d2db4fa7660ef37`, the snapshot fetched for this adoption; the body's author commit is `5ae008de`, bigbeno37) | `func_002D7B18` (v1.01 address) |
| `SaveLoadStateMachine` in `going-decompiled/src/usa/text/198FA0.cpp` (adapted: every global and callee respelled by address to the ROM's relocation targets, named in the `Source:` line; their card-state block, disc-TOC record and directory-entry buffer as EE-only typed views of `g_areaTable`, `g_discToc` and `g_mapTextureWidth + 0xA0` (RULING #13168); `D_14B884` and `g_fileLoadState` with `section(".data")` (RULING #8620); their `D_001A7A00[8]` as an EE-only `.sdata` view `D_1A7A28Base` equated to `D_1A7A28 - 0x20` and sized 16 (FACT #8036 form); their `void` / `int` prototypes for `DebugPrintStub` / `func_00289398` kept through EE-only asm-label declarations; `(char *)` casts on the unit's `u8[]` string buffers; their `sda` dropped; s136os arm as the unit is; six switch tables bound to the ROM's by `tools/ee/jtbl_bind.sh`; blob identical at `5776dcc3` and `71af8fdc`), task #2551 | `src/us2/game/00299CF0.c` (author commit `3b7195d7`, Flavius) | `func_US2_00299CF0` (v2.00 address) |

The four `cod/015180` files are byte-identical between `b2a35115` and
`8b26d8adeb0b5515a962f83230df4bf909fd0cf5`, the commit task #2497 read.

`VfprintfInteger`'s upstream file is named by neither upstream's
`THIRD_PARTY_NOTICES.md` nor its `CREDITS.md`; its header says it was
reconstructed from retail control flow and the newlib 1999 implementation in
rac1-decomp `src/core/00116FA0.c`.

The GPL-2.0-or-later note in the section "OpenRAC rac2-gc-decomp (GPL-3.0)" above applies to these bodies too.

### OpenRAC rac2-decomp at fe6adf9b, level programs

C function bodies of the USA v2.00 level programs are taken from OpenRAC's
`rac2-decomp`, <https://github.com/OpenRAC/rac2-decomp> at commit
`fe6adf9baddcd31323c35adf102d078aa250852d` (licence per row: GPL-3.0 (root
LICENSE) unless the row names a file's own MIT notice), with the authors'
permission (RULINGs #10405, #12108, #13082 and #13291). Each is adapted, with
every respelling named in the function's `Source:` line.

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_L24_0037AE10` in `going-decompiled/levels/usa/L24/src/func_L24_0037AE10.c`, level program 24 (adapted: our name; their `sda`/`nosda` as `section(".sdata")`/`section(".data")` (RULING #8620); the boot's `g_soundBankHandlesBlk` for `D_00189E20`; two level data names and eight callees renamed to the v2.00 disc's own targets, L24's copies of boot code; assembled through `tools/ee/asm_unit.sh`), task #2552; GPL-3.0, no notice in the file, no Lombyte mention | `src/us2/levels/24_ship_shack/L24_0037AE10-g8.c` (author commit `c4928ea3`, Flavius) | `func_US2_L24_0037AE10` (v2.00 address) |
| `func_L11_003417D0` in `going-decompiled/levels/usa/L11/src/func_L11_003417D0.c`, level program 11 (verbatim but for names: our name at the v2.00 place; callees func_00282CF0 and func_L02_00333480 as L11's copies func_L11_00301210 and func_L11_003415E8), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_002DE9E0.c` (author commit `c7d9e30a`, bigbeno37) | `func_L11_00340500` (v1.01 address) |
| `func_L11_0036A7F8` in `going-decompiled/levels/usa/L11/src/func_L11_0036A7F8.c`, level program 11 (verbatim but for names: our name at the v2.00 place; the boot's `g_anAvailableLevelOrder`; five level data names renamed to the v2.00 ROM's relocation targets, three of them address-named inside larger labels (RULING #13291 §2); `include_asm.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00369700-init-g8.c` (author commit `a96d3126`, Louis-Philippe Le Sieur) | `func_L11_00369700` (v1.01 address) |
| `func_L11_003F4310` in `going-decompiled/levels/usa/L11/src/func_L11_003F4310.c`, level program 11 (adapted: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); `common.h` dropped, its typedefs written out), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_003D6270.c` (author commit `ee3fee45`, llesieur99) | `func_L11_003F2E00` (v1.01 address) |
| `func_L11_00414290` in `going-decompiled/levels/usa/L11/src/func_L11_00414290.c`, level program 11 (adapted: our name at the v2.00 place; `sda`/`NOT_SDA` as `section(".sdata")`/`section(".data")` (RULING #8620); the boot's `g_heroPos`; four callees as L11's copies; two data names renamed to the v2.00 ROM's relocation targets, address-named (RULING #13291 §2); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00412B20-g8.c` (author commit `c7d9e30a`, bigbeno37) | `func_L11_00412B20` (v1.01 address) |
| `func_L11_00427310` in `going-decompiled/levels/usa/L11/src/func_L11_00427310.c`, level program 11 (adapted: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); two data names renamed to the v2.00 ROM's relocation targets, address-named (RULING #13291 §2); `include_asm.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00420C88.c` (author commit `ba383701`, bigbeno37) | `func_L11_00425AF8` (v1.01 address) |
| `func_L11_0042BCF8` in `going-decompiled/levels/usa/L11/src/func_L11_0042BCF8.c`, level program 11 (adapted: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); two data names renamed to the v2.00 ROM's relocation targets, address-named (RULING #13291 §2); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00425F10.c` (author commit `ba383701`, bigbeno37) | `func_L11_0042A4C8` (v1.01 address) |
| `func_L11_00439600` in `going-decompiled/levels/usa/L11/src/func_L11_00439600.c`, level program 11 (adapted: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); the record base renamed to the v2.00 target, address-named (RULING #13291 §2); one callee as L11's copy; `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00432628.c` (author commit `fa8d6672`, llesieur99) | `func_L11_00437DC0` (v1.01 address) |
| `func_L11_004396A8` in `going-decompiled/levels/usa/L11/src/func_L11_004396A8.c`, level program 11 (adapted: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); the record base renamed to the v2.00 target, address-named (RULING #13291 §2); one callee as L11's copy; `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00432628.c` (author commit `6b943916`, Louis-Philippe Le Sieur) | `func_L11_00437E68` (v1.01 address) |
| `func_L11_0043B430` in `going-decompiled/levels/usa/L11/src/func_L11_0043B430.c`, level program 11 (adapted: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); the boot's `g_listenerPosHistory`; the record base renamed to the v2.00 target, address-named (RULING #13291 §2); three callees as L11's copies; `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00432628.c` (author commit `431ac503`, llesieur99) | `func_L11_00439BF0` (v1.01 address) |
| `func_L11_00442A18` in `going-decompiled/levels/usa/L11/src/func_L11_00442A18.c`, level program 11 (adapted: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); their asm-label view `D_L11_001B359C_base` written as the one direct declaration `D_L11_001B361C` (no asm label); two data names renamed to the v2.00 targets, address-named (RULING #13291 §2); nine handlers renamed to their v2.00 places; `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_0043E1A0.c` (author commit `f6353318`, DrLegitamate) | `func_L11_004411C8` (v1.01 address) |
| `func_L11_00443038` in `going-decompiled/levels/usa/L11/src/func_L11_00443038.c`, level program 11 (adapted: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620); the table base renamed to the v2.00 target, address-named (RULING #13291 §2); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_0043E1A0.c` (author commit `264cb2be`, llesieur99) | `func_L11_004417E8` (v1.01 address) |
| `func_L11_00443E18` in `going-decompiled/levels/usa/L11/src/func_L11_00443E18.c`, level program 11 (adapted: our name at the v2.00 place; `sda` as `section(".sdata")` (RULING #8620) on two externs; the boot's `g_heroPos`; two data names renamed to the v2.00 targets, address-named (RULING #13291 §2); ten callees as L11's own functions or copies), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_004425C8.c` (author commit `3cd8af2d`, bigbeno37) | `func_L11_004425C8` (v1.01 address) |
| `func_L11_00456538` in `going-decompiled/levels/usa/L11/src/func_L11_00456538.c`, level program 11 (adapted: our name at the v2.00 place; two callees as L11's copies; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `91e04732`, llesieur99) | `func_L11_00454AE0` (v1.01 address) |
| `func_L11_004565B8` in `going-decompiled/levels/usa/L11/src/func_L11_004565B8.c`, level program 11 (adapted: our name at the v2.00 place; three callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `5964c620`, Flavius) | `func_L11_00454B60` (v1.01 address) |
| `func_L11_004566D0` in `going-decompiled/levels/usa/L11/src/func_L11_004566D0.c`, level program 11 (adapted: our name at the v2.00 place; six callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `daf0e4a1`, Flavius) | `func_L11_00454C78` (v1.01 address) |
| `func_L11_00456830` in `going-decompiled/levels/usa/L11/src/func_L11_00456830.c`, level program 11 (adapted: our name at the v2.00 place; the boot's `g_listenerPosHistory`; seven callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `daf0e4a1`, Flavius) | `func_L11_00454DD8` (v1.01 address) |
| `func_L11_00457A68` in `going-decompiled/levels/usa/L11/src/func_L11_00457A68.c`, level program 11 (adapted: our name at the v2.00 place; one callee as L11's copy; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `91e04732`, llesieur99) | `func_L11_00456010` (v1.01 address) |
| `func_L11_00457DD0` in `going-decompiled/levels/usa/L11/src/func_L11_00457DD0.c`, level program 11 (adapted: our name at the v2.00 place; two callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `5964c620`, Flavius) | `func_L11_00456378` (v1.01 address) |
| `func_L11_00457F98` in `going-decompiled/levels/usa/L11/src/func_L11_00457F98.c`, level program 11 (adapted: our name at the v2.00 place; four callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `431ac503`, llesieur99) | `func_L11_00456540` (v1.01 address) |
| `func_L11_00458678` in `going-decompiled/levels/usa/L11/src/func_L11_00458678.c`, level program 11 (adapted: our name at the v2.00 place; twelve callees renamed to their v2.00 targets; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `3cd8af2d`, bigbeno37) | `func_L11_00456C20` (v1.01 address) |
| `func_L11_0045A840` in `going-decompiled/levels/usa/L11/src/func_L11_0045A840.c`, level program 11 (adapted: our name at the v2.00 place; one callee as L11's copy; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `7fccf735`, Flavius) | `func_L11_00458D88` (v1.01 address) |
| `func_L11_0045AA10` in `going-decompiled/levels/usa/L11/src/func_L11_0045AA10.c`, level program 11 (adapted: our name at the v2.00 place; one callee as L11's copy; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `91e04732`, llesieur99) | `func_L11_00458F58` (v1.01 address) |
| `func_L11_00461AD8` in `going-decompiled/levels/usa/L11/src/func_L11_00461AD8.c`, level program 11 (adapted: our name at the v2.00 place; one callee as L11's copy; the body otherwise verbatim, and the audit's `long long` respelling not needed (no `long long` in this function); `common.h` dropped), task #2553; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358.c` (author commit `7fccf735`, Flavius) | `func_L11_00460038` (v1.01 address) |
| `func_L02_003DCEB8` in `going-decompiled/levels/usa/L02/src/func_L02_003DCEB8.c`, level program 02 (verbatim but for names: our name at the v2.00 place; no other symbol; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `773eafc2`, llesieur99) | `func_L02_003DB950` (v1.01 address) |
| `func_L02_003E8B18` in `going-decompiled/levels/usa/L02/src/func_L02_003E8B18.c`, level program 02 (verbatim but for names: our name at the v2.00 place; four callees as L02's copies; the track table D_L02_002A53B0 as the v2.00 label D_L02_002A5470; `u16` written out; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `b7374531`, bigmak94) | `func_L02_003E75A8` (v1.01 address) |
| `func_L02_003EB9F8` in `going-decompiled/levels/usa/L02/src/func_L02_003EB9F8.c`, level program 02 (verbatim but for names: our name at the v2.00 place; callee func_31E6A0 as L02's copy func_L02_0031F998; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `ee3fee45`, llesieur99) | `func_L02_003EA488` (v1.01 address) |
| `func_L02_003F01C8` in `going-decompiled/levels/usa/L02/src/func_L02_003F01C8.c`, level program 02 (verbatim but for names: our name at the v2.00 place; the boot's `g_heroPos`; one callee as L02's copy; `D_L02_001FF1C0` kept, address-named (RULING #13291 §2); the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `eef580e5`, Flavius) | `func_L02_003EEC58` (v1.01 address) |
| `func_L02_003F0528` in `going-decompiled/levels/usa/L02/src/func_L02_003F0528.c`, level program 02 (verbatim but for names: our name at the v2.00 place; the boot's `g_heroPos`; two callees at their v2.00 places; `D_L02_001FF1C0` kept, address-named (RULING #13291 §2); the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `246b78db`, bigmak94) | `func_L02_003EEFB8` (v1.01 address) |
| `func_L02_003F2AE8` in `going-decompiled/levels/usa/L02/src/func_L02_003F2AE8.c`, level program 02 (verbatim but for names: our name at the v2.00 place; two callees as L02's copies; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `adcbd51e`, llesieur99) | `func_L02_003F1578` (v1.01 address) |
| `func_L02_003F2C60` in `going-decompiled/levels/usa/L02/src/func_L02_003F2C60.c`, level program 02 (verbatim but for names: our name at the v2.00 place; two callees at their v2.00 places; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `e80a585a`, Flavius) | `func_L02_003F16F0` (v1.01 address) |
| `func_L02_003F2D20` in `going-decompiled/levels/usa/L02/src/func_L02_003F2D20.c`, level program 02 (verbatim but for names: our name at the v2.00 place; three callees as L02's copies; `u8`/`u16` written out; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `e80a585a`, Flavius) | `func_L02_003F17B0` (v1.01 address) |
| `func_L02_003F2E58` in `going-decompiled/levels/usa/L02/src/func_L02_003F2E58.c`, level program 02 (verbatim but for names: our name at the v2.00 place; nine callees at their v2.00 places; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `ee3fee45`, llesieur99) | `func_L02_003F18E8` (v1.01 address) |
| `func_L02_003F30E0` in `going-decompiled/levels/usa/L02/src/func_L02_003F30E0.c`, level program 02 (verbatim but for names: our name at the v2.00 place; one callee as L02's copy; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `773eafc2`, llesieur99) | `func_L02_003F1B70` (v1.01 address) |
| `func_L02_003FC4C8` in `going-decompiled/levels/usa/L02/src/func_L02_003FC4C8.c`, level program 02 (verbatim but for names: our name at the v2.00 place; three callees as L02's copies; the callback D_L02_004004D8 as its v2.00 glabel func_L02_00401C20 (upstream's `char []` type kept); the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `ee3fee45`, llesieur99) | `func_L02_003FAF28` (v1.01 address) |
| `func_L02_003FCC60` in `going-decompiled/levels/usa/L02/src/func_L02_003FCC60.c`, level program 02 (verbatim but for names: our name at the v2.00 place; two callees at their v2.00 places; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `455cde53`, Flavius) | `func_L02_003FB6C0` (v1.01 address) |
| `func_L02_003FCFF8` in `going-decompiled/levels/usa/L02/src/func_L02_003FCFF8.c`, level program 02 (verbatim but for names: our name at the v2.00 place; two callees at their v2.00 places; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `adcbd51e`, llesieur99) | `func_L02_003FBA58` (v1.01 address) |
| `func_L02_003FD5C0` in `going-decompiled/levels/usa/L02/src/func_L02_003FD5C0.c`, level program 02 (verbatim but for names: our name at the v2.00 place; one callee at its v2.00 place; `D_L02_001BA3C0` kept, address-named (RULING #13291 §2); the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `773eafc2`, llesieur99) | `func_L02_003FC020` (v1.01 address) |
| `func_L02_003FD658` in `going-decompiled/levels/usa/L02/src/func_L02_003FD658.c`, level program 02 (verbatim but for names: our name at the v2.00 place; no other symbol; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `adcbd51e`, llesieur99) | `func_L02_003FC0B8` (v1.01 address) |
| `func_L02_003FD670` in `going-decompiled/levels/usa/L02/src/func_L02_003FD670.c`, level program 02 (verbatim but for names: our name at the v2.00 place; no other symbol; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `455cde53`, Flavius) | `func_L02_003FC0D0` (v1.01 address) |
| `func_L02_00401D98` in `going-decompiled/levels/usa/L02/src/func_L02_00401D98.c`, level program 02 (verbatim but for names: our name at the v2.00 place; the boot's `g_fileLoadVoiceState`; the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `ee3fee45`, llesieur99) | `func_L02_00400650` (v1.01 address) |
| `func_L02_00402410` in `going-decompiled/levels/usa/L02/src/func_L02_00402410.c`, level program 02 (verbatim but for names: our name at the v2.00 place; `D_0017A80` as the v2.00 relocation target `D_L02_001CAA80`, address-named (RULING #13291 §2); the file's includes, other functions and their externs not carried), task #2559; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `597e1ea6`, ReaperOfVeriod) | `func_L02_00400CC8` (v1.01 address) |
| `func_L11_0043A758` in `going-decompiled/levels/usa/L11/src/func_L11_0043A758.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `D_L11_001FF580` as the address-named `D_L11_001FF600`, two boot-copy callees by their L11 places, `common.h` dropped, its u8/u16/s16 typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_00432628.c` (author commit `b4b0fe9b`, bigbeno37) | `func_L11_00438F18` (v1.01 address) |
| `func_L11_0043B5A0` in `going-decompiled/levels/usa/L11/src/func_L11_0043B5A0.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `common.h` dropped), task #2555 | `src/levels/11_joba/L11_00432628.c` (author commit `fa8d6672`, llesieur99) | `func_L11_00439D60` (v1.01 address) |
| `func_L11_0043BE60` in `going-decompiled/levels/usa/L11/src/func_L11_0043BE60.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `func_32E480` as `func_L11_0032F698`, `common.h` dropped), task #2555 | `src/levels/11_joba/L11_00432628.c` (author commit `fa8d6672`, llesieur99) | `func_L11_0043A620` (v1.01 address) |
| `func_L11_0043C040` in `going-decompiled/levels/usa/L11/src/func_L11_0043C040.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `common.h` dropped), task #2555 | `src/levels/11_joba/L11_00432628.c` (author commit `fa8d6672`, llesieur99) | `func_L11_0043A800` (v1.01 address) |
| `func_L11_0043C368` in `going-decompiled/levels/usa/L11/src/func_L11_0043C368.c`, level program 11 (verbatim but for names: our name at the v2.00 place, ten boot-copy callees by their L11 places, `common.h` dropped), task #2555 | `src/levels/11_joba/L11_00432628.c` (author commit `5acf9869`, Flavius) | `func_L11_0043AB28` (v1.01 address) |
| `func_L11_004419B0` in `going-decompiled/levels/usa/L11/src/func_L11_004419B0.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `D_L11_004465F8` as our label `func_L11_00448020` (type kept), three boot-copy callees by their L11 places, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `264cb2be`, llesieur99) | `func_L11_00440160` (v1.01 address) |
| `func_L11_004420B8` in `going-decompiled/levels/usa/L11/src/func_L11_004420B8.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `D_L11_001FC420`/`D_L11_001B359C` as the address-named `D_L11_001FC4A0`/`D_L11_001B361C` (upstream's own `section(".sdata")` kept), two level callees by their v2.00 places, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `96f56db3`, bigbeno37) | `func_L11_00440868` (v1.01 address) |
| `func_L11_004424D0` in `going-decompiled/levels/usa/L11/src/func_L11_004424D0.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `431ac503`, llesieur99) | `func_L11_00440C80` (v1.01 address) |
| `func_L11_004424E0` in `going-decompiled/levels/usa/L11/src/func_L11_004424E0.c`, level program 11 (verbatim but for names: our name at the v2.00 place, their three `func_L02_…` callees as L11's own copies at the v2.00 places, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `c7d9e30a`, bigbeno37) | `func_L11_00440C90` (v1.01 address) |
| `func_L11_004428E0` in `going-decompiled/levels/usa/L11/src/func_L11_004428E0.c`, level program 11 (verbatim but for names: our name at the v2.00 place, two callees by their L11 places, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `ee3fee45`, llesieur99) | `func_L11_00441090` (v1.01 address) |
| `func_L11_00442FF8` in `going-decompiled/levels/usa/L11/src/func_L11_00442FF8.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `D_L11_001BA780` as the address-named `D_L11_001BA800`, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `264cb2be`, llesieur99) | `func_L11_004417A8` (v1.01 address) |
| `func_L11_00443058` in `going-decompiled/levels/usa/L11/src/func_L11_00443058.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `D_L11_001FF580` as the address-named `D_L11_001FF600`, `func_002A8AF0` as `func_L11_0032F328`, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `7fccf735`, Flavius) | `func_L11_00441808` (v1.01 address) |
| `func_L11_004430A8` in `going-decompiled/levels/usa/L11/src/func_L11_004430A8.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `431ac503`, llesieur99) | `func_L11_00441858` (v1.01 address) |
| `func_L11_004430B8` in `going-decompiled/levels/usa/L11/src/func_L11_004430B8.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `7fccf735`, Flavius) | `func_L11_00441868` (v1.01 address) |
| `func_L11_004431C8` in `going-decompiled/levels/usa/L11/src/func_L11_004431C8.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `7fccf735`, Flavius) | `func_L11_00441978` (v1.01 address) |
| `func_L11_00447750` in `going-decompiled/levels/usa/L11/src/func_L11_00447750.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `D_001A63A8` as the boot's `g_fileLoadVoiceState`, `include_asm.h`/`common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `4541c4ce`, ReaperOfVeriod) | `func_L11_00445D28` (v1.01 address) |
| `func_L11_00447BE8` in `going-decompiled/levels/usa/L11/src/func_L11_00447BE8.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `func_L00_003206B8` as L11's own copy `func_L11_00333F00`, `include_asm.h`/`common.h` dropped, typedef written in the TU), task #2555 | `src/levels/11_joba/L11_0043E1A0.c` (author commit `6b943916`, Louis-Philippe Le Sieur) | `func_L11_004461C0` (v1.01 address) |
| `func_L11_00441A88` in `going-decompiled/levels/usa/L11/src/func_L11_00441A88.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `D_L11_001B359C`/`D_L11_001FC420` as the address-named `D_L11_001B361C`/`D_L11_001FC4A0`, `D_00189E20` as the boot's `g_soundBankHandlesBlk` (upstream's `sda`/`nosda` text kept, ignored by cc1), `include_asm.h` dropped), task #2555 | `src/levels/11_joba/L11_00440238-g8.c` (author commit `68795b5b`, Gianni) | `func_L11_00440238` (v1.01 address) |
| `func_L11_00441FE0` in `going-decompiled/levels/usa/L11/src/func_L11_00441FE0.c`, level program 11 (verbatim but for names: our name at the v2.00 place, the two address-named bank externs as above, two level callees by their v2.00 places (one is our C `func_L11_004424E0`), `include_asm.h` dropped), task #2555 | `src/levels/11_joba/L11_00440238-g8.c` (author commit `68795b5b`, Gianni) | `func_L11_00440790` (v1.01 address) |
| `func_L11_00447A48` in `going-decompiled/levels/usa/L11/src/func_L11_00447A48.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `D_00189E20` as `g_soundBankHandlesBlk`, `D_L11_001B1870` as our label `D_L11_001B18C8`, two callees by their v2.00 places (one is our C `func_L11_00447750`), `common.h` dropped), task #2555 | `src/levels/11_joba/L11_00446020-g8.c` (author commit `3cd8af2d`, bigbeno37) | `func_L11_00446020` (v1.01 address) |
| `func_L11_0044A750` in `going-decompiled/levels/usa/L11/src/func_L11_0044A750.c`, level program 11 (verbatim but for names: our name at the v2.00 place, the two address-named bank externs `D_L11_001FC4A0`/`D_L11_001B361C`, `common.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_004465F8-g8.c` (author commit `352d5dad`, Louis-Philippe Le Sieur) | `func_L11_00448D28` (v1.01 address) |
| `func_L11_0044A270` in `going-decompiled/levels/usa/L11/src/func_L11_0044A270.c`, level program 11 (adapted: our name at the v2.00 place, four boot-copy callees by their L11 places, the `(long)` pointer cast written `(s64)` with `typedef long s64` (RULING #13197; the same type), `common.h`/`include_asm.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_004465F8.c` (author commit `fa8d6672`, llesieur99) | `func_L11_00448848` (v1.01 address) |
| `func_L11_0044A330` in `going-decompiled/levels/usa/L11/src/func_L11_0044A330.c`, level program 11 (verbatim but for names: our name at the v2.00 place, `func_32C9E0`/`func_32C700` as `func_L11_0032DBF8`/`func_L11_0032D918`, `common.h`/`include_asm.h` dropped, typedefs written in the TU), task #2555 | `src/levels/11_joba/L11_004465F8.c` (author commit `fa8d6672`, llesieur99) | `func_L11_00448908` (v1.01 address) |
| `func_L14_0032A188` in `going-decompiled/levels/usa/L14/src/func_L14_0032A188.c`, level program 14 (verbatim but for names: our name at the v2.00 place; three callees as L14's copies; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190.c` (author commit `db93b56d`, llesieur99) | `func_L14_00328E90` (v1.01 address) |
| `func_L14_00392440` in `going-decompiled/levels/usa/L14/src/func_L14_00392440.c`, level program 14 (verbatim but for names: our name at the v2.00 place; one callee at its v2.00 place; `D_L14_00237B00` as the v2.00 relocation target `D_L14_00237A80`, address-named (RULING #13291 §2); the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190.c` (author commit `adcbd51e`, llesieur99) | `func_L14_00390FE8` (v1.01 address) |
| `func_L14_00392FB0` in `going-decompiled/levels/usa/L14/src/func_L14_00392FB0.c`, level program 14 (verbatim but for names: our name at the v2.00 place; two callees as L14's copies; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190.c` (author commit `6b943916`, Louis-Philippe Le Sieur) | `func_L14_00391B58` (v1.01 address) |
| `func_L14_003CAA18` in `going-decompiled/levels/usa/L14/src/func_L14_003CAA18.c`, level program 14 (verbatim but for names: our name at the v2.00 place; no other symbol; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190.c` (author commit `db93b56d`, llesieur99) | `func_L14_003C9548` (v1.01 address) |
| `func_L14_003DF838` in `going-decompiled/levels/usa/L14/src/func_L14_003DF838.c`, level program 14 (verbatim but for names: our name at the v2.00 place; six callees as L14's copies; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190.c` (author commit `913bd1bc`, Gianni) | `func_L14_003DE360` (v1.01 address) |
| `func_L14_003DF960` in `going-decompiled/levels/usa/L14/src/func_L14_003DF960.c`, level program 14 (verbatim but for names: our name at the v2.00 place; one callee as L14's copy; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190.c` (author commit `db93b56d`, llesieur99) | `func_L14_003DE488` (v1.01 address) |
| `func_L14_003DFDC0` in `going-decompiled/levels/usa/L14/src/func_L14_003DFDC0.c`, level program 14 (verbatim but for names: our name at the v2.00 place; no other symbol; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190.c` (author commit `db93b56d`, llesieur99) | `func_L14_003DE8E8` (v1.01 address) |
| `func_L14_003DFDD0` in `going-decompiled/levels/usa/L14/src/func_L14_003DFDD0.c`, level program 14 (verbatim but for names: our name at the v2.00 place; no other symbol; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190.c` (author commit `db93b56d`, llesieur99) | `func_L14_003DE8F8` (v1.01 address) |
| `func_L14_003DFE68` in `going-decompiled/levels/usa/L14/src/func_L14_003DFE68.c`, level program 14 (verbatim but for names: our name at the v2.00 place; one callee as L14's copy; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190.c` (author commit `db93b56d`, llesieur99) | `func_L14_003DE990` (v1.01 address) |
| `func_L14_003E0618` in `going-decompiled/levels/usa/L14/src/func_L14_003E0618.c`, level program 14 (verbatim but for names: our name at the v2.00 place; three callees as L14's copies / v2.00 places; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190-g8.c` (author commit `ee3fee45`, llesieur99) | `func_L14_003DF100` (v1.01 address) |
| `func_L14_003E0B68` in `going-decompiled/levels/usa/L14/src/func_L14_003E0B68.c`, level program 14 (verbatim but for names: our name at the v2.00 place; two callees at their v2.00 places; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_00304190.c` (author commit `db93b56d`, llesieur99) | `func_L14_003DF650` (v1.01 address) |
| `func_L14_00415288` in `going-decompiled/levels/usa/L14/src/func_L14_00415288.c`, level program 14 (verbatim but for names: our name at the v2.00 place; four callees as L14's copies; `D_L14_001BF980` as the v2.00 relocation target `D_L14_001BF900`, address-named (RULING #13291 §2); the audit's `long long`->`long` respelling touches no line carried; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_003F4F98.c` (author commit `7fccf735`, Flavius) | `func_L14_00413CD8` (v1.01 address) |
| `func_L14_004220D0` in `going-decompiled/levels/usa/L14/src/func_L14_004220D0.c`, level program 14 (adapted: our name at the v2.00 place; two callees as L14's copies; `D_L14_001B259C` as the v2.00 relocation target `D_L14_001B251C`, address-named (RULING #13291 §2), its `sda` attribute written `section(".sdata")` (RULING #8620); the audit's `long long`->`long` respelling touches no line carried; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_003F4F98.c` (author commit `4912fc03`, bigbeno37) | `func_L14_00420B10` (v1.01 address) |
| `func_L14_00424560` in `going-decompiled/levels/usa/L14/src/func_L14_00424560.c`, level program 14 (verbatim but for names: our name at the v2.00 place; five callees at their v2.00 places; the audit's `long long`->`long` respelling touches no line carried; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_003F4F98.c` (author commit `91e04732`, llesieur99) | `func_L14_00422FA0` (v1.01 address) |
| `func_L14_00425380` in `going-decompiled/levels/usa/L14/src/func_L14_00425380.c`, level program 14 (verbatim but for names: our name at the v2.00 place; four callees as L14's copies; the audit's `long long`->`long` respelling touches no line carried; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_003F4F98.c` (author commit `91e04732`, llesieur99) | `func_L14_00423DC0` (v1.01 address) |
| `func_L14_004283B8` in `going-decompiled/levels/usa/L14/src/func_L14_004283B8.c`, level program 14 (adapted: our name at the v2.00 place; the boot's `g_gsScreenContext` (upstream's `short []` type kept); two callees as L14's copies; one `unsigned long` parameter written `u64` (RULING #13197), typedef written out; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_003F4F98.c` (author commit `431ac503`, llesieur99) | `func_L14_00426DE8` (v1.01 address) |
| `func_L14_0042E5F8` in `going-decompiled/levels/usa/L14/src/func_L14_0042E5F8.c`, level program 14 (adapted: our name at the v2.00 place; the boot's `sqrt`, `dpmul`, `dpdiv`, `dptofp` for upstream's 0x80000000-based spellings; `func_801234F0` as `func_001234F0`, address-named; one callee as L14's copy; `long long` written `s64` (RULING #13197), typedef written out; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_003F4F98.c` (author commit `431ac503`, llesieur99) | `func_L14_0042D018` (v1.01 address) |
| `func_L14_004337B0` in `going-decompiled/levels/usa/L14/src/func_L14_004337B0.c`, level program 14 (verbatim but for names: our name at the v2.00 place; the boot's `g_listenerPosHistory`; eight callees as L14's copies / v2.00 places, one of them through upstream's asm label; the audit's `long long`->`long` respelling touches no line carried; the file's includes, other functions and their externs not carried), task #2563; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/14_aranos_prison/L14_003F4F98.c` (author commit `312b7eea`, Louis-Philippe Le Sieur) | `func_L14_004321D0` (v1.01 address) |
| `func_L02_002EE5A0` in `going-decompiled/levels/usa/L02/src/func_L02_002EE5A0.c`, level program 02 (adapted: `sda` -> `section(".sdata")` on four extern declarations (RULING #8620); the four address-named externs keep their upstream names (RULING #13291 §2); includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_002ED7B0.c` (author commit `6e138d9a`, bigmak94) | `func_L02_002ED7B0` (v1.01 address) |
| `func_L02_0035CF30` in `going-decompiled/levels/usa/L02/src/func_L02_0035CF30.c`, level program 02 (verbatim but for names: the boot's `g_anAvailableLevelOrder`; three level symbols to our split labels and `D_L02_0021E630` to the address-named `D_L02_0021E660`; `volatile` and `sda` kept; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_0035BD60-init-g8.c` (author commit `a96d3126`, Louis-Philippe Le Sieur) | `func_L02_0035BD60` (v1.01 address) |
| `func_L02_003CC348` in `going-decompiled/levels/usa/L02/src/func_L02_003CC348.c`, level program 02 (verbatim but for names: two callees to our split names; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_002ED7B0.c` (author commit `91e04732`, llesieur99) | `func_L02_003CADF0` (v1.01 address) |
| `func_L02_003CEB08` in `going-decompiled/levels/usa/L02/src/func_L02_003CEB08.c`, level program 02 (adapted: `sda` -> `section(".sdata")` on two extern declarations (RULING #8620); three address-named externs keep their upstream names; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_002ED7B0.c` (author commit `ac1ee013`, bigmak94) | `func_L02_003CD5B0` (v1.01 address) |
| `func_L02_003D2988` in `going-decompiled/levels/usa/L02/src/func_L02_003D2988.c`, level program 02 (verbatim but for names: four callees to our split names; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_002ED7B0.c` (author commit `91e04732`, llesieur99) | `func_L02_003D1430` (v1.01 address) |
| `func_L02_003D2A50` in `going-decompiled/levels/usa/L02/src/func_L02_003D2A50.c`, level program 02 (verbatim but for names: two callees to our split names; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_002ED7B0.c` (author commit `91e04732`, llesieur99) | `func_L02_003D14F8` (v1.01 address) |
| `func_L02_003D2DD8` in `going-decompiled/levels/usa/L02/src/func_L02_003D2DD8.c`, level program 02 (verbatim but for names: none besides the function name (no relocations); includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_002ED7B0.c` (author commit `91e04732, last changed 863719df`, llesieur99; Louis-Philippe Le Sieur) | `func_L02_003D1880` (v1.01 address) |
| `func_L02_003D2E30` in `going-decompiled/levels/usa/L02/src/func_L02_003D2E30.c`, level program 02 (verbatim but for names: two address-named externs keep their upstream names; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_002ED7B0-g8.c` (author commit `ac1ee013`, bigmak94) | `func_L02_003D18D8` (v1.01 address) |
| `func_L02_003D37A0` in `going-decompiled/levels/usa/L02/src/func_L02_003D37A0.c`, level program 02 (verbatim but for names: six callees to our split names; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_002ED7B0.c` (author commit `431ac503`, llesieur99) | `func_L02_003D2248` (v1.01 address) |
| `func_L02_003DC2F0` in `going-decompiled/levels/usa/L02/src/func_L02_003DC2F0.c`, level program 02 (adapted: four callees to our split names; `sda` -> `section(".sdata")` on the `D_L02_001B359C` extern declaration (RULING #8620), which keeps its address name; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_003D26B0.c` (author commit `431ac503`, llesieur99) | `func_L02_003DAD88` (v1.01 address) |
| `func_L02_003DCC40` in `going-decompiled/levels/usa/L02/src/func_L02_003DCC40.c`, level program 02 (adapted: the boot's `g_listenerPosHistory` for `D_00188660`; six callees to our split names; `sda` -> `section(".sdata")` on the `D_L02_001B359C` extern declaration (RULING #8620), which keeps its address name; the trailing `#undef` dropped; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_003DB310.c` (author commit `0a1d634f`, bigmak94) | `func_L02_003DB6D8` (v1.01 address) |
| `func_L02_003F69A0` in `going-decompiled/levels/usa/L02/src/func_L02_003F69A0.c`, level program 02 (adapted: one callee to our split name; `sda` -> `section(".sdata")` on the `D_L02_001B3AF8` extern declaration (RULING #8620), which keeps its address name; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `adcbd51e`, llesieur99) | `func_L02_003F5430` (v1.01 address) |
| `func_L02_003FCA78` in `going-decompiled/levels/usa/L02/src/func_L02_003FCA78.c`, level program 02 (adapted: one callee to our split name; built at -G8 where upstream's file is -G0 (an `L02_c.tsv` gflag); two address-named externs keep their upstream names; `sda` kept (our cc1 ignores it); includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `9e5c365f`, bigmak94) | `func_L02_003FB4D8` (v1.01 address) |
| `func_L02_003FCB40` in `going-decompiled/levels/usa/L02/src/func_L02_003FCB40.c`, level program 02 (adapted: two callees to our split names; `sda` -> `section(".sdata")` on the `D_L02_001B359C` extern declaration (RULING #8620); two address-named externs keep their upstream names; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `7a7816b9`, bigmak94) | `func_L02_003FB5A0` (v1.01 address) |
| `func_L02_003FD130` in `going-decompiled/levels/usa/L02/src/func_L02_003FD130.c`, level program 02 (adapted: six callees to our split names; `sda` -> `section(".sdata")` on the `D_L02_001B359C` extern declaration (RULING #8620); two address-named externs keep their upstream names; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `455cde53`, Flavius) | `func_L02_003FBB90` (v1.01 address) |
| `func_L02_003FD638` in `going-decompiled/levels/usa/L02/src/func_L02_003FD638.c`, level program 02 (adapted: `sda` -> `section(".sdata")` on the `D_L02_001B368C` extern declaration (RULING #8620), which keeps its address name; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `adcbd51e`, llesieur99) | `func_L02_003FC098` (v1.01 address) |
| `func_L02_003FE020` in `going-decompiled/levels/usa/L02/src/func_L02_003FE020.c`, level program 02 (adapted: the boot's `g_heroPos` for `D_00189EA0`; thirteen callees to our split names; `sda` -> `section(".sdata")` on three extern declarations (RULING #8620); the address-named externs keep their upstream names; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `24d977dd`, bigmak94) | `func_L02_003FCA80` (v1.01 address) |
| `func_L02_00402258` in `going-decompiled/levels/usa/L02/src/func_L02_00402258.c`, level program 02 (adapted: nine callees to our split names; `sda` -> `section(".sdata")` on the `D_L02_001B35A8` extern declaration (RULING #8620), which keeps its address name; includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_003F5430.c` (author commit `6eae4d38`, bigmak94) | `func_L02_00400B10` (v1.01 address) |
| `func_L02_004112C8` in `going-decompiled/levels/usa/L02/src/func_L02_004112C8.c`, level program 02 (adapted: the boot's `g_heroPos` for `D_00189EA0`; `D_001A7BCB` to `D_001A7BD3` (the address the ROM's relocations imply) and `D_L02_001395B8` to `D_001395B8` (a core address), both address-named; eight callees to our split names; `sda` -> `section(".sdata")` on the `D_001A7BD3` extern declaration (RULING #8620); includes and unused declarations dropped), task #2557 | `src/us2/levels/02_maktar_nebula/L02_004112C8.c` (author commit `50c79a7f`, Flavius) | `func_US2_L02_004112C8` (v2.00 address) |
| `func_L02_00425010` in `going-decompiled/levels/usa/L02/src/func_L02_00425010.c`, level program 02 (adapted: two colour-pair symbols and eight callees to our split names; `sda` -> `section(".sdata")` on the two colour-pair extern declarations (RULING #8620); includes and unused declarations dropped), task #2557 | `src/levels/02_maktar_nebula/L02_0040A4A8.c` (author commit `11c21414`, bigmak94) | `func_L02_00423838` (v1.01 address) |
| `func_L11_003983E0` in `going-decompiled/levels/usa/L11/src/func_L11_003983E0.c`, level program 11 (verbatim but for names: our name, 1 data name, 6 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_002DE9E0-g8.c` (author commit `daf0e4a1`, Flavius) | `func_L11_00397068` (v1.01 address) |
| `func_L11_003F3178` in `going-decompiled/levels/usa/L11/src/func_L11_003F3178.c`, level program 11 (adapted: built at -G8 (upstream -G0), text verbatim but for names; our name, 1 data name, 8 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_003D6270.c` (author commit `91e04732`, llesieur99) | `func_L11_003F1C68` (v1.01 address) |
| `func_L11_003F4448` in `going-decompiled/levels/usa/L11/src/func_L11_003F4448.c`, level program 11 (verbatim but for names: our name, 2 data names, 8 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_003F2F38-g8.c` (author commit `eabc4759`, Flavius) | `func_L11_003F2F38` (v1.01 address) |
| `func_L11_00418AB0` in `going-decompiled/levels/usa/L11/src/func_L11_00418AB0.c`, level program 11 (verbatim but for names: our name, 2 data names (`g_soundBankHandlesBlk` the boot's), 11 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_003D6270-g8.c` (author commit `aaa28c3b`, Flavius) | `func_L11_00417340` (v1.01 address) |
| `func_L11_00426FA0` in `going-decompiled/levels/usa/L11/src/func_L11_00426FA0.c`, level program 11 (adapted: built at -G8 (upstream -G0), text verbatim but for names; our name, 10 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00420C88.c` (author commit `aaa28c3b`, Flavius) | `func_L11_00425788` (v1.01 address) |
| `func_L11_0042B988` in `going-decompiled/levels/usa/L11/src/func_L11_0042B988.c`, level program 11 (adapted: built at -G8 (upstream -G0), text verbatim but for names; our name, 10 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00425F10.c` (author commit `76d5f55b`, Flavius) | `func_L11_0042A158` (v1.01 address) |
| `func_L11_00436338` in `going-decompiled/levels/usa/L11/src/func_L11_00436338.c`, level program 11 (adapted: `sda` as `section(".sdata")` (RULING #8620); our name, 5 data names, 6 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00432628-g8.c` (author commit `9ad4d690`, Flavius) | `func_L11_00434AF8` (v1.01 address) |
| `func_L11_00437B90` in `going-decompiled/levels/usa/L11/src/func_L11_00437B90.c`, level program 11 (verbatim but for names: our name, 1 data name, 20 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00436350-g8.c` (author commit `644c5b4d`, bigbeno37) | `func_L11_00436350` (v1.01 address) |
| `func_L11_0043AE08` in `going-decompiled/levels/usa/L11/src/func_L11_0043AE08.c`, level program 11 (verbatim but for names: our name, 1 data name renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00432628-g8.c` (author commit `4541c4ce`, ReaperOfVeriod) | `func_L11_004395C8` (v1.01 address) |
| `func_L11_0043BC38` in `going-decompiled/levels/usa/L11/src/func_L11_0043BC38.c`, level program 11 (adapted: built at -G8 (upstream -G0), text verbatim but for names; our name, 1 data name, 9 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00432628.c` (author commit `5acf9869`, Flavius) | `func_L11_0043A3F8` (v1.01 address) |
| `func_L11_0043C218` in `going-decompiled/levels/usa/L11/src/func_L11_0043C218.c`, level program 11 (verbatim but for names: our name, 1 data name, 5 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00432628-g8.c` (author commit `47360109`, Flavius) | `func_L11_0043A9D8` (v1.01 address) |
| `func_L11_00442668` in `going-decompiled/levels/usa/L11/src/func_L11_00442668.c`, level program 11 (adapted: `sda` as `section(".sdata")` (RULING #8620); our name, 5 data names (`g_heroPos` the boot's), 3 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_0043E1A0-g8.c` (author commit `d932dae8`, bigmak94) | `func_L11_00440E18` (v1.01 address) |
| `func_L11_00443C48` in `going-decompiled/levels/usa/L11/src/func_L11_00443C48.c`, level program 11 (verbatim but for names: our name, 16 data names (`g_soundBankHandlesBlk` the boot's), 4 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_0043E1A0-g8.c` (author commit `c7d9e30a`, bigbeno37) | `func_L11_004423F8` (v1.01 address) |
| `func_L11_00443F60` in `going-decompiled/levels/usa/L11/src/func_L11_00443F60.c`, level program 11 (verbatim but for names: our name, 1 data name renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_0043E1A0-g8.c` (author commit `daf0e4a1`, Flavius) | `func_L11_00442710` (v1.01 address) |
| `func_L11_00444BD0` in `going-decompiled/levels/usa/L11/src/func_L11_00444BD0.c`, level program 11 (verbatim but for names: our name, 4 data names, 5 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_0043E1A0-g8.c` (author commit `264cb2be`, llesieur99) | `func_L11_00443380` (v1.01 address) |
| `func_L11_00444CE8` in `going-decompiled/levels/usa/L11/src/func_L11_00444CE8.c`, level program 11 (verbatim but for names: our name, 5 data names (`g_heroState`, `g_itemEquippedSlot` the boot's), 15 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00443498-g8.c` (author commit `3cd8af2d`, bigbeno37) | `func_L11_00443498` (v1.01 address) |
| `func_L11_00449ED0` in `going-decompiled/levels/usa/L11/src/func_L11_00449ED0.c`, level program 11 (verbatim but for names: our name, 2 data names, 16 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_004465F8-g8.c` (author commit `3cd8af2d`, bigbeno37) | `func_L11_004484A8` (v1.01 address) |
| `func_L11_0044A4D0` in `going-decompiled/levels/usa/L11/src/func_L11_0044A4D0.c`, level program 11 (adapted: built at -G8 (upstream -G0), text verbatim but for names; our name, 1 data name (`g_heroPos` the boot's), 9 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00448AA8.c` (author commit `d282effa`, Flavius) | `func_L11_00448AA8` (v1.01 address) |
| `func_L11_0044AAC0` in `going-decompiled/levels/usa/L11/src/func_L11_0044AAC0.c`, level program 11 (adapted: built at -G8 (upstream -G0), text verbatim but for names; our name, 7 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_004465F8.c` (author commit `5acf9869`, Flavius) | `func_L11_00449098` (v1.01 address) |
| `func_L11_0044AD58` in `going-decompiled/levels/usa/L11/src/func_L11_0044AD58.c`, level program 11 (adapted: built at -G8 (upstream -G0); `sda` as `section(".sdata")` (RULING #8620); our name, 4 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention (the file's later commit `7c6f71af`, Mateusz Kłysz, renamed one data name only; not Lombyte-origin) | `src/levels/11_joba/L11_00449330.c` (author commit `72a6596f`, Flavius) | `func_L11_00449330` (v1.01 address) |
| `func_L11_0044D7A0` in `going-decompiled/levels/usa/L11/src/func_L11_0044D7A0.c`, level program 11 (adapted: `sda` as `section(".sdata")` (RULING #8620); our name, 2 data names, 17 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_0044BD68-g8.c` (author commit `644c5b4d`, bigbeno37) | `func_L11_0044BD68` (v1.01 address) |
| `func_L11_00453EA8` in `going-decompiled/levels/usa/L11/src/func_L11_00453EA8.c`, level program 11 (verbatim but for names: our name, 2 data names, 19 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_004465F8-g8.c` (author commit `96f56db3`, bigbeno37) | `func_L11_00452450` (v1.01 address) |
| `func_L11_00455F60` in `going-decompiled/levels/usa/L11/src/func_L11_00455F60.c`, level program 11 (verbatim but for names: our name, 2 data names (`g_listenerPosHistory` the boot's), 27 callees renamed to the v2.00 disc's own relocation targets; no `long long` in this body, so no respelling (FACT #13321); `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358-g8.c` (author commit `4014e0b6`, Flavius) | `func_L11_00454508` (v1.01 address) |
| `func_L11_0045A6F0` in `going-decompiled/levels/usa/L11/src/func_L11_0045A6F0.c`, level program 11 (verbatim but for names: our name, 4 data names, 4 callees renamed to the v2.00 disc's own relocation targets; upstream's `SDATA()` macro written out as its section attribute; `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00458C38-g8.c` (author commit `5f3b0386`, Louis-Philippe Le Sieur) | `func_L11_00458C38` (v1.01 address) |
| `func_L11_0045B800` in `going-decompiled/levels/usa/L11/src/func_L11_0045B800.c`, level program 11 (verbatim but for names: our name, 1 data name, 8 callees renamed to the v2.00 disc's own relocation targets; no `long long` in this body, so no respelling (FACT #13321); `common.h` dropped, typedefs written; assembled through `tools/ee/asm_unit.sh`), task #2578; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/11_joba/L11_00453358-g8.c` (author commit `431ac503`, llesieur99) | `func_L11_00459D48` (v1.01 address) |
| `func_L09_002B5E70` in `going-decompiled/levels/usa/L09/src/func_L09_002B5E70.c`, level program 09 (adapted: our name at the v2.00 place; `D_00189E20` as the boot's `g_soundBankHandlesBlk`; three callees renamed to their L09 places; built at -G8 against upstream's -G0; `include_asm.h` dropped; assembled through `tools/ee/asm_unit.sh`), task #2580; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/09_dobbo/L09_002AFC90.c` (author commit `aac730b3`, Flavius) | `func_L09_002B5CE8` (v1.01 address) |
| `func_L09_003FCC18` in `going-decompiled/levels/usa/L09/src/func_L09_003FCC18.c`, level program 09 (adapted: our name at the v2.00 place; four data names as address-named externs (RULING #13291 section 2); one callee renamed to its L09 place; built at -G8 against upstream's -G0; upstream's `MACRO_ADDR` (`section(".sdata")`, RULING #8620) kept and defined in the TU; `common.h` dropped; assembled through `tools/ee/asm_unit.sh`), task #2580; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/09_dobbo/L09_003FB788.c` (author commit `757a3e46`, bigbeno37) | `func_L09_003FB788` (v1.01 address) |
| `func_L09_003FCCF0` in `going-decompiled/levels/usa/L09/src/func_L09_003FCCF0.c`, level program 09 (adapted: our name at the v2.00 place; four data names as address-named externs (RULING #13291 section 2); one callee renamed to its L09 place; built at -G8 against upstream's -G0; upstream's `MACRO_ADDR` (`section(".sdata")`, RULING #8620) kept and defined in the TU; `include_asm.h`/`common.h` dropped; brace moved (whitespace only); assembled through `tools/ee/asm_unit.sh`), task #2580; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/09_dobbo/L09_003FB860.c` (author commit `88c5f521`, Flavius) | `func_L09_003FB860` (v1.01 address) |
| `func_L09_003FF768` in `going-decompiled/levels/usa/L09/src/func_L09_003FF768.c`, level program 09 (verbatim but for names: our name at the v2.00 place; one data label and one callee renamed to their L09 targets (`sda` text kept, ignored by cc1); `common.h` dropped; assembled through `tools/ee/asm_unit.sh`), task #2580; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/09_dobbo/L09_002F8BA0-g8.c` (author commit `ba383701`, bigbeno37) | `func_L09_003FE2D8` (v1.01 address) |
| `func_L09_003FFF78` in `going-decompiled/levels/usa/L09/src/func_L09_003FFF78.c`, level program 09 (verbatim but for names: our name at the v2.00 place; seven data labels and seven callees renamed to their L09 targets (`sda` text kept, ignored by cc1); `common.h` dropped, its typedefs written in the TU; assembled through `tools/ee/asm_unit.sh`), task #2580; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/09_dobbo/L09_003FEAE8-g8.c` (author commit `1794e699`, Louis-Philippe Le Sieur) | `func_L09_003FEAE8` (v1.01 address) |
| `func_L09_00406020` in `going-decompiled/levels/usa/L09/src/func_L09_00406020.c`, level program 09 (verbatim but for names: our name at the v2.00 place; one data label and one callee renamed to their L09 targets (`sda` text kept, ignored by cc1); `common.h` dropped, its typedefs written in the TU; assembled through `tools/ee/asm_unit.sh`), task #2580; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/09_dobbo/L09_003FEA48-g8.c` (author commit `264cb2be`, llesieur99) | `func_L09_00404B90` (v1.01 address) |
| `func_L09_00406080` in `going-decompiled/levels/usa/L09/src/func_L09_00406080.c`, level program 09 (verbatim but for names: our name at the v2.00 place; one data label and one callee renamed to their L09 targets (`sda` text kept, ignored by cc1); `common.h` dropped, its typedefs written in the TU; assembled through `tools/ee/asm_unit.sh`), task #2580; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/09_dobbo/L09_003FEA48-g8.c` (author commit `264cb2be`, llesieur99) | `func_L09_00404BF0` (v1.01 address) |
| `func_L09_0040C5B8` in `going-decompiled/levels/usa/L09/src/func_L09_0040C5B8.c`, level program 09 (verbatim but for names: our name at the v2.00 place; five data labels and nine callees renamed to their L09 targets (`sda` text kept, ignored by cc1; `func_L09_002EB958` still undeclared, as upstream); upstream's second name `D_L09_001B15B8_4` for 0x001B1578 as the unprefixed address name `D_001B1578`, since `D_L09_001B1578` is the scalar's; `common.h` dropped, its typedefs and `MACRO_ADDR` written in the TU; assembled through `tools/ee/asm_unit.sh`), task #2580; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/09_dobbo/L09_003FEA48-g8.c` (author commit `27b929ce`, Lynder063) | `func_L09_0040B0C8` (v1.01 address) |
| `func_L09_0041EB58` in `going-decompiled/levels/usa/L09/src/func_L09_0041EB58.c`, level program 09 (verbatim but for names: our name at the v2.00 place; four data labels and thirteen callees renamed to their L09 targets (`sda` text kept, ignored by cc1); `common.h` dropped; assembled through `tools/ee/asm_unit.sh`), task #2580; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/09_dobbo/L09_0041D650-g8.c` (author commit `913f6380`, bigbeno37) | `func_L09_0041D650` (v1.01 address) |
| `func_L20_00441690` in `going-decompiled/levels/usa/L20/src/func_L20_00441690.c`, level program 20 (verbatim but for names: our name, 1 level data name, 1 callee renamed to the v2.00 disc's own relocation targets; upstream's per-file `-mno-split-addresses` not passed (measured inert); `include_asm.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0-g8.c` (author commit `ee3fee45`, llesieur99) | `func_L20_00440150` (v1.01 address) |
| `func_L20_004419A0` in `going-decompiled/levels/usa/L20/src/func_L20_004419A0.c`, level program 20 (verbatim but for names: our name, 1 callee renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `db93b56d`, llesieur99) | `func_L20_00440460` (v1.01 address) |
| `func_L20_004419F8` in `going-decompiled/levels/usa/L20/src/func_L20_004419F8.c`, level program 20 (verbatim but for names: our name, 5 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `ee3fee45`, llesieur99) | `func_L20_004404B8` (v1.01 address) |
| `func_L20_00441AB0` in `going-decompiled/levels/usa/L20/src/func_L20_00441AB0.c`, level program 20 (verbatim but for names: our name, 1 callee renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `db93b56d`, llesieur99) | `func_L20_00440570` (v1.01 address) |
| `func_L20_00443D10` in `going-decompiled/levels/usa/L20/src/func_L20_00443D10.c`, level program 20 (verbatim but for names: our name, 4 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `adcbd51e`, llesieur99) | `func_L20_004427C8` (v1.01 address) |
| `func_L20_00444158` in `going-decompiled/levels/usa/L20/src/func_L20_00444158.c`, level program 20 (verbatim but for names: our name, 4 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `adcbd51e`, llesieur99) | `func_L20_00442C10` (v1.01 address) |
| `func_L20_00444478` in `going-decompiled/levels/usa/L20/src/func_L20_00444478.c`, level program 20 (verbatim but for names: our name, 3 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `db93b56d`, llesieur99) | `func_L20_00442F30` (v1.01 address) |
| `func_L20_00444A50` in `going-decompiled/levels/usa/L20/src/func_L20_00444A50.c`, level program 20 (verbatim but for names: our name, 4 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `765097bb`, ReaperOfVeriod) | `func_L20_00443508` (v1.01 address) |
| `func_L20_00444E50` in `going-decompiled/levels/usa/L20/src/func_L20_00444E50.c`, level program 20 (verbatim but for names: our name, 7 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `3b12a35b`, Louis-Philippe Le Sieur) | `func_L20_00443908` (v1.01 address) |
| `func_L20_004450A8` in `going-decompiled/levels/usa/L20/src/func_L20_004450A8.c`, level program 20 (verbatim but for names: our name, 6 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_00443B60-helpers.c` (author commit `bf60a47f`, Louis-Philippe Le Sieur) | `func_L20_00443B60` (v1.01 address) |
| `func_L20_00445238` in `going-decompiled/levels/usa/L20/src/func_L20_00445238.c`, level program 20 (verbatim but for names: our name, 2 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `c17a465e`, Fromnowhere) | `func_L20_00443CF0` (v1.01 address) |
| `func_L20_004452A0` in `going-decompiled/levels/usa/L20/src/func_L20_004452A0.c`, level program 20 (verbatim but for names: our name, 3 callees renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `c17a465e`, Fromnowhere) | `func_L20_00443D58` (v1.01 address) |
| `func_L20_004461B0` in `going-decompiled/levels/usa/L20/src/func_L20_004461B0.c`, level program 20 (adapted, no device: our name, 3 callees renamed to the v2.00 disc's own relocation targets; one extern prototype added for `func_L20_00441AB0`, defined earlier in the upstream file; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `db93b56d`, llesieur99) | `func_L20_00444C68` (v1.01 address) |
| `func_L20_00448E50` in `going-decompiled/levels/usa/L20/src/func_L20_00448E50.c`, level program 20 (verbatim but for names: our name, 1 callee renamed to the v2.00 disc's own relocation targets; `common.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_0043ECB0.c` (author commit `db93b56d`, llesieur99) | `func_L20_00447908` (v1.01 address) |
| `func_L20_0044AFC0` in `going-decompiled/levels/usa/L20/src/func_L20_0044AFC0.c`, level program 20 (verbatim but for names: our name, 1 callee renamed to the v2.00 disc's own relocation targets; `include_asm.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_00449178.c` (author commit `ee3fee45`, llesieur99) | `func_L20_00449A78` (v1.01 address) |
| `func_L20_0044B020` in `going-decompiled/levels/usa/L20/src/func_L20_0044B020.c`, level program 20 (verbatim but for names: our name (no relocations); `include_asm.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_00449178.c` (author commit `ee3fee45`, llesieur99) | `func_L20_00449AD8` (v1.01 address) |
| `func_L20_0044B030` in `going-decompiled/levels/usa/L20/src/func_L20_0044B030.c`, level program 20 (verbatim but for names: our name, 2 level data names, 5 callees renamed to the v2.00 disc's own relocation targets; `include_asm.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_00449178-g8.c` (author commit `e0711884`, ReaperOfVeriod) | `func_L20_00449AE8` (v1.01 address) |
| `func_L20_0044B650` in `going-decompiled/levels/usa/L20/src/func_L20_0044B650.c`, level program 20 (verbatim but for names: our name, 3 callees renamed to the v2.00 disc's own relocation targets; `include_asm.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_00449178.c` (author commit `ee3fee45`, llesieur99) | `func_L20_0044A108` (v1.01 address) |
| `func_L20_0044C0B0` in `going-decompiled/levels/usa/L20/src/func_L20_0044C0B0.c`, level program 20 (verbatim but for names: our name, 4 callees renamed to the v2.00 disc's own relocation targets; `include_asm.h` dropped; plain `as`), task #2566; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/20_yeedil/L20_00449178.c` (author commit `3b12a35b`, Louis-Philippe Le Sieur) | `func_L20_0044AB68` (v1.01 address) |
| `func_L23_003C1278` in `going-decompiled/levels/usa/L23/src/func_L23_003C1278.c`, level program 23 (verbatim but for names), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `91e04732`, llesieur99) | `func_L23_003BFE38` (v1.01 address) |
| `func_L23_003C1400` in `going-decompiled/levels/usa/L23/src/func_L23_003C1400.c`, level program 23 (verbatim but for names), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `91e04732`, llesieur99) | `func_L23_003BFFC0` (v1.01 address) |
| `func_L23_003C1468` in `going-decompiled/levels/usa/L23/src/func_L23_003C1468.c`, level program 23 (verbatim but for names), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `91e04732`, llesieur99) | `func_L23_003C0028` (v1.01 address) |
| `func_L23_003C2238` in `going-decompiled/levels/usa/L23/src/func_L23_003C2238.c`, level program 23 (adapted: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; `unsigned long long` written `u64` (RULING #13197)), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `91e04732`, llesieur99) | `func_L23_003C0DF8` (v1.01 address) |
| `func_L23_003C22D8` in `going-decompiled/levels/usa/L23/src/func_L23_003C22D8.c`, level program 23 (adapted: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; `sda` as `section(".sdata")` (RULING #8620); `long long` written `s64` (RULING #13197); three address-named externs (RULING #13291 section 2)), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `ee3fee45`, llesieur99) | `func_L23_003C0E98` (v1.01 address) |
| `func_L23_003C2388` in `going-decompiled/levels/usa/L23/src/func_L23_003C2388.c`, level program 23 (verbatim but for names), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `91e04732`, llesieur99) | `func_L23_003C0F48` (v1.01 address) |
| `func_L23_003C23C8` in `going-decompiled/levels/usa/L23/src/func_L23_003C23C8.c`, level program 23 (adapted: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; two `sda` as `section(".sdata")` (RULING #8620); `long long` written `s64`/`u64` (RULING #13197); three address-named externs (RULING #13291 section 2); the boot's `g_heroPos` for `D_00189EA0`), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `fa31bfc3`, bigmak94) | `func_L23_003C0F88` (v1.01 address) |
| `func_L23_003C2DA0` in `going-decompiled/levels/usa/L23/src/func_L23_003C2DA0.c`, level program 23 (adapted: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; `sda` as `section(".sdata")` (RULING #8620); an address-named extern (RULING #13291 section 2)), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `91e04732`, llesieur99) | `func_L23_003C1960` (v1.01 address) |
| `func_L23_003C2DC8` in `going-decompiled/levels/usa/L23/src/func_L23_003C2DC8.c`, level program 23 (verbatim but for names), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `91e04732`, llesieur99) | `func_L23_003C1988` (v1.01 address) |
| `func_L23_003C2EF0` in `going-decompiled/levels/usa/L23/src/func_L23_003C2EF0.c`, level program 23 (adapted: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; `sda` as `section(".sdata")` (RULING #8620); an address-named extern (RULING #13291 section 2)), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `91e04732`, llesieur99) | `func_L23_003C1AB0` (v1.01 address) |
| `func_L23_004174C8` in `going-decompiled/levels/usa/L23/src/func_L23_004174C8.c`, level program 23 (verbatim but for names, inert `sda` attributes dropped at -G8), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90-g8.c` (author commit `91e04732`, llesieur99) | `func_L23_00415FC8` (v1.01 address) |
| `func_L23_00417590` in `going-decompiled/levels/usa/L23/src/func_L23_00417590.c`, level program 23 (verbatim but for names), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_0035CE90.c` (author commit `91e04732`, llesieur99) | `func_L23_00416090` (v1.01 address) |
| `func_L23_004195F0` in `going-decompiled/levels/usa/L23/src/func_L23_004195F0.c`, level program 23 (verbatim but for names), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_00416388.c` (author commit `773eafc2`, llesieur99) | `func_L23_004180F0` (v1.01 address) |
| `func_L23_0041A430` in `going-decompiled/levels/usa/L23/src/func_L23_0041A430.c`, level program 23 (adapted: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; the file's `u64` typedef written `unsigned long` (RULING #13197)), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_00416388.c` (author commit `adcbd51e`, llesieur99) | `func_L23_00418F30` (v1.01 address) |
| `func_L23_0041A4D0` in `going-decompiled/levels/usa/L23/src/func_L23_0041A4D0.c`, level program 23 (adapted: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; the file's `u64` typedef written `unsigned long` (RULING #13197); two address-named externs (RULING #13291 section 2)), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_00416388.c` (author commit `d2e11c19`, bigmak94) | `func_L23_00418FD0` (v1.01 address) |
| `func_L23_0041CDF8` in `going-decompiled/levels/usa/L23/src/func_L23_0041CDF8.c`, level program 23 (verbatim but for names, inert `sda` attributes dropped at -G8), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_00416388-g8.c` (author commit `773eafc2`, llesieur99) | `func_L23_0041B8F8` (v1.01 address) |
| `func_L23_0041CE48` in `going-decompiled/levels/usa/L23/src/func_L23_0041CE48.c`, level program 23 (verbatim but for names, inert `sda` attributes dropped at -G8), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_00416388-g8.c` (author commit `773eafc2`, llesieur99) | `func_L23_0041B948` (v1.01 address) |
| `func_L23_0041CE60` in `going-decompiled/levels/usa/L23/src/func_L23_0041CE60.c`, level program 23 (verbatim but for names, inert `sda` attributes dropped at -G8), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_00416388-g8.c` (author commit `adcbd51e`, llesieur99) | `func_L23_0041B960` (v1.01 address) |
| `func_L23_0041D540` in `going-decompiled/levels/usa/L23/src/func_L23_0041D540.c`, level program 23 (verbatim but for names, inert `sda` attributes dropped at -G8), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_00416388-g8.c` (author commit `adcbd51e`, llesieur99) | `func_L23_0041C040` (v1.01 address) |
| `func_L23_0041D5B0` in `going-decompiled/levels/usa/L23/src/func_L23_0041D5B0.c`, level program 23 (verbatim but for names, inert `sda` attributes dropped at -G8), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_00416388-g8.c` (author commit `ee3fee45`, llesieur99) | `func_L23_0041C0B0` (v1.01 address) |
| `func_L23_0041DCD8` in `going-decompiled/levels/usa/L23/src/func_L23_0041DCD8.c`, level program 23 (adapted: our name; level data names and callees renamed to the v2.00 disc's targets, L23's own functions; `nosda` as `section(".data")` on the boot's `g_pHeroMoby` for `D_0018C0B0` (RULING #8620); an address-named extern (RULING #13291 section 2)), task #2567; GPL-3.0, no notice in the file, no Lombyte mention | `src/levels/23_damosel_orbit/L23_00416388-g8.c` (author commit `babfe411`, bigmak94) | `func_L23_0041C7D8` (v1.01 address) |

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
| `func_0028E680` in `going-decompiled/src/usa/text/188858.c` (adapted: our callee and data spellings by address, a `section(".data")` addressing device on the `D_18B69A` declaration, `char *` written `u8 *` with `HudElement *` casts, this unit's prototypes for their extern block) | `src/game/0028BDB0.c` | `func_0028DDD8` (v1.01 address) |

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

Four C bodies are adapted from OpenRAC's `rac2-gc-decomp`,
<https://github.com/OpenRAC/rac2-gc-decomp> at commit
`96927f872c709e13aaa0f32265bb81a5531bf7ed`, from files that carry their own
MIT notice, not the repository's GPL-3.0. The notice is the same, word for
word, in all four files. It is reproduced verbatim here and in each function's doc
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
- `func_003418D8` in `going-decompiled/src/usa/text/235FE8.c`,
  from `src/game/003409C8-rc2.c`, `func_003409C8` (v1.01 address; read at
  OpenRAC `rac2-decomp` `b2a35115e0cf580a1fdab8dd1071b157799e590e`, where the
  file is blob-identical; adapted: callee and data spellings ours, our
  signature with their `struct Obj` as our `GuiWeaponGridCursor`, their typed
  table as an EE-only asm-label view of `D_00259F38`). That file was carried
  over from atlasbez-dot/rac2-gc-decomp at `921a4dee` by atlasbez-dot
  (upstream commit `13cb3d0b`), task #2469.

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
#2471, #2472, #2459, #2503, #2504 and #2541 (tasks #2503, #2504 and #2541 read
their files at `5776dcc3`; they are identical at `b2a35115`). Each is adapted, with every respelling named in the function's `Source:`
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
| `func_00279F08` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: their index holder, item table and per-level `sda` state word as our `g_textRunCursor` view, `g_textRunTable` and a sized equate at `g_cameraCallbackCount + 0x80`; `func_002798A0` as `func_00279E00`; struct/local names; read at `5776dcc3`, file blob identical at `b2a35115`; s136os arm, gcse ON via `S136EXTRA=`, RULING #12178; task #2504) | `src/game/00274128.c` (author commit `65ef3209`, ThorpenCodes) | `func_002799A8` |
| `func_0027DC40` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: data and callees by address to our names; `sda`/`volatile` externs as EE-only RULING #8620/#12316/#13168/#8404 devices; asm-label prototype views of `func_00280C98` and `func_00280BB8`; `long` written `s64`/`u64`, RULING #13197; read at `5776dcc3`, body unchanged from `b2a35115`; s136os arm, gcse ON; task #2504) | `src/game/0027B948.c` (author commit `bf067513`, bigbeno37; layout credited upstream to OpenRAC `rac1-decomp` `src/game/draw.c`, GPL-3.0) | `func_0027D5C0` |
| `SetupGsDisplayBuffers` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: `long long` u64/s64 as our `long`-based u64/s64, RULING #13197; every scalar through an EE-only sized alias and `.data` on two u64 images, RULING #8620; their `ScreenExtent` struct as its fields' own symbols; struct/parameter names; read at `5776dcc3`, file blob identical at `b2a35115`; s136os arm, gcse ON; task #2504) | `src/game/00274128.c` (author commit `00ebb44e`, bigbeno37; credited upstream as adapted from RAC1 `SetPalMode` and Lombyte `src/video/display/set_pal_mode.c`, GPL-3.0) | `func_0027B440` |
| `DrawRotatedSprite2d` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: our callee spellings Vec4ScaleVu0 / Vec4AddVu0 / Vec4SubVu0 / func_00283B48 / func_00283B30 / FloatToInt for their func_00282D30 / func_00282CC0 / func_00282CF0 / func_00283198 / func_00283180 / func_00283CF0, Vec4ScaleVu0 called with our (dst, scale, src) prototype; their `D_001B22A8` read through a RULING #13168 typed view of our `g_frameDmaCursor`, their `Ff67aa9df_D_001A7350`/`7354` as the unit's `g_gsPixelOffsetXAbs`/`YAbs`; `sda` dropped; `long`/`unsigned long` written `s64`/`u64` (RULING #13197), `int`/`float`/`unsigned int`/`unsigned char` written `s32`/`f32`/`u32`/`u8`; s136os arm with two RULING #8549 Ps2EeAs dli rows, 0x27ED6C and 0x27ED78), task #2541 | `src/game/0027B948.c`, whose comment says the body is adapted from rac1-decomp `src/game/draw.c` `append_rotated_sprite_quad`, itself adapted from Lombyte `src/rendering/append_rotated_sprite_quad.c` (GPL-3.0) | `func_0027E4A8` (author commit `7dc80be1`, bigbeno37) |

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


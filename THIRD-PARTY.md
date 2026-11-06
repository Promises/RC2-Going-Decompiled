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
`Source:` line. Neither source file below carries a notice of its own.

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_00346878` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol names, signature and `PlayGlobalSound` prototype; their owner struct as our `GuiGridCursor` view; their `.sdata` `g_inventoryOwned` and their typed grid array as EE-only asm-label aliases; `sda` dropped), task #2467 | `src/us2/game/00346878-g8.c` | `func_US2_00346878` (v2.00 address) |
| `func_L24_0034D598` in `going-decompiled/levels/usa/L24/src/func_L24_0034D598.c`, level program 24 (adapted: our name at the v2.00 place, the boot's `g_anAvailableLevelOrder`, four level symbols renamed to the v2.00 ROM's relocation targets, `volatile` and `sda` dropped as measured inert, `include_asm.h` dropped; the blob is identical at the repository's head `5833be5b716983046be737d85080b96bc9f1435a`), task #2495 | `src/levels/24_ship_shack/L24_0034C450-init-g8.c` | `func_L24_0034C450` (v1.01 address) |

The GPL-2.0-or-later note in the section "OpenRAC rac2-gc-decomp (GPL-3.0)" above applies to these bodies too.

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

### OpenRAC rac2-decomp at b2a35115, text/178E88 Window X bodies (GPL-3.0)

One C function body is taken from OpenRAC's `rac2-decomp`,
<https://github.com/OpenRAC/rac2-decomp> at commit
`b2a35115e0cf580a1fdab8dd1071b157799e590e` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405/#12108), task
#2471. It is adapted, with every respelling named in the function's `Source:`
line.

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_00280EC8` in `going-decompiled/src/usa/text/178E88.cpp` (adapted: our callee spellings func_002859E0 / func_0027B858 / AppendGsRegPacket for their func_00285030 / func_0027B2F8 / func_002FC560, their `D_001A65EE` read as `*(s16 *)(g_gsScreenContext + 0x16E)`, `int`/`float` spelled `s32`/`f32`; s136os arm with one RULING #8549 Ps2EeAs dli row at 0x280F88) | `src/game/0027F128.c` | `func_00280510` (author commit `bf067513e`, bigbeno37) |

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

### OpenRAC rac2-decomp at b2a35115 (GPL-3.0)

One C function body is taken from OpenRAC's `rac2-decomp`,
<https://github.com/OpenRAC/rac2-decomp> at commit
`b2a35115e0cf580a1fdab8dd1071b157799e590e` (root `LICENSE`: GNU General Public
License, version 3; the file carries no notice of its own), with the authors'
permission (RULINGs #10405 and #12108), task #2468:

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_00346CD8` in `going-decompiled/src/usa/text/235FE8.c` (adapted: our symbol spellings, two through EE-only asm-label aliases replacing their typed array and `section(".sdata")`; their `sda` dropped; struct views renamed, their material record as our `GuiWeaponTableEntry`; signature ours with a local cast) | `src/us2/game/00346CD8-pad-g8.c` (author commit `13388073`, Flavius) | `func_US2_00346CD8` |

The GPL-2.0-or-later note in the section "OpenRAC rac2-gc-decomp (GPL-3.0)" above applies to this body too.

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

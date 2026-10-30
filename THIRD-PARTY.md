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

Three C function bodies are taken from OpenRAC's `rac2-gc-decomp`,
<https://github.com/OpenRAC/rac2-gc-decomp> at commit
`96927f872c709e13aaa0f32265bb81a5531bf7ed` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405), tasks #2230 and #2232:

| Here | Upstream file | Upstream name |
|---|---|---|
| `ApplyCameraShakeAxis` in `going-decompiled/src/usa/text/16E980.c` (adapted: camera block, signature and callee prototypes are ours) | `src/game/00271958-g8.c` | `func_00271958` (v1.01 address) |
| `SampleCameraFogZone` in `going-decompiled/src/usa/text/16E980.c` (adapted: fog block and region array reached through our symbols; signature and callees ours) | `src/game/0026EA00.c` | `func_002735A0` (v1.01 address) |
| `func_0011DE08` in `going-decompiled/src/usa/cod/015180.c` (adapted: the unit's own prototypes, with a `(SifRpcClient *)` and two `(char *)` casts) | `src/core/0011DDC8.c` (from rac1-decomp `src/core/00119D88.c`) | `func_0011DE08` |

The GPL-2.0-or-later note in the section "OpenRAC rac2-gc-decomp (GPL-3.0)" above applies to these bodies too.

### Lombyte (MIT)

The C body of `_sysbitFlush` in `going-decompiled/src/usa/cod/022FA8.c` is
taken from Lombyte, <https://github.com/mateuszklysz/Lombyte> at commit
`2c4452dd03f5f7ebb2868dbe073ccdb1afb27f61`, `src/sdk/debug/sysbit_flush.c`,
under this notice:

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

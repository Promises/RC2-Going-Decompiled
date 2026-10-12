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

### OpenRAC rac2-gc-decomp (GPL-3.0)

Five C function bodies are taken from OpenRAC's `rac2-gc-decomp`,
<https://github.com/OpenRAC/rac2-gc-decomp> at commit
`974302ba936bb455acd78034b58ad4b98bafcc6b` (root `LICENSE`: GNU General Public
License, version 3), with the authors' permission (RULING #10405):

| Here | Upstream file | Upstream name |
|---|---|---|
| `func_00131850`, `func_00131908` in `going-decompiled/src/usa/cod/0314C0.c` | `src/core/00131780.c` (from rac1-decomp `src/core/0012CC90.c`) | same names (v1.01 addresses equal v2.00 here) |
| `MatrixToEulerAngles` in `going-decompiled/src/usa/text/1A8180.c` | `src/game/002AA800.c` | `func_002AB6D8` |
| `InitMenuBgImageBuffers`, `func_002DCDC0` in `going-decompiled/src/usa/text/1D54C0.cpp` | `src/game/002D5E98.c` | `func_002DA238`, `func_002DC090` |

This project is `GPL-2.0-or-later`; the GPL-3.0 text of these bodies is
combined into it under the "or later" grant. Any wider licensing question is
the operator's (RULING #8304 defers it until the repository goes public).

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

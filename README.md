# RC2-Going-Decompiled

A matching decompilation of **Ratchet & Clank: Going Commando** (PlayStation 2, 2003).

| Target | Executable | Role |
|---|---|---|
| USA v2.00 | `SCUS_972.68` | primary |
| EU v1.00 | `SCES_516.07` | region validator |

The build reassembles the game's executable from C and splat-generated assembly
and is checked byte-for-byte against the original.

## This repository contains no game code or data

You need your own legally obtained copy of the game. Everything derived from it
(the disassembly under `going-decompiled/asm/`, extracted assets, the ROM image)
is generated locally from **your** ISO and is git-ignored.

The proprietary toolchain (SN Systems ProDG `ee-gcc`) and the Sony PS2 SDK
headers/libraries are not included either; the `scripts/fetch_*.sh` scripts
download them from public archives and verify them.

## Building

The matching compiler is a 32-bit Windows binary (`ee-gcc 2.9-ee-991111`), run
with [wibo](https://github.com/decompals/wibo) inside an **x86_64 Linux** Docker
image (`tools/ee/Dockerfile`). On Apple Silicon use a full x86 VM such as
`colima start ee-x86 --arch x86_64 --vm-type qemu`; Rosetta and qemu-user
cannot run it.

1. Put the ISO in `source-isos/` (file names are listed in the script) and
   extract the executable:
   ```sh
   scripts/extract_isos.sh usa   # needs bash 4+, bsdtar, and llvm-objcopy or mips-linux-gnu-objcopy
   ```
2. Fetch the toolchain, SDK libraries and SDK headers:
   ```sh
   scripts/fetch_ee_toolchain.sh
   scripts/fetch_sdk_libs.sh
   scripts/fetch_sdk_headers.sh         # verifies every header by hash
   ```
3. Split the executable with splat:
   ```sh
   python3 -m venv .venv-decomp && .venv-decomp/bin/pip install -r requirements-decomp.txt
   .venv-decomp/bin/python scripts/configure.py --region usa
   ```
4. Build the image and the game, then compare with the original:
   ```sh
   docker build -t ee-build tools/ee
   docker run --rm -v "$PWD":/work ee-build sh tools/ee/build.sh usa
   ```

`tools/ee/objdiff_build.sh` and `tools/ee/unit_report.sh` produce per-function
match reports with [objdiff](https://github.com/encounter/objdiff).

## Layout

- `going-decompiled/src/` – decompiled C, one file per splat unit, per region
- `going-decompiled/include/` – game headers (`rtl/` is fetched, not tracked)
- `going-decompiled/config/` – splat configuration
- `going-decompiled/symbol_addrs/` – symbol names and addresses
- `going-decompiled/libgcc/` – the compiler runtime, built from GCC source
- `tools/ee/` – build, assembly fix-up and match-verification scripts
- `tools/native/` – a host (non-PS2) build of the decompiled C for testing

A function that does not match yet is kept as `INCLUDE_ASM` and, where one
exists, carries a portable C body under `#else` (`TARGET_NATIVE`).

## Licence

GPL-2.0-or-later; see [LICENSE](LICENSE). The licence covers this project's own
work only, never the game. Bundled third-party source keeps its own licence;
see [THIRD-PARTY.md](THIRD-PARTY.md).

## Credits

See [credits.md](credits.md).

## Status

Work in progress. This repository is synced from the project's working
repository; each commit carries a `Synced-From:` trailer.

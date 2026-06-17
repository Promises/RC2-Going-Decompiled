# Native functional-test harness (`TARGET_NATIVE`)

Validates the **functionally-equivalent `#else` bodies** of the decomp — the
hand-written C that runs in the portable build but is *not* byte-matched. The
matching PS2 build can't catch a wrong arg-order or a bad struct offset in these
bodies, because it uses the original `INCLUDE_ASM` (the C is inert there). This
harness compiles and *runs* that C as ILP32 (4-byte pointers, like the EE) and
asserts behavior.

> Scope: the bodies only execute under `TARGET_NATIVE`. Nothing here can change
> the byte-matching PS2 build. A function that is byte-matched needs no test.

## Two tiers

### Tier 1 — compile-check gate (host, no VM)
```
tools/native/check.sh            # all TARGET_NATIVE units
tools/native/check.sh <file.c>   # one unit
```
ILP32 `clang -DTARGET_NATIVE -m32 -c`. Catches malformed C, bad casts, type
errors, signature contradictions. The unported-MIPS-callee boundary is kept as
warnings (expected), not failures. Already caught a real bug: `250080.c`
`func_00350910` is defined `void(s32*)` but called `s32(void)`.

### Tier 2 — functional run (colima `native-build` image)
```
tools/native/build_image.sh                          # one-time: build the image
tools/native/run_test.sh <unit.c> <tests/test_x.c>   # compile+run a test
```
Compiles unit + test as a runnable ILP32 x86 ELF in the VM, links with
`--gc-sections` so only the function-under-test and its real callees survive —
you mock only what's reached. Exit status = failed checks.

## Files
| File | Role |
|---|---|
| `check.sh` | Tier 1 compile-check gate |
| `mips_callees.h` | libc prelude force-included into the native build |
| `Dockerfile` / `build_image.sh` | the `native-build` image (gcc-multilib) |
| `run_test.sh` | Tier 2 runner |
| `native_test.h` | CHECK/CHECK_FEQ asserts + mock-call recorder |
| `tests/` | test files (one per unit / concern) |
| `ORACLE.md` | how to capture ground-truth from PCSX2 (the oracle) |

## Oracle
Pure functions: hand-author expected output. State-touching functions: capture
from the retail ROM in PCSX2 and replay (see `ORACLE.md`). Capture needs PCSX2
installed + driven — the harness is ready to receive traces.

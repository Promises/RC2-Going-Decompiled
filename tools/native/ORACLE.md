# Golden-trace oracle — capturing ground truth from PCSX2

The native functional tests need an **oracle**: the *correct* output for a given
input. For pure functions you can hand-author it (see `tests/test_235FE8.c`).
For functions that read/mutate real game state (struct mutators, anything
touching globals), you can't eyeball "correct" — you capture it from the retail
ROM running in **PCSX2**, then replay against the native build.

This is the one part of the harness with an **external dependency**: PCSX2 must
be installed, and a human (or a scripted fork) must drive it to the game state.

## Why PCSX2 for capture (and *not* for replay)

- **Capture → PCSX2 only.** Only the running game has the actual heap/object/
  global state at a given moment. No standalone CPU emulator can manufacture a
  believable RC2 state.
- **Replay → native C on the host.** Once you have (inputs → outputs), run the
  portable C against the captured inputs and assert. No emulator needed.
- **Do NOT replay through Unicorn/QEMU/Ghidra-PCode.** They model *generic*
  MIPS; the EE R5900 has 128-bit MMI ops and a non-IEEE FPU they don't emulate.
  Functions using those (e.g. the 128-bit camera copies) would diverge. PCSX2
  is the only thing that is actually the EE — keep it as the source of truth.

## PCSX2 instrumentation surfaces (current 2.x Qt)

| Surface | Can do | Cannot do |
|---|---|---|
| **Qt debugger** (GUI) | exec/memory breakpoints; read R5900 GPRs (a0–a3, v0–v1, sp, ra) + 128-bit regs; dump RAM | no scripting (Lua removed, no Python); manual point-and-click only |
| **PINE / IPC** (port 28011) | Read/Write 8/16/32/64 memory; savestates; Python client `pcsx2-interface` | **no register reads, no breakpoints** — memory poller only |
| **DebugServer fork** (`hkmodd/PCSX2-MCP`, port 21512) | breakpoints + register reads programmatically | non-official build you compile yourself |

**Implication:** register capture (a0–a3/v0 at the boundary) needs the GUI
debugger *or* the fork. PINE alone can't do it — it's for the bulk memory dump.

## Capture recipe (per function)

1. **Reach the state in PCSX2** (Apple Silicon: runs via Rosetta x86_64 build)
   and **save a savestate** — makes every capture reproducible.
2. **Compute the function's footprint statically in Ghidra** — param regs +
   referenced globals + pointer-arg target ranges. This avoids "watch all 32 MB
   of RAM"; you only dump what the function actually touches. (You already have
   the decomp — this is the cheap path.)
3. **Entry breakpoint** at the function's first instruction. Record `a0–a3`,
   `sp`, `ra`, FP arg regs (`$f12..`), and dump the footprint memory ranges +
   the incoming stack frame (for >4 args / struct-by-value; this game is
   EABI/o32 — first 4 int args in `$a0–$a3`).
4. **Return breakpoint at the entry-time `ra` value** (NOT the textual `jr ra`
   — there can be many, and you must clear the delay slot). Pair entry/return by
   `sp` to survive recursion. Record `v0` (`v0:v1` for 64-bit, `$f0` for float)
   and re-dump the mutated ranges.
5. **Persist the case**: `{ entry: regs+mem, return: regs+mem }`.

### Top 3 gotchas (from recon)

1. **PINE can't read registers or set breakpoints** — memory + savestates only.
   Don't architect around PINE doing the whole job.
2. **Return capture must breakpoint the entry-time `ra`, paired by `sp`, past
   the delay slot** — not `jr ra`. Otherwise recursion / multiple epilogues /
   the delay slot corrupt your v0/return snapshot.
3. **Replay emulators model generic MIPS, not the EE.** Pre-screen target
   functions for MMI / EE-FPU use before trusting any non-PCSX2 re-execution.

## Turning a captured case into a native test

Use the **pointer-graph-rebuild** pattern (see `tests/test_235FE8_oracle.c` for
a worked example):

1. Rebuild the object graph natively — allocate the object + each pointee block,
   wire the pointers together (you do NOT reuse PS2 absolute addresses).
2. Seed it with the captured ENTRY bytes + arg values.
3. Call the function under test via `run_test.sh`.
4. `CHECK` the mutated blocks + return value against the captured RETURN bytes.

Works for functions whose footprint is "pointer args + their pointees".
Functions that touch absolute globals need those mapped too (deferred — would
need fixed `mmap` of the EE address range, a bigger lift).

## Recommended next step when wiring real capture

- Install PCSX2 (Rosetta build on Apple Silicon), enable PINE.
- For a handful of functions: capture by hand via the GUI debugger + dump memory
  with the `pcsx2-interface` Python PINE client at the two stop points.
- For many functions: build the `hkmodd` DebugServer fork to script the
  breakpoint → register-read → memory-dump → continue loop.
- A JSON trace format + a generic data-driven loader is worth building **once a
  real capture exists to shape the format** — not before (it'd be a guess).

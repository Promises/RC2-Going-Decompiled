# M2 — globals arena (`TARGET_NATIVE` HLE backend)

Storage for the data globals (`g_*`/`D_*`) the native build references but
doesn't define. See `docs/HLE.md` for where this sits in the backend roadmap.

## Mechanic: generated linker script (Mechanic B)

A prototype A/B settled the linking mechanic (`docs/HLE.md` M2). **Mechanic B
(generated linker script) won** over macro-deref because it needs **zero source
edits** (vs ~619 `extern`-guard lines), **no per-symbol types** (placement is
pure offset), is PIE-clean, and a ROM-address-keyed PINE snapshot `memcpy`s
straight into `__gamedata_start`.

One contiguous `.gamedata` block mirrors the ROM data-segment layout; the script
`PROVIDE`s each global at `__gamedata_start + (rom_addr - rom_base)`, so existing
`extern T SYM;` decls bind to real symbols at exact ROM-relative offsets and
adjacency (indexing past a named global into its neighbour) just works.

## Files

| File | Role |
|---|---|
| `gen_arena.py` | generator (production) |
| `data_globals.txt` | input: the data-global gap list (from `tools/native/linkgap.sh`) |
| `arena.ld` | GENERATED — `INSERT AFTER .bss` fragment + 362 `PROVIDE`s |
| `arena_storage.c` | GENERATED — the contiguous backing block (`g_dataArena[]`) |
| `arena_unresolved.txt` | GENERATED — globals with no canonical address yet (Track-B follow-up) |

## Address sourcing (no per-symbol type needed)

- `D_xxxxxx` → ROM address is the hex in the name (`D_1A8A60` → `0x1A8A60`).
- named → `symbol_addrs/usa` (`SYM = 0x...;`).
- alias → a `symbol_addrs` comment `... alias SYM ...` (documented Ghidra
  aliases only — evidence-based, not guessed).

Current result: **362 placed / 408 referenced**. The 46 unplaced split into
~29 covered by tentative C definitions and **17 genuinely unaddressed**
(`arena_unresolved.txt`) — a Track-B task: recover each address in Ghidra, add
to `symbol_addrs`, regenerate. (The 29 tentatively-defined ones should also be
addressed eventually so their adjacency is faithful, not floating COMMON.)

## Regenerate

```
tools/native/linkgap.sh /tmp/lg.txt            # refresh the gap inventory
# extract the runtime-global bucket -> data_globals.txt, then:
python3 tools/native/runtime/arena/gen_arena.py \
    tools/native/runtime/arena/data_globals.txt \
    going-decompiled/symbol_addrs/usa/symbol_addrs.txt \
    tools/native/runtime/arena/
```

## Verify (real ILP32 link on the colima image)

`tools/native/runtime/arena/verify_link.sh` links all `TARGET_NATIVE` units +
`arena_storage.c` against `arena.ld` (with `--unresolved-symbols=ignore-all`)
and asserts **no placed global remains undefined**. The residual undefined set
is the other phases' work (M1 libc, M3 hardware, M4 still-asm funcs).

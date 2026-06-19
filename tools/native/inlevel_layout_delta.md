# In-level global layout delta — RE deliverable (2026-06-19, CORRECTED)

**For:** the tester's autonomous in-build harness, to seed/validate native
frame-path state against the in-level dump.

> ## ⚠️ CORRECTION — my earlier conclusion was WRONG
> An earlier version of this doc concluded "**no structural in-level relocation**;
> the `inlevel/` seed is just a mid-load capture artifact." **That was wrong**, and
> the tester empirically refuted it at human-confirmed controllable gameplay
> (Ratchet actively moving), with a 100%-byte-match relocation map.
>
> **There IS a persistent in-level data relocation.** My error: I anchored
> "settled/canonical" on `inlevel_run2_load` (which happens to be captured at a
> state-6 *pre-relocation transition*) and dismissed `inlevel/` (the real,
> relocated gameplay state) as a "transient artifact." Exactly backwards. The
> static-only RE could see the rodata-string overlap but mis-explained it as a
> transient spill instead of a real runtime relocation. Owning that.

## The relocation (ground truth — tester + independently reconfirmed)

During in-level gameplay, a contiguous data block is **relocated to higher
addresses**. To read the LIVE value of a global whose static (ELF/arena) address
is `A`:

| static range | in-level live location | contains |
|---|---|---|
| `0x1A8000 .. 0x1A9480` | **`A + 0x350`** (block A) | `g_nGameState` 0x1A8BB0 → live **0x1A8F00** |
| `0x1A9480 .. 0x1B1600` | **`A + 0x340`** (block B) | `g_gameTime` 0x1B1608 → live **0x1B1948** |
| everything else | `A` (unshifted) | PlayerStats 0x1A7A00+, g_pHeroMoby 0x18C0B0, g_pSkyData 0x1B2040, g_mobyClassTable 0x18B040, and 0x1B1600+ |

Reproduce / re-map with `tools/native/runtime/arena/detect_reloc_shift.py
[inlevel_dir]` — it slides the dump against the static ELF and recovers the shift
profile per 0x100 block. Confirmed boundaries: unshifted `<0x1A8000`, **+0x350**
`0x1A8000–0x1A9480`, **+0x340** `0x1A9480–0x1B1600`, unshifted `0x1B1600+`.

### Proof (my own dumps, both directions)
- `inlevel/` (real gameplay, RELOCATED): `g_nGameState` static 0x1A8BB0 = `"render
  s[etup]"` (string), reloc **0x1A8F00 = 0** (live state); `g_gameTime` static
  0x1B1608 = 0, reloc **0x1B1948 = 0x4DF6 (ticking)**.
- `inlevel_run2_load` (state-6 transition, UNRELOCATED): `g_nGameState` static
  0x1A8BB0 = **6** (live), reloc 0x1A8F00 = garbage; `g_gameTime` static 0x1B1608
  = **0x4F (ticking)**, reloc 0x1B1948 = 0.

So `inlevel/` is the canonical relocated gameplay layout; `run2_load` was a
pre-relocation snapshot (which is what fooled the static-only pass).

### The "render setup" / "rend" at g_nGameState, re-explained correctly
`g_nGameState` static 0x1A8BB0 reads the bytes of `"render setup"` (static rodata
at 0x1A8860) **because block A is relocated +0x350**: the loader's copy places
static-`0x1A8860` content at `0x1A8860+0x350 = 0x1A8BB0`. It is **not** a transient
spill — it is the persistent consequence of the +0x350 relocation. The real live
`g_nGameState` is at `0x1A8BB0 + 0x350 = 0x1A8F00` (= 0 during gameplay).

## Mechanism: NOT a gp-shift — the relocator is outside the boot ELF (likely a disc overlay)

A gp-base-shift hypothesis (gp 0x1AEFF0 → 0x1AF330) was **REFUTED** by an
independent Ghidra RE:
- **gp is set exactly once**, in `_start` (0x131C50: `move gp,a0`, a0=0x1AEFF0),
  and **never reassigned** anywhere in the boot ELF. No level-load/overlay/main-loop
  path shifts it. (The lone non-`_start` `lui gp,0x1001` is scratch-register reuse
  in a VU1 packet builder, not a gp reassignment.)
- The **resident** in-level accessors read/write the **static** addresses: ~19
  gameplay functions read `g_gameTime` via `lw 0x2618(gp)` = 0x1B1608; the clock
  writer `FUN_002F6110` writes 0x1B1608 **absolutely** (`lui v0,0x1b; sw …,0x1608`).
- `0x1A8F00` (live g_nGameState) has **zero** references in the boot ELF;
  `0x1B1948`'s only static refs are a coincidental GIF-template table.
- **No** memcpy/DMA/decompress of a ~0x9600 block into 0x1A8340, anywhere.

**Yet the relocated data is genuinely LIVE, not a frozen copy:** the dump at
0x1B1948 holds `g_gameTime`=0x4DF6 plus live floats and `0x9` — values that are
**all-zero in the ELF**, so something *writes* them at runtime. And nothing in the
boot ELF does.

**Therefore the gameplay code driving the captured frames is not the boot-ELF
resident engine** — the relocation is produced *outside* `SCUS_972.68`, consistent
with a **disc-loaded engine/level overlay linked +0x340/+0x350** (R&C's overlays,
the "DVP/VU overlays TODO" in CLAUDE.md). That overlay's code+data is not in the
boot ELF, so from this binary alone: the block-A +0x350 vs block-B +0x340 split
(the 0x10 insertion at 0x1A9480), the writer of 0x1A8F00, and completeness
(whether code/other segments outside the captured window are also relocated)
**cannot be determined.** I will not guess them.

> **⚠️ Implication worth surfacing (not yet confirmed):** if gameplay runs a
> +0x340-relocated overlay instance, then the boot-ELF resident gameplay functions
> we've been matching (StepMobyMotion 0x2B6000, UpdateGameState 0x2B5B38, …) may be
> a frontend/dormant instance, with the *running* in-level copies at a shifted
> base. This needs runtime proof before it changes any decomp assumption.

### Decisive next step (runtime — the tester's instrumentation can settle it)
Static analysis is exhausted. In PCSX2, during confirmed controllable gameplay,
set a **write-watchpoint on 0x1B1948 and on 0x1B1608** for one frame:
- which address the running clock actually writes tells us live-vs-dormant;
- the **PC** of the 0x1B1948 writer tells us whether it is resident `.text`
  (→ a relocating loader patched the absolute refs) or shifted `.text`
  (→ a relocated overlay instance). Either answer resolves the mechanism and the
  completeness question in one shot.

The **empirical un-relocation map below is ground truth regardless of mechanism**,
so the tester's seeding/validation is unblocked now.

## What the tester needs to do

The native arena (`arena_map.txt`) is laid out at **static** addresses. To seed it
from the in-level dump you must **un-relocate**: for a global at static `A`,

```
if 0x1A8000 <= A < 0x1A9480:  read dump at A + 0x350
elif 0x1A9480 <= A < 0x1B1600: read dump at A + 0x340
else:                          read dump at A
```

i.e. seed arena slot `A` from `dump[reloc(A)]`. Conversely, to validate native
output against the dump, compare native `A` to `dump[reloc(A)]`. Globals outside
`0x1A8000–0x1B1600` (PlayerStats, sky/hero/mobyClass pointers, the 0x1B1600+
camera/render block) are at their static addresses — no adjustment.

Liveness gate for capturing a clean gameplay seed (unchanged, still valid): read
g_nGameState at its **live** location `0x1A8F00` (== 0 for gameplay), g_gameTime at
`0x1B1948` (!= 0), g_pSkyData `0x1B2040` (!= 0, unshifted), g_pHeroMoby `0x18C0B0`
(!= 0, unshifted).

## Still valid from the first pass (these parts hold)

- **Capture geometry:** `inlevel/` globals window base **0x1A7000** (snapshot
  addr=1732608); input 0x138300+, sound 0x189E00+ (holds g_bPlayerMode 0x18C0D4,
  g_pHeroMoby 0x18C0B0), sky 0x1F4000+.
- **STATIC-RODATA over-capture (category 2):** a set of arena entries sit on static
  rodata string literals (`"render setup"`@0x1A8860, `"map level %d"`@0x1A9448,
  `g_vendorCaptionFmt`@0x1AD338, etc.) — const, never live state. Already handled:
  11 demoted to `data_const.txt` (the effect-diff exclude-list, commit fc200f2).
  NOTE these rodata addresses are themselves INSIDE the relocated block, so their
  live copy is at `A+0x350/+0x340` too — but being const, both copies are equal.
- **Unshifted live anchors:** PlayerStats 0x1A7A00+, g_pHeroMoby 0x18C0B0,
  g_pSkyData 0x1B2040, g_mobyClassTable 0x18B040 — valid at static addresses.

## Tooling
- `detect_reloc_shift.py [dir]` — recovers the shift profile (dump vs static ELF).
- `correlate_inlevel.py [--dir=] [--ranges] [--table]` — per-global value/class
  table (note: its "overlaid:span"/"zero" classes for the 0x1A8000–0x1B1600 block
  were the *symptom* of the relocation, now correctly explained above).

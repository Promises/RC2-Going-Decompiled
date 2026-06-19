# In-level global layout delta — RE deliverable (2026-06-19)

**For:** the tester's autonomous in-build harness, gated on making the captured
in-level seed (`tools/ee/eetest/state/inlevel/`) usable for native frame-path
effect-diff.

**Headline result (overturns the working assumption):** there is **NO structural
in-level global relocation**. The New-Game arena map (`arena_map.txt`, 441
globals) is **structurally valid in-level** — at a *settled* gameplay instant,
30+ named globals read sane live values at their exact mapped addresses. The
"layout differs / MIXED" behaviour the harness saw is **an artifact of WHEN the
canonical `inlevel/` seed was captured** (a transient mid-load instant), not of
where the globals live.

Reproduce all of this with `tools/native/runtime/arena/correlate_inlevel.py
[--dir=inlevel|inlevel_run2_load] [--ranges] [--table]`.

## Capture geometry (from snapshot.json)

`inlevel/` regions (EE addresses): globals `0x1A7000..0x1BC000` (86 KB), input
`0x138300..0x138700`, sound `0x189E00..0x18DE00`, sky_obj `0x1F4000..0x1FC000`.
(Base is **0x1A7000**, i.e. snapshot `addr=1732608` — easy to misread as
0x1A6400 by hand; the script reads it from JSON.)

## The three categories of the delta

Correlating arena globals against the ground-truth dumps + the **static boot ELF**
(`extracted/usa/SCUS_972.68`, single LOAD seg `0x100080`, filesz==memsz so the
whole `0x100080..0x352D08` is PROGBITS) splits every arena entry into:

### 1. LIVE — valid in-level at the mapped address (the large majority)
Confirmed by `run2_load` (settled, state=6): sane values at the New-Game
addresses. Examples (run2_load):
- `g_mobyTableBase`=0x858C40, `g_mobySpawnStart`=0x858D40, `g_mobyTableEnd`,
  `g_mobyAuxBlockBase`, `g_sceneActorMobys`=0x858D40
- `g_frameArenaBase`=0x354000, `g_frameDmaCursor`=0x470500,
  `g_renderTaskList`=0x4D6000, `g_deferredSegment2Tag`=0x467540
- `g_pSkyData`=0x6FEB00, `g_pSkySegmentOpenTag`=0x454090
- `g_guiInstance`/`g_pGuiManager`=0x1FB8000, `g_debugMallocCursor`=0x1FA8A00
- vram cursors `0x1A72D0..0x1A730C`, HUD asset pointers `0x1B1808..0x1B1864`
- `g_pHeroMoby` (0x18C0B0)=0x1960680, `g_mobyClassTable` (0x18B040)=0x1984C80,
  `g_soundBankHandles` (0x189E00)=0x193E00 (sound region)

These need **no exclude** when the seed is captured at a settled frame.

### 2. STATIC-RODATA over-capture — never a live global (persistent, exclude always)
A handful of arena_map entries were placed on addresses that are actually
**string-literal / const rodata** in the ELF, not mutable globals. They read the
same constant in menu, in-level, and the static ELF. The arena seed/native build
should treat these as constants, not state. Confirmed static rodata in this span:
`"error: sceSifBindRpc"`@0x1A7500, `"render setup"`@0x1A8860 & 0x1ACE10,
`"loaders.cpp"`@0x1A91F0, `"map level %d"`/`"map level 0"`@0x1A9448, the vendor
caption format `g_vendorCaptionFmt`@0x1AD338. (This is also an `arena_map.txt`
quality note — see "Follow-ups".)

### 3. CAPTURE-INSTANT noise — only in the canonical `inlevel/` seed (re-capture to fix)
The `inlevel/` seed was taken at a **transient mid-load instant**. Two effects,
both absent from `run2_load`:
- **Rodata block shifted +0x340.** In `inlevel/`, the static rodata string block
  appears ~+0x340 above its ELF/settled position (`"map level 0"` at 0x1A9788 vs
  static 0x1A9448; `"render setup"` lands on **0x1A8BB0 = g_nGameState**). The
  loader was caught mid-relocation/staging. In `run2_load` those strings sit at
  their exact static addresses — i.e. settled/intact.
- **Gameplay globals not yet populated.** `g_gameTime` (0x1B1608)=0, the moby
  table pointers=0, all camera state (0x1B5180+)=0, `g_frameDmaCursor`=0,
  `g_mobySpawnCredit`=0x87654321 (poison). All are **live at the same addresses
  in `run2_load`** (g_gameTime=79, g_mobySpawnStart=0x858D40, camera floats live).

## The `g_nGameState` = "rend" anomaly — explained

`g_nGameState` (0x1A8BB0) is a **real global**, correctly mapped: static ELF init
there is `00 00 00 00 | 00 00 00 00 | fe ff ff ff | 02 00 00 00` (state/prev/
pending/soundDefs), menu reads **4**, `run2_load` reads **6** (matching its
snapshot `state=6`). The `inlevel/` seed reads `0x646E6572` ("rend", from the
"…**rend**er setup" string) **only because** the +0x340-shifted rodata block
transiently overlapped 0x1A8BB0 at that capture instant. It is a runtime spill of
a const string over the global, not the global's real in-level location.

## Recommendation (unblocks the tester cleanly)

**Re-capture the canonical seed at a SETTLED gameplay frame**, gated on a liveness
predicate so the harness never snapshots a mid-load instant again:

```
seed_is_live  ==  (u32@0x1A8BB0 < 16)            # g_nGameState is a small enum 0..9
              &&  (u32@0x1B1608 != 0)            # g_gameTime is ticking
              &&  (u32@0x1B2040 != 0)            # g_pSkyData set (sky resident)
```

At such an instant the New-Game arena map is **directly valid** — seed every
captured arena global from the dump, exclude only the category-2 static-rodata
constants. No structural remap, no per-address shift, no big exclude-list.
`inlevel_run2_load` already demonstrates a near-clean settled capture (only the
1 static format-string flagged). State 0 (pure gameplay) would be ideal; state 6
already gives a fully-populated layout.

If the existing `inlevel/` seed must be used as-is, it is only trustworthy for the
**category-1 always-resident pointers** above; the rest is mid-load noise and must
be excluded — but the right fix is re-capture, not a 70-block exclude-list (which
would be encoding a capture artifact as if it were structure).

## Follow-ups
- **arena_map quality:** category-2 entries are static const/rodata mislabelled as
  globals; worth demoting in `data_globals.txt` / the gen_arena seed so the native
  build doesn't treat constants as mutable state.
- **Optional Ghidra corroboration (dispatched separately):** identify the loader
  code that relocates/stages the rodata block (the +0x340 shift) so the harness
  can also key its capture off "loader idle", and confirm no other in-level
  remap exists outside the captured window.

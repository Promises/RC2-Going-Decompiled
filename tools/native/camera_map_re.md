# Camera-map RE — answer to the in-level camera handoff (2026-06-19)

**In reply to** `tools/ee/eetest/CAMERA_MAP_EVIDENCE.md`. Source of truth for the
static struct: the matched `going-decompiled/src/usa/text/16E980.c` (Camera /
CameraSysState / HeroCamMotion typedefs). Correlated against the tester's
`inlevel_combat` capture.

## TL;DR
- The **HeroCamMotion sub-block at cs+0x1A0 is the one fully-static, in-level-live
  part** — it maps 1:1 to every empirical anchor the tester reported (table below).
  This validates: `TrackHeroMotionForCamera` (boot ELF) writes it in-level.
- `cs+0x190` (`activeCamera`) in-level reads **0x089C1000, which is an UNMAPPED EE
  address** — not a valid `Camera*`. So the frontend "`activeCamera` is a pointer
  at +0x190" mapping **does not hold in-level**; the live camera transform
  (activeCamera, the Camera objects, the matrix/pos heads) is **overlay-resident**
  and not reconstructable from the boot ELF.
- **Native-reclaim verdict:** `UpdateCamera` and the activeCamera-deref cluster are
  **TRAPS to shim/no-op** in the native frame-path (they fault dereferencing an
  overlay pointer). The hero-motion tracking validates. (Feeds the trap list.)

## Ask 1 — what `g_activeCamera` points to, and what `0x089C1000` is

### The static (frontend) types
```
Camera (0xA0 bytes)                 CameraSysState  (g_cameraState = 0x1B5180)
  +0x30  Vec4 pos   (-> cs+0x140)     +0x000  sentinel-init header (0x7FFF0000 pattern)
  +0x7D  u8  unk7D  (clr on activate) +0x140  Vec4 camPos     (copied from activeCamera+0x30)
  +0x7E  s16 unk7E  (=1 on activate)  +0x190  Camera* activeCamera   <-- the faulting field
  +0x84  s16 configIndex              +0x194  Camera* prevCamera
  +0x86  s16 type   (slot key)        +0x1A0  HeroCamMotion  (see Ask 2)
  +0x8C  s16 modeId (vtbl index)      +0x268/+0x26C  fadeBlackTarget / fadeBlackRate
  +0x8E  s16 snapFlag                 +0x3D4..+0x3E9  FOV / fade-mode block
```
`UpdateCamera` reads `active = cs->activeCamera` and **dereferences it on every
branch** (transition==3 passes `active` to `ApplyCameraTransition`; the else branch
does `cs.camPos(+0x140) = active->pos(+0x30)`) — so an invalid `activeCamera` faults
regardless of path. In the **frontend**, `activeCamera` is a valid `Camera*`, set by
`SwitchActiveCamera` (0x2705E0, store @0x2707E8). In-level it is not (below).

### `0x089C1000` is an UNMAPPED EE address
Checked against every EE address space:

| space | range | 0x089C1000 in it? |
|---|---|---|
| main RAM | 0x0000000–0x2000000 | no (it's 137.8 MB, RAM is 32 MB) |
| EE I/O regs | 0x10000000–0x10002000 | no |
| VU0/VU1 mem | 0x11000000–0x1100C000 | no |
| GS priv regs | 0x12000000–0x12002000 | no |
| scratchpad | 0x70000000–0x70004000 | no |
| kseg0 mirror | 0x80000000+ | no (below 0x80000000) |

`0x08xxxxxx` is **not a defined PS2 EE region** — dereferencing it faults on native
*and* would fault on real hardware. So `cs+0x190` in-level is **not** a
frontend-style `Camera*`. The tester's own read confirms it "does not read as a
clean pointer": the 4 words at cs+0x190 are `089C1000 8013089C 089C1000 8014089C`
— interleaved, with `0x089C` recurring (looks like packed/overlay data, not a
pointer). The neighbor `0x8013089C` *would* decode as a kseg0 pointer
(→ phys 0x13089C) but the field as a whole is not a coherent pointer.

**Conclusion:** the in-level (overlay) camera struct has a **different layout at
+0x190** than the frontend map. The live in-level `activeCamera` / `Camera` objects
are managed by overlay code and live in overlay memory — *where* is not
determinable from the boot ELF (the overlay is disc-loaded, not in `SCUS_972.68`;
same terminus as the engine-global relocation finding). The region is NOT relocated
(the hero-track anchor below sits at its exact static offset, proving cs+0x190 is
genuinely activeCamera's slot — it just holds overlay-layout data in-level).

## Ask 2 — the camera-state struct, and the live hero-motion sub-block

The tester's live sub-block (0x1B5304+) is the **`HeroCamMotion` struct at
cs+0x1A0** (0x1B5320), written every frame by `TrackHeroMotionForCamera`. Every
empirical anchor maps exactly:

| tester anchor | field | offset | EE addr |
|---|---|---|---|
| "hero facing 0x1B5340" | `facingDir` (Vec4) | cs+0x1C0 | 0x1B5340 ✓ |
| "velocity 0x1B5390" | `velocity` (Vec4) | cs+0x210 | 0x1B5390 ✓ |
| "fwd/lat 0x1B53A0" | `lateralDir` (Vec4) | cs+0x220 | 0x1B53A0 ✓ |
| "0x1B53B0" | `forwardProj` (Vec4) | cs+0x230 | 0x1B53B0 ✓ |
| "0x1B53C4" | `lateralSpeed` (f32) | cs+0x244 | 0x1B53C4 ✓ |
| "0x1B53C8" | `forwardSpeed` (f32) | cs+0x248 | 0x1B53C8 ✓ |
| "orient ring 0x1B53CC" | `orientZRing[5]` | cs+0x24C | 0x1B53CC ✓ |

(Full field list: smoothX/Y/Height/rawZ/heightVel at cs+0x1A0..0x1B0; facingDir,
facingRingCur/Prev, facingVel, prevPos, velocity, lateralDir, forwardProj at
cs+0x1C0..0x230; speed/lateralSpeed/forwardSpeed at cs+0x240/4/8; orientZRing[5]
at cs+0x24C. Struct ends ~cs+0x260 — matches the tester's "+0x250 onward = 0".)

### Why the camera HEADS read zero in-level
- `cs+0x000..+0x180` is the **sentinel-init header** (the `0x7FFF0000`-every-other-
  word pattern the tester saw) — `g_cameraState` 0x1B5180 = first word = 0 is
  expected (initialised, not "wrong").
- `g_cameraPos` (cs+0x140) and `g_cameraMatrix` are the **overlay-managed** live
  camera transform — zero at the frontend offsets because the in-level transform
  lives in the overlay's camera object (via the activeCamera chain), not at these
  frontend cs offsets. (Note: the symbol names `g_cameraPos`/`g_cameraRot`/
  `g_cameraMatrix`/`g_cameraSlots` are approximate frontend offsets; only the
  HeroCamMotion sub-block is confirmed in-level-live.)

## Native frame-path implication (for the trap list)
The activeCamera-deref cluster CANNOT run against the in-level seed — it
dereferences `cs->activeCamera` (= unmapped 0x089C1000) → fault:
- `UpdateCamera` (16E980.c) — `active->pos` deref. **TRAP / shim.**
- The 4 `activeCamera` readers: `FUN_002E4C24`, `FUN_002F1F60`, `FUN_0027A100`,
  `FUN_002F03D0`. **TRAP / shim.**
- `CallCameraPollHandler` / the `unk7D/unk7E` activation writes (16E980.c ~L655)
  and the `CopyQwords(g_cameraSnapshot, activeCamera, 0xA0)` snapshot. **TRAP / shim.**
- **Reconstructable / validates:** `TrackHeroMotionForCamera` and the HeroCamMotion
  sub-block — it reads `g_heroPos`/`g_heroFacingDir` (unshifted hero globals) and
  writes cs+0x1A0 (static offset, confirmed live). Native can run it.

So in the native frame-path the camera transform should be treated as
**externally-supplied state** (seed cs+0x140/matrix from the dump, or no-op the
camera tick), while the hero-motion tracking can be computed natively. The tester's
trap list will confirm which exact functions fault — this RE says the boundary is
"anything that follows cs->activeCamera" vs "the HeroCamMotion math".

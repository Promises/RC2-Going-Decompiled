# Inject-Spec: front-end New-Game navigation → in-level (USA SCUS_972.68 v2.00)

Purpose: let the tester script controller-input injection to drive the boot
front-end menu into an in-level state, so it can capture the in-level seed.

Confidence tags: **[ASM]** = assembly/decompile-proven; **[EMP]** = tester should
confirm at runtime (static analysis under-determines it).

---

## 0. TL;DR minimal sequence

1. Inject on the **primary pad buffer** the menu actually reads:
   `g_padButtonsPressed = 0x00138344` (edge/just-pressed) and
   `g_padButtonsHeld = 0x00138340`. **NOT 0x138320.** [ASM]
2. After boot reaches the front-end (`g_nGameState==3`), a save-data/system
   prompt overlay (`g_nGameState==4`) is auto-raised ~1 s in. **Press CROSS
   (0x40)** to acknowledge/dismiss it. [ASM mechanism / EMP exact timing]
3. On the root menu (screen 6): **press CROSS (0x40)** on the start/New-Game item
   → transitions to the save/level list (screen 8). [ASM chain / EMP which item]
4. On screen 8: **UP (0x1000) / DOWN (0x4000)** to pick a slot/level, then
   **CROSS (0x40)** → `RequestLevelExit(index,1)` → the level loads. [ASM]

A no-navigation **CROSS → (dismiss) → CROSS → CROSS** still loads *a* level
(screen-8 index seeds to 0x18, the special last entry → `RequestLevelExit(0x19,1)`),
which is enough to reach an in-level state for the seed. [ASM]

---

## 1. Pad input — target buffer + bit map  [ASM]

Port-0 pad buffer base = `0x00138180` (`InitControllers @0x2B9468`). The
front-end menu nav/confirm handlers read **`g_padButtonsPressed = 0x00138344`**
(just-pressed/edge) and **`g_padButtonsHeld = 0x00138340`**. The state-4 overlay
machine reads the sibling edge field **`0x00138324`**.

Proven via `TickActiveMenuScreen @0x2CCAA0`: it loads `a0 = *(0x00138344)`, masks,
and dispatches into `LevelSelectListHandleInput` with that value. The repo's
CONFIRMED symbols agree: `g_padButtonsPressed=0x138344`, `g_padButtonsHeld=0x138340`.

> Correction to earlier guidance: a prior pass said "use 0x138320, not 0x138344."
> That is **inverted** for menu nav — menus read 0x138344. (0x138320/0x138324 are
> different fields in the same 0x138180 buffer, used by the attract/boot phase;
> the tester may still need 0x138320 to skip attract → menu, but **menu
> navigation consumes 0x138344**.)

Button bit map (libpad layout, verified against `UpdateQuickSelectWheelInput
@0x28C9D0` dpad nibble `&0xF000` and the confirm handlers):

| Button | Bit | Role in menus |
|--------|--------|---------------|
| UP | `0x1000` | move selection up |
| DOWN | `0x4000` | move selection down |
| LEFT | `0x2000` | (map/horizontal) |
| RIGHT | `0x8000` | (map/horizontal) |
| **CROSS (X)** | **`0x40`** | **confirm / activate** |
| TRIANGLE | `0x10` | back / cancel |
| CIRCLE | `0x20` | (context) |
| SQUARE | `0x80` | (context) |
| START | `0x800` | — |
| SELECT | `0x100` | — |
| START+SELECT | `0x900` | exit chord |

UP/DOWN/LEFT/RIGHT/CROSS/TRIANGLE are [ASM]. The other single bits are inferred
from the exit chords (0x900/0x910) and are consistent but not each independently
bottomed out [EMP]. CROSS=0x40 is [ASM] (matches the tester's anchor).

---

## 2. Start-game call + full chain  [ASM mechanism; EMP screen-6 item index]

The actual "load a level" primitive:

```
RequestLevelExit(levelId, commitSave=1)   @0x2896D8
  -> g_nLevelExitRequested = 1            (0x1A8B84)
  -> g_nLevelExitDestination = levelId    (0x1B1600)
  -> CommitProgressCheckpoint(0, levelId)
```

`g_nLevelExitRequested=1` breaks the `while (g_nLevelExitRequested==0)` loop in
`LoadLevelAndInitHealth @0x26EDE8`, which then tears down the menu and loads the
destination level (in-level entry confirmed by `g_nHealth=1` init). **arg1 = the
level/destination id.** [ASM]

### Chain from front-end entry to that call

```
boot -> RequestGameStateChange(3,1,6,0,0) -> g_nGameState=3, screen id 6
        (screen 6 = ROOT MENU object DAT_00259438; MenuScreenLoad @0x2CB0A0 case 6)
   |
   |  [root-menu widget, CROSS 0x40]
   v
MenuScreenDoAction(opcode,arg) @0x2D67C0    (front-end action dispatcher)
   opcode 6/8/10/0xb = push/pop/swap to another menu screen
   opcode 5          = raise state-4 overlay (save/system prompt), mode 1
   -> transitions current screen to screen id 8
   |
   v
screen 8 = SAVE/LOAD + level list  (g_MenuScreen_SaveLoadList @0x258FF8;
        MenuScreenLoad case 8 installs scroll callbacks
        LevelSelectListHandleInput @0x2CC4C8 / LevelSelectListRender)
   |
   |  [UP 0x1000 / DOWN 0x4000 to select; CROSS 0x40 to confirm]
   v
LevelSelectListHandleInput: CROSS -> RequestLevelExit(g_nLevelSelectListIndex, 1)
   -> level loads.
```

### Screen-6 root-menu widget array  [ASM structure; EMP exact "New Game" index]

`MenuScreenUpdate @0x2CBAB8` iterates up to 14 widgets at
`g_pCurrentMenuScreen + 0xEC`, vtable **+0x8 = update**, **+0xC = activate**, for
whatever screen is current. The screen-6 root object `DAT_00259438` carries a
static widget array there; its slot-0 widget logic is
`SaveMessageWidgetTick @0x2D6248` (vtable `0x1AA618`), which on CROSS reads the
selected GUI-list entry and calls `MenuScreenDoAction(opcode,arg)`.

**The exact +0xEC index of the "New Game" item is NOT statically label-identified**
[EMP]: menu labels come from external localized text tables (no "NEW GAME" string
in the ELF), so the item is identified structurally by its action opcode, not by
text. The robust path for the tester: on the root menu, **CROSS on the selected
item and observe** which `MenuScreenDoAction` opcode fires / which screen id
becomes current (expect a push to screen 8). If the default-selected item isn't
the start item, step the selection with UP/DOWN first. The *mechanism* downstream
(screen 8 → `RequestLevelExit`) is [ASM] regardless of which root item routes there.

> Note: screen 8 is the **save/load list** (24 entries, `g_nLevelSelectListCount=0x18`).
> On a fresh boot with no save, selecting a slot starts a new game and loads the
> first level via `RequestLevelExit`. The galactic-map path is equivalent:
> `GalacticMapConfirmTravelInput @0x2D65D8` CROSS → `RequestGameStateChange(6,2,1,destLevel)`
> → `StartTravelToLevel @0x2E7EE8`, or `RequestLevelExit(level,1)` for unlocked planets.

### Selection seeding gotcha  [ASM]

`MenuScreenLoad` case 8 seeds **`g_nLevelSelectListIndex = 0x18` (@0x2109F4)** —
the last/special entry of the 24-entry list. So:
- CROSS-on-entry (no nav) hits the special-cased branch
  (`entry==0xb47 && marker[0x19]==0`) → `RequestLevelExit(0x19,1)`: loads level
  `0x19`. Good enough to reach *an* in-level state.
- To pick a specific planet, press **UP** N times before CROSS.

---

## 3. State 3 vs 4, the ~1 s auto-transition, the ~200 s idle  [ASM mechanism]

Corrections to the scoping anchors (assembly-proven three ways: the
`g_nGameState` switch in `LoadLevelAndInitHealth`, the state map on
`UpdateGameState @0x2B5B38`, and the case bodies):

- **Front-end menu = `g_nGameState==3`.** (`RequestMenuScreenChange`,
  `MenuScreenLoad`, `TickFrontEndScreenMachine @0x2CB860` run under state 3.)
- **`g_nGameState==4` = the in-game PAUSE / SYSTEM-MESSAGE overlay**
  (`UpdateMenuOverlayStateMachine @0x287140`, enter `EnterMenuOverlayMode @0x286270`).
  It is **not** the main menu and has no New-Game widget.

### What the tester saw as "state 3 auto → state 4 in ~1 s"

The state-3 front-end **auto-raises a state-4 overlay on top of itself**: in the
front-end tick (`TickFrontEndScreenMachine @0x2CB860`), when
`(DAT_001a7424 & 1) && DAT_001f27c0==4 && DAT_001b1604>=8`, it runs
`RequestGameStateChange(4,1,1,g_pCurrentMenuScreen,0)` — the same call as
`MenuScreenDoAction` **opcode 5** (raise overlay **mode 1 = save-data /
system-message prompt**). So the front-end menu (state 3) is still underneath;
state 4 is a modal prompt layered on it ~1 s after entry. [ASM]

### The ~200 s idle

That state-4 overlay (mode 1) is the **save-data/system-message prompt**
(`UpdateMenuOverlayStateMachine` case 1), gated on `(0x138324 & 0x40)` CROSS /
`(& 0x10)`. It is **dismissible**, not a demo/attract reel: pressing **CROSS
(0x40)** acknowledges it and returns control to the state-3 menu underneath.
No hard auto-reset timer that fights input was isolated; the visible counters
(`DAT_001bac5c/60`) are per-mode debounce/hold, not a reset. [ASM that it is a
dismissible prompt; EMP the exact ~200 s expiry/what it resets to]

**Sequencing guidance:** do not wait the overlay out — **CROSS to dismiss it
first**, then drive the root menu. Inject CROSS into `0x138324`/`0x138344` (both
get written from the same source each frame in practice; the overlay reads
`0x138324`, the menus read `0x138344`).

---

## 4. Empirical residuals for the tester to confirm at runtime

1. **Which root-menu (+0xEC) item is "New Game"** and the exact `MenuScreenDoAction`
   opcode it fires (expect a 6/8 push to screen 8). Observe `g_nLastMenuScreenId`
   / `g_pCurrentMenuScreen` after CROSS.
2. **Whether the live front-end re-seeds `g_nLevelSelectListIndex`** (it loads as
   0x18; drive the cursor explicitly rather than assuming index 0).
3. **The first-level destination value** in `g_levelSelectEntries[0]` (entries are
   runtime-populated; static template read as zeros). CROSS→CROSS reaching id 0x19
   is the empirical fallback for "reach *an* in-level state."
4. **The ~200 s overlay expiry behavior** (what it auto-resets to) — avoided by
   dismissing with CROSS.

---

## 5. Symbol anchors (USA SCUS_972.68)

| Symbol | Addr | Role |
|--------|------|------|
| `g_padButtonsPressed` | 0x00138344 | menu just-pressed (inject here) |
| `g_padButtonsHeld` | 0x00138340 | menu held |
| (overlay edge field) | 0x00138324 | state-4 overlay input |
| `RequestLevelExit` | 0x002896D8 | start-game: sets exit-requested + dest |
| `g_nLevelExitRequested` | 0x001A8B84 | =1 breaks the level loop → load |
| `g_nLevelExitDestination` | 0x001B1600 | destination level id |
| `LoadLevelAndInitHealth` | 0x0026EDE8 | top frame loop (state switch) |
| `MenuScreenLoad` | 0x002CB0A0 | screen loader (case 6 root / case 8 list) |
| `MenuScreenDoAction` | 0x002D67C0 | front-end action dispatcher (opcodes) |
| `MenuScreenUpdate` | 0x002CBAB8 | iterates +0xEC widgets (vtable +8/+0xC) |
| `SaveMessageWidgetTick` | 0x002D6248 | screen-6 root widget slot-0 (CROSS → DoAction) |
| `LevelSelectListHandleInput` | 0x002CC4C8 | screen-8 list (CROSS → RequestLevelExit) |
| `g_nLevelSelectListIndex` | 0x002109F4 | screen-8 selection (seeds 0x18) |
| `g_MenuScreen_SaveLoadList` | 0x00258FF8 | screen-8 object |
| `DAT_00259438` | 0x00259438 | screen-6 root-menu object |
| `TickFrontEndScreenMachine` | 0x002CB860 | state-3 per-frame tick (auto-raises overlay) |
| `UpdateMenuOverlayStateMachine` | 0x00287140 | state-4 overlay machine (save prompt) |
| `EnterMenuOverlayMode` | 0x00286270 | state-4 enter |
| `UpdateGameState` | 0x002B5B38 | state-transition dispatcher |
| `g_nGameState` | 0x001A8BB0 | 3=front-end menu, 4=pause/overlay |

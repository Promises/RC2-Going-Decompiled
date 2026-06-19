# Runtime naming-confirmation batch (W2) — for the tester's behavior_probe.py

10 high-confidence **CONFIRMED** gameplay globals, each with the injected input
that changes it and how. All observable via injected gameplay (movement onto
pickups, firing, vendor/menu navigation). USA (SCUS_972.68) addresses; values are
the in-RAM globals. Format: `name | addr | type | expected runtime behavior`.

| name | addr | type | injected input → how the value changes |
|------|------|------|----------------------------------------|
| `g_boltCount` | `0x001A7A00` | u32 | Move hero onto a bolt pickup → **increments** by the bolt denomination (CollectBolts). Vendor purchase → **decrements** by item price. Monotonic-up on pickup — cleanest observable. |
| `g_health` | `0x0018C2EC` | s32 | Take enemy/hazard damage → **decreases** by the hit amount. Nanotech (green) pickup → **increases**, clamped to `g_maxHealth`. |
| `g_maxHealth` | `0x001A7A08` | s32 | Collect a Nanotech health upgrade → **steps up** one cube; otherwise constant within a level (it is the heal-clamp ceiling + HUD-bar max). |
| `g_weaponAmmo` | `0x00139688` | s32[0x38] | Fire the equipped weapon → slot `[itemId]` (base + itemId*4; e.g. id9 at `0x1396AC`) **decrements** per shot, clamp ≥0. Pick up / buy ammo → **increases** up to `g_weaponTable[+0x8E]` capacity. |
| `g_inventoryOwned` | `0x001A7B00` | u8[0x38] | Acquire an item (vendor buy / GiveInventoryItem) → byte `[itemId]` flips **0→1**. |
| `g_equippedItemSlots` | `0x001A73B8` | u32[8] | Assign an item in the quick-select wheel → that slot's u32 **becomes the equipped itemId**. Default loadout seeds slot0=`0x1E`, slot1=`0x2A`. |
| `g_itemEquippedSlot` | `0x00139568` | u8[0x38] | Buy a weapon-variant upgrade (SetWeaponUpgradeSlot) → byte `[itemId]` **remaps** from identity to the new variant slot. |
| `g_skillPointFlags` | `0x001A7A68` | flag array | Complete a skill-point challenge → its flag sets **0→1** (gates cheat unlocks). |
| `g_nGameState` | `0x001A8BB0` | s32 | UI navigation flips it deterministically: vendor→**5**, pause/overlay→**4**, front-end/map→**3**, cinematic→**1**, in-level→**0**. Most deterministically drivable. |
| `g_nBoltCounterDisplayed` | `0x001B18C8` | u32 | Collect bolts → this **lags `g_boltCount` and rolls toward it** over frames (target at +4, delta accum at +8). Distinguishes the animated HUD counter from the instantaneous `g_boltCount`. |

Notes for the probe:
- All ten are already CONFIRMED in `going-decompiled/symbol_addrs/usa/symbol_addrs.txt`; this file restates them with the *runtime-observable* contract so the probe can assert (input → delta) rather than just name them.
- EU twins: `g_weaponAmmo`/`g_weaponXp`/`g_inventoryOwned`/`g_equippedItemSlots` are EU-anchored via the Made-in-Slovakia cheat pnach (same absolute addresses where noted); the rest sit in the data/globals region with the usual +0x80-class relocation — confirm the EU address before probing on the PAL build.

# W2 runtime naming-confirmation batch — for behavior_probe.py

10 highest-confidence CONFIRMED observable gameplay globals (USA SCUS_972.68).
One per line: `<name>  <addr>  — <trigger: injected input that should change it>`.
Tester designs the actual injected input.

g_boltCount              0x001A7A00  — collect a bolt pickup → increases (vendor purchase → decreases)
g_health                 0x0018C2EC  — take enemy/hazard damage → decreases (nanotech pickup → increases)
g_maxHealth              0x001A7A08  — collect a Nanotech health upgrade → increases one cube
g_weaponAmmo             0x00139688  — fire the equipped weapon → slot[itemId] decreases (base+itemId*4)
g_inventoryOwned         0x001A7B00  — acquire an item (vendor buy) → byte[itemId] flips 0→1
g_equippedItemSlots      0x001A73B8  — assign a quick-select wheel slot → slot u32 becomes the equipped itemId
g_itemEquippedSlot       0x00139568  — buy a weapon-variant upgrade → byte[itemId] remaps to the new slot
g_skillPointFlags        0x001A7A68  — complete a skill-point challenge → its flag flips 0→1
g_nGameState             0x001A8BB0  — open vendor→5 / pause→4 / front-end→3 / cinematic→1 / in-level→0
g_nBoltCounterDisplayed  0x001B18C8  — after a bolt pickup → rolls toward g_boltCount over frames

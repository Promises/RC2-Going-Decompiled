#include "common.h"

/*
 * cod/033970 (EU mirror) — core.text tail after the carved 989snd sub-TU
 * cod/0321A0, the EU sibling of USA cod/033970 (0x1339F0..). Begins at the
 * func_001339F0-equiv (EU 0x133A50 / USA 0x1339F0, delta +0x60). Built at the
 * default -O2 -G0.
 *
 * EU<->USA function map (delta +0x60):
 *   func_00133A50 = USA func_001339F0  (indexed byte copy)
 *   func_00133A88 = USA CdReadSync     (sync CdStartRead descriptor)
 *   func_00133AD8 = USA LoadDiscToc    (MATCHED — region-agnostic body)
 *   func_00133B00 = USA LoadLevelToc   (3-operand mult wall)
 * g_discToc is at EU 0x0014B5C0 (USA 0x0014B540, delta +0x80).
 */

extern u8 D_0014B5C0[]; /* g_discToc (EU) — master disc asset directory */
extern s32 func_00133A88(s32 arg0, s32 arg1, void *arg2); /* CdReadSync (EU) */

/* func_001339F0 (USA): indexed byte copy — the delay-slot store scheduling wall
 * (see USA cod/033970). Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/033970", func_00133A50);

/* CdReadSync (USA func_00133A88 here): stack descriptor {0x20,1,0,0} + CdStartRead
 * then the snd service trio — the early sd-save / jal-delay-slot scheduling wall
 * (see USA cod/033970). Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/033970", func_00133A88);

/**
 * LoadDiscToc (USA name; EU func_00133AD8) - synchronously read the global disc
 * TOC: 0xB sectors from LBA 0x3E9 into g_discToc (EU D_0014B5C0) via CdReadSync,
 * forwarding its return value. The value-return keeps ee-gcc from
 * sibling-call-optimising into a tail jump. Region-agnostic body, byte-exact in
 * both builds.
 */
s32 func_00133AD8(void) {
    return func_00133A88(0x3E9, 0xB, D_0014B5C0);
}

/* LoadLevelToc (USA func_00133B00 here): per-level TOC refresh — blocked by the
 * 3-operand `mult` (R5900 mult-rd) this cc1 never emits. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/cod/033970", func_00133B00);

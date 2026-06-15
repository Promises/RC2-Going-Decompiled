#include "common.h"

/*
 * EU text/19FC78 - REGION-AXIS twin of USA text/1A00F0 (moby render/anim/grid +
 * ammo-drop + bestiary TU). Ported verbatim from src/usa/text/1A00F0.c across
 * the REGION axis (2026-06-15); the matched body is region-agnostic - only the
 * unit-local callee name is retargeted to its EU address. Built -O2 -G8
 * -fno-gcse (mirrors the USA unit).
 */

extern void FillMemory32(void *dst, u32 pattern, s32 len);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_0029FCF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_0029FD50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_0029FE10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_0029FEE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_0029FFE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0060);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0200);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0320);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A03B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0448);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A04A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A04E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A05E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0680);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0708);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A07A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A07B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A08A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0978);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0A08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0B40);

/* Clears 0x3C0 bytes of the moby scratchpad block at 0x70003A00 to 0x40000000.
 * The empty-asm guard suppresses cc1's sibling-call (tail-jump) so the original
 * jal + frame is reproduced. USA twin: func_002A1000. */
void func_002A0B88(void) {
    FillMemory32((void *)0x70003A00, 0x40000000, 0x3C0);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0BB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0BE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0C10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0CC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0D40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0D80);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0DF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0E28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0E48);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0E78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0EA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0F18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0F68);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1280);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1408);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1568);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A15C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1648);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1AC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1B10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1D58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1E70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A2118);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A2E30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A3B08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A47B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A4908);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A4A70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A54D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A5780);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A5860);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A7038);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A7174);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", PickLowAmmoWeaponForDrop);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A74D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A7650);

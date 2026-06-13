#include "common.h"

/*
 * text/1D54C0 — front-end / pause-menu screens band, part B (vaddr
 * 0x2D5540..0x2DFFFF). Carved out of the big text/1B21E8 asm tile as
 * ranked-carve-pipeline pick #5 ("menu-screens"); part A is text/1CA080.
 * This part holds the Galactic Map screen, the text-table streamer, the menu
 * background-image loader, and the screen-command (MenuScreenDoAction)
 * forwarding wrappers.
 *
 * Built at -O2 -G8 -fno-gcse (later SN cc1 TU model — see the part-A header for
 * the full -G8 extern-sizing rules). Walls left as INCLUDE_ASM (per function):
 * the 8-byte-packed callee-save wall (2+ GPR saves), the delay-slot store
 * scheduling wall (a lone global store + return-0 the original keeps OUT of
 * the jr delay slot while cc1 sinks it in), pad-load reorder, register
 * coloring, switch/jtbl, and handwritten frameless stub fragments.
 */

/* 2D draw-batch begin/end fence used by every menu draw function. */
extern void Begin2dDrawBatch(s32 mode);
extern void End2dDrawBatch(void);

/* Per-screen draw helpers in the preceding text/1A00F0 asm band. */
extern void func_0029D958(void);
extern void func_0029D8E8(void);
extern void func_0029D218(void);
extern void func_0029DD10(void);
extern void func_0029D1A8(void);

/* Forwarding-wrapper target. */
extern void func_002DF1B8(s32 mode);

/* Front-end / pause screen-action dispatcher (op, arg, out-flag ptr). */
extern s32 MenuScreenDoAction(s32 op, s32 arg, void *outFlag);

/* A menu command record: opcode at +2 (s16), arg at +4 (s32). */
typedef struct MenuCmd {
    u8  _pad0[2];
    s16 op;     /* 0x2 */
    s32 arg;    /* 0x4 */
} MenuCmd;

/* gp-addressable globals read via the assembler-absolute macro (see header). */
__asm__(".extern D_1A7BA8, 16");
__asm__(".extern g_sndChannelVolumes, 16");
extern s32 D_1A7BA8;              /* saved sound-channel volume snapshot */
extern s32 g_sndChannelVolumes[]; /* sound-channel volume table */

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5540);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5A10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5A48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5C08);

/* Delay-slot store scheduling wall: a lone `D_25BA60 = 0; return 0` the
 * original keeps out of the jr delay slot while cc1 sinks the store in. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5C48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5C58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5D10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5EC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D5F10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6028);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D60E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6158);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6180);

/* Draw-batch wrapper. */
s32 func_002D61A8(void) {
    Begin2dDrawBatch(0);
    func_0029DD10();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D61D8);

/* return 0 stub. */
s32 func_002D6240(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6248);

/* Draw-batch wrapper. */
s32 func_002D6380(void) {
    Begin2dDrawBatch(0);
    func_0029D958();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D63B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6408);

/* Draw-batch wrapper. */
s32 func_002D64D8(void) {
    Begin2dDrawBatch(0);
    func_0029D8E8();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6508);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6560);

/* Draw-batch wrapper. */
s32 func_002D6588(void) {
    Begin2dDrawBatch(0);
    func_0029D1A8();
    End2dDrawBatch();
    return 0;
}

/* Delay-slot store scheduling wall: `g_mapActiveSlot = -1; return 0`. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D65B8);

/* return 0 stub. */
s32 func_002D65D0(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D65D8);

/* Draw-batch wrapper. */
s32 func_002D6770(void) {
    Begin2dDrawBatch(0);
    func_0029D218();
    End2dDrawBatch();
    return 0;
}

/* Screen-action wrapper: MenuScreenDoAction(op, arg, &localFlag). */
s32 func_002D67A0(s32 op, s32 arg) {
    s32 outFlag = 0;
    return MenuScreenDoAction(op, arg, &outFlag);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", MenuScreenDoAction);

/* Register-coloring near-miss (97%): list-entry screen-action forwarder. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6AA0);

/* Command-record wrapper: MenuScreenDoAction(cmd->op, cmd->arg, out). */
s32 func_002D6AD8(MenuCmd *cmd, void *out) {
    return MenuScreenDoAction(cmd->op, cmd->arg, out);
}

/* Command-record wrapper with a local out-flag. */
s32 func_002D6B00(MenuCmd *cmd) {
    s32 outFlag = 0;
    return MenuScreenDoAction(cmd->op, cmd->arg, &outFlag);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6B28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D6E98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", SetGalacticMapFadeAlpha);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D75D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D7AE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", GalacticMapScreenTick);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8270);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", HandleGalacticMapPlanetSelectInput);

/* Handwritten frameless stub fragment (no jr — addiu $sp run). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8770);

/* Delay-slot store scheduling wall: `g_mapActiveSlot = -1; return 0`. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8778);

/* Forwarding wrapper: func_002DF1B8(1); return 0. */
s32 func_002D8790(void) {
    func_002DF1B8(1);
    return 0;
}

/* Snapshot the saved channel volume back into the live table slot 2. */
s32 func_002D87B0(void) {
    g_sndChannelVolumes[2] = D_1A7BA8;
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D87C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8A68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8DD0);

/* return 0 stub. */
s32 func_002D8E58(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D8E60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", RestorePrevTextTable);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", StreamTextTable);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9178);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9718);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9C18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002D9D60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA330);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA358);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA488);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA4F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DA740);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DAA50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DAAF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DAE70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", InitMenuBgImageBuffers);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DB028);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DB080);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DB700);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", LoadMenuBgImagePair);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", UploadMenuBgImagePair);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DBC98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DBEE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC0F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC378);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC520);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC6B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC7D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC800);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC838);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC878);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC8A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DC940);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCBB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCBF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCCC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCDC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DCF58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD0F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD450);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD630);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD7E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD858);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DD888);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DDD30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DE168);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DE768);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DECC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DECE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DEEE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF1B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF368);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF428);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF500);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF560);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF5B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF600);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF620);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF660);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF668);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DF710);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", RestorePlayerProgressState);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFE60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFF68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFFA0);

/* Always-ready gate: return 1. */
s32 func_002DFFC8(void) {
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFFD0);

/* Register-coloring near-miss: is the current menu screen the given fixed
 * screen instance? (pointer compare lowered to xor + sltiu). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1D54C0", func_002DFFE0);

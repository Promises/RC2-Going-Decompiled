#include "common.h"

/*
 * text/1D5488 — EU (SCES_516.07) twin of USA text/1D54C0 — front-end /
 * pause-menu screens band, part B. EU file-offset / address-shifted sibling of
 * the USA unit (USA 0x2D5540.. -> EU 0x2D5508..). Ported per Phase B: the C
 * bodies are region-agnostic; only the extern global/function NAMES are
 * retargeted to their EU addresses (objdiff masks the gp/reloc deltas).
 *
 * Built at -O2 -G8 -fno-gcse (later SN cc1 TU model — see the USA part-B header
 * for the -G8 extern-sizing rules). The same per-function walls the USA unit
 * leaves as INCLUDE_ASM stay INCLUDE_ASM here.
 */

/* 2D draw-batch begin/end fence used by every menu draw function.
 * USA Begin2dDrawBatch -> EU func_0027CA28; End2dDrawBatch -> EU func_0027CB48. */
extern void func_0027CA28(s32 mode); /* Begin2dDrawBatch (EU) */
extern void func_0027CB48(void);     /* End2dDrawBatch (EU) */

/* Per-screen draw helpers in the preceding text/1A00F0 asm band (EU names). */
extern void func_0029D870(void); /* USA func_0029DD10 */
extern void func_0029D4B8(void); /* USA func_0029D958 */
extern void func_0029D448(void); /* USA func_0029D8E8 */
extern void func_0029CD08(void); /* USA func_0029D1A8 */
extern void func_0029CD78(void); /* USA func_0029D218 */

/* Forwarding-wrapper target (EU name; USA func_002DF1B8). */
extern void func_002DF178(s32 mode);

/* Front-end / pause screen-action dispatcher (op, arg, out-flag ptr). */
extern s32 MenuScreenDoAction(s32 op, s32 arg, void *outFlag);

/* A menu command record: opcode at +2 (s16), arg at +4 (s32). */
typedef struct MenuCmd {
    u8  _pad0[2];
    s16 op;     /* 0x2 */
    s32 arg;    /* 0x4 */
} MenuCmd;

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5508);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D59D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5A08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5BC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5C10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5C20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5CD8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5E90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5ED8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D5FF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6058);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D60C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D60F0);

/* Draw-batch wrapper. */
s32 func_002D6118(void) {
    func_0027CA28(0);
    func_0029D870();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6148);

/* return 0 stub. */
s32 func_002D61B0(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D61B8);

/* Draw-batch wrapper. */
s32 func_002D62F0(void) {
    func_0027CA28(0);
    func_0029D4B8();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6320);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6378);

/* Draw-batch wrapper. */
s32 func_002D6448(void) {
    func_0027CA28(0);
    func_0029D448();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6478);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D64D0);

/* Draw-batch wrapper. */
s32 func_002D64F8(void) {
    func_0027CA28(0);
    func_0029CD08();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6528);

/* return 0 stub. */
s32 func_002D6540(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6548);

/* Draw-batch wrapper. */
s32 func_002D66E0(void) {
    func_0027CA28(0);
    func_0029CD78();
    func_0027CB48();
    return 0;
}

/* Screen-action wrapper: MenuScreenDoAction(op, arg, &localFlag). */
s32 func_002D6710(s32 op, s32 arg) {
    s32 outFlag = 0;
    return MenuScreenDoAction(op, arg, &outFlag);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", MenuScreenDoAction);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6A10);

/* Command-record wrapper: MenuScreenDoAction(cmd->op, cmd->arg, out). */
s32 func_002D6A48(MenuCmd *cmd, void *out) {
    return MenuScreenDoAction(cmd->op, cmd->arg, out);
}

/* Command-record wrapper with a local out-flag. */
s32 func_002D6A70(MenuCmd *cmd) {
    s32 outFlag = 0;
    return MenuScreenDoAction(cmd->op, cmd->arg, &outFlag);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6A98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D6E30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D74D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D75A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D7AA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", GalacticMapScreenTick);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8228);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D82D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8700);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8708);

/* Forwarding wrapper: func_002DF178(1); return 0. */
s32 func_002D8720(void) {
    func_002DF178(1);
    return 0;
}

/* Snapshot the saved channel volume back into the live table slot 2.
 *
 * HELD as INCLUDE_ASM in EU (region-delta): the USA twin func_002D87B0 matches
 * because g_sndChannelVolumes is a NAMED symbol at 0x1886A8, so slot 2 is the
 * small folded reloc `g_sndChannelVolumes+0x8` (one lui %hi / sw %lo pair). The
 * EU split named no symbol at the corresponding EU address (0x188728), so the EU
 * target references it section-relative as `D_00180000 + 0x8730`. cc1 cannot
 * fold a 0x8730 addend into a single %hi/%lo reloc (it exceeds the 0x7FFF %lo
 * range and gets materialised with an extra li/addu), and naming the EU symbol
 * would require a re-split (out of Phase-B scope). Only the relocation symbol
 * differs; the instruction body is otherwise byte-identical to the USA match. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8740);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8758);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D89F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8D60);

/* return 0 stub. */
s32 func_002D8DE8(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8DF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D8E90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", StreamTextTable);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D9108);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D96F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D9BE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002D9D28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA2F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA320);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA450);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA4B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DA708);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAA18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAAC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAE38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAF58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DAFF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DB048);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DB6C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBA38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBB88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBC60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DBEA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC0B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC340);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC4E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC680);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC7A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC7C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC800);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC840);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC870);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DC908);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCB78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCBB8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCC90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCD88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DCF20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD0C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD418);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD5F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD7B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD820);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DD850);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DDCE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DE110);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DE710);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DEC88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DECA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DEEA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF178);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF328);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF3E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF4C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF520);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF570);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF5C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF5E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF620);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF628);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DF6D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", RestorePlayerProgressState);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFE20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFF28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFF60);

/* Always-ready gate: return 1. */
s32 func_002DFF88(void) {
    return 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFF90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1D5488", func_002DFFA0);

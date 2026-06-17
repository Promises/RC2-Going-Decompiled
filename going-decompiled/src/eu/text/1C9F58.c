#include "common.h"

/*
 * text/1C9F58 — EU (SCES_516.07) twin of USA text/1CA080: front-end /
 * pause-menu screens band, part A. Same source as the USA sibling; the C
 * bodies are region-agnostic (objdiff masks the gp/reloc target deltas), so
 * only the extern global NAMES are retargeted to their EU equivalents. Built
 * at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA override in
 * tools/ee/objdiff_build.sh: "eu/text/1C9F58 -> USA 1CA080 twin").
 *
 * EU symbol retargets vs the USA file:
 *   - g_particleFxBlob+0x100   (USA)  ->  D_001F0000+0x2840 (EU; the EU build
 *                                         leaves the blob slot un-named, so the
 *                                         menu screen-state scratch is reached
 *                                         off the D_001F0000 segment base).
 *   - g_bestiaryCursor         (USA)  ->  D_1ABA50 (EU sdata, gp-relative).
 *   - Begin2dDrawBatch         (USA)  ->  func_0027CA28 (EU).
 *   - End2dDrawBatch           (USA)  ->  func_0027CB48 (EU).
 *   - func_0033A7B8 widget mtd  (USA) ->  func_0033B698 (EU).
 *   - the per-screen draw helpers (func_0029xxxx) and the bestiary widget
 *     offset (USA 0x3CEA0 -> EU 0x3CF50) differ per region; see each body.
 *   - g_pTextTableLoadBuf / g_guiInstance / MenuScreenLoad keep their names
 *     (named in the EU symbol map too).
 *
 * -G8 extern-sizing model is identical to the USA file: a complete <=8-byte
 * extern goes in small data (gp-relative); an object read with the adjacent
 * lui/%lo "assembler macro" shape gets a `.extern sym,16` override so cc1
 * emits the one-insn symbolic macro.
 */

/* cc1-small / assembler-absolute symbols (see header). */
__asm__(".extern g_guiInstance, 16");
/* The two right-justified-label draw functions (func_002CE9C0 / func_002D0230,
 * USA twins func_002CE9D8 / func_002D0240) read their Y position word via the
 * lui/%lo "assembler macro" shape, so each needs a 16-byte extern override. */
__asm__(".extern D_1ABA4C, 16");
__asm__(".extern D_1ABA9C, 16");

/* Singleton GUI-manager instance (null until the GUI is up). The wrappers here
 * only ever forward `instance + fixed-widget-offset` to widget methods. */
extern char *g_guiInstance;

/* Per-level effect-def blob base (EU segment D_001F0000); the menu code reuses
 * the slot at +0x2840 as a small front-end screen-state scratch struct (the
 * EU equivalent of the USA g_particleFxBlob+0x100 slot). */
extern u8 D_001F0000[];

/* Active language text-table load buffer (also a base for menu screen-state
 * words at +0xD8/+0x118). */
extern u8 g_pTextTableLoadBuf[];

/* Selected bestiary entry cursor (1..0x3f), EU sdata scalar. */
extern s32 D_1ABA50;

/* 2D draw-batch begin/end fence used by every menu draw function (EU names). */
extern void func_0027CA28(s32 mode); /* Begin2dDrawBatch */
extern void func_0027CB48(void);     /* End2dDrawBatch */

/* Widget-method target forwarded to by the g_guiInstance wrappers (EU name). */
extern void func_0033B698(char *widget, s32 arg);

/* Per-screen draw/update helpers in the preceding text/1A00F0 asm band (EU
 * addresses). */
extern void func_0029CAD0(void);
extern void func_0029CB70(void);
extern void func_0029D5B8(void);
extern void func_0029CDE8(void);
extern void func_0029CFA8(void);
extern void func_0029D018(void);
extern void func_0029D0C8(void);
extern void func_0029D138(void);
extern void func_0029D1A8(void);
extern void func_0029D3D8(void);
extern void func_0029D368(void);
extern void func_0029D218(void);
extern void func_0029D288(void);
extern void func_0029D2F8(void);
extern void func_0029CF38(void);

extern void MenuScreenLoad(void);

/* Localized-string lookup (EU 0x2898E8) and the right-justified text draw
 * primitive (EU func_0027FF28 = USA func_00280090). */
extern char *GetLocalizedString(s32 id);
extern void func_0027FF28(s32 x, s32 y, u64 color, char *str, s64 wrap);

/* Per-screen helpers + label-position words for the two right-justified-label
 * draw functions (EU addresses; X word is gp-relative small data, Y word is the
 * absolute lui/%lo macro sized above). */
extern void func_0029CE58(void); /* USA func_0029D2F8 */
extern void func_0029CEC8(void); /* USA func_0029D368 */
extern s32 D_1ABA48; /* USA D_1AB9E0 (gp-rel X) */
extern s32 D_1ABA4C; /* USA D_1AB9E4 (abs Y)    */
extern s32 D_1ABA98; /* USA D_1ABA28 (gp-rel X) */
extern s32 D_1ABA9C; /* USA D_1ABA2C (abs Y)    */

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002C9FD8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CA010);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CA2C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CA4F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CA618);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CA838);

/* Read the screen-state word stashed at g_pTextTableLoadBuf+0xD8. */
s32 func_002CA848(void) {
    return *(s32 *)(g_pTextTableLoadBuf + 0xD8);
}

/* Reset two state words in the menu screen-state scratch. */
void func_002CA858(void) {
    u8 *p = D_001F0000 + 0x2840;
    *(s32 *)(p + 0xF0) = 0;
    *(s32 *)(p + 0x1F0) = 0;
}

/* Install the screen-state ptr/flag pair: blob[0xF0] = &buf[0x118], flag=1. */
void func_002CA870(void) {
    u8 *p = D_001F0000 + 0x2840;
    *(u8 **)(p + 0xF0) = g_pTextTableLoadBuf + 0x118;
    *(s32 *)(p + 0x1F0) = 1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", ListScrollerSelectPrev);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", ListScrollerSelectNext);

/* GUI wrapper: forward the bestiary widget to its hide/show method. */
s32 func_002CA910(void) {
    if (g_guiInstance) {
        func_0033B698(g_guiInstance + 0x3CF50, 0);
    }
    return 0;
}

/* GUI wrapper: same widget, opposite visibility flag. */
s32 func_002CA948(void) {
    if (g_guiInstance) {
        func_0033B698(g_guiInstance + 0x3CF50, 1);
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CAA18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CAA58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CAA88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", RequestMenuScreenChange);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CAC90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CAD98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", MenuScreenLoad);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CB410);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CB5D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CB710);

/* MenuScreenLoad then mark the screen-state scratch ready (state=2). */
void func_002CB8C0(void) {
    u8 *p;
    MenuScreenLoad();
    p = D_001F0000 + 0x2840;
    *(s32 *)(p + 0x0) = 2;
    *(s32 *)(p + 0x4) = 0;
}

/* Mark the screen-state scratch (state=3, clear sub-state). */
void func_002CB8F0(void) {
    u8 *p = D_001F0000 + 0x2840;
    *(s32 *)(p + 0x0) = 3;
    *(s32 *)(p + 0x4) = 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", MenuScreenCommitTransition);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", MenuScreenUpdate);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CBC18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", BuildPausePromptPopup);

/* Level-select slot-index validity gate (idx < 0x15 or idx == 0x18). */
s32 IsLevelListEntryEnabled(s32 idx) {
    return (idx < 0x15 || idx == 0x18);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", LevelSelectListHandleInput);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", LevelSelectListRender);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC638);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC688);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC708);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC7B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC8C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC950);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CCAC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CD300);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CD398);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CD3C0);

/* return 0 stub. */
s32 func_002CD500(void) {
    return 0;
}

/* return 0 stub. */
s32 func_002CD508(void) {
    return 0;
}

/* Clear a list-state field (entry +0x44 = -1) and return 0. */
s32 func_002CD510(s32 *list) {
    list[0x11] = -1;
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CD520);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CDE98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CDF30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE0B0);

/* Draw-batch wrapper: render one menu sub-element inside a 2D batch. */
s32 func_002CE1E8(void) {
    func_0027CA28(0);
    func_0029CAD0();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE218);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE388);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE480);

/* Draw-batch wrapper. */
s32 func_002CE5C0(void) {
    func_0027CA28(0);
    func_0029CB70();
    func_0027CB48();
    return 0;
}

/* return 0 stub. */
s32 func_002CE5F0(void) {
    return 0;
}

/* return 0 stub. */
s32 func_002CE5F8(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE600);

/* Draw-batch wrapper. */
s32 func_002CE690(void) {
    func_0027CA28(0);
    func_0029D5B8();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE6C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE818);

/* Draw-batch wrapper. */
s32 func_002CE860(void) {
    func_0027CA28(0);
    func_0029CDE8();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE890);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE8F0);

/* Draw the localized string right-justified at the alternate label slot.
 * EU twin of USA func_002CE9D8; the localized-string id is the PAL id 0xB60
 * (NTSC uses 0x2BE5). */
s32 func_002CE9C0(void) {
    char *str;
    func_0027CA28(0);
    func_0029CE58();
    str = GetLocalizedString(0xB60);
    func_0027FF28(D_1ABA48, D_1ABA4C, 0x80F0F0F0, str, -1);
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CEA20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CEAA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CECF8);

/* Reset the bestiary cursor to entry 1. */
s32 func_002CF530(void) {
    D_1ABA50 = 1;
    return 0;
}

/* return 0 stub. */
s32 func_002CF540(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CF548);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CFB48);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D0018);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D0100);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D0148);

/* Draw the localized string right-justified as a label inside a 2D batch.
 * EU twin of USA func_002D0240; the localized-string id is the PAL id 0xB60
 * (NTSC uses 0x2BE5). */
s32 func_002D0230(void) {
    char *str;
    func_0027CA28(0);
    func_0029CEC8();
    str = GetLocalizedString(0xB60);
    func_0027FF28(D_1ABA98, D_1ABA9C, 0x80F0F0F0, str, -1);
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D0290);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", UpdateCheatMenuInput);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawCheatMenu);

/* return 0 stub. */
s32 func_002D0B30(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", UpdateSkillPointsMenu);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawSkillPointsMenu);

/* func_002D1118 (USA func_002D1150): trivial `return 0` stub, byte-identical to
 * the USA twin. The unit's tail boundary is settled by the verified Phase-A
 * tiling, so the stub is safe to port. */
s32 func_002D1118(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D1120);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D1488);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D17F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D1840);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D1AA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D1E10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D1EB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D2158);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D24C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D2600);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D28A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D2BE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D2C58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D2DA8);

/* return 0 stub. */
s32 func_002D2F48(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D2F50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D30C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3310);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3330);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3470);

/* return 0 stub. */
s32 func_002D3768(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3770);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3850);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3B10);

/* Draw-batch wrapper. */
s32 func_002D3B38(void) {
    func_0027CA28(0);
    func_0029CFA8();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3B68);

/* Draw-batch wrapper. */
s32 func_002D3BF0(void) {
    func_0027CA28(0);
    func_0029D018();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3C20);

/* Draw-batch wrapper. */
s32 func_002D3DB8(void) {
    func_0027CA28(0);
    func_0029D0C8();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3DE8);

/* Draw-batch wrapper. */
s32 func_002D3E70(void) {
    func_0027CA28(0);
    func_0029D138();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3EA0);

/* Draw-batch wrapper. */
s32 func_002D3F28(void) {
    func_0027CA28(0);
    func_0029D1A8();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3F58);

/* Draw-batch wrapper. */
s32 func_002D3FE0(void) {
    func_0027CA28(0);
    func_0029D3D8();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4010);

/* Draw-batch wrapper. */
s32 func_002D4098(void) {
    func_0027CA28(0);
    func_0029D368();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D40C8);

/* Draw-batch wrapper. */
s32 func_002D4150(void) {
    func_0027CA28(0);
    func_0029D218();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4180);

/* Draw-batch wrapper. */
s32 func_002D4208(void) {
    func_0027CA28(0);
    func_0029D288();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4238);

/* Draw-batch wrapper. */
s32 func_002D4308(void) {
    func_0027CA28(0);
    func_0029D2F8();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4338);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4378);

/* Draw-batch wrapper. */
s32 func_002D4400(void) {
    func_0027CA28(0);
    func_0029CF38();
    func_0027CB48();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4430);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D44B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4530);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4618);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4D00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4EA0);

#include "common.h"

/*
 * text/1CA080 — front-end / pause-menu screens band, part A (vaddr
 * 0x2CA100..0x2D553F). Carved out of the big text/1B21E8 asm tile as
 * ranked-carve-pipeline pick #5 ("menu-screens"); part B is text/1D54C0.
 *
 * Built at -O2 -G8 -fno-gcse (see the per-unit GFLAG/CC1EXTRA overrides in
 * tools/ee/objdiff_build.sh / diff.sh / build.sh): these gameplay/UI TUs were
 * compiled by the later SN cc1 without the load-PRE pass (-fno-gcse reproduces
 * it) and use the -G8 small-data + assembler-absolute extern model.
 *
 * -G8 extern-sizing model for this file (same as text/198FA0 / text/1907F0):
 *   - a complete extern object of size <= 8 bytes goes in small data
 *     (gp-relative);
 *   - an object the original reads with the ADJACENT lui/%lo "assembler macro"
 *     shape gets a `.extern sym,16` override so cc1 emits the one-insn
 *     symbolic macro (matching the SN prologue scheduling) and GNU as expands
 *     it absolutely.
 *
 * Walls left as INCLUDE_ASM (documented per function): the 8-byte-packed
 * callee-save wall (every function saving 2+ GPRs — the later cc1 packs save
 * slots 8-byte vs our 16-byte), switch/jtbl functions (splat jtbl reloc gap),
 * and assorted register-coloring / reload-artifact near-misses.
 */

/* cc1-small / assembler-absolute symbols (see header). */
__asm__(".extern g_guiInstance, 16");
__asm__(".extern g_padButtonsPressed, 16");
__asm__(".extern D_1F27C0, 16");
__asm__(".extern D_1A7318, 16");
__asm__(".extern D_2617C0, 16");
__asm__(".extern D_25BA70, 16");
__asm__(".extern D_1ABA2C, 16");
__asm__(".extern D_1AB9E4, 16");
__asm__(".extern D_1B8FC0, 16");
__asm__(".extern g_miscExtras, 16");
__asm__(".extern D_1ABA84, 16");
__asm__(".extern D_1ABA88, 16");
__asm__(".extern D_1ABA8C, 16");
__asm__(".extern D_1ABA90, 16");

/* Singleton GUI-manager instance (null until the GUI is up). The wrappers here
 * only ever forward `instance + fixed-widget-offset` to widget methods. */
extern char *g_guiInstance;

/* Per-level effect-def blob; the menu code reuses the slot at +0x100 as a
 * small front-end screen-state scratch struct. */
extern u8 g_particleFxBlob[];

/* Selected catalog/list cursors (small-data scalars). */
extern s32 g_bestiaryCursor;     /* selected bestiary entry (1..0x3f) */
extern u8 g_pTextTableLoadBuf[]; /* active language text-table load buffer (also a base for menu screen-state words at +0xD8/+0x118) */

/* 2D draw-batch begin/end fence used by every menu draw function. */
extern void Begin2dDrawBatch(s32 mode);
extern void End2dDrawBatch(void);

/* Widget-method targets forwarded to by the g_guiInstance wrappers. */
extern void func_0033A7B8(char *widget, s32 arg);
extern void func_0033F3B8(char *widget, void *arg);

/* Per-screen draw/update helpers in the preceding text/1A00F0 asm band. */
extern void func_0029CF70(void);
extern void func_0029D010(void);
extern void func_0029DA58(void);
extern void func_0029D288(void);
extern void func_0029D448(void);
extern void func_0029D4B8(void);
extern void func_0029D568(void);
extern void func_0029D5D8(void);
extern void func_0029D798(void);
extern void func_0029D808(void);
extern void func_0029D6B8(void);
extern void func_0029D728(void);
extern void func_0029D3D8(void);
extern void func_0029D648(void);
extern void func_0029D878(void);

extern void MenuScreenLoad(void);

/* Menu sub-screen error/status latch (absolute %hi/%lo data word). */
extern s32 D_25CABC;

/* Pad button state + camera state used by a couple menu helpers. */
extern s32 g_padButtonsPressed;

/* Localized-string + 2D label draw helpers in adjacent text bands. */
extern char *GetLocalizedString(s32 id);
extern void func_00280090(s32 x, s32 y, u64 color, char *str, s64 wrap);
extern void func_0029D368(void);
extern void func_0029D478(s32 buttons);
extern s32 func_002D67A0(s32 a, void *b);

/* gp-relative + absolute label position words consumed by func_002D0240. */
extern s32 D_1ABA28;
extern s32 D_1ABA2C;
extern char D_25BA70[];

/* Front-end screen-state blob slice at D_1F27C0 (menu reciprocal/clear scratch). */
extern u8 D_1F27C0[];

/* Bestiary/list base-pointer pair selected by D_1A7318. */
extern s32 D_1A7318;
extern u8 D_1AB678[];
extern u8 D_1AB648[];

/* Widget-data blob forwarded by the func_002D4370 GUI wrapper. */
extern u8 D_2617C0;

/* Camera/projection scratch blob + frame-matrix builders (func_002CAFD8). */
extern u8 D_1B8FC0[];
extern void BuildCameraProjection(void);
extern void BuildFrameViewMatrices(void);
extern void func_002CABC0(void);

/* Twin of func_002D0240 for the alternate label slot (func_002CE9D8). */
extern void func_0029D2F8(void);
extern s32 D_1AB9E0;
extern s32 D_1AB9E4;

/* Extras-menu "new content" flags latched by func_002D2C60 when the GUI is up. */
extern u8 g_miscExtras;            /* master extras-unlocked byte */
extern s32 D_1AA450, D_1AA454, D_1AA458; /* per-feature availability (gp-rel) */
extern s32 D_1ABA80;               /* skill-points highlight flag (gp-rel) */
extern s32 D_1ABA84, D_1ABA88, D_1ABA8C, D_1ABA90; /* per-menu "new" flags (abs) */

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA100);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA138);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA3E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA618);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA740);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA960);

/* Read the screen-state word stashed at g_pTextTableLoadBuf+0xD8. */
s32 func_002CA970(void) {
    return *(s32 *)(g_pTextTableLoadBuf + 0xD8);
}

/* Reset two state words in the menu screen-state scratch. */
void func_002CA980(void) {
    u8 *p = g_particleFxBlob + 0x100;
    *(s32 *)(p + 0xF0) = 0;
    *(s32 *)(p + 0x1F0) = 0;
}

/* Install the screen-state ptr/flag pair: blob[0xF0] = &buf[0x118], flag=1. */
void func_002CA998(void) {
    u8 *p = g_particleFxBlob + 0x100;
    *(u8 **)(p + 0xF0) = g_pTextTableLoadBuf + 0x118;
    *(s32 *)(p + 0x1F0) = 1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", ListScrollerSelectPrev);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", ListScrollerSelectNext);

/* GUI wrapper: forward the bestiary widget to its hide/show method. */
s32 func_002CAA38(void) {
    if (g_guiInstance) {
        func_0033A7B8(g_guiInstance + 0x3CEA0, 0);
    }
    return 0;
}

/* GUI wrapper: same widget, opposite visibility flag. */
s32 func_002CAA70(void) {
    if (g_guiInstance) {
        func_0033A7B8(g_guiInstance + 0x3CEA0, 1);
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CAAA8);

/* Clear a 20-entry s32 array (D_1F27C0+0x16C..+0x1BC) to -1, back to front.
 * Near-miss: cc1 folds %lo(D_1F27C0)+0x1BC into one addiu, but the original
 * keeps the symbol-%lo and the +0x1BC offset as two separate addiu (the SN
 * assembler-absolute macro shape). Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CAB50);
#else
void func_002CAB50(void) {
    s32 *p = (s32 *)D_1F27C0 + 0x6F;
    s32 i = 0x13;
    do {
        *p = -1;
        i--;
        p--;
    } while (i >= 0);
}
#endif

/* Store a reciprocal into the menu scratch: [0x1C0]=1.0f, [0x1C8]=0, [0x1C4]=1/x.
 * Near-miss: cc1 anchors the store base at +0x1C0 (CSE) instead of keeping the
 * symbol address as the base with immediate offsets, as the original does.
 * Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CAB90);
#else
void func_002CAB90(float x) {
    float one = 1.0f;
    u8 *p = D_1F27C0;
    *(float *)(p + 0x1C0) = one;
    *(s32 *)(p + 0x1C8) = 0;
    *(float *)(p + 0x1C4) = one / x;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CABC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", RequestMenuScreenChange);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", CaptureScreenToVram);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", RestoreScreenFromVram);

/* Seed the camera/projection scratch (near/aspect/scale + clears) then rebuild
 * the camera projection, the front-end matrix block, and the frame view mats.
 * Near-miss: cc1 tail-calls the final BuildFrameViewMatrices (j) and anchors the
 * store base differently than the original (jal + symbol-base). Preserved as C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CAFD8);
#else
void func_002CAFD8(void) {
    *(float *)(D_1B8FC0 + 0x21C) = 524288.0f;
    *(float *)(D_1B8FC0 + 0xB0) = 0.62f;
    *(float *)(D_1B8FC0 + 0x228) = 255.0f;
    *(s32 *)(D_1B8FC0 + 0x218) = 0;
    *(s32 *)(D_1B8FC0 + 0x22C) = 0;
    BuildCameraProjection();
    func_002CABC0();
    BuildFrameViewMatrices();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", MenuScreenLoad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CB560);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CB720);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CB860);

/* MenuScreenLoad then mark the screen-state scratch ready (state=2). */
void func_002CBA10(void) {
    u8 *p;
    MenuScreenLoad();
    p = g_particleFxBlob + 0x100;
    *(s32 *)(p + 0x0) = 2;
    *(s32 *)(p + 0x4) = 0;
}

/* Mark the screen-state scratch (state=3, clear sub-state). */
void func_002CBA40(void) {
    u8 *p = g_particleFxBlob + 0x100;
    *(s32 *)(p + 0x0) = 3;
    *(s32 *)(p + 0x4) = 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", MenuScreenCommitTransition);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", MenuScreenUpdate);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CBD68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", BuildPausePromptPopup);

/* Level-select slot-index validity gate (idx < 0x15 or idx == 0x18). */
s32 IsLevelListEntryEnabled(s32 idx) {
    return (idx < 0x15 || idx == 0x18);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", LevelSelectListHandleInput);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", LevelSelectListRender);

/* Dispatch a 3-way menu action: 0 -> 1, 1 -> func_002D67A0(3,&D_25BA70), else 0.
 * Near-miss: the original threads the result through a single register with a
 * jump-table-like branch layout our cc1 won't reproduce from an if/else chain.
 * Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC788);
#else
s32 func_002CC788(s32 action) {
    s32 result = 0;
    if (action == 0) {
        result = 1;
    } else if (action == 1) {
        result = func_002D67A0(3, D_25BA70);
    } else {
        return 0;
    }
    return result;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC7D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC858);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC908);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CCA18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", TickActiveMenuScreen);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", RenderMenuScreenWidgets);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CD450);

/* Stores D_1A7318 ? &D_1AB648 : &D_1AB678 into list[0x34], returns 0. Left as
 * INCLUDE_ASM: the original has an anomalous +0x60 stack adjust prologue with no
 * matching restore (frame artifact) that our cc1 won't reproduce from clean C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CD4E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", BuildCheatMenuItemList);

/* return 0 stub. */
s32 func_002CD650(void) {
    return 0;
}

/* return 0 stub. */
s32 func_002CD658(void) {
    return 0;
}

/* Clear a list-state field (entry +0x44 = -1) and return 0. */
s32 func_002CD660(s32 *list) {
    list[0x11] = -1;
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CD670);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CDEB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CDF48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE0C8);

/* Draw-batch wrapper: render one menu sub-element inside a 2D batch. */
s32 func_002CE200(void) {
    Begin2dDrawBatch(0);
    func_0029CF70();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE230);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE3A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE498);

/* Draw-batch wrapper. */
s32 func_002CE5D8(void) {
    Begin2dDrawBatch(0);
    func_0029D010();
    End2dDrawBatch();
    return 0;
}

/* return 0 stub. */
s32 func_002CE608(void) {
    return 0;
}

/* return 0 stub. */
s32 func_002CE610(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE618);

/* Draw-batch wrapper. */
s32 func_002CE6A8(void) {
    Begin2dDrawBatch(0);
    func_0029DA58();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE6D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE830);

/* Draw-batch wrapper. */
s32 func_002CE878(void) {
    Begin2dDrawBatch(0);
    func_0029D288();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE8A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE908);

/* Draw the localized string 0x2BE5 right-justified at the alternate label slot. */
s32 func_002CE9D8(void) {
    char *str;
    Begin2dDrawBatch(0);
    func_0029D2F8();
    str = GetLocalizedString(0x2BE5);
    func_00280090(D_1AB9E0, D_1AB9E4, 0x80F0F0F0, str, -1);
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CEA38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateBestiaryMenuInput);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawBestiaryEntry);

/* Reset the bestiary cursor to entry 1. */
s32 func_002CF540(void) {
    g_bestiaryCursor = 1;
    return 0;
}

/* return 0 stub. */
s32 func_002CF550(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawBestiaryPagingArrows);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawMenuPagingChrome);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawMenuItemSelectionBox);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D0110);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D0158);

/* Draw the localized string 0x2BE5 as a right-justified label inside a 2D batch. */
s32 func_002D0240(void) {
    char *str;
    Begin2dDrawBatch(0);
    func_0029D368();
    str = GetLocalizedString(0x2BE5);
    func_00280090(D_1ABA28, D_1ABA2C, 0x80F0F0F0, str, -1);
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D02A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateCheatMenuInput);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawCheatMenu);

/* return 0 stub. */
s32 func_002D0B40(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateSkillPointsMenu);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawSkillPointsMenu);

/* return 0 stub. */
s32 func_002D1150(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateExtrasMenuInput);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawExtrasMenu);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D1850);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", CinematicsMenuTick);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawCinematicsMenu);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D1E88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdatePlanetWarpMenuInput);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawPlanetWarpMenu);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D2538);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateInsomniacMuseumInput);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawInsomniacMuseumMenu);

/* When the GUI is up, latch the "new content" flags for each extras-menu entry.
 * Near-miss: cc1 hoists the g_miscExtras load above (and CSEs it into) the
 * g_guiInstance short-circuit test, reordering the two guard loads vs the
 * original's separate guiInstance check. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D2C60);
#else
s32 func_002D2C60(void) {
    if (g_guiInstance) {
        if (g_miscExtras) {
            D_1ABA8C = 1;
            D_1ABA88 = 1;
        }
        if (D_1AA450) {
            D_1ABA84 = 1;
        }
        if (D_1AA454) {
            D_1ABA90 = 1;
        }
        if (D_1AA458) {
            D_1ABA80 = 1;
        }
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateHelpTopicMenuInput);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawHelpTopicMenu);

/* return 0 stub. */
s32 func_002D2FC0(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D2FC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3138);

/* GUI null-check gate: if the GUI is up, latch error code -0x12C; return 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3388);
#else
s32 func_002D3388(void) {
    if (g_guiInstance) {
        D_25CABC = -0x12C;
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D33A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D34E8);

/* return 0 stub. */
s32 func_002D37E0(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D37E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D38C8);

/* Forward the currently-pressed pad buttons to the menu input handler; return 0.
 * Near-miss: the original hoists the %hi(g_padButtonsPressed) lui above the
 * frame-setup addiu (separate base reg), a SN-scheduling shape our cc1 won't
 * reproduce from the sized-extern macro. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3B88);
#else
s32 func_002D3B88(void) {
    func_0029D478(g_padButtonsPressed);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3BB0(void) {
    Begin2dDrawBatch(0);
    func_0029D448();
    End2dDrawBatch();
    return 0;
}

/* 8-byte-packed-save wall: input handler saving $16 + $31 (2 GPRs). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3BE0);

/* Draw-batch wrapper. */
s32 func_002D3C68(void) {
    Begin2dDrawBatch(0);
    func_0029D4B8();
    End2dDrawBatch();
    return 0;
}

/* 8-byte-packed-save wall: input handler saving $16 + $31 (2 GPRs). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3C98);

/* Draw-batch wrapper. */
s32 func_002D3DD8(void) {
    Begin2dDrawBatch(0);
    func_0029D568();
    End2dDrawBatch();
    return 0;
}

/* 8-byte-packed-save wall: input handler saving $16 + $31 (2 GPRs). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3E08);

/* Draw-batch wrapper. */
s32 func_002D3E90(void) {
    Begin2dDrawBatch(0);
    func_0029D5D8();
    End2dDrawBatch();
    return 0;
}

/* 8-byte-packed-save wall: input handler saving $16 + $31 (2 GPRs). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3EC0);

/* Draw-batch wrapper. */
s32 func_002D3F48(void) {
    Begin2dDrawBatch(0);
    func_0029D648();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3F78);

/* Draw-batch wrapper. */
s32 func_002D4000(void) {
    Begin2dDrawBatch(0);
    func_0029D878();
    End2dDrawBatch();
    return 0;
}

/* 8-byte-packed-save wall: input handler saving $16 + $31 (2 GPRs). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4030);

/* Draw-batch wrapper. */
s32 func_002D40B8(void) {
    Begin2dDrawBatch(0);
    func_0029D808();
    End2dDrawBatch();
    return 0;
}

/* 8-byte-packed-save wall: input handler saving $16 + $31 (2 GPRs). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D40E8);

/* Draw-batch wrapper. */
s32 func_002D4188(void) {
    Begin2dDrawBatch(0);
    func_0029D6B8();
    End2dDrawBatch();
    return 0;
}

/* 8-byte-packed-save wall: input handler saving $16 + $31 (2 GPRs). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D41B8);

/* Draw-batch wrapper. */
s32 func_002D4240(void) {
    Begin2dDrawBatch(0);
    func_0029D728();
    End2dDrawBatch();
    return 0;
}

/* 8-byte-packed-save wall: input handler saving $16 + $17 + $31 (3 GPRs). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4270);

/* Draw-batch wrapper. */
s32 func_002D4340(void) {
    Begin2dDrawBatch(0);
    func_0029D798();
    End2dDrawBatch();
    return 0;
}

/* GUI wrapper: forward a widget at instance+0x3F3F0 to func_0033F3B8(&D_2617C0).
 * Near-miss: byte-identical except cc1 schedules the addiu %lo(D_2617C0) before
 * the ori of the 0x3F3F0 offset (the two independent address builds interleave
 * in the opposite order from the original). Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4370);
#else
s32 func_002D4370(void) {
    if (g_guiInstance) {
        func_0033F3B8(g_guiInstance + 0x3F3F0, &D_2617C0);
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D43B0);

/* Draw-batch wrapper. */
s32 func_002D4438(void) {
    Begin2dDrawBatch(0);
    func_0029D3D8();
    End2dDrawBatch();
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4468);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D44E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4568);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateShipCustomizeInput);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4D38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawShipCustomizeMenu);

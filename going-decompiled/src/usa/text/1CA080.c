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
__asm__(".extern D_138180, 16");
/* func_002CDEB0 cheat-flag mirror: source byte flags + s16 menu mirrors. */
__asm__(".extern D_1A7BD1, 16");
__asm__(".extern D_1A7BD2, 16");
__asm__(".extern D_1A7BD3, 16");
__asm__(".extern D_1A7BD4, 16");
__asm__(".extern D_1A7BD6, 16");
__asm__(".extern D_1AA5A2, 16");
__asm__(".extern D_1AA5BA, 16");
__asm__(".extern D_1AA5D2, 16");
__asm__(".extern D_1AA5EA, 16");
__asm__(".extern D_1AA602, 16");
/* func_002D1850 extras-availability latch targets. */
__asm__(".extern g_lastMenuScreenId, 16");
__asm__(".extern D_1ABA50, 16");
__asm__(".extern D_1ABA54, 16");
__asm__(".extern D_1ABA58, 16");
__asm__(".extern D_1ABA4C, 16");

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

/* Global input/UI flags object; the menu code reads the flag word at +0x1C4. */
extern u8 D_138180[];

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

/* Mis-split fragment: orphaned stack-pointer adjusts (addiu $sp / nops) with
 * no jr $ra — not a real function entry; left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA100);

/* menu helper: uses 128-bit sq/lq (vector) loads/stores — left as INCLUDE_ASM
 * (the EE quadword ops are not emitted from scalar C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA138);

/* menu helper: uses 128-bit sq/lq (vector) loads/stores — left as INCLUDE_ASM
 * (the EE quadword ops are not emitted from scalar C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA3E8);

/* menu helper: uses 128-bit sq/lq (vector) loads/stores — left as INCLUDE_ASM
 * (the EE quadword ops are not emitted from scalar C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA618);

/* menu helper: uses 128-bit sq/lq (vector) loads/stores — left as INCLUDE_ASM
 * (the EE quadword ops are not emitted from scalar C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CA740);

/* Mis-split fragment: orphaned stack-pointer adjusts (addiu $sp / nops) with
 * no jr $ra — not a real function entry; left as INCLUDE_ASM. */
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

/* List-scroller "select previous": decrement the cursor (list[1]); when it drops
 * to 0 or below, wrap to the limit (list[0]). Then skip backwards over empty
 * (==0) slots in the entry array (list+0xC, one s32 per row). Returns the entry
 * value the cursor lands on.
 * Near-miss (~91%): the original emits a 64-bit sign-extension `daddu` copy of
 * the decremented index for the `> 0` test (stored vs tested registers differ)
 * plus a base-first commutative `addu`; our cc1 reuses one register and emits
 * the addu operands swapped. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", ListScrollerSelectPrev);
#else
/* TODO(match): functional equivalent - not byte-exact; EE 64-bit sign-extend
 * `daddu` copy + commutative-addu operand order not reproduced by cc1. */
s32 ListScrollerSelectPrev(s32 *list) {
    s32 *entries = list + 3;
    s32 entry;
    do {
        s32 idx = list[1] - 1;
        list[1] = idx;
        if (idx <= 0) {
            idx = list[0];
        }
        list[1] = idx;
        entry = entries[idx];
    } while (entry == 0);
    return entry;
}
#endif

/* List-scroller "select next": increment the cursor (list[1]); when it passes
 * the limit (list[0]) wrap to 0, then skip forward over empty (==0) slots in the
 * entry array (list+0xC). Returns the entry value the cursor lands on.
 * Near-miss (99.33%): single commutative-`addu` operand order differs — the
 * original forms `entries_base + idx<<2` (loop-invariant base first); cc1 emits
 * `idx<<2 + entries_base`. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", ListScrollerSelectNext);
#else
/* TODO(match): functional equivalent - not byte-exact; commutative-addu operand
 * order (base-first vs index-first) not reproduced by cc1. */
s32 ListScrollerSelectNext(s32 *list) {
    s32 limit = list[0];
    s32 *entries = list + 3;
    s32 entry;
    do {
        s32 idx = list[1] + 1;
        if (limit < idx) {
            idx = 0;
        }
        list[1] = idx;
        entry = entries[idx];
    } while (entry == 0);
    return entry;
}
#endif

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

/* Draw a centered two-line header inside a 2D batch: from the screen-rect at
 * `p` (center x = p[0x18], y = p[0x1C], width = p[0x20]) it half-splits the
 * width and draws localized string 0x31C8 left-justified at center-half and
 * 0x31C4 at center+half, both in 0x80F0F0F0.
 * Wall: 8-byte-packed-save (saves $16/$17/$18/$31). Preserved as portable C. */
extern void DrawDebugString(s32 x, s32 y, u64 color, char *str, s64 wrap);
extern void func_00280120(s32 x, s32 y, u64 color, char *str, s64 wrap);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CAAA8);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-GPR packed-save frame. */
s32 func_002CAAA8(s32 *p) {
    s32 cx = p[6];   /* p[0x18] */
    s32 half = p[8] >> 1; /* p[0x20] width / 2 */
    s32 y = p[7];    /* p[0x1C] */
    char *s;
    Begin2dDrawBatch(0);
    s = GetLocalizedString(0x31C8);
    DrawDebugString(cx - half, y, 0x80F0F0F0, s, -1);
    s = GetLocalizedString(0x31C4);
    func_00280120(cx + half, y, 0x80F0F0F0, s, -1);
    End2dDrawBatch();
    return 0;
}
#endif

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

/* menu helper: uses 128-bit sq/lq (vector) loads/stores — left as INCLUDE_ASM
 * (the EE quadword ops are not emitted from scalar C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CABC0);

/* menu-screen lifecycle routine: 8-byte-packed-save wall (saves 4 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", RequestMenuScreenChange);

/* screen-capture/restore routine: 8-byte-packed-save wall (saves 5 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", CaptureScreenToVram);

/* screen-capture/restore routine: 8-byte-packed-save wall (saves 5 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
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

/* menu-screen lifecycle routine: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", MenuScreenLoad);

/* menu helper: 8-byte-packed-save wall (saves 5 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CB560);

/* menu helper: uses 128-bit sq/lq (vector) loads/stores — left as INCLUDE_ASM
 * (the EE quadword ops are not emitted from scalar C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CB720);

/* menu helper: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
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

/* Commit a menu transition: full-screen tint then latch the screen-state
 * scratch (state=4, capture pending sub-state). Near-miss: the original emits a
 * dead conditional store (p[0x4]=1 then unconditional =0) the later cc1
 * load-PRE keeps but ours eliminates — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", MenuScreenCommitTransition);

/* menu-screen lifecycle routine: 8-byte-packed-save wall (saves 8 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", MenuScreenUpdate);

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CBD68);

/* menu data/list builder: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", BuildPausePromptPopup);

/* Level-select slot-index validity gate (idx < 0x15 or idx == 0x18). */
s32 IsLevelListEntryEnabled(s32 idx) {
    return (idx < 0x15 || idx == 0x18);
}

/* menu input/update handler: 8-byte-packed-save wall (saves 7 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", LevelSelectListHandleInput);

/* level-select list routine: 8-byte-packed-save wall (saves 9 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
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

/* menu helper: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC7D8);

/* menu helper: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC858);

/* menu helper: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC908);

/* 3-way menu action dispatch (0/1/2 -> toggle / func_002D67A0 / func_0029DCB8)
 * plus the nanotech-bonus-heal-timer store. Near-miss: single-register result
 * threading + store scheduling differ — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CCA18);

/* menu-screen lifecycle routine: 8-byte-packed-save wall (saves 4 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", TickActiveMenuScreen);

/* menu-screen lifecycle routine: 8-byte-packed-save wall (saves 9 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", RenderMenuScreenWidgets);

/* Menu screen-state query: only acts when the active screen (ss[0x14]) is the
 * one passed in. Returns a tri-state confirm/cancel code driven by the global
 * input flags (D_138180[0x1C4]) and the screen's pending-result fields.
 * Near-miss (~48%): the original keeps the second D_138180[0x1C4] reload and the
 * branch-likely (beql/bnel) loop shape from the load-PRE-present SN cc1; our cc1
 * CSEs the reload and the andi 0x10 test, producing a structurally different
 * (shorter) body. Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CD450);
#else
/* TODO(match): functional equivalent - not byte-exact; cc1 CSEs the global-flag
 * reload + andi the original re-emits under branch-likely. */
s32 func_002CD450(s32 screen) {
    u8 *ss = g_particleFxBlob + 0x100;
    s32 flags;
    s32 v;
    if (*(s32 *)(*(u8 **)(ss + 0x14) + 0xE8) != screen) {
        return 0;
    }
    flags = *(s32 *)(D_138180 + 0x1C4);
    if (flags & 0x900) {
        if (*(s32 *)(ss + 0x134) == 0) {
            return 1;
        }
        flags = *(s32 *)(D_138180 + 0x1C4);
    }
    if (!(flags & 0x10)) {
        return 0;
    }
    v = *(s32 *)(*(u8 **)(ss + 0x14) + 0xE0);
    if (v != 0) {
        *(s32 *)(ss + 0x18) = v;
        return 0;
    }
    if (*(s32 *)(ss + 0x134) == 0) {
        return -1;
    }
    return 0;
}
#endif

/* Stores D_1A7318 ? &D_1AB648 : &D_1AB678 into list[0x34], returns 0. Left as
 * INCLUDE_ASM: the original has an anomalous +0x60 stack adjust prologue with no
 * matching restore (frame artifact) that our cc1 won't reproduce from clean C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CD4E8);

/* menu data/list builder: ldl/ldr/sdl/sdr unaligned struct/const copy — left as INCLUDE_ASM
 * (cc1 won't reproduce the unaligned 64-bit copy idiom from clean C). */
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

/* menu helper: ldl/ldr/sdl/sdr unaligned struct/const copy — left as INCLUDE_ASM
 * (cc1 won't reproduce the unaligned 64-bit copy idiom from clean C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CD670);

/* Cheat-flag mirror: for each source toggle byte (D_1A7BDn) write 3 ("on") or 0
 * ("off") into the matching cheat-menu item-state halfword (D_1AA5xx/D_1AA602).
 * Leaf, pure data shuffle.
 * Near-miss (~83%): the original schedules each ternary's `move rd,zero` into
 * the beqz delay slot while emitting the previous result's store before the
 * branch; our cc1 hoists the move ahead of the store and leaves a nop in the
 * delay slot (branch-fill scheduling the later cc1 won't reproduce from clean
 * C). Preserved as portable C. */
extern u8 D_1A7BD1, D_1A7BD2, D_1A7BD3, D_1A7BD4, D_1A7BD6;
extern s16 D_1AA5A2, D_1AA5BA, D_1AA5D2, D_1AA5EA, D_1AA602;

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CDEB0);
#else
/* TODO(match): functional equivalent - not byte-exact; ternary move/store
 * delay-slot scheduling not reproduced by cc1. */
void func_002CDEB0(void) {
    D_1AA5A2 = D_1A7BD1 ? 3 : 0;
    D_1AA5BA = D_1A7BD2 ? 3 : 0;
    D_1AA5D2 = D_1A7BD3 ? 3 : 0;
    D_1AA5EA = D_1A7BD4 ? 3 : 0;
    D_1AA602 = D_1A7BD6 ? 3 : 0;
}
#endif

/* Level-exit confirm dispatch for the active menu screen: validates the pending
 * pick against the global input flags + per-screen tables and sets g_nLevelExit*.
 * Near-miss: the chained per-screen pointer compares the later cc1 lays out
 * differently — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CDF48);

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE0C8);

/* Draw-batch wrapper: render one menu sub-element inside a 2D batch. */
s32 func_002CE200(void) {
    Begin2dDrawBatch(0);
    func_0029CF70();
    End2dDrawBatch();
    return 0;
}

/* menu helper: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE230);

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE3A0);

/* menu helper: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
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

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE618);

/* Draw-batch wrapper. */
s32 func_002CE6A8(void) {
    Begin2dDrawBatch(0);
    func_0029DA58();
    End2dDrawBatch();
    return 0;
}

/* menu helper: 8-byte-packed-save wall (saves 4 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE6D8);

/* GUI wrapper: when the GUI is up, register a widget (instance + 0x3A000) and
 * stash the returned handle in widget[0x34].
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern s32 func_00345298(void *widget);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE830);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002CE830(s32 *out) {
    if (g_guiInstance) {
        out[0xD] = func_00345298(g_guiInstance + 0x3A000);
    }
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002CE878(void) {
    Begin2dDrawBatch(0);
    func_0029D288();
    End2dDrawBatch();
    return 0;
}

/* menu helper: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE8A8);

/* menu helper: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
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

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CEA38);

/* menu input/update handler: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateBestiaryMenuInput);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 9 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
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

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 4 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawBestiaryPagingArrows);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 6 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawMenuPagingChrome);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 6 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawMenuItemSelectionBox);

/* GUI wrapper: when the GUI is up, register a widget (instance + 0x3C160) and
 * stash the returned handle in widget[0x34].
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern s32 func_00342468(void *widget);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D0110);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002D0110(s32 *out) {
    if (g_guiInstance) {
        out[0xD] = func_00342468(g_guiInstance + 0x3C160);
    }
    return 0;
}
#endif

/* menu helper: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
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

/* menu helper: 8-byte-packed-save wall (saves 7 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D02A0);

/* menu input/update handler: 8-byte-packed-save wall (saves 4 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateCheatMenuInput);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 9 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawCheatMenu);

/* return 0 stub. */
s32 func_002D0B40(void) {
    return 0;
}

/* menu input/update handler: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateSkillPointsMenu);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 8 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawSkillPointsMenu);

/* return 0 stub. */
s32 func_002D1150(void) {
    return 0;
}

/* menu input/update handler: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateExtrasMenuInput);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 6 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawExtrasMenu);

/* When the GUI is up, latch the extras-menu availability flags: always mark the
 * last screen id (=1), then set the per-feature "new" flags for each unlocked
 * extras feature (D_1AA450/D_1AA458) and, if any extras are unlocked
 * (g_miscExtras), the museum + master flags. Leaf.
 * Near-miss (~69%): the original fills each beqz delay slot with the next flag
 * store (and emits the museum/master pair in the opposite commutative order);
 * our cc1 leaves nops in the delay slots and stores in source order — the
 * later-cc1 branch-fill scheduling we can't reproduce from clean C. Preserved
 * as portable C. */
extern s32 g_lastMenuScreenId;
extern s32 D_1ABA50, D_1ABA54, D_1ABA58, D_1ABA4C;

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D1850);
#else
/* TODO(match): functional equivalent - not byte-exact; delay-slot branch-fill +
 * commutative store order not reproduced by cc1. */
s32 func_002D1850(void) {
    if (g_guiInstance) {
        g_lastMenuScreenId = 1;
        if (D_1AA450) {
            D_1ABA50 = 1;
        }
        if (D_1AA458) {
            D_1ABA54 = 1;
        }
        if (g_miscExtras) {
            D_1ABA58 = 1;
            D_1ABA4C = 1;
        }
    }
    return 0;
}
#endif

/* menu input/update handler: 8-byte-packed-save wall (saves 6 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", CinematicsMenuTick);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 6 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawCinematicsMenu);

/* When the GUI is up, mark each cinematics-menu row available iff its bit in
 * g_cinematicUnlockedFlags is set (sllv bit-test loop). The variable-shift mask
 * idiom isn't reproduced from clean C — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D1E88);

/* menu input/update handler: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdatePlanetWarpMenuInput);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 6 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawPlanetWarpMenu);

/* When the GUI is up and extras unlocked, latch the per-extra-feature
 * availability flags (D_1ABA71..D_1ABA77) from the source toggle bytes
 * (D_1A7BF2..D_1A7C0A). Near-miss: same ternary move/store delay-slot wall as
 * func_002CDEB0 — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D2538);

/* menu input/update handler: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateInsomniacMuseumInput);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 7 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
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

/* menu input/update handler: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateHelpTopicMenuInput);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawHelpTopicMenu);

/* return 0 stub. */
s32 func_002D2FC0(void) {
    return 0;
}

/* menu helper: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D2FC8);

/* menu helper: 8-byte-packed-save wall (saves 3 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
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

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D33A8);

/* menu helper: 8-byte-packed-save wall (saves 4 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D34E8);

/* return 0 stub. */
s32 func_002D37E0(void) {
    return 0;
}

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D37E8);

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
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

/* Menu confirm/cancel poll: on the "confirm" pad bit (0x10) returns the active
 * screen's pending result (latches it into screen[0x18]) or -1 when the screen
 * has no pending sub-result; on a "back/cancel" bit (0x900) returns 1; otherwise
 * ticks the idle handler func_0029D838 and returns 0.
 * Wall: 8-byte-packed-save (saves $16 + $31) + the load-PRE/branch-likely shape.
 * Preserved as portable C. */
extern void func_0029D838(void);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3F78);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002D3F78(void) {
    s32 flags = g_padButtonsPressed;
    s32 *ss = (s32 *)(g_particleFxBlob + 0x100);
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)ss + 0x14) + 0xE0);
        if (v != 0) {
            ss[6] = v; /* screen[0x18] */
            return 0;
        }
        if (ss[0x4D] == 0) { /* screen[0x134] */
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D838();
    return 0;
}
#endif

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

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D43B0);

/* Draw-batch wrapper. */
s32 func_002D4438(void) {
    Begin2dDrawBatch(0);
    func_0029D3D8();
    End2dDrawBatch();
    return 0;
}

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4468);

/* menu helper: 8-byte-packed-save wall (saves 2 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D44E8);

/* menu helper: 8-byte-packed-save wall (saves 6 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4568);

/* menu input/update handler: uses 128-bit sq/lq (vector) loads/stores — left as INCLUDE_ASM
 * (the EE quadword ops are not emitted from scalar C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdateShipCustomizeInput);

/* menu helper: 8-byte-packed-save wall (saves 9 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4D38);

/* menu/HUD draw routine: ldl/ldr/sdl/sdr unaligned struct/const copy — left as INCLUDE_ASM
 * (cc1 won't reproduce the unaligned 64-bit copy idiom from clean C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawShipCustomizeMenu);

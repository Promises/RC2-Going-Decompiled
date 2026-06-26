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
__asm__(".extern D_0025BA70, 16");
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

/* Menu-screen manager block (== g_particleFxBlob + 0x100). The same scratch the
 * functions above reach via `g_particleFxBlob + 0x100`; the SN cc1 anchors it on
 * its own symbol. */
extern u8 g_menuScreenBlock[];

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
extern char D_0025BA70[];

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

/* World camera default-pose targets (func_002CABC0). */
extern f32 g_cameraPos[4];     /* 0x1B52C0 - camera world position vec4 */
extern f32 g_cameraMatrix[12]; /* 0x1B54F0 - camera rotation matrix, 3 vec4 rows */

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
 * `daddu` copy + commutative-addu operand order not reproduced by cc1.
 * Returns the ADDRESS of the landed (non-empty) entry: the asm leaves
 * v0 = &entries[idx] at jr ra (not the entry value; sibling SelectNext returns
 * the value). The caller discards the result, so the game is unaffected. */
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
    return (s32)&entries[list[1]];   /* list[1] holds the final idx */
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

/* Clear a 20-entry s32 array (g_menuScreenBlock+0x16C..+0x1BC) to -1, back to
 * front. Wall: cc1 folds %lo(g_menuScreenBlock)+0x1BC into one addiu, but the
 * original keeps the symbol-%lo and the +0x1BC offset as two separate addiu (the
 * SN assembler-absolute macro shape). Preserved as portable C. (g_menuScreenBlock
 * 0x1F27C0 == the old D_1F27C0 - same address, named here.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CAB50);
#else
void func_002CAB50(void) {
    s32 *p = (s32 *)(g_menuScreenBlock + 0x1BC);
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

/* ResetWorldCamera: snap the world camera back to its default pose. Writes
 * g_cameraPos = (256, 256, 64) and rebuilds g_cameraMatrix as a 3x4 basis that
 * is identity on the diagonal (matrix[0]=matrix[5]=matrix[10]=1) plus a 1.0 in
 * the row-2 translation slot (matrix[11]). Takes no inputs and calls nothing -
 * a pure constant-store, so the native shim is byte-faithful to the asm. The
 * matching build keeps the asm: the original zeroes the matrix with 128-bit
 * `sq` writes that scalar C does not emit. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CABC0);
#else
void func_002CABC0(void) {
    s32 i;
    g_cameraPos[0] = 256.0f;
    g_cameraPos[1] = 256.0f;
    g_cameraPos[2] = 64.0f;
    for (i = 0; i < 12; i++) {
        g_cameraMatrix[i] = 0.0f;
    }
    g_cameraMatrix[0]  = 1.0f;
    g_cameraMatrix[5]  = 1.0f;
    g_cameraMatrix[10] = 1.0f;
    g_cameraMatrix[11] = 1.0f;
}
#endif

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
 * load-PRE keeps but ours eliminates — preserved as portable C. */
extern void DrawFullScreenTint(s32 r, s32 g, s32 b, s32 a);
#ifndef TARGET_NATIVE
/* NEAR-MISS (91%, not byte-exact): real C reaches this far — the body (bnel
 * branch-likely dead store, address-rematerialise-after-call, final-store order)
 * all match — but cc1 schedules the callee-save `sd ra` after only ONE of the
 * three zeroed-arg `move`s for DrawFullScreenTint, where the original interleaves
 * it after two (`daddu a0; daddu a1; sd ra; daddu a2`). That frame-save
 * scheduling slot has no C-level lever, so this stays INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", MenuScreenCommitTransition);
#else
/* TODO(match): functional equivalent - not byte-exact; the original keeps a dead
 * conditional store (block[0x4]=1 then unconditionally =0) the later cc1 load-PRE
 * retains but ours eliminates. The transient block[0x4]=1 has no observable effect
 * (no intervening call), so the captured block[0x18] -> block[0x14] latch and the
 * state=4 store are byte-faithful. */
void MenuScreenCommitTransition(void) {
    s32 *block = (s32 *)g_menuScreenBlock;
    DrawFullScreenTint(0, 0, 0, 0x38);
    block[0x14 / 4] = block[0x18 / 4]; /* latch pending sub-state */
    block[0]        = 4;               /* state = commit */
    block[0x4 / 4]  = 0;
    block[0x18 / 4] = 0;
}
#endif

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

/* Dispatch a menu action via a 2-case switch: action 0 -> result 1, action 1 ->
 * func_002D67A0(3, &D_0025BA70), default -> result 0. (MATCHED: the `switch`
 * reproduces cc1's early-out layout — the two cases emitted out-of-line after
 * the default fall-through that merges to a single tail return — which the
 * earlier if/else-if chain could not.) */
s32 func_002CC788(s32 action) {
    s32 result = 0;
    switch (action) {
    case 0:
        result = 1;
        break;
    case 1:
        result = func_002D67A0(3, D_0025BA70);
        break;
    }
    return result;
}

/* menu helper: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC7D8);

/* menu helper: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC858);

/* menu helper: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CC908);

/* 3-case menu action dispatch via switch, result merged to one tail return:
 *   action==0: arm g_nNanotechBonusHealTimer mirror (s16 at +4 = 0xA), result 1
 *   action==1: result = func_002D67A0(3, &D_0025BA70)
 *   action==2: func_0029DCB8(), result 1
 *   default:   result 0
 * (MATCHED: written as a `switch` so cc1 emits the comparison-tree dispatch with
 * out-of-line case bodies and a single-register result threaded to the tail; the
 * earlier ordered if-chain produced a structurally different body.) */
extern u8 g_nNanotechBonusHealTimer[];
extern void func_0029DCB8(void);
s32 func_002CCA18(s32 action) {
    s32 result = 0;
    switch (action) {
    case 0:
        *(s16 *)(g_nNanotechBonusHealTimer + 4) = 0xA;
        result = 1;
        break;
    case 1:
        result = func_002D67A0(3, D_0025BA70);
        break;
    case 2:
        func_0029DCB8();
        result = 1;
        break;
    }
    return result;
}

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
 * matching restore (frame artifact) that our cc1 won't reproduce from clean C.
 * Preserved as portable C for the native target. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CD4E8);
#else
/* TODO(match): functional equivalent - not byte-exact; anomalous +0x60 frame
 * prologue without matching restore not reproduced from clean C. The selector
 * is D_1A7318 (== g_vramTextureBase_28 + 0xC); movn picks &D_1AB648 when nonzero,
 * else &D_1AB678. Stores into list[0x34/4] (= list[0xD]) and returns 0. */
s32 func_002CD4E8(s32 *list) {
    list[0xD] = (s32)(D_1A7318 ? D_1AB648 : D_1AB678);
    return 0;
}
#endif

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

/* Galactic-map / level-select input dispatcher. Reads g_padButtonsPressed:
 *   - any nav-button bit (mask 0x910) set: returns 1 (the key is swallowed);
 *   - confirm bit (0x40) pressed: refresh the panel (func_0029CFA0) and capture
 *     its return as the SELECTION, then route to the matching confirm handler —
 *     func_002CC908, func_002CC7D8, func_002CCA18, or the func_002CC788/
 *     func_002CC858 dispatch chosen by the 0x31/0x1C map-cell codes from
 *     func_0026F800/func_0026F7F8 — passing the selection and returning the
 *     handler's result. The func_0026F7D0/D8/F0/E8 queries only GATE which
 *     handler runs; they are NOT the handler argument.
 *   - otherwise just refreshes the panel (func_0029CFA0).
 * If a handler accepted (result != 0), plays the confirm SFX (id 0x12).
 * EU twin func_002CE0B0 (byte-identical; region-shifted call targets).
 * Routes to tester-EE: drives live map/GUI state + PlayGlobalSound; not
 * standalone cmp-oracle'able.
 * Wall: the selection (func_0029CFA0's return) is moved into a saved reg in the
 * delay slot of the NEXT call (jal func_0026F7D0; daddu $16,$2,$0 — captures $2
 * BEFORE func_0026F7D0 runs, i.e. func_0029CFA0's result), via the EE 64-bit
 * `daddu rd,rs,zero` move idiom (plus a 1-GPR packed save) — not reproduced from
 * clean C. Preserved as portable C. */
extern s32 func_0029CFA0(s32 buttons);
extern s32 func_0026F7D0(void);
extern s32 func_0026F7D8(void);
extern s32 func_0026F7E8(void);
extern s32 func_0026F7F0(void);
extern s32 func_0026F7F8(void);
extern s32 func_0026F800(void);
extern s32 func_002CC788(s32 q);
extern s32 func_002CC7D8(s32 q);
extern s32 func_002CC858(s32 q);
extern s32 func_002CC908(s32 q);
extern s32 func_002CCA18(s32 q);
extern void PlayGlobalSound(s32 id, s32 a, s32 b);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE0C8);
#else
/* TODO(match): functional equivalent - not byte-exact; 64-bit `daddu` move idiom
 * + 1-GPR packed-save frame not reproduced by cc1. */
s32 func_002CE0C8(void) {
    s32 buttons = g_padButtonsPressed;
    s32 result = 0;

    if (buttons & 0x910) {
        result = 1;
    } else if (buttons & 0x40) {
        /* sel = func_0029CFA0(buttons)'s return ($16): refresh the popup panel,
         * forwarding the pad-buttons word ($a0 = g_padButtonsPressed at the asm
         * jal site), and reuse its return as the argument to every confirm
         * handler. The func_0026F7D0/D8/F0/E8 queries only gate which handler
         * runs. (Dropping the buttons arg drove a phantom nav sound at
         * 0x1886D0 — the BUG #22 residual.) */
        s32 sel = func_0029CFA0(buttons);
        if (func_0026F7D0() != 0 && func_0026F7D8() != 0) {
            result = func_002CC908(sel);
        } else if (func_0026F7F0() != 0) {
            result = func_002CC7D8(sel);
        } else if (func_0026F7E8() != 0) {
            result = func_002CCA18(sel);
        } else if (func_0026F800() == 0x31 || func_0026F7F8() != 0 ||
                   func_0026F800() == 0x1C) {
            result = func_002CC788(sel);
        } else {
            result = func_002CC858(sel);
        }
    } else {
        func_0029CFA0(buttons);
    }

    if (result != 0) {
        PlayGlobalSound(0x12, 0, 0);
    }
    return result;
}
#endif

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

/* Insomniac-museum (or sibling extras) screen draw: inside a 2D batch, run the
 * per-screen overlay (func_0029CFE0) and draw localized title string 0x2BE5 at
 * (D_1AB9D8, D_1AB9DC) in 0x80F0F0F0; then, if the GUI is up and its museum
 * widget handle (g_guiInstance+0x38000 .+0x79EC) is non-null, render that moby
 * model (BeginMobyDrawSegment .. FinishMobyRenderChain + the func_0034F9xx model
 * setup chain), wait one DMA fence and patch the moby packet's TEX0.
 * EU twin func_002CE388 (byte-identical; string id 0xB60, widget +0x7A9C,
 * region-shifted call/data targets).
 * Routes to tester-EE: drives the live GUI moby + DMA render path; not
 * standalone cmp-oracle'able.
 * Wall: `beql` branch-likely on the null-handle guard + EE 64-bit `daddu rd,rs,
 * zero` handle-copy idiom (1-GPR packed save) — not reproduced from clean C.
 * Preserved as portable C. */
extern void func_0029CFE0(void);
extern void func_002801B8(s32 x, s32 y, u64 color, char *str, s64 sel);
extern s32 D_1AB9D8, D_1AB9DC;
extern void BeginMobyDrawSegment(void);
extern void func_002A1000(void);
extern void func_002A1028(void);
extern void func_002A1058(void);
extern void FinishMobyRenderChain(void);
extern void func_0034F928(s32 handle);
extern void func_0034F9B8(s32 handle);
extern void func_0034F9F8(s32 handle);
extern void func_0034FAF8(s32 handle, s32 arg);
extern void WaitFrameDmaFence(s32 mask);
extern void PatchMobyPacketTex0(void);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE3A0);
#else
/* TODO(match): functional equivalent - not byte-exact; `beql` branch-likely null
 * guard + 64-bit `daddu` handle-copy idiom not reproduced by cc1. */
s32 func_002CE3A0(void) {
    s32 handle;

    Begin2dDrawBatch(0);
    func_0029CFE0();
    func_002801B8(D_1AB9D8, D_1AB9DC, 0x80F0F0F0, GetLocalizedString(0x2BE5), -1);
    End2dDrawBatch();

    if (g_guiInstance != NULL &&
        *(s32 *)(g_guiInstance + 0x38000 + 0x79EC) != 0) {
        BeginMobyDrawSegment();
        func_002A1000();
        func_002A1028();
        handle = *(s32 *)(g_guiInstance + 0x38000 + 0x79EC);
        func_0034F928(handle);
        func_0034F9B8(handle);
        func_0034F9F8(handle);
        func_0034FAF8(handle, handle + 0xC00);
        func_002A1058();
        FinishMobyRenderChain();
        WaitFrameDmaFence(0x10);
        PatchMobyPacketTex0();
    }
    return 0;
}
#endif

/* Per-screen menu tick + render-fence latch (one of the func_002CE498 family,
 * the cleanest with the standard menuScreenBlock confirm latch). Confirm (0x10)
 * latches the active screen's pending result (block[0x14]->0xE0 into block[0x18],
 * else -1/0); cancel (0x900) returns 1; otherwise it ticks the idle handler
 * func_0029D080(buttons, &scratch). Then, with the GUI up, it samples the frame
 * timestamp twice via func_00337D98 (a gp-relative frame counter); when the two
 * reads agree, the file-load is idle, and the per-screen present record
 * (D_00259C58) shows this frame already presented (offsets 0x50/0x54 == now and
 * state 0x44 in {2,4}) it CLEARS the "needs redraw" bit 0x4 of the live object
 * (*D_259C24)[0x10]; otherwise it SETS that bit. Finally it stamps the present
 * record (D_00259C58[0x58] = now).
 * Wall: 8-byte-packed-save ($16 + $17 + $31). Preserved as portable C. */
extern s32 func_00337D98(void);
extern void func_0029D080(s32 buttons, void *scratch);
extern s16 g_fileLoadState;
extern s32 D_259C24;       /* ptr-to-live-object global */
extern u8 D_00259C58[];    /* per-screen present record */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE498);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-GPR packed-save frame +
 * branch-likely present-record fence shape. */
s32 func_002CE498(void) {
    s32 t0 = func_00337D98();
    s32 flags = g_padButtonsPressed;
    s32 result = 0;
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        result = 1;
    } else {
        u8 scratch[0x30];
        func_0029D080(flags, scratch);
    }
    if (g_guiInstance) {
        s32 now = func_00337D98();
        u8 *rec = D_00259C58;
        s32 *live = (s32 *)D_259C24;
        s32 clear = 0;
        if (now == t0 && g_fileLoadState == 0) {
            if (*(s32 *)(rec + 0x50) == now && *(s32 *)(rec + 0x44) == 2) {
                clear = 1;
            } else if (*(s32 *)(rec + 0x54) == now &&
                       *(s32 *)(rec + 0x44) == 4) {
                clear = 1;
            }
        }
        if (clear) {
            live[0x10 / 4] &= ~0x4;
        } else {
            live[0x10 / 4] |= 0x4;
        }
        *(s32 *)(rec + 0x58) = now;
    }
    return result;
}
#endif

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

/* Confirm/cancel poll variant: on the "confirm" pad bit (0x10) acknowledges the
 * input (func_0028C7A8) then returns the active screen's pending result (latched
 * into block[0x18]) or -1 when the screen has no pending sub-result; on a
 * "back/cancel" bit (0x900) acknowledges + returns 1; otherwise ticks the idle
 * handler func_0029DA18 and returns 0.
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern void func_0028C7A8(void);
extern s32 func_0029DA18(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE618);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely confirm shape. */
s32 func_002CE618(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v;
        func_0028C7A8();
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        func_0028C7A8();
        return 1;
    }
    func_0029DA18(flags);
    return 0;
}
#endif

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

/* GUI accessor: when the GUI is up, mark the widget at instance+0x3C160 active
 * (func_00342460(w, 1)) and store its queried value (func_00342468(w)) into
 * out[0x34]. Returns 0.
 * Wall: 8-byte-packed-save ($16 + $17 + $31). Preserved as portable C. */
extern void func_00342460(void *widget, s32 arg);
extern s32 func_00342468(void *widget);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE8A8);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-GPR packed-save frame. */
s32 func_002CE8A8(s32 *out) {
    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C160;
        func_00342460(w, 1);
        out[0x34 / 4] = func_00342468(w);
    }
    return 0;
}
#endif

/* Menu confirm/cancel poll + command builder, twin of func_002D0158 without the
 * arg==0 sound. Confirm (0x10) latches the active screen's pending result
 * (block[0x14]->0xE0 into block[0x18], else -1/0); cancel (0x900) returns 1;
 * otherwise it ticks the idle handler func_0029D328 and, on the confirm pad bit
 * (0x40) with the GUI up, reads the selected entry of the list widget at
 * g_guiInstance+0x3C160 (func_003424C8) and builds an 8-byte command record
 * (op = (u16)entry[0x8] at rec+0x2, arg = entry[0xC] at rec+0x4) handed to
 * func_002D6B00 (-> MenuScreenDoAction).
 * Wall: 8-byte-packed-save ($16 + $17 + $31). Preserved as portable C. */
extern s32 func_0029D328(s32 padPressed);
extern void *func_003424C8(void *widget);
extern void func_002D6B00(void *record);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CE908);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely confirm shape / single-register result threading. */
s32 func_002CE908(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D328(flags);
    if ((*(s32 *)(D_138180 + 0x1C4) & 0x40) && g_guiInstance) {
        u8 record[0x30];
        u8 *entry = (u8 *)func_003424C8(g_guiInstance + 0x3C160);
        *(u16 *)(record + 0x2) = *(u16 *)(entry + 0x8);
        *(s32 *)(record + 0x4) = *(s32 *)(entry + 0xC);
        func_002D6B00(record);
    }
    return 0;
}
#endif

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

/* GUI wrapper: when the GUI is up, configure the widget at instance+0x3C160 —
 * mark it active (func_00342460(w,1)), bind its three data blobs
 * (func_00342450(w, &D_2615D8, &D_261678, &D_261730)) and clear its selection
 * (func_00342520(w,0)). Returns 0.
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern void func_00342460(void *widget, s32 arg);
extern void func_00342450(void *widget, void *a, void *b, void *c);
extern void func_00342520(void *widget, s32 arg);
extern u8 D_2615D8, D_261678, D_261730;
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002CEA38);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002CEA38(void) {
    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C160;
        func_00342460(w, 1);
        func_00342450(w, &D_2615D8, &D_261678, &D_261730);
        func_00342520(w, 0);
    }
    return 0;
}
#endif

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

/* Menu confirm/cancel poll + command builder, twin of func_002D4270. Confirm
 * (0x10) latches the active screen's pending result (block[0x14]->0xE0 into
 * block[0x18], else -1/0); cancel (0x900) returns 1; otherwise it ticks the idle
 * handler func_0029D398 and, on the confirm pad bit (0x40) with the GUI up, reads
 * the selected entry of the list widget at g_guiInstance+0x3C160 (func_003424C8)
 * and builds an 8-byte command record (op = (u16)entry[0x8] at rec+0x2, arg =
 * entry[0xC] at rec+0x4). When arg is 0 it plays UI sound 5, then hands the
 * record to func_002D6B00 (-> MenuScreenDoAction).
 * Wall: 8-byte-packed-save ($16 + $17 + $31). Preserved as portable C. */
extern s32 func_0029D398(s32 padPressed);
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D0158);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely confirm shape / single-register result threading. */
s32 func_002D0158(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D398(flags);
    if ((*(s32 *)(D_138180 + 0x1C4) & 0x40) && g_guiInstance) {
        u8 record[0x30];
        u8 *entry = (u8 *)func_003424C8(g_guiInstance + 0x3C160);
        s32 arg = *(s32 *)(entry + 0xC);
        *(u16 *)(record + 0x2) = *(u16 *)(entry + 0x8);
        *(s32 *)(record + 0x4) = arg;
        if (arg == 0) {
            PlayGlobalSound(5, 0, 0);
        }
        func_002D6B00(record);
    }
    return 0;
}
#endif

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

/* When the GUI is up, set g_lastMenuScreenId=1 then walk the 32-entry, 6-byte-
 * stride cinematics-menu row table (D_26183A): for each row, if any extras are
 * unlocked (g_miscExtras) mark the row available (+8 field = 1); otherwise test
 * the row's cinematic id (the +6 field) against g_cinematicUnlockedFlags and
 * write the bit result (1 set / 0 clear) into the same +8 availability field.
 * EU twin func_002D1E10 (byte-identical; table D_2615E2, region-shifted).
 * Wall: `bnel` branch-likely (the extras-set fast path's available-store sits in
 * the nullified delay slot) — the later cc1's branch-likely emission isn't
 * reproduced from clean C. Preserved as portable C. */
extern u8 g_cinematicUnlockedFlags[]; /* 0x139768 - cinematic-watched/unlocked bitfield */
extern u8 D_26183A[];                 /* cinematics-menu row table (6-byte stride records) */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D1E88);
#else
/* TODO(match): functional equivalent - not byte-exact; `bnel` branch-likely
 * delay-slot store on the extras-unlocked fast path not reproduced by cc1. */
void func_002D1E88(void) {
    u8 *row;
    if (g_guiInstance == NULL) {
        return;
    }
    g_lastMenuScreenId = 1;
    for (row = D_26183A; row < D_26183A + 0xC0; row += 6) {
        if (g_miscExtras != 0) {
            *(s16 *)(row + 8) = 1;
        } else {
            s16 id = *(s16 *)(row + 6);
            u32 mask = 1u << (id & 0x1F);
            if (*(u32 *)(g_cinematicUnlockedFlags + (id & ~3)) & mask) {
                *(s16 *)(row + 8) = 1;
            } else {
                *(s16 *)(row + 8) = 0;
            }
        }
    }
}
#endif

/* menu input/update handler: switch/jump-table dispatch (splat jtbl reloc gap) — left as
 * INCLUDE_ASM (cc1 jtbl layout not reproduced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", UpdatePlanetWarpMenuInput);

/* menu/HUD draw routine: 8-byte-packed-save wall (saves 6 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", DrawPlanetWarpMenu);

/* When the GUI is up and extras unlocked, latch the per-extra-feature
 * availability flags (g_planetWarpEnabled @0x1ABA70 + D_1ABA71..D_1ABA77) from a
 * pair of inputs: a per-feature "menu enabled" toggle byte (D_1A7BF2/F4/FB/C06/
 * C07/C0A) gating each block, and the relevant save-data completion bytes in the
 * progress block D_1395B8. Each available block stores a 1; only the planet-warp
 * block clears (the rest leave the flag untouched when their guard fails).
 * EU twin func_002D24C0 (byte-identical; toggles D_1A7C72.., block D_139638,
 * targets D_1ABAD8.., all region-shifted).
 * Wall: `bnel`/`beql` branch-likely (the value-1 move sits in the nullified
 * delay slot of the OR short-circuit tests) — the later cc1's branch-likely
 * emission isn't reproduced from clean C. Preserved as portable C. */
extern u8 g_planetWarpEnabled;   /* 0x1ABA70 - planet-warp menu enabled flag */
extern u8 D_1ABA71, D_1ABA72, D_1ABA73, D_1ABA74, D_1ABA75, D_1ABA76, D_1ABA77;
extern u8 D_1A7BF2, D_1A7BF4, D_1A7BFB, D_1A7C06, D_1A7C07, D_1A7C0A;
extern u8 D_1395B8[];            /* save-data per-feature completion block */
extern u8 D_1395E9;              /* planet-warp prerequisite completion byte */
extern s32 g_playerProgress;     /* 0x1A79F8 - current save progress slot */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D2538);
#else
/* TODO(match): functional equivalent - not byte-exact; `bnel`/`beql` branch-
 * likely delay-slot value moves not reproduced by cc1. */
void func_002D2538(void) {
    if (g_guiInstance == NULL) {
        return;
    }
    if (g_miscExtras == 0) {
        return;
    }
    if (D_1A7BF2 != 0) {
        if (D_1395E9 != 0) {
            g_planetWarpEnabled = 1;
        }
    }
    if (D_1A7BF4 != 0) {
        if (D_1395B8[0x4D] != 0 || D_1395B8[0x52] != 0) {
            D_1ABA71 = 1;
        }
    }
    if (D_1A7BFB != 0) {
        if (D_1395B8[0x57] != 0 || D_1395B8[0x5C] != 0) {
            D_1ABA72 = 1;
        }
    }
    if (D_1A7BFB != 0) {
        if (D_1395B8[0x3D] != 0) {
            D_1ABA73 = 1;
        }
    }
    if (D_1A7C0A != 0) {
        D_1ABA74 = 1;
    }
    if (D_1A7C06 != 0) {
        D_1ABA75 = 1;
    }
    if (D_1A7C07 != 0) {
        D_1ABA76 = 1;
    }
    if (D_1AA458 != 0 && g_playerProgress > 0) {
        D_1ABA77 = 1;
    }
}
#endif

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

/* Options sub-screen input handler. Confirm (0x10) latches the screen result
 * (resets g_optionsSubCursor) like the other polls. Cancel (0x900) resets the
 * cursor and returns 1. Up (0x8000) decrements the 6-entry cursor (clamp at 0);
 * Down (0x2000) increments (clamp at 5). Each move plays sound 3 (moved) or 5
 * (blocked at an edge). After a move, if the cursor changed it latches an error
 * code (-0x12C) into D_25CA80[0x3C]; then mirrors the cursor into D_25CB30 and
 * stores the s16 entry from the D_1ABDEA table into D_25CA80[0x34]. Returns the
 * confirm/cancel tri-state.
 * Wall: 8-byte-packed-save (saves $16 + $17 + $31). Preserved as portable C. */
extern s32 g_optionsSubCursor;
extern u8 D_25CA80[];
extern s32 D_25CB30;
extern u8 D_1ABDEA[]; /* s16 entries on a 4-byte stride */
extern void PlayGlobalSound(s32 id, s32 a, s32 b);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D2FC8);
#else
/* TODO(match): functional equivalent - not byte-exact; 3-GPR packed-save frame +
 * branch-likely / reload scheduling not reproduced by cc1. */
s32 func_002D2FC8(void) {
    s32 flags = g_padButtonsPressed;
    s32 old = g_optionsSubCursor;
    s32 result = 0;
    s32 cur;

    if (flags & 0x10) {
        s32 *block;
        s32 v;
        g_optionsSubCursor = 0;
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }

    if (flags & 0x900) {
        g_optionsSubCursor = 0;
        result = 1;
    } else if (flags & 0x8000) {
        PlayGlobalSound(old <= 0 ? 5 : 3, 0, 0);
        cur = g_optionsSubCursor - 1;
        g_optionsSubCursor = cur;
        if (cur < 0) {
            g_optionsSubCursor = 0;
        }
    } else if (flags & 0x2000) {
        PlayGlobalSound(old < 5 ? 3 : 5, 0, 0);
        cur = g_optionsSubCursor + 1;
        g_optionsSubCursor = cur;
        if (cur >= 6) {
            g_optionsSubCursor = 5;
        }
    }

    if (old != g_optionsSubCursor) {
        *(s32 *)(D_25CA80 + 0x3C) = -0x12C;
    }
    cur = g_optionsSubCursor;
    D_25CB30 = cur;
    *(s32 *)(D_25CA80 + 0x34) = *(s16 *)(D_1ABDEA + cur * 4);
    return result;
}
#endif

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

/* Help-topic sub-browser input handler. Confirm (0x10) latches the screen's
 * pending result like the other confirm polls (resets g_helpPageCursor to 0).
 * Cancel (0x900) returns 1. Up (0x8000) decrements the 7-page cursor clamped at
 * 0; Down (0x2000) increments clamped at 6. Each move plays sound 3 (moved) or
 * sound 5 (blocked at an edge). The landed page is mirrored into D_25CCC8.
 * Returns the confirm/cancel tri-state (1 / -1 / 0).
 * Wall: 8-byte-packed-save (saves $16 + $31) + branch-likely shape. Portable C. */
extern s32 g_helpPageCursor;
extern s32 D_25CCC8;
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D33A8);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely / reload scheduling not reproduced by cc1. */
s32 func_002D33A8(void) {
    s32 flags = g_padButtonsPressed;
    s32 result = 0;
    s32 page;

    if (flags & 0x10) {
        s32 *block = (s32 *)g_menuScreenBlock;
        s32 v;
        g_helpPageCursor = 0;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }

    if (flags & 0x900) {
        g_helpPageCursor = 0;
        result = 1;
        page = g_helpPageCursor;
        D_25CCC8 = page;
        return result;
    }

    if (flags & 0x8000) {
        s32 cur = g_helpPageCursor - 1;
        g_helpPageCursor = cur;
        PlayGlobalSound(cur < 0 ? 5 : 3, 0, 0);
        cur = g_helpPageCursor;
        if (cur < 0) {
            g_helpPageCursor = 0;
        }
    } else if (flags & 0x2000) {
        s32 cur = g_helpPageCursor + 1;
        g_helpPageCursor = cur;
        PlayGlobalSound(cur < 7 ? 3 : 5, 0, 0);
        cur = g_helpPageCursor;
        if (cur >= 7) {
            g_helpPageCursor = 6;
        }
    }

    page = g_helpPageCursor;
    D_25CCC8 = page;
    return result;
}
#endif

/* menu helper: 8-byte-packed-save wall (saves 4 GPRs incl $31; later cc1
 * packs save slots 8-byte vs our 16-byte) — left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D34E8);

/* return 0 stub. */
s32 func_002D37E0(void) {
    return 0;
}

/* Planet-warp / cinematic-camera confirm input handler. Reads g_padButtonsPressed:
 *   - Triangle (0x10): the same menu-screen "back/confirm" latch as the other
 *     menu input handlers — if the active screen instance (block[0x14]) has a
 *     pending result at +0xE0, store it into block[0x18] and return 0; else
 *     return -1 when block[0x134] is clear, 0 otherwise;
 *   - {L1|R1}=0x900: returns 1 (consume the page nav);
 *   - X (0x40): plays the confirm SFX (id 4); then if a cinematic camera is
 *     already active (D_1A790C) tears it down (clears D_1A790C + the two sound-
 *     bank cinematic-channel words at g_soundBankHandlesBlk+0x14F8/+0x1500),
 *     otherwise arms it (D_1A790C=1) and seeds the cinematic camera vector via
 *     Vec3RescaleToLenVu0(camBlock+0x10, soundBlkBase, 1.0f);
 *   - otherwise returns 0.
 * EU twin func_002D3770 (byte-identical; region-shifted symbols).
 * Routes to tester-EE: drives PlayGlobalSound + the VU0 Vec3RescaleToLenVu0
 * micro-op + live cinematic-camera state; not standalone cmp-oracle'able.
 * Wall: `beql` branch-likely on the +0xE0 latch + VU0/float-arg scheduling — not
 * reproduced from clean C. Preserved as portable C. */
extern s32 D_1A790C;             /* cinematic-camera-active latch */
extern u8 g_soundBankHandlesBlk[];   /* 0x189E20 - sound-bank handle block */
extern u8 g_cinematicCameraBlock[];  /* 0x18B2F0 - cinematic camera override block */
extern void Vec3RescaleToLenVu0(void *dst, void *src, f32 len);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D37E8);
#else
/* TODO(match): functional equivalent - not byte-exact; `beql` branch-likely latch
 * + VU0 float-arg scheduling not reproduced by cc1. */
s32 func_002D37E8(void) {
    s32 flags = g_padButtonsPressed;
    s32 result = 0;

    if (flags & 0x10) {
        s32 *block = (s32 *)g_menuScreenBlock;
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }

    if (flags & 0x900) {
        return 1;
    }

    if (flags & 0x40) {
        PlayGlobalSound(4, 0, 0);
        if (D_1A790C != 0) {
            D_1A790C = 0;
            *(s32 *)(g_soundBankHandlesBlk + 0x14F8) = 0;
            *(s32 *)(g_soundBankHandlesBlk + 0x1500) = 0;
        } else {
            D_1A790C = 1;
            Vec3RescaleToLenVu0(g_cinematicCameraBlock + 0x10,
                                g_cinematicCameraBlock + 0x10 - 0x14E0, 1.0f);
        }
    }

    return result;
}
#endif

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

/* Menu confirm/cancel poll variant (same shape as func_002D3F78): confirm bit
 * (0x10) latches the active screen's pending result into block[0x18] (or returns
 * -1 when no pending sub-result); cancel bit (0x900) returns 1; otherwise ticks
 * the idle handler func_0029D4E8 and returns 0.
 * Wall: 8-byte-packed-save ($16 + $31) with the result-threaded-$16 /
 * branch-likely merge shape the later cc1 emits. Preserved as portable C. */
extern s32 func_0029D4E8(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3BE0);
#else
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame +
 * branch-likely confirm shape / single-register result threading. */
s32 func_002D3BE0(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D4E8(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3C68(void) {
    Begin2dDrawBatch(0);
    func_0029D4B8();
    End2dDrawBatch();
    return 0;
}

/* Cinematics/cutscene confirm poll: on the confirm (0x10) and cancel (0x900) pad
 * bits it first "unlocks" the current cutscene record (struct at g_health+0x464):
 * clamps the play-count at +0x1D4 toward g_gsPixelOffsetY+0x3C, raises the
 * high-water mark at +0x1D8 to that same value, and sets the bit for the current
 * g_playerProgress slot (plus the 0x80000000 sentinel) in the seen-mask at +0x1DC.
 * Confirm then returns the active screen's pending result tri-state (latched into
 * block[0x18]); cancel returns 1; the idle path ticks func_0029D528 and returns 0.
 * Wall: 8-byte-packed-save ($16 + $31) + bnel branch-likely dead-store shape.
 * Preserved as portable C. */
extern s32 func_0029D528(s32 padPressed);
extern s32 g_health;            /* 0x18C2EC - base of the per-cutscene unlock records at +0x464 */
extern s32 g_gsPixelOffsetY;    /* 0x1A7354 - play-count source at +0x3C */
extern s32 g_playerProgress;    /* 0x1A79F8 - current progress slot (seen-mask bit index) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3C98);
#else
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame +
 * bnel branch-likely dead stores / single-register result threading. */
static void MenuCutsceneUnlockCurrent(void) {
    u8 *rec = (u8 *)&g_health + 0x464;
    s32 target = *(s32 *)((u8 *)&g_gsPixelOffsetY + 0x3C);
    if (*(u16 *)(rec + 0x1D4) <= 0xFFFE) {
        *(u16 *)(rec + 0x1D4) = (u16)(target + 1);
    }
    if (*(s32 *)(rec + 0x1D8) < target) {
        *(s32 *)(rec + 0x1D8) = target;
    }
    *(u32 *)(rec + 0x1DC) |= (1u << g_playerProgress) | 0x80000000u;
}
s32 func_002D3C98(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v;
        MenuCutsceneUnlockCurrent();
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        MenuCutsceneUnlockCurrent();
        return 1;
    }
    func_0029D528(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3DD8(void) {
    Begin2dDrawBatch(0);
    func_0029D568();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll variant (idle handler func_0029D598). Same shape as
 * func_002D3F78. Wall: 8-byte-packed-save ($16 + $31) + branch-likely confirm
 * shape / single-register result threading. Preserved as portable C. */
extern s32 func_0029D598(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3E08);
#else
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame. */
s32 func_002D3E08(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D598(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3E90(void) {
    Begin2dDrawBatch(0);
    func_0029D5D8();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll variant (idle handler func_0029D608). Same shape as
 * func_002D3F78. Wall: 8-byte-packed-save ($16 + $31) + branch-likely confirm
 * shape / single-register result threading. Preserved as portable C. */
extern s32 func_0029D608(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D3EC0);
#else
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame. */
s32 func_002D3EC0(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D608(flags);
    return 0;
}
#endif

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
extern s32 func_0029D838(s32 padPressed);
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
    func_0029D838(flags);
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

/* Menu confirm/cancel poll variant (idle handler func_0029D7C8). Same shape as
 * func_002D3F78. Wall: 8-byte-packed-save ($16 + $31) + branch-likely confirm
 * shape / single-register result threading. Preserved as portable C. */
extern s32 func_0029D7C8(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4030);
#else
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame. */
s32 func_002D4030(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D7C8(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D40B8(void) {
    Begin2dDrawBatch(0);
    func_0029D808();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll variant with a deferred game-state action: confirm
 * (0x10) and cancel (0x900) behave like func_002D3F78; the idle path ticks
 * func_0029D678 and, when that signals (nonzero), fires MenuScreenDoAction(0xD,0,
 * &outFlag) — opcode 0xD is a RequestGameStateChange — with the out-flag slot
 * pre-zeroed. Wall: 8-byte-packed-save ($16 + $31). Preserved as portable C. */
extern s32 func_0029D678(s32 padPressed);
extern s32 MenuScreenDoAction(s32 opcode, s32 arg, s32 *outFlag);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D40E8);
#else
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame +
 * branch-likely confirm shape. */
s32 func_002D40E8(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    if (func_0029D678(flags) != 0) {
        s32 outFlag = 0;
        MenuScreenDoAction(0xD, 0, &outFlag);
    }
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4188(void) {
    Begin2dDrawBatch(0);
    func_0029D6B8();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll variant (idle handler func_0029D6E8). Same shape as
 * func_002D3F78. Wall: 8-byte-packed-save ($16 + $31) + branch-likely confirm
 * shape / single-register result threading. Preserved as portable C. */
extern s32 func_0029D6E8(s32 padPressed);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D41B8);
#else
/* TODO(match): functional equivalent - not byte-exact; 1-GPR packed-save frame. */
s32 func_002D41B8(void) {
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)g_menuScreenBlock;
    if (flags & 0x10) {
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D6E8(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4240(void) {
    Begin2dDrawBatch(0);
    func_0029D728();
    End2dDrawBatch();
    return 0;
}

/* Menu confirm/cancel poll driven by the global input flag word (D_138180[0x1C4])
 * that also forwards a selected GUI-list entry as a menu command. Confirm (0x10)
 * latches the active screen's pending result like the other polls; cancel (0x900)
 * returns 1; otherwise it ticks the idle handler func_0029D758 and, on the
 * confirm pad bit (0x40) with the GUI up, reads the selected entry of the list
 * widget at g_guiInstance+0x3F3F0 (func_0033F360) and builds a command record
 * (opcode = entry[0x8], arg = entry[0xC]) handed to func_002D6B00 (-> MenuScreenDoAction).
 * Wall: 8-byte-packed-save ($16 + $17 + $31). Preserved as portable C. */
extern s32 func_0029D758(s32 padPressed);
extern void *func_0033F360(void *widget);
extern void func_002D6B00(void *record);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4270);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely confirm shape / single-register result threading. */
s32 func_002D4270(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D758(flags);
    if ((*(s32 *)(D_138180 + 0x1C4) & 0x40) && g_guiInstance) {
        u8 record[0x30];
        u8 *entry = (u8 *)func_0033F360(g_guiInstance + 0x3F3F0);
        *(u16 *)(record + 0x2) = *(u16 *)(entry + 0x8);
        *(s32 *)(record + 0x4) = *(s32 *)(entry + 0xC);
        func_002D6B00(record);
    }
    return 0;
}
#endif

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

/* Confirm/cancel poll driven by the global input flag word (D_138180[0x1C4]).
 * Confirm (0x10) latches the active screen's pending result like the other
 * polls; cancel (0x900) returns 1; otherwise ticks the idle handler
 * func_0029D408 with D_138180[0x1C0] and returns 0.
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern void func_0029D408(s32 arg);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D43B0);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame +
 * branch-likely confirm shape. */
s32 func_002D43B0(void) {
    s32 flags = *(s32 *)(D_138180 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)g_menuScreenBlock;
        v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            return 0;
        }
        if (block[0x134 / 4] == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029D408(*(s32 *)(D_138180 + 0x1C0));
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4438(void) {
    Begin2dDrawBatch(0);
    func_0029D3D8();
    End2dDrawBatch();
    return 0;
}

/* GUI wrapper: when the GUI is up, configure the list widget at instance+0x3C480
 * — func_003444D0(w, 0) (mode), func_00344458(w, &D_259F38) (bind data),
 * func_003444A0(w) (rebuild), func_003444C0(w, &D_25D0C0) (bind labels). Returns 0.
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
extern void func_003444D0(void *widget, s32 mode);
extern void func_00344458(void *widget, void *data);
extern void func_003444A0(void *widget);
extern void func_003444C0(void *widget, void *labels);
extern u8 D_259F38, D_25D0C0, D_259CC0, D_25D268;
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D4468);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002D4468(void) {
    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C480;
        func_003444D0(w, 0);
        func_00344458(w, &D_259F38);
        func_003444A0(w);
        func_003444C0(w, &D_25D0C0);
    }
    return 0;
}
#endif

/* GUI wrapper: twin of func_002D4468 for the same widget (instance+0x3C480) with
 * the alternate mode/data/labels (func_003444D0(w,1), &D_259CC0, &D_25D268).
 * Wall: 8-byte-packed-save (saves $16 + $31). Preserved as portable C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1CA080", func_002D44E8);
#else
/* TODO(match): functional equivalent - not byte-exact; 2-GPR packed-save frame. */
s32 func_002D44E8(void) {
    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C480;
        func_003444D0(w, 1);
        func_00344458(w, &D_259CC0);
        func_003444A0(w);
        func_003444C0(w, &D_25D268);
    }
    return 0;
}
#endif

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

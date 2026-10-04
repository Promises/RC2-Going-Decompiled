#include "common.h"

/* common.h does not pull in vec.h; the #else models of the 128-bit lq/sq
 * vertex copies need a 16-byte Vec4 (matches the USA 1CA080 twin). Only the
 * portable arm uses it — the matching build is unaffected. */
#ifdef TARGET_NATIVE
typedef struct Vec4 { f32 x, y, z, w; } __attribute__((aligned(16))) Vec4;
#endif

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
/* Insomniac-museum title label Y word (func_002CE388), same abs lui/%lo shape. */
__asm__(".extern D_1ABA44, 16");

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

/* func_002CA4F0: EU twin of USA func_002CA618 — sample a piecewise-linear Vec4
 * path at distance `dist`. The path is a leading s32 count followed by a Vec4
 * array at +0x10 (0x10 stride). Segment index = floor(dist / segLen); the
 * output index (*outSeg), fractional remainder (*outFrac) and interpolated
 * point (*outVec) are written. Index is clamped to [0, count-1): below 0 ->
 * segment 0, at/above the last interpolable segment -> the last one, and in
 * both clamp cases the frac is 0 and the point is the segment's start vertex.
 * In range, the point is path[seg] + normalize(path[seg] - path[seg+1]) * frac
 * (VU0 vector helpers). Matching arm stays INCLUDE_ASM (128-bit lq/sq vertex
 * copies the scalar matcher can't emit); the #else is the structure-exact
 * model. Callees are the EU twins of the USA vec/convert helpers. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CA4F0);
#else
extern s32  func_002845B0(f32 v);                         /* FloatToInt */
extern f32  IntToFloat(s32 v);                         /* IntToFloat */
extern void func_002835B0(void *dst, void *a, void *b);   /* Vec4SubVu0 */
extern void Vec4AddVu0(void *dst, void *a, void *b);   /* Vec4AddVu0 */
extern void func_002837E0(void *dst, void *src, f32 len); /* Vec3RescaleToLenVu0 */
void func_002CA4F0(u8 *path, s32 *outSeg, f32 *outFrac, Vec4 *outVec, f32 dist,
                   f32 segLen) {
    s32 seg = func_002845B0(dist / segLen);
    s32 last = *(s32 *)path - 1;

    *outSeg = seg;
    if (seg < last) {
        if (seg >= 0) {
            Vec4 *start = (Vec4 *)(path + seg * 0x10 + 0x10);
            Vec4 *next  = (Vec4 *)(path + seg * 0x10 + 0x20);
            f32 frac = dist - IntToFloat(seg) * segLen;
            Vec4 tmp;

            *outFrac = frac;
            func_002835B0(&tmp, start, next);
            *outVec = tmp;
            func_002837E0(outVec, outVec, frac);
            Vec4AddVu0(&tmp, outVec, start);
            *outVec = tmp;
            return;
        }
        *outSeg = 0;
    } else {
        *outSeg = last;
    }
    *outFrac = 0.0f;
    *outVec = *(Vec4 *)(path + *outSeg * 0x10 + 0x10);
}
#endif

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

/* List-scroller "select previous": decrement the cursor (list[1]); when it drops
 * to 0 or below, wrap to the limit (list[0]). Then skip backwards over empty (==0)
 * slots in the entry array (list+0xC, one s32/row); returns the ADDRESS of the
 * landed entry (asm leaves v0=&entries[idx] at jr; caller discards it). Matching
 * arm stays INCLUDE_ASM (EE 64-bit sign-extend daddu copy + commutative addu order
 * not reproduced by cc1); #else is the structure model. Word-verified vs USA
 * ListScrollerSelectPrev: register-only, no reloc/callee symbols. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", ListScrollerSelectPrev);
#else
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
    return (s32)&entries[list[1]];
}
#endif

/* List-scroller "select next": increment the cursor (list[1]); when it passes the
 * limit (list[0]) wrap to 0, then skip forward over empty (==0) slots in the entry
 * array (list+0xC). Returns the entry value the cursor lands on. Matching arm stays
 * INCLUDE_ASM (commutative addu operand order not reproduced by cc1); #else is the
 * structure model. Word-verified vs USA ListScrollerSelectNext: register-only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", ListScrollerSelectNext);
#else
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

/* Clear a 20-entry s32 array (menu block +0x1BC..+0x16C) to -1, back to front.
 * Matching arm stays INCLUDE_ASM (SN keeps symbol-%lo and +0x1BC as two separate
 * addiu); #else is the structure model. Word-verified vs USA func_002CAB50:
 * g_menuScreenBlock -> D_001F0000+0x2840 (EU +0x80 data lane). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CAA18);
#else
void func_002CAA18(void) {
    s32 *p = (s32 *)(D_001F0000 + 0x2840 + 0x1BC);
    s32 i = 0x13;
    do {
        *p = -1;
        i--;
        p--;
    } while (i >= 0);
}
#endif

/* Store a reciprocal into the menu scratch: [0x1C0]=1.0f, [0x1C8]=0, [0x1C4]=1/x.
 * Matching arm stays INCLUDE_ASM (cc1 anchors the store base at +0x1C0 via CSE);
 * #else is the structure model. Word-verified vs USA func_002CAB90: D_1F27C0 ->
 * D_001F0000+0x2840 (EU +0x80 data lane); float arg via $f12. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CAA58);
#else
void func_002CAA58(f32 x) {
    f32 one = 1.0f;
    u8 *p = D_001F0000 + 0x2840;
    *(f32 *)(p + 0x1C0) = one;
    *(s32 *)(p + 0x1C8) = 0;
    *(f32 *)(p + 0x1C4) = one / x;
}
#endif

/* ResetWorldCamera: snap the world camera to its default pose. g_cameraPos =
 * (256,256,64) and g_cameraMatrix rebuilt as an identity-diagonal 3x4 basis with a
 * 1.0 in the row-2 translation slot ([11]). Matching arm stays INCLUDE_ASM (original
 * zeroes the matrix with 128-bit sq writes scalar C can't emit); #else is the
 * structure model. Word-verified vs USA func_002CABC0: EU splat anchors these globals
 * via g_nVendorBuyQuantity+0x2FB8 = the +0x80 twins of USA g_cameraPos/g_cameraMatrix
 * (named here to match the TickFrontEndScreenIdle #else precedent). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CAA88);
#else
void func_002CAA88(void) {
    extern f32 g_cameraPos[4];      /* EU 0x1B5340 (g_nVendorBuyQuantity+0x30F8); USA 0x1B52C0 */
    extern f32 g_cameraMatrix[12];  /* EU 0x1B5570 (g_nVendorBuyQuantity+0x3328); USA 0x1B54F0 */
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

/* RequestMenuScreenChange: open or switch a front-end / pause screen. If leaving the
 * in-game pause overlay for anything but the audio-options sub-screen (prev state 4 AND
 * overlay mode != 8), clears the block's overlay-active flag (+0x1F4). When no change is
 * queued it silences the dialog voice. Marks the screen live (block[0]=1), builds the
 * pause prompt when not in-game, picks the list-vtable flag from D_1A7A90 and block[0x108],
 * wires the two list records, latches the active screen id into block[0x8], clears
 * transition fields, and — unless returning to the map (screen==6) — re-syncs the galactic
 * map. Marks the change committed (block[0x14C]=1). Matching arm stays INCLUDE_ASM (8-byte-
 * packed 4-GPR save + branch-likely shapes); #else is the structure model. Word-verified vs
 * USA RequestMenuScreenChange: g_menuScreenBlock->D_001F0000+0x2840; GetPrevGameState->
 * func_002B57C8, GetMenuOverlayMode->func_00286070, func_00132AF8->func_00132B58,
 * SetDialogVoiceVolumesMax->func_002B7EA0, func_002DFE60->func_002DFE20; D_1A7A10->D_1A7A90,
 * D_002598F8/AD8/A88->D_00259918/AF8/AA8, D_25B5D8->D_25B5F8 (EU +0x80 data lane). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", RequestMenuScreenChange);
#else
void RequestMenuScreenChange(s32 screen) {
    extern s32  func_002B57C8(void);     /* USA GetPrevGameState */
    extern s32  func_00286070(void);     /* USA GetMenuOverlayMode */
    extern void func_00132B58(s32 arg);  /* USA func_00132AF8 */
    extern void func_002B7EA0(s32 arg);  /* USA SetDialogVoiceVolumesMax */
    extern void BuildPausePromptPopup(void);
    extern void MapSetCurrentLevel(s32 level);
    extern void UpdateLevelObjectiveStates(void);
    extern void func_002DFE20(void);     /* USA func_002DFE60 */
    extern void snd_Pump(void);
    extern s32  g_nGameState;
    extern s32  D_1A7A90;                /* USA D_1A7A10 */
    extern u8   D_00259918[], D_00259AF8[], D_00259AA8[]; /* USA D_002598F8/AD8/A88 */
    extern s32  D_25B5F8;                /* USA D_25B5D8 */
    extern s32  g_playerProgress;
    u8 *mb = D_001F0000 + 0x2840;
    s32 cur;

    if (!(func_002B57C8() == 4 && func_00286070() == 8)) {
        *(s32 *)(mb + 0x1F4) = 0;
    }

    if (*(s32 *)(mb + 0x1F4) == 0) {
        func_00132B58(0x5D);
        func_002B7EA0(0);
        *(u8 *)(mb + 0xDB) = 0;
        if (*(s32 *)(mb + 0x1F4) == 0) {
            snd_Pump();
        }
    } else {
        *(u8 *)(mb + 0xDB) = 0;
    }

    *(s32 *)(mb + 0) = 1;
    if (g_nGameState == 0) {
        BuildPausePromptPopup();
    }

    *(s32 *)(mb + 0x144) = 0;
    *(s32 *)(mb + 0x148) = 0;
    if ((D_1A7A90 & 0xFFFF0000) != 0 || *(s32 *)(mb + 0x108) != 0) {
        *(s32 *)(mb + 0xE8) = 1;
    } else {
        *(s32 *)(mb + 0xE8) = 0;
    }
    if (*(s32 *)(mb + 0xE8) != 0) {
        *(u8 **)(D_00259918 + 0x38) = D_00259AF8;
        *(u8 **)(D_00259AA8 + 0x3C) = D_00259AF8;
    } else {
        *(u8 **)(D_00259918 + 0x38) = D_00259AA8;
        *(u8 **)(D_00259AA8 + 0x3C) = D_00259918;
    }

    cur = *(s32 *)(mb + 0x104);
    *(s32 *)(mb + 0x1C8) = 0;
    *(s32 *)(mb + 0x1C) = 0;
    *(s32 *)(mb + 0x8) = (cur == 0) ? screen : cur;
    *(s32 *)(mb + 0x20) = 0;
    *(s32 *)(mb + 0x120) = 0;
    *(s32 *)(mb + 0x1C0) = 0;
    *(s32 *)(mb + 0x1C4) = 0;
    if (screen != 6) {
        MapSetCurrentLevel(g_playerProgress);
        UpdateLevelObjectiveStates();
        D_25B5F8 = 0;
        func_002DFE20();
    }
    *(s32 *)(mb + 0x1F4) = 0;
    *(s32 *)(mb + 0x14C) = 1;
    *(s32 *)(mb + 0x150) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", CaptureScreenToVram);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", RestoreScreenFromVram);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", MenuScreenLoad);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CB410);

/* TickFrontEndScreenIdle: EU twin of USA func_002CB720 — enter/refresh a menu screen's
 * render context. Resets the screen state word, publishes the screen's stored
 * camera position (two Vec4s at +0x50/+0x60) and view block to the live camera
 * globals, rebuilds the camera projection + frame view matrices, swaps to moby
 * table 0, clears the per-frame scratch fields, and — for the fade-in screen
 * kinds {3,4,5,6} — fades from black. When not already shut down (+0x1F4 == 0)
 * it also runs the frame's sound service: a fixed sound event (0x5D),
 * dialog-voice mute (unless a level exit is pending on kind 2), the sound pump,
 * emitter update, and a one-shot dialog-voice pump gated by the +0xDB flag.
 * Matching arm stays INCLUDE_ASM (128-bit lq/sq camera-vec copies); #else is
 * the structure-exact model. Globals/callees retargeted to their EU
 * equivalents: the menu block is the file's D_001F0000+0x2840 scratch (USA
 * g_menuScreenBlock), the camera globals are the standard +0x80 EU twins, and
 * the callees are the EU func_ twins (USA name in each comment). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", TickFrontEndScreenIdle);
#else
extern void func_002FD190(void);                          /* func_002FCFC8 */
extern void func_00283370(void *dst, void *src, s32 len); /* func_00283460 (qword copy) */
extern void func_0027AF28(void);                          /* BuildCameraProjection */
extern void func_0027A100(void);                          /* BuildFrameViewMatrices */
extern void SwapMobyTableContext(s32 tableId);                   /* SwapMobyTableContext */
extern void func_0027A3D8(void);                          /* func_0027A550 */
extern void func_0027D818(s32 frames);                    /* FadeOutToBlackBlocking */
extern void func_00132B88(s32 id);                        /* func_00132B28 */
extern s32  IsLevelExitRequested(void);
extern void func_002B7ED8(void);                          /* SetDialogVoiceVolumesMute */
extern void func_002E5898(void);                          /* UpdateSoundEmitters */
extern void func_002B8898(s32 blocking);                  /* PumpDialogVoiceSystem */
extern void func_002B90D0(void);                          /* func_002B9438 */
extern void snd_Pump(void);
/* Camera world-position vec4 (USA g_cameraPos @0x1B52C0; EU twin @0x1B5340, +0x80). */
extern f32 g_cameraPos[4];
/* Projection scale cot(fov/2) (USA g_cameraProjScale @0x1B9070; EU twin @0x1B90F0, +0x80). */
extern f32 g_cameraProjScale;
void TickFrontEndScreenIdle(void) {
    u8 *scr = D_001F0000 + 0x2840;   /* EU menu screen-state block (USA g_menuScreenBlock) */
    s32 kind;

    func_002FD190();
    *(s32 *)scr = 0;
    *(Vec4 *)g_cameraPos = *(Vec4 *)(scr + 0x50);
    *(Vec4 *)((u8 *)g_cameraPos + 0x10) = *(Vec4 *)(scr + 0x60);
    func_00283370((u8 *)g_cameraPos + 0x230, scr + 0x200, 0x30);
    g_cameraProjScale = *(f32 *)(scr + 0xF8);
    func_0027AF28();   /* BuildCameraProjection */
    func_0027A100();   /* BuildFrameViewMatrices */
    SwapMobyTableContext(0);  /* SwapMobyTableContext */
    func_0027A3D8();
    *(s32 *)(scr + 0x118) = 0;
    *(s32 *)(scr + 0x11C) = 0;
    *(s32 *)(scr + 0x114) = 0;
    *(s32 *)(scr + 0x20) = 0;

    kind = *(s32 *)(scr + 0x1C);
    if ((u32)(kind - 3) < 2 || kind == 6 || kind == 5) {
        func_0027D818(0xD);    /* FadeOutToBlackBlocking - GENUINE region diff:
                                * EU fades 0xD (13) frames @50Hz vs USA 0x10
                                * (16) @60Hz, matching wall-clock fade duration */
    }

    if (*(s32 *)(scr + 0x1F4) == 0) {
        func_00132B88(0x5D);
        if (*(s32 *)(scr + 0x1C) != 2 || !IsLevelExitRequested()) {
            func_002B7ED8();   /* SetDialogVoiceVolumesMute */
        }
        snd_Pump();
    }

    func_002E5898();   /* UpdateSoundEmitters */
    if (scr[0xDB] != 0) {
        func_002B8898(1);  /* PumpDialogVoiceSystem */
        scr[0xDB] = 0;
    }
    func_002B90D0();   /* func_002B9438 */
}
#endif

/* Front-end screen-machine per-frame tick (TickFrontEndScreenMachine). Advances the
 * menu-idle counter (saturating at 0x7D00) and transition countdown, runs the area-
 * transition fade timer, lets the save/load driver (func_002DECA0) run, and — if a
 * save/load op finished while state 4 has a level-exit queued — kicks the game-state
 * change. Then dispatches the current screen state (block[0], 1..5) and, for non-terminal
 * states, runs the shared moby + sound-emitter tick and optional post-hook. Matching arm
 * stays INCLUDE_ASM (cc1 jtbl reloc layout); #else is the structure model. Word-verified vs
 * USA func_002CB860: g_areaTable->D_139460, func_002DECE0->func_002DECA0, case handlers
 * func_002CBA10/A40/BD68->MenuScreenBeginLoad/func_002CB8F0/func_002CBC18, UpdateSoundEmitters->func_002E5898,
 * UpdateActiveMobys->func_002B7210, func_002CB560->func_002CB410, g_hudClutSlots+0x10->
 * g_pActiveTextTable+0x68. GENUINE region diff: case-5 heal-timer 0xA(60Hz)->0x8(50Hz).
 * The USA-model omission of the RequestGameStateChange()==0 early-return (present in both
 * regions' asm) is preserved for lockstep parity. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", TickFrontEndScreenMachine);
#else
s32 TickFrontEndScreenMachine(void) {
    extern u8   D_139460[];              /* USA g_areaTable */
    extern u8   g_nSaveLoadStatusCode[];
    extern u8   g_nLevelExitDestination[];
    extern u8   g_nNanotechBonusHealTimer[];
    extern u8   g_pActiveTextTable[];    /* USA g_hudClutSlots (base +0x68 zero-store) */
    extern s32  RequestGameStateChange(s32 stateId, s32 push, s32 c, s32 d, s32 e);
    extern void func_002DECA0(void);     /* USA func_002DECE0 */
    extern void func_002B7210(void);     /* USA UpdateActiveMobys */
    extern void MenuScreenBeginLoad(void);     /* same name in USA (defined later, file scope) */
    extern void func_002CB8F0(void);     /* USA func_002CBA40 (defined later, file scope) */
    extern void func_002CBC18(void);     /* USA func_002CBD68 */
    extern void func_002CB410(void);     /* USA func_002CB560 */
    extern void MenuScreenCommitTransition(void);
    extern void MenuScreenUpdate(void);
    u8 *mb = D_001F0000 + 0x2840;
    s32 counter;

    *(s32 *)(mb + 0x168) = 0;
    counter = *(s32 *)(mb + 0x120);
    *(s32 *)(mb + 0x120) = (counter > 0x7CFF) ? 0x7D00 : counter + 1;
    if (*(s32 *)(mb + 0x158) != 0)
        *(s32 *)(mb + 0x158) -= 1;

    if (*(s32 *)(D_139460 + 0x15C) < 3 &&
        *(s32 *)(D_139460 + 0x164) < 0)
        *(s32 *)(mb + 0x164) += 1;
    else
        *(s32 *)(mb + 0x164) = 0;

    func_002DECA0();

    if ((*(s32 *)(g_nSaveLoadStatusCode + 4) & 1) &&
        *(s32 *)mb == 4 &&
        *(s32 *)(g_nLevelExitDestination + 4) >= 8)
        RequestGameStateChange(4, 1, 1, *(s32 *)(mb + 0x14), 0);

    switch (*(s32 *)mb) {
    case 1:
        MenuScreenBeginLoad();
        break;
    case 2:
        func_002CB8F0();
        break;
    case 3:
        MenuScreenCommitTransition();
        break;
    case 4:
        MenuScreenUpdate();
        break;
    case 5:
        func_002CBC18();
        func_002E5898();
        *(s16 *)(g_nNanotechBonusHealTimer + 4) = 0x8; /* region diff: EU 0x8 @50Hz vs USA 0xA @60Hz */
        *(s32 *)(g_pActiveTextTable + 0x68) = 0;
        return 0;
    default:
        break;
    }

    func_002B7210();
    func_002E5898();
    if (*(s32 *)(mb + 0x1C) != 0)
        func_002CB410();
    return 0;
}
#endif

/* MenuScreenLoad then mark the screen-state scratch ready (state=2). */
void MenuScreenBeginLoad(void) {
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

/* func_002CBC18: EU twin of USA func_002CBD68 — screen-state-5 handler (terminal
 * "leaving the front-end" state). Runs a per-frame countdown (block+0x24); once it
 * hits zero and no file load is in flight, commits the queued transition by its
 * sub-state (block+0x1C): 2 = pop back to the map, 3/4/6 = push game-state 1 with the
 * queued arg (+0xF4), 5 = push game-state 2, else plain pop. Then, if the equipped-
 * weapon slot changed (+0x40 != 0 and != +0x30), plays that weapon's voice line via
 * g_weaponTable, bracketed by dialog-voice pumps. Matching arm stays INCLUDE_ASM (later
 * cc1 packs 8-byte save slots vs our 16); #else is the structure model. Word-verified vs
 * USA func_002CBD68: g_menuScreenBlock->D_001F0000+0x2840; g_fileLoadState(s16)->
 * g_saveImageArea+0x1004 (lh); PumpDialogVoiceSystem->func_002B8898, func_00294CD0->
 * func_00294D30 (g_mapCurrentLevel/PopGameState/RequestGameStateChange/g_itemEquippedSlot/
 * g_weaponTable keep names). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CBC18);
#else
void func_002CBC18(void) {
    extern u8   g_saveImageArea[];              /* USA g_fileLoadState @ +0x1004 (s16) */
    extern s32  g_mapCurrentLevel;
    extern u8   g_itemEquippedSlot[];
    extern u8   g_weaponTable[];
    extern void PopGameState(s32 a, s32 b);
    extern s32  RequestGameStateChange(s32 stateId, s32 push, s32 c, s32 d, s32 e);
    extern void func_002B8898(s32 blocking);    /* USA PumpDialogVoiceSystem */
    extern void func_00294D30(s32 arg);         /* USA func_00294CD0 */
    u8 *mb = D_001F0000 + 0x2840;
    s32 sub;

    if (*(s32 *)(mb + 0x24) != 0)
        *(s32 *)(mb + 0x24) -= 1;
    if (*(s32 *)(mb + 0x24) != 0)
        return;
    if (*(s16 *)(g_saveImageArea + 0x1004) != 0)
        return;

    sub = *(s32 *)(mb + 0x1C);
    if (sub == 2) {
        PopGameState(0, g_mapCurrentLevel);
    } else if (sub == 3 || sub == 4 || sub == 6) {
        RequestGameStateChange(1, 1, 0, *(s32 *)(mb + 0xF4), 0);
    } else if (sub == 5) {
        RequestGameStateChange(2, 1, *(s32 *)(mb + 0xF4), 0, 0);
    } else {
        PopGameState(0, 0);
    }

    if (*(s32 *)(mb + 0x40) != 0 &&
        *(s32 *)(mb + 0x30) != *(s32 *)(mb + 0x40)) {
        u8 *weapon;
        func_002B8898(1);
        weapon = g_weaponTable + g_itemEquippedSlot[*(s32 *)(mb + 0x40)] * 0xE0;
        func_00294D30(*(s32 *)(weapon + 0x14));
        func_002B8898(1);
    }
    func_002B8898(1);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", BuildPausePromptPopup);

/* Level-select slot-index validity gate (idx < 0x15 or idx == 0x18). */
s32 IsLevelListEntryEnabled(s32 idx) {
    return (idx < 0x15 || idx == 0x18);
}

/* LevelSelectListHandleInput: per-frame input for the level-select scroller (rooted at
 * g_nLevelSelectListCount). Refreshes each entry's enabled flag (+0xC array) from
 * g_abLevelAvailableFlags for entries IsLevelListEntryEnabled reports, then resets +0xC.
 * Held-button `flags`: up (0x1000)/down (0x4000) move the selection (ListScrollerSelect
 * Prev/Next); confirm (0x40) requests the exit to the selected destination via
 * RequestLevelExit(sel, 1) — except the special label 0xB47 with D_1A7C89 clear, which
 * exits to 0x19. Returns the selected index on confirm, else -1. Matching arm stays
 * INCLUDE_ASM (8-byte-packed-save wall); #else is the structure model. Word-verified vs USA
 * LevelSelectListHandleInput: only D_1A7C09->D_1A7C89 retargets (rest keep names). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", LevelSelectListHandleInput);
#else
s32 LevelSelectListHandleInput(s32 flags) {
    extern s32  g_nLevelSelectListCount;
    extern u8   g_abLevelAvailableFlags[];
    extern u8   g_levelSelectEntries[];
    extern u8   D_1A7C89;                        /* USA D_1A7C09 */
    extern void RequestLevelExit(s32 destination, s32 commitSave);
    s32 *scroller = &g_nLevelSelectListCount;
    s32 result = -1;
    s32 i;

    if (g_nLevelSelectListCount >= 0) {
        for (i = 0; i <= g_nLevelSelectListCount; i++) {
            if (IsLevelListEntryEnabled(i)) {
                *(s32 *)((char *)scroller + 0xC + i * 4) =
                    (g_abLevelAvailableFlags[i] != 0);
            }
        }
    }
    *(s32 *)((char *)scroller + 0xC) = 0;

    if (flags & 0x1000) {           /* up */
        ListScrollerSelectPrev(scroller);
    } else if (flags & 0x4000) {    /* down */
        ListScrollerSelectNext(scroller);
    } else if (flags & 0x40) {      /* confirm */
        s32 sel = *(s32 *)((char *)scroller + 0x4);
        result = sel;
        if (*(s32 *)&g_levelSelectEntries[sel * 8] == 0xB47 && D_1A7C89 == 0) {
            RequestLevelExit(0x19, 1);
        } else {
            RequestLevelExit(sel, 1);
        }
    }
    return result;
}
#endif

/* LevelSelectListRender: draws the galactic-map level-select list. Dims the screen
 * (DrawFullScreenTint), then for each enabled entry (scroller flag at +0xC+i*4 != 0) of
 * the g_nLevelSelectListCount-rooted scroller, formats "<name> <detail>" into a local
 * buffer (func_00115DA8 sprintf, format D_1AB990, from localized strings entry+0x4 /
 * entry+0x0), draws a row background bar (func_0027F070) and the row text (func_00280050)
 * at y = D_1AB988 + row*D_1AB984 where row = 2*i (or 0x24 for the special last row i==0x18);
 * row color is 0x80FFDE8D on the selected row (scroller +0x4) else 0x80808080. Matching arm
 * stays INCLUDE_ASM (8-byte-packed 9-GPR save); #else is the structure model. Word-verified
 * vs USA LevelSelectListRender: DrawFullScreenTint->func_0027E2A8, func_0027F208->func_0027F070,
 * func_002801B8->func_00280050; D_1AB914->D_1AB984(gp), D_1AB918->D_1AB988(gp), D_1AB920->
 * D_1AB990 (+0x70). Bar color 0x442D00 per EU (and USA) asm — the USA #else's 0x60442D00 is
 * a model-only typo. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", LevelSelectListRender);
#else
s32 LevelSelectListRender(void) {
    extern s32  g_nLevelSelectListCount;
    extern void func_0027E2A8(s32 r, s32 g, s32 b, s32 a);              /* DrawFullScreenTint */
    extern void func_00280050(s32 x, s32 y, u64 color, char *str, s64 sel); /* func_002801B8 */
    extern void func_0027F070(s32 y0, s32 y1, s32 x0, s32 x1, s32 h, s32 color); /* func_0027F208 */
    extern void func_00115DA8(char *dst, const char *fmt, ...);        /* SDK sprintf */
    extern s32  D_1AB984;             /* row pitch  (USA D_1AB914) */
    extern s32  D_1AB988;             /* base row y (USA D_1AB918) */
    extern char D_1AB990[];           /* "<name> <detail>" format (USA D_1AB920) */
    char buf[0x100];
    s32 *scroller = &g_nLevelSelectListCount;
    s32 i;

    func_0027E2A8(0, 0, 0, 0x60);
    for (i = 0; i <= g_nLevelSelectListCount; i++) {
        s32 *entry;
        s32 flag = *(s32 *)((char *)scroller + 0xC + i * 4);
        s32 row, y;
        char *name, *detail;
        u32 color;
        if (flag == 0) {
            continue;
        }
        entry = (s32 *)((char *)scroller[2] + i * 8);   /* scroller[0x8] = entry array */
        row = (i != 0x18) ? i * 2 : 0x24;
        name = GetLocalizedString(entry[1]);            /* entry+0x4 */
        detail = GetLocalizedString(entry[0]);          /* entry+0x0 */
        func_00115DA8(buf, D_1AB990, name, detail);
        y = D_1AB988 + row * D_1AB984;
        func_0027F070(y, y + 0xF, 0x40, 0x1C0, 0x60, 0x442D00);
        color = (scroller[1] == i) ? 0x80FFDE8D : 0x80808080;
        func_00280050(0x100, y, color, buf, -1);
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC638);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC688);

/* func_002CC708: EU twin of USA func_002CC858 — dispatch one of seven map-cell config
 * descriptors (D_1AA758, stride 0x18, selected by `q` in 0..6) through func_002D6A48,
 * returning its result into a scratch buffer; out-of-range `q` returns 0. Selecting
 * descriptor 0 also snapshots the sound-bank handle (blk+0x1248 -> +0x22C8). Matching arm
 * stays INCLUDE_ASM (cc1 jump-table layout not reproduced); #else is the structure model.
 * Word-verified vs USA func_002CC858: D_1AA6D8->D_1AA758(gp, +0x80), func_002D6AD8->
 * func_002D6A48, g_soundBankHandlesBlk->g_sndChannelVolumes+0x1778. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC708);
#else
s32 func_002CC708(s32 q) {
    extern s32 func_002D6A48(void *config, void *buf);  /* USA func_002D6AD8 */
    extern u8  D_1AA758[];                               /* USA D_1AA6D8 (gp-rel) */
    extern u8  g_sndChannelVolumes[];                    /* USA g_soundBankHandlesBlk base @ +0x1778 */
    s32 buf[4];

    buf[0] = 0;
    if ((u32)q >= 7) {
        return 0;
    }
    if (q == 0) {
        u8 *blk = g_sndChannelVolumes + 0x1778;
        *(s32 *)(blk + 0x22C8) = *(s32 *)(blk + 0x1248);
    }
    return func_002D6A48(&D_1AA758[q * 0x18], buf);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC7B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CC8C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", TickActiveMenuScreen);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", RenderMenuScreenWidgets);

/* func_002CD300: EU twin of USA func_002CD450 — menu screen-state query. Only acts when
 * the active screen (ss[0x14]->[0xE8]) is the one passed in. Returns a tri-state confirm/
 * cancel code driven by the global input flags (D_138200[0x1C4]) and the screen's pending-
 * result fields. Matching arm stays INCLUDE_ASM (near-miss: SN cc1 keeps the second
 * D_138200[0x1C4] reload+andi under branch-likely, our cc1 CSEs it); #else is the structure
 * model. Word-verified vs USA func_002CD450: g_particleFxBlob+0x100 -> D_001F0000+0x2840;
 * D_138180 -> D_138200 (+0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CD300);
#else
s32 func_002CD300(s32 screen) {
    extern u8 D_138200[];               /* USA D_138180 */
    u8 *ss = D_001F0000 + 0x2840;
    s32 flags;
    s32 v;
    if (*(s32 *)(*(u8 **)(ss + 0x14) + 0xE8) != screen) {
        return 0;
    }
    flags = *(s32 *)(D_138200 + 0x1C4);
    if (flags & 0x900) {
        if (*(s32 *)(ss + 0x134) == 0) {
            return 1;
        }
        flags = *(s32 *)(D_138200 + 0x1C4);
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

/* func_002CD398: EU twin of USA func_002CD4E8 — stores (selector ? &D_1AB6B8 : &D_1AB6E8)
 * into list[0x34], returns 0. Matching arm stays INCLUDE_ASM (anomalous +0x60 frame prologue
 * with no matching restore not reproduced from clean C); #else is the structure model.
 * Word-verified vs USA func_002CD4E8: selector D_1A7318 -> D_001A7308+0x90 (lw); D_1AB648->
 * D_1AB6B8, D_1AB678->D_1AB6E8 (+0x70, gp-rel). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CD398);
#else
s32 func_002CD398(s32 *list) {
    extern u8 D_001A7308[];              /* selector word @ +0x90 (USA D_1A7318) */
    extern u8 D_1AB6B8[], D_1AB6E8[];    /* USA D_1AB648 / D_1AB678 (gp-rel) */
    list[0xD] = (s32)(*(s32 *)(D_001A7308 + 0x90) ? D_1AB6B8 : D_1AB6E8);
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", BuildCheatMenuItemList);

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

/* func_002CDE98: EU twin of USA func_002CDEB0 — cheat-flag mirror. For each source toggle
 * byte (D_1A7C5n) write 3 ("on") or 0 ("off") into the matching cheat-menu item-state
 * halfword (D_1AA6xx). Leaf, pure data shuffle. Matching arm stays INCLUDE_ASM (near-miss:
 * SN cc1 fills each beqz delay slot with the ternary's `move rd,zero`, our cc1 won't); #else
 * is the structure model. Word-verified vs USA func_002CDEB0: source bytes D_1A7BD1/2/3/4/6->
 * D_1A7C51/52/53/54/56 and dest halfwords D_1AA5A2/5BA/5D2/5EA/602->D_1AA622/63A/652/66A/682
 * (all +0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CDE98);
#else
void func_002CDE98(void) {
    extern u8  D_1A7C51, D_1A7C52, D_1A7C53, D_1A7C54, D_1A7C56;
    extern s16 D_1AA622, D_1AA63A, D_1AA652, D_1AA66A, D_1AA682;
    D_1AA622 = D_1A7C51 ? 3 : 0;
    D_1AA63A = D_1A7C52 ? 3 : 0;
    D_1AA652 = D_1A7C53 ? 3 : 0;
    D_1AA66A = D_1A7C54 ? 3 : 0;
    D_1AA682 = D_1A7C56 ? 3 : 0;
}
#endif

/* Level-exit confirm dispatch for the active menu screen (EU twin of USA func_002CDF48):
 * validates the pending pick against the global input flags + per-screen widget tables
 * and arms g_nLevelExitRequested/g_nLevelExitDestination. Only acts when `item` is the
 * screen's active widget (screen+0xE8); the widget's table entry (item+0x34[item+0x40],
 * stride 0xC, +0x2) must mark it a "level exit" (kind 3); maps the six menu-item records
 * to a destination code gated by its enabled byte (D_1A7C51..56); rolls the destination
 * back to its saved value when the byte is clear. Anything not handled forwards to
 * func_002D6A98. Matching arm stays INCLUDE_ASM (later cc1 lays out the chained pointer
 * compares differently); #else is the structure model. Word-verified vs USA func_002CDF48:
 * func_002D6B28->func_002D6A98; D_138180->D_138200 (+0x80); item widget records
 * D_00259128/178/1C8/218/268/2B8->D_00259148/198/1E8/238/288/2D8 (+0x20); enabled bytes
 * D_1A7BD1/2/3/4/6->D_1A7C51/52/53/54/56 (+0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CDF30);
#else
s32 func_002CDF30(void *item) {
    extern u8 *g_pCurrentMenuScreen;
    extern s32 g_nLevelExitRequested;
    extern u8  g_nLevelExitDestination[];
    extern s32 func_002D6A98(void *item);
    extern u8  D_138200[];
    extern u8  D_00259148[], D_00259198[], D_002591E8[], D_00259238[],
               D_00259288[], D_002592D8[];
    extern u8  D_1A7C51, D_1A7C52, D_1A7C53, D_1A7C54, D_1A7C56;
    u8 *screen = g_pCurrentMenuScreen;
    s32 savedDest;
    u8 *table;
    s32 dest;
    u8 enabled;

    if (*(void **)(screen + 0xE8) != item)
        return func_002D6A98(item);

    if ((*(s32 *)(D_138200 + 0x1C4) & ~0x40) != 0 && item == (void *)D_00259148)
        return 0;
    if (g_nLevelExitRequested != 0)
        return func_002D6A98(item);

    savedDest = *(s32 *)g_nLevelExitDestination;

    table = *(u8 **)((u8 *)item + 0x34);
    if (*(s16 *)(table + *(s32 *)((u8 *)item + 0x40) * 0xC + 0x2) != 3)
        return func_002D6A98(item);

    if (item == (void *)D_00259148) {
        *(s32 *)g_nLevelExitDestination = 1;
        g_nLevelExitRequested = 1;
        return 0;
    }
    if (!(*(s32 *)(D_138200 + 0x1C4) & 0x40))
        return func_002D6A98(item);

    if (item == (void *)D_00259198)      { dest = 1; enabled = D_1A7C51; }
    else if (item == (void *)D_002591E8) { dest = 2; enabled = D_1A7C52; }
    else if (item == (void *)D_00259238) { dest = 3; enabled = D_1A7C53; }
    else if (item == (void *)D_00259288) { dest = 4; enabled = D_1A7C54; }
    else if (item == (void *)D_002592D8) { dest = 6; enabled = D_1A7C56; }
    else {
        if (g_nLevelExitRequested == 0)
            *(s32 *)g_nLevelExitDestination = savedDest;
        return 0;
    }

    *(s32 *)g_nLevelExitDestination = dest;
    g_nLevelExitRequested = (enabled != 0);
    if (g_nLevelExitRequested == 0)
        *(s32 *)g_nLevelExitDestination = savedDest;
    return 0;
}
#endif

/* Galactic-map / level-select input dispatcher (EU twin of USA func_002CE0C8). Reads
 * g_padButtonsPressed: any nav bit (mask 0x910) swallows the key (returns 1); confirm bit
 * (0x40) refreshes the panel (func_0029CB00), captures its return as the SELECTION, then
 * routes to the confirm handler chosen by the func_0026F638..660 map-cell queries; else
 * just refreshes. On an accepted handler (result != 0) plays the confirm SFX (0x12) via
 * func_002E6C28. Matching arm stays INCLUDE_ASM (64-bit daddu selection-capture idiom +
 * 1-GPR packed-save frame not reproduced by cc1); #else is the structure model.
 * Word-verified vs USA func_002CE0C8 (query-func delta is NON-uniform — read from the .s
 * jal order): panel func_0029CFA0->func_0029CB00; queries func_0026F7D0/7D8->638/640,
 * func_0026F7F0->650, func_0026F7E8->648, func_0026F800->660, func_0026F7F8->658; handlers
 * func_002CC908->func_002CC7B8, func_002CC7D8->func_002CC688, func_002CCA18->func_002CC8C8,
 * func_002CC788->func_002CC638, func_002CC858->func_002CC708; PlayGlobalSound->func_002E6C28. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE0B0);
#else
s32 func_002CE0B0(void) {
    extern s32 g_padButtonsPressed;
    extern s32 func_0029CB00(s32 buttons);
    extern s32 func_0026F638(void);
    extern s32 func_0026F640(void);
    extern s32 func_0026F648(void);
    extern s32 func_0026F650(void);
    extern s32 func_0026F658(void);
    extern s32 func_0026F660(void);
    extern s32 func_002CC638(s32 q);
    extern s32 func_002CC688(s32 q);
    extern s32 func_002CC708(s32 q);
    extern s32 func_002CC7B8(s32 q);
    extern s32 func_002CC8C8(s32 q);
    extern void func_002E6C28(s32 id, s32 a, s32 b);
    s32 buttons = g_padButtonsPressed;
    s32 result = 0;

    if (buttons & 0x910) {
        result = 1;
    } else if (buttons & 0x40) {
        s32 sel = func_0029CB00(buttons);
        if (func_0026F638() != 0 && func_0026F640() != 0) {
            result = func_002CC7B8(sel);
        } else if (func_0026F650() != 0) {
            result = func_002CC688(sel);
        } else if (func_0026F648() != 0) {
            result = func_002CC8C8(sel);
        } else if (func_0026F660() == 0x31 || func_0026F658() != 0 ||
                   func_0026F660() == 0x1C) {
            result = func_002CC638(sel);
        } else {
            result = func_002CC708(sel);
        }
    } else {
        func_0029CB00(buttons);
    }

    if (result != 0) {
        func_002E6C28(0x12, 0, 0);
    }
    return result;
}
#endif

/* Draw-batch wrapper: render one menu sub-element inside a 2D batch. */
s32 func_002CE1E8(void) {
    func_0027CA28(0);
    func_0029CAD0();
    func_0027CB48();
    return 0;
}

/* Present-record fence variant driving the front-end "leave" navigation (EU twin of USA
 * func_002CE230). Samples the frame timestamp (func_00338C48); on confirm (0x10) points
 * g_pNextMenuScreen at D_00259328 and, if the GUI widget handle (g_guiInstance+0x38000
 * .+0x7A9C) is live, hands it to func_00350D08; on cancel (0x900) does the same hand-off
 * and returns 1; else ticks the idle handler func_0029CBA0. Then, with the GUI up, runs the
 * shared present-record redraw fence (record D_00259E70 / live *D_259E54): clears the
 * "needs redraw" bit 0x4 when the two timestamps agree, the file-load is idle
 * (g_saveImageArea+0x1004 == 0) and the record shows this frame already presented; else sets
 * it. Stamps rec[0x58]=now. Matching arm stays INCLUDE_ASM (later cc1 packs 8-byte save
 * slots vs our 16); #else is the structure model. Word-verified vs USA func_002CE230:
 * func_00337D98->func_00338C48; D_00259308->D_00259328 (+0x20); func_0034F868->func_00350D08;
 * func_0029D040->func_0029CBA0; present record D_00259E50->D_00259E70 (+0x20); live
 * D_259E34->D_259E54 (+0x20); GUI field 0x79EC->0x7A9C (+0xB0); g_fileLoadState->
 * g_saveImageArea+0x1004 (s16). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE218);
#else
s32 func_002CE218(void) {
    extern s32  func_00338C48(void);
    extern void func_00350D08(s32 handle);
    extern void func_0029CBA0(s32 buttons);
    extern s32  g_padButtonsPressed;
    extern u8  *g_pNextMenuScreen;
    extern u8   D_00259328[];
    extern u8   D_00259E70[];
    extern s32  D_259E54;
    extern u8   g_saveImageArea[];
    s32 t0 = func_00338C48();
    s32 flags = g_padButtonsPressed;
    s32 result = 0;

    if (flags & 0x10) {
        g_pNextMenuScreen = D_00259328;
        if (g_guiInstance == NULL)
            return result;
        if (*(s32 *)(g_guiInstance + 0x38000 + 0x7A9C) != 0)
            func_00350D08(*(s32 *)(g_guiInstance + 0x38000 + 0x7A9C));
    } else if (flags & 0x900) {
        if (g_guiInstance != NULL &&
            *(s32 *)(g_guiInstance + 0x38000 + 0x7A9C) != 0)
            func_00350D08(*(s32 *)(g_guiInstance + 0x38000 + 0x7A9C));
        result = 1;
    } else {
        func_0029CBA0(flags);
    }

    if (g_guiInstance) {
        s32 now = func_00338C48();
        u8 *rec = D_00259E70;
        s32 *live = (s32 *)D_259E54;
        s32 clear = 0;
        if (now == t0 && *(s16 *)(g_saveImageArea + 0x1004) == 0) {
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

/* Insomniac-museum (or sibling extras) screen draw (EU twin of USA func_002CE3A0): inside a
 * 2D batch runs the per-screen overlay (func_0029CB40) and draws the localized title string
 * (PAL id 0xB60) at (D_1ABA40, D_1ABA44) in 0x80F0F0F0; then, if the GUI is up and its museum
 * widget handle (g_guiInstance+0x38000 .+0x7A9C) is non-null, renders that moby model
 * (BeginMobyDrawSegment begin .. FinishMobyRenderChain finish + the func_00350Dxx/Exx/F98 model-setup chain),
 * waits one DMA fence (WaitFrameDmaFence(0x10)) and patches the moby packet's TEX0 (PatchMobyPacketTex0).
 * Matching arm stays INCLUDE_ASM (beql branch-likely null guard + 64-bit daddu handle-copy not
 * reproduced by cc1); #else is the structure model. Word-verified vs USA func_002CE3A0: overlay
 * func_0029CFE0->func_0029CB40; string id 0x2BE5->0xB60 (PAL localized-table index, genuine
 * region diff); label pos D_1AB9D8/DC->D_1ABA40/44 (+0x68); text draw func_002801B8->func_0027FF28;
 * GUI field 0x79EC->0x7A9C (+0xB0); moby chain func_002A1000/1028/1058->func_002A0B88/0BB0/0BE0,
 * BeginMobyDrawSegment->BeginMobyDrawSegment, FinishMobyRenderChain->FinishMobyRenderChain, func_0034F928/9B8/9F8/
 * AF8->func_00350DC8/E58/E98/F98, WaitFrameDmaFence->WaitFrameDmaFence, PatchMobyPacketTex0->PatchMobyPacketTex0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE388);
#else
s32 func_002CE388(void) {
    extern void func_0029CB40(void);
    extern s32  D_1ABA40;
    extern s32  D_1ABA44;
    extern void BeginMobyDrawSegment(void);
    extern void func_002A0B88(void);
    extern void func_002A0BB0(void);
    extern void func_002A0BE0(void);
    extern void FinishMobyRenderChain(void);
    extern void PatchMobyPacketTex0(void);
    extern void WaitFrameDmaFence(s32 mask);
    extern void func_00350DC8(s32 handle);
    extern void func_00350E58(s32 handle);
    extern void func_00350E98(s32 handle);
    extern void func_00350F98(s32 handle, s32 arg);
    s32 handle;

    func_0027CA28(0);
    func_0029CB40();
    func_0027FF28(D_1ABA40, D_1ABA44, 0x80F0F0F0, GetLocalizedString(0xB60), -1);
    func_0027CB48();

    if (g_guiInstance != NULL &&
        *(s32 *)(g_guiInstance + 0x38000 + 0x7A9C) != 0) {
        BeginMobyDrawSegment();
        func_002A0B88();
        func_002A0BB0();
        handle = *(s32 *)(g_guiInstance + 0x38000 + 0x7A9C);
        func_00350DC8(handle);
        func_00350E58(handle);
        func_00350E98(handle);
        func_00350F98(handle, handle + 0xC00);
        func_002A0BE0();
        FinishMobyRenderChain();
        WaitFrameDmaFence(0x10);
        PatchMobyPacketTex0();
    }
    return 0;
}
#endif

/* Per-screen menu tick + present-record fence latch (EU twin of USA func_002CE498). Confirm
 * (0x10) latches the active screen's pending result (block[0x14]->0xE0 into block[0x18], else
 * -1/0); cancel (0x900) returns 1; otherwise ticks the idle handler func_0029CBE0(buttons,
 * &scratch). Then, with the GUI up, samples the frame timestamp twice (func_00338C48); clears
 * the "needs redraw" bit 0x4 of the live object (*D_259C44)[0x10] when the two reads agree, the
 * file-load is idle (g_saveImageArea+0x1004 == 0) and the present record (D_00259C78) shows this
 * frame already presented; else sets it; stamps rec[0x58]=now. Matching arm stays INCLUDE_ASM
 * (3-GPR packed-save frame + branch-likely fence shape); #else is the structure model.
 * Word-verified vs USA func_002CE498: func_00337D98->func_00338C48; g_menuScreenBlock->
 * D_001F0000+0x2840; idle func_0029D080->func_0029CBE0; present record D_00259C58->D_00259C78
 * (+0x20); live D_259C24->D_259C44 (+0x20); g_fileLoadState->g_saveImageArea+0x1004 (s16). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE480);
#else
s32 func_002CE480(void) {
    extern s32  func_00338C48(void);
    extern s32  func_0029CBE0(s32 buttons, s32 *out);
    extern s32  g_padButtonsPressed;
    extern u8   g_saveImageArea[];
    extern u8   D_00259C78[];
    extern s32  D_259C44;
    s32 t0 = func_00338C48();
    s32 flags = g_padButtonsPressed;
    s32 result = 0;
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)(D_001F0000 + 0x2840);
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
        func_0029CBE0(flags, (s32 *)scratch);
    }
    if (g_guiInstance) {
        s32 now = func_00338C48();
        u8 *rec = D_00259C78;
        s32 *live = (s32 *)D_259C44;
        s32 clear = 0;
        if (now == t0 && *(s16 *)(g_saveImageArea + 0x1004) == 0) {
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

/* Confirm/cancel poll variant (EU twin of USA func_002CE618): confirm (0x10) acknowledges
 * input (func_0028C730) then returns the active screen's pending result (latched into
 * block[0x18]) or -1 when the screen has no pending sub-result; back/cancel (0x900)
 * acknowledges + returns 1; otherwise ticks the idle handler func_0029D578 and returns 0.
 * Matching arm stays INCLUDE_ASM (2-GPR packed-save frame + branch-likely confirm shape);
 * #else is the structure model. Word-verified vs USA func_002CE618: func_0028C7A8->func_0028C730;
 * func_0029DA18->func_0029D578; g_menuScreenBlock->D_001F0000+0x2840. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE600);
#else
s32 func_002CE600(void) {
    extern void func_0028C730(void);
    extern void func_0029D578(s32 padPressed);
    extern s32  g_padButtonsPressed;
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)(D_001F0000 + 0x2840);
    if (flags & 0x10) {
        s32 v;
        func_0028C730();
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
        func_0028C730();
        return 1;
    }
    func_0029D578(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002CE690(void) {
    func_0027CA28(0);
    func_0029D5B8();
    func_0027CB48();
    return 0;
}

/* Confirm/cancel poll + present-record fence (EU twin of USA func_002CE6D8, record D_0025E058 /
 * live *D_25E024). Samples the frame timestamp (func_00338C48); confirm (0x10) latches the
 * active screen's pending result (block[0x14]->0xE0 into block[0x18], else -1/0); cancel (0x900)
 * returns 1; else ticks idle handler func_0029CE18. With the GUI up, clears the "needs redraw"
 * bit 0x4 when the two timestamps agree, the file-load is idle (g_saveImageArea+0x1004 == 0) and
 * the record shows this frame already presented (compared against t0); else sets it. When the
 * timestamp advanced during the tick it latches D_25E204 = -0x22C, then re-samples to stamp
 * rec[0x58]. Matching arm stays INCLUDE_ASM (later cc1 packs 8-byte save slots vs our 16); #else
 * is the structure model. Word-verified vs USA func_002CE6D8: func_00337D98->func_00338C48;
 * g_menuScreenBlock->D_001F0000+0x2840; idle func_0029D2B8->func_0029CE18; present record
 * D_0025E298->D_0025E058, live D_25E264->D_25E024, latch D_25E444->D_25E204 (ALL -0x240, the
 * 0x25Exxx region delta, NOT +0x20); g_fileLoadState->g_saveImageArea+0x1004 (s16). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE6C0);
#else
s32 func_002CE6C0(void) {
    extern s32  func_00338C48(void);
    extern void func_0029CE18(s32 buttons);
    extern s32  g_padButtonsPressed;
    extern u8   g_saveImageArea[];
    extern u8   D_0025E058[];
    extern s32  D_25E024;
    extern s32  D_25E204;
    s32 t0 = func_00338C48();
    s32 flags = g_padButtonsPressed;
    s32 result = 0;

    if (flags & 0x10) {
        s32 *block = (s32 *)(D_001F0000 + 0x2840);
        s32 v = *(s32 *)(*(u8 **)((u8 *)block + 0x14) + 0xE0);
        if (v != 0) {
            block[0x18 / 4] = v;
            result = 0;
        } else {
            result = (block[0x134 / 4] == 0) ? -1 : 0;
        }
    } else if (flags & 0x900) {
        result = 1;
    } else {
        func_0029CE18(flags);
    }

    if (g_guiInstance) {
        s32 now = func_00338C48();
        u8 *rec = D_0025E058;
        s32 *live = (s32 *)D_25E024;
        s32 clear = 0;
        if (now == t0 && *(s16 *)(g_saveImageArea + 0x1004) == 0) {
            if (*(s32 *)(rec + 0x50) == t0 && *(s32 *)(rec + 0x44) == 2) {
                clear = 1;
            } else if (*(s32 *)(rec + 0x54) == t0 &&
                       *(s32 *)(rec + 0x44) == 4) {
                clear = 1;
            }
        }
        if (clear) {
            live[0x10 / 4] &= ~0x4;
        } else {
            live[0x10 / 4] |= 0x4;
        }
        if (now != t0) {
            D_25E204 = -0x22C;
        }
        *(s32 *)(rec + 0x58) = func_00338C48();
    }
    return result;
}
#endif

/* GUI wrapper (EU twin of USA func_002CE830): when the GUI is up, register a widget (instance
 * + 0x3A0B0) and stash the returned handle in widget[0x34] (out[0xD]). Matching arm stays
 * INCLUDE_ASM (2-GPR packed-save frame); #else is the structure model. Word-verified vs USA
 * func_002CE830: func_00345298->func_00346428; GUI field offset 0x3A000->0x3A0B0 (+0xB0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE818);
#else
s32 func_002CE818(s32 *out) {
    extern s32 func_00346428(void *widget);
    if (g_guiInstance) {
        out[0xD] = func_00346428(g_guiInstance + 0x3A0B0);
    }
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002CE860(void) {
    func_0027CA28(0);
    func_0029CDE8();
    func_0027CB48();
    return 0;
}

/* GUI wrapper (EU twin of USA func_002CE8A8): when the GUI is up, mark the list widget at
 * g_guiInstance+0x3C210 active and stash its registered handle in out[0x34]. Returns 0. Matching arm
 * stays INCLUDE_ASM (3-GPR packed-save frame); #else is the structure model. Word-verified vs USA
 * func_002CE8A8: func_00342460->func_003435D0, func_00342468->func_003435D8; GUI offset 0x3C160->
 * 0x3C210 (+0xB0); g_guiInstance kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE890);
#else
s32 func_002CE890(s32 *out) {
    extern void func_003435D0(void *widget, s32 arg);  /* USA func_00342460 */
    extern s32  func_003435D8(void *widget);           /* USA func_00342468 */
    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C210;
        func_003435D0(w, 1);
        out[0x34 / 4] = func_003435D8(w);
    }
    return 0;
}
#endif

/* Menu confirm/cancel poll + command builder (EU twin of USA func_002CE908): confirm (0x10) latches
 * the active screen's pending result; cancel (0x900) returns 1; else ticks idle handler func_0029CE88
 * and, on the confirm pad bit (0x40) with the GUI up, reads the selected entry of the list widget at
 * g_guiInstance+0x3C210 (func_00343638) and builds an 8-byte command record (op=entry[0x8],
 * arg=entry[0xC]) handed to func_002D6A70. Matching arm stays INCLUDE_ASM (2-GPR packed-save +
 * branch-likely confirm shape); #else is the structure model. Word-verified vs USA func_002CE908:
 * D_138180->D_138200; g_menuScreenBlock->D_001F0000+0x2840; idle func_0029D328->func_0029CE88;
 * func_003424C8->func_00343638; func_002D6B00->func_002D6A70; GUI offset 0x3C160->0x3C210 (+0xB0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CE8F0);
#else
s32 func_002CE8F0(void) {
    extern u8    D_138200[];                       /* USA D_138180 */
    extern s32   func_0029CE88(s32 padPressed);    /* USA func_0029D328 */
    extern void *func_00343638(void *widget);      /* USA func_003424C8 */
    extern void  func_002D6A70(void *record);      /* USA func_002D6B00 */
    s32 flags = *(s32 *)(D_138200 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029CE88(flags);
    if ((*(s32 *)(D_138200 + 0x1C4) & 0x40) && g_guiInstance) {
        u8 record[0x30];
        u8 *entry = (u8 *)func_00343638(g_guiInstance + 0x3C210);
        *(u16 *)(record + 0x2) = *(u16 *)(entry + 0x8);
        *(s32 *)(record + 0x4) = *(s32 *)(entry + 0xC);
        func_002D6A70(record);
    }
    return 0;
}
#endif

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

/* GUI wrapper: configure the vendor/extras list widget (instance+0x3C210) — mark it active
 * (arg 1), bind its three data blobs, then clear its selection (arg 0). Matching arm stays
 * INCLUDE_ASM (2-GPR packed-save frame); #else is the structure model. Word-verified vs USA
 * func_002CEA38: widget offset 0x3C160->0x3C210 (EU +0xB0 GUI-manager shift); func_00342460->
 * func_003435D0, func_00342450->func_003435C0, func_00342520->func_00343690; blobs
 * D_2615D8/D_261678/D_261730->D_261398/D_261438/D_2614D8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002CEA20);
#else
s32 func_002CEA20(void) {
    extern void func_003435D0(void *widget, s32 arg);                    /* USA func_00342460 */
    extern void func_003435C0(void *widget, void *a, void *b, void *c);  /* USA func_00342450 */
    extern void func_00343690(void *widget, s32 arg);                    /* USA func_00342520 */
    extern u8 D_261398, D_261438, D_2614D8;  /* USA D_2615D8/D_261678/D_261730 */
    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C210;
        func_003435D0(w, 1);
        func_003435C0(w, &D_261398, &D_261438, &D_2614D8);
        func_00343690(w, 0);
    }
    return 0;
}
#endif

/* UpdateBestiaryMenuInput (EU twin of USA UpdateBestiaryMenuInput): per-frame input for the
 * bestiary browser (cursor D_1ABA50, 1..0x3F). Back (0x10) pops the menu-screen block. Exit
 * (0x900) returns 1. Left (0x8000)/right (0x2000) move the cursor to the precomputed prev/next
 * entry (clamped 1/0x3F) with the move-or-deny sound; rescans the kill-count table for the
 * nearest defeated neighbours + updates the panel; pokes D_0025AC28+0x3C by -0x240 when the
 * cursor moved. Matching arm stays INCLUDE_ASM (8-byte-packed-save wall); #else is the structure
 * model. Word-verified vs USA UpdateBestiaryMenuInput: g_menuScreenBlock->D_001F0000+0x2840;
 * cursor/prev/next->D_1ABA50/54/58 (gp); killcounts->D_00180000+0x8590; entry table->D_2643B0;
 * D_0025ABA0->D_0025ABC0, D_0025AC08->D_0025AC28; PlayGlobalSound->func_002E6C28. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", UpdateBestiaryMenuInput);
#else
s32 UpdateBestiaryMenuInput(void) {
    extern s32  g_padButtonsPressed;
    extern s32  D_1ABA54;                /* USA g_bestiaryPrevEntry (gp) */
    extern s32  D_1ABA58;                /* USA g_bestiaryNextEntry (gp) */
    extern u8   D_00180000[];            /* USA g_bestiaryKillCounts @ +0x8590 */
    extern u8   D_2643B0[];              /* USA g_bestiaryEntryTable */
    extern u8   D_0025ABC0[];            /* USA D_0025ABA0 */
    extern u8   D_0025AC28[];            /* USA D_0025AC08 */
    extern void func_002E6C28(s32 id, s32 a, s32 b);  /* USA PlayGlobalSound */
    u8 *mb = D_001F0000 + 0x2840;
    u8 *kills = D_00180000 + 0x8590;
    s32 buttons = g_padButtonsPressed;
    s32 cursor0 = D_1ABA50;
    s32 ret = 0;
    s32 cursor, i;

    if (buttons & 0x10) {           /* back */
        s32 e0 = *(s32 *)(*(u8 **)(mb + 0x14) + 0xE0);
        if (e0 != 0) {
            *(s32 *)(mb + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(mb + 0x134) == 0) ? -1 : 0;
    }

    if (buttons & 0x900) {          /* exit */
        ret = 1;
    } else if (buttons & 0x8000) {  /* left: move to the previous entry */
        s32 prev = D_1ABA54;
        func_002E6C28((cursor0 == prev) ? 5 : 3, 0, 0);
        D_1ABA50 = (prev > 0) ? prev : 1;
    } else if (buttons & 0x2000) {  /* right: move to the next entry */
        s32 next = D_1ABA58;
        func_002E6C28((cursor0 == next) ? 5 : 3, 0, 0);
        D_1ABA50 = (next < 0x40) ? next : 0x3F;
    }

    cursor = D_1ABA50;
    D_1ABA54 = cursor - 1;
    D_1ABA58 = cursor + 1;

    i = cursor - 1;
    for (;;) {
        if (*(u16 *)(kills + i * 4) != 0 ||
            *(u16 *)(kills + i * 4 + 2) != 0) {
            break;
        }
        i--;
        if (i <= 0) {
            i = 1;
            break;
        }
    }
    D_1ABA54 = i;

    i = cursor + 1;
    for (;;) {
        if (*(u16 *)(kills + i * 4) != 0 ||
            *(u16 *)(kills + i * 4 + 2) != 0) {
            break;
        }
        i++;
        if (i >= 0x40) {
            i = cursor;
            break;
        }
    }
    D_1ABA58 = i;

    if (*(u16 *)(kills + cursor * 4) != 0 ||
        *(u16 *)(kills + cursor * 4 + 2) != 0) {
        *(s32 *)(D_0025ABC0 + 0x58) = cursor - 1;
        *(s32 *)(D_0025AC28 + 0x34) =
            *(s16 *)(D_2643B0 + cursor * 0x18 + 0xA);
    } else {
        *(s32 *)(D_0025ABC0 + 0x58) = 0x3F;
        *(s32 *)(D_0025AC28 + 0x34) = 0;
    }
    if (D_1ABA50 != cursor0) {
        *(s32 *)(D_0025AC28 + 0x3C) = -0x240;
    }
    return ret;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawBestiaryEntry);

/* Reset the bestiary cursor to entry 1. */
s32 func_002CF530(void) {
    D_1ABA50 = 1;
    return 0;
}

/* return 0 stub. */
s32 func_002CF540(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawBestiaryPagingArrows);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawMenuPagingChrome);

/* DrawMenuItemSelectionBox (EU twin of USA DrawMenuItemSelectionBox): draw the four-sided
 * highlight box around a menu item, centred on screen. `width` sets the half-extent
 * (width/2 + 5 each side of screen centre); four func_002904C8 fills form the top (y 0x138..
 * 0x13A), bottom (0x14D..0x14F), left and right (0x139..0x14E) borders, all in `color`.
 * Matching arm stays INCLUDE_ASM (6-GPR packed-save wall); #else is the structure model.
 * Word-verified vs USA DrawMenuItemSelectionBox: g_screenWidth -> *(s32*)(D_001A7308 + 0xB8)
 * (EU reaches the screen-width word off D_001A7308); func_002904B0->func_002904C8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawMenuItemSelectionBox);
#else
void DrawMenuItemSelectionBox(s32 width, s32 color) {
    extern u8 D_001A7308[];   /* USA g_screenWidth, read at +0xB8 */
    extern void func_002904C8(s32 x0, s32 y0, s32 x1, s32 y1, s32 color, s32 flag); /* USA func_002904B0 */
    s32 half = width / 2 + 5;
    s32 cx = *(s32 *)(D_001A7308 + 0xB8) / 2;
    s32 right = cx + half;
    s32 left = cx - half;
    s32 x0 = left - 2;
    s32 x1 = right + 4;

    func_002904C8(x0, 0x138, x1, 0x13A, color, 0);        /* top */
    func_002904C8(x0, 0x14D, x1, 0x14F, color, 0);        /* bottom */
    func_002904C8(x0, 0x139, left, 0x14E, color, 0);      /* left */
    func_002904C8(right + 2, 0x139, x1, 0x14E, color, 0); /* right */
}
#endif

/* GUI wrapper (EU twin of USA func_002D0110): when the GUI is up, register a widget
 * (instance+0x3C210) and stash the returned handle in out[0x34]. Matching arm stays
 * INCLUDE_ASM (2-GPR packed-save frame); #else is the structure model. Word-verified vs USA
 * func_002D0110: widget offset 0x3C160->0x3C210 (EU +0xB0); func_00342468->func_003435D8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D0100);
#else
s32 func_002D0100(s32 *out) {
    extern s32 func_003435D8(void *widget);   /* USA func_00342468 */
    if (g_guiInstance) {
        out[0xD] = func_003435D8(g_guiInstance + 0x3C210);
    }
    return 0;
}
#endif

/* Menu confirm/cancel poll + command builder (EU twin of USA func_002D0158). Confirm (0x10)
 * latches the active screen's pending result (block[0x14]->0xE0 into block[0x18], else -1/0);
 * cancel (0x900) returns 1; otherwise ticks the idle handler func_0029CEF8 and, on the confirm
 * pad bit (0x40) with the GUI up, reads the selected entry of the list widget at
 * g_guiInstance+0x3C210 (func_00343638) and builds an 8-byte command record (op = (u16)entry
 * [0x8] at rec+0x2, arg = entry[0xC] at rec+0x4). When arg is 0 it plays UI sound 5, then hands
 * the record to func_002D6A70. Matching arm stays INCLUDE_ASM (2-GPR packed-save + branch-likely
 * confirm shape); #else is the structure model. Word-verified vs USA func_002D0158: D_138180->
 * D_138200; g_menuScreenBlock->D_001F0000+0x2840; widget 0x3C160->0x3C210; func_0029D398->
 * func_0029CEF8, func_003424C8->func_00343638, func_002D6B00->func_002D6A70, PlayGlobalSound->
 * func_002E6C28. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D0148);
#else
s32 func_002D0148(void) {
    extern u8   D_138200[];                       /* USA D_138180 */
    extern s32  func_0029CEF8(s32 padPressed);    /* USA func_0029D398 */
    extern void *func_00343638(void *widget);     /* USA func_003424C8 */
    extern void func_002D6A70(void *record);      /* USA func_002D6B00 */
    extern void func_002E6C28(s32 id, s32 a, s32 b); /* USA PlayGlobalSound */
    s32 flags = *(s32 *)(D_138200 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029CEF8(flags);
    if ((*(s32 *)(D_138200 + 0x1C4) & 0x40) && g_guiInstance) {
        u8 record[0x30];
        u8 *entry = (u8 *)func_00343638(g_guiInstance + 0x3C210);
        s32 arg = *(s32 *)(entry + 0xC);
        *(u16 *)(record + 0x2) = *(u16 *)(entry + 0x8);
        *(s32 *)(record + 0x4) = arg;
        if (arg == 0) {
            func_002E6C28(5, 0, 0);
        }
        func_002D6A70(record);
    }
    return 0;
}
#endif

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

/* Refreshes the weapon/gadget-vendor availability record (D_2614D8) and three global
 * availability flags from the current inventory (EU twin of USA func_002D02A0). Scans the 0x38
 * item slots (skipping empty g_weaponTable entries and slots 9/10): any unowned slot clears
 * "all owned" (D_1AA4D0); for owned slots any unseen weapon variant clears the "all variants
 * seen" flags (D_1AA4D4/D_1AA4D8). Then populates the D_2614D8 record with caption string-ids /
 * sub-page pointers for the base or extras set, lays out + resets the vendor widget
 * (g_guiInstance+0x3C210). Matching arm stays INCLUDE_ASM (packed-save + gp-rel scratch
 * scheduling); #else is the structure model. Word-verified vs USA func_002D02A0: D_1AA450/454/
 * 458->D_1AA4D0/4D4/4D8 (gp); g_itemStateFlags->D_1A7B08; D_0025C430/C5C8/CCD8->D_0025C450/C5E8/
 * CCF8; D_2615D8/D_261678/D_261730->D_261398/D_261438/D_2614D8; widget 0x3C160->0x3C210;
 * func_00342450->func_003435C0, func_00342520->func_00343690. GENUINE region diff (PAL string-
 * table ids): base-set 0x2C56->0xBD1; extras-set 0x3098->0x1114, 0x30A2->0x111E, 0x30D5->0x1408. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D0290);
#else
s32 func_002D0290(void) {
    extern s32  D_1AA4D0, D_1AA4D4, D_1AA4D8;   /* USA D_1AA450/454/458 (gp) */
    extern u8   g_itemEquippedSlot[];
    extern u8   g_weaponTable[];
    extern u8   g_inventoryOwned[];
    extern u8   D_1A7B08[];                      /* USA g_itemStateFlags */
    extern u8   g_miscExtras;
    extern u8   D_0025C450[], D_0025C5E8[], D_0025CCF8[]; /* USA D_0025C430/C5C8/CCD8 */
    extern u8   D_261398, D_261438, D_2614D8;    /* USA D_2615D8/D_261678/D_261730 */
    extern void func_003435C0(void *widget, void *a, void *b, void *c); /* USA func_00342450 */
    extern void func_00343690(void *widget, s32 arg);                   /* USA func_00342520 */
    u8 *record;
    s32 i;

    if (g_guiInstance == NULL)
        return 0;

    D_1AA4D0 = 1;
    D_1AA4D4 = 1;
    D_1AA4D8 = 1;

    for (i = 0; i < 0x38; i++) {
        s32 slot = g_itemEquippedSlot[i];
        u8 *weapon = g_weaponTable + slot * 0xE0;
        s32 seen4D4, seen4D8;
        s32 j;

        if (*(s32 *)weapon == 0 || i == 9 || i == 0xA)
            continue;

        if (g_inventoryOwned[i] == 0) {
            D_1AA4D0 = 0;
            D_1AA4D4 = 0;
            D_1AA4D8 = 0;
            continue;
        }

        seen4D4 = D_1AA4D4;
        seen4D8 = D_1AA4D8;
        for (j = 0; j < 3; j++) {
            if (*(s32 *)(weapon + 0x98 + j * 4) != 0 &&
                (D_1A7B08[i] & (1 << (j * 2))) == 0) {
                seen4D4 = 0;
                seen4D8 = 0;
            }
        }
        D_1AA4D8 = seen4D8;
        D_1AA4D4 = seen4D4;
    }

    record = (u8 *)&D_2614D8;
    if (g_miscExtras == 0) {
        *(s32 *)(record + 0x68) = 0xBD1;   /* region diff: USA 0x2C56 */
        *(s32 *)(record + 0x70) = 0;
        *(s32 *)(record + 0x40) = 0xBD1;   /* region diff: USA 0x2C56 */
        *(s32 *)(record + 0x48) = 0;
        *(s32 *)(record + 0x54) = 0xBD1;   /* region diff: USA 0x2C56 */
        *(s32 *)(record + 0x5C) = 0;
    } else {
        *(s32 *)(record + 0x70) = (s32)D_0025CCF8;
        *(s32 *)(record + 0x40) = 0x1114;  /* region diff: USA 0x3098 */
        *(s32 *)(record + 0x48) = (s32)D_0025C450;
        *(s32 *)(record + 0x54) = 0x111E;  /* region diff: USA 0x30A2 */
        *(s32 *)(record + 0x5C) = (s32)D_0025C5E8;
        *(s32 *)(record + 0x68) = 0x1408;  /* region diff: USA 0x30D5 */
    }

    func_003435C0(g_guiInstance + 0x3C210, &D_261398, &D_261438, &D_2614D8);
    func_00343690(g_guiInstance + 0x3C210, 2);
    return 0;
}
#endif

/* UpdateCheatMenuInput (EU twin of USA UpdateCheatMenuInput): per-frame input for the cheats
 * menu (cursor D_1ABAA0 over the 12-entry table D_1ABDC0, stride 4: +0x2 cheat id, +0x3 unlock
 * gate). Back (0x10) resets the cursor and pops the menu-screen block. Exit (0x900) resets the
 * cursor and returns 1. Up (0x1000)/down (0x4000) move to the prev/next non-disabled entry
 * (gate 0x64 = disabled), wrapping, with the move sound. Confirm (0x40) is allowed when the
 * entry is unlocked — gate 0x5A requires g_skillPointFlags, else the completed skill-point
 * count must reach the gate; on allow it plays accept + toggles g_cheatFlags[cheatId], reloading
 * the player model for the model cheats (2/9/10/0xB); on deny it plays reject. Matching arm stays
 * INCLUDE_ASM (packed-save wall); #else is the structure model. Word-verified vs USA
 * UpdateCheatMenuInput: D_1ABA30->D_1ABAA0 (gp); D_138180->D_138200; D_1ABD50->D_1ABDC0;
 * g_menuScreenBlock->D_001F0000+0x2840; LoadPlayerDisplayModel->LoadPlayerDisplayModel, PlayGlobalSound->
 * func_002E6C28. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", UpdateCheatMenuInput);
#else
s32 UpdateCheatMenuInput(void) {
    extern s32  CountSkillPointsCompleted(void);
    extern u8   D_138200[];               /* USA D_138180 */
    extern s32  D_1ABAA0;                 /* USA D_1ABA30 (gp cursor) */
    extern u8   D_1ABDC0[];               /* USA D_1ABD50 (cheat table) */
    extern u8   g_cheatFlags[];
    extern u8   g_skillPointFlags;
    extern s16  g_equippedArmor;
    extern void LoadPlayerDisplayModel(s16 armor); /* USA LoadPlayerDisplayModel */
    extern void func_002E6C28(s32 id, s32 a, s32 b); /* USA PlayGlobalSound */
    u8 *mb = D_001F0000 + 0x2840;
    s32 skillPts = CountSkillPointsCompleted();
    s32 flags = *(s32 *)(D_138200 + 0x1C4);
    s32 gate, cheatId, allow;

    if (flags & 0x10) {                 /* back */
        s32 e0 = *(s32 *)(*(u8 **)(mb + 0x14) + 0xE0);
        D_1ABAA0 = 0;
        if (e0 != 0) {
            *(s32 *)(mb + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(mb + 0x134) == 0) ? -1 : 0;
    }

    if (flags & 0x900) {                /* exit */
        D_1ABAA0 = 0;
        return 1;
    }

    if (flags & 0x1000) {               /* up: previous enabled entry */
        func_002E6C28(3, 0, 0);
        do {
            D_1ABAA0--;
            if (D_1ABAA0 < 0) {
                D_1ABAA0 = 0xB;
            }
        } while (D_1ABDC0[D_1ABAA0 * 4 + 3] == 0x64);
    } else if (flags & 0x4000) {        /* down: next enabled entry */
        func_002E6C28(3, 0, 0);
        do {
            D_1ABAA0++;
            if (D_1ABAA0 >= 0xC) {
                D_1ABAA0 = 0;
            }
        } while (D_1ABDC0[D_1ABAA0 * 4 + 3] == 0x64);
    }

    /* confirm */
    if (!(*(s32 *)(D_138200 + 0x1C4) & 0x40)) {
        return 0;
    }
    gate = D_1ABDC0[D_1ABAA0 * 4 + 3];
    if (gate == 0x5A) {
        allow = (g_skillPointFlags != 0);
    } else {
        allow = !(skillPts < gate);
    }
    if (!allow) {
        func_002E6C28(5, 0, 0);         /* reject */
        return 0;
    }
    func_002E6C28(4, 0, 0);             /* accept */

    cheatId = D_1ABDC0[D_1ABAA0 * 4 + 2];
    g_cheatFlags[cheatId] = (g_cheatFlags[cheatId] != 0) ? 0 : 1;

    cheatId = D_1ABDC0[D_1ABAA0 * 4 + 2];
    if (cheatId == 9 || cheatId == 10 || cheatId == 2 || cheatId == 0xB) {
        LoadPlayerDisplayModel(g_equippedArmor);
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawCheatMenu);

/* return 0 stub. */
s32 func_002D0B30(void) {
    return 0;
}

/* UpdateSkillPointsMenu (EU twin of USA UpdateSkillPointsMenu): per-frame input for the
 * skill-points menu (cursor g_nSkillPointsMenuCursor over 30 skill points). Back (0x10) resets
 * the cursor and pops the menu-screen block. Exit (0x900) resets the cursor and returns 1. Up
 * (0x1000)/down (0x4000) move the cursor (wrapping 0..0x1D) with the move sound. It then updates
 * the info panel for the current entry (D_0025C0B8+0x58 = cursor): when the point is completed
 * it records its meta id (g_skillPointMetaTable[cursor*6+2] -> D_25C094) and, if the cursor
 * didn't move, no file load is pending and the panel shows this entry, clears bit 0x4 of the
 * render object (*D_25C024+0x10); when not completed it sets that bit and zeroes D_25C094.
 * Matching arm stays INCLUDE_ASM (packed-save wall); #else is the structure model. Word-verified
 * vs USA UpdateSkillPointsMenu: g_menuScreenBlock->D_001F0000+0x2840; D_0025C098->D_0025C0B8;
 * D_25C004->D_25C024; D_25C074->D_25C094; g_fileLoadState->g_saveImageArea+0x1004 (lh);
 * PlayGlobalSound->func_002E6C28. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", UpdateSkillPointsMenu);
#else
s32 UpdateSkillPointsMenu(void) {
    extern s32  g_nSkillPointsMenuCursor;
    extern s32  g_padButtonsPressed;
    extern u8   g_skillPointFlags;
    extern u8   g_skillPointMetaTable[];
    extern u8   D_0025C0B8[];            /* USA D_0025C098 */
    extern u8  *D_25C024;               /* USA D_25C004 */
    extern s32  D_25C094;               /* USA D_25C074 */
    extern u8   g_saveImageArea[];       /* USA g_fileLoadState @ +0x1004 (s16) */
    extern void func_002E6C28(s32 id, s32 a, s32 b); /* USA PlayGlobalSound */
    u8 *mb = D_001F0000 + 0x2840;
    s32 buttons = g_padButtonsPressed;
    s32 cursor0 = g_nSkillPointsMenuCursor;
    s32 ret = 0;
    s32 cursor;

    if (buttons & 0x10) {           /* back */
        s32 e0 = *(s32 *)(*(u8 **)(mb + 0x14) + 0xE0);
        g_nSkillPointsMenuCursor = 0;
        if (e0 != 0) {
            *(s32 *)(mb + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(mb + 0x134) == 0) ? -1 : 0;
    }

    if (buttons & 0x900) {          /* exit */
        g_nSkillPointsMenuCursor = 0;
        ret = 1;
    } else if (buttons & 0x1000) {  /* up */
        s32 v;
        func_002E6C28(3, 0, 0);
        v = g_nSkillPointsMenuCursor - 1;
        g_nSkillPointsMenuCursor = (v >= 0) ? v : 0x1D;
    } else if (buttons & 0x4000) {  /* down */
        s32 v;
        func_002E6C28(3, 0, 0);
        v = g_nSkillPointsMenuCursor + 1;
        g_nSkillPointsMenuCursor = (v < 0x1E) ? v : 0;
    }

    cursor = g_nSkillPointsMenuCursor;
    *(s32 *)(D_0025C0B8 + 0x58) = cursor;

    if ((&g_skillPointFlags)[cursor] != 0) {
        s32 clear = 0;
        if (cursor0 == cursor && *(s16 *)(g_saveImageArea + 0x1004) == 0) {
            s32 mode = *(s32 *)(D_0025C0B8 + 0x44);
            if (*(s32 *)(D_0025C0B8 + 0x50) == cursor0 && mode == 2) {
                clear = 1;
            } else if (*(s32 *)(D_0025C0B8 + 0x54) == cursor0 && mode == 4) {
                clear = 1;
            }
        }
        if (clear) {
            *(s32 *)(D_25C024 + 0x10) &= ~0x4;
        }
        D_25C094 = *(s16 *)(g_skillPointMetaTable + cursor * 6 + 2);
    } else {
        *(s32 *)(D_25C024 + 0x10) |= 0x4;
        D_25C094 = 0;
    }
    return ret;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawSkillPointsMenu);

/* func_002D1118 (USA func_002D1150): trivial `return 0` stub, byte-identical to
 * the USA twin. The unit's tail boundary is settled by the verified Phase-A
 * tiling, so the stub is safe to port. */
s32 func_002D1118(void) {
    return 0;
}

/* UpdateExtrasMenuInput twin: per-frame input for the Extras carousel. Back (0x10) resets
 * the cursor + latches the screen's pending sub-result. Cancel (0x900) resets + returns 1.
 * Up/down play UI sound 3 and move the cursor. Confirm (0x40) dispatches per cursor: 0 requests
 * the sub-screen (g_pNextMenuScreen=D_25C2B8); 1..3 act only when enabled (D_1ABAB8[cursor]!=0,
 * else sound 5) — 1 freezes the screen + enters making-of state, 2/3 queue reels. Every
 * non-back path runs the present-record redraw fence. Matching arm stays INCLUDE_ASM
 * (comparison-tree switch + branch-likely fence); #else is the structure model. Word-verified
 * vs USA UpdateExtrasMenuInput: D_138180->D_138200; cursor->D_1ABAB4 (gp); enabled->D_1ABAB8
 * (gp); g_menuScreenBlock->D_001F0000+0x2840; D_25C298->D_25C2B8; CaptureScreenToVram->
 * CaptureScreenToVram; g_cameraCallbackCount+0x80->D_001B1380+0x200; EnqueueCinematic->func_002893B8,
 * StartCinematicFromQueue->StartCinematicFromQueue; g_cinematicQueue->g_nVendorBuyQuantity+0x8AF8;
 * g_cinematicExitPending->D_001A74F0+0x8; PlayGlobalSound->func_002E6C28; present record
 * D_0025C230->D_0025C250; live D_25C1F0->D_25C210; g_fileLoadState->g_saveImageArea+0x1004.
 * GENUINE REGION DIFFS (EU Extras menu has ONE FEWER item, cursor [0,3] not [0,4]): (a) up
 * wraps to 3 / down wraps at 4; (b) EU drops USA's `case 2: reel=0xBF` and shifts down (EU
 * case2={0xBD,0xBE}, case3=0xC1); (c) present-record slot remap slot=(cursor>=2)?cursor+1:cursor
 * (USA stores raw cursor). Confirmed against the .s. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", UpdateExtrasMenuInput);
#else
s32 UpdateExtrasMenuInput(void) {
    extern u8   D_138200[];
    extern s32  D_1ABAB4;
    extern s32  D_1ABAB8[];
    extern u8  *g_pNextMenuScreen;
    extern u8   D_25C2B8[];
    extern void CaptureScreenToVram(s32 mode);
    extern u8   D_001B1380[];                 /* g_cameraCallbackCount+0x80 -> +0x200 */
    extern s32  RequestGameStateChange(s32 stateId, s32 push, s32 c, s32 d, s32 e);
    extern s32  func_002893B8(void *queue, s32 reelId);
    extern s32  StartCinematicFromQueue(void *queue);
    extern u8   g_nVendorBuyQuantity[];       /* +0x8AF8 = g_cinematicQueue */
    extern u8   D_001A74F0[];                 /* +0x8 = g_cinematicExitPending */
    extern void func_002E6C28(s32 id, s32 a, s32 b);
    extern u8   D_0025C250[];                  /* present record */
    extern u8  *D_25C210;                      /* live menu object */
    extern u8   g_saveImageArea[];            /* +0x1004 = g_fileLoadState (s16) */
    s32 flags = *(s32 *)(D_138200 + 0x1C4);
    s32 cursor0 = D_1ABAB4;
    s32 result = 0;
    s32 sound = -1;
    s32 reel0 = -1, reel1 = -1;
    u8 *mb = D_001F0000 + 0x2840;
    s32 cursor;
    s32 slot;

    if (flags & 0x10) {
        s32 e0 = *(s32 *)(*(u8 **)(mb + 0x14) + 0xE0);
        D_1ABAB4 = 0;
        if (e0 != 0) {
            *(s32 *)(mb + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(mb + 0x134) == 0) ? -1 : 0;
    }

    if (flags & 0x900) {
        D_1ABAB4 = 0;
        return 1;
    } else if (flags & 0x1000) {
        s32 v;
        func_002E6C28(3, 0, 0);
        v = D_1ABAB4 - 1;
        D_1ABAB4 = (v >= 0) ? v : 3;
    } else if (flags & 0x4000) {
        s32 v;
        func_002E6C28(3, 0, 0);
        v = D_1ABAB4 + 1;
        D_1ABAB4 = (v < 4) ? v : 0;
    }

    flags = *(s32 *)(D_138200 + 0x1C4);
    if (flags & 0x40) {
        cursor = D_1ABAB4;
        switch (cursor) {
        case 0:
            g_pNextMenuScreen = D_25C2B8;
            sound = 4;
            break;
        case 1:
            if (D_1ABAB8[cursor] == 0) {
                sound = 5;
            } else {
                CaptureScreenToVram(1);
                *(s32 *)(D_001B1380 + 0x200) = 2;
                *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
                *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
                RequestGameStateChange(9, 1, 0, 0, 0);
                sound = 4;
            }
            break;
        case 2:
            if (D_1ABAB8[cursor] == 0) { sound = 5; }
            else { sound = 4; reel0 = 0xBD; reel1 = 0xBE; }
            break;
        case 3:
            if (D_1ABAB8[cursor] == 0) { sound = 5; }
            else { sound = 4; reel0 = 0xC1; }
            break;
        }
    }

    if (reel0 != -1) {
        CaptureScreenToVram(0);
        *(s32 *)(mb + 0x140) = *(s32 *)(D_001A74F0 + 0x8);
        *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
        *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
        *(s32 *)(D_001A74F0 + 0x8) = 2;
        func_002893B8(g_nVendorBuyQuantity + 0x8AF8, reel0);
        if (reel1 != -1) {
            func_002893B8(g_nVendorBuyQuantity + 0x8AF8, reel1);
        }
        StartCinematicFromQueue(g_nVendorBuyQuantity + 0x8AF8);
    }
    if (sound != -1) {
        func_002E6C28(sound, 0, 0);
    }

    cursor = D_1ABAB4;
    slot = (cursor >= 2) ? cursor + 1 : cursor;
    *(s32 *)(D_0025C250 + 0x58) = slot;
    if (D_1ABAB8[cursor] == 0) {
        *(s32 *)(D_25C210 + 0x10) |= 0x4;
    } else if (cursor0 == cursor && *(s16 *)(g_saveImageArea + 0x1004) == 0) {
        s32 mode = *(s32 *)(D_0025C250 + 0x44);
        if ((*(s32 *)(D_0025C250 + 0x50) == cursor0 && mode == 2) ||
            (*(s32 *)(D_0025C250 + 0x54) == cursor0 && mode == 4)) {
            *(s32 *)(D_25C210 + 0x10) &= ~0x4;
        }
    }
    return result;
}
#endif

/* DrawExtrasMenu twin: renders the Extras menu (chrome: four title glyphs, three header
 * strings, paging chrome) then a cursor-relative carousel. Matching arm stays INCLUDE_ASM
 * (8-byte-packed save + FP-arg scheduling); #else is the structure model. Word-verified vs USA
 * DrawExtrasMenu: func_003017F8->func_00301AC0; GuiFontAtlasLookupGlyph->func_00338AA8;
 * func_002801B8->func_00280050; DrawMenuPagingChrome->DrawMenuPagingChrome; func_0027F818->func_0027F680;
 * func_00280250->func_002800E8; DrawMenuItemSelectionBox->DrawMenuItemSelectionBox; g_screenWidth->
 * *(s32*)(D_001A7308+0xB8); y-fudge->g_nVendorBuyQuantity+0x160; row D_1ABA5C->D_1ABAC8; cursor->
 * D_1ABAB4; enabled->D_1ABAB8; label ids D_1ABD80->D_1ABDF0.
 * GENUINE REGION DIFFS (EU Extras = 4 items / 3 visible slots vs USA 5/5): loop runs 3 slots
 * (centre=1) not 5 (centre=2); index base (i+cursor-1) mod 4 not (i+cursor-2) mod 5; y 0x122 /
 * pitch 0x18 not 0x112 / 0x14; edge colour 0x30F0F0F0 (no 0x10/0x50 mid tier). PAL string ids:
 * header 0x3095->0x1111, 0x2BE5->0xB60, 0x2C0B->0xB86; locked-slot 0x2C56->0xBD1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawExtrasMenu);
#else
s32 DrawExtrasMenu(void) {
    extern void func_00301AC0(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                              f32 px, f32 py, f32 sx, f32 syg, f32 v38);
    extern s32  func_00338AA8(void *atlas, s32 codepoint);
    extern void DrawMenuPagingChrome(void);
    extern s32  func_001157AC(const char *s);
    extern s32  func_0027F680(char *str, s32 len);
    extern void func_002800E8(s32 x, s32 y, u32 color, const char *str, s32 flag);
    extern void func_00280050(s32 x, s32 y, u64 color, char *str, s64 sel);
    extern void DrawMenuItemSelectionBox(s32 width, s32 color);
    extern s32  D_1ABAB4;                        /* cursor */
    extern s32  D_1ABAB8[];                       /* per-item enabled flags */
    extern u8   D_1ABDF0[];                       /* per-item label string ids (stride 4) */
    extern s32  D_1ABAC8;                         /* title glyph row (int -> float) */
    extern u8   D_001A7308[];                      /* screen-width word @ +0xB8 */
    extern u8   g_nVendorBuyQuantity[];           /* sprite y-fudge @ +0x160 (f32) */
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(*(s32 *)(D_001A7308 + 0xB8) / 2);
    f32 row = (f32)D_1ABAC8;
    f32 yfudge = *(f32 *)(g_nVendorBuyQuantity + 0x160);
    s32 cursor;
    s32 i;
    s32 y;

    func_0027CA28(0);
    func_00301AC0(func_00338AA8(atlas, 0xD7), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xD8), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xD9), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xDD), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00280050(0xB3, 0x1B, 0x80F0F0F0, GetLocalizedString(0x1111), -1);
    func_00280050(0x161, 0x177, 0x80F0F0F0, GetLocalizedString(0xB60), -1);
    func_00280050(0xB5, 0x177, 0x80F0F0F0, GetLocalizedString(0xB86), -1);
    DrawMenuPagingChrome();
    cursor = D_1ABAB4;
    y = 0x122;
    for (i = 0; i < 3; i++) {
        s32 idx = i + cursor - 1;
        s32 strId;
        char *str;
        u32 color;
        if (idx < 0) idx += 4;
        if (idx >= 4) idx -= 4;
        strId = (D_1ABAB8[idx] == 0) ? 0xBD1 : *(s16 *)(D_1ABDF0 + idx * 4);
        str = GetLocalizedString(strId);
        if (i == 0 || i == 2) {
            color = 0x30F0F0F0;
        } else {
            color = 0x70F0F0F0;
            DrawMenuItemSelectionBox(func_0027F680(str, func_001157AC(str)), 0x70F0F0F0);
        }
        func_002800E8(*(s32 *)(D_001A7308 + 0xB8) / 2, y, color, str, -1);
        y += 0x18;
    }
    func_0027CB48();
    return 0;
}
#endif

/* When the GUI is up, latch the extras-menu availability flags: mark the last screen id (=1),
 * set the per-feature "new" flag when its feature is unlocked (D_1AA4D8) and, if any extras are
 * unlocked (g_miscExtras), the museum + master flags. Leaf. Matching arm stays INCLUDE_ASM
 * (delay-slot branch-fill + commutative store order not reproduced by cc1); #else is the
 * structure model. Word-verified vs USA func_002D1850: g_lastMenuScreenId/g_miscExtras keep
 * names; feature guard D_1AA450->D_1AA4D8 (gp); targets D_1ABA50->D_1ABAC0, D_1ABA58->D_1ABAC4,
 * D_1ABA4C->D_1ABABC (abs). GENUINE REGION DIFF: the EU Extras menu has ONE FEWER feature — EU
 * asm (0x50) drops USA's (0x64) SECOND guard block `if (D_1AA458) D_1ABA54=1;` entirely, so this
 * body has 2 guarded blocks, not 3. Confirmed against both .s files. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D17F0);
#else
s32 func_002D17F0(void) {
    extern s32 g_lastMenuScreenId;
    extern u8  g_miscExtras;
    extern s32 D_1AA4D8;    /* USA D_1AA450 (gp-rel feature-unlocked guard) */
    extern s32 D_1ABAC0;    /* USA D_1ABA50 */
    extern s32 D_1ABAC4;    /* USA D_1ABA58 */
    extern s32 D_1ABABC;    /* USA D_1ABA4C */
    if (g_guiInstance) {
        g_lastMenuScreenId = 1;
        if (D_1AA4D8) {
            D_1ABAC0 = 1;
        }
        if (g_miscExtras) {
            D_1ABAC4 = 1;
            D_1ABABC = 1;
        }
    }
    return 0;
}
#endif

/* CinematicsMenuTick twin: per-frame input for the Goodies "Cinematics" screen, a 33-reel
 * carousel over D_2615E0 (6-byte entries: u16 nameStringId, u16 reelId @+2, u16 unlocked @+4).
 * Back (0x10) resets the cursor and latches the active screen's pending sub-result. Cancel
 * (0x900) resets + returns 1. Up/down (0x1000/0x4000) play UI sound 3 and move the cursor with
 * wrap in [0,0x20]. Confirm (0x40) on an unlocked reel plays sound 4, freezes the screen, stashes
 * exit-pending + block fields, sets exit-pending=2, enqueues + starts the reel; a locked reel
 * plays 5. Every non-back/non-cancel path runs the present-record redraw fence. Matching arm
 * stays INCLUDE_ASM (8-byte-packed save wall); #else is the structure model. Word-verified vs USA
 * CinematicsMenuTick: D_138180->D_138200; cursor D_1ABA60->D_1ABACC (gp); g_menuScreenBlock->
 * D_001F0000+0x2840; g_cinematicsMenuTable->D_2615E0; g_cinematicExitPending->D_001A74F0+0x8;
 * g_cinematicQueue->g_nVendorBuyQuantity+0x8AF8; EnqueueCinematic->func_002893B8,
 * StartCinematicFromQueue->StartCinematicFromQueue; CaptureScreenToVram->CaptureScreenToVram; PlayGlobalSound->
 * func_002E6C28; present record D_0025C3C8->D_0025C3E8; live D_25C388->D_25C3A8; g_fileLoadState->
 * g_saveImageArea+0x1004. Clean twin. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", CinematicsMenuTick);
#else
s32 CinematicsMenuTick(void) {
    extern u8   D_138200[];
    extern s32  D_1ABACC;
    extern u8   D_2615E0[];
    extern u8   D_001A74F0[];                 /* +0x8 = g_cinematicExitPending */
    extern u8   g_nVendorBuyQuantity[];       /* +0x8AF8 = g_cinematicQueue */
    extern u8   D_0025C3E8[];                  /* present record */
    extern u8  *D_25C3A8;                      /* live menu object */
    extern u8   g_saveImageArea[];            /* +0x1004 = g_fileLoadState (s16) */
    extern void func_002E6C28(s32 id, s32 a, s32 b);
    extern void CaptureScreenToVram(s32 mode);
    extern s32  func_002893B8(void *queue, s32 reelId);
    extern s32  StartCinematicFromQueue(void *queue);
    s32 flags = *(s32 *)(D_138200 + 0x1C4);
    s32 cursor0 = D_1ABACC;
    s32 result = 0;
    u8 *mb = D_001F0000 + 0x2840;
    s32 cursor;

    if (flags & 0x10) {
        s32 e0 = *(s32 *)(*(u8 **)(mb + 0x14) + 0xE0);
        D_1ABACC = 0;
        if (e0 != 0) {
            *(s32 *)(mb + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(mb + 0x134) == 0) ? -1 : 0;
    }

    if (flags & 0x900) {
        D_1ABACC = 0;
        return 1;
    } else if (flags & 0x1000) {
        s32 v;
        func_002E6C28(3, 0, 0);
        v = D_1ABACC - 1;
        D_1ABACC = (v >= 0) ? v : 0x20;
    } else if (flags & 0x4000) {
        s32 v;
        func_002E6C28(3, 0, 0);
        v = D_1ABACC + 1;
        D_1ABACC = (v < 0x21) ? v : 0;
    }

    flags = *(s32 *)(D_138200 + 0x1C4);
    if (flags & 0x40) {
        cursor = D_1ABACC;
        if (*(s16 *)(D_2615E0 + cursor * 6 + 4) != 0) {
            s16 reelId;
            func_002E6C28(4, 0, 0);
            CaptureScreenToVram(0);
            cursor = D_1ABACC;
            reelId = *(s16 *)(D_2615E0 + cursor * 6 + 2);
            *(s32 *)(mb + 0x140) = *(s32 *)(D_001A74F0 + 0x8);
            *(s32 *)(mb + 0x100) = *(s32 *)(mb + 0x14);
            *(s32 *)(mb + 0x104) = *(s32 *)(mb + 0x8);
            *(s32 *)(D_001A74F0 + 0x8) = 2;
            func_002893B8(g_nVendorBuyQuantity + 0x8AF8, reelId);
            StartCinematicFromQueue(g_nVendorBuyQuantity + 0x8AF8);
        } else {
            func_002E6C28(5, 0, 0);
        }
    }

    cursor = D_1ABACC;
    *(s32 *)(D_0025C3E8 + 0x58) = cursor;
    if (*(s16 *)(D_2615E0 + cursor * 6 + 4) == 0) {
        *(s32 *)(D_25C3A8 + 0x10) |= 0x4;
    } else if (cursor0 == cursor && *(s16 *)(g_saveImageArea + 0x1004) == 0) {
        s32 mode = *(s32 *)(D_0025C3E8 + 0x44);
        if ((*(s32 *)(D_0025C3E8 + 0x50) == cursor0 && mode == 2) ||
            (*(s32 *)(D_0025C3E8 + 0x54) == cursor0 && mode == 4)) {
            *(s32 *)(D_25C3A8 + 0x10) &= ~0x4;
        }
    }
    return result;
}
#endif

/* DrawCinematicsMenu twin: renders the cinematics reel-carousel screen (chrome: four title
 * glyphs, three header strings, paging chrome) then a five-slot cursor-relative carousel over
 * D_2615E0 (33 x 6-byte reels, cursor D_1ABACC), each slot showing the reel at (i+cursor-2) mod
 * 0x21, labelled with the reel name when unlocked else 0xBD1, colour fading by distance from
 * centre. Matching arm stays INCLUDE_ASM (8-byte-packed save + FP-arg scheduling); #else is the
 * structure model. Word-verified vs USA DrawCinematicsMenu: func_003017F8->func_00301AC0;
 * GuiFontAtlasLookupGlyph->func_00338AA8; func_002801B8->func_00280050; DrawMenuPagingChrome->
 * DrawMenuPagingChrome; func_0027F818->func_0027F680; func_00280250->func_002800E8;
 * DrawMenuItemSelectionBox->DrawMenuItemSelectionBox; g_screenWidth->*(s32*)(D_001A7308+0xB8); y-fudge->
 * g_nVendorBuyQuantity+0x160; row D_1ABA64->D_1ABAD0; cursor D_1ABA60->D_1ABACC; table->D_2615E0.
 * REGION DIFF (PAL string ids): header 0x2CA9->0xC23, 0x2BE5->0xB60, 0x2C0B->0xB86; locked-slot
 * 0x2C56->0xBD1. Structurally a clean 5-slot twin. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawCinematicsMenu);
#else
s32 DrawCinematicsMenu(void) {
    extern void func_00301AC0(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                              f32 px, f32 py, f32 sx, f32 syg, f32 v38);
    extern s32  func_00338AA8(void *atlas, s32 codepoint);
    extern void DrawMenuPagingChrome(void);
    extern s32  func_001157AC(const char *s);
    extern s32  func_0027F680(char *str, s32 len);
    extern void func_002800E8(s32 x, s32 y, u32 color, const char *str, s32 flag);
    extern void func_00280050(s32 x, s32 y, u64 color, char *str, s64 sel);
    extern void DrawMenuItemSelectionBox(s32 width, s32 color);
    extern u8   D_2615E0[];
    extern s32  D_1ABACC;                       /* cursor */
    extern s32  D_1ABAD0;                       /* title glyph row (int -> float) */
    extern u8   D_001A7308[];                    /* screen-width word @ +0xB8 */
    extern u8   g_nVendorBuyQuantity[];         /* sprite y-fudge @ +0x160 (f32) */
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(*(s32 *)(D_001A7308 + 0xB8) / 2);
    f32 row = (f32)D_1ABAD0;
    f32 yfudge = *(f32 *)(g_nVendorBuyQuantity + 0x160);
    s32 cursor;
    s32 i;
    s32 y;

    func_0027CA28(0);
    func_00301AC0(func_00338AA8(atlas, 0xD7), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xD8), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xD9), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xDD), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00280050(0xB3, 0x1B, 0x80F0F0F0, GetLocalizedString(0xC23), -1);
    func_00280050(0x161, 0x177, 0x80F0F0F0, GetLocalizedString(0xB60), -1);
    func_00280050(0xB5, 0x177, 0x80F0F0F0, GetLocalizedString(0xB86), -1);
    DrawMenuPagingChrome();
    cursor = D_1ABACC;
    y = 0x112;
    for (i = 0; i < 5; i++) {
        s32 idx = i + cursor - 2;
        s32 base;
        s32 strId;
        char *str;
        u32 color;
        if (idx < 0) idx += 0x21;
        if (idx >= 0x21) idx -= 0x21;
        base = idx * 6;
        strId = (*(s16 *)(D_2615E0 + base + 4) != 0)
                    ? *(s16 *)(D_2615E0 + base + 0) : 0xBD1;
        str = GetLocalizedString(strId);
        if (i == 0 || i == 4) {
            color = 0x10F0F0F0;
        } else if (i == 1 || i == 3) {
            color = 0x50F0F0F0;
        } else {
            color = 0x70F0F0F0;
            DrawMenuItemSelectionBox(func_0027F680(str, func_001157AC(str)), 0x70F0F0F0);
        }
        func_002800E8(*(s32 *)(D_001A7308 + 0xB8) / 2, y, color, str, -1);
        y += 0x14;
    }
    func_0027CB48();
    return 0;
}
#endif

/* When the GUI is up, set g_lastMenuScreenId=1 then walk the 32-entry, 6-byte-stride
 * cinematics-menu row table (D_2615E2): for each row, if any extras are unlocked (g_miscExtras)
 * mark the row available (+8 = 1); otherwise test the row's cinematic id (the +6 field) against
 * g_cinematicUnlockedFlags and write the bit result into the +8 availability field. Leaf.
 * Matching arm stays INCLUDE_ASM (bnel branch-likely delay-slot store on the extras-unlocked
 * fast path not reproduced by cc1); #else is the structure model. Word-verified vs USA
 * func_002D1E88: g_lastMenuScreenId/g_miscExtras/g_cinematicUnlockedFlags keep names; table
 * D_26183A->D_2615E2 (region-shifted). Structurally identical twin. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D1E10);
#else
void func_002D1E10(void) {
    extern s32 g_lastMenuScreenId;
    extern u8  g_miscExtras;
    extern u8  g_cinematicUnlockedFlags[];
    extern u8  D_2615E2[];                 /* USA D_26183A (cinematics-menu row table) */
    u8 *row;
    if (g_guiInstance == NULL) {
        return;
    }
    g_lastMenuScreenId = 1;
    for (row = D_2615E2; row < D_2615E2 + 0xC0; row += 6) {
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

/* UpdatePlanetWarpMenuInput twin: per-frame input for the planet-warp menu, an 8-item cursor
 * list [0,7]. Back (0x10) resets + latches; cancel (0x900) resets + returns 1; up/down play sound
 * 3 and wrap-move; confirm (0x40) plays sound 4 for an enabled item (5 for disabled) and, via the
 * cursor switch, sets warp mode D_1A7984 and calls RequestLevelExit for a fixed level id (cursor
 * 7 also stashes g_playerProgress into D_1A7988 when != 0x15). Every non-back/cancel path runs
 * the present-record redraw fence. Matching arm stays INCLUDE_ASM (switch/jtbl + branch-likely
 * fence); #else is the structure model. Word-verified vs USA UpdatePlanetWarpMenuInput: D_138180->
 * D_138200; cursor->D_1ABAD4 (gp); enabled->D_1ABAD8 (gp); g_menuScreenBlock->D_001F0000+0x2840;
 * warp mode D_1A7904->D_1A7984 (gp); stashed progress D_1A7908->D_1A7988 (abs); g_playerProgress/
 * RequestLevelExit kept; PlayGlobalSound->func_002E6C28; present record D_0025C560->D_0025C580;
 * live D_25C520->D_25C540; g_fileLoadState->g_saveImageArea+0x1004. Clean twin (8 items). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", UpdatePlanetWarpMenuInput);
#else
s32 UpdatePlanetWarpMenuInput(void) {
    extern u8   D_138200[];
    extern s32  D_1ABAD4;
    extern u8   D_1ABAD8[];                    /* per-item enabled flags */
    extern s32  D_1A7984;                      /* warp-mode selector (gp) */
    extern s32  D_1A7988;                      /* stashed prior progress (abs) */
    extern s32  g_playerProgress;
    extern void RequestLevelExit(s32 destination, s32 commitSave);
    extern void func_002E6C28(s32 id, s32 a, s32 b);
    extern u8   D_0025C580[];                   /* present record */
    extern u8  *D_25C540;                       /* live menu object */
    extern u8   g_saveImageArea[];             /* +0x1004 = g_fileLoadState (s16) */
    s32 flags = *(s32 *)(D_138200 + 0x1C4);
    s32 cursor0 = D_1ABAD4;
    s32 result = 0;
    u8 *mb = D_001F0000 + 0x2840;
    s32 cursor;

    if (flags & 0x10) {
        s32 e0 = *(s32 *)(*(u8 **)(mb + 0x14) + 0xE0);
        D_1ABAD4 = 0;
        if (e0 != 0) {
            *(s32 *)(mb + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(mb + 0x134) == 0) ? -1 : 0;
    }

    if (flags & 0x900) {
        D_1ABAD4 = 0;
        return 1;
    } else if (flags & 0x1000) {
        s32 v;
        func_002E6C28(3, 0, 0);
        v = D_1ABAD4 - 1;
        D_1ABAD4 = (v >= 0) ? v : 7;
    } else if (flags & 0x4000) {
        s32 v;
        func_002E6C28(3, 0, 0);
        v = D_1ABAD4 + 1;
        D_1ABAD4 = (v < 8) ? v : 0;
    }

    flags = *(s32 *)(D_138200 + 0x1C4);
    if (flags & 0x40) {
        cursor = D_1ABAD4;
        if (D_1ABAD8[cursor] == 0) {
            func_002E6C28(5, 0, 0);
        } else {
            func_002E6C28(4, 0, 0);
            cursor = D_1ABAD4;
            if ((u32)cursor < 8) {
                switch (cursor) {
                case 0: D_1A7984 = 2; RequestLevelExit(0x2, 1); break;
                case 1: D_1A7984 = 3; RequestLevelExit(0x4, 1); break;
                case 2: D_1A7984 = 3; RequestLevelExit(0xB, 1); break;
                case 3: D_1A7984 = 2; RequestLevelExit(0xB, 1); break;
                case 4: D_1A7984 = 1; RequestLevelExit(0x1A, 1); break;
                case 5: D_1A7984 = 1; RequestLevelExit(0x16, 1); break;
                case 6: D_1A7984 = 1; RequestLevelExit(0x17, 1); break;
                case 7:
                    D_1A7984 = 1;
                    if (g_playerProgress != 0x15) {
                        D_1A7988 = g_playerProgress;
                    }
                    RequestLevelExit(0x15, 1);
                    break;
                }
            }
        }
    }

    cursor = D_1ABAD4;
    *(s32 *)(D_0025C580 + 0x58) = cursor;
    if (D_1ABAD8[cursor] == 0) {
        *(s32 *)(D_25C540 + 0x10) |= 0x4;
    } else if (cursor0 == cursor && *(s16 *)(g_saveImageArea + 0x1004) == 0) {
        s32 mode = *(s32 *)(D_0025C580 + 0x44);
        if ((*(s32 *)(D_0025C580 + 0x50) == cursor0 && mode == 2) ||
            (*(s32 *)(D_0025C580 + 0x54) == cursor0 && mode == 4)) {
            *(s32 *)(D_25C540 + 0x10) &= ~0x4;
        }
    }
    return result;
}
#endif

/* DrawPlanetWarpMenu twin: renders the planet-warp menu (same chrome) then a five-slot cursor-
 * relative carousel of 8 planets (cursor D_1ABAD4) at (i+cursor-2) mod 8, labelled with
 * D_1ABE00[idx] when enabled (D_1ABAD8[idx]!=0) else 0xBD1, colour fading by distance from centre.
 * Matching arm stays INCLUDE_ASM (8-byte-packed save + FP-arg scheduling); #else is the structure
 * model. Word-verified vs USA DrawPlanetWarpMenu: func_003017F8->func_00301AC0;
 * GuiFontAtlasLookupGlyph->func_00338AA8; func_002801B8->func_00280050; DrawMenuPagingChrome->
 * DrawMenuPagingChrome; func_0027F818->func_0027F680; func_00280250->func_002800E8;
 * DrawMenuItemSelectionBox->DrawMenuItemSelectionBox; g_screenWidth->*(s32*)(D_001A7308+0xB8); y-fudge->
 * g_nVendorBuyQuantity+0x160; row D_1ABA78->D_1ABAE0; cursor->D_1ABAD4; enabled->D_1ABAD8; label
 * array D_1ABD98->D_1ABE00. REGION DIFF (PAL string ids): header 0x3098->0x1114, 0x2BE5->0xB60,
 * 0x2C0B->0xB86; locked-slot 0x2C56->0xBD1. Structurally a clean 5-slot twin. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawPlanetWarpMenu);
#else
s32 DrawPlanetWarpMenu(void) {
    extern void func_00301AC0(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                              f32 px, f32 py, f32 sx, f32 syg, f32 v38);
    extern s32  func_00338AA8(void *atlas, s32 codepoint);
    extern void DrawMenuPagingChrome(void);
    extern s32  func_001157AC(const char *s);
    extern s32  func_0027F680(char *str, s32 len);
    extern void func_002800E8(s32 x, s32 y, u32 color, const char *str, s32 flag);
    extern void func_00280050(s32 x, s32 y, u64 color, char *str, s64 sel);
    extern void DrawMenuItemSelectionBox(s32 width, s32 color);
    extern u8   D_1ABAD8[];                      /* per-planet enabled flags */
    extern s16  D_1ABE00[];                      /* per-planet label string ids */
    extern s32  D_1ABAD4;                        /* cursor */
    extern s32  D_1ABAE0;                        /* title glyph row (int -> float) */
    extern u8   D_001A7308[];                     /* screen-width word @ +0xB8 */
    extern u8   g_nVendorBuyQuantity[];          /* sprite y-fudge @ +0x160 (f32) */
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(*(s32 *)(D_001A7308 + 0xB8) / 2);
    f32 row = (f32)D_1ABAE0;
    f32 yfudge = *(f32 *)(g_nVendorBuyQuantity + 0x160);
    s32 cursor;
    s32 i;
    s32 y;

    func_0027CA28(0);
    func_00301AC0(func_00338AA8(atlas, 0xD7), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xD8), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xD9), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xDD), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00280050(0xB3, 0x1B, 0x80F0F0F0, GetLocalizedString(0x1114), -1);
    func_00280050(0x161, 0x177, 0x80F0F0F0, GetLocalizedString(0xB60), -1);
    func_00280050(0xB5, 0x177, 0x80F0F0F0, GetLocalizedString(0xB86), -1);
    DrawMenuPagingChrome();
    cursor = D_1ABAD4;
    y = 0x112;
    for (i = 0; i < 5; i++) {
        s32 idx = i + cursor - 2;
        s32 strId;
        char *str;
        u32 color;
        if (idx < 0) idx += 8;
        if (idx >= 8) idx -= 8;
        strId = (D_1ABAD8[idx] != 0) ? D_1ABE00[idx] : 0xBD1;
        str = GetLocalizedString(strId);
        if (i == 0 || i == 4) {
            color = 0x10F0F0F0;
        } else if (i == 1 || i == 3) {
            color = 0x50F0F0F0;
        } else {
            color = 0x70F0F0F0;
            DrawMenuItemSelectionBox(func_0027F680(str, func_001157AC(str)), 0x70F0F0F0);
        }
        func_002800E8(*(s32 *)(D_001A7308 + 0xB8) / 2, y, color, str, -1);
        y += 0x14;
    }
    func_0027CB48();
    return 0;
}
#endif

/* func_002D24C0: EU twin of USA func_002D2538 — when the GUI is up and extras are unlocked,
 * latch the per-feature "new content" availability flags for the planet-warp / museum sub-menus.
 * Leaf. Matching arm stays INCLUDE_ASM (bnel/beql branch-likely delay-slot value moves); #else
 * is the structure model. Word-verified vs USA func_002D2538: g_guiInstance/g_miscExtras/
 * g_playerProgress keep names; source bytes D_1A7BF2/F4/FB/C06/C07/C0A->D_1A7C72/74/7B/86/87/8A
 * (+0x80); prereq bytes D_1395E9->D_139669, D_1395B8[]->D_139638[] (+0x80); D_1AA458->D_1AA4D8
 * (gp); target flags D_1ABA70[]->D_1ABAD8[] (gp / +0x68). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D24C0);
#else
void func_002D24C0(void) {
    extern u8  g_miscExtras;
    extern s32 g_playerProgress;
    extern u8  D_1A7C72, D_1A7C74, D_1A7C7B, D_1A7C86, D_1A7C87, D_1A7C8A;
    extern u8  D_139669;
    extern u8  D_139638[];
    extern s32 D_1AA4D8;
    extern u8  D_1ABAD8[];  /* planet-warp per-item enabled flags (same array as UpdatePlanetWarpMenuInput) */

    if (g_guiInstance == NULL) {
        return;
    }
    if (g_miscExtras == 0) {
        return;
    }
    if (D_1A7C72 != 0) {
        if (D_139669 != 0) {
            D_1ABAD8[0] = 1;
        }
    }
    if (D_1A7C74 != 0) {
        if (D_139638[0x4D] != 0 || D_139638[0x52] != 0) {
            D_1ABAD8[1] = 1;
        }
    }
    if (D_1A7C7B != 0) {
        if (D_139638[0x57] != 0 || D_139638[0x5C] != 0) {
            D_1ABAD8[2] = 1;
        }
    }
    if (D_1A7C7B != 0) {
        if (D_139638[0x3D] != 0) {
            D_1ABAD8[3] = 1;
        }
    }
    if (D_1A7C8A != 0) {
        D_1ABAD8[4] = 1;
    }
    if (D_1A7C86 != 0) {
        D_1ABAD8[5] = 1;
    }
    if (D_1A7C87 != 0) {
        D_1ABAD8[6] = 1;
    }
    if (D_1AA4D8 != 0 && g_playerProgress > 0) {
        D_1ABAD8[7] = 1;
    }
}
#endif

/* UpdateInsomniacMuseumInput: EU twin of USA UpdateInsomniacMuseumInput — per-frame input for the Insomniac
 * Museum menu, a 5-item carousel. Back/cancel/up/down + confirm dispatch (items 1/2 spawn the
 * exhibit moby with a 128-bit hero-pos copy; 0/3/4 request sub-screens) + present-record redraw
 * fence. Matching arm stays INCLUDE_ASM (switch/jtbl + lq/sq hero-pos copy + branch-likely fence);
 * #else is the structure model. Word-verified vs USA UpdateInsomniacMuseumInput: D_138180->
 * D_138200; cursor->D_1ABAE4 (gp); enabled->D_1ABAE8 (gp); g_menuScreenBlock->D_001F0000+0x2840;
 * PlayGlobalSound->func_002E6C28; SwapMobyTableContext->SwapMobyTableContext; SpawnMoby->SpawnMoby;
 * sub-screen records D_25C760/950/B40->D_25C780/970/B60; g_heroPos->g_sndChannelVolumes+0x17F8;
 * present record D_0025C6F8->D_0025C718; live D_25C6B8->D_25C6D8; g_fileLoadState->
 * g_saveImageArea+0x1004. REGION: 5 items in BOTH builds (no divergence). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", UpdateInsomniacMuseumInput);
#else
s32 UpdateInsomniacMuseumInput(void) {
    extern void  func_002E6C28(s32 id, s32 a, s32 b);   /* PlayGlobalSound */
    extern void  SwapMobyTableContext(s32 tableId);            /* SwapMobyTableContext */
    extern void *SpawnMoby(s32 classId);            /* SpawnMoby */
    extern s32   D_1ABAE4;                 /* museum cursor 0..4 (gp) */
    extern s32   D_1ABAE8;                 /* per-item enabled flags base (gp) */
    extern u8    D_138200[];               /* global input/UI flags (+0x1C4) */
    extern u8   *g_pNextMenuScreen;        /* requested next screen */
    extern u8    g_sndChannelVolumes[];    /* +0x17F8 = hero world position vec4 */
    extern u8    D_25C780[], D_25C970[], D_25CB60[]; /* museum sub-screen records */
    extern u8    D_0025C718[];             /* per-screen present record */
    extern u8   *D_25C6D8;                 /* pointer to the live menu object */
    extern u8    g_saveImageArea[];        /* +0x1004 = g_fileLoadState (s16) */
    u8 *mb = D_001F0000 + 0x2840;
    s32 flags = *(s32 *)(D_138200 + 0x1C4);
    s32 cursor0 = D_1ABAE4;
    s32 result = 0;
    s32 cursor;

    if (flags & 0x10) {                 /* back */
        s32 e0 = *(s32 *)(*(u8 **)(mb + 0x14) + 0xE0);
        D_1ABAE4 = 0;
        if (e0 != 0) {
            *(s32 *)(mb + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(mb + 0x134) == 0) ? -1 : 0;
    }

    if (flags & 0x900) {                /* cancel */
        D_1ABAE4 = 0;
        return 1;
    } else if (flags & 0x1000) {        /* up */
        s32 v;
        func_002E6C28(3, 0, 0);
        v = D_1ABAE4 - 1;
        D_1ABAE4 = (v >= 0) ? v : 4;
    } else if (flags & 0x4000) {        /* down */
        s32 v;
        func_002E6C28(3, 0, 0);
        v = D_1ABAE4 + 1;
        D_1ABAE4 = (v < 5) ? v : 0;
    }

    flags = *(s32 *)(D_138200 + 0x1C4);
    if (flags & 0x40) {                 /* confirm */
        cursor = D_1ABAE4;
        if ((&D_1ABAE8)[cursor] == 0) {
            func_002E6C28(5, 0, 0);     /* disabled item */
        } else {
            func_002E6C28(4, 0, 0);
            cursor = D_1ABAE4;
            if ((u32)cursor < 5) {
                switch (cursor) {
                case 0:
                    g_pNextMenuScreen = D_25C780;
                    break;
                case 1:
                case 2: {
                    void *moby;
                    s32 id = ((cursor ^ 2) != 0) ? 0x1335 : 0x88D;
                    SwapMobyTableContext(0);
                    moby = SpawnMoby(id);
                    SwapMobyTableContext(1);
                    if (moby != 0) {
                        u8 *m = (u8 *)moby;
                        m[0x30] = 0xFF;
                        *(u32 *)(m + 0x10) = *(u32 *)(g_sndChannelVolumes + 0x17F8);
                        *(u32 *)(m + 0x14) = *(u32 *)(g_sndChannelVolumes + 0x17FC);
                        *(u32 *)(m + 0x18) = *(u32 *)(g_sndChannelVolumes + 0x1800);
                        *(u32 *)(m + 0x1C) = *(u32 *)(g_sndChannelVolumes + 0x1804);
                        result = 1;
                    }
                    break;
                }
                case 3:
                    g_pNextMenuScreen = D_25C970;
                    break;
                case 4:
                    g_pNextMenuScreen = D_25CB60;
                    break;
                }
            }
        }
    }

    cursor = D_1ABAE4;
    *(s32 *)(D_0025C718 + 0x58) = cursor;
    if ((&D_1ABAE8)[cursor] == 0) {
        *(s32 *)(D_25C6D8 + 0x10) |= 0x4;
    } else if (cursor0 == cursor && *(s16 *)(g_saveImageArea + 0x1004) == 0) {
        s32 mode = *(s32 *)(D_0025C718 + 0x44);
        if ((*(s32 *)(D_0025C718 + 0x50) == cursor0 && mode == 2) ||
            (*(s32 *)(D_0025C718 + 0x54) == cursor0 && mode == 4)) {
            *(s32 *)(D_25C6D8 + 0x10) &= ~0x4;
        }
    }
    return result;
}
#endif

/* DrawInsomniacMuseumMenu: EU twin of USA DrawInsomniacMuseumMenu — renders the Insomniac Museum screen:
 * three title glyphs (0x8B/0x8C/0x8D, v38 0.775f), three centred header strings, four framing
 * lines, then five column items (selected row highlighted 0x7000FFFF else 0x80F0F0F0; label is
 * the item's table id when enabled, else the PAL fallback). Matching arm stays INCLUDE_ASM
 * (packed-save + FP-arg scheduling); #else is the structure model. Word-verified vs USA
 * DrawInsomniacMuseumMenu: func_003017F8->func_00301AC0; GuiFontAtlasLookupGlyph->func_00338AA8;
 * func_002801B8->func_00280050; func_002904B0->func_002904C8; g_screenWidth->*(s32*)(D_001A7308+
 * 0xB8); row D_1ABA94->D_1ABAFC (gp); y-fudge->g_nVendorBuyQuantity+0x160; cursor->D_1ABAE4 (gp);
 * enabled->D_1ABAE8 (gp); label table D_1ABDA8->D_1ABE10. REGION: 5 items in BOTH builds. PAL
 * string ids: headers 0x30A2->0x111E, 0x2C0B->0xB86, 0x2BE5->0xB60; locked fallback 0x2C56->0xBD1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawInsomniacMuseumMenu);
#else
s32 DrawInsomniacMuseumMenu(void) {
    extern void func_00301AC0(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                              f32 px, f32 py, f32 sx, f32 syg, f32 v38);
    extern s32  func_00338AA8(void *atlas, s32 codepoint);
    extern void func_002904C8(s32 x0, s32 y0, s32 x1, s32 y1, s32 color, s32 flag);
    extern void func_00280050(s32 x, s32 y, u64 color, char *str, s64 sel);
    extern s32  D_1ABAE4;                /* museum cursor (gp) */
    extern s32  D_1ABAE8;               /* per-item enabled flags base (gp) */
    extern s32  D_1ABE10[];             /* per-item label string ids (stride 4) */
    extern s32  D_1ABAFC;               /* title glyph row (int -> float, gp) */
    extern u8   D_001A7308[];            /* screen-width word @ +0xB8 */
    extern u8   g_nVendorBuyQuantity[]; /* sprite y-fudge @ +0x160 (f32) */
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(*(s32 *)(D_001A7308 + 0xB8) / 2);
    f32 row = (f32)D_1ABAFC;
    f32 yfudge = *(f32 *)(g_nVendorBuyQuantity + 0x160);
    s32 cursor;
    s32 i;
    s32 y;

    func_0027CA28(0);
    func_00301AC0(func_00338AA8(atlas, 0x8B), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.775f);
    func_00301AC0(func_00338AA8(atlas, 0x8C), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.775f);
    func_00301AC0(func_00338AA8(atlas, 0x8D), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.775f);
    func_00280050(*(s32 *)(D_001A7308 + 0xB8) / 2, 0x41, 0x80F0F0F0, GetLocalizedString(0x111E), -1);
    func_00280050(*(s32 *)(D_001A7308 + 0xB8) / 2, 0x141, 0x80F0F0F0, GetLocalizedString(0xB86), -1);
    func_00280050(*(s32 *)(D_001A7308 + 0xB8) / 2, 0x15A, 0x80F0F0F0, GetLocalizedString(0xB60), -1);
    func_002904C8(0x138, 0x80, 0x1C1, 0x82, 0x55F0C070, 0);
    func_002904C8(0x138, 0x109, 0x1C3, 0x10B, 0x55F0C070, 0);
    func_002904C8(0x138, 0x80, 0x13A, 0x109, 0x55F0C070, 0);
    func_002904C8(0x1C1, 0x80, 0x1C3, 0x109, 0x55F0C070, 0);
    cursor = D_1ABAE4;
    y = 0x7D;
    for (i = 0; i < 5; i++) {
        u32 color = (i == cursor) ? 0x7000FFFF : 0x80F0F0F0;
        s32 strId = ((&D_1ABAE8)[i] != 0) ? D_1ABE10[i] : 0xBD1;
        func_00280050(0xA5, y, color, GetLocalizedString(strId), -1);
        y += 0x1F;
    }
    func_0027CB48();
    return 0;
}
#endif

/* func_002D2BE8: EU twin of USA func_002D2C60 — when the GUI is up, latch the extras-menu "new
 * content" flags for each entry. Matching arm stays INCLUDE_ASM (cc1 hoists the g_miscExtras load
 * into the g_guiInstance short-circuit); #else is the structure model. Word-verified vs USA
 * func_002D2C60: g_guiInstance/g_miscExtras keep names; sources D_1AA450/454/458->D_1AA4D0/4D4/4D8
 * (gp); flags D_1ABA8C/88/84/90->D_1ABAF4/F0/EC/F8 (+0x68), D_1ABA80->D_1ABAE8 (gp). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D2BE8);
#else
s32 func_002D2BE8(void) {
    extern u8  g_miscExtras;
    extern s32 D_1AA4D0, D_1AA4D4, D_1AA4D8;   /* gp-rel */
    extern s32 D_1ABAEC, D_1ABAF0, D_1ABAF4, D_1ABAF8;
    extern s32 D_1ABAE8;                        /* gp-rel */

    if (g_guiInstance) {
        if (g_miscExtras) {
            D_1ABAF4 = 1;
            D_1ABAF0 = 1;
        }
        if (D_1AA4D0) {
            D_1ABAEC = 1;
        }
        if (D_1AA4D4) {
            D_1ABAF8 = 1;
        }
        if (D_1AA4D8) {
            D_1ABAE8 = 1;
        }
    }
    return 0;
}
#endif

/* UpdateHelpTopicMenuInput: EU twin of USA UpdateHelpTopicMenuInput — per-frame input for the help-topics
 * screen, an 18-entry page list. Back/cancel + prev(0x8000)/next(0x2000) paging with edge sounds,
 * then stamps the active topic + its string id. Matching arm stays INCLUDE_ASM (packed-save);
 * #else is the structure model. Word-verified vs USA UpdateHelpTopicMenuInput: g_padButtonsPressed
 * kept; cursor->D_1ABB00 (gp); g_menuScreenBlock->D_001F0000+0x2840; PlayGlobalSound->func_002E6C28;
 * active-topic mirror D_25C940->D_25C960; string-id table D_1ABDC0->D_1ABE28 (s16); active-topic
 * string id D_25C8C4->D_25C8E4. REGION: 18 topics (0..0x11) in both. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", UpdateHelpTopicMenuInput);
#else
s32 UpdateHelpTopicMenuInput(void) {
    extern void func_002E6C28(s32 id, s32 a, s32 b);   /* PlayGlobalSound */
    extern s32  g_padButtonsPressed;
    extern s32  D_1ABB00;                 /* help topic cursor 0..0x11 (gp) */
    extern s32  D_25C960;                 /* active topic index mirror */
    extern s16  D_1ABE28[];               /* per-topic string-id table (halfwords) */
    extern s32  D_25C8E4;                 /* active topic's string id */
    u8 *mb = D_001F0000 + 0x2840;
    s32 buttons = g_padButtonsPressed;
    s32 result = 0;
    s32 cursor;

    if (buttons & 0x10) {               /* back */
        s32 e0 = *(s32 *)(*(u8 **)(mb + 0x14) + 0xE0);
        D_1ABB00 = 0;
        if (e0 != 0) {
            *(s32 *)(mb + 0x18) = e0;
            return 0;
        }
        return (*(s32 *)(mb + 0x134) == 0) ? -1 : 0;
    }

    if (buttons & 0x900) {              /* cancel */
        D_1ABB00 = 0;
        result = 1;
    } else if (buttons & 0x8000) {      /* previous topic */
        s32 v;
        if (D_1ABB00 > 0) {
            func_002E6C28(3, 0, 0);
        } else {
            func_002E6C28(5, 0, 0);
        }
        v = D_1ABB00 - 1;
        D_1ABB00 = (v >= 0) ? v : 0;
    } else if (buttons & 0x2000) {      /* next topic */
        s32 v;
        if (D_1ABB00 < 0x11) {
            func_002E6C28(3, 0, 0);
        } else {
            func_002E6C28(5, 0, 0);
        }
        v = D_1ABB00 + 1;
        D_1ABB00 = (v < 0x12) ? v : 0x11;
    }

    cursor = D_1ABB00;
    D_25C960 = cursor;
    D_25C8E4 = D_1ABE28[cursor];
    return result;
}
#endif

/* DrawHelpTopicMenu: EU twin of USA DrawHelpTopicMenu — renders the help-topics screen chrome:
 * three header glyphs (0xDE/0xDF/0xE0), the left/right paging arrows (left when not on the first
 * page, right when not on the last page 0x11), and a localized footer string. Matching arm stays
 * INCLUDE_ASM (packed-save + FP-arg scheduling); #else is the structure model. Word-verified vs
 * USA DrawHelpTopicMenu: func_003017F8->func_00301AC0; GuiFontAtlasLookupGlyph->func_00338AA8;
 * DrawBestiaryPagingArrows->DrawBestiaryPagingArrows; func_002801B8->func_00280050; g_screenWidth->
 * *(s32*)(D_001A7308+0xB8); row D_1ABA9C->D_1ABB04 (gp); y-fudge->g_nVendorBuyQuantity+0x160;
 * cursor->D_1ABB00 (gp). REGION: last page 0x11 in both. PAL footer 0x2BE5->0xB60. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawHelpTopicMenu);
#else
s32 DrawHelpTopicMenu(void) {
    extern void func_00301AC0(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                              f32 px, f32 py, f32 sx, f32 syg, f32 v38);
    extern s32  func_00338AA8(void *atlas, s32 codepoint);
    extern void DrawBestiaryPagingArrows(s32 leftEnabled, s32 rightEnabled);
    extern void func_00280050(s32 x, s32 y, u64 color, char *str, s64 sel);
    extern s32  D_1ABB00;                 /* help-topic cursor (gp) */
    extern s32  D_1ABB04;                 /* glyph row (int -> float, gp) */
    extern u8   D_001A7308[];              /* screen-width word @ +0xB8 */
    extern u8   g_nVendorBuyQuantity[];   /* sprite y-fudge @ +0x160 (f32) */
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(*(s32 *)(D_001A7308 + 0xB8) / 2);
    f32 row = (f32)D_1ABB04;
    f32 yfudge = *(f32 *)(g_nVendorBuyQuantity + 0x160);
    s32 cursor;
    char *text;

    func_0027CA28(0);
    func_00301AC0(func_00338AA8(atlas, 0xDE), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xDF), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xE0), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    cursor = D_1ABB00;
    DrawBestiaryPagingArrows(cursor != 0, cursor != 0x11);
    text = GetLocalizedString(0xB60);
    func_00280050(0x1B0, 0x177, 0x80F0F0F0, text, -1);
    func_0027CB48();
    return 0;
}
#endif

/* return 0 stub. */
s32 func_002D2F48(void) {
    return 0;
}

/* func_002D2F50: EU twin of USA func_002D2FC8 — options sub-screen input handler. Confirm(0x10)
 * latches the screen result; cancel(0x900) returns 1; up(0x8000)/down(0x2000) move a 6-entry
 * cursor with edge sounds; a changed cursor latches error -0x12C, then mirrors the cursor and
 * stores the s16 table entry. Matching arm stays INCLUDE_ASM (packed-save + reload scheduling);
 * #else is the structure model. Word-verified vs USA func_002D2FC8: g_padButtonsPressed kept;
 * cursor->D_1ABB08 (gp); g_menuScreenBlock->D_001F0000+0x2840; PlayGlobalSound->func_002E6C28;
 * error/state block D_25CA80->D_0025CAA0; cursor mirror D_25CB30->D_25CB50; s16 entry table
 * D_1ABDEA->D_1ABE52. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D2F50);
#else
s32 func_002D2F50(void) {
    extern void func_002E6C28(s32 id, s32 a, s32 b);   /* PlayGlobalSound */
    extern s32  g_padButtonsPressed;
    extern s32  D_1ABB08;                 /* options sub cursor (gp) */
    extern u8   D_0025CAA0[];             /* USA D_25CA80 */
    extern s32  D_25CB50;                 /* USA D_25CB30 */
    extern u8   D_1ABE52[];               /* s16 entries on a 4-byte stride */
    s32 flags = g_padButtonsPressed;
    s32 old = D_1ABB08;
    s32 result = 0;
    s32 cur;

    if (flags & 0x10) {
        u8 *mb = D_001F0000 + 0x2840;
        s32 v;
        D_1ABB08 = 0;
        v = *(s32 *)(*(u8 **)(mb + 0x14) + 0xE0);
        if (v != 0) {
            *(s32 *)(mb + 0x18) = v;
            return 0;
        }
        if (*(s32 *)(mb + 0x134) == 0) {
            return -1;
        }
        return 0;
    }

    if (flags & 0x900) {
        D_1ABB08 = 0;
        result = 1;
    } else if (flags & 0x8000) {
        func_002E6C28(old <= 0 ? 5 : 3, 0, 0);
        cur = D_1ABB08 - 1;
        D_1ABB08 = cur;
        if (cur < 0) {
            D_1ABB08 = 0;
        }
    } else if (flags & 0x2000) {
        func_002E6C28(old < 5 ? 3 : 5, 0, 0);
        cur = D_1ABB08 + 1;
        D_1ABB08 = cur;
        if (cur >= 6) {
            D_1ABB08 = 5;
        }
    }

    if (old != D_1ABB08) {
        *(s32 *)(D_0025CAA0 + 0x3C) = -0x12C;
    }
    cur = D_1ABB08;
    D_25CB50 = cur;
    *(s32 *)(D_0025CAA0 + 0x34) = *(s16 *)(D_1ABE52 + cur * 4);
    return result;
}
#endif

/* func_002D30C0: EU twin of USA func_002D3138 — options-menu screen draw. Three header glyphs,
 * the left/right paging arrows keyed on the option cursor (0..5), a vertical slider fill whose
 * endpoints come from the slider fraction scaled by 306/308, then the current option's localized
 * label (screen-centred) and the localized footer. Matching arm stays INCLUDE_ASM (packed-save +
 * FP-arg scheduling); #else is the structure model. Word-verified vs USA func_002D3138:
 * func_003017F8->func_00301AC0; GuiFontAtlasLookupGlyph->func_00338AA8; DrawBestiaryPagingArrows->
 * DrawBestiaryPagingArrows; func_002904B0->func_002904C8; func_002801B8->func_00280050; g_screenWidth->
 * *(s32*)(D_001A7308+0xB8); row D_1ABAA4->D_1ABB0C (gp); cursor->D_1ABB08 (gp); y-fudge->
 * g_nVendorBuyQuantity+0x160; slider frac->g_nVendorBuyQuantity+0x15C; option-label table
 * D_1ABDE8->D_1ABE50 (s16 stride 4). GENUINE REGION DIFF: PAL slider vertical endpoints scale by
 * 306.0f/308.0f (NTSC 330.0f/332.0f) — followed from the EU asm (0x43990000/0x439A0000). PAL
 * footer 0x2BE5->0xB60. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D30C0);
#else
s32 func_002D30C0(void) {
    extern void func_00301AC0(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                              f32 px, f32 py, f32 sx, f32 syg, f32 v38);
    extern s32  func_00338AA8(void *atlas, s32 codepoint);
    extern void DrawBestiaryPagingArrows(s32 leftEnabled, s32 rightEnabled);
    extern void func_002904C8(s32 x0, s32 y0, s32 x1, s32 y1, s32 color, s32 flag);
    extern void func_00280050(s32 x, s32 y, u64 color, char *str, s64 sel);
    extern s32  D_1ABB08;                 /* options cursor (gp) */
    extern s32  D_1ABB0C;                 /* glyph row (int -> float, gp) */
    extern u8   D_1ABE50[];               /* per-option label ids, s16 stride 4 */
    extern u8   D_001A7308[];              /* screen-width word @ +0xB8 */
    extern u8   g_nVendorBuyQuantity[];   /* +0x160 y-fudge, +0x15C slider frac (f32) */
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(*(s32 *)(D_001A7308 + 0xB8) / 2);
    f32 row = (f32)D_1ABB0C;
    f32 yfudge = *(f32 *)(g_nVendorBuyQuantity + 0x160);
    f32 frac = *(f32 *)(g_nVendorBuyQuantity + 0x15C);
    s32 cursor;
    char *text;

    func_0027CA28(0);
    func_00301AC0(func_00338AA8(atlas, 0xDE), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xDF), 0x60241700, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xE0), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    cursor = D_1ABB08;
    DrawBestiaryPagingArrows(cursor != 0, cursor != 5);
    func_002904C8(0x42, (s32)(frac * 306.0f + 0.5f), 0x1CF,
                  (s32)(frac * 308.0f + 0.5f), 0x55F0C070, 0);
    text = GetLocalizedString(*(s16 *)(D_1ABE50 + D_1ABB08 * 4));
    func_00280050(*(s32 *)(D_001A7308 + 0xB8) / 2, 0x137, 0x80F0F0F0, text, -1);
    text = GetLocalizedString(0xB60);
    func_00280050(0x1B0, 0x177, 0x80F0F0F0, text, -1);
    func_0027CB48();
    return 0;
}
#endif

/* GUI null-check gate: if the GUI is up, latch error code -0x12C; return 0. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_002D3388: g_guiInstance
 * keeps its name; D_25CABC -> D_25CADC (abs %hi/%lo). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3310);
#else
s32 func_002D3310(void) {
    extern s32 D_25CADC;                 /* USA D_25CABC */
    if (g_guiInstance) {
        D_25CADC = -0x12C;
    }
    return 0;
}
#endif

/* Help-topic sub-browser input handler (EU twin of USA func_002D33A8). Confirm (0x10) latches the
 * screen's pending result and resets the 7-page cursor to 0. Cancel (0x900) returns 1. Up (0x8000)
 * decrements the cursor clamped at 0; Down (0x2000) increments clamped at 6; each move plays sound
 * 3 (moved) / 5 (edge). The landed page is mirrored into D_25CCE8. Returns confirm/cancel tri-state.
 * Matching arm stays INCLUDE_ASM (packed-save + branch-likely); #else is the structure model.
 * Word-verified vs USA func_002D33A8: g_menuScreenBlock->D_001F0000+0x2840; g_helpPageCursor->
 * D_1ABB10 (gp); D_25CCC8->D_25CCE8 (abs); PlayGlobalSound->func_002E6C28. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3330);
#else
s32 func_002D3330(void) {
    extern void func_002E6C28(s32 id, s32 a, s32 b);   /* USA PlayGlobalSound */
    extern s32  g_padButtonsPressed;
    extern s32  D_1ABB10;                              /* USA g_helpPageCursor (gp) */
    extern s32  D_25CCE8;                              /* USA D_25CCC8 (abs)        */
    s32 flags = g_padButtonsPressed;
    s32 result = 0;
    s32 page;

    if (flags & 0x10) {
        s32 *block = (s32 *)(D_001F0000 + 0x2840);
        s32 v;
        D_1ABB10 = 0;
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
        D_1ABB10 = 0;
        result = 1;
        page = D_1ABB10;
        D_25CCE8 = page;
        return result;
    }

    if (flags & 0x8000) {
        s32 cur = D_1ABB10 - 1;
        D_1ABB10 = cur;
        func_002E6C28(cur < 0 ? 5 : 3, 0, 0);
        cur = D_1ABB10;
        if (cur < 0) {
            D_1ABB10 = 0;
        }
    } else if (flags & 0x2000) {
        s32 cur = D_1ABB10 + 1;
        D_1ABB10 = cur;
        func_002E6C28(cur < 7 ? 3 : 5, 0, 0);
        cur = D_1ABB10;
        if (cur >= 7) {
            D_1ABB10 = 6;
        }
    }

    page = D_1ABB10;
    D_25CCE8 = page;
    return result;
}
#endif

/* Help/hint-page screen draw (EU twin of USA func_002D34E8): two header glyphs (0xE5/0xE4,
 * centred at row D_1ABB14), then left/right page arrows gated by the page cursor D_1ABB10 (0..6):
 * "prev" glyph 0x4B at (216,373) when cursor>0, "next" glyph 0x4A at (295,373) when cursor<6, each
 * white while its pad dir is held (L1 0x8000 / R1 0x2000) else the pulsing inactive colour from
 * func_002A9FA0. Title + footer centred. Returns 0. Matching arm stays INCLUDE_ASM (packed-save +
 * FP-arg scheduling); #else is the structure model. Word-verified vs USA func_002D34E8:
 * func_003017F8->func_00301AC0, GuiFontAtlasLookupGlyph->func_00338AA8, func_002AA3F0->func_002A9FA0,
 * func_0027FBA8->func_0027FA40, func_00280090->func_0027FF28, func_002801B8->func_00280050;
 * g_screenWidth->*(s32*)(D_001A7308+0xB8); y-fudge->g_nVendorBuyQuantity+0x160; row D_1ABAAC->
 * D_1ABB14 (gp); cursor->D_1ABB10 (gp); g_padButtonsHeld kept. PAL string ids: 0x2DD6->0xE65,
 * 0x2DD7->0xE66, 0x312A->0x1B62, 0x2BE5->0xB60. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3470);
#else
s32 func_002D3470(void) {
    extern void func_00301AC0(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                              f32 px, f32 py, f32 sx, f32 syg, f32 v38);
    extern s32  func_00338AA8(void *atlas, s32 codepoint);
    extern u32  func_002A9FA0(u32 color1, u32 color2, s32 period, s32 counterSel, s32 reset);
    extern void func_0027FA40(s32 x, s32 y, u64 color, char *str, s64 wrap);  /* USA func_0027FBA8 */
    extern void func_00280050(s32 x, s32 y, u64 color, char *str, s64 sel);   /* USA func_002801B8 */
    extern s32  D_1ABB10;                          /* USA g_helpPageCursor (gp)   */
    extern s32  D_1ABB14;                          /* USA D_1ABAAC glyph row (gp)  */
    extern s32  g_padButtonsHeld;
    extern u8   D_001A7308[];                       /* screen-width word @ +0xB8   */
    extern u8   g_nVendorBuyQuantity[];             /* sprite y-fudge @ +0x160 (f32)*/
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(*(s32 *)(D_001A7308 + 0xB8) / 2);
    f32 row = (f32)D_1ABB14;
    f32 yfudge = *(f32 *)(g_nVendorBuyQuantity + 0x160);
    u32 inactiveArrow;
    s32 cursor;
    char *text;

    func_0027CA28(0);
    func_00301AC0(func_00338AA8(atlas, 0xE5), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    func_00301AC0(func_00338AA8(atlas, 0xE4), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.0f);
    inactiveArrow = func_002A9FA0(0x60241700, 0x55F0C070, 0x19, 0, 0);

    cursor = D_1ABB10;
    if (cursor > 0) {
        u32 color = (g_padButtonsHeld & 0x8000) ? 0x80F0F0F0 : inactiveArrow;
        func_00301AC0(func_00338AA8(atlas, 0x4B), color, (f32 *)0, (f32 *)0,
                      216.0f, 373.0f, 1.0f, yfudge * 0.8f, 0.0f);
        text = GetLocalizedString(0xE65);
        func_0027FA40(0x40, 0x171, 0x80F0F0F0, text, -1);
        cursor = D_1ABB10;
    }
    if (cursor < 6) {
        u32 color = (g_padButtonsHeld & 0x2000) ? 0x80F0F0F0 : inactiveArrow;
        func_00301AC0(func_00338AA8(atlas, 0x4A), color, (f32 *)0, (f32 *)0,
                      295.0f, 373.0f, 1.0f, yfudge * 0.8f, 0.0f);
        text = GetLocalizedString(0xE66);
        func_0027FF28(0x1C1, 0x171, 0x80F0F0F0, text, -1);
    }

    text = GetLocalizedString(0x1B62);
    func_00280050(*(s32 *)(D_001A7308 + 0xB8) / 2, 0x1D, 0x80F0F0F0, text, -1);
    text = GetLocalizedString(0xB60);
    func_00280050(*(s32 *)(D_001A7308 + 0xB8) / 2, 0x173, 0x80F0F0F0, text, -1);
    func_0027CB48();
    return 0;
}
#endif

/* return 0 stub. */
s32 func_002D3768(void) {
    return 0;
}

/* Planet-warp / cinematic-camera confirm input handler (EU twin of USA func_002D37E8). Triangle
 * (0x10): standard menu back/confirm latch off the active screen instance (block[0x14]+0xE0).
 * {L1|R1}=0x900: returns 1. X (0x40): plays confirm SFX 4; if a cinematic camera is active (D_1A798C)
 * tears it down (clears the two sound-bank cinematic-channel words), else arms it and seeds the
 * cinematic camera vector via func_002837E0(dst, dst-0x14E0, 1.0f). Matching arm stays INCLUDE_ASM
 * (beql latch + VU0/FP scheduling); #else is the structure model. Word-verified vs USA func_002D37E8:
 * g_menuScreenBlock->D_001F0000+0x2840; D_1A790C->D_1A798C; PlayGlobalSound->func_002E6C28;
 * Vec3RescaleToLenVu0->func_002837E0; sound-bank stores off g_sndChannelVolumes+0x1778, seeded
 * camera vec off g_nNanotechBonusHealTimer+0x1304 (offsets 0x14F8/0x1500, -0x14E0 src identical). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3770);
#else
s32 func_002D3770(void) {
    extern void func_002E6C28(s32 id, s32 a, s32 b);          /* USA PlayGlobalSound       */
    extern void func_002837E0(void *dst, void *src, f32 len); /* USA Vec3RescaleToLenVu0   */
    extern s32  g_padButtonsPressed;
    extern s32  D_1A798C;                                     /* USA D_1A790C latch        */
    extern u8   g_sndChannelVolumes[];                        /* USA g_soundBankHandlesBlk */
    extern u8   g_nNanotechBonusHealTimer[];                  /* USA g_cinematicCameraBlock*/
    s32 flags = g_padButtonsPressed;
    s32 result = 0;

    if (flags & 0x10) {
        s32 *block = (s32 *)(D_001F0000 + 0x2840);
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
        func_002E6C28(4, 0, 0);
        if (D_1A798C != 0) {
            u8 *snd = g_sndChannelVolumes + 0x1778;
            D_1A798C = 0;
            *(s32 *)(snd + 0x14F8) = 0;
            *(s32 *)(snd + 0x1500) = 0;
        } else {
            u8 *cam = g_nNanotechBonusHealTimer + 0x1304;
            D_1A798C = 1;
            func_002837E0(cam, cam - 0x14E0, 1.0f);
        }
    }
    return result;
}
#endif

/* Summary/confirmation screen draw (EU twin of USA func_002D38C8): three header glyphs (0x8B/0x8C/
 * 0x8D, centred at row D_1ABB18), three centred lines, then two composed lines built with the SDK
 * sprintf (func_00115DA8): the left formats D_1ABA60 with localized 0x1408 (drawn via func_0027FA40),
 * the right formats D_1ABAA8 with a string chosen by the cinematic latch D_1A798C (0xBD7 set / 0xBD8
 * clear; drawn via func_00280050). Both composed draws pass strlen (func_001157AC) as the clip arg.
 * Matching arm stays INCLUDE_ASM (packed-save + FP-arg scheduling); #else is the structure model.
 * Word-verified vs USA func_002D38C8: func_003017F8->func_00301AC0, GuiFontAtlasLookupGlyph->
 * func_00338AA8, func_002801B8->func_00280050, func_0027FBA8->func_0027FA40; g_screenWidth->
 * *(s32*)(D_001A7308+0xB8); y-fudge->g_nVendorBuyQuantity+0x160; row D_1ABAB0->D_1ABB18 (gp); fmt
 * D_1AB9F8->D_1ABA60, D_1ABA38->D_1ABAA8; latch D_1A790C->D_1A798C. PAL string ids: 0x30D5->0x1408,
 * 0x2BE4->0xB5F, 0x2BE5->0xB60, 0x2C5C->0xBD7, 0x2C5D->0xBD8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3850);
#else
s32 func_002D3850(void) {
    extern void func_00301AC0(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                              f32 px, f32 py, f32 sx, f32 syg, f32 v38);
    extern s32  func_00338AA8(void *atlas, s32 codepoint);
    extern void func_00280050(s32 x, s32 y, u64 color, char *str, s64 sel);   /* USA func_002801B8 */
    extern void func_0027FA40(s32 x, s32 y, u64 color, char *str, s64 wrap);  /* USA func_0027FBA8 */
    extern void func_00115DA8(char *dst, const char *fmt, ...);               /* SDK sprintf */
    extern s32  func_001157AC(const char *s);                                 /* strlen      */
    extern s32  D_1ABB18;                          /* USA D_1ABAB0 glyph row (gp)  */
    extern char D_1ABA60[];                         /* USA D_1AB9F8 fmt (left)      */
    extern char D_1ABAA8[];                         /* USA D_1ABA38 fmt (right)     */
    extern s32  D_1A798C;                            /* USA D_1A790C latch          */
    extern u8   D_001A7308[];                        /* screen-width word @ +0xB8   */
    extern u8   g_nVendorBuyQuantity[];              /* sprite y-fudge @ +0x160 (f32)*/
    void *atlas = g_guiInstance + 0x8710;
    f32 centerX = (f32)(*(s32 *)(D_001A7308 + 0xB8) / 2);
    f32 row = (f32)D_1ABB18;
    f32 yfudge = *(f32 *)(g_nVendorBuyQuantity + 0x160);
    char buf[0x40];
    char *text;

    func_0027CA28(0);
    func_00301AC0(func_00338AA8(atlas, 0x8B), 0x60442D00, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.56f);
    func_00301AC0(func_00338AA8(atlas, 0x8C), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.56f);
    func_00301AC0(func_00338AA8(atlas, 0x8D), 0x55F0C070, (f32 *)0, (f32 *)0,
                  centerX, row, 1.0f, yfudge, 0.56f);

    text = GetLocalizedString(0x1408);
    func_00280050(*(s32 *)(D_001A7308 + 0xB8) / 2, 0x41, 0x80F0F0F0, text, -1);
    text = GetLocalizedString(0xB5F);
    func_00280050(*(s32 *)(D_001A7308 + 0xB8) / 2, 0x141, 0x80F0F0F0, text, -1);
    text = GetLocalizedString(0xB60);
    func_00280050(*(s32 *)(D_001A7308 + 0xB8) / 2, 0x15A, 0x80F0F0F0, text, -1);

    func_00115DA8(buf, D_1ABA60, GetLocalizedString(0x1408));
    func_0027FA40(0x86, 0xBA, 0x80F0F0F0, buf, func_001157AC(buf));

    func_00115DA8(buf, D_1ABAA8, GetLocalizedString(D_1A798C != 0 ? 0xBD7 : 0xBD8));
    func_00280050(0x15D, 0xBA, 0x80F0F0F0, buf, func_001157AC(buf));

    func_0027CB48();
    return 0;
}
#endif

/* Forward the currently-pressed pad buttons to the menu input handler; return 0 (EU twin of USA
 * func_002D3B88). Matching arm stays INCLUDE_ASM (SN %hi-hoist scheduling); #else is the structure
 * model. Word-verified vs USA func_002D3B88: func_0029D478->func_0029CFD8; g_padButtonsPressed kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3B10);
#else
s32 func_002D3B10(void) {
    extern void func_0029CFD8(s32 padPressed);   /* USA func_0029D478 */
    extern s32  g_padButtonsPressed;
    func_0029CFD8(g_padButtonsPressed);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3B38(void) {
    func_0027CA28(0);
    func_0029CFA8();
    func_0027CB48();
    return 0;
}

/* Menu confirm/cancel poll variant (EU twin of USA func_002D3BE0): confirm bit (0x10) latches the
 * active screen's pending result into block[0x18] (or -1 when no pending sub-result); cancel (0x900)
 * returns 1; otherwise ticks idle handler func_0029D048 and returns 0. Matching arm stays INCLUDE_ASM
 * (packed-save + branch-likely); #else is the structure model. Word-verified vs USA func_002D3BE0:
 * g_menuScreenBlock->D_001F0000+0x2840; func_0029D4E8->func_0029D048; g_padButtonsPressed kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3B68);
#else
s32 func_002D3B68(void) {
    extern s32  func_0029D048(s32 padPressed);   /* USA func_0029D4E8 */
    extern s32  g_padButtonsPressed;
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029D048(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3BF0(void) {
    func_0027CA28(0);
    func_0029D018();
    func_0027CB48();
    return 0;
}

/* Cinematics/cutscene confirm poll (EU twin of USA func_002D3C98): on confirm (0x10) and cancel
 * (0x900) it "unlocks" the current cutscene record (struct at g_health+0x464): increments the
 * play-count at +0x1D4 (saturating at 0xFFFF), raises the high-water mark at +0x1D8 toward a
 * frame-count target, and sets the bit for the current g_playerProgress slot (plus the 0x80000000
 * sentinel) in the seen-mask at +0x1DC. Confirm returns the active screen's pending result tri-state;
 * cancel returns 1; the idle path ticks func_0029D088 and returns 0. Matching arm stays INCLUDE_ASM
 * (packed-save + bnel dead-store shape); #else is the structure model. Word-verified vs USA
 * func_002D3C98: g_menuScreenBlock->D_001F0000+0x2840; func_0029D528->func_0029D088; g_health /
 * g_playerProgress kept. GENUINE PAL DIFF: EU scales the frame-count source by 5/6 (50/60 Hz) —
 * target=(field*5+2)/6 — where field = D_001A7308+0x108 (USA g_gsPixelOffsetY+0x3C, +0x80 lane);
 * USA reads that field RAW. NOTE: USA #else stores target+1 to +0x1D4, but both regions' asm
 * self-increment rec[0x1D4]+1 — modeled faithfully here (USA #else should be corrected). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3C20);
#else
static void MenuCutsceneUnlockCurrent(void) {
    extern s32 g_health;
    extern u8  D_001A7308[];        /* frame-count source @ +0x108 (USA g_gsPixelOffsetY+0x3C) */
    extern s32 g_playerProgress;
    u8 *rec = (u8 *)&g_health + 0x464;
    s32 target = (*(s32 *)(D_001A7308 + 0x108) * 5 + 2) / 6;   /* EU PAL 5/6 scale */
    if (*(u16 *)(rec + 0x1D4) <= 0xFFFE) {
        *(u16 *)(rec + 0x1D4) += 1;
    }
    if (*(s32 *)(rec + 0x1D8) < target) {
        *(s32 *)(rec + 0x1D8) = target;
    }
    *(u32 *)(rec + 0x1DC) |= (1u << g_playerProgress) | 0x80000000u;
}
s32 func_002D3C20(void) {
    extern s32 func_0029D088(s32 padPressed);   /* USA func_0029D528 */
    extern s32 g_padButtonsPressed;
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029D088(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3DB8(void) {
    func_0027CA28(0);
    func_0029D0C8();
    func_0027CB48();
    return 0;
}

/* Menu confirm/cancel poll (EU twin of USA func_002D3E08): confirm (0x10) latches the active
 * screen's pending result into block[0x18] (or -1 when none); cancel (0x900) returns 1; else ticks
 * idle handler func_0029D0F8 and returns 0. Matching arm stays INCLUDE_ASM (packed-save + branch-
 * likely); #else is the structure model. Word-verified vs USA func_002D3E08: g_menuScreenBlock->
 * D_001F0000+0x2840; idle func_0029D598->func_0029D0F8 (-0x4A0); g_padButtonsPressed kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3DE8);
#else
s32 func_002D3DE8(void) {
    extern s32  func_0029D0F8(s32 padPressed);   /* USA func_0029D598 */
    extern s32  g_padButtonsPressed;
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029D0F8(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3E70(void) {
    func_0027CA28(0);
    func_0029D138();
    func_0027CB48();
    return 0;
}

/* Menu confirm/cancel poll (EU twin of USA func_002D3EC0): confirm (0x10) latches the active
 * screen's pending result; cancel (0x900) returns 1; else ticks idle handler func_0029D168 and
 * returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_002D3EC0: g_menuScreenBlock->D_001F0000+0x2840; idle func_0029D608->func_0029D168 (-0x4A0);
 * g_padButtonsPressed kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3EA0);
#else
s32 func_002D3EA0(void) {
    extern s32  func_0029D168(s32 padPressed);   /* USA func_0029D608 */
    extern s32  g_padButtonsPressed;
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029D168(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3F28(void) {
    func_0027CA28(0);
    func_0029D1A8();
    func_0027CB48();
    return 0;
}

/* Menu confirm/cancel poll (EU twin of USA func_002D3F78): confirm (0x10) latches the active
 * screen's pending result; cancel (0x900) returns 1; else ticks idle handler func_0029D398 and
 * returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_002D3F78: g_menuScreenBlock->D_001F0000+0x2840; idle func_0029D838->func_0029D398 (-0x4A0);
 * g_padButtonsPressed kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D3F58);
#else
s32 func_002D3F58(void) {
    extern s32  func_0029D398(s32 padPressed);   /* USA func_0029D838 */
    extern s32  g_padButtonsPressed;
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029D398(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D3FE0(void) {
    func_0027CA28(0);
    func_0029D3D8();
    func_0027CB48();
    return 0;
}

/* Menu confirm/cancel poll (EU twin of USA func_002D4030): confirm (0x10) latches the active
 * screen's pending result; cancel (0x900) returns 1; else ticks idle handler func_0029D328 and
 * returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_002D4030: g_menuScreenBlock->D_001F0000+0x2840; idle func_0029D7C8->func_0029D328 (-0x4A0);
 * g_padButtonsPressed kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4010);
#else
s32 func_002D4010(void) {
    extern s32  func_0029D328(s32 padPressed);   /* USA func_0029D7C8 */
    extern s32  g_padButtonsPressed;
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029D328(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4098(void) {
    func_0027CA28(0);
    func_0029D368();
    func_0027CB48();
    return 0;
}

/* Menu confirm/cancel poll (EU counterpart of USA func_002D40E8): confirm (0x10) latches the
 * active screen's pending result; cancel (0x900) returns 1; else ticks idle handler func_0029D1D8
 * and returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * TWIN NOTE: the twin table tentatively paired this with USA func_002D41B8, but by position + idle-
 * handler delta this is the EU counterpart of USA func_002D40E8 (func_002D4180 is the true
 * func_002D41B8 twin). GENUINE REGION DIFF: USA func_002D40E8 runs a deferred game-state action on
 * the idle path (if func_0029D678(flags)!=0 -> MenuScreenDoAction(0xD,0,&outFlag)); the EU build
 * DROPS it entirely (EU asm: plain jal func_0029D1D8; size 0x84/frame 0x10 vs USA 0x9C/0x20).
 * Word-verified vs USA func_002D40E8: g_menuScreenBlock->D_001F0000+0x2840; idle func_0029D678->
 * func_0029D1D8 (-0x4A0); g_padButtonsPressed kept; MenuScreenDoAction path absent in EU. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D40C8);
#else
s32 func_002D40C8(void) {
    extern s32  func_0029D1D8(s32 padPressed);   /* USA func_0029D678 */
    extern s32  g_padButtonsPressed;
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029D1D8(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4150(void) {
    func_0027CA28(0);
    func_0029D218();
    func_0027CB48();
    return 0;
}

/* Menu confirm/cancel poll (EU twin of USA func_002D41B8): confirm (0x10) latches the active
 * screen's pending result; cancel (0x900) returns 1; else ticks idle handler func_0029D248 and
 * returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_002D41B8: g_menuScreenBlock->D_001F0000+0x2840; idle func_0029D6E8->func_0029D248 (-0x4A0);
 * g_padButtonsPressed kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4180);
#else
s32 func_002D4180(void) {
    extern s32  func_0029D248(s32 padPressed);   /* USA func_0029D6E8 */
    extern s32  g_padButtonsPressed;
    s32 flags = g_padButtonsPressed;
    s32 *block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029D248(flags);
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4208(void) {
    func_0027CA28(0);
    func_0029D288();
    func_0027CB48();
    return 0;
}

/* Menu confirm/cancel poll + command builder (EU twin of USA func_002D4270). Confirm (0x10) latches
 * the active screen's pending result; cancel (0x900) returns 1; else ticks idle handler func_0029D2B8
 * and, on the confirm pad bit (0x40) with the GUI up, reads the selected entry of the list widget at
 * g_guiInstance+0x3F4A0 (func_003402B8) and builds a command record (op=entry[0x8], arg=entry[0xC])
 * handed to func_002D6A70. Matching arm stays INCLUDE_ASM (packed-save + branch-likely); #else is the
 * structure model. Word-verified vs USA func_002D4270: D_138180->D_138200; g_menuScreenBlock->
 * D_001F0000+0x2840; idle func_0029D758->func_0029D2B8 (-0x4A0); func_0033F360->func_003402B8;
 * func_002D6B00->func_002D6A70; GUI widget offset 0x3F3F0->0x3F4A0 (+0xB0); g_guiInstance kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4238);
#else
s32 func_002D4238(void) {
    extern u8    D_138200[];                        /* USA D_138180 */
    extern s32   func_0029D2B8(s32 padPressed);     /* USA func_0029D758 */
    extern void *func_003402B8(void *widget);       /* USA func_0033F360 */
    extern void  func_002D6A70(void *record);       /* USA func_002D6B00 */
    s32 flags = *(s32 *)(D_138200 + 0x1C4);
    s32 *block;
    s32 v;
    if (flags & 0x10) {
        block = (s32 *)(D_001F0000 + 0x2840);
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
    func_0029D2B8(flags);
    if ((*(s32 *)(D_138200 + 0x1C4) & 0x40) && g_guiInstance) {
        u8 record[0x30];
        u8 *entry = (u8 *)func_003402B8(g_guiInstance + 0x3F4A0);
        *(u16 *)(record + 0x2) = *(u16 *)(entry + 0x8);
        *(s32 *)(record + 0x4) = *(s32 *)(entry + 0xC);
        func_002D6A70(record);
    }
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4308(void) {
    func_0027CA28(0);
    func_0029D2F8();
    func_0027CB48();
    return 0;
}

/* GUI wrapper (EU twin of USA func_002D4370): when the GUI is up, forward the list widget at
 * g_guiInstance+0x3F4A0 to func_00340310(&D_261568). Returns 0. Matching arm stays INCLUDE_ASM
 * (near-miss address-build scheduling); #else is the structure model. Word-verified vs USA
 * func_002D4370: func_0033F3B8->func_00340310; D_2617C0->D_261568; GUI widget offset 0x3F3F0->
 * 0x3F4A0 (+0xB0); g_guiInstance kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4338);
#else
s32 func_002D4338(void) {
    extern void func_00340310(char *widget, void *arg);   /* USA func_0033F3B8 */
    extern u8 D_261568;                                   /* USA D_2617C0 */
    if (g_guiInstance) {
        func_00340310(g_guiInstance + 0x3F4A0, &D_261568);
    }
    return 0;
}
#endif

/* Confirm/cancel poll driven by the global input flag word (D_138200[0x1C4]). Confirm (0x10) latches
 * the active screen's pending result; cancel (0x900) returns 1; otherwise ticks the idle handler
 * func_0029CF68 with D_138200[0x1C0] and returns 0. Matching arm stays INCLUDE_ASM (8-byte-packed-save
 * wall); #else is the structure model. Word-verified vs USA func_002D43B0: g_menuScreenBlock->
 * D_001F0000+0x2840; D_138180->D_138200 (+0x80); idle func_0029D408->func_0029CF68. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4378);
#else
s32 func_002D4378(void) {
    extern u8 D_138200[];                 /* USA D_138180 */
    extern void func_0029CF68(s32 arg);   /* USA func_0029D408 */
    s32 flags = *(s32 *)(D_138200 + 0x1C4);
    u8 *block;
    s32 v;

    if (flags & 0x10) {
        block = D_001F0000 + 0x2840;      /* USA g_menuScreenBlock */
        v = *(s32 *)(*(u8 **)(block + 0x14) + 0xE0);
        if (v != 0) {
            *(s32 *)(block + 0x18) = v;
            return 0;
        }
        if (*(s32 *)(block + 0x134) == 0) {
            return -1;
        }
        return 0;
    }
    if (flags & 0x900) {
        return 1;
    }
    func_0029CF68(*(s32 *)(D_138200 + 0x1C0));
    return 0;
}
#endif

/* Draw-batch wrapper. */
s32 func_002D4400(void) {
    func_0027CA28(0);
    func_0029CF38();
    func_0027CB48();
    return 0;
}

/* GUI wrapper: when the GUI is up, configure the list widget at instance+0x3C530 — func_00345628(w,0)
 * (mode), func_003455B0(w, &D_00259F58) (bind data), func_003455F8(w) (rebuild), func_00345618(w,
 * &D_0025D0E0) (bind labels). Returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002D4468: GUI offset 0x3C480->0x3C530 (+0xB0); func_003444D0/00344458/
 * 003444A0/003444C0->func_00345628/003455B0/003455F8/00345618; D_259F38->D_00259F58 (+0x20);
 * D_25D0C0->D_0025D0E0 (+0x20); g_guiInstance kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4430);
#else
s32 func_002D4430(void) {
    extern void func_00345628(void *widget, s32 mode);
    extern void func_003455B0(void *widget, void *data);
    extern void func_003455F8(void *widget);
    extern void func_00345618(void *widget, void *labels);
    extern u8 D_00259F58, D_0025D0E0;

    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C530;
        func_00345628(w, 0);
        func_003455B0(w, &D_00259F58);
        func_003455F8(w);
        func_00345618(w, &D_0025D0E0);
    }
    return 0;
}
#endif

/* GUI wrapper: twin of func_002D4430 for the same widget (instance+0x3C530) with the alternate
 * mode/data/labels (func_00345628(w,1), &D_259CE0, &D_0025D288). Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_002D44E8: GUI offset 0x3C480->0x3C530 (+0xB0);
 * func_003444D0/00344458/003444A0/003444C0->func_00345628/003455B0/003455F8/00345618; D_259CC0->
 * D_259CE0 (+0x20); D_25D268->D_0025D288 (+0x20); g_guiInstance kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D44B0);
#else
s32 func_002D44B0(void) {
    extern void func_00345628(void *widget, s32 mode);
    extern void func_003455B0(void *widget, void *data);
    extern void func_003455F8(void *widget);
    extern void func_00345618(void *widget, void *labels);
    extern u8 D_259CE0, D_0025D288;

    if (g_guiInstance) {
        char *w = g_guiInstance + 0x3C530;
        func_00345628(w, 1);
        func_003455B0(w, &D_259CE0);
        func_003455F8(w);
        func_00345618(w, &D_0025D288);
    }
    return 0;
}
#endif

/* Refresh a weapon-swap widget's two caption/price fields from the currently selected weapon. With
 * the GUI up, lays out the swap gadget (func_003435D0 on g_guiInstance+0x3C210), reads the selected
 * weapon slot for the gadget at g_guiInstance+0x3C530 (slot = g_itemEquippedSlot[func_003455D8(gadget)])
 * and stores that weapon's g_weaponTable field +0x42 into the widget arg's +0x34; then fetches the
 * gadget's sub-widget (func_00345620) and stores the selected weapon's g_weaponTable field +0x6 into
 * that sub-widget's +0x58. Returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002D4568: func_00342460->func_003435D0; func_003444C8->func_00345620; GUI
 * offsets 0x3C160->0x3C210 and 0x3C480->0x3C530 (+0xB0); g_itemEquippedSlot/g_weaponTable kept. REGION
 * NOTE: EU keeps the index-reader forwarder func_003455D8(gadget) (no +0x2C8 sub-offset) where USA's
 * #else unwrapped it to func_00343AD0(gadget+0x2C8) — same selected slot; modeled to the EU asm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4530);
#else
s32 func_002D4530(void *widget) {
    extern void func_003435D0(void *widget, s32 arg);   /* USA func_00342460 */
    extern s32  func_003455D8(void *gadget);            /* selected-slot index reader */
    extern s32  func_00345620(void *gadget);            /* USA func_003444C8 (sub-widget getter) */
    extern u8   g_itemEquippedSlot[];
    extern u8   g_weaponTable[];
    u8 *gadget;
    u8 *subWidget;
    s32 slot;

    if (g_guiInstance == NULL)
        return 0;

    func_003435D0(g_guiInstance + 0x3C210, 1);

    gadget = (u8 *)g_guiInstance + 0x3C530;
    slot = g_itemEquippedSlot[func_003455D8(gadget)];
    *(s32 *)((u8 *)widget + 0x34) = *(s16 *)(g_weaponTable + slot * 0xE0 + 0x42);

    subWidget = (u8 *)func_00345620(gadget);
    slot = g_itemEquippedSlot[func_003455D8(gadget)];
    *(s32 *)(subWidget + 0x58) = *(s16 *)(g_weaponTable + slot * 0xE0 + 0x6);
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", UpdateShipCustomizeInput);

/* Draws a two-segment horizontal gradient bar (an audio/level meter) for the D_26CD90[idx] record.
 * Each segment's fill width comes from `val` offset by +2 and +0x28: width = 0x80 - toInt(|256 -
 * (val+off)| * 0.75), clamped >= 0, packed as the high (alpha) byte of colour 0x00F0F0B0.
 * func_0028F2D8 renders the quad (corners alternate the two colours) into the element handle from
 * func_0028EE08(0xEA97, record[0x10]). The magnitude/scale math runs through the SDK soft-double
 * helpers. Matching arm stays INCLUDE_ASM (soft-float call scheduling + 9-GPR packed save); #else is
 * the structure model. Word-verified vs USA func_002D4D38: meter table D_26CFD0->D_26CD90 (-0x240);
 * func_0028EDF0->func_0028EE08; func_0028F2C0->func_0028F2D8; the soft-double helpers are SDK-region
 * and byte-identical (same addresses in both regions). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", func_002D4D00);
#else
#define METER_FILL_SCALE 0x3FE8000000000000LL   /* 0.75 (built ori 0xFFA0; dsll32 14) */

/* fill width for one meter segment: 0x80 - round(|256 - v| * 0.75), soft-double. */
static s32 MeterSegmentWidth(s32 v) {
    extern s64 func_001234F0(float f);
    extern s32 func_00123028(s64 a, s64 b);
    extern s64 func_00122A98(s64 a, s64 b);
    extern s64 func_00122B00(s64 a, s64 b);
    extern s32 func_00123130(s64 x);
    s64 d = func_001234F0(256.0f - (f32)v);
    if (func_00123028(d, 0) < 0)
        d = func_00122A98(0, d);
    return 0x80 - func_00123130(func_00122B00(d, METER_FILL_SCALE));
}

void func_002D4D00(s32 idx, s32 val) {
    extern s32 func_0028EE08(s32 u, s32 v);
    extern void func_0028F2D8(s32 handle, s32 x0, s32 y0, s32 x1, s32 y1, void *quad);
    extern u8 D_26CD90[];    /* meter record table, stride 0x14 (USA D_26CFD0) */
    u8 *rec = D_26CD90 + idx * 0x14;
    s32 handle = func_0028EE08(0xEA97, *(s32 *)(rec + 0x10));
    s32 w1 = MeterSegmentWidth(val + 2);
    s32 w2 = MeterSegmentWidth(val + 0x28);
    u32 color1, color2;
    s32 quad[4];

    if (w2 < 0) w2 = 0;
    if (w1 < 0) w1 = 0;
    color1 = ((u32)w1 << 24) | 0xF0F0B0;
    color2 = ((u32)w2 << 24) | 0xF0F0B0;
    quad[0] = color1;
    quad[1] = color2;
    quad[2] = color1;
    quad[3] = color2;
    func_0028F2D8(handle, val + 2, 0x14E, 0x28, 0x28, quad);
}
#undef METER_FILL_SCALE
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1C9F58", DrawShipCustomizeMenu);

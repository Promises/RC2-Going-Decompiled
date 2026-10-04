#include "common.h"

/*
 * text/198B58 — EU (SCES_516.07) port of the USA text/198FA0 save-game +
 * GUI-manager wrapper unit. The C bodies are region-agnostic; only the
 * referenced global/function symbol NAMES differ (different EU addresses).
 * objdiff masks the gp/reloc deltas, so the same source shapes match here.
 *
 * Built (per-unit) at -O2 -G8 -fno-gcse, same recipe as USA text/198FA0:
 * see the GFLAG/CC1EXTRA override in tools/ee/objdiff_build.sh. The -G8
 * extern-sizing + `.extern sym,16` absolute-macro model is carried over
 * verbatim (the codegen wall it reproduces is region-independent).
 *
 * Functions still left INCLUDE_ASM here are exactly the same ones walled in
 * USA text/198FA0 (reload-artifact / register-coloring / scc-tail /
 * callee-save-packing walls — see the USA unit's per-function comments), plus
 * the dead inter-function fill fragments. Every function USA matches (82) also
 * matches here at the EU address: the GUI widget offsets inside the GuiManager
 * instance are shifted +0xB0 in EU (the wrapper bodies carry the EU offsets),
 * and the dialog-TOC / loaded-model globals resolve to different EU symbols,
 * but the C shapes are identical. No genuine region-divergence was found in
 * this unit's matched set — func_00299538 (USA func_00299980), flagged as a
 * possible mid-body-count divergence, was verified byte-identical and matches.
 */

/* Original cc1-small / assembler-absolute symbols (EU addresses). */
__asm__(".extern g_guiInstance, 16");
__asm__(".extern D_1A7C18, 16");
__asm__(".extern g_nSaveLoadStatusCode, 16");
__asm__(".extern D_1A8D14, 16");
__asm__(".extern D_0014B5C0, 16");
__asm__(".extern D_001A7308, 16");

extern char *g_guiInstance;                   /* singleton GUI-manager instance */
extern s32 D_1A7C18;                           /* PAL video-mode flag (USA g_bPalMode) */
extern s32 D_1A798C;                           /* small-data flag cleared by func_0029BFC0 */
extern s32 D_1A9B10;                           /* small-data GUI state, set to -1 by func_0029C630 */
extern char D_0014B5C0;                        /* level dialog/scene TOC block base (byte-addressed) */
extern char D_001A7308;                        /* loaded armor/held-item model id pair base */
extern s32 g_nBoltCounterDisplayed;            /* base for the save-prompt latch equate below */

/* Save/load status pair: [0] popup status code, [1] pending-action flag word. */
extern s32 g_nSaveLoadStatusCode[2];
extern s32 D_1A8D14;                           /* GUI popup-busy gate */

/* Two card-error gate words in the save-prompt small-data block (EU twins of
 * USA D_1A8C88 / D_1A8C8C; read by the save/load status handlers below).
 *   D_1A8D38 : set on a hard/unrecoverable card error (USA D_1A8C88)
 *   D_1A8D3C : set when a card-removal abort is in progress (USA D_1A8C8C)
 * (Declared for the TARGET_NATIVE #else arms only — emit no code.) */
#ifdef TARGET_NATIVE
extern s32 D_1A8D38;
extern s32 D_1A8D3C;
#endif

/* Save/load engine context (EU D_139460). Fields recovered from USA unit. */
typedef struct SaveLoadContext {
    s32 unk0[2];        /* 0x000 */
    s32 phase;          /* 0x008 */
    s32 unkC;           /* 0x00C */
    s32 busy;           /* 0x010 */
    s32 unk14;          /* 0x014 */
    s16 slot;           /* 0x018 */
    s16 unk1A;          /* 0x01A */
    s32 unk1C[0x4A];    /* 0x01C */
    s32 unk144;         /* 0x144 */
    s32 unk148;         /* 0x148 */
    s32 unk14C[3];      /* 0x14C */
    s32 reqState;       /* 0x158 */
    s32 mode;           /* 0x15C */
    s32 unk160;         /* 0x160 */
    s32 result;         /* 0x164 */
    s32 subResult;      /* 0x168 */
    s32 unk16C;         /* 0x16C */
    s32 unk170[3];      /* 0x170 */
    s32 dirty;          /* 0x17C */
} SaveLoadContext;
extern SaveLoadContext D_139460;

/* Alias view of D_139460.busy (0x139470). */
extern s32 D_139470[4];
extern char D_1A9AE0[];   /* memory-card init failure debug string */
extern s32 McInit(void);
extern s32 func_0026FD28(char *msg);           /* DebugPrintStub (EU) */
extern void func_0028E9B8(s32 arg);
extern s32 func_0029C640(void);
extern s32 func_0033B7D0(void *widget, s32 arg);

typedef struct SaveSection {
    void *srcPtr;
    s32   len;
    s32   tag;
    s32   matchResult; /* 0xC deserializer reconcile result: 0=unmatched, 1=exact, -1=image-shorter, -2=image-longer */
} SaveSection;

/* GUI widget methods (EU addresses). All value-returning. */
extern s32 func_00350660(void *widget);
extern s32 func_003506E0(void *widget);
extern s32 func_0034B088(void *widget);
extern s32 UpdatePopupMenu(void *popup, s32 arg);
extern s32 func_00349AB8(void *widget);
extern s32 func_00348728(void *widget);
extern s32 GuiWeaponGridTick(void *widget, s32 arg);
extern s32 GuiMapScreenTick(void *widget, s32 buttons, s32 *out);
extern s32 GuiConfirmPopupTick(void *widget, s32 arg);
extern s32 GuiConfirmPopupDraw(void *widget);
extern s32 GuiLevelInfoPanelTick(void *widget, s32 arg);
extern s32 func_0033C308(void *widget);
extern s32 func_0033D838(void *widget);
extern s32 func_0033D468(void *widget, s32 arg);
extern s32 func_00341020(void *widget);
extern s32 func_00340A50(void *widget, s32 arg);
extern s32 func_00347438(void *widget);
extern s32 func_00346528(void *widget, s32 arg);
extern s32 func_00343C88(void *widget);
extern s32 func_00343928(void *widget, s32 arg);
extern s32 func_00345940(void *widget);
extern s32 func_003457F0(void *widget, s32 arg);
extern s32 func_00340610(void *widget);
extern s32 func_003405B0(void *widget, s32 arg);
extern s32 func_0033F840(void *widget);
extern s32 func_0033F738(void *widget, s32 arg);
extern s32 func_0033E460(void *widget, s32 arg);
extern s32 func_0033E608(void *widget);
extern s32 func_0033E9E8(void *widget, s32 arg);
extern s32 func_0033EAF0(void *widget);
extern s32 func_0033F470(void *widget, s32 arg);
extern s32 func_0033F508(void *widget);
extern s32 func_0033DDC0(void *widget, s32 arg);
extern s32 func_0033DF90(void *widget);
extern s32 func_0033E160(void *widget, s32 arg);
extern s32 func_0033E248(void *widget);
extern s32 func_00340348(void *widget, s32 arg);
extern s32 func_00340418(void *widget);
extern s32 func_0033EEF8(void *widget, s32 arg);
extern s32 func_0033F190(void *widget);
extern s32 func_0033FB00(void *widget, s32 arg);
extern s32 func_0033FBE0(void *widget);
extern s32 func_00344528(void *widget, s32 arg);
extern s32 func_003447C0(void *widget);
extern s32 func_00343F50(void *widget, s32 arg);
extern s32 func_00343FA0(void *widget);
extern s32 UpdateBoltCounterHud(void);
extern s32 func_0028B038(void);
extern s32 func_0034F060(void *widget, s32 value);
extern s32 GuiQuickSelectWheelTick(void *widget, s32 arg);
extern s32 func_003422D0(void *widget);
extern s32 func_0033A4A0(void *widget, s32 arg0, s32 arg1, s32 arg2);
extern s32 func_0033A528(void *widget, s32 arg0, s32 arg1);
extern s32 func_00339DC8(void *widget, s32 arg0, s32 arg1);
extern s32 func_00337AA0(s32 palMode);
extern s32 func_0034EFF0(void *widget, s32 arg);
extern s32 func_0034ECB0(void *widget, s32 arg0, s32 arg1);
extern s32 GuiScreenSetEventAndReveal(void *screen, s32 eventId);
extern s32 func_003506A0(void *widget);

/** Reset the save/load popup to status 3 (idle/none) and clear the context
 *  transaction-active + save-pending flags. */
void func_00298BD8(void) {
    g_nSaveLoadStatusCode[0] = 3;
    D_139460.dirty = 0;
    D_139460.busy = 0;
}

/** Save/load top-level status arbiter (EU twin of USA func_00299040). Always
 *  consumes the pending-flag's 0x2 and 0x4 bits first. Then: if no save is
 *  pending (dirty +0x17C == 0) -> status 3. Otherwise route the original flags:
 *  bit 0x80 (or secondary-path +0x16C) -> status 0x15 (consume 0x80, set 0x40);
 *  bit 0x100 -> status 0x14 (consume 0x100, set 0x40); else if a card
 *  transaction is active (busy != 0) clear the pending flag and, on a hard/abort
 *  card error (D_1A8D38/D_1A8D3C) show status 3, else show status 2 (set bit
 *  0x1); else (idle) bit 0x200 -> status 0x19 (consume 0x200).
 *  Matching arm stays INCLUDE_ASM; #else is the structure model.
 *  Word-verified vs USA func_00299040: D_1393E0->D_139460, D_1A8C88->D_1A8D38
 *  (gp_rel here), D_1A8C8C->D_1A8D3C (hi/lo here); g_nSaveLoadStatusCode kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298BF8);
#else
void func_00298BF8(void) {
    s32 flags = g_nSaveLoadStatusCode[1];
    s32 cleared = flags & ~0x6;
    g_nSaveLoadStatusCode[1] = cleared;
    if (D_139460.dirty == 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if ((flags & 0x80) || D_139460.unk16C != 0) {
        g_nSaveLoadStatusCode[0] = 0x15;
        g_nSaveLoadStatusCode[1] = (cleared & ~0x80) | 0x40;
        return;
    }
    if (flags & 0x100) {
        g_nSaveLoadStatusCode[0] = 0x14;
        g_nSaveLoadStatusCode[1] = (cleared ^ 0x100) | 0x40;
        return;
    }
    if (D_139460.busy != 0) {
        D_139460.dirty = 0;
        if (D_1A8D38 != 0 || D_1A8D3C != 0) {
            g_nSaveLoadStatusCode[0] = 3;
        } else {
            g_nSaveLoadStatusCode[0] = 2;
            g_nSaveLoadStatusCode[1] = cleared | 0x1;
        }
    } else if (flags & 0x200) {
        g_nSaveLoadStatusCode[0] = 0x19;
        g_nSaveLoadStatusCode[1] = cleared ^ 0x200;
    }
}
#endif

/** If no save/load action is pending (flag bit 0 clear), reset the popup
 *  status to 3 (idle). */
void func_00298CE0(void) {
    /* `(flags ^ 1) & 1` is bit0==0; spelled this way to reproduce the
     * original xori/andi evaluation order. */
    if ((g_nSaveLoadStatusCode[1] ^ 1) & 1) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** Mark the card transaction results as pending (-1) and show popup status 4. */
void func_00298D08(void) {
    D_139460.result = -1;
    g_nSaveLoadStatusCode[0] = 4;
    D_139460.subResult = -1;
}

/** Save-prompt step (EU twin of USA func_00299178): clear the pending-flag's
 *  0x20 bit, then if a card transaction is in flight (phase==2) translate the
 *  libmc busy-result into a popup status. busy 0 -> status 9; busy -1 -> ack
 *  (busy=0) + status 9; busy -2 -> status 5.
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298D30);
#else
void func_00298D30(void) {
    g_nSaveLoadStatusCode[1] &= ~0x20;
    if (D_139460.phase == 2) {
        s32 busy = D_139460.busy;
        if (busy == 0) {
            g_nSaveLoadStatusCode[0] = 9;
        } else if (busy == -1) {
            D_139460.busy = 0;
            g_nSaveLoadStatusCode[0] = 9;
        } else if (busy == -2) {
            g_nSaveLoadStatusCode[0] = 5;
        }
    }
}
#endif

/** Card-removal step (EU twin of USA func_002991E8): if no abort is in flight
 *  (busy != -2) show status 3. Otherwise, if a hard card error is latched
 *  (D_1A8D3C) show status 6; if the pending-flag's 0x2 bit is set show status 6.
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298DA0);
#else
void func_00298DA0(void) {
    s32 cardErr = D_1A8D3C;
    if (D_139470[0] != -2) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if (cardErr != 0) {
        g_nSaveLoadStatusCode[0] = 6;
    } else if (g_nSaveLoadStatusCode[1] & 0x2) {
        g_nSaveLoadStatusCode[0] = 6;
    }
}
#endif

/** Card-abort step (EU twin of USA func_00299238): if no abort is in flight
 *  (busy != -2) show status 3. Otherwise dispatch on the pending-flag word:
 *  bit 0x20 -> if a hard card error is latched (D_1A8D38) clear it and show
 *  status 0x17, else show status 5; bit 0x8 -> clear it and show status 7.
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298DF0);
#else
void func_00298DF0(void) {
    s32 flags = g_nSaveLoadStatusCode[1];
    if (D_139470[0] != -2) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if (flags & 0x20) {
        /* The 0x20 bit is cleared on BOTH the error and no-error paths: the
         * original writes it back in the branch delay slot before testing
         * D_1A8D38, so the clear happens regardless of the error result. */
        g_nSaveLoadStatusCode[1] = flags ^ 0x20;
        if (D_1A8D38 != 0) {
            g_nSaveLoadStatusCode[0] = 0x17;
        } else {
            g_nSaveLoadStatusCode[0] = 5;
        }
    } else if (flags & 0x8) {
        g_nSaveLoadStatusCode[1] = flags ^ 0x8;
        g_nSaveLoadStatusCode[0] = 7;
    }
}
#endif

/** Result-timeout step (EU twin of USA func_002992B8): always clear the busy
 *  flag; then if the libmc result is still pending (result < 0) force result 3
 *  + subResult 0. Always show status 8 (formatting/working).
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298E70);
#else
void func_00298E70(void) {
    D_139460.busy = 0;
    if (D_139460.result < 0) {
        D_139460.subResult = 0;
        D_139460.result = 3;
    }
    g_nSaveLoadStatusCode[0] = 8;
}
#endif

/** Format-confirm step (EU twin of USA func_002992E8): when a format request
 *  (mode 2) is still pending (result < 0): if the secondary/format path flag
 *  (unk16C) is set, show status 0x11 and set pending-flag bit 0x40; otherwise
 *  show status 0xE.
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298EA0);
#else
void func_00298EA0(void) {
    if (D_139460.mode == 2 && D_139460.result < 0) {
        if (D_139460.unk16C != 0) {
            g_nSaveLoadStatusCode[0] = 0x11;
            g_nSaveLoadStatusCode[1] |= 0x40;
        } else {
            g_nSaveLoadStatusCode[0] = 0xE;
        }
    }
}
#endif

/** Load-prompt step (EU twin of USA func_00299348): if no transaction is
 *  active (busy==0) dispatch on the pending-flag word: bits 0x6 -> status 0xA;
 *  else bit 0x200 -> status 0x19. If a transaction is active show status 3.
 *  (Walled by the reload-artifact named in the file header.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298F00);
#else
void func_00298F00(void) {
    s32 flags = g_nSaveLoadStatusCode[1];
    if (D_139470[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if (flags & 0x6) {
        g_nSaveLoadStatusCode[0] = 0xA;
    } else if (flags & 0x200) {
        g_nSaveLoadStatusCode[0] = 0x19;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298F50);

/** Save/load step (EU twin of USA func_002993D8): only while a card transaction
 *  finished selecting (mode 2, result < 0): secondary-path flag (+0x16C) set ->
 *  status 0xC; else a deep abort (busy < -1) -> status 3; else if the active
 *  slot is the sentinel -2, pick status 0x13 (or 0xC when unkC + unk1C[1]
 *  reaches 0x1DB); else (slot >= -1) -> status 0x10.
 *  Matching arm stays INCLUDE_ASM; #else is the structure model.
 *  Word-verified vs USA func_002993D8: D_1393E0->D_139460; g_nSaveLoadStatusCode
 *  kept. Fields mode 0x15C/result 0x164/unk16C 0x16C/busy 0x10/slot 0x18/
 *  unkC 0xC/unk1C[1] 0x20. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298F90);
#else
void func_00298F90(void) {
    if (D_139460.mode != 2 || D_139460.result >= 0) {
        return;
    }
    if (D_139460.unk16C != 0) {
        g_nSaveLoadStatusCode[0] = 0xC;
        return;
    }
    if (D_139460.busy < -1) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    if (D_139460.slot == -2) {
        if (D_139460.unkC + D_139460.unk1C[1] < 0x1DB) {
            g_nSaveLoadStatusCode[0] = 0x13;
        } else {
            g_nSaveLoadStatusCode[0] = 0xC;
        }
    } else if (D_139460.slot >= -1) {
        g_nSaveLoadStatusCode[0] = 0x10;
    }
}
#endif

/** Save/load status predicate (EU twin of USA func_00299478): while a card
 *  transaction is active (D_139470/busy != 0) show status 3; otherwise, if the
 *  pending-flag's 0x2 bit is set, show status 0xD.
 *  Matching arm stays INCLUDE_ASM; #else is the structure model.
 *  Word-verified vs USA func_00299478: D_1393F0->D_139470; g_nSaveLoadStatusCode
 *  kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299030);
#else
void func_00299030(void) {
    if (D_139470[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
    } else if (g_nSaveLoadStatusCode[1] & 0x2) {
        g_nSaveLoadStatusCode[0] = 0xD;
    }
}
#endif

/** Save/load status dispatch (EU twin of USA func_002994B0): while a card
 *  transaction is active (D_139470/busy != 0) show status 3. Otherwise route on
 *  the pending-flag word: bit 0x20 -> clear it, then if a hard card error is
 *  latched (D_1A8D38) show status 0x18 else status 0xC; bit 0x10 -> clear it and
 *  show status 0xE.
 *  Matching arm stays INCLUDE_ASM; #else is the structure model.
 *  Word-verified vs USA func_002994B0: D_1393F0->D_139470, D_1A8C88->D_1A8D38
 *  (hi/lo); g_nSaveLoadStatusCode kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299068);
#else
void func_00299068(void) {
    s32 flags;
    if (D_139470[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x20) {
        g_nSaveLoadStatusCode[1] = flags ^ 0x20;   /* consume bit 0x20 */
        if (D_1A8D38 != 0) {
            g_nSaveLoadStatusCode[0] = 0x18;
        } else {
            g_nSaveLoadStatusCode[0] = 0xC;
        }
    } else if (flags & 0x10) {
        g_nSaveLoadStatusCode[1] = flags ^ 0x10;   /* consume bit 0x10 */
        g_nSaveLoadStatusCode[0] = 0xE;
    }
}
#endif

/** If a card transaction finished selecting (mode 2) with no result yet,
 *  force result 9 (cancelled) and show popup status 0xF. */
void func_002990E0(void) {
    if (D_139460.mode == 2 && D_139460.result < 0) {
        D_139460.result = 9;
        D_139460.subResult = 0;
        g_nSaveLoadStatusCode[0] = 0xF;
    }
}

/** Save/load step (EU twin of USA func_00299568): only while a card transaction
 *  finished selecting (mode 2, result < 0): if the secondary-path flag (+0x16C)
 *  is set, show status 0x12 and set the pending-flag's 0x40 bit; otherwise commit
 *  result 7 / subResult 0, show status 0x16, and reset the transaction (unk148 +
 *  slot cleared).
 *  Matching arm stays INCLUDE_ASM; #else is the structure model.
 *  Word-verified vs USA func_00299568: D_1393E0->D_139460; g_nSaveLoadStatusCode
 *  kept. Fields mode 0x15C/result 0x164/unk16C 0x16C/subResult 0x168/
 *  unk148 0x148/slot 0x18. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299120);
#else
void func_00299120(void) {
    if (D_139460.mode != 2 || D_139460.result >= 0) {
        return;
    }
    if (D_139460.unk16C != 0) {
        g_nSaveLoadStatusCode[0] = 0x12;
        g_nSaveLoadStatusCode[1] |= 0x40;
    } else {
        D_139460.result = 7;
        D_139460.subResult = 0;
        g_nSaveLoadStatusCode[0] = 0x16;
        D_139460.unk148 = 0;
        D_139460.slot = 0;
    }
}
#endif

/** Save/load status (EU twin of USA func_002995E0): first consume the
 *  pending-flag's 0x4 and 0x2 bits if set. Then route on the remaining flags:
 *  bit 0x80 -> status 0x15 (consume 0x80, set 0x40); bit 0x100 -> status 0x14
 *  (consume 0x100, set 0x40); else if a card transaction is active (busy != 0)
 *  -> status 3; else if the save-pending flag (dirty +0x17C) is set -> status 1.
 *  Matching arm stays INCLUDE_ASM; #else is the structure model.
 *  Word-verified vs USA func_002995E0: D_1393E0->D_139460; g_nSaveLoadStatusCode
 *  kept. Fields busy 0x10/dirty 0x17C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299198);
#else
void func_00299198(void) {
    s32 flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x4) {
        g_nSaveLoadStatusCode[1] = flags & ~0x4;
    }
    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x2) {
        g_nSaveLoadStatusCode[1] = flags & ~0x2;
    }
    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x80) {
        g_nSaveLoadStatusCode[0] = 0x15;
        g_nSaveLoadStatusCode[1] = (flags ^ 0x80) | 0x40;
    } else if (flags & 0x100) {
        g_nSaveLoadStatusCode[0] = 0x14;
        g_nSaveLoadStatusCode[1] = (flags ^ 0x100) | 0x40;
    } else if (D_139460.busy != 0) {
        g_nSaveLoadStatusCode[0] = 3;
    } else if (D_139460.dirty != 0) {
        g_nSaveLoadStatusCode[0] = 1;
    }
}
#endif

/** If no save/load confirmation is pending (flag bit 6 clear), reset the popup
 *  status to 3 (idle). First of four identical per-call-site stubs. */
void func_00299250(void) {
    if (!(g_nSaveLoadStatusCode[1] & 0x40)) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** Identical twin of func_00299698 (separate call-site stub). */
void func_00299278(void) {
    if (!(g_nSaveLoadStatusCode[1] & 0x40)) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** If a card transaction is active (D_139470, the context busy flag), reset
 *  the popup status to 3 (idle). */
void func_002992A0(void) {
    if (D_139470[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** Identical twin of func_00299698 (separate call-site stub). */
void func_002992C0(void) {
    if (!(g_nSaveLoadStatusCode[1] & 0x40)) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** Identical twin of func_00299698 (separate call-site stub). */
void func_002992E8(void) {
    if (!(g_nSaveLoadStatusCode[1] & 0x40)) {
        g_nSaveLoadStatusCode[0] = 3;
    }
}

/** Save/load step (EU twin of USA func_00299758): only while a card transaction
 *  finished selecting (mode 2, result < 0): if the secondary-path flag (+0x16C)
 *  or the retry slot (unk1C[2] +0x24) is set, clear the save-pending flag
 *  (dirty +0x17C), show status 0x15 and set the pending-flag's 0x440 bits;
 *  otherwise mark save-pending (dirty +0x17C = 1) and show status 1.
 *  Matching arm stays INCLUDE_ASM; #else is the structure model.
 *  Word-verified vs USA func_00299758: D_1393E0->D_139460; g_nSaveLoadStatusCode
 *  kept. Fields mode 0x15C/result 0x164/unk16C 0x16C/unk1C[2] 0x24/dirty 0x17C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299310);
#else
void func_00299310(void) {
    if (D_139460.mode != 2 || D_139460.result >= 0) {
        return;
    }
    if (D_139460.unk16C != 0 || D_139460.unk1C[2] != 0) {
        g_nSaveLoadStatusCode[0] = 0x15;
        D_139460.dirty = 0;
        g_nSaveLoadStatusCode[1] |= 0x440;
    } else {
        g_nSaveLoadStatusCode[0] = 1;
        D_139460.dirty = 1;
    }
}
#endif

/** Save-complete handler (EU twin of USA func_002997C8). Only while a card
 *  transaction finished (mode 2, result < 0). Nothing pending (secondary-path
 *  +0x16C == 0 and not busy) -> mark save-pending (dirty +0x17C = 1) and show
 *  status 1. Secondary path (+0x16C != 0) -> clear the pending flag, show status
 *  0x15, drive a game-state change to the save/load screen
 *  (RequestGameStateChange(4, g_nGameState == 0 ? 1 : 2, 1, 0, 0)); if already in
 *  the level-exit state (g_nGameState == 6) flush the pending prompt
 *  (SetSavePromptPending); finally show status 2. Else (busy, no secondary path)
 *  -> consume the pending-flag's 0x40 bit, set bit 0x1, show status 2.
 *  Matching arm stays INCLUDE_ASM; #else is the structure model.
 *  Word-verified vs USA func_002997C8: D_1393E0->D_139460, func_00289798->
 *  SetSavePromptPending (EU-named); g_nGameState / RequestGameStateChange kept;
 *  g_nSaveLoadStatusCode kept. Fields mode 0x15C/result 0x164/unk16C 0x16C/
 *  busy 0x10/dirty 0x17C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", UpdateSaveTaskState);
#else
extern s32 g_nGameState;
extern void RequestGameStateChange(s32 newState, s32 argA, s32 argB, s32 argC, s32 argD);
extern void SetSavePromptPending(void);
void UpdateSaveTaskState(void) {
    if (D_139460.mode != 2 || D_139460.result >= 0) {
        return;
    }
    if (D_139460.unk16C == 0 && D_139460.busy == 0) {
        D_139460.dirty = 1;
        g_nSaveLoadStatusCode[0] = 1;
        return;
    }
    D_139460.dirty = 0;
    if (D_139460.unk16C != 0) {
        g_nSaveLoadStatusCode[0] = 0x15;
        g_nSaveLoadStatusCode[1] = 0x40;
        if (g_nGameState == 0) {
            RequestGameStateChange(4, 1, 1, 0, 0);
        } else {
            RequestGameStateChange(4, 2, 1, 0, 0);
        }
        if (g_nGameState == 6) {
            SetSavePromptPending();
        }
        g_nSaveLoadStatusCode[0] = 2;
    } else {
        g_nSaveLoadStatusCode[1] = (g_nSaveLoadStatusCode[1] & ~0x40) | 0x1;
        g_nSaveLoadStatusCode[0] = 2;
    }
}
#endif

/* Save/load status (EU twin of USA func_002998D0): while a card-removal abort
 * is NOT in flight (D_139470/busy != -2) show status 3; otherwise consume the
 * pending-flag's 0x20 bit and show status 5. Matching arm stays INCLUDE_ASM;
 * #else is the structure model.
 * Word-verified vs USA func_002998D0: D_1393F0[0] -> D_139470[0] (alias of
 * D_139460.busy); g_nSaveLoadStatusCode kept (named). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299488);
#else
void func_00299488(void) {
    s32 flags;
    if (D_139470[0] != -2) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x20) {
        g_nSaveLoadStatusCode[1] = flags ^ 0x20;   /* consume bit 0x20 */
        g_nSaveLoadStatusCode[0] = 5;
    }
}
#endif

/* Save/load status (EU twin of USA func_00299918): while a card transaction is
 * active (D_139470/busy != 0) show status 3; otherwise consume the pending-flag's
 * 0x20 bit and show status 0xC. Matching arm stays INCLUDE_ASM; #else is the
 * structure model.
 * Word-verified vs USA func_00299918: D_1393F0[0] -> D_139470[0]; g_nSaveLoadStatusCode
 * kept (named). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_002994D0);
#else
void func_002994D0(void) {
    s32 flags;
    if (D_139470[0] != 0) {
        g_nSaveLoadStatusCode[0] = 3;
        return;
    }
    flags = g_nSaveLoadStatusCode[1];
    if (flags & 0x20) {
        g_nSaveLoadStatusCode[1] = flags ^ 0x20;   /* consume bit 0x20 */
        g_nSaveLoadStatusCode[0] = 0xC;
    }
}
#endif

/* func_00299518: EU twin of USA func_00299960 (delta -0x448, consistent across
 * this whole trio: peek/consume/status = 0x299518/0x299520/0x299538 against USA
 * 0x299960/0x299968/0x299980). 2-insn leaf `return g_savePromptLatch;`.
 * UNMATCHABLE for the same reason as USA: the original reads the latch with a
 * single %gp_rel($gp) load in the jr delay slot, while cc1 under this unit's
 * -G0 model emits the absolute lui/lw pair for a normal extern. That is a
 * byte-MATCHING wall and does not block a faithful portable arm. The ROM arm is
 * untouched; the arm below is NOT a byte-match claim. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299518);
#else
/* Declared again here because the ROM arm's declaration + gas equate sit a few
 * lines BELOW this guard, and the INCLUDE_ASM must not be moved (its position
 * fixes this function's placement in .text). Scoped to the portable arm. */
extern s32 g_savePromptLatch;

/** Peek the latched save-prompt flag without consuming it.
 *  Companion to func_00299520, which reads-and-clears the same word; this one
 *  only reads, so repeated calls keep observing a pending prompt.
 *
 *  NOTE the two regions reach the same word through DIFFERENT base symbols:
 *  USA spells it g_pRainHeightmap+0x1C, EU g_nBoltCounterDisplayed+0xF4. Both
 *  are splat attributing one bss word to a nearby symbol; the gas equate below
 *  is what makes the name agree. A symbol+offset pair is not an identity --
 *  only the resolved address is.
 *  @return the raw latch value (non-zero while a prompt is armed). */
s32 func_00299518(void) {
    return g_savePromptLatch;
}
#endif

/* Latched event flag at g_nBoltCounterDisplayed+0xF4 (EU equivalent of the
 * USA g_pSkyShellSpinRates+0xAC latch): an unrelated bss word; aliased via a
 * gas symbol equate because no symbol exists at that address. The equate is
 * EE-only: under native PIC the GOT load of an undefined-symbol-plus-offset
 * alias cannot be relocated, and the native link must give the latch its own
 * storage instead. */
extern s32 g_savePromptLatch;
#ifndef TARGET_NATIVE
__asm__("g_savePromptLatch = g_nBoltCounterDisplayed+0xF4");
#endif

/** Consume the latched flag: read it, clear it, return whether it was set. */
s32 func_00299520(void) {
    s32 was = g_savePromptLatch;
    g_savePromptLatch = 0;
    return was != 0;
}

/** True when the save/load popup is showing status 2 (card-access prompt). */
s32 func_00299538(void) {
    return g_nSaveLoadStatusCode[0] == 2;
}

/* BuildSaveGamePaths(cfg): build the per-slot memory-card path strings into the
 * bss path buffers. Reads the region code at cfg+0x12, patches g_saveDirTemplate[2],
 * copies device-id bytes into the dir template, then makes six func_00115AC0
 * (dst, &g_saveDirTemplate, 0xD) formatter calls. Matching arm stays INCLUDE_ASM;
 * #else is the structure model.
 * Word-verified vs USA BuildSaveGamePaths: interior slots D_1A7950 -> D_1A79D0 and
 * D_1A79A8 -> D_1A7A28 (+0x80 data lane); g_saveDirTemplate/g_saveIconSysPath/
 * g_saveStaticIcoPath/g_saveFileFmt + func_00115AC0 kept (named). Byte-copy loops
 * (3..6 / 8..0xA / 0xB..0xC) identical.
 * REGION DIFFERENCE: EU asm has ONLY the 'E'->'E' and 'P'->'I' cases; the USA build
 * has a third 'K'->'K' (Korean) case (USA .s: bne against 0x4B at 0x2999C4). The
 * PAL build dropped the 'K' region-code path, so the EU switch has two cases. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", BuildSaveGamePaths);
#else
extern u8 g_saveDirTemplate[];
extern u8 g_saveIconSysPath[];
extern u8 g_saveStaticIcoPath[];
extern u8 g_saveFileFmt[];
extern u8 D_1A79D0[]; /* interior dir-path slot (EU +0x80 of USA D_1A7950) */
extern u8 D_1A7A28[]; /* interior save/icon path slots (EU +0x80 of USA D_1A79A8) */
extern void func_00115AC0(void *dst, const void *src, s32 len); /* SDK formatter */
void BuildSaveGamePaths(u8 *cfg) {
    s32 i;

    switch (cfg[0x12]) {
    case 'E':
        g_saveDirTemplate[2] = 'E';
        break;
    case 'P':
        g_saveDirTemplate[2] = 'I';
        break;
    }

    for (i = 3; i < 7; i++) {
        g_saveDirTemplate[i] = cfg[i + 0xD];
    }
    for (i = 8; i < 0xB; i++) {
        g_saveDirTemplate[i] = cfg[i + 0xD];
    }
    for (i = 0xB; i < 0xD; i++) {
        g_saveDirTemplate[i] = cfg[i + 0xE];
    }

    func_00115AC0(D_1A79D0, g_saveDirTemplate, 0xD);
    func_00115AC0(g_saveIconSysPath, g_saveDirTemplate, 0xD);
    func_00115AC0(g_saveStaticIcoPath, g_saveDirTemplate, 0xD);
    func_00115AC0(D_1A7A28, g_saveDirTemplate, 0xD);
    func_00115AC0(D_1A7A28 + 0x14, g_saveDirTemplate, 0xD);
    func_00115AC0(g_saveFileFmt, g_saveDirTemplate, 0xD);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_002996A8);

/* func_002996C0(image): restore a save image (EU twin of USA func_00299B18).
 * Validates the image's two leading size words against the live section-table
 * sizes (global then area); on a mismatch logs D_1A9A28 and bails. Otherwise skips
 * the size header, deserializes the global block (slot 0) then all 0x1C area slots,
 * advancing the cursor by each block's size, then finalizes via func_0029BFC0.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00299B18: DebugPrintStub -> func_0026FD28 (file-scope,
 * line 82); size-mismatch string D_1A99A8 -> D_1A9A28 (+0x80 data lane); finalize
 * func_0029C418 -> func_0029BFC0; CalcSaveSectionsSize/DeserializeSaveSections/
 * g_saveSectionTableGlobal/g_saveSectionTableArea kept (named). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_002996C0);
#else
extern s32  CalcSaveSectionsSize(SaveSection *table);
extern s32  DeserializeSaveSections(void *image, s32 slotMul, SaveSection *table);
extern void func_0029BFC0(void);
extern SaveSection g_saveSectionTableGlobal[];
extern SaveSection g_saveSectionTableArea[];
extern char D_1A9A28[];   /* "save size mismatch" log string (EU, +0x80 of USA D_1A99A8) */
void func_002996C0(u8 *image) {
    s32 sizeGlobal = CalcSaveSectionsSize(g_saveSectionTableGlobal);
    s32 sizeArea = CalcSaveSectionsSize(g_saveSectionTableArea);
    s32 i;

    if (*(s32 *)(image + 0) != sizeGlobal || *(s32 *)(image + 4) != sizeArea) {
        func_0026FD28(D_1A9A28);
        return;
    }

    image += 8;
    DeserializeSaveSections(image, 0, g_saveSectionTableGlobal);
    image += sizeGlobal;
    for (i = 0; i < 0x1C; i++) {
        DeserializeSaveSections(image, i, g_saveSectionTableArea);
        image += sizeArea;
    }
    func_0029BFC0();
}
#endif

/* func_002997A0 — load-side save-image restore orchestrator (EU twin of USA
 * func_00299BF8, counterpart of BuildSaveImage). Reads the current save file off
 * the disc TOC, restores the serialized state through func_002996C0 while
 * preserving the 0x28-byte video-config block (D_1A7C18) across it, then clears
 * the resident-asset and progress latches. Matching arm stays INCLUDE_ASM; #else
 * is the structure model.
 * Word-verified vs USA func_00299BF8: g_discToc AND g_levelDialogToc both fold onto
 * EU D_0014B5C0 (TOC reads at +0x344/+0x340/+0x32C; dialog-scene writes at
 * +0x76D8 and +0x76F0 — same base+0x76C0 pointer as the already-landed
 * func_0029BFC0 p[6]/p[12]); g_bPalMode -> D_1A7C18; g_areaTable -> D_139460
 * (struct: dirty@0x17C, slot@0x18); g_loadedArmorVariant -> D_001A7308+0x8;
 * g_loadedHeldItemModelId -> D_001A7308+0x38; g_playerProgress kept (named).
 * Callees (by EU jal order): func_00289398->func_00289288, PumpDialogVoiceSystem->
 * func_002B8898, StartFileLoadPumpingVoice->func_002B8838, memcpy func_00283460->
 * func_00283370, func_00299B18->func_002996C0 (cross-body twin). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_002997A0);
#else
extern void func_00289288(s32 sectorByteOffset, void *outBuf); /* load save file -> *outBuf */
extern void func_002B8898(s32 blocking);                       /* PumpDialogVoiceSystem */
extern void func_002B8838(void *buf, s32 lba, s32 size);       /* StartFileLoadPumpingVoice */
extern void *func_00283370(void *dst, const void *src, s32 nbytes); /* EU byte memcpy */
extern s32 g_playerProgress; /* 0x1A79F8 first word of the persistent save block */
void func_002997A0(void) {
    u8   scratch[0x28];
    void *buf;

    func_00289288(*(s32 *)(&D_0014B5C0 + 0x344) << 11, &buf);
    func_002B8898(1);
    func_002B8838(buf,
                  *(s32 *)(&D_0014B5C0 + 0x340) + *(s32 *)(&D_0014B5C0 + 0x32C),
                  *(s32 *)(&D_0014B5C0 + 0x344));

    /* preserve the D_1A7C18 video-config block across the destructive restore */
    func_00283370(scratch, &D_1A7C18, 0x28);
    func_002996C0((u8 *)buf + *(s32 *)((u8 *)buf + 0x10));
    func_00283370(&D_1A7C18, scratch, 0x28);

    g_playerProgress = 0;
    if (D_139460.dirty != 0) {
        D_139460.dirty = 0;
    }
    if (D_139460.slot >= 0) {
        D_139460.slot = -1;
    }
    *(s32 *)(&D_0014B5C0 + 0x76D8) = -1;    /* dialog-scene TOC +0x18 */
    *(s32 *)(&D_001A7308 + 0x8) = -1;       /* g_loadedArmorVariant (EU) */
    g_nSaveLoadStatusCode[1] &= ~0x200;
    *(s32 *)(&D_001A7308 + 0x38) = -1;      /* g_loadedHeldItemModelId (EU) */
    *(s32 *)(&D_0014B5C0 + 0x76F0) = -1;    /* dialog-scene TOC +0x30 */
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", SaveLoadStateMachine);

/* BuildSaveImage(dest) — assemble the full save payload (EU twin of USA
 * BuildSaveImage, inverse of func_002996C0). Writes a two-word size header
 * (global total at +0, area total at +4, from CalcSaveSectionsSize), then
 * serializes the global block (slot 0) followed by all 0x1C area slots
 * (SerializeSaveSections), advancing the cursor by each block's written size.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA BuildSaveImage: all symbols named/region-agnostic
 * (CalcSaveSectionsSize, SerializeSaveSections, g_saveSectionTableGlobal/Area) —
 * no region deltas. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", BuildSaveImage);
#else
extern s32 CalcSaveSectionsSize(SaveSection *table);
extern s32 SerializeSaveSections(void *dst, s32 slot, SaveSection *table);
extern SaveSection g_saveSectionTableGlobal[];
extern SaveSection g_saveSectionTableArea[];
void BuildSaveImage(u8 *dest) {
    s32 i;

    *(s32 *)(dest + 0) = CalcSaveSectionsSize(g_saveSectionTableGlobal);
    *(s32 *)(dest + 4) = CalcSaveSectionsSize(g_saveSectionTableArea);
    dest += 8;
    dest += SerializeSaveSections(dest, 0, g_saveSectionTableGlobal);
    for (i = 0; i < 0x1C; i++) {
        dest += SerializeSaveSections(dest, i, g_saveSectionTableArea);
    }
}
#endif

/** Game-level libmc bring-up: bind the memory-card RPC services (McInit) and
 *  log the failure string (compiled-out func_0026FD28) when it errors.
 *  Falls off the end on success (the caller ignores the value; the fall-off
 *  keeps the original from materialising a return value). */
s32 InitMemCardLib(void) {
    if (McInit() != 0) {
        return func_0026FD28(D_1A9AE0);
    }
}

/* CalcSaveSectionsSize(table): return the number of bytes the section table
 * `table` serializes to. Layout is a leading 8-byte block, then for every
 * non-terminator entry an 8-byte header plus its (4-byte-aligned) payload, then
 * an 8-byte trailing terminator. Used to size the memory-card read/write
 * buffers for the two global save-section tables. */
s32 CalcSaveSectionsSize(SaveSection *table) {
    s32 size = 8;
    if (table->srcPtr != 0) {
        do {
            size += 8;
            size += table->len;
            table++;
            size = (size + 3) & -4;
        } while (table->srcPtr != 0);
    }
    return size + 8;
}

/* ComputeSaveSectionsCrc16 — CRC-16 over the save buffer (EU twin of USA func_0029BCA0).
 * Returns 0 if `len` exceeds the global save-section size. Otherwise runs a
 * bit-at-a-time CRC-16 over `len` bytes: seed 0xEDB88320, feedback poly 0x1F45 on
 * bit 15 — each byte XORed into the accumulator's high byte, then 8 shift/xor
 * steps. Returns the low 16 bits. Matching arm stays INCLUDE_ASM; #else is the
 * structure model.
 * Word-verified vs USA func_0029BCA0: CalcSaveSectionsSize + g_saveSectionTableGlobal
 * kept (named) — no region deltas. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", ComputeSaveSectionsCrc16);
#else
extern SaveSection g_saveSectionTableGlobal[];
s32 ComputeSaveSectionsCrc16(void *buf, s32 len) {
    u8 *data = (u8 *)buf;
    u8 *end;
    u32 crc;

    if (CalcSaveSectionsSize(g_saveSectionTableGlobal) < len) {
        return 0;
    }
    end = data + len;
    crc = 0xEDB88320;
    while (data < end) {
        s32 i;
        crc ^= (u32)(*data << 8);
        for (i = 7; i >= 0; i--) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1F45;
            } else {
                crc = crc << 1;
            }
        }
        data++;
    }
    return crc & 0xFFFF;
}
#endif

extern s32 ComputeSaveSectionsCrc16(void *buf, s32 len); /* EU ComputeSaveSectionsCrc16 */
#ifdef TARGET_NATIVE
extern void FillMemory32(void *dst, s32 pattern, s32 nbytes);
#endif
/* func_00283370 (EU byte memcpy) is declared once in func_002997A0's #else arm
 * (file-scope from there onward, covering the later users); the duplicate
 * file-scope decl that stood here was removed. */

/* VerifySaveHeaderChecksum(image): EU twin of USA func_0029BD48/VerifySaveHeaderChecksum.
 * The image header is { s32 payloadLen; s32 storedCrc; payload[payloadLen] };
 * recompute the CRC (ComputeSaveSectionsCrc16 from image+8 over payloadLen) and return 1
 * iff it equals the stored CRC. Stored CRC 0 -> empty/invalid -> 0.
 *
 * WALLED at the byte level by the 8-byte-packed callee-save frame (see
 * project_matching_ceiling); the portable #else mirrors the USA body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", VerifySaveHeaderChecksum);
#else
s32 VerifySaveHeaderChecksum(void *image) {
    s32 storedCrc = ((s32 *)image)[1];
    if (storedCrc == 0) {
        return 0;
    }
    return ComputeSaveSectionsCrc16((char *)image + 8, ((s32 *)image)[0]) == storedCrc;
}
#endif

/* SerializeSaveSections(dst, slot, table): EU twin of the USA serializer. Emits
 * the save image { s32 payloadLen; s32 crc; <sections> } into dst; each section
 * emits an 8-byte header { tag, len } then `len` payload bytes (4-byte aligned);
 * tag 0x1770 zero-fills (FillMemory32), else memcpy (func_00283370) from
 * srcPtr+len*slot. Closes with a { -1, 0 } terminator and stores the payload CRC
 * (ComputeSaveSectionsCrc16) in the header. Returns the total image size (payloadLen + 8).
 *
 * WALLED at the byte level by the 8-byte-packed callee-save frame (8 saved regs;
 * see project_matching_ceiling). Portable #else mirrors the USA body (logic +
 * cmp-oracle validated there; structurally identical EU asm). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", SerializeSaveSections);
#else
s32 SerializeSaveSections(void *dst, s32 slot, SaveSection *table) {
    s32 *cursor = (s32 *)((char *)dst + 8);
    s32 size = 0;

    if (table->srcPtr != 0) {
        do {
            s32 len = table->len;
            cursor[0] = table->tag;
            cursor[1] = len;
            size += 8;
            cursor += 2;
            if (table->tag == 0x1770) {
                FillMemory32(cursor, 0, table->len);
            } else {
                func_00283370(cursor, (char *)table->srcPtr + len * slot,
                              table->len);
            }
            len = table->len;
            table++;
            cursor = (s32 *)(((s32)cursor + len + 3) & -4);
            size = (size + len + 3) & -4;
        } while (table->srcPtr != 0);
    }

    size += 8;
    cursor[1] = 0;
    cursor[0] = -1;
    ((s32 *)dst)[1] = ComputeSaveSectionsCrc16((char *)dst + 8, size);
    ((s32 *)dst)[0] = size;
    return size + 8;
}
#endif

/* FillSaveSlotInfo — fill one save-slot info-display entry (EU twin of USA
 * func_0029BEA0/FillSaveSlotInfo). Entry is D_139460 + slot*0xA0 + dir*0x1C.
 * Copies the verified header image's slot metadata (rec +0x10/+0x1C/+0x2E/+0x64 ->
 * entry +0x30/+0x34/+0x38/+0x3C) plus an unaligned 8-byte block (rec+0x70 ->
 * entry+0x40), and stores whether the image CRC is invalid (VerifySaveHeaderChecksum == 0)
 * at entry+0x48. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0029BEA0: func_0029BD48 -> VerifySaveHeaderChecksum (CRC verify,
 * defined above at file scope); g_areaTable -> (u8*)&D_139460. All rec/entry byte
 * offsets identical to the twin (confirmed in the EU .s). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", FillSaveSlotInfo);
#else
void FillSaveSlotInfo(u8 *rec, s32 slot, s32 dir) {
    s32 crcValid = VerifySaveHeaderChecksum(rec);
    u8 *entry = (u8 *)&D_139460 + slot * 0xA0 + dir * 0x1C;

    *(s32 *)(entry + 0x48) = (crcValid == 0);
    *(s32 *)(entry + 0x30) = *(s32 *)(rec + 0x10);
    *(s32 *)(entry + 0x34) = *(s32 *)(rec + 0x1C);
    *(s32 *)(entry + 0x38) = *(u8 *)(rec + 0x2E);
    *(s32 *)(entry + 0x3C) = *(s32 *)(rec + 0x64);
    *(u64 *)(entry + 0x40) = *(u64 *)(rec + 0x70);   /* unaligned 8-byte block */
}
#endif

#ifdef TARGET_NATIVE
extern int memcmp(); /* K&R decl: avoids the ee-gcc builtin-prototype conflict warning */
extern s32 D_1A9A20;  /* EU 0x1A9A20 changed-section counter (= USA D_1A99A0) */
#endif
/* D_139460 (the area-record table base, stride 0xA0) is touched at base+0x14C
 * (selected index) and record+0x24 (result slot); byte-addressed via casts. */
#define EU_AREA_RECORD_STRIDE 0xA0
#define EU_AREA_SELECTED_INDEX_OFF 0x14C
#define EU_AREA_LOAD_RESULT_OFF 0x24

/* DeserializeSaveSections(image, slotMul, table): EU twin of the USA deserializer.
 * Restores `table` from a save image (inverse of SerializeSaveSections),
 * reconciling each image section against the descriptor table and returning the
 * mismatch tally (1 on a bad-CRC image). See the USA unit comment for the full
 * step breakdown. WALLED by the 8-byte-packed callee-save frame (10 saved regs;
 * see project_matching_ceiling). Portable #else mirrors the USA body (logic +
 * cmp-oracle validated there; byte-identical EU asm modulo region symbols). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", DeserializeSaveSections);
#else
s32 DeserializeSaveSections(void *image, s32 slotMul, SaveSection *table) {
    s32 *section;
    s32 mismatchCount;
    s32 runningOffset;
    s32 sentinel;
    SaveSection *entry;

    if (VerifySaveHeaderChecksum(image) == 0) {
        return 1;
    }

    section = (s32 *)((char *)image + 8);
    mismatchCount = 0;
    runningOffset = 8;

    for (entry = table; entry->srcPtr != 0; entry++) {
        entry->matchResult = 0;
    }

    while (section[0] != -1) {
        s32 sectionTag = section[0];
        s32 sectionLen = section[1];
        SaveSection *match = 0;

        for (entry = table; entry->srcPtr != 0; entry++) {
            if (entry->tag == sectionTag) {
                match = entry;
                break;
            }
        }

        if (match != 0 && match->srcPtr != 0) {
            s32 descLen = match->len;
            char *dest = (char *)match->srcPtr + slotMul * descLen;
            char *payload = (char *)(section + 2);
            s32 copyLen;

            if (sectionLen == descLen) {
                match->matchResult = 1;
                copyLen = descLen;
            } else if (sectionLen < descLen) {
                match->matchResult = -1;
                copyLen = sectionLen;
            } else {
                match->matchResult = -2;
                copyLen = descLen;
            }

            if (memcmp(dest, payload, copyLen) != 0) {
                D_1A9A20 += 1;
            }

            if (match->tag == 0x1770) {
                runningOffset += 8 + ((sectionLen + 3) & -4);
            } else {
                func_00283370(dest, payload, copyLen);
                runningOffset += 8 + ((copyLen + 3) & -4);
            }
        } else {
            mismatchCount += 1;
        }

        section = (s32 *)((char *)section + 8 + ((sectionLen + 3) & -4));
    }

    runningOffset += 8;

    if (table->srcPtr != 0) {
        if (runningOffset != CalcSaveSectionsSize(table)) {
            mismatchCount += 1;
        }
        sentinel = section[2];
        if (table->tag != sentinel) {
            entry = table;
            for (;;) {
                if (entry->matchResult <= 0) {
                    mismatchCount += 1;
                }
                entry++;
                if (entry->srcPtr == 0 || entry->tag == sentinel) {
                    break;
                }
            }
        }
    }

    {
        s32 idx = *(s32 *)((char *)&D_139460 + EU_AREA_SELECTED_INDEX_OFF);
        *(s32 *)((char *)&D_139460 + idx * EU_AREA_RECORD_STRIDE +
                 EU_AREA_LOAD_RESULT_OFF) = mismatchCount;
    }
    return mismatchCount;
}
#endif

/* CommitProgressCheckpoint(flag, destination): EU twin of USA CommitProgressCheckpoint
 * (text/198FA0). Stage the progress checkpoint into the save-image buffers.
 * Timestamps from the CD RTC (func_001256D8 into the D_001A7308+0xD8 buffer,
 * post-processed by func_00131AF8), runs func_00298A30 + the per-progress hook
 * func_00297FD0, then - only when the current area (D_139460+0x148) is valid and
 * armed - records the live game state into the area's stride-0x1C sub-record and
 * serializes both save-section tables. `flag`==0 also arms status bit 0x200.
 * The R5900 fn leaves a status word in $v0 but callers ignore it (void proto).
 * Matching arm stays INCLUDE_ASM (packed-save frame + reload-artifact wall);
 * #else is the structure model.
 * Word-verified vs USA CommitProgressCheckpoint: g_gsPixelOffsetY -> D_001A7308+0xD8;
 * g_areaTable -> D_139460; D_1A7BC8 -> D_1A7C48 (+0x80); g_levelVisitedMarkers ->
 * D_1A7C70 (+0x80); callees sceCdReadClock->func_001256D8, func_00131A98->
 * func_00131AF8, func_00298A00->func_00298A30, func_00297FA0->func_00297FD0;
 * g_boltCount/g_miscExtras/g_health/g_playerProgress/g_saveImageGlobal+Area/
 * g_saveSectionTableGlobal+Area/g_nSaveLoadStatusCode kept (named). Area
 * sub-record offsets unchanged. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", CommitProgressCheckpoint);
#else
extern u8 g_health[];
extern s32 g_boltCount;
extern u8 g_miscExtras;
extern s32 D_1A7C48;
extern u8 D_1A7C70[];
extern u8 g_saveImageGlobal[];
extern u8 g_saveImageArea[];
extern SaveSection g_saveSectionTableGlobal[];
extern SaveSection g_saveSectionTableArea[];
extern void func_001256D8(void *rtc);
extern void func_00131AF8(void *rtc);
extern void func_00298A30(void);
extern void func_00297FD0(void *record);
void CommitProgressCheckpoint(s32 flag, s32 destination) {
    u8 *ctx = (u8 *)&D_139460;
    u8 *rtc = (u8 *)&D_001A7308 + 0xD8;
    s32 areaIdx;
    s16 slot;
    u8 *rec;
    s32 armed;
    s32 savedMarker;

    func_001256D8(rtc);
    func_00131AF8(rtc);
    func_00298A30();
    func_00297FD0(g_health + 0xF8C + g_playerProgress * 0x800);

    areaIdx = *(s32 *)(ctx + 0x148);
    if (areaIdx == -1) {
        return;
    }
    if (*(s16 *)(ctx + areaIdx * 0xA0 + 0x18) < 0) {
        return;
    }

    armed = *(s32 *)(ctx + 0x17C) | flag;
    *(s32 *)(ctx + 0x17C) = armed;
    if (armed == 0) {
        return;
    }
    if (flag == 0) {
        g_nSaveLoadStatusCode[1] |= 0x200;
    }
    if (*(s32 *)(ctx + 0x15C) >= 3) {
        return;
    }
    if (*(s32 *)(ctx + 0x164) >= 0) {
        return; /* a write is already pending */
    }

    *(s32 *)(ctx + 0x150) = g_playerProgress;
    savedMarker = 0;
    if (destination >= 0) {
        g_playerProgress = destination;
        savedMarker = D_1A7C70[destination];
        if (savedMarker == 0) {
            D_1A7C70[destination] = 1;
        }
    }

    slot = *(s16 *)(ctx + 0x18);
    rec = ctx + slot * 0x1C;
    *(s32 *)(rec + 0x34) = g_boltCount;
    *(s32 *)(rec + 0x30) = g_playerProgress;
    *(s32 *)(rec + 0x3C) = D_1A7C48;
    *(s32 *)(rec + 0x38) = g_miscExtras;
    *(u32 *)(rec + 0x40) = *(u32 *)rtc;
    *(u32 *)(rec + 0x44) = *(u32 *)(rtc + 4);

    SerializeSaveSections(g_saveImageGlobal, 0, g_saveSectionTableGlobal);
    SerializeSaveSections(g_saveImageArea, *(s32 *)(ctx + 0x150),
                          g_saveSectionTableArea);

    if (destination >= 0) {
        D_1A7C70[g_playerProgress] = savedMarker;
        g_playerProgress = *(s32 *)(ctx + 0x150);
    }

    if (*(s32 *)(ctx + 0x164) < 0) {
        *(s32 *)(ctx + 0x164) = 0xF;
        *(s32 *)(ctx + 0x168) = *(s32 *)(ctx + 0x148);
    }
}
#endif

/** Reset the dialog-scene model bookkeeping: mark armor-variant
 *  (D_001A7308+0x8) and held-item (D_001A7308+0x38) models unloaded (-1),
 *  clear the two cached TOC entries (D_0014B5C0+0x76D8/+0x76F0), and clear
 *  the D_1A798C flag. EU equivalent of USA func_0029C418. */
void func_0029BFC0(void) {
    volatile s32 *p;
    *(volatile s32 *)(&D_001A7308 + 0x8) = -1;
    p = (volatile s32 *)(&D_0014B5C0 + 0x76C0);
    p[6] = -1;
    *(volatile s32 *)(&D_001A7308 + 0x38) = -1;
    p[12] = -1;
    D_1A798C = 0;
}

/* func_0029BFF0(a,b): EU twin of USA func_0029C448 — forward two args to the
 * widget at g_guiInstance+0x36F28 (method func_0033A248). Register-coloring wall
 * (same $v0/$v1 residue as USA); matching arm stays INCLUDE_ASM, #else is the model.
 * Word-verified vs USA func_0029C448: method func_00339398->func_0033A248; widget
 * offset 0x36F28 UNCHANGED (widget-embed offsets are NOT +0xB0-shifted in EU). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029BFF0);
#else
extern s32 func_0033A248(char *widget, s32 a, s32 b);
s32 func_0029BFF0(s32 a, s32 b) {
    if (g_guiInstance != 0) {
        return func_0033A248(g_guiInstance + 0x36F28, a, b);
    }
}
#endif

/** Forward `arg` to the HUD render object at g_guiInstance+0x376C8 (method
 *  func_0034EFF0). */
s32 func_0029C030(s32 arg) {
    if (g_guiInstance != 0) {
        return func_0034EFF0(g_guiInstance + 0x376C8, arg);
    }
}

/** Forward two args to the HUD render object at g_guiInstance+0x376C8 (method
 *  func_0034ECB0). */
s32 func_0029C068(s32 arg0, s32 arg1) {
    if (g_guiInstance != 0) {
        return func_0034ECB0(g_guiInstance + 0x376C8, arg0, arg1);
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C0A8);

/** Post a GUI page/event id to the active screen object at
 *  g_guiInstance+0x37ED0 via GuiScreenSetEventAndReveal (no-op while the GUI
 *  is down). Used by the orb-heal path and the level transition. */
s32 PostGuiScreenEvent(s32 eventId) {
    if (g_guiInstance != 0) {
        return GuiScreenSetEventAndReveal(g_guiInstance + 0x37ED0, eventId);
    }
}

/** Register the GUI pump callback pair at g_guiInstance+0x3F9F0/+0x3F9F4
 *  (b into +0x3F9F4 first, then a into +0x3F9F0 — the volatile stores pin the
 *  original descending store order; func_0029C640/func_0029CD18 invoke these
 *  slots via jalr). */
void func_0029C0F0(s32 a, s32 b) {
    char *gui = g_guiInstance;
    if (gui != 0) {
        *(volatile s32 *)(gui + 0x3FAA4) = b;
        *(volatile s32 *)(gui + 0x3FAA0) = a;
    }
}

/** True when either of the GUI pump callback slots at
 *  g_guiInstance+0x3F9F0/+0x3F9F4 is registered. */
s32 func_0029C118(void) {
    char *gui = g_guiInstance;
    if (gui != 0) {
        if (*(s32 *)(gui + 0x3FAA0) != 0 || *(s32 *)(gui + 0x3FAA4) != 0) {
            return 1;
        }
    }
    return 0;
}

/* func_0029C158(a,b,idx): EU twin of USA func_0029C5B0 — if the GUI is up and
 * idx<2, store the (a,b) pair into the slot table at g_guiInstance+0x3FAC8+idx*8
 * and return 1; else 0. Register-coloring wall (same as USA); #else is the model.
 * Word-verified vs USA func_0029C5B0: 0x38000 base + idx*8 unchanged; slot inner
 * offsets 0x7A18/0x7A1C -> 0x7AC8/0x7ACC (GUI instance field +0xB0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C158);
#else
s32 func_0029C158(s32 a, s32 b, s32 idx) {
    char *slot;
    if (g_guiInstance == 0 || (u32)idx >= 2) {
        return 0;
    }
    slot = g_guiInstance + 0x38000 + idx * 8;
    *(s32 *)(slot + 0x7AC8) = a;
    *(s32 *)(slot + 0x7ACC) = b;
    return 1;
}
#endif

/* func_0029C1A8(idx): EU twin of USA func_0029C600 — clear the slot pair at
 * g_guiInstance+0x3FAC8+idx*8 (counterpart of func_0029C158). Matching arm stays
 * INCLUDE_ASM (reload-artifact wall), #else is the model.
 * Word-verified vs USA func_0029C600: slot inner offsets 0x7A18/0x7A1C ->
 * 0x7AC8/0x7ACC (GUI instance field +0xB0); idx checked before the gui read. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C1A8);
#else
void func_0029C1A8(s32 idx) {
    char *slot;
    if ((u32)idx >= 2) {
        return;
    }
    slot = g_guiInstance + 0x38000 + idx * 8;
    *(s32 *)(slot + 0x7AC8) = 0;
    *(s32 *)(slot + 0x7ACC) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C1E0);

/** Forward to the widget at g_guiInstance+0x36F28 (method func_003506A0). */
s32 func_0029C1F0(void) {
    if (g_guiInstance != 0) {
        return func_003506A0(g_guiInstance + 0x36F28);
    }
}

/* func_0029C220(a1..a10): EU twin of USA func_0029C678 — forward all ten
 * passthrough args to the widget at g_guiInstance+0x36F28 (method func_00339E38),
 * prepending the widget pointer. No-op if the GUI isn't up. Packed callee-save
 * frame wall; matching arm stays INCLUDE_ASM, #else is the model.
 * Word-verified vs USA func_0029C678: method func_00338F88->func_00339E38; widget
 * offset 0x36F28 UNCHANGED. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C220);
#else
extern s32 func_00339E38(char *widget, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
                         s32 a6, s32 a7, s32 a8, s32 a9, s32 a10);
s32 func_0029C220(s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7,
                  s32 a8, s32 a9, s32 a10) {
    if (g_guiInstance != 0) {
        return func_00339E38(g_guiInstance + 0x36F28, a1, a2, a3, a4, a5, a6,
                             a7, a8, a9, a10);
    }
}
#endif

/* func_0029C2A8 — EU twin of USA func_0029C700: refresh the 4 front-end menu-item
 * widgets from their sources. No-op if the GUI isn't up. For each of 4 items looks
 * up its source id (func_002EE7B0 over the D_1A9B50 id table); when valid (id>=0)
 * reads the source record (D_2626C0 + id*0x28) icon halfword (+0x1C) and count
 * (+0x24), writes them + a colour (0x6029A1FF when count>=1 else 0x60F0F0B0) into
 * the display slot (D_256418 + 8 + i*0x18, icon at -4), and shows it (func_0033C8F8
 * mode 0); else hides it (mode 2). Widget base g_guiInstance+0x3F860. Packed-save
 * wall; matching arm stays INCLUDE_ASM, #else is the model.
 * Word-verified vs USA func_0029C700: func_002EE6F8->func_002EE7B0, func_0033BA18->
 * func_0033C8F8; D_1A9AD0->D_1A9B50 (+0x80), D_262920->D_2626C0 (region delta
 * -0x260, verified), D_256398->D_256418 (+0x80); widget base 0x3F7B0->0x3F860
 * (+0xB0); rec offsets +0x1C/+0x24 and slot stride 0x18 unchanged. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C2A8);
#else
extern s32 func_002EE7B0(s32 idSelector);
extern void func_0033C8F8(void *widget, s32 item, s32 mode);
extern s32 D_1A9B50[];
extern u8 D_2626C0[];
extern u8 D_256418[];

void func_0029C2A8(void) {
    u8 *slot;
    s32 i;

    if (g_guiInstance == 0) {
        return;
    }
    slot = D_256418 + 8;
    for (i = 0; i < 4; i++) {
        s32 id = func_002EE7B0(D_1A9B50[i]);
        if (id >= 0) {
            u8 *rec = D_2626C0 + id * 0x28;
            s32 count = *(s32 *)(rec + 0x24);
            *(s32 *)(slot - 4) = *(s16 *)(rec + 0x1C);
            *(s32 *)slot = (count >= 1) ? 0x6029A1FF : 0x60F0F0B0;
            func_0033C8F8(g_guiInstance + 0x3F860, i, 0);
        } else {
            func_0033C8F8(g_guiInstance + 0x3F860, i, 2);
        }
        slot += 0x18;
    }
}
#endif

/* func_0029C3C0 — EU twin of USA func_0029C818: poll GUI-ready state and, on the
 * transition into "ready", configure the front-end screen widget. No-op if the GUI
 * isn't up. Computes the new ready state (whether func_0026F640 is nonzero when
 * func_0026F638 is set), returns early if unchanged (D_1A9B0C). On a real change
 * runs func_0029C630 and latches the state; only when it becomes 1 does it set up
 * the screen widget at g_guiInstance+0x3F860 (func_0033C928 / func_0033C8F0 scale
 * 36.0 / func_0033C908 id -1), prime g_guiInstance+0x3CDF8 (func_0034B658), and
 * pump func_0029C2A8. Packed-save wall; matching arm stays INCLUDE_ASM, #else is
 * the model.
 * Word-verified vs USA func_0029C818: func_0026F7D0->func_0026F638, func_0026F7D8->
 * func_0026F640, func_0029CA88->func_0029C630, func_0034A1D0->func_0034B658,
 * func_0033BA48->func_0033C928, func_0033BA10->func_0033C8F0, func_0033BA28->
 * func_0033C908, func_0029C700->func_0029C2A8; D_1A9A8C->D_1A9B0C, D_1A9A98->
 * D_1A9B18, D_1A9AC0->D_1A9B40, D_256398->D_256418; screen widget bases 0x3F7B0->
 * 0x3F860 and 0x3CD48->0x3CDF8 (both +0xB0); scale 36.0f unchanged. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C3C0);
#else
extern s32 func_0026F638(void);
extern s32 func_0026F640(void);
extern void func_0029C630(void);
extern void func_0034B658(void *a, void *b);
extern void func_0033C928(void *a, void *b, void *c);
extern void func_0033C8F0(void *a, f32 scale);
extern void func_0033C908(void *a, s32 id);
extern void func_0029C2A8(void);
extern s32 D_1A9B0C;
extern u8 D_1A9B18[];
extern u8 D_1A9B40[];
extern u8 D_256418[];

void func_0029C3C0(void) {
    s32 ready;

    if (g_guiInstance == 0) {
        return;
    }
    ready = 0;
    if (func_0026F638() != 0) {
        ready = (func_0026F640() != 0);
    }
    if (D_1A9B0C == ready) {
        return;
    }
    func_0029C630();
    D_1A9B0C = ready;
    if (ready != 1) {
        return;
    }
    func_0034B658(g_guiInstance + 0x3CDF8, D_1A9B18);
    func_0033C928(g_guiInstance + 0x3F860, D_1A9B40, D_256418);
    func_0033C8F0(g_guiInstance + 0x3F860, 36.0f);
    func_0033C908(g_guiInstance + 0x3F860, -1);
    func_0029C2A8();
}
#endif

/* func_0029C498 — EU twin of USA func_0029C8F0: the front-end screen-action pump.
 * Returns -1 unless an action is pending (D_1A9B08), the popup-busy gate is clear
 * (D_1A8D14==0) and the GUI is up. GUI down: counts the pending action toward zero
 * (returns 0). GUI up: polls readiness (func_0029C3C0) and, once ready, refreshes
 * the menu items (func_0029C2A8), reconciles the pending-item latch D_1A9B60 vs the
 * screen state block D_138200, rebuilds the screen (func_0033C5B0/func_0033C920),
 * resolves the highlighted item via func_0034B358 and validates its id through the
 * D_1A9B50 table + func_002EE7B0, optionally advances the selection (func_0034B650,
 * latching D_1A9B10), and commits it (func_0033C908). Returns the committed item
 * index when an item was committed, else -1. Packed-save wall; matching arm stays
 * INCLUDE_ASM, #else is the model.
 * Word-verified vs USA func_0029C8F0: func_0029C818->func_0029C3C0, func_0029C700->
 * func_0029C2A8, func_0033B6D0->func_0033C5B0, func_0033BA40->func_0033C920,
 * func_00349ED0->func_0034B358, func_002EE6F8->func_002EE7B0, func_0034A1C8->
 * func_0034B650, func_0033BA28->func_0033C908; D_1A9A88->D_1A9B08, D_1A8C64->
 * D_1A8D14 (+0xB0), D_1A9A8C->D_1A9B0C, D_138180->D_138200 (+0x80), D_1A9AE0->
 * D_1A9B60, D_1A9AD0->D_1A9B50, D_1A9A90->D_1A9B10; screen widget bases 0x3F7B0->
 * 0x3F860 and 0x3CD48->0x3CDF8 (both +0xB0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C498);
#else
extern u8 D_138200[];
extern s32 D_1A9B08;
extern s32 D_1A9B60;
extern void func_0033C5B0(void *widget);
extern void func_0033C920(void *screen, s32 arg);
extern s32 func_0034B358(void *widget, void *stateBlock);
extern void func_0034B650(void *widget);
s32 func_0029C498(void) {
    s32 ready;
    s32 sel;
    s32 committed;
    s32 latch;

    if (D_1A9B08 == 0) {
        return -1;
    }
    if (D_1A8D14 != 0) {
        return -1;
    }
    if (g_guiInstance == 0) {
        /* GUI down: count the pending action down toward zero. */
        D_1A9B08 = D_1A9B08 - 1;
        if (D_1A9B08 < 0) {
            D_1A9B08 = 0;
        }
        return 0;
    }

    func_0029C3C0();
    ready = D_1A9B0C;
    if (ready == 0) {
        return -1;
    }

    committed = 0;
    if (ready == 1) {
        func_0029C2A8();
        if (*(s32 *)(D_138200 + 0x1C0) & 0x10) {
            D_1A9B60 = ready; /* arm the pending-item latch (ready == 1) */
            *(s32 *)(D_138200 + 0x1CC) = 2;
        } else if (D_1A9B60 != 0) {
            D_1A9B60 = 0;
            committed = 1;
        }
        func_0033C5B0(g_guiInstance + 0x3F860);
        latch = D_1A9B60;
    } else {
        latch = 0;
    }

    func_0033C920(g_guiInstance + 0x3F860, latch);
    sel = func_0034B358(g_guiInstance + 0x3CDF8, D_138200);
    if (sel >= 0) {
        if (func_002EE7B0(D_1A9B50[sel]) < 0) {
            sel = -1; /* highlighted id no longer valid */
        }
    }

    if (committed != 0) {
        func_0034B650(g_guiInstance + 0x3CDF8);
        if (D_1A9B10 < 0) {
            D_1A9B10 = sel;
        }
    }
    func_0033C908(g_guiInstance + 0x3F860, sel);
    return committed ? sel : -1;
}
#endif

/** Invalidate the D_1A9B10 GUI state (set to -1). */
void func_0029C630(void) {
    D_1A9B10 = -1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C640);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C7F0);

/* func_0029C820 (EU twin of USA func_0029CCB8): if the GUI is up and several
 * gating flags (D_1A9B08, g_nNanotechBonusHealTimer+4, D_1A8D14, D_1A9B0C)
 * permit, forward to the widget at g_guiInstance+0x3F860 (func_0033C600).
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0029CCB8: D_1A9A88->D_1A9B08, D_1A9A8C->D_1A9B0C
 * (gp_rel; file-scope), D_1A8C64->D_1A8D14 (hi/lo; file-scope), widget offset
 * 0x3F7B0->0x3F860 (+0xB0 GUI-instance shift), func_0033B720->func_0033C600;
 * g_guiInstance / g_nNanotechBonusHealTimer kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C820);
#else
extern s32 g_nNanotechBonusHealTimer;
extern s32 func_0033C600(void *widget);
void func_0029C820(void) {
    if (D_1A9B08 == 0) {
        return;
    }
    if (*(s16 *)((char *)&g_nNanotechBonusHealTimer + 0x4) != 0) {
        return;
    }
    if (D_1A8D14 != 0) {
        return;
    }
    if (D_1A9B0C == 0) {
        return;
    }
    if (g_guiInstance == 0) {
        return;
    }
    func_0033C600(g_guiInstance + 0x3F860);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C880);

/* func_0029CF08: empty jr-ra stub - installed by GuiManagerCreate as a default no-op GUI callback. */
void func_0029CA68(void) {
}

/** Forward to the widget at g_guiInstance+0x36F28 (method func_00350660). */
s32 func_0029CA70(void) {
    if (g_guiInstance != 0) {
        return func_00350660(g_guiInstance + 0x36F28);
    }
}

/** Forward to the widget at g_guiInstance+0x36F28 (method func_003506E0). */
s32 func_0029CAA0(void) {
    if (g_guiInstance != 0) {
        return func_003506E0(g_guiInstance + 0x36F28);
    }
}

/** Forward to the popup-menu widget at g_guiInstance+0x39160 (method func_0034B088). */
s32 func_0029CAD0(void) {
    if (g_guiInstance != 0) {
        return func_0034B088(g_guiInstance + 0x39160);
    }
}

/** Run the popup-menu widget at g_guiInstance+0x39160 (UpdatePopupMenu, returns
 *  the selected item index); 0 when the GUI is down. */
s32 func_0029CB00(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return UpdatePopupMenu(g_guiInstance + 0x39160, arg);
}

/** Forward to the widget at g_guiInstance+0x39620 (method func_00349AB8). */
s32 func_0029CB40(void) {
    if (g_guiInstance != 0) {
        return func_00349AB8(g_guiInstance + 0x39620);
    }
}

/** Forward to the widget at g_guiInstance+0x39AF0 (method func_00348728). */
s32 func_0029CB70(void) {
    if (g_guiInstance != 0) {
        return func_00348728(g_guiInstance + 0x39BA0);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x39620 (method GuiWeaponGridTick);
 *  0 when the GUI is down. */
s32 func_0029CBA0(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return GuiWeaponGridTick(g_guiInstance + 0x39620, arg);
}

/** Tick the map screen at g_guiInstance+0x39BA0 (GuiMapScreenTick,
 *  0x003480C8; USA func_0029D080 / +0x39AF0): `buttons` is the pad-button
 *  mask, `out` receives the state the tick latches (write-only). Returns the
 *  tick's result (0), or 0 when the GUI is down. */
s32 func_0029CBE0(s32 buttons, s32 *out) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return GuiMapScreenTick(g_guiInstance + 0x39BA0, buttons, out);
}

/** Forward `arg` to the widget at g_guiInstance+0x3B418 (method GuiConfirmPopupTick);
 *  0 when the GUI is down. */
s32 func_0029CC28(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return GuiConfirmPopupTick(g_guiInstance + 0x3B4C8, arg);
}

/** Forward to the widget at g_guiInstance+0x3B418 (method GuiConfirmPopupDraw). */
s32 func_0029CC68(void) {
    if (g_guiInstance != 0) {
        return GuiConfirmPopupDraw(g_guiInstance + 0x3B4C8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3AF80 (method GuiLevelInfoPanelTick);
 *  0 when the GUI is down. */
s32 func_0029CC98(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return GuiLevelInfoPanelTick(g_guiInstance + 0x3B030, arg);
}

/** Forward to the widget at g_guiInstance+0x3AF80 (method func_0033C308). */
s32 func_0029CCD8(void) {
    if (g_guiInstance != 0) {
        return func_0033C308(g_guiInstance + 0x3B030);
    }
}

/** Forward to the widget at g_guiInstance+0x3B6F8 (method func_0033D838). */
s32 func_0029CD08(void) {
    if (g_guiInstance != 0) {
        return func_0033D838(g_guiInstance + 0x3B7A8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3B6F8 (method func_0033D468);
 *  0 when the GUI is down. */
s32 func_0029CD38(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033D468(g_guiInstance + 0x3B7A8, arg);
}

/** Forward to the widget at g_guiInstance+0x3BA58 (method func_00341020). */
s32 func_0029CD78(void) {
    if (g_guiInstance != 0) {
        return func_00341020(g_guiInstance + 0x3BB08);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3BA58 (method func_00340A50);
 *  0 when the GUI is down. */
s32 func_0029CDA8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00340A50(g_guiInstance + 0x3BB08, arg);
}

/** Forward to the widget at g_guiInstance+0x3A000 (method func_00347438). */
s32 func_0029CDE8(void) {
    if (g_guiInstance != 0) {
        return func_00347438(g_guiInstance + 0x3A0B0);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3A000 (method func_00346528);
 *  0 when the GUI is down. */
s32 func_0029CE18(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00346528(g_guiInstance + 0x3A0B0, arg);
}

/** Forward to the widget at g_guiInstance+0x3C160 (method func_00343C88). */
s32 func_0029CE58(void) {
    if (g_guiInstance != 0) {
        return func_00343C88(g_guiInstance + 0x3C210);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C160 (method func_00343928);
 *  0 when the GUI is down. */
s32 func_0029CE88(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00343928(g_guiInstance + 0x3C210, arg);
}

/** Forward to the widget at g_guiInstance+0x3C160 (method func_00343C88) —
 *  byte-identical twin of func_0029D2F8 (two call sites got their own stubs). */
s32 func_0029CEC8(void) {
    if (g_guiInstance != 0) {
        return func_00343C88(g_guiInstance + 0x3C210);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C160 (method func_00343928)
 *  — byte-identical twin of func_0029D328; 0 when the GUI is down. */
s32 func_0029CEF8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00343928(g_guiInstance + 0x3C210, arg);
}

/** Forward to the widget at g_guiInstance+0x3C480 (method func_00345940). */
s32 func_0029CF38(void) {
    if (g_guiInstance != 0) {
        return func_00345940(g_guiInstance + 0x3C530);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C480 (method func_003457F0);
 *  0 when the GUI is down. */
s32 func_0029CF68(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003457F0(g_guiInstance + 0x3C530, arg);
}

/** Forward to the widget at g_guiInstance+0x3C7E8 (method func_00340610). */
s32 func_0029CFA8(void) {
    if (g_guiInstance != 0) {
        return func_00340610(g_guiInstance + 0x3C898);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C7E8 (method func_003405B0);
 *  0 when the GUI is down. */
s32 func_0029CFD8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003405B0(g_guiInstance + 0x3C898, arg);
}

/** Forward to the widget at g_guiInstance+0x3D1D8 (method func_0033F840). */
s32 func_0029D018(void) {
    if (g_guiInstance != 0) {
        return func_0033F840(g_guiInstance + 0x3D288);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3D1D8 (method func_0033F738);
 *  0 when the GUI is down. */
s32 func_0029D048(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033F738(g_guiInstance + 0x3D288, arg);
}

/** Forward `arg` to the widget at g_guiInstance+0x3D4B8 (method func_0033E460);
 *  0 when the GUI is down. */
s32 func_0029D088(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033E460(g_guiInstance + 0x3D568, arg);
}

/** Forward to the widget at g_guiInstance+0x3D4B8 (method func_0033E608). */
s32 func_0029D0C8(void) {
    if (g_guiInstance != 0) {
        return func_0033E608(g_guiInstance + 0x3D568);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3D798 (method func_0033E9E8);
 *  0 when the GUI is down. */
s32 func_0029D0F8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033E9E8(g_guiInstance + 0x3D848, arg);
}

/** Forward to the widget at g_guiInstance+0x3D798 (method func_0033EAF0). */
s32 func_0029D138(void) {
    if (g_guiInstance != 0) {
        return func_0033EAF0(g_guiInstance + 0x3D848);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3DDE8 (method func_0033F470);
 *  0 when the GUI is down. */
s32 func_0029D168(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033F470(g_guiInstance + 0x3DE98, arg);
}

/** Forward to the widget at g_guiInstance+0x3DDE8 (method func_0033F508). */
s32 func_0029D1A8(void) {
    if (g_guiInstance != 0) {
        return func_0033F508(g_guiInstance + 0x3DE98);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E0C8 (method func_0033DDC0);
 *  0 when the GUI is down. */
s32 func_0029D1D8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033DDC0(g_guiInstance + 0x3E178, arg);
}

/** Forward to the widget at g_guiInstance+0x3E0C8 (method func_0033DF90). */
s32 func_0029D218(void) {
    if (g_guiInstance != 0) {
        return func_0033DF90(g_guiInstance + 0x3E178);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E3A8 (method func_0033E160);
 *  0 when the GUI is down. */
s32 func_0029D248(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033E160(g_guiInstance + 0x3E458, arg);
}

/** Forward to the widget at g_guiInstance+0x3E3A8 (method func_0033E248). */
s32 func_0029D288(void) {
    if (g_guiInstance != 0) {
        return func_0033E248(g_guiInstance + 0x3E458);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3F3F0 (method func_00340348);
 *  0 when the GUI is down. */
s32 func_0029D2B8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00340348(g_guiInstance + 0x3F4A0, arg);
}

/** Forward to the widget at g_guiInstance+0x3F3F0 (method func_00340418). */
s32 func_0029D2F8(void) {
    if (g_guiInstance != 0) {
        return func_00340418(g_guiInstance + 0x3F4A0);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3DA78 (method func_0033EEF8);
 *  0 when the GUI is down. */
s32 func_0029D328(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033EEF8(g_guiInstance + 0x3DB28, arg);
}

/** Forward to the widget at g_guiInstance+0x3DA78 (method func_0033F190). */
s32 func_0029D368(void) {
    if (g_guiInstance != 0) {
        return func_0033F190(g_guiInstance + 0x3DB28);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3EB50 (method func_0033FB00);
 *  0 when the GUI is down. */
s32 func_0029D398(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033FB00(g_guiInstance + 0x3EC00, arg);
}

/** Forward to the widget at g_guiInstance+0x3EB50 (method func_0033FBE0). */
s32 func_0029D3D8(void) {
    if (g_guiInstance != 0) {
        return func_0033FBE0(g_guiInstance + 0x3EC00);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E760 (method func_00344528);
 *  0 when the GUI is down. */
s32 func_0029D408(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00344528(g_guiInstance + 0x3E810, arg);
}

/** Forward to the widget at g_guiInstance+0x3E760 (method func_003447C0). */
s32 func_0029D448(void) {
    if (g_guiInstance != 0) {
        return func_003447C0(g_guiInstance + 0x3E810);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E9C8 (method func_00343F50);
 *  0 when the GUI is down. */
s32 func_0029D478(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00343F50(g_guiInstance + 0x3EA78, arg);
}

/** Forward to the widget at g_guiInstance+0x3E9C8 (method func_00343FA0). */
s32 func_0029D4B8(void) {
    if (g_guiInstance != 0) {
        return func_00343FA0(g_guiInstance + 0x3EA78);
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029D4E8);

/** Tick the on-screen bolt counter (UpdateBoltCounterHud) only when the HUD
 *  render context (g_guiInstance) is present. */
s32 TickBoltCounterHud(void) {
    if (g_guiInstance != 0) {
        return UpdateBoltCounterHud();
    }
}

/** Run the func_0028B038 HUD updater only when the GUI is present. */
s32 func_0029D518(void) {
    if (g_guiInstance != 0) {
        return func_0028B038();
    }
}

/** Flush a HUD display value into the render object at g_guiInstance+0x376C8
 *  (used to push g_nBoltCounterDisplayed to the renderer). */
s32 FlushHudDisplayValue(s32 value) {
    if (g_guiInstance != 0) {
        return func_0034F060(g_guiInstance + 0x376C8, value);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3A4C0 (method GuiQuickSelectWheelTick);
 *  0 when the GUI is down. */
s32 func_0029D578(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return GuiQuickSelectWheelTick(g_guiInstance + 0x3A570, arg);
}

/** Forward to the widget at g_guiInstance+0x3A4C0 (method func_003422D0). */
s32 func_0029D5B8(void) {
    if (g_guiInstance != 0) {
        return func_003422D0(g_guiInstance + 0x3A570);
    }
}

/** Forward three args to the widget at g_guiInstance+0x36F28 (method
 *  func_0033A4A0). */
s32 func_0029D5E8(s32 arg0, s32 arg1, s32 arg2) {
    if (g_guiInstance != 0) {
        return func_0033A4A0(g_guiInstance + 0x36F28, arg0, arg1, arg2);
    }
}

/** Forward two args to the widget at g_guiInstance+0x36F28 (method
 *  func_0033A528). */
s32 func_0029D630(s32 arg0, s32 arg1) {
    if (g_guiInstance != 0) {
        return func_0033A528(g_guiInstance + 0x36F28, arg0, arg1);
    }
}

/** Forward two args to the widget at g_guiInstance+0x36F28 (method
 *  func_00339DC8). */
s32 func_0029D670(s32 arg0, s32 arg1) {
    if (g_guiInstance != 0) {
        return func_00339DC8(g_guiInstance + 0x36F28, arg0, arg1);
    }
}

/* func_0029DB50: empty jr-ra stub - installed by GuiManagerCreate as a default no-op GUI callback. */
void func_0029D6B0(void) {
}

/** Forward the PAL-mode flag (as 0/1) to func_00337AA0. */
s32 func_0029D6B8(void) {
    return func_00337AA0(D_1A7C18 != 0);
}

/* GuiManagerCreate — allocate and initialise the GuiManager singleton (EU twin
 * of USA GuiManagerCreate). Fills the manager backing buffer with 0xCD, clears
 * the 12-word GUI state block (D_138200 +0x1A0..+0x1CC), runs func_0029D6B8,
 * builds the placement allocator (GuiPlacementNew, 0x3FBD0 bytes) + GUI system
 * (GuiSystemInit, 0x40000) into g_guiInstance, then installs the pump-callback
 * pair at gui+0x38000+0x7A84/+0x7A88 (endgame variants func_0029CA68/func_0029D6B0
 * when g_playerProgress==0x1F5, else defaults func_0029CAA0/func_0029CA70).
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA GuiManagerCreate: arena anchor g_memoryArenaTable ->
 * &g_nVendorBuyQuantity+0x8C78; D_138180->D_138200 (+0x80 lane, interior
 * 0x1A0..0x1CC unchanged); func_0029DB58->func_0029D6B8; GuiPlacementNew size
 * 0x3FB20->0x3FBD0 (+0xB0 instance); callback slots +0x79D4/+0x79D8 -> +0x7A84/
 * +0x7A88 (+0xB0); func_0029CF08->func_0029CA68, func_0029DB50->func_0029D6B0,
 * func_0029CF40->func_0029CAA0, func_0029CF10->func_0029CA70; GuiSystemInit /
 * g_guiInstance / g_playerProgress kept. NOTE: like USA (which loads g_bPalMode
 * into $4 before jal func_0029DB58), EU loads D_1A7C18 into $4 before
 * jal func_0029D6B8 — identical shape, NOT a region difference. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", GuiManagerCreate);
#else
extern u8 g_nVendorBuyQuantity[];   /* arena-table anchor (+0x8C78 == USA g_memoryArenaTable) */
extern void *GuiPlacementNew(s32 size, void *buf);
extern char *GuiSystemInit(void *handle, s32 arg);
void GuiManagerCreate(void) {
    u8 *arena = (u8 *)g_nVendorBuyQuantity + 0x8C78;
    char *gui;
    void *handle;
    s32 i;

    memset(*(void **)(arena + 0x80), 0xCD, 0x40000);
    for (i = 0x1A0; i <= 0x1CC; i += 4) {
        *(s32 *)(D_138200 + i) = 0;
    }
    func_0029D6B8();
    handle = GuiPlacementNew(0x3FBD0, *(void **)(arena + 0x80));
    gui = GuiSystemInit(handle, 0x40000);
    g_guiInstance = gui;
    if (g_playerProgress == 0x1F5) {
        *(void **)(gui + 0x38000 + 0x7A84) = (void *)func_0029CA68;
        *(void **)(gui + 0x38000 + 0x7A88) = (void *)func_0029D6B0;
    } else {
        *(void **)(gui + 0x38000 + 0x7A84) = (void *)func_0029CAA0;
        *(void **)(gui + 0x38000 + 0x7A88) = (void *)func_0029CA70;
    }
}
#endif

/** If the GUI is up and the popup-busy gate (D_1A8D14) is clear, pause the
 *  game world (func_0028E9B8(1)) and run the GUI pump (func_0029C640),
 *  propagating its result. */
s32 func_0029D7D0(void) {
    if (g_guiInstance != 0) {
        if (D_1A8D14 == 0) {
            func_0028E9B8(1);
            return func_0029C640();
        }
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029D810);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029D868);

/** Forward `arg` to the widget at g_guiInstance+0x3CEA0 (method func_0033B7D0);
 *  0 when the GUI is down. */
s32 func_0029D8A0(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033B7D0(g_guiInstance + 0x3CF50, arg);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029D8E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029D910);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029D948);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", EnableDmacChannels);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", FlushPendingTexUploads);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029DBF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029DD30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", DecompressWad);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E138);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E158);

/* One per-level objective record (stride 0x28). EU twin of the USA struct. */
typedef struct LevelObjective {
    s16 id;             /* 0x00 objective id; 0 terminates the list */
    s16 cond1Sel;       /* 0x02 first EvaluateProgressCondition selector (cond) */
    s32 cond1Arg;       /* 0x04 first EvaluateProgressCondition operand (arg) */
    s16 cond2Sel;       /* 0x08 second EvaluateProgressCondition selector (cond) */
    s16 _pad0A;         /* 0x0A */
    s32 cond2Arg;       /* 0x0C second EvaluateProgressCondition operand (arg) */
    u16 flags;          /* 0x10 bit 0x2 = hidden-when-complete, 0x4 = level-gated */
    u8  _pad12[0xA];    /* 0x12 */
    s32 (*tickFn)(s32); /* 0x1C optional per-tick callback */
    s32 tickArg;        /* 0x20 argument passed to tickFn */
    s16 state;          /* 0x24 0 = inactive, 1 = active, 2 = complete */
    s16 tickResult;     /* 0x26 last tickFn return value */
} LevelObjective;

#ifdef TARGET_NATIVE
extern LevelObjective *D_258BA0[]; /* EU 0x258BA0 objective-list head table (USA D_258B20) */
extern u8 g_mapVertexData[];       /* EU 0x1C4FA0 MapCache base (+0x230 = currentLevel) */
extern u8 D_1A7C70[];              /* EU 0x1A7C70 per-level visited byte markers (USA 0x1A7BF0) */
#endif

/* Objective-scan scratch (EU 0x1B1A54): { outstanding @ +0x10C, head @ +0x110 }
 * off the g_nBoltCounterDisplayed anchor (USA g_pRainHeightmap+0x34/0x38). */
typedef struct ObjectiveScan {
    s32             outstanding; /* g_nBoltCounterDisplayed + 0x10C */
    LevelObjective *head;        /* g_nBoltCounterDisplayed + 0x110 */
} ObjectiveScan;

/* UpdateLevelObjectiveStates: EU twin. WALLED (later-cc1 register-coloring/frame
 * wall, same class as the USA twin and the unit's GatherActiveObjectives /
 * EvaluateProgressCondition); the TARGET_NATIVE arm is the faithful portable
 * body, cmp-oracle'd 69/69 on the USA twin (region-identical logic). See the USA
 * 198FA0.c doc comment for the per-pass behaviour. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", UpdateLevelObjectiveStates);
#else
s32 UpdateLevelObjectiveStates(void) {
    extern s32 EvaluateProgressCondition(s32 cond, s32 arg);  /* defined later in-unit */
    ObjectiveScan *scan =
        (ObjectiveScan *)((char *)&g_nBoltCounterDisplayed + 0x10C);
    LevelObjective *rec;

    rec = D_258BA0[*(s32 *)(g_mapVertexData + 0x230)];
    scan->head = rec;
    if (rec == NULL) {
        return 0;
    }

    /* Pass 1: score each objective from its progress conditions. */
    for (; rec->id != 0; rec++) {
        if ((rec->flags & 0x4) &&
            D_1A7C70[*(s32 *)(g_mapVertexData + 0x230)] == 0) {
            rec->state = 0;
            continue;
        }
        if (EvaluateProgressCondition(rec->cond1Sel, rec->cond1Arg) == 0) {
            rec->state = 0;
            continue;
        }
        if (EvaluateProgressCondition(rec->cond2Sel, rec->cond2Arg) != 0) {
            rec->state = 2;
        } else {
            rec->state = 1;
        }
    }

    /* Pass 2: tick callbacks. */
    for (rec = scan->head; rec->id != 0; rec++) {
        if (rec->tickFn != NULL) {
            rec->tickResult = rec->tickFn(rec->tickArg);
        }
    }

    /* Pass 3: count outstanding (active, not hidden-when-complete) objectives. */
    scan->outstanding = 0;
    for (rec = scan->head; rec->id != 0; rec++) {
        if (rec->state == 1 && (rec->flags & 0x2) == 0) {
            scan->outstanding += 1;
        }
    }

    return scan->outstanding == 0;
}
#endif

/* Progress-condition flag arrays read by EvaluateProgressCondition's 12 cases
 * (EU addresses). Declared for the TARGET_NATIVE #else arm; emit no code. */
#ifdef TARGET_NATIVE
extern u8  g_abLevelAvailableFlags[]; /* EU 0x1A7C50 per-level available (case 1) */
extern u8  g_inventoryOwned[];        /* EU 0x1A7B80 per-item have-flag (case 2) */
extern u8  g_inventoryNewFlag[];      /* EU 0x1A7BB8 per-item newly-acquired (case 3) */
extern u8  D_139638[];                /* EU 0x139638 dialog/story flag byte-array (case 6) */
extern u8  g_platinumBoltFlags[];     /* EU 0x19B2F8 per-platinum-bolt flag (case 8) */
extern s32 g_cinematicUnlockedFlags;  /* EU 0x1397E8 cinematic bitfield (case 9) */
extern s32 g_mapCurrentLevel;         /* EU current map level id (case 10) */
extern s32 func_002FD068(s32 level, s32 bitIndex); /* EU map-progress predicate (case 10
                                       * callee). EU asm 0x2FD068: `daddu $s0,$a1,$zero` in
                                       * the delay slot of `jal LookupIdValuePair`, then
                                       * `sllv $a0,$a0,$s0` -- $a1 IS the shift amount.
                                       * Derived from the EU listing, not from USA. */
#endif

/* Per-weapon upgrade record (EU 0x139AA8 base; level field at +0xC). */
typedef struct WeaponUpgradeRecord {
    s32 _pad0[3];
    s32 upgradeLevel;
} WeaponUpgradeRecord;
#ifdef TARGET_NATIVE
extern WeaponUpgradeRecord D_139AA8[]; /* EU 0x139AA8 per-weapon upgrade table (stride 0x10) */
#endif

/* EvaluateProgressCondition(cond, arg): EU twin of the USA progress-condition
 * evaluator. `cond` is a sign-extended 16-bit selector; each case (0..11)
 * returns a 0/1 truth value (case 10 returns the map predicate verbatim); any
 * cond outside [0,11] returns 0. WALLED by the jump-table reloc-identity gap +
 * the case-9 bit-test lowering residue (best 95% under the unit recipe; see the
 * USA unit comment). Portable #else mirrors the USA body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", EvaluateProgressCondition);
#else
s32 EvaluateProgressCondition(s32 cond, s32 arg) {
    cond = (s32)(s16)cond;
    if ((u32)cond >= 0xC) {
        return 0;
    }
    switch (cond) {
    case 0:
        return 1;
    case 1:
        return g_abLevelAvailableFlags[arg] != 0;
    case 2:
        return g_inventoryOwned[arg] != 0;
    case 3:
        return g_inventoryNewFlag[arg] != 0;
    case 4:
        if (arg >= 0x72) {
            return 0;
        }
        return D_139AA8[arg].upgradeLevel != 0;
    case 5:
        if (arg >= 0x72) {
            return 0;
        }
        return (D_139AA8[arg].upgradeLevel < 2) ? 0 : 1;
    case 6:
        return D_139638[arg] != 0;
    case 7:
        return ((s32 (*)(void))arg)() != 0;
    case 8:
        return g_platinumBoltFlags[(arg & 0xFFFF) + ((arg >> 16) * 4)] != 0;
    case 9: {
        s32 word = *(s32 *)((char *)&g_cinematicUnlockedFlags + ((arg >> 2) << 2));
        return (word & (1 << (arg & 0x1F))) != 0;
    }
    case 10:
        /* EU asm 0x29E49C: the call site sets only $4 (`lw $4,%lo(g_mapCurrentLevel)`),
         * and NOTHING in EU EvaluateProgressCondition writes $a1 -- 0 writes, measured --
         * so $a1 arrives as the incoming `arg`. */
        return func_002FD068(g_mapCurrentLevel, arg);
    default: /* case 11 */
        return 0;
    }
}
#endif

/* GatherActiveObjectives(outIds, outMask, outVals, wantValues): EU twin of USA
 * GatherActiveObjectives. Walks the 0x28-stride objective list (head pointer at
 * &g_nBoltCounterDisplayed+0x110, the EU objective-scan anchor also used by the
 * landed UpdateLevelObjectiveStates), emits visible objective ids/values, sets
 * completion bits in *outMask (bit 31 = all non-hidden complete), returns count.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA GatherActiveObjectives: head anchor g_pRainHeightmap+0x38
 * -> &g_nBoltCounterDisplayed+0x110; all rec field offsets (0x0/0x10/0x12/0x14/
 * 0x24/0x26, stride 0x28) identical.
 * REGION DIFFERENCE: the "completed objective" display sentinel is 0x31B9 in USA
 * but 0x112D in EU (EU .s: addiu $2,$0,0x112D at 0x29E55C) — a PAL/NTSC text/
 * message-id difference, modeled faithfully below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", GatherActiveObjectives);
#else
s32 GatherActiveObjectives(s32 *outIds, s32 *outMask, s32 *outVals, s32 wantValues) {
    u8 *rec = *(u8 **)((char *)&g_nBoltCounterDisplayed + 0x110); /* objective list head */
    s32 count = 0;
    s32 allComplete = 1;

    *outIds = 0;
    if (outMask != 0) {
        *outMask = 0;
    }
    if (outVals != 0) {
        *outVals = -1;
    }
    if (rec == 0) {
        return 0;
    }

    while (*(s16 *)(rec + 0x0) != 0) {           /* rec->id */
        s32 state = *(s16 *)(rec + 0x24);
        u16 flags = *(u16 *)(rec + 0x10);

        if (state != 2 && (flags & 2) == 0) {
            allComplete = 0;                     /* a visible, not-complete one */
        }
        if ((flags & 2) != 0) {                  /* hidden -> skip, no count */
            rec += 0x28;
            continue;
        }
        if (state == 0) {                        /* inactive -> skip, no count */
            rec += 0x28;
            continue;
        }
        if ((flags & 1) != 0 && state == 2) {    /* completed+flag1 -> skip */
            rec += 0x28;
            continue;
        }

        outIds[0] = *(s16 *)(rec + 0x0);
        if (wantValues != 0) {
            if (state == 2) {
                outIds[0] = 0x112D;              /* EU sentinel (USA 0x31B9) */
            } else {
                outIds[0] = *(s16 *)(rec + *(s16 *)(rec + 0x26) * 2 + 0x14);
            }
        }
        if (outMask != 0 && state == 2) {
            *outMask |= (1 << count);            /* completion bit for this slot */
        }
        if (outVals != 0) {
            *outVals = *(s16 *)(rec + 0x12) + *(s16 *)(rec + 0x26);
            outVals++;
        }
        outIds++;
        count++;
        rec += 0x28;
    }

    if (outMask != 0 && allComplete != 0) {
        *outMask |= 0x80000000;                  /* bit 31 = all non-hidden complete */
    }
    return count;
}
#endif

/* func_0029E5F0 (EU twin of USA func_0029EA90): returns 1 iff the misc-extras
 * gate is clear (g_miscExtras==0) AND the D_139860 busy bit (0x8000000) is set;
 * 0 otherwise. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0029EA90: D_1397E0->D_139860 (+0x80 lane); g_miscExtras
 * kept (file-scope). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E5F0);
#else
extern s32 D_139860;
s32 func_0029E5F0(void) {
    if (g_miscExtras != 0) {
        return 0;
    }
    if ((D_139860 & 0x8000000) != 0) {
        return 1;
    }
    return 0;
}
#endif

/* func_0029E628 (EU twin of USA func_0029EAC8): returns 1 iff all three dialog
 * flags are set (D_139641, D_1A7B8D, D_1A7B94); 0 as soon as any is clear.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0029EAC8: D_1395C1->D_139641, D_1A7B0D->D_1A7B8D,
 * D_1A7B14->D_1A7B94 (all +0x80 lane, lbu/u8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E628);
#else
extern u8 D_139641;
extern u8 D_1A7B8D;
extern u8 D_1A7B94;
s32 func_0029E628(void) {
    if (D_139641 == 0) {
        return 0;
    }
    if (D_1A7B8D == 0) {
        return 0;
    }
    if (D_1A7B94 != 0) {
        return 1;
    }
    return 0;
}
#endif

/* func_0029E668 (EU twin of USA func_0029EB08): returns 1 iff a stream is in
 * flight (D_139844 < 0) AND the busy flag D_139655 is set; 0 otherwise.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0029EB08: D_1397C4->D_139844 (s32, sign test),
 * D_1395D5->D_139655 (u8) (both +0x80 lane). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E668);
#else
extern s32 D_139844;
extern u8 D_139655;
s32 func_0029E668(void) {
    if (D_139844 >= 0) {
        return 0;
    }
    if (D_139655 != 0) {
        return 1;
    }
    return 0;
}
#endif

/* func_0029E698 (EU twin of USA func_0029EB38): returns 1 iff both dialog flags
 * are set (D_1A7C5D AND D_1A7B90); 0 otherwise. Matching arm stays INCLUDE_ASM;
 * #else is the structure model.
 * Word-verified vs USA func_0029EB38: D_1A7BDD->D_1A7C5D, D_1A7B10->D_1A7B90
 * (both +0x80 lane, lbu/u8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E698);
#else
extern u8 D_1A7C5D;
extern u8 D_1A7B90;
s32 func_0029E698(void) {
    if (D_1A7C5D == 0) {
        return 0;
    }
    if (D_1A7B90 != 0) {
        return 1;
    }
    return 0;
}
#endif

/* func_0029E6C8 (EU twin of USA func_0029EB68): cinematic skip-prompt arbiter.
 * Posts HUD message 0x1F/0x20 into D_257582 and returns 0/1/2 from the
 * D_139638[0x1D] story flag / cinematic word (&g_cinematicUnlockedFlags+0x5C)
 * pair. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0029EB68: D_1395B8->D_139638 (file-scope; +0x80 lane),
 * D_257502->D_257582 (+0x80 lane); g_cinematicUnlockedFlags kept (file-scope). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E6C8);
#else
extern s16 D_257582;   /* cinematic-unlock status code (0x1F / 0x20) */
s32 func_0029E6C8(void) {
    /* cinematic word sits at &g_cinematicUnlockedFlags + 0x5C (region-anchor
     * sibling global; same base-relative access as the +0x90/+0x98 readers). */
    s32 cinWord = *(s32 *)((char *)&g_cinematicUnlockedFlags + 0x5C);

    if (D_139638[0x1D] == 0) {
        D_257582 = 0x1F;                    /* story flag clear */
        return (cinWord >= 0) ? 0 : 2;      /* cinematic ready vs still locked */
    }
    if (cinWord >= 0) {
        D_257582 = 0x20;                    /* story flag set + cinematic ready */
        return 1;
    }
    return 0;                               /* story flag set, cinematic locked */
}
#endif

/* func_0029E758 — EU twin of USA func_0029EBF8: dialog-skip arbiter (0/1/2 form)
 * on the D_1A7C5D / D_1A7B90 flag pair; posts skip-code 0x28/0x29 into D_257A32.
 * Distinct from the landed func_0029E698 (USA func_0029EB38) 0/1 form: that one
 * shares the same two flags but posts no status code and returns only 0/1.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0029EBF8: D_1A7BDD->D_1A7C5D, D_1A7B10->D_1A7B90,
 * D_2579B2->D_257A32 (all +0x80 data lane). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E758);
#else
extern u8  D_1A7C5D;   /* first dialog flag  (USA D_1A7BDD, +0x80) */
extern u8  D_1A7B90;   /* second dialog flag (USA D_1A7B10, +0x80) */
extern s16 D_257A32;   /* dialog-skip status code, 0x28/0x29 (USA D_2579B2, +0x80) */
s32 func_0029E758(void) {
    if (D_1A7C5D == 0) {
        if (D_1A7B90 == 0) {
            D_257A32 = 0x28;   /* neither flag: base skip code */
            return 0;
        }
        D_257A32 = 0x29;       /* only the second flag set */
        return 2;
    }
    if (D_1A7B90 == 0) {
        D_257A32 = 0x29;       /* only the first flag set */
        return 1;
    }
    return 0;                  /* both set: no skip, D_257A32 unchanged */
}
#endif

/** func_0029E7D0 — EU twin of USA func_0029EC70. Returns 1 iff cinematic-flag
 *  word +0x90 has bit 0x10000 set AND word +0x98 has bit 0x4000000 set; 0
 *  otherwise. (The original's redundant early-out on the same two bits cannot
 *  change the result, so the #else collapses to the single conjunction.)
 *  Matching arm stays INCLUDE_ASM; #else is the structure model.
 *  Word-verified vs USA func_0029EC70: g_cinematicUnlockedFlags kept (delta 0,
 *  file-scope). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E7D0);
#else
s32 func_0029E7D0(void) {
    char *flags = (char *)&g_cinematicUnlockedFlags;
    if ((*(s32 *)(flags + 0x90) & 0x10000) == 0) {
        return 0;
    }
    if ((*(s32 *)(flags + 0x98) & 0x4000000) != 0) {
        return 1;
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E840);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029F958);

/* SpawnMoby — EU twin of USA SpawnMoby: allocate + initialise a moby from
 * the spawn free-list. Scans the moby table [start..end) (stride 0x100) for a
 * free/dead slot (state +0x20 >= 0xFE) whose reuse timer (+0xA0) has elapsed
 * (g_gameTime >= it). On a hit: stamps +0x120 when fully dead (0xFF), inits it
 * from classId via InitMobyFromClass, binds+zero-fills its 0x80 aux block, decrements
 * the spawn credit, and returns the moby. Returns NULL (debug print) when full.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA SpawnMoby: g_mobySpawnStart->&g_nBoltCounterDisplayed+0x218,
 * g_mobyTableEnd->+0x21C, g_mobyAuxBlockBase->+0x224, g_mobySpawnCredit->+0x138,
 * g_gameTime->&g_nLevelExitDestination+0x8, D_1A9DC8->D_1A9E48 (+0x80),
 * InitMobyFromClass->InitMobyFromClass, DebugPrintStub->func_0026FD28. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", SpawnMoby);
#else
extern s32  g_nLevelExitDestination;  /* g_gameTime sits at +0x8 (EU anchor) */
extern char D_1A9E48[];               /* "moby table full" debug string (USA D_1A9DC8, +0x80) */
extern void InitMobyFromClass(void *moby, s32 classId);  /* InitMobyFromClass twin */

void *SpawnMoby(s32 classId) {
    u8 *moby = *(u8 **)((char *)&g_nBoltCounterDisplayed + 0x218);
    u8 *end  = *(u8 **)((char *)&g_nBoltCounterDisplayed + 0x21C);

    while (moby < end) {
        u8 state = moby[0x20];
        if (state >= 0xFE &&
            *(u32 *)((char *)&g_nLevelExitDestination + 0x8) >= *(u32 *)(moby + 0xA0)) {
            if (state == 0xFF) {
                moby[0x120] = 0xFF;
            }
            InitMobyFromClass(moby, classId);
            {
                s32 slot = (s32)(moby - *(u8 **)((char *)&g_nBoltCounterDisplayed + 0x218)) >> 8;
                u8 *aux = *(u8 **)((char *)&g_nBoltCounterDisplayed + 0x224) + slot * 0x80;
                *(u8 **)(moby + 0x68) = aux;
                FillMemory32(aux, 0, 0x80);
            }
            {
                s32 *credit = (s32 *)((char *)&g_nBoltCounterDisplayed + 0x138);
                if (*credit != 0) {
                    (*credit)--;
                }
            }
            return moby;
        }
        moby += 0x100;
    }
    func_0026FD28(D_1A9E48);   /* asm also passes g_gameTime; the stub ignores it */
    return 0;
}
#endif

/* InitMobyFromClass — EU twin of USA InitMobyFromClass: zero-fills a moby's 0x100-byte
 * state (FillMemory32) and binds it to a class. Stamps the spawn-path defaults
 * (classSlot +0x22, alpha +0x23, tint qword +0x38, uid +0xAC, classId +0xAA, the
 * 0x7F/0x80 colour bytes, 1.0 anim rates +0x48/+0x4C). Validates the slot through
 * the reverse map: if it does NOT round-trip to classId the class has no loaded
 * header -> headerless path (flags |= 5, pUpdate from the no-header table, |= 2
 * when none). Otherwise binds the header (pClass +0x24, pUpdate, scale +0x2C,
 * header flag bits, optional collision mesh +0x78) and resolves anim-frame ptrs.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 *
 * REGION DIFFERENCE (PAL/NTSC) — see the trailing `else` branch: the USA build
 * jumps straight to the epilogue in the (pc[0x0C]==1 and animSet[0x10]>=2) case
 * (no-op). The PAL build scales the default anim rate m[0x48] to 5/6 (= 50/60,
 * 0x3F555556) when the class has no update function. FLAGGED for docs/ARCHITECTURE.md.
 *
 * Word-verified vs USA InitMobyFromClass: g_mobyClassSlotRemap->&g_mapTextureWidth
 * +0x9300, g_mobyClassSlotToId->+0x9120, g_mobyClassUpdateFuncs->+0x8D60,
 * g_mobyClassHeaders->+0x89A0, g_mobyClassUpdateFuncsNoHeader->&D_001D009D+0x443,
 * g_mobyTableBase->&g_nBoltCounterDisplayed+0x214, ResolveMobyAnimFramePtrs->
 * ResolveMobyAnimFramePtrs, FillMemory32 kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", InitMobyFromClass);
#else
extern u8   g_mapTextureWidth;          /* EU anchor for the moby-class binding tables */
extern u8   D_001D009D;                 /* EU anchor for the headerless pUpdate table */
extern void ResolveMobyAnimFramePtrs(void *moby);  /* ResolveMobyAnimFramePtrs twin */

void InitMobyFromClass(void *moby, s32 classId) {
    u8   *m         = (u8 *)moby;
    u8   *slotRemap = (u8   *)((char *)&g_mapTextureWidth + 0x9300); /* classId -> slot */
    s16  *slotToId  = (s16  *)((char *)&g_mapTextureWidth + 0x9120); /* slot -> classId */
    void **updHdr   = (void **)((char *)&g_mapTextureWidth + 0x8D60);
    void **hdrTable = (void **)((char *)&g_mapTextureWidth + 0x89A0);
    void **updNoHdr = (void **)((char *)&D_001D009D + 0x443);
    u8    slot;
    s32   index;
    u8   *pc;
    void *animSet;

    FillMemory32(m, 0, 0x100);

    slot = slotRemap[classId];
    m[0x23] = 0x80;                 /* alpha */
    m[0x22] = slot;                 /* class slot */
    m[0xA8] = 0xFF;
    m[0x21] = 0xFF;
    m[0x61] = 0xFF;
    m[0x62] = 0xFF;
    *(u64 *)(m + 0x38) = 0x0040404000000000ULL; /* default tint qword */
    *(s16 *)(m + 0x36) = 0x7F80;
    index = (s32)(m - *(u8 **)((char *)&g_nBoltCounterDisplayed + 0x214)) >> 8;
    *(s32 *)(m + 0xAC) = index << 16;            /* uid */
    m[0x6D] = 0xFF;
    m[0xA5] = 0x7F;
    m[0xA7] = 0x80;
    *(s16 *)(m + 0xAA) = (s16)classId;
    m[0x6E] = 0;
    m[0x6C] = 0xFF;
    m[0xA4] = 0x7F;
    m[0xA6] = 0x80;

    if (slotToId[slot] != classId) {
        /* headerless class - no loaded header for this slot */
        u16 flags = (u16)(*(u16 *)(m + 0x34) | 0x5);
        void *upd = updNoHdr[slot];
        *(s32 *)(m + 0x24) = 0;
        *(s32 *)(m + 0x98) = 0;
        *(void **)(m + 0x64) = upd;
        if (upd == 0) {
            flags |= 0x2;
        }
        *(u16 *)(m + 0x34) = flags;
        return;
    }

    /* header class */
    {
        void *upd = updHdr[slot];
        *(void **)(m + 0x64) = upd;
        if (upd == 0) {
            *(u16 *)(m + 0x34) |= 0x2;
        }
    }
    pc = (u8 *)hdrTable[slot];
    *(void **)(m + 0x24) = pc;
    m[0x62] = pc[0x0E];
    *(u16 *)(m + 0x34) |= *(u16 *)(pc + 0x44);
    *(s32 *)(m + 0x98) = *(s32 *)(pc + 0x10);
    *(f32 *)(m + 0x4C) = 1.0f;
    *(f32 *)(m + 0x2C) = *(f32 *)(pc + 0x24);    /* default scale */
    *(f32 *)(m + 0x48) = 1.0f;
    if (*(s32 *)(pc + 0x40) != 0) {
        *(u16 *)(m + 0x34) |= 0x10;
        *(s32 *)(m + 0x78) = *(s32 *)(pc + 0x40); /* collision mesh */
    }

    pc = *(u8 **)(m + 0x24);
    if (pc[0x0F] != 0) {
        m[0x6F] = 0x18;
        *(s32 *)(m + 0x70) = 0;
        *(u16 *)(m + 0x34) |= 0x400;
        *(s32 *)(m + 0x74) = 0;
        m[0xBD] = 0;
    }

    pc = *(u8 **)(m + 0x24);
    if (pc[0x06] != 0) {
        m[0x63] = 0x18;
    }

    pc = *(u8 **)(m + 0x24);
    animSet = *(void **)(pc + 0x48);
    if (animSet != 0) {
        ResolveMobyAnimFramePtrs(m);
        pc = *(u8 **)(m + 0x24);
        animSet = *(void **)(pc + 0x48);
        if (*(u8 *)((u8 *)animSet + 0x10) >= 2) {
            *(u16 *)(m + 0x34) &= 0xFFFD;
        }
        pc = *(u8 **)(m + 0x24);
        if (pc[0x0C] == 1) {
            animSet = *(void **)(pc + 0x48);
            if (*(u8 *)((u8 *)animSet + 0x10) < 2) {
                *(s32 *)(m + 0x48) = 0;
                animSet = *(void **)(pc + 0x48);
                if (*(s8 *)((u8 *)animSet + 0x11) < 0) {
                    *(u16 *)(m + 0x34) |= 0x40;
                }
            } else {
                /* PAL-only: NTSC/USA is a no-op here. Scale the default anim
                 * rate to 50/60 (5/6) when the class has no update function. */
                if (*(void **)(m + 0x64) == 0) {
                    *(f32 *)(m + 0x48) = 0.83333337f; /* 0x3F555556 = 50/60. NOT the
                     * correctly-rounded 5/6: that is 0x3F555555, one ULP LOW. The ROM
                     * value sits 1 ULP above it (PAL constants were derived as
                     * USA x 1.2 in float). Spell to the ROM bits; never derive. */
                }
            }
        }
    }
}
#endif

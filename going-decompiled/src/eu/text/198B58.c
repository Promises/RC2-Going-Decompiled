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
extern s32 D_1A8D38;
extern s32 D_1A8D3C;

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
extern s32 func_00348DF8(void *widget, s32 arg);
extern s32 func_003480C8(void *widget, s32 arg0, s32 arg1);
extern s32 func_0033B218(void *widget, s32 arg);
extern s32 func_0033B4B8(void *widget);
extern s32 func_0033BE50(void *widget, s32 arg);
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
extern s32 func_00341C18(void *widget, s32 arg);
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
/** Reset the save/load popup to status 3 (idle/none) and clear the context
 *  transaction-active + save-pending flags. */
void func_00298BD8(void) {
    g_nSaveLoadStatusCode[0] = 3;
    D_139460.dirty = 0;
    D_139460.busy = 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298BF8);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00298F90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299030);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299068);

/** If a card transaction finished selecting (mode 2) with no result yet,
 *  force result 9 (cancelled) and show popup status 0xF. */
void func_002990E0(void) {
    if (D_139460.mode == 2 && D_139460.result < 0) {
        D_139460.result = 9;
        D_139460.subResult = 0;
        g_nSaveLoadStatusCode[0] = 0xF;
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299120);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299198);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299310);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", UpdateSaveTaskState);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299488);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_002994D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_00299518);

/* Latched event flag at g_nBoltCounterDisplayed+0xF4 (EU equivalent of the
 * USA g_pSkyShellSpinRates+0xAC latch): an unrelated bss word; aliased via a
 * gas symbol equate because no symbol exists at that address. */
extern s32 g_savePromptLatch;
__asm__("g_savePromptLatch = g_nBoltCounterDisplayed+0xF4");

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", BuildSaveGamePaths);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_002996A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_002996C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_002997A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", SaveLoadStateMachine);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", BuildSaveImage);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029B848);

extern s32 func_0029B848(void *buf, s32 len); /* EU ComputeSaveSectionsCrc16 */
extern void FillMemory32(void *dst, s32 pattern, s32 nbytes);
extern void *func_00283370(void *dst, const void *src, s32 nbytes); /* EU byte memcpy */

/* func_0029B8F0(image): EU twin of USA func_0029BD48/VerifySaveHeaderChecksum.
 * The image header is { s32 payloadLen; s32 storedCrc; payload[payloadLen] };
 * recompute the CRC (func_0029B848 from image+8 over payloadLen) and return 1
 * iff it equals the stored CRC. Stored CRC 0 -> empty/invalid -> 0.
 *
 * WALLED at the byte level by the 8-byte-packed callee-save frame (see
 * project_matching_ceiling); the portable #else mirrors the USA body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029B8F0);
#else
s32 func_0029B8F0(void *image) {
    s32 storedCrc = ((s32 *)image)[1];
    if (storedCrc == 0) {
        return 0;
    }
    return func_0029B848((char *)image + 8, ((s32 *)image)[0]) == storedCrc;
}
#endif

/* SerializeSaveSections(dst, slot, table): EU twin of the USA serializer. Emits
 * the save image { s32 payloadLen; s32 crc; <sections> } into dst; each section
 * emits an 8-byte header { tag, len } then `len` payload bytes (4-byte aligned);
 * tag 0x1770 zero-fills (FillMemory32), else memcpy (func_00283370) from
 * srcPtr+len*slot. Closes with a { -1, 0 } terminator and stores the payload CRC
 * (func_0029B848) in the header. Returns the total image size (payloadLen + 8).
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
    ((s32 *)dst)[1] = func_0029B848((char *)dst + 8, size);
    ((s32 *)dst)[0] = size;
    return size + 8;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029BA48);

extern int memcmp(); /* K&R decl: avoids the ee-gcc builtin-prototype conflict warning */
extern s32 D_1A9A20;  /* EU 0x1A9A20 changed-section counter (= USA D_1A99A0) */
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

    if (func_0029B8F0(image) == 0) {
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", CommitProgressCheckpoint);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029BFF0);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C158);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C1A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C1E0);

/** Forward to the widget at g_guiInstance+0x36F28 (method func_003506A0). */
s32 func_0029C1F0(void) {
    if (g_guiInstance != 0) {
        return func_003506A0(g_guiInstance + 0x36F28);
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C220);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C2A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C3C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C498);

/** Invalidate the D_1A9B10 GUI state (set to -1). */
void func_0029C630(void) {
    D_1A9B10 = -1;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C640);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C7F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029C820);

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

/** Forward `arg` to the widget at g_guiInstance+0x39620 (method func_00348DF8);
 *  0 when the GUI is down. */
s32 func_0029CBA0(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00348DF8(g_guiInstance + 0x39620, arg);
}

/** Forward two args to the widget at g_guiInstance+0x39AF0 (method
 *  func_003480C8); 0 when the GUI is down. */
s32 func_0029CBE0(s32 arg0, s32 arg1) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003480C8(g_guiInstance + 0x39BA0, arg0, arg1);
}

/** Forward `arg` to the widget at g_guiInstance+0x3B418 (method func_0033B218);
 *  0 when the GUI is down. */
s32 func_0029CC28(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033B218(g_guiInstance + 0x3B4C8, arg);
}

/** Forward to the widget at g_guiInstance+0x3B418 (method func_0033B4B8). */
s32 func_0029CC68(void) {
    if (g_guiInstance != 0) {
        return func_0033B4B8(g_guiInstance + 0x3B4C8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3AF80 (method func_0033BE50);
 *  0 when the GUI is down. */
s32 func_0029CC98(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033BE50(g_guiInstance + 0x3B030, arg);
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

/** Forward `arg` to the widget at g_guiInstance+0x3A4C0 (method func_00341C18);
 *  0 when the GUI is down. */
s32 func_0029D578(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00341C18(g_guiInstance + 0x3A570, arg);
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", GuiManagerCreate);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029D9E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029DA78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029DBF0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029DD30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029DE68);

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

extern LevelObjective *D_258BA0[]; /* EU 0x258BA0 objective-list head table (USA D_258B20) */
extern u8 g_mapVertexData[];       /* EU 0x1C4FA0 MapCache base (+0x230 = currentLevel) */
extern u8 D_1A7C70[];              /* EU 0x1A7C70 per-level visited byte markers (USA 0x1A7BF0) */

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
extern u8  g_abLevelAvailableFlags[]; /* EU 0x1A7C50 per-level available (case 1) */
extern u8  g_inventoryOwned[];        /* EU 0x1A7B80 per-item have-flag (case 2) */
extern u8  g_inventoryNewFlag[];      /* EU 0x1A7BB8 per-item newly-acquired (case 3) */
extern u8  D_139638[];                /* EU 0x139638 dialog/story flag byte-array (case 6) */
extern u8  g_platinumBoltFlags[];     /* EU 0x19B2F8 per-platinum-bolt flag (case 8) */
extern s32 g_cinematicUnlockedFlags;  /* EU 0x1397E8 cinematic bitfield (case 9) */
extern s32 g_mapCurrentLevel;         /* EU current map level id (case 10) */
extern s32 func_002FD068(s32 level);  /* EU map-progress predicate (case 10 callee) */

/* Per-weapon upgrade record (EU 0x139AA8 base; level field at +0xC). */
typedef struct WeaponUpgradeRecord {
    s32 _pad0[3];
    s32 upgradeLevel;
} WeaponUpgradeRecord;
extern WeaponUpgradeRecord D_139AA8[]; /* EU 0x139AA8 per-weapon upgrade table (stride 0x10) */

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
        return func_002FD068(g_mapCurrentLevel);
    default: /* case 11 */
        return 0;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", GatherActiveObjectives);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E5F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E628);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E668);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E698);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E6C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E758);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E7D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029E840);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029F958);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029F968);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/198B58", func_0029FA60);

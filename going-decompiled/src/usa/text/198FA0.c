#include "common.h"

/*
 * text/198FA0 — save-game + GUI-manager wrapper unit (pilot c-carve out of the
 * text/16E980 asm segment).
 *
 * The matcher builds THIS unit at -O2 -G8 -fno-gcse (see the per-unit GFLAG/
 * CC1EXTRA override in tools/ee/objdiff_build.sh / diff.sh / build.sh): the
 * gameplay-text TUs were built by a later SN cc1 without the load-PRE pass
 * (-fno-gcse reproduces it), and the g_guiInstance forwarding wrappers need
 * the cc1-small / assembler-absolute extern model below to reproduce the
 * original prologue scheduling (addiu $sp BEFORE the adjacent lui/lw pair).
 *
 * -G8 extern-sizing rules for this file (same as text/1907F0):
 *   - a complete extern object of size <= 8 bytes is placed in small data
 *     (gp-relative access);
 *   - an object the original reads with the ADJACENT lui/%lo "assembler
 *     macro" shape is declared as a small scalar/pointer PLUS a file-scope
 *     `.extern sym,16` override: cc1 then emits the one-insn symbolic macro
 *     (so it schedules like one insn, matching the SN codegen) and GNU as
 *     expands it absolutely.
 */

/* Original cc1-small / assembler-absolute symbols (see header). */
__asm__(".extern g_guiInstance, 16");
__asm__(".extern g_bPalMode, 16");
__asm__(".extern g_loadedArmorVariant, 16");
__asm__(".extern g_loadedHeldItemModelId, 16");
__asm__(".extern g_levelDialogToc, 16");

/* Singleton GUI-manager instance (0x3FB20-byte object allocated by
 * GuiManagerCreate; null until the GUI is up). Declared as a plain byte
 * pointer here because the wrappers below only ever forward `instance +
 * fixed-widget-offset` to the widget methods. */
extern char *g_guiInstance;
extern s32 g_bPalMode; /* PAL video-mode flag (cleared at boot in the USA build) */
extern volatile s32 g_loadedArmorVariant;     /* armor variant whose model is resident (-1 = none) */
extern volatile s32 g_loadedHeldItemModelId;  /* held-item model resident in the dialog scene (-1 = none) */
extern char g_levelDialogToc;                 /* level dialog/scene TOC block (byte-addressed here) */
extern s32 D_1A790C;                          /* small-data flag cleared by func_0029C418 */
extern s32 D_1A9A90;                          /* small-data GUI state, set to -1 by func_0029CA88 */

/* One entry of a save-section descriptor table. The serialized layout each
 * entry contributes is an 8-byte header followed by `len` payload bytes,
 * padded up to a 4-byte boundary. The table is terminated by an entry whose
 * srcPtr is NULL. (Stride 0x10; tag/_pad carry per-section metadata used by
 * the (de)serializers, not by the size calculation.) */
typedef struct SaveSection {
    void *srcPtr;
    s32   len;
    s32   tag;
    s32   _pad;
} SaveSection;

/* GUI widget methods the g_guiInstance wrappers forward to (text/1A00F0
 * range). All are value-returning (which keeps this cc1 from sibling-call
 * optimising the forwarding tail into a plain `j`). */
extern s32 func_0034F1C0(void *widget);
extern s32 func_0034F240(void *widget);
extern s32 func_00349C00(void *widget);
extern s32 UpdatePopupMenu(void *popup, s32 arg);
extern s32 func_00348628(void *widget);
extern s32 func_00347550(void *widget);
extern s32 func_00347B88(void *widget, s32 arg);
extern s32 func_00346EF0(void *widget, s32 arg0, s32 arg1);
extern s32 func_0033A368(void *widget, s32 arg);
extern s32 func_0033A5D8(void *widget);
extern s32 func_0033AF70(void *widget, s32 arg);
extern s32 func_0033B428(void *widget);
extern s32 func_0033C958(void *widget);
extern s32 func_0033C588(void *widget, s32 arg);
extern s32 func_0033FEF8(void *widget);
extern s32 func_0033FAB0(void *widget, s32 arg);
extern s32 func_003462C0(void *widget);
extern s32 func_003453D0(void *widget, s32 arg);
extern s32 func_00342B30(void *widget);
extern s32 func_003427D0(void *widget, s32 arg);
extern s32 func_00344808(void *widget);
extern s32 func_003446B8(void *widget, s32 arg);
extern s32 func_0033F670(void *widget);
extern s32 func_0033F610(void *widget, s32 arg);
extern s32 func_0033E9B8(void *widget);
extern s32 func_0033E8B0(void *widget, s32 arg);
extern s32 func_0033D5D8(void *widget, s32 arg);
extern s32 func_0033D780(void *widget);
extern s32 func_0033DB60(void *widget, s32 arg);
extern s32 func_0033DC68(void *widget);
extern s32 func_0033E5E8(void *widget, s32 arg);
extern s32 func_0033E680(void *widget);
extern s32 func_0033CEE0(void *widget, s32 arg);
extern s32 func_0033D070(void *widget);
extern s32 func_0033D320(void *widget, s32 arg);
extern s32 func_0033D3C0(void *widget);
extern s32 func_0033F3F0(void *widget, s32 arg);
extern s32 func_0033F478(void *widget);
extern s32 func_0033E070(void *widget, s32 arg);
extern s32 func_0033E308(void *widget);
extern s32 func_0033EC80(void *widget, s32 arg);
extern s32 func_0033ED18(void *widget);
extern s32 func_003433D0(void *widget, s32 arg);
extern s32 func_00343668(void *widget);
extern s32 func_00342DF8(void *widget, s32 arg);
extern s32 func_00342E48(void *widget);
extern s32 UpdateBoltCounterHud(void);
extern s32 func_0028B0B0(void);
extern s32 func_0034DBD8(void *widget, s32 value);
extern s32 func_00340AA8(void *widget, s32 arg);
extern s32 func_00341160(void *widget);
extern s32 func_003395F0(void *widget, s32 arg0, s32 arg1, s32 arg2);
extern s32 func_00339678(void *widget, s32 arg0, s32 arg1);
extern s32 func_00338F18(void *widget, s32 arg0, s32 arg1);
extern s32 func_00336BC8(s32 palMode);
extern s32 func_0034DB68(void *widget, s32 arg);
extern s32 func_0034D828(void *widget, s32 arg0, s32 arg1);
extern s32 GuiScreenSetEventAndReveal(void *screen, s32 eventId);
extern s32 func_0034F200(void *widget);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299020);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299040);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299128);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299150);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299178);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002991E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299238);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002992B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002992E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299348);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299398);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002993D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299478);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002994B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299528);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299568);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002995E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299698);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002996C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002996E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299708);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299730);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299758);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002997C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_002998D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299918);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299960);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299968);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299980);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", BuildSaveGamePaths);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299B00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299B18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_00299BF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SaveLoadStateMachine);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", BuildSaveImage);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", InitMemCardLib);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BCA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BD48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SerializeSaveSections);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029BEA0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", DeserializeSaveSections);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", CommitProgressCheckpoint);

/** Reset the dialog-scene model bookkeeping: mark the armor-variant and
 *  held-item models unloaded (-1), clear the two cached entries inside the
 *  level dialog TOC block (+0x13C8/+0x13E0), and clear the D_1A790C flag.
 *  (The TOC entries are written through a volatile pointer purely to pin the
 *  original interleaved store order — there is no concurrency here.) */
void func_0029C418(void) {
    volatile s32 *p;
    g_loadedArmorVariant = -1;
    p = (volatile s32 *)(&g_levelDialogToc + 0x13B0);
    p[6] = -1;
    g_loadedHeldItemModelId = -1;
    p[12] = -1;
    D_1A790C = 0;
}

/* func_0029C448(a,b): forward two args to the widget at g_guiInstance+0x36F28
 * (method func_00339398) — same shape as the matched func_0029DAD0. Best
 * attempt 97.67% (every insn/reloc exact): the residue is a pure $v0/$v1
 * register-coloring swap — the original (later SN) cc1 colours the gui load
 * $v0 and the arg copy $v1 HERE while colouring the identical shape the other
 * way in func_0029DAD0/func_0029DB10; no source shape found that flips it
 * (local-gui, copy-first, bare-return variants all probed). Same coloring
 * wall as func_002911F0. Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C448);

/** Forward `arg` to the HUD render object at g_guiInstance+0x376C8 (method
 *  func_0034DB68). */
s32 func_0029C488(s32 arg) {
    if (g_guiInstance != 0) {
        return func_0034DB68(g_guiInstance + 0x376C8, arg);
    }
}

/** Forward two args to the HUD render object at g_guiInstance+0x376C8 (method
 *  func_0034D828). */
s32 func_0029C4C0(s32 arg0, s32 arg1) {
    if (g_guiInstance != 0) {
        return func_0034D828(g_guiInstance + 0x376C8, arg0, arg1);
    }
}

/* func_0029C500: 8 bytes of dead inter-function fill (two `addiu $sp,+0x10`
 * epilogue orphans), not compiler-reachable C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C500);

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
 *  original descending store order; func_0029CA98/func_0029CD18 invoke these
 *  slots via jalr). */
void func_0029C548(s32 a, s32 b) {
    char *gui = g_guiInstance;
    if (gui != 0) {
        *(volatile s32 *)(gui + 0x3F9F4) = b;
        *(volatile s32 *)(gui + 0x3F9F0) = a;
    }
}

/** True when either of the GUI pump callback slots at
 *  g_guiInstance+0x3F9F0/+0x3F9F4 is registered. */
s32 func_0029C570(void) {
    char *gui = g_guiInstance;
    if (gui != 0) {
        if (*(s32 *)(gui + 0x3F9F0) != 0 || *(s32 *)(gui + 0x3F9F4) != 0) {
            return 1;
        }
    }
    return 0;
}

/* func_0029C5B0(a,b,idx): if the GUI is up and idx<2, store the (a,b) pair
 * into the slot table at g_guiInstance+0x3FA18+idx*8 and return 1; else 0.
 * Best attempt 87.6% under -G8 -fno-gcse (base-0x38000 split + volatile store
 * order reproduce exactly): the residue is pure register COLORING — the
 * original (later SN) cc1 copies BOTH args to $t0/$t1 and duplicates the slot
 * base ($a0 + a gratuitous $v1 copy) while the pinned cc1 copies only `a` and
 * keeps one base. Same coloring wall as func_002911F0. Left as asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C5B0);

/* func_0029C600(idx): clear the slot pair at g_guiInstance+0x3FA18+idx*8 (the
 * counterpart of func_0029C5B0). UNMATCHABLE in this unit: it reads
 * g_guiInstance via %gp_rel($gp) while every other function here reads it via
 * the adjacent absolute lui/lw pair — the same symbol, both ways, inside one
 * original TU (the proven reload-artifact wall). The unit-wide extern model
 * can only express one side per symbol (g_guiInstance is modeled absolute,
 * favouring the ~60 wrappers), so this stays asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C600);

/* func_0029C638: 16 bytes of dead inter-function fill (`li $v0,0` + epilogue
 * orphan, no jr) — not compiler-reachable C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C638);

/** Forward to the widget at g_guiInstance+0x36F28 (method func_0034F200). */
s32 func_0029C648(void) {
    if (g_guiInstance != 0) {
        return func_0034F200(g_guiInstance + 0x36F28);
    }
}

/* func_0029C678 (and C700/C818/C8F0/CA98/CC48/CD18): 2+-callee-save functions
 * walled by the 8-byte-packed callee-save layout of the later SN cc1 (ours
 * reserves 16 bytes per save) - the wall characterized in the 1907F0 round. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C678);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C700);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C818);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029C8F0);

/** Invalidate the D_1A9A90 GUI state (set to -1). */
void func_0029CA88(void) {
    D_1A9A90 = -1;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CA98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CC48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CCB8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029CD18);

/* func_0029CF08: empty jr-ra stub - installed by GuiManagerCreate as a default no-op GUI callback. */
void func_0029CF08(void) {
}

/*
 * ---- g_guiInstance forwarding-wrapper family ----------------------------
 * Each wrapper forwards to one method of a GUI widget embedded at a fixed
 * offset inside the GuiManager instance, guarded on the GUI being up. Two
 * recurring shapes:
 *   - update/draw style: `if (gui) return method(gui + WIDGET);` — falls off
 *     the end (no return value materialised) when the GUI is down, exactly
 *     like the original;
 *   - query/handler style: `if (!gui) return 0; return method(gui + WIDGET,
 *     ...);` — returns 0 when the GUI is down.
 * The widget offsets pair the wrappers up per widget (one no-arg method +
 * one arg-taking method per offset).
 */

/** Forward to the widget at g_guiInstance+0x36F28 (method func_0034F1C0). */
s32 func_0029CF10(void) {
    if (g_guiInstance != 0) {
        return func_0034F1C0(g_guiInstance + 0x36F28);
    }
}

/** Forward to the widget at g_guiInstance+0x36F28 (method func_0034F240). */
s32 func_0029CF40(void) {
    if (g_guiInstance != 0) {
        return func_0034F240(g_guiInstance + 0x36F28);
    }
}

/** Forward to the popup-menu widget at g_guiInstance+0x39160 (method func_00349C00). */
s32 func_0029CF70(void) {
    if (g_guiInstance != 0) {
        return func_00349C00(g_guiInstance + 0x39160);
    }
}

/** Run the popup-menu widget at g_guiInstance+0x39160 (UpdatePopupMenu, returns
 *  the selected item index); 0 when the GUI is down. */
s32 func_0029CFA0(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return UpdatePopupMenu(g_guiInstance + 0x39160, arg);
}

/** Forward to the widget at g_guiInstance+0x39620 (method func_00348628). */
s32 func_0029CFE0(void) {
    if (g_guiInstance != 0) {
        return func_00348628(g_guiInstance + 0x39620);
    }
}

/** Forward to the widget at g_guiInstance+0x39AF0 (method func_00347550). */
s32 func_0029D010(void) {
    if (g_guiInstance != 0) {
        return func_00347550(g_guiInstance + 0x39AF0);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x39620 (method func_00347B88);
 *  0 when the GUI is down. */
s32 func_0029D040(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00347B88(g_guiInstance + 0x39620, arg);
}

/** Forward two args to the widget at g_guiInstance+0x39AF0 (method
 *  func_00346EF0); 0 when the GUI is down. */
s32 func_0029D080(s32 arg0, s32 arg1) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00346EF0(g_guiInstance + 0x39AF0, arg0, arg1);
}

/** Forward `arg` to the widget at g_guiInstance+0x3B418 (method func_0033A368);
 *  0 when the GUI is down. */
s32 func_0029D0C8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033A368(g_guiInstance + 0x3B418, arg);
}

/** Forward to the widget at g_guiInstance+0x3B418 (method func_0033A5D8). */
s32 func_0029D108(void) {
    if (g_guiInstance != 0) {
        return func_0033A5D8(g_guiInstance + 0x3B418);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3AF80 (method func_0033AF70);
 *  0 when the GUI is down. */
s32 func_0029D138(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033AF70(g_guiInstance + 0x3AF80, arg);
}

/** Forward to the widget at g_guiInstance+0x3AF80 (method func_0033B428). */
s32 func_0029D178(void) {
    if (g_guiInstance != 0) {
        return func_0033B428(g_guiInstance + 0x3AF80);
    }
}

/** Forward to the widget at g_guiInstance+0x3B6F8 (method func_0033C958). */
s32 func_0029D1A8(void) {
    if (g_guiInstance != 0) {
        return func_0033C958(g_guiInstance + 0x3B6F8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3B6F8 (method func_0033C588);
 *  0 when the GUI is down. */
s32 func_0029D1D8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033C588(g_guiInstance + 0x3B6F8, arg);
}

/** Forward to the widget at g_guiInstance+0x3BA58 (method func_0033FEF8). */
s32 func_0029D218(void) {
    if (g_guiInstance != 0) {
        return func_0033FEF8(g_guiInstance + 0x3BA58);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3BA58 (method func_0033FAB0);
 *  0 when the GUI is down. */
s32 func_0029D248(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033FAB0(g_guiInstance + 0x3BA58, arg);
}

/** Forward to the widget at g_guiInstance+0x3A000 (method func_003462C0). */
s32 func_0029D288(void) {
    if (g_guiInstance != 0) {
        return func_003462C0(g_guiInstance + 0x3A000);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3A000 (method func_003453D0);
 *  0 when the GUI is down. */
s32 func_0029D2B8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003453D0(g_guiInstance + 0x3A000, arg);
}

/** Forward to the widget at g_guiInstance+0x3C160 (method func_00342B30). */
s32 func_0029D2F8(void) {
    if (g_guiInstance != 0) {
        return func_00342B30(g_guiInstance + 0x3C160);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C160 (method func_003427D0);
 *  0 when the GUI is down. */
s32 func_0029D328(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003427D0(g_guiInstance + 0x3C160, arg);
}

/** Forward to the widget at g_guiInstance+0x3C160 (method func_00342B30) —
 *  byte-identical twin of func_0029D2F8 (two call sites got their own stubs). */
s32 func_0029D368(void) {
    if (g_guiInstance != 0) {
        return func_00342B30(g_guiInstance + 0x3C160);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C160 (method func_003427D0)
 *  — byte-identical twin of func_0029D328; 0 when the GUI is down. */
s32 func_0029D398(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003427D0(g_guiInstance + 0x3C160, arg);
}

/** Forward to the widget at g_guiInstance+0x3C480 (method func_00344808). */
s32 func_0029D3D8(void) {
    if (g_guiInstance != 0) {
        return func_00344808(g_guiInstance + 0x3C480);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C480 (method func_003446B8);
 *  0 when the GUI is down. */
s32 func_0029D408(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003446B8(g_guiInstance + 0x3C480, arg);
}

/** Forward to the widget at g_guiInstance+0x3C7E8 (method func_0033F670). */
s32 func_0029D448(void) {
    if (g_guiInstance != 0) {
        return func_0033F670(g_guiInstance + 0x3C7E8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3C7E8 (method func_0033F610);
 *  0 when the GUI is down. */
s32 func_0029D478(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033F610(g_guiInstance + 0x3C7E8, arg);
}

/** Forward to the widget at g_guiInstance+0x3D1D8 (method func_0033E9B8). */
s32 func_0029D4B8(void) {
    if (g_guiInstance != 0) {
        return func_0033E9B8(g_guiInstance + 0x3D1D8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3D1D8 (method func_0033E8B0);
 *  0 when the GUI is down. */
s32 func_0029D4E8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033E8B0(g_guiInstance + 0x3D1D8, arg);
}

/** Forward `arg` to the widget at g_guiInstance+0x3D4B8 (method func_0033D5D8);
 *  0 when the GUI is down. */
s32 func_0029D528(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033D5D8(g_guiInstance + 0x3D4B8, arg);
}

/** Forward to the widget at g_guiInstance+0x3D4B8 (method func_0033D780). */
s32 func_0029D568(void) {
    if (g_guiInstance != 0) {
        return func_0033D780(g_guiInstance + 0x3D4B8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3D798 (method func_0033DB60);
 *  0 when the GUI is down. */
s32 func_0029D598(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033DB60(g_guiInstance + 0x3D798, arg);
}

/** Forward to the widget at g_guiInstance+0x3D798 (method func_0033DC68). */
s32 func_0029D5D8(void) {
    if (g_guiInstance != 0) {
        return func_0033DC68(g_guiInstance + 0x3D798);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3DDE8 (method func_0033E5E8);
 *  0 when the GUI is down. */
s32 func_0029D608(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033E5E8(g_guiInstance + 0x3DDE8, arg);
}

/** Forward to the widget at g_guiInstance+0x3DDE8 (method func_0033E680). */
s32 func_0029D648(void) {
    if (g_guiInstance != 0) {
        return func_0033E680(g_guiInstance + 0x3DDE8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E0C8 (method func_0033CEE0);
 *  0 when the GUI is down. */
s32 func_0029D678(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033CEE0(g_guiInstance + 0x3E0C8, arg);
}

/** Forward to the widget at g_guiInstance+0x3E0C8 (method func_0033D070). */
s32 func_0029D6B8(void) {
    if (g_guiInstance != 0) {
        return func_0033D070(g_guiInstance + 0x3E0C8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E3A8 (method func_0033D320);
 *  0 when the GUI is down. */
s32 func_0029D6E8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033D320(g_guiInstance + 0x3E3A8, arg);
}

/** Forward to the widget at g_guiInstance+0x3E3A8 (method func_0033D3C0). */
s32 func_0029D728(void) {
    if (g_guiInstance != 0) {
        return func_0033D3C0(g_guiInstance + 0x3E3A8);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3F3F0 (method func_0033F3F0);
 *  0 when the GUI is down. */
s32 func_0029D758(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033F3F0(g_guiInstance + 0x3F3F0, arg);
}

/** Forward to the widget at g_guiInstance+0x3F3F0 (method func_0033F478). */
s32 func_0029D798(void) {
    if (g_guiInstance != 0) {
        return func_0033F478(g_guiInstance + 0x3F3F0);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3DA78 (method func_0033E070);
 *  0 when the GUI is down. */
s32 func_0029D7C8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033E070(g_guiInstance + 0x3DA78, arg);
}

/** Forward to the widget at g_guiInstance+0x3DA78 (method func_0033E308). */
s32 func_0029D808(void) {
    if (g_guiInstance != 0) {
        return func_0033E308(g_guiInstance + 0x3DA78);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3EB50 (method func_0033EC80);
 *  0 when the GUI is down. */
s32 func_0029D838(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_0033EC80(g_guiInstance + 0x3EB50, arg);
}

/** Forward to the widget at g_guiInstance+0x3EB50 (method func_0033ED18). */
s32 func_0029D878(void) {
    if (g_guiInstance != 0) {
        return func_0033ED18(g_guiInstance + 0x3EB50);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E760 (method func_003433D0);
 *  0 when the GUI is down. */
s32 func_0029D8A8(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_003433D0(g_guiInstance + 0x3E760, arg);
}

/** Forward to the widget at g_guiInstance+0x3E760 (method func_00343668). */
s32 func_0029D8E8(void) {
    if (g_guiInstance != 0) {
        return func_00343668(g_guiInstance + 0x3E760);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3E9C8 (method func_00342DF8);
 *  0 when the GUI is down. */
s32 func_0029D918(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00342DF8(g_guiInstance + 0x3E9C8, arg);
}

/** Forward to the widget at g_guiInstance+0x3E9C8 (method func_00342E48). */
s32 func_0029D958(void) {
    if (g_guiInstance != 0) {
        return func_00342E48(g_guiInstance + 0x3E9C8);
    }
}

/* func_0029D988: 2 orphan unreachable words (`addu $2,$3,$2; nop`) — a dead
 * code fragment between the wrapper bodies, not compiler-reachable C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029D988);

/** Tick the on-screen bolt counter (UpdateBoltCounterHud) only when the HUD
 *  render context (g_guiInstance) is present. */
s32 TickBoltCounterHud(void) {
    if (g_guiInstance != 0) {
        return UpdateBoltCounterHud();
    }
}

/** Run the func_0028B0B0 HUD updater only when the GUI is present. */
s32 func_0029D9B8(void) {
    if (g_guiInstance != 0) {
        return func_0028B0B0();
    }
}

/** Flush a HUD display value into the render object at g_guiInstance+0x376C8
 *  (used to push g_nBoltCounterDisplayed to the renderer). */
s32 FlushHudDisplayValue(s32 value) {
    if (g_guiInstance != 0) {
        return func_0034DBD8(g_guiInstance + 0x376C8, value);
    }
}

/** Forward `arg` to the widget at g_guiInstance+0x3A4C0 (method func_00340AA8);
 *  0 when the GUI is down. */
s32 func_0029DA18(s32 arg) {
    if (g_guiInstance == 0) {
        return 0;
    }
    return func_00340AA8(g_guiInstance + 0x3A4C0, arg);
}

/** Forward to the widget at g_guiInstance+0x3A4C0 (method func_00341160). */
s32 func_0029DA58(void) {
    if (g_guiInstance != 0) {
        return func_00341160(g_guiInstance + 0x3A4C0);
    }
}

/** Forward three args to the widget at g_guiInstance+0x36F28 (method
 *  func_003395F0). */
s32 func_0029DA88(s32 arg0, s32 arg1, s32 arg2) {
    if (g_guiInstance != 0) {
        return func_003395F0(g_guiInstance + 0x36F28, arg0, arg1, arg2);
    }
}

/** Forward two args to the widget at g_guiInstance+0x36F28 (method
 *  func_00339678). */
s32 func_0029DAD0(s32 arg0, s32 arg1) {
    if (g_guiInstance != 0) {
        return func_00339678(g_guiInstance + 0x36F28, arg0, arg1);
    }
}

/** Forward two args to the widget at g_guiInstance+0x36F28 (method
 *  func_00338F18). */
s32 func_0029DB10(s32 arg0, s32 arg1) {
    if (g_guiInstance != 0) {
        return func_00338F18(g_guiInstance + 0x36F28, arg0, arg1);
    }
}

/* func_0029DB50: empty jr-ra stub - installed by GuiManagerCreate as a default no-op GUI callback. */
void func_0029DB50(void) {
}

/** Forward the PAL-mode flag (as 0/1) to func_00336BC8. */
s32 func_0029DB58(void) {
    return func_00336BC8(g_bPalMode != 0);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", GuiManagerCreate);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DC70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DCB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD80);

/* func_0029DD90: toSPR DMA-kick helper (writes SADR/QWC/MADR at 0x1000D400,
 * CHCR 0x100) - called 4x from the sky piece stagers. Split off func_0029DD80
 * (4 orphan unreachable words). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DD90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DDB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029DDE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", EnableDmacChannels);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", FlushPendingTexUploads);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E1D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", DecompressWad);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E5D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029E5F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", UpdateLevelObjectiveStates);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", EvaluateProgressCondition);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", GatherActiveObjectives);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EA90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EAC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EB68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EBF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029EC70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029ECE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", func_0029FDF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", SpawnMoby);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/198FA0", InitMobyFromClass);

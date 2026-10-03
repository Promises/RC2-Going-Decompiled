#include "common.h"

/*
 * text/235FE8 - GUI widget-method band (carve-pipeline pick #3b, 2026-06-13;
 * vaddr 0x336068..0x348BCF, 344 functions): the backward continuation of the
 * already-matched text/248B50 GuiWidget accessor band (0x348BD0+). Same shape:
 * vtable installers, single-field getters/setters over a widget object passed
 * in $a0, plus the GUI element draw/layout primitives (GuiElementSetPos /
 * GetColor / SetVisible / SetScale), the sprite/text/list element helpers, and
 * the dialog-box / screen constructors.
 *
 * Built at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA override in
 * tools/ee/objdiff_build.sh / diff.sh / build.sh) - the later-SN-cc1 GUI TU
 * model shared by text/248B50 / text/198FA0. Under -G8 the small sdata globals
 * (g_waterPool, D_1ADAD8/D_1ADAF0) get their one-insn %gp_rel form; the
 * absolute-addressed globals (g_GuiElementVtable, D_1AD988) are declared with
 * `.extern sym, 16` so cc1 keeps the two-insn %hi/%lo macro.
 *
 * WALLS (left INCLUDE_ASM, per-function notes where relevant):
 *  - 8-byte-packed callee saves: any method saving two or more GPRs hits the
 *    later-cc1 8-byte-slot vs pinned-cc1 16-byte-slot wall.
 *  - switch functions: blocked by the splat jtbl reloc-identity gap.
 *  - the handwritten 8-byte stub-table fragments (func_00336068/00336218/...
 *    = bare addiu-sp/nop runs that are their own functions) stay INCLUDE_ASM.
 *
 * ⚠ READ BEFORE TRUSTING ANY PER-FUNCTION `engine96 probe (...): NN%` COMMENT
 * BELOW. Those 175 numbers are task #466/#497-era SCREEN values and they do NOT
 * describe what this tree compiles to. Re-measured on the committed file by
 * task #583 (all 175 #else arms guarded onto the engine96 arm at the gate's own
 * flags, cc1 2.96-ee-001003-1 -O2 -G8 -fno-schedule-insns -fno-strict-aliasing):
 * only 51 of the 175 still reproduce to 0.01. 57 differ by under a point and 67
 * differ by a point or more, the worst by 42 points (func_00336678 83.33 -> 41.25).
 * The multi-point class is the defect FACT #7375 named: the comments quote the
 * state with the sibcall barrier applied, and the barrier was reverted with the
 * arms, so the committed bodies compile to a different residual — and for the
 * 69 sibcall-shaped bodies the CLASS is wrong too (they read SIBCALL on the
 * tree, not the SCHED/UNKNOWN the comment states).
 *
 * So: triage from a fresh measurement, never from these numbers. They are left
 * in place rather than rewritten because the replacement values available today
 * are blanket-screen values (all arms compiled on one arm at once), which t276
 * forbids as a landing configuration and which must not be written into the .c
 * as if they were landing measurements — that conflation is how the stale set
 * arose. Members and the full comparison: FACT filed by task #583; the earlier
 * census is FACT #7375, narrowed by #7484.
 */

/* Keep the absolute-addressed globals on the two-insn %hi/%lo macro under -G8. */
__asm__(".extern g_GuiElementVtable, 16");
__asm__(".extern D_1AD988, 16");
__asm__(".extern D_1AD9A8, 16");
__asm__(".extern D_1AD9F8, 16");
__asm__(".extern g_GuiListRowVtable, 16");

/* Addressing-mode pins for the globals the #else / MATCH_ bodies reference
 * (task #466). Under -G8 the assembler chooses %gp_rel for an extern whose
 * declared size is <= 8 and the two-insn %hi/%lo macro otherwise, and the
 * FIRST `.extern` size wins over the one cc1 appends. The ROM is consistent
 * per symbol across this unit (tools/ee/.t466/14_symbol_addressing.txt: 285
 * absolute-only, 181 gp-only, 4 mixed); these lines make each symbol's
 * addressing follow the ROM regardless of how its C declaration sizes it.
 * Re-derive: objdump -dr expected.o vs obj/<unit>.engine96.o, per symbol
 * (tools/ee/.t466/addrmode.py). Byte-neutral on the 2.9 arm: the sdk29
 * object's .text bytes, relocations and undefined-symbol set are unchanged
 * (only symtab metadata moves), so no matched function is touched. */
/* ROM addresses these absolutely (%hi/%lo) although their C declarations are
 * sized <= 8. The opposite case (ROM gp-relative, declaration an incomplete
 * array) is fixed at the declaration instead: cc1 itself expands %hi/%lo for
 * an unsized extern, so those arrays are declared with their 8-byte size. */
__asm__(".extern D_1A7B9C, 16");
__asm__(".extern D_1A7B9D, 16");
__asm__(".extern D_1A7B9E, 16");
__asm__(".extern D_1A7BAC, 16");
__asm__(".extern D_1A7BB0, 16");
__asm__(".extern D_1A7BB4, 16");
__asm__(".extern D_1A7BB9, 16");
__asm__(".extern D_1A7BBA, 16");
__asm__(".extern D_1A7BBB, 16");
__asm__(".extern D_1A8C88, 16");
__asm__(".extern D_1A8C8C, 16");
__asm__(".extern D_1AA856, 16");
__asm__(".extern D_1AD8C8, 16");
__asm__(".extern D_1AD968, 16");
__asm__(".extern D_1ADA18, 16");
__asm__(".extern D_1ADA38, 16");
__asm__(".extern D_1ADC38, 16");
__asm__(".extern D_1ADC40, 16");
__asm__(".extern D_1ADC48, 16");
__asm__(".extern D_1ADC6C, 16");
__asm__(".extern D_1ADCB8, 16");
__asm__(".extern D_1ADCC0, 16");
__asm__(".extern D_1ADCC8, 16");
__asm__(".extern D_1ADCD0, 16");
__asm__(".extern D_1ADCDC, 16");
__asm__(".extern D_1ADCE4, 16");
__asm__(".extern D_1ADCEC, 16");
__asm__(".extern D_1ADCFC, 16");
__asm__(".extern D_1ADD04, 16");
__asm__(".extern D_1ADDDC, 16");
__asm__(".extern D_1ADDE4, 16");
__asm__(".extern D_1ADDEC, 16");
__asm__(".extern D_1ADDF4, 16");
__asm__(".extern D_1ADE34, 16");
__asm__(".extern D_1ADE3C, 16");
__asm__(".extern D_1ADF3C, 16");
__asm__(".extern D_1ADF44, 16");
__asm__(".extern D_1ADF4C, 16");
__asm__(".extern D_1ADF54, 16");
__asm__(".extern D_1AE020, 16");
__asm__(".extern D_1AE028, 16");
__asm__(".extern D_1AE030, 16");
__asm__(".extern D_1AE038, 16");
__asm__(".extern D_1AE040, 16");
__asm__(".extern D_1AE048, 16");
__asm__(".extern D_1AE0B4, 16");
__asm__(".extern D_1AE0C4, 16");
__asm__(".extern D_1AE0D4, 16");
__asm__(".extern D_1AE0DC, 16");
__asm__(".extern D_1AE0E4, 16");
__asm__(".extern D_1AE0EC, 16");
__asm__(".extern D_1AE0F4, 16");
__asm__(".extern D_1AE0FC, 16");
__asm__(".extern D_1AE154, 16");
__asm__(".extern D_1AE164, 16");
__asm__(".extern D_1AE174, 16");
__asm__(".extern D_1AE17C, 16");
__asm__(".extern D_1AE19C, 16");
__asm__(".extern D_1AE1D4, 16");
__asm__(".extern D_1AE1D8, 16");
__asm__(".extern D_1AE1DC, 16");
__asm__(".extern D_1AE1E4, 16");
__asm__(".extern D_1AE1EC, 16");
__asm__(".extern D_1AE1F4, 16");
__asm__(".extern D_1AE204, 16");
__asm__(".extern D_1AE20C, 16");
__asm__(".extern D_1AE214, 16");
__asm__(".extern D_1AE21C, 16");
__asm__(".extern D_1AE224, 16");
__asm__(".extern D_1AE25C, 16");
__asm__(".extern D_1AE264, 16");
__asm__(".extern D_1AE26C, 16");
__asm__(".extern D_1AE274, 16");
__asm__(".extern D_1AE27C, 16");
__asm__(".extern D_1AE28C, 16");
__asm__(".extern D_1AE294, 16");
__asm__(".extern D_1AE29C, 16");
__asm__(".extern D_1AE2A4, 16");
__asm__(".extern D_1AE2AC, 16");
__asm__(".extern D_1AE2B4, 16");
__asm__(".extern D_1AE3DC, 16");
__asm__(".extern D_1AE3E4, 16");
__asm__(".extern D_1AE3EC, 16");
__asm__(".extern D_1AE3F4, 16");
__asm__(".extern D_1AE3FC, 16");
__asm__(".extern D_1AE404, 16");
__asm__(".extern D_1AE40C, 16");
__asm__(".extern D_1AE414, 16");
__asm__(".extern D_1AE41C, 16");
__asm__(".extern D_1AE424, 16");
__asm__(".extern D_1AE4D0, 16");
__asm__(".extern D_1AE4D8, 16");
__asm__(".extern D_1AE4E0, 16");
__asm__(".extern D_1AE4E8, 16");
__asm__(".extern D_1AE518, 16");
__asm__(".extern D_1AE524, 16");
__asm__(".extern D_1AE52C, 16");
__asm__(".extern D_1AE53C, 16");
__asm__(".extern D_1AE544, 16");
__asm__(".extern D_1AE548, 16");
__asm__(".extern D_1AE550, 16");
__asm__(".extern D_1AE558, 16");
__asm__(".extern D_1AE560, 16");
__asm__(".extern D_1AE700, 16");
__asm__(".extern D_1AE704, 16");
__asm__(".extern D_1AE708, 16");
__asm__(".extern D_1AE70C, 16");
__asm__(".extern D_1AE710, 16");
__asm__(".extern D_1AE714, 16");
__asm__(".extern D_263B10, 16");
__asm__(".extern g_activeCamera, 16");
__asm__(".extern g_audioStereoMode, 16");
__asm__(".extern g_bProgressiveScan, 16");
__asm__(".extern g_currentLanguage, 16");
__asm__(".extern g_equippedArmor, 16");
__asm__(".extern g_gameTime, 16");
__asm__(".extern g_mapCurrentLevel, 16");
__asm__(".extern g_miscExtras, 16");
__asm__(".extern g_musicVolume, 16");
__asm__(".extern g_padButtonsHeld, 16");
__asm__(".extern g_padButtonsPressed, 16");
__asm__(".extern g_playerProgress, 16");
__asm__(".extern g_sfxVolume, 16");

extern void *g_GuiElementVtable; /* base GuiElement vtable installed at +0x30 */
extern void *D_1AD988;           /* vtable installed by func_003368D0 / func_003368E8 */
extern void *D_1AD9A8;           /* vtable installed by func_00336678 */
extern void *D_1AD9F8;           /* vtable installed by func_00337830 */
extern void *g_GuiListRowVtable; /* GuiListRow element vtable installed by func_00337278 */

extern void GuiSpriteElementDraw(void *p);
extern void GuiTextElementDraw(void *e);
extern void GuiDialogBoxInitElements(void *p);

/* Small-data (gp_rel) vtable globals installed by the func_00336B88/BA8 ctors. */
extern void *D_1AD908;
extern void *D_1AD8E8;

/* A GUI element: first words are pointers into a shared float-vector pool
 * (pos +0x0, scale +0x4), a color block +0xC, a visibility-scalar pointer
 * +0x10; the vtable lives at +0x30. Only touched fields are typed. */
typedef struct GuiElement {
    /* 0x00 */ f32 *pos;     /* -> [x,y,z,w] */
    /* 0x04 */ f32 *scale;   /* -> [x,y,z,w] */
    /* 0x08 */ f32 *unk08;   /* -> 16-byte vector block (allocated 2nd by base init) */
    /* 0x0C */ s32 *color;   /* -> color block */
    /* 0x10 */ f32 *visible; /* -> visibility scalar (>0 shown) */
} GuiElement;

/* GUI fixed-size node pool (allocator body at GuiPoolAlloc, initialiser at
 * GuiPoolInit). Defined up front so the element constructors (GuiElementBaseInit,
 * GuiElementInit) and GuiPoolInit all see the field layout regardless of source
 * order. Pure type info: byte-neutral.
 *   +0x00 base     pointer to the backing storage
 *   +0x04 capacity byte limit
 *   +0x08 elemSize  per-node byte size / element kind
 *   +0x0C cursor    bump offset into the backing storage
 *   +0x10 count     live allocation count
 *   +0x14 freeList  head of the singly-linked free list (next ptr at node+0) */
typedef struct GuiPool {
    /* 0x00 */ char *base;
    /* 0x04 */ u32 capacity;
    /* 0x08 */ u32 elemSize;
    /* 0x0C */ u32 cursor;
    /* 0x10 */ s32 count;
    /* 0x14 */ void *freeList;
} GuiPool;
extern void *GuiPoolAlloc(GuiPool *pool);
extern void *GuiPlacementNew(s32 size, void *buf);
extern void GuiElementSetVisible(GuiElement *e, s32 show);


/* callees of the tail-call wrappers below (return values are discarded). */
extern void func_002704E0(void);
extern void func_00336F00(void *p);
extern void func_00336678(void *p, s32 flag);
extern void func_003368E8(void *p, s32 flag);
extern void func_00337C48(void);
extern s32 func_00343AD0(void *p);
extern void func_00343E80(void *p);
extern void func_00343F68(void *p);
extern void func_0033BE60(void *p, s32 v);
extern void func_0033BF90(void *p);
extern void GuiMenuListSetRows(void *p, void *records);
extern s32 func_00348E60(void *p);  /* 248B50:308 - returns a state value */
extern void func_00348E10(void *w, s32 a, s32 b); /* 248B50:269 */
extern void GuiMenuListDraw(void *p);

/* Shared GUI sub-element init/widget callees referenced by the functional-
 * equivalent #else bodies below (declared up front so each body sees a
 * consistent prototype regardless of source order). Unconditional since #466:
 * the engine96 arm (objdiff_build.sh, MATCH_<fn> guards) compiles those bodies
 * too, and the declarations are byte-neutral on the 2.9 arm (sdk29 object
 * md5 unchanged with the block visible). */
extern void GuiElementInitTypeB(void *p);
extern void GuiElementInitTypeC(void *p);
extern GuiElement *GuiListRowElementInit(void *p);
extern void func_00348BD0(void *p);
extern s32 GuiMenuListHandleInput(void *w, s32 inputMask); /* 248B50: selection-advance by input mask; returns 1/0 (248B50.c) */
extern void GuiMenuListSetOrigin(void *w, f32 x, f32 y);
extern void func_00348E58(void *w, s32 res);
extern void func_00115AC0(void *dst, const void *src, s32 len); /* SDK memcpy */
extern void func_00338D48(void *p, s32 a1, f32 x, f32 y, s32 a2);
extern void func_0033FDD8(void *p);
extern void func_00342958(void *p);
extern void func_00342978(void *p);
extern void func_00342998(void *p);
extern f32 D_1AE1E8;                    /* GUI x-position offset constant */
extern f32 D_1AE1EC;                    /* GUI y-position offset constant */
extern int func_00115DA8(char *dst, const char *fmt, ...); /* SDK sprintf (returns char count) */
extern s32 GetLocalizedString(s32 id);  /* textId -> char* (declared early for func_003395F0) */
extern s32 g_padButtonsPressed;         /* 0x138344 - buttons pressed this frame */
extern f32 D_1AE0C0, D_1AE0C4;          /* GUI anchor x/y position offsets */
extern f32 D_1AE0C8, D_1AE0CC;          /* GUI size params (float, truncated to int) */
extern f32 D_1AE0D0, D_1AE0D4, D_1AE0D8, D_1AE0DC, D_1AE0E0; /* sub-element x/y offset pairs */
extern f32 D_1AE0E4, D_1AE0E8, D_1AE0EC, D_1AE0F0, D_1AE0F4;
extern void func_00342670(void *w);     /* positions w+0x130 from the selected row */
extern s32 func_00348E68(void *w);      /* 248B50:313 - list row count */
extern void GuiMenuListDraw(void *w);   /* 0x348E70 - per-frame menu-list draw */
extern f32 D_1AE108, D_1AE10C;          /* list row Y-step / X-offset */
extern f32 D_1AE160, D_1AE164;          /* func_00343338 anchor x/y offsets */
extern f32 D_1AE168, D_1AE16C;          /* func_00343338 size params (float->int) */
extern void func_00339740(void *w);     /* func_00339A88 mode-0 sub-updater */
extern u8 D_1A7BBA;                      /* toggled widescreen/mode flag */
/* func_003453D0 (weapon-select builder) deps */
extern void SetWeaponUpgradeSlot(s32 a, s32 b); /* 0x288A20 - retarget upgrade slot */
extern void func_00344E08(void *w, void *a);    /* mode-0 handler */
extern void func_00344F18(void *w, void *a);    /* mode-2 handler */
extern void func_00345080(void *w, void *a);    /* mode-3 handler */
extern void func_003451B8(void *w, void *a);    /* mode-1 handler */
extern void func_003457A0(void *w);             /* weapon-panel tail draw */
extern s16 g_equippedArmor;             /* 0x1A7A20 */
extern u8 g_inventoryOwned[];           /* per-item owned flags */
extern u8 g_weaponTable[];              /* 0x239B20 - variant table, stride 0xE0 */
extern u8 g_itemEquippedSlot[];         /* 0x139568 - itemId -> variant slot */
extern s32 D_1AE248[];                  /* 4-entry stringId table */
extern f32 D_1AE280[];                  /* {x,y} offset pair for the +0x1C8 element */
extern f32 D_1AE258, D_1AE25C, D_1AE260, D_1AE264, D_1AE268, D_1AE26C, D_1AE270;
extern f32 D_1AE274, D_1AE278, D_1AE27C, D_1AE288, D_1AE28C, D_1AE290, D_1AE294;
extern s32 D_1ADC68;                    /* GUI x-offset (int, used as float) */
extern s32 D_1ADC6C;                    /* GUI y-offset (int, used as float) */
extern s32 D_1ADC70;                    /* GUI y per-counter step (int multiplier) */
/* defined later in-unit; forward-declared for func_0033A8F0's earlier #else use */
extern void func_0033C060(void *p, f32 a, f32 b);
extern void func_0033BE70(void *p, s32 flags);
/* color-pulse: blends color1/color2 by a period counter, returns packed RGBA (d1) */
extern u32 func_002AA3F0(u32 color1, u32 color2, s32 period, s32 counterSel, s32 reset);
/* func_00338CD8 (void*,s32,f32,f32,s32) + func_0033BE70 (void*,s32) are defined
 * later in this unit, before their #else callers below — no extern needed. */

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336068);

/* func_00336168: copy a 16-byte (quadword) vector from *p into the active
 * camera's render block at +0x80. The camera is g_activeCamera unless its
 * mode word (+0x86) differs from 5, in which case func_00270290(5) supplies a
 * substitute camera; the render block lives at *(camera+0x70). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336168);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00336168, unit objdiff): 9.23%,
   27/34 insns differ. Residual: UNKNOWN-addiu + lq/sq (first differing insn: '' vs 'addiu v0, zero, 0x5').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; sq/lq 128-bit copy +
   branch wall. */
/* A 16-byte (128-bit) quadword record. func_00336168/func_003361C0 copy it with
 * a single lq/sq (`lq v0,0x0(src); sq v0,0x0(dst)`) into the active camera's
 * render block at +0x80 / +0x90 respectively. Size pin is byte-neutral. */
typedef struct GuiQword { s32 w[4]; } GuiQword;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(GuiQword) == 0x10, "GuiQword is a 128-bit quadword");
#endif
extern void *g_activeCamera;
extern void *func_00270290(s32 mode);
void func_00336168(GuiQword *src) {
    void *cam = g_activeCamera;
    void *base = (*(s16 *)((char *)cam + 0x86) == 5) ? cam : func_00270290(5);
    GuiQword *dst = (GuiQword *)((char *)*(void **)((char *)base + 0x70) + 0x80);
    *dst = *src;
}
#endif

/* func_003361C0: as func_00336168 but writes the quadword to the camera render
 * block at +0x90 instead of +0x80. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003361C0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003361C0, unit objdiff): 9.23%,
   27/34 insns differ. Residual: UNKNOWN-addiu + lq/sq (first differing insn: '' vs 'addiu v0, zero, 0x5').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; sq/lq 128-bit copy +
   branch wall. */
void func_003361C0(GuiQword *src) {
    void *cam = g_activeCamera;
    void *base = (*(s16 *)((char *)cam + 0x86) == 5) ? cam : func_00270290(5);
    GuiQword *dst = (GuiQword *)((char *)*(void **)((char *)base + 0x70) + 0x90);
    *dst = *src;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336218);

/* func_00336230(a, b, mode, arg3, arg4): allocate + switch to a camera (mode 5)
 * and seed its transform block. Snapshots four qwords (*a, *b, g_cameraPos[0],
 * g_cameraPos[1]) to locals, then SwitchActiveCamera; sub = cam[+0x70]. Common:
 * cam[0x7D]=1; cam+0x30 = g_cameraPos = *a; cam+0x40 = *b; sub+0x80 = *a;
 * sub+0x90 = *b; sub+0x120 = arg4; cam[0x88] = mode. mode 3 additionally seeds a
 * 1.0 blend at sub+0x124 (sub+0x128=0), overrides cam+0x30/+0x40 with the saved
 * g_cameraPos qwords, copies g_heroPos to sub+0x110 and runs func_00272560(sub+0xFC,
 * &camPos). mode 2 and 3 both stamp arg3 at sub+0xE0/+0xE4. Faithful TARGET_NATIVE
 * #else (engine 2.96 qword-copy regalloc = no byte-match). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336230);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00336230, unit objdiff): 0.00%,
   169/176 insns differ. Residual: UNKNOWN-addiu + lq/sq (first differing insn: 'addiu sp, sp, -0xa0' vs 'addiu sp, sp, -0x90').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8   g_cameraPos[];
extern u8   g_heroPos[];
extern void SwitchActiveCamera(void *cam);
extern void func_00272560(void *dst, void *src);

typedef struct { u32 w0, w1, w2, w3; } Qw128_235FE8;   /* one 16-byte lq/sq qword */

void func_00336230(void *a, void *b, s32 mode, s32 arg3, s32 arg4) {
    u8 *gcp = g_cameraPos;
    Qw128_235FE8 srcA = *(Qw128_235FE8 *)a;
    Qw128_235FE8 srcB = *(Qw128_235FE8 *)b;
    Qw128_235FE8 camPos0 = *(Qw128_235FE8 *)gcp;
    Qw128_235FE8 camPos1 = *(Qw128_235FE8 *)(gcp + 0x10);
    u8 *cam = (u8 *)func_00270290(5);
    u8 *sub;

    SwitchActiveCamera(cam);
    sub = *(u8 **)(cam + 0x70);

    cam[0x7D] = 1;
    *(Qw128_235FE8 *)(cam + 0x30) = srcA;
    *(Qw128_235FE8 *)gcp          = srcA;
    *(Qw128_235FE8 *)(cam + 0x40) = srcB;
    *(Qw128_235FE8 *)(sub + 0x80) = srcA;
    *(Qw128_235FE8 *)(sub + 0x90) = srcB;
    *(s32 *)(sub + 0x120) = arg4;
    cam[0x88] = (u8)mode;

    if (mode == 3) {
        *(s32 *)(sub + 0x128) = 0;
        *(f32 *)(sub + 0x124) = 1.0f;
        *(Qw128_235FE8 *)(cam + 0x30) = camPos0;
        *(Qw128_235FE8 *)(cam + 0x40) = camPos1;
        *(Qw128_235FE8 *)(sub + 0x110) = *(Qw128_235FE8 *)g_heroPos;
        func_00272560(sub + 0xFC, &camPos0);
    }
    if (mode == 2 || mode == 3) {
        *(s32 *)(sub + 0xE4) = arg3;
        *(s32 *)(sub + 0xE0) = arg3;
    }
}
#endif

/* func_003363A0(mode): set the active camera's transition mode. func_002704E0()
 * refreshes camera bookkeeping; cam = *(g_cameraState+0x190). mode 0 rebuilds the
 * camera look vector at cam+0x30 from the hero orientation (g_heroOrientVec):
 * transform it into a scratch matrix (func_00283DC0 then func_00284028, which
 * fills buf+0x20..), rescale two rows (Vec3RescaleToLenVu0, len -1.0 then 1.2),
 * combine (Vec4AddVu0) and set cam[0x8A]=1. modes 1/2/4 stamp the transition id at
 * cam+0x7E (3/2/5) and, for 2/4, clear g_cameraState[0x293] and set the 0.018 fade
 * rate at +0x2A8/+0x2B4. Faithful TARGET_NATIVE #else (engine 2.96 regalloc = no
 * byte-match); buf+0x40 is read from the callee-written scratch matrix region. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003363A0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003363A0, unit objdiff): 85.19%,
   37/84 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x90' vs 'addiu sp, sp, -0x80').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern s32  g_cameraState[];       /* +0x190 = active camera obj ptr */
extern u8   g_heroOrientVec[];
extern void func_00283DC0(void *dst, void *src);
extern void func_00284028(void *cam, void *scratch);
extern void Vec3RescaleToLenVu0(void *dst, void *src, f32 len);
extern void Vec4AddVu0(void *dst, void *a, void *b);

void func_003363A0(s32 mode) {
    u8 *cs  = (u8 *)g_cameraState;
    u8 *cam = *(u8 **)(cs + 0x190);

    func_002704E0();
    if (mode == 0) {
        u8  buf[0x50];
        u8 *hero = g_heroOrientVec;
        func_00283DC0(buf + 0x20, hero);
        func_00284028(cam, buf + 0x20);
        Vec3RescaleToLenVu0(buf + 0x10, buf + 0x20, -1.0f);
        Vec3RescaleToLenVu0(buf + 0x00, buf + 0x40, 1.2f);
        Vec4AddVu0(cam + 0x30, hero - 0x10, buf + 0x00);
        Vec4AddVu0(cam + 0x30, cam + 0x30, buf + 0x10);
        *(s16 *)(*(u8 **)(cs + 0x190) + 0x8A) = 1;
    } else if (mode == 2) {
        cs[0x293] = 0;
        *(s16 *)(cam + 0x7E) = 2;
        *(f32 *)(cs + 0x2A8) = 0.018f;
        *(f32 *)(cs + 0x2B4) = 0.018f;
    } else if (mode == 1) {
        *(s16 *)(cam + 0x7E) = 3;
    } else if (mode == 4) {
        cs[0x293] = 0;
        *(s16 *)(cam + 0x7E) = 5;
        *(f32 *)(cs + 0x2A8) = 0.018f;
        *(f32 *)(cs + 0x2B4) = 0.018f;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003364E0);

/* func_003365A0: activate a camera by callback-table lookup. Resolves a camera
 * slot (func_00270290(0x1B) -> obj), writes param_3 to its entry+0x20 / sets +0x1D,
 * copies 4 qwords (*param_1, *param_2, g_cameraPos[0], g_cameraPos[0x10]) into its
 * own frame, then calls SwitchActiveCamera(obj).
 *
 * PARK #70 (unresolved arity, NOT an Option-D case): traced SwitchActiveCamera
 * (0x2705A8) — it is 1-arg (Ghidra's param_count=0 was a misread), reading/writing
 * only param_1 (the camera-slot struct; copies g_pActiveCamera history INTO it). It
 * does NOT read caller-stack vec args. So SwitchActiveCamera(obj) here is a normal
 * 1-arg call — but that leaves the 4 qword stores to this frame UNEXPLAINED (dead
 * stores are implausible at -O2). Either (a) they are genuinely dead, or (b)
 * SwitchActiveCamera reads them as hidden stack args Ghidra missed (which would make
 * it a 5-arg fn + turn its many 1-arg callers into the Option-D class). Resolving
 * needs SwitchActiveCamera's own asm traced for incoming-stack reads (its .s is not
 * in nonmatchings — matched/elsewhere). A #else now would be a guess either way
 * (dead-store vs guessed 5-arg cast) -> honest park > shaky #else. Un-park once the
 * 4-qword purpose is proven. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003365A0);

/* func_00336648: forward to func_002704E0 (no args). */
void func_00336648(void) {
    func_002704E0();
    __asm__ __volatile__("");
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336668);

/**
 * Install the D_1AD9A8 vtable at p+0x4, then call func_00337C48() when bit 0 of
 * `flag` is set.
 *
 * p: object whose +0x4 slot receives the vtable pointer. flag: bit 0 selects
 * the follow-up call. No return value.
 *
 * The store sits in the delay slot of the `beqz flag&1` branch, so it is
 * unconditional; the call is the fall-through. Matched byte-exact on sdk29
 * (cc1 2.9, -O2 -G8 -fno-gcse -fno-strict-aliasing, task #851): the empty asm
 * after the call suppresses cc1's sibling call, so the ROM's jal + frame is
 * reproduced (without it cc1 emits `j func_00337C48` and the arm is 65.00%).
 */
void func_00336678(void *p, s32 flag) {
    *(void **)((char *)p + 0x4) = &D_1AD9A8;
    if (flag & 1) {
        func_00337C48();
    }
    __asm__ __volatile__("");
}

/* GuiComputeBlendWeights: from a single blend factor t, fill a 4-float weight
 * vector for a two-control-point blend: out[0]=t, out[1]=t*0.5, out[2]=1-t,
 * out[3]=(1-t)*0.5 (the .5-scaled entries are the tangent weights). Asserts the
 * output pointer is non-NULL first (AssertFail with the D_1AD890 source path,
 * line 0x3D, predicate D_1AD8A8). Like the other widget-method blends in this
 * unit (func_00336918 / func_00336988) the leading object pointer (a0) is unused
 * by the body; t arrives in $f12 and the output pointer in $a1. */
/* On cc1 2.9 not byte-exact: the 8-byte-packed
   callee-save frame wall (original packs s0/ra/f20 into a -0x20 frame at
   0x0/0x8/0x10; this cc1 emits a -0x30 frame at 0x0/0x10/0x20). The body insn
   stream (assert, the 1.0/0.5 li.s constants, the four stores) is otherwise
   bit-identical. 99.62% best.
   MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (FACT #8810; census FACT #8830).
   GUARD (task #1272): on EE this C is the image's body, compiled alone by the
   s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
   S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
   that skips the splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiComputeBlendWeights)
S136OS_SLOT(GuiComputeBlendWeights);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiComputeBlendWeights, unit objdiff): 78.24%,
   13/31 insns differ. Residual: UNKNOWN-swc1 (first differing insn: 'swc1 fs0, 0x10(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
extern void AssertFail(const char *file, s32 line, const char *expr);
extern const char D_1AD890[]; /* assert source-file path */
extern const char D_1AD8A8[]; /* assert predicate text ("out") */
void GuiComputeBlendWeights(void *self, f32 t, f32 *out) {
    (void)self;
    if (out == 0) {
        AssertFail(D_1AD890, 0x3D, D_1AD8A8);
    }
    out[0] = t;
    out[2] = 1.0f - t;
    out[1] = t * 0.5f;
    out[3] = (1.0f - t) * 0.5f;
}
#endif

/* func_00336720: install the D_1AD968 vtable at p+0x4, then run the base ctor
 * func_00336678(p, flag) (the field store sits in the jal delay slot). */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; the original fills the
   func_00336678 jal delay slot with the +0x4 vtable store; cc1 emits the store
   ahead of the call and nops the slot. Same jal-delay-slot-store wall as
   func_00336B88. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336720);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00336720, unit objdiff): 52.94%,
   8/17 insns differ. Residual: UNKNOWN-sw (first differing insn: 'sw zero, 0x118(a0)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void *D_1AD968;
void func_00336720(void *p, s32 flag) {
    *(void **)((char *)p + 0x4) = &D_1AD968;
    func_00336678(p, flag);
}
#endif

/* func_00336768(arg0, out, t): sample the keyframe track at arg0->field_8 into a
 * Vec4 at out for time t. obj = arg0[+8]; count = obj[+0x10]; kf = obj[+0xC]
 * (linked keyframes: next@+0, time@+0x8, Vec4@+0xC). count 0 -> no-op; count 1 ->
 * copy kf[0]'s Vec4. Otherwise walk the list to the bracket [prev, cur] with
 * prev.time <= t (stopping at cur.time > t, list end, or the count cap), compute
 * f = (t - prev.time)/(cur.time - prev.time), and — only when obj[+0x14] is 0 or 1
 * — write the lerp (1-f)*prev.Vec4 + f*cur.Vec4. Faithful TARGET_NATIVE #else
 * (engine 2.96 FP scheduling/likely-branch = no byte-match). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336768);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00336768, unit objdiff): 55.56%,
   78/106 insns differ. Residual: UNKNOWN-lw (first differing insn: 'lw a2, 0x8(a0)' vs 'lw a4, 0x8(a0)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00336768(void *arg0, void *out, f32 t) {
    u8 *o   = (u8 *)out;
    u8 *obj = *(u8 **)((u8 *)arg0 + 8);
    s32 count = *(s32 *)(obj + 0x10);
    u8 *kf, *prev, *cur, *next;
    s32 i, mode;
    f32 f, invf, pt, ct;

    if (count == 0) {
        return;
    }
    kf = *(u8 **)(obj + 0xC);
    if (count == 1) {
        *(f32 *)(o + 0x0) = *(f32 *)(kf + 0xC);
        *(f32 *)(o + 0x4) = *(f32 *)(kf + 0x10);
        *(f32 *)(o + 0x8) = *(f32 *)(kf + 0x14);
        *(f32 *)(o + 0xC) = *(f32 *)(kf + 0x18);
        return;
    }

    prev = kf;
    cur  = kf;
    next = *(u8 **)kf;
    if (next != 0 && *(f32 *)(kf + 0x8) <= t) {
        for (i = 0; ; ) {
            cur = next;
            i++;
            if ((u32)i >= (u32)count) {
                break;
            }
            next = *(u8 **)cur;
            if (next == 0) {
                break;
            }
            if (*(f32 *)(cur + 0x8) > t) {
                break;
            }
            prev = cur;
        }
    }

    pt = *(f32 *)(prev + 0x8);
    ct = *(f32 *)(cur + 0x8);
    f  = (t - pt) / (ct - pt);
    mode = *(s32 *)(obj + 0x14);
    if (mode < 0 || mode >= 2) {
        return;
    }
    invf = 1.0f - f;
    *(f32 *)(o + 0x0) = invf * *(f32 *)(prev + 0xC)  + f * *(f32 *)(cur + 0xC);
    *(f32 *)(o + 0x4) = invf * *(f32 *)(prev + 0x10) + f * *(f32 *)(cur + 0x10);
    *(f32 *)(o + 0x8) = invf * *(f32 *)(prev + 0x14) + f * *(f32 *)(cur + 0x14);
    *(f32 *)(o + 0xC) = invf * *(f32 *)(prev + 0x18) + f * *(f32 *)(cur + 0x18);
}
#endif

/* func_003368D0: install the D_1AD988 vtable at +0x0 and return the object. */
void *func_003368D0(void *p) {
    *(void **)p = &D_1AD988;
    return p;
}

/**
 * Install the D_1AD988 vtable at p+0x0, then call func_00337C48() when bit 0 of
 * `flag` is set.
 *
 * p: object whose +0x0 slot receives the vtable pointer. flag: bit 0 selects
 * the follow-up call. No return value.
 *
 * The store sits in the delay slot of the `beqz flag&1` branch, so it is
 * UNCONDITIONAL and only the call is conditional - the same shape as
 * func_00336678. (The earlier C had it inverted: a conditional store and an
 * unconditional call.) Matched byte-exact on sdk29 (cc1 2.9, -O2 -G8 -fno-gcse
 * -fno-strict-aliasing, task #858): the empty asm after the call suppresses
 * cc1's sibling call, so the ROM's jal + frame is reproduced.
 */
void func_003368E8(void *p, s32 flag) {
    *(void **)p = &D_1AD988;
    if (flag & 1) {
        func_00337C48();
    }
    __asm__ __volatile__("");
}

/* func_00336918: 4-component linear interpolation dst = (1-t)*a + t*b. The
 * leading object pointer (a0) is unused by the body - this is a widget method
 * whose `this` carries no state into the blend. */
void func_00336918(void *self, f32 t, f32 *dst, f32 *a, f32 *b) {
    f32 it = 1.0f - t;
    dst[0] = it * a[0] + t * b[0];
    dst[1] = it * a[1] + t * b[1];
    dst[2] = it * a[2] + t * b[2];
    dst[3] = it * a[3] + t * b[3];
}

/* func_00336988: per-channel blend of a 4-int vector. For each component i in
 * {0,1,2,3}, out[i] = ColorLerpPacked(t, aSrc[i], bSrc[i]) - the shared scalar
 * color/alpha interpolation helper applied component-wise. The leading object
 * pointer (a0) carries no state into the blend and is unused by the body. */
/* On cc1 2.9 not byte-exact: the 8-byte-packed
   callee-save frame wall (this cc1 emits a -0x50 frame with 16-byte-strided
   spill slots for s0/s1/s2/ra/f20; the original packs them into a -0x30 frame
   at 0x0/0x8/0x10/0x18/0x20). The body insn stream is otherwise identical.
   87.5% best.
   MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (FACT #8810; census FACT #8830).
   GUARD (task #1272): on EE this C is the image's body, compiled alone by the
   s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
   S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
   that skips the splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00336988)
S136OS_SLOT(func_00336988);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00336988, unit objdiff): 60.14%,
   22/43 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a2, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
extern s32 ColorLerpPacked(f32 t, s32 a, s32 b);
void func_00336988(void *self, s32 *out, s32 *aSrc, s32 *bSrc, f32 t) {
    out[0] = ColorLerpPacked(t, aSrc[0], bSrc[0]);
    out[1] = ColorLerpPacked(t, aSrc[1], bSrc[1]);
    out[2] = ColorLerpPacked(t, aSrc[2], bSrc[2]);
    out[3] = ColorLerpPacked(t, aSrc[3], bSrc[3]);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336A18);

/* func_00336A28: cubic Hermite blend of t between the two control values at
 * src+0x8 and src+0xC (endpoint tangents 0 and 1), writing the result to
 * *dst. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00336A28)
S136OS_SLOT(func_00336A28);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00336A28, unit objdiff): 85.07%,
   6/16 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd ra, 0x8(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* On cc1 2.9 not byte-exact: single-callee-save plus
   the float-constant setup hoist the original interleaves differently.
   MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (FACT #8810; census FACT #8830).
   GUARD (task #1272): on EE this C is the image's body, compiled alone by the
   s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
   S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
   that skips the splice drops the function. On native it is plain C, as before. */
extern f32 GuiHermiteInterp(f32 t, f32 c0, f32 c1, f32 c2, f32 c3);
void func_00336A28(f32 *src, f32 t, f32 *dst) {
    *dst = GuiHermiteInterp(t, 0.0f, src[2], src[3], 1.0f);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336A68);

/* func_00336A78: install the D_1AD8C8 vtable at p+0x4, free the pooled node
 * (*(p+0x2C) is the pool, *(p+0x28) the node) via func_00337D78, then run the
 * base ctor func_00336678(p, flag). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00336A78)
S136OS_SLOT(func_00336A78);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00336A78, unit objdiff): 89.50%,
   3/21 insns differ. Residual: SCHED (prologue/epilogue or delay-slot order only; sched1 ON/OFF and sched2 OFF RUN, none reproduce it).
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* On cc1 2.9 not byte-exact: 3-callee-save frame
   wall.
   MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (FACT #8810; census FACT #8830).
   GUARD (task #1272): on EE this C is the image's body, compiled alone by the
   s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
   S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
   that skips the splice drops the function. On native it is plain C, as before. */
extern void *D_1AD8C8;
extern void func_00337D78(void *pool, void **node);
void func_00336A78(void *p, s32 flag) {
    *(void **)((char *)p + 0x4) = &D_1AD8C8;
    func_00337D78(*(void **)((char *)p + 0x2C), *(void ***)((char *)p + 0x28));
    func_00336678(p, flag);
}
#endif

/* func_00336AC8: 4-component lerp between two interleaved vec4s packed in src
 * (a[i] at src+0x8+8i, b[i] at src+0xC+8i): dst[i] = (1-t)*a[i] + t*b[i]. */
void func_00336AC8(f32 *src, f32 t, f32 *dst) {
    f32 it = 1.0f - t;
    dst[0] = it * src[2] + t * src[3];
    dst[1] = it * src[4] + t * src[5];
    dst[2] = it * src[6] + t * src[7];
    dst[3] = it * src[8] + t * src[9];
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336B38);

/* func_00336B68: forward to func_00336678. */
void func_00336B68(void *p, s32 flag) {
    func_00336678(p, flag);
    __asm__ __volatile__("");
}

/*
 * func_00336B88: install the vtable &D_1AD908 at p+0x0, then forward
 * (p, flag) to func_003368E8.
 *
 *   p     the GUI object being initialised; word 0 receives the vtable
 *   flag  passed through unchanged
 *
 * The ROM stores the vtable in the jal delay slot, after the prologue's
 * `sd ra`. cc1 2.9 treats an incoming-argument pointer as unable to alias the
 * frame, so it hoists the store above `sd ra` and reorg can no longer move it
 * into the slot. Passing `p` through a tied empty asm hides its origin: the
 * store stays after `sd ra` and reorg fills the slot with it (task #948). The
 * trailing empty asm keeps cc1 from turning the call into a sibling `j` (the
 * ROM's compiled code never tail-jumps, FACT #8177).
 */
void func_00336B88(void *p, s32 flag) {
    __asm__("" : "+r"(p));
    *(void **)p = &D_1AD908;
    func_003368E8(p, flag);
    __asm__ __volatile__("");
}

/* func_00336BA8: as func_00336B88, installing the vtable &D_1AD8E8; the same
 * tied asm on `p` puts the store in the jal delay slot. */
void func_00336BA8(void *p, s32 flag) {
    __asm__("" : "+r"(p));
    *(void **)p = &D_1AD8E8;
    func_003368E8(p, flag);
    __asm__ __volatile__("");
}

/*
 * Assembler aliases into the gadget-swap block at g_swapGadgetItemIndex
 * (0x1B229A):
 *   g_swapGadgetWord86 = +0x86 (0x1B2320) the swap flag word
 *   g_timerHudYScale   = +0x8A (0x1B2324) the Y scale DrawVehicleRaceTimerHud
 *                        reads (FACT #6124)
 *   g_spriteYFudge     = +0x8E (0x1B2328) the sprite y-fudge
 *                        GuiSpriteElementDraw reads
 * func_00336BC8 writes all three. cc1 2.9 fills a delay slot with a small
 * symbol's load or store only when it carries no offset (FACT #8386), so these
 * are offset-free names for cc1; gas resolves them back to
 * g_swapGadgetItemIndex plus the addend, the ROM's own relocations. Their
 * .extern size decides how gas addresses them: 4 is small, so gas uses
 * %gp_rel($28). 16 is not, so +0x8A gets the absolute `lui $at; swc1 %lo($at)`
 * form the ROM uses there (tasks #948, #978).
 */
#ifndef TARGET_NATIVE
__asm__(".extern g_swapGadgetWord86, 4\n\tg_swapGadgetWord86 = g_swapGadgetItemIndex + 0x86");
__asm__(".extern g_timerHudYScale, 16\n\tg_timerHudYScale = g_swapGadgetItemIndex + 0x8A");
__asm__(".extern g_spriteYFudge, 4\n\tg_spriteYFudge = g_swapGadgetItemIndex + 0x8E");
extern s32 g_swapGadgetWord86;
extern f32 g_timerHudYScale;
extern f32 g_spriteYFudge;
#endif

/*
 * func_00336BC8: seed the gadget-swap zoom animation. Store the swap flag/index
 * (a0) as a word at g_swapGadgetItemIndex+0x86, then two zoom factors: at +0x8A
 * the start scale (1.0 when a0==0, else 1.0769) and at +0x8E the end scale (1.0
 * when a0==0, else 0.9).
 *   flag - swap flag/index; zero selects the 1.0 scales
 *
 * Byte-exact on sdk29 (task #978). The ROM mixes addressing modes per field:
 * +0x86 and +0x8E through %gp_rel($28), +0x8A through the absolute %hi/%lo
 * macro. The aliases above give each field its own mode. Each scale is written
 * as a default then an override on flag == 0, which is the ROM's
 * `li.s; bnez; <slot>; li.s 1.0` shape.
 */
#ifndef TARGET_NATIVE
void func_00336BC8(s32 flag) {
    f32 start = 1.076923f;
    f32 end;
    g_swapGadgetWord86 = flag;
    if (flag == 0) {
        start = 1.0f;
    }
    g_timerHudYScale = start;
    end = 0.9f;
    if (flag == 0) {
        end = 1.0f;
    }
    g_spriteYFudge = end;
}
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00336BC8, unit objdiff): 21.89%,
   19/27 insns differ. Residual: UNKNOWN-lui + gp/abs-mixed symbol (first differing insn: 'lui at, 0x3f89' vs 'addiu v0, gp, %gp_rel(g_swapGadgetItemIndex)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* Native arm. The original reaches +0x86 and +0x8E through the one-insn
   %gp_rel($28) form but +0x8A through the two-insn absolute %hi/%lo macro;
   this plain spelling cannot express that mix (34.67% on sdk29, NOTE #8391).
   The per-field aliases in the EE arm above do (task #978). */
extern s32 g_swapGadgetItemIndex;
void func_00336BC8(s32 flag) {
    char *base = (char *)&g_swapGadgetItemIndex;
    *(s32 *)(base + 0x86) = flag;
    *(f32 *)(base + 0x8A) = (flag != 0) ? 1.076923f : 1.0f;
    *(f32 *)(base + 0x8E) = (flag != 0) ? 0.9f : 1.0f;
}
#endif

/*
 * func_00336C10: return the word at 0x1B2320 (g_waterPool + 0xC0).
 *
 * The ROM reads it as `lw $2, %gp_rel(g_swapGadgetItemIndex + 0x86)($28)` in
 * the `jr` delay slot. cc1 2.9 costs a small symbol's load as one instruction
 * (slot-eligible) only when it carries no offset; symbol+offset is costed as
 * two and left out of the slot. g_swapGadgetWord86 is an ASSEMBLER alias of
 * g_swapGadgetItemIndex + 0x86 sized 4: cc1 sees a plain small symbol and fills
 * the slot, gas addresses it off $gp, and the relocation lands on
 * g_swapGadgetItemIndex with addend 0x86, the ROM's own (task #948). The native
 * build keeps the g_waterPool spelling.
 */
#ifndef TARGET_NATIVE
s32 func_00336C10(void) {
    return g_swapGadgetWord86;
}
#else
extern s32 g_waterPool[];
s32 func_00336C10(void) {
    return g_waterPool[0xC0 / 4];
}
#endif

/* func_00336C18: return the element's pos-vector pointer (+0x0). */
f32 *func_00336C18(GuiElement *e) {
    return e->pos;
}

/* GuiElementGetScaleVec: return the element's scale-vector pointer (+0x4). */
f32 *GuiElementGetScaleVec(GuiElement *e) {
    return e->scale;
}

/* GuiElementGetColor: return the element's color pointer (+0xC). */
s32 *GuiElementGetColor(GuiElement *e) {
    return e->color;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336C30);

/* GuiElementSetPos: write four floats into *(e+0x0), re-reading the vector
 * pointer before every store. */
/* Byte-exact under the canonical engine flags (-fno-strict-aliasing restores
   the per-store e->pos reload the original emits between the four stores). */
void GuiElementSetPos(GuiElement *e, f32 x, f32 y, f32 z, f32 w) {
    e->pos[0] = x;
    e->pos[1] = y;
    e->pos[2] = z;
    e->pos[3] = w;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336C68);

/* GuiElementSetVisible: write the visibility scalar at *(e+0x10): 1.0 shown,
 * 0.0 hidden. */
void GuiElementSetVisible(GuiElement *e, s32 show) {
    if (show) {
        *e->visible = 1.0f;
    } else {
        *e->visible = 0.0f;
    }
}

/* GuiElementIsVisible: true when the visibility scalar is > 0. */
s32 GuiElementIsVisible(GuiElement *e) {
    return *e->visible > 0.0f;
}

/* GuiElementShareScaleVec: rebind the element's scale vector (+0x4) to an
 * externally-owned vector. If the new pointer differs from the current scale,
 * and the element owns a pool (+0x2C) and has not yet released its own scale
 * node (the +0x18 latch is clear), free the old scale node back to the pool
 * (func_00337D78) and set the +0x18 latch; then install the new pointer. */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): func_00337D78 prototyped void in
 * the arm (an implicit int declaration moved the latch constant off $2). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiElementShareScaleVec)
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact: the early-out compares
   compile to branch-likely (beql/bnel) with the shared-tail store sunk into the
   delay slots, plus the 8-byte-packed callee-save frame wall (original -0x20
   frame at 0x0/0x8/0x10; cc1 emits -0x30 at 0x0/0x10/0x20). 95% best. */
S136OS_SLOT(GuiElementShareScaleVec);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiElementShareScaleVec, unit objdiff): 91.60%,
   3/26 insns differ. Residual: SCHED (prologue/epilogue or delay-slot order only; sched1 ON/OFF and sched2 OFF RUN, none reproduce it).
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
extern void func_00337D78(void *pool, void **node);
void GuiElementShareScaleVec(GuiElement *e, f32 *newScale) {
    if (newScale == e->scale) {
        return;
    }
    if (*(void **)((char *)e + 0x2C) != 0 && *(s32 *)((char *)e + 0x18) == 0) {
        func_00337D78(*(void **)((char *)e + 0x2C), (void **)e->scale);
        *(s32 *)((char *)e + 0x18) = 1;
    }
    e->scale = newScale;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336D20);

/* func_00336D28: rebind the element's visibility-scalar vector (+0x10) to an
 * externally-owned vector. If the new pointer differs from the current one, and
 * the element owns a pool (+0x2C) and has not yet released its own visibility
 * node (the +0x24 latch is clear), free the old node back to the pool
 * (func_00337D78) and set the +0x24 latch; then install the new pointer. The
 * +0x10/+0x24 pairing here mirrors the +0x4/+0x18 pairing in
 * GuiElementShareScaleVec. */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): func_00337D78 prototyped void in
 * the arm (an implicit int declaration moved the latch constant off $2). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00336D28)
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact: same beql/bnel early-out
   + 8-byte-packed callee-save frame wall as GuiElementShareScaleVec. */
S136OS_SLOT(func_00336D28);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00336D28, unit objdiff): 91.60%,
   3/26 insns differ. Residual: SCHED (prologue/epilogue or delay-slot order only; sched1 ON/OFF and sched2 OFF RUN, none reproduce it).
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
extern void func_00337D78(void *pool, void **node);
void func_00336D28(GuiElement *e, f32 *newVisible) {
    if (newVisible == e->visible) {
        return;
    }
    if (*(void **)((char *)e + 0x2C) != 0 && *(s32 *)((char *)e + 0x24) == 0) {
        func_00337D78(*(void **)((char *)e + 0x2C), (void **)e->visible);
        *(s32 *)((char *)e + 0x24) = 1;
    }
    e->visible = newVisible;
}
#endif

/* GuiElementSetScale: as GuiElementSetPos for the scale vector at +0x4. */
/* Byte-exact under the canonical engine flags (same per-store reload as
   GuiElementSetPos, on the scale vector at +0x4). */
void GuiElementSetScale(GuiElement *e, f32 x, f32 y, f32 z, f32 w) {
    e->scale[0] = x;
    e->scale[1] = y;
    e->scale[2] = z;
    e->scale[3] = w;
}

/* GuiElementInstallBaseVtable: install the base vtable at +0x30, return e. */
GuiElement *GuiElementInstallBaseVtable(GuiElement *e) {
    *(void **)((char *)e + 0x30) = &g_GuiElementVtable;
    return e;
}

/* GuiElementBaseInit: construct the shared base of a GUI element. When a pool is
 * supplied (a2), stash it at +0x2C and carve five zeroed 16-byte vector blocks
 * from it (via GuiPoolAlloc + GuiPlacementNew), wiring them into the element in
 * allocation order at +0x0 (pos), +0x8 (unk08), +0x4 (scale), +0xC (color) and
 * +0x10 (visible). Then record the element tag (a1) at +0x28, zero the scalar
 * fields +0x14/+0x18/+0x1C/+0x20/+0x24, and mark the element visible. */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - not byte-exact; the 8-byte-packed
   callee-save frame wall (the original packs s0/s1/ra into a -0x20 frame at
   0x0/0x8/0x10; this cc1 emits a -0x30 frame at 0x0/0x10/0x20), plus the
   per-block store scheduling - the original stores the block pointer into the
   element immediately, then zeroes the block (0x0..0xC) with the trailing
   0xc(v0) zero sunk into the next GuiPlacementNew delay slot; cc1 batches them
   differently. 95.9% best. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementBaseInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiElementBaseInit, unit objdiff): 53.08%,
   46/95 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu a0, a2, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void GuiElementBaseInit(GuiElement *e, s32 tag, GuiPool *pool) {
    *(GuiPool **)((char *)e + 0x2C) = pool;
    if (pool != 0) {
        e->pos    = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        e->pos[0] = 0.0f; e->pos[1] = 0.0f; e->pos[2] = 0.0f; e->pos[3] = 0.0f;
        e->unk08  = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        e->unk08[0] = 0.0f; e->unk08[1] = 0.0f; e->unk08[2] = 0.0f; e->unk08[3] = 0.0f;
        e->scale  = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        e->scale[0] = 0.0f; e->scale[1] = 0.0f; e->scale[2] = 0.0f; e->scale[3] = 0.0f;
        e->color  = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        e->color[0] = 0; e->color[1] = 0; e->color[2] = 0; e->color[3] = 0;
        e->visible = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        e->visible[0] = 0.0f; e->visible[1] = 0.0f; e->visible[2] = 0.0f; e->visible[3] = 0.0f;
    }
    *(s32 *)((char *)e + 0x28) = tag;
    *(s32 *)((char *)e + 0x20) = 0;
    *(s32 *)((char *)e + 0x14) = 0;
    *(s32 *)((char *)e + 0x1C) = 0;
    *(s32 *)((char *)e + 0x18) = 0;
    *(s32 *)((char *)e + 0x24) = 0;
    GuiElementSetVisible(e, 1);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336EF8);

/* func_00336F00: shared GuiElement base destructor. Reinstall the base vtable
 * at +0x30, then (only when the element owns a pool at +0x2C) free each of its
 * four still-owned vector nodes back to the pool, guarded by per-node release
 * latches: pos(+0x0)/latch +0x14, unk08(+0x8)/latch +0x1C, scale(+0x4)/latch
 * +0x20, color(+0xC)/latch +0x18 (each free passes a0 = the pool, not the
 * element). Finally, if (flag & 1), run the dtor tail hook func_00337C48.
 * Left INCLUDE_ASM (no #else): the destructor takes a second (flag) arg, but the
 * 1-arg `func_00336F00` extern that the matched tail-callers (func_00337278 /
 * func_00337830) rely on - they call it with a1 left untouched - is part of the
 * matched (non-TARGET_NATIVE) build and cannot coexist with a 2-arg C definition
 * in the same TU. A #else body would require region-splitting that extern, which
 * the matched callers would then fail to satisfy under TARGET_NATIVE. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00336F00);

/* GuiElementInitTypeB: install the TypeB GuiElement vtable (D_1ADA38) at +0x30.
 * The asm installs the base vtable first, but that store is fully overwritten by
 * this one - functionally a single store. Return value (p) is unused by callers. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementInitTypeB);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiElementInitTypeB, unit objdiff): 24.23%,
   13/13 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x10' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame wall
   (the GuiElementInstallBaseVtable call holds p across $16/$31 in a -0x10 frame).
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiElementInitTypeB, run_cmp_235FE8_iso.sh): offset oracle confirms +0x30
   == &D_1ADA38 and every other element byte stays the 0xAA sentinel. */
void GuiElementInitTypeB(void *p) {
    extern void *D_1ADA38;
    *(void **)((char *)p + 0x30) = &D_1ADA38;
}
#endif

/* GuiElementInit: run the base GuiElement init (forwarding tag/pool unchanged),
 * then, when a pool is present (+0x2C != 0), carve two more zeroed 16-byte
 * vector blocks from it (GuiPoolAlloc + GuiPlacementNew) into +0x34 and +0x38,
 * and set the scale vector (*(e+0x4)) to {1.0, 1.0, 1.0}. BOTH latch words are
 * zeroed regardless of pool: +0x48=0 on the pool==0 path via the branch-likely
 * delay slot (and again on the pool!=0 fall-through), and +0x44=0 at the shared
 * tail (.L0033707C) reached by both paths.
 * (The asm leaves a1/a2 untouched across the GuiElementBaseInit call, i.e. this
 * forwards the tag and pool it was called with.) */
/* GUARD (task #1338): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1338 lever): the second pool pointer and the
 * scale pointer read before the block stores (as the ROM loads them), each
 * block zeroed in the order 1,2,3,0 (the scheduler emits the last store first,
 * giving the ROM's 0,4,8,12), and scale[1]/scale[2] stored through fresh
 * reloads of +0x4 rather than one reused local. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiElementInit)
S136OS_SLOT(GuiElementInit);
#else
/* engine96 probe (task #466, cc1 2.96, unit objdiff): 78.90%; not the image arm (see GUARD). */
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact. */
/* cmp-oracle VALIDATED bit-exact vs the original .s on real R5900 on BOTH paths (cmp_GuiElementInit pool!=0, cmp_GuiElementInit_nullpool pool==0; run_cmp_235FE8_iso.sh): the pool!=0 oracle confirms the two pool blocks land at e+0x34/+0x38 zeroed and scale[0..2]=1.0; both oracles confirm +0x44 and +0x48 end up 0 regardless of pool. */
extern void GuiElementBaseInit(GuiElement *e, s32 tag, GuiPool *pool);
void GuiElementInit(GuiElement *e, s32 tag, GuiPool *pool) {
    GuiElementBaseInit(e, tag, pool);
    if (*(GuiPool **)((char *)e + 0x2C) == 0) {
        /* asm: beql $4,$0 with `sw $0,0x48($16)` in the branch-likely delay slot
         * -> on the pool==0 path +0x48 is zeroed and we skip straight to the
         * shared tail that zeroes +0x44. */
        *(s32 *)((char *)e + 0x48) = 0;
    } else {
        f32 *blk0, *blk1, *scale;
        GuiPool *pool2;
        blk0 = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        pool2 = *(GuiPool **)((char *)e + 0x2C);
        *(f32 **)((char *)e + 0x34) = blk0;
        blk0[1] = 0.0f; blk0[2] = 0.0f; blk0[3] = 0.0f; blk0[0] = 0.0f;
        blk1 = GuiPlacementNew(0x10, GuiPoolAlloc(pool2));
        scale = *(f32 **)((char *)e + 0x4);
        *(f32 **)((char *)e + 0x38) = blk1;
        blk1[1] = 0.0f; blk1[2] = 0.0f; blk1[3] = 0.0f; blk1[0] = 0.0f;
        scale[0] = 1.0f;
        (*(f32 **)((char *)e + 0x4))[1] = 1.0f;
        (*(f32 **)((char *)e + 0x4))[2] = 1.0f;
        /* asm L41: `sw $0,0x48($16)` on the pool!=0 fall-through */
        *(s32 *)((char *)e + 0x48) = 0;
    }
    /* asm .L0033707C (shared merge): `sw $0,0x44($16)` on BOTH paths */
    *(s32 *)((char *)e + 0x44) = 0;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337090);

/* func_00337098: list-element ctor. Install the D_1ADA38 vtable at p+0x30; when
 * the pool at p+0x2C is live, free up to two pooled nodes back to it - the node
 * at p+0x34 unless the +0x44 latch is set, and the node at p+0x38 unless the
 * +0x48 latch is set (each free passes a0 = the pool, not p). Then run the
 * shared ctor func_00336F00(p, flag). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00337098)
S136OS_SLOT(func_00337098);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00337098, unit objdiff): 93.33%,
   2/31 insns differ. Residual: SCHED (prologue/epilogue or delay-slot order only; sched1 ON/OFF and sched2 OFF RUN, none reproduce it).
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* On cc1 2.9 not byte-exact: 2-callee-save frame +
   branch-likely (bnel) guard wall.
   MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (FACT #8810; census FACT #8830).
   GUARD (task #1272): on EE this C is the image's body, compiled alone by the
   s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
   S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
   that skips the splice drops the function. On native it is plain C, as before. */
extern void *D_1ADA38;
void func_00337098(void *p, s32 flag) {
    *(void **)((char *)p + 0x30) = &D_1ADA38;
    if (*(s32 *)((char *)p + 0x2C) != 0) {
        if (*(s32 *)((char *)p + 0x44) == 0) {
            func_00337D78(*(void **)((char *)p + 0x2C),
                          *(void ***)((char *)p + 0x34));
        }
        if (*(s32 *)((char *)p + 0x48) == 0) {
            func_00337D78(*(void **)((char *)p + 0x2C),
                          *(void ***)((char *)p + 0x38));
        }
    }
    ((void (*)(void *, s32))func_00336F00)(p, flag);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337110);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337120);

/* GuiSpriteElementDraw: if the element is visible (*(e+0x10) scalar != 0) and
 * has a live texture handle (+0x40), submit the sprite to the 2D blitter
 * func_003017F8 - position (*(e+0x0)), scale (*(e+0x4)) with y pre-scaled by the
 * global sprite y-fudge (g_swapGadgetItemIndex+0x8E), color (*(e+0xC)), the
 * +0x38 vec and the +0x40 handle.
 *
 * Byte-exact on sdk29 (task #978). The y-fudge is read through
 * g_spriteYFudge, the assembler alias of g_swapGadgetItemIndex + 0x8E defined
 * above func_00336BC8: cc1 fills the `beqz` slot only with an offset-free small
 * symbol (FACT #8386), and the relocation still lands on
 * g_swapGadgetItemIndex + 0x8E. Pinning the
 * fudge to $f0 gives the ROM's `mul.s $f15,$f15,$f0`. The volatile read of
 * vec38[0] keeps cc1 from choosing it for the jal slot. The ROM puts pos[0]
 * ($f12) there, after $f14 and $f16. The trailing empty asm keeps the call
 * from becoming a sibling-call `j`.
 *
 * This EE arm keeps the 9-param declaration although the callee reads no
 * $a2/$a3 (FACT #8918). The ROM caller does load scale into $6 and vec38 into
 * $7, and this body is byte-exact as written; task #1311 changed only native
 * arms, so it was not re-measured with the 7-param form. */
#ifndef TARGET_NATIVE
extern void func_003017F8(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                          f32 px, f32 py, f32 sx, f32 syg, f32 v38);
void GuiSpriteElementDraw(void *p) {
    GuiElement *e = (GuiElement *)p;
    s32 handle;
    f32 *pos, *scale, *vec38;
    s32 *color;
    if (e->visible[0] == 0.0f) {
        return;
    }
    handle = *(s32 *)((char *)e + 0x40);
    if (handle == 0) {
        return;
    }
    pos = e->pos;
    scale = e->scale;
    color = *(s32 **)((char *)e + 0xC);
    vec38 = *(f32 **)((char *)e + 0x38);
    {
        register f32 fudge __asm__("$f0") = g_spriteYFudge;
        func_003017F8(handle, color[0], scale, vec38,
                      pos[0], pos[1], scale[0], scale[1] * fudge,
                      *(volatile f32 *)&vec38[0]);
    }
    __asm__ __volatile__("");
}
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiSpriteElementDraw, unit objdiff): 77.14%,
   19/30 insns differ. Residual: UNKNOWN-daddu (first differing insn: 'daddu a1, a0, zero' vs 'mtc1 zero, fv0').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* Native arm. Its call uses the 7-param declaration (FACT #8918); the two
   dead pointer args were dropped in task #1311, after both scores in this
   comment were measured. This plain spelling measured 92.7% on sdk29: structure, frame
   (asm barrier defeats the tail-call), branches and reloc all match; the only
   delta is -O2 instruction scheduling of the independent argument loads - the
   original loads the gp_rel y-fudge into fv0 first then scale[1] into fa3
   (mul.s fa3,fa3,fv0) and fills the jal delay slot with pos[0]; this cc1
   schedules the global load late and hoists pos[0]. The EE arm above closes
   both (task #978). */
extern s32 g_swapGadgetItemIndex;
extern void func_003017F8(s32 handle, s32 color0, f32 px, f32 py, f32 sx, f32 syg, f32 v38);
void GuiSpriteElementDraw(void *p) {
    GuiElement *e = (GuiElement *)p;
    s32 handle;
    f32 *pos, *scale, *vec38;
    s32 *color;
    if (e->visible[0] == 0.0f) {
        return;
    }
    handle = *(s32 *)((char *)e + 0x40);
    if (handle == 0) {
        return;
    }
    pos = e->pos;
    scale = e->scale;
    color = *(s32 **)((char *)e + 0xC);
    vec38 = *(f32 **)((char *)e + 0x38);
    func_003017F8(handle, color[0],
                  pos[0], pos[1], scale[0],
                  scale[1] * *(f32 *)((char *)&g_swapGadgetItemIndex + 0x8E),
                  vec38[0]);
    __asm__ __volatile__("");
}
#endif

/* GuiElementSetGlyph: look up the glyph for (codepoint, font) and store the
 * resulting handle at +0x40. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiElementSetGlyph)
S136OS_SLOT(GuiElementSetGlyph);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiElementSetGlyph, unit objdiff): 82.33%,
   4/13 insns differ. Residual: SCHED (prologue/epilogue or delay-slot order only; sched1 ON/OFF and sched2 OFF RUN, none reproduce it).
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* On cc1 2.9 not byte-exact: 2-callee-save frame wall
   ($16/$31 16-byte vs 8-byte slot packing).
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiElementSetGlyph, run_cmp_235FE8_iso.sh): offset-correctness oracle
   confirms the lookup result lands at e+0x40 (and nowhere else) with arg order
   preserved through a deterministic GuiFontAtlasLookupGlyph mock.
   The callee is the in-unit GuiFontAtlasLookupGlyph (real signature
   (GuiFontAtlas*, s32), defined below); declared here with a matching prototype
   so the TARGET_NATIVE TU has ONE consistent type for the symbol. The matched
   asm forwards a0=codepoint, a1=font verbatim, so the leading int is reached as
   the atlas-ptr register - byte-neutral (the cast changes no instruction).
   MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (FACT #8810; census FACT #8830).
   GUARD (task #1272): on EE this C is the image's body, compiled alone by the
   s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
   S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
   that skips the splice drops the function. On native it is plain C, as before. */
struct GuiFontAtlas;
extern s32 GuiFontAtlasLookupGlyph(struct GuiFontAtlas *atlas, s32 codepoint);
void GuiElementSetGlyph(GuiElement *e, s32 codepoint, s32 font) {
    *(s32 *)((char *)e + 0x40) =
        GuiFontAtlasLookupGlyph((struct GuiFontAtlas *)codepoint, font);
}
#endif

/* GuiElementSetAlpha: write the alpha float through the element's +0x38 ptr. */
void GuiElementSetAlpha(GuiElement *e, f32 alpha) {
    **(f32 **)((char *)e + 0x38) = alpha;
}

/* GuiListRowElementInit: run the base GuiElement vtable install, then overwrite
 * the +0x30 vtable slot with the GuiListRow vtable. Returns the element (the
 * ROM leaves it in $2 at 0x3371F4; every caller discards it). */
/* GUARD (task #1338): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1338 lever): the element pointer is returned
 * (GuiElement *), as the ROM leaves it in $2 (0x3371F4); the void definition
 * was one word short. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiListRowElementInit)
S136OS_SLOT(GuiListRowElementInit);
#else
/* engine96 probe (task #466, cc1 2.96, unit objdiff): 87.69%; not the image arm (see GUARD). */
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact. */
/* cmp-oracle VALIDATED bit-exact vs the original .s on real R5900 (cmp_GuiListRowElementInit, run_cmp_235FE8_iso.sh): offset oracle confirms +0x30 == &g_GuiListRowVtable and every other element byte stays sentinel. */
extern GuiElement *GuiElementInstallBaseVtable(GuiElement *e);
GuiElement *GuiListRowElementInit(void *p) {
    GuiElement *e = (GuiElement *)p;
    GuiElementInstallBaseVtable(e);
    *(void **)((char *)e + 0x30) = &g_GuiListRowVtable;
    return e;
}
#endif

/* GuiListElementInit: run the base GuiElement init using the 4th/5th args as its
 * tag/pool, then store the 2nd arg at +0x3C and the 3rd at +0x34, seed the
 * sentinel word +0x44 = 0x80000000 and the count/limit word +0x40 = 100. */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): the +0x40 store written before the
 * +0x44 store. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiListElementInit)
S136OS_SLOT(GuiListElementInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiListElementInit, unit objdiff): 76.96%,
   14/25 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s0, 0x0(sp)' vs 'sd s1, 0x8(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact: 4-callee-save frame wall
   ($16/$17/$18/$31 16-byte vs 8-byte slot packing) - the four held arg values
   force the deeper save frame.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiListElementInit, run_cmp_235FE8_iso.sh): offset-correctness oracle confirms
   v3C->+0x3C, v34->+0x34, +0x44=0x80000000, +0x40=100, base tag/pool forwarded. */
void GuiListElementInit(GuiElement *e, s32 v3C, s32 v34, s32 tag, GuiPool *pool) {
    GuiElementBaseInit(e, tag, pool);
    *(s32 *)((char *)e + 0x3C) = v3C;
    *(s32 *)((char *)e + 0x34) = v34;
    *(s32 *)((char *)e + 0x40) = 0x64;
    *(s32 *)((char *)e + 0x44) = (s32)0x80000000;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337270);

/* func_00337278: install the GuiListRow vtable at p+0x30, then call
 * func_00336F00(p). */
void func_00337278(void *p) {
    *(void **)((char *)p + 0x30) = &g_GuiListRowVtable;
    func_00336F00(p);
    __asm__ __volatile__("");
}

/* GuiListSetColorPair0: write two colors into *(e+0xC) at +0/+4, re-reading the
 * block pointer between stores. */
/* Byte-exact under the canonical engine flags (-fno-strict-aliasing restores
   the e->color reload between the two stores). */
void GuiListSetColorPair0(GuiElement *e, s32 c0, s32 c1) {
    e->color[0] = c0;
    e->color[1] = c1;
}

/* GuiListSetColorPair1: as GuiListSetColorPair0 at +8/+0xC. */
/* Byte-exact under the canonical engine flags (e->color reload as in
   GuiListSetColorPair0, at +8/+0xC). */
void GuiListSetColorPair1(GuiElement *e, s32 c0, s32 c1) {
    e->color[2] = c0;
    e->color[3] = c1;
}

/* func_003372D0(e, alpha0, alpha1): set the alpha (high) byte of colour words
 * [0] and [1] of the element's colour block (+0xC), preserving each word's low
 * 24 RGB bits. The alpha-only counterpart of GuiListSetColorPair0, which
 * replaces the same two words outright. No return.
 *
 * Two details are load-bearing for the byte match and must survive any
 * refactor (re-run the unit gate after touching either):
 *  - the colour-block pointer is RE-READ from e->color between the two stores,
 *    as the ROM does; a single hoisted load is a different instruction stream;
 *  - the two reads are held in TWO SEPARATE locals. Reusing one local gives
 *    cc1 2.9 one pseudo and therefore one hard register ($7 twice); the ROM
 *    carries the second pointer in $5, and two locals are what let the
 *    allocator colour them apart. (Identity: func_003372D0, 0x003372D0 — the
 *    name is the frozen .s glabel objdiff matches on, so it is not renamed.) */
void func_003372D0(GuiElement *e, s32 alpha0, s32 alpha1) {
    s32 *block0 = e->color;
    s32 *block1;
    block0[0] = (block0[0] & 0xFFFFFF) | (alpha0 << 24);
    block1 = e->color;
    block1[1] = (block1[1] & 0xFFFFFF) | (alpha1 << 24);
}

/* func_00337310(e, alpha0, alpha1): as func_003372D0, on colour words [2] and
 * [3] — the alpha-only counterpart of GuiListSetColorPair1. No return.
 *
 * Same two load-bearing details as func_003372D0 (re-read of e->color between
 * the stores; two separate pointer locals so cc1 2.9 colours them into two
 * registers). The shifted alphas are additionally hoisted into locals here,
 * mirroring the ROM's two early `sll ..,24`; that phrasing was measured
 * byte-exact and func_003372D0's un-hoisted one was too, so the hoist is not
 * itself required — it is kept because it is what was gated.
 * (Identity: func_00337310, 0x00337310; name frozen by the .s glabel.) */
void func_00337310(GuiElement *e, s32 alpha0, s32 alpha1) {
    s32 *block0 = e->color;
    s32 shifted0 = alpha0 << 24;
    s32 shifted1 = alpha1 << 24;
    s32 *block1;
    block0[2] = (block0[2] & 0xFFFFFF) | shifted0;
    block1 = e->color;
    block1[3] = (block1[3] & 0xFFFFFF) | shifted1;
}

/* func_00337350(self): draw a two-bar progress/meter widget. When the gate float
 * at *(self[+0x10]) is non-zero, draws a background bar (func_0027F168 =
 * DrawFlatRect2d) from (ptrA.x, ptrA.y) of width valCf = (f32)(u32)self[+0x3C],
 * and a fill bar of width min(ptrB.x, valCf) — clamping ptrB.x to valCf first.
 * Both share height (int)(ptrA.y + ptrB.y) and colour D_1AD9C8; the &params blocks
 * carry self[+0x44] (bg) / self[+0xC][0..1] (fill). ptrA = self[+0], ptrB =
 * self[+4]. Struct/idiom layout Ghidra-verified (unsigned-int->float + min.s).
 * Faithful TARGET_NATIVE #else (engine 2.96 = no byte-match). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337350);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00337350, unit objdiff): 50.44%,
   87/112 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s1, 0x18(sp)' vs 'sd s3, 0x28(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void *D_1AD9C8;
extern void func_0027F168(s32 x0, s32 y0, s32 x1, s32 y1, void *colour, void *params);

void func_00337350(void *self) {
    u8   *s = (u8 *)self;
    void *colour = D_1AD9C8;
    f32  *gate = *(f32 **)(s + 0x10);

    if (*gate != 0.0f) {
        f32 *ptrA = *(f32 **)(s + 0x00);
        f32 *ptrB = *(f32 **)(s + 0x04);
        f32  ax = ptrA[0];
        f32  ay = ptrA[1];
        f32  valCf = (f32)(u32)*(s32 *)(s + 0x3C);
        f32  bx = ptrB[0];
        f32  bxClamped;
        s32  y1;
        s32  rp[2];

        ptrB[0] = (bx < valCf) ? bx : valCf;
        rp[0] = *(s32 *)(s + 0x44);
        bxClamped = ptrB[0];
        y1 = (s32)(ay + ptrB[1]);
        rp[1] = rp[0];
        func_0027F168((s32)ax, (s32)ay, (s32)(ax + valCf), y1, colour, rp);

        {
            s32 *ptr9 = *(s32 **)(s + 0x0C);
            rp[0] = ptr9[0];
            rp[1] = ptr9[1];
        }
        func_0027F168((s32)ax, (s32)ay, (s32)(ax + bxClamped), y1, colour, rp);
    }
}
#endif

/* func_003374D8: run the base GuiElement init then overwrite the +0x30 vtable
 * slot with the D_1ADA18 vtable; return the object. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003374D8)
S136OS_SLOT(func_003374D8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003374D8, unit objdiff): 84.62%,
   2/14 insns differ. Residual: SCHED (prologue/epilogue or delay-slot order only; sched1 ON/OFF and sched2 OFF RUN, none reproduce it).
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* On cc1 2.9 not byte-exact: 2-callee-save frame
   wall ($16/$31 16-byte vs 8-byte slot packing).
   MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (FACT #8810; census FACT #8830).
   GUARD (task #1272): on EE this C is the image's body, compiled alone by the
   s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
   S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
   that skips the splice drops the function. On native it is plain C, as before. */
extern void *D_1ADA18;
void *func_003374D8(void *p) {
    GuiElementInstallBaseVtable(p);
    *(void **)((char *)p + 0x30) = &D_1ADA18;
    return p;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337510);

/* GuiSpriteElementInit: run the base GuiElement init (forwarding tag/pool
 * unchanged), then when a pool is present carve a zeroed 16-byte vector block
 * from it (via the element's +0x2C pool) into +0x34. Always: zero the +0x34
 * vec's [0],[1] words, seed the position vector (*(e+0x0)) to {100.0, 100.0} and
 * the scale vector (*(e+0x4)) to {64.0, 64.0}, and clear +0x38.
 * (The asm forwards a1/a2 unchanged across the GuiElementBaseInit call, so the
 * pool argument doubles as the carve guard.) */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): the pooled block zeroed in the
 * order 1,2,3,0 (the ROM's 0,4,8,12 after scheduling) and the vectors stored
 * through e->pos/e->scale directly. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiSpriteElementInit)
S136OS_SLOT(GuiSpriteElementInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiSpriteElementInit, unit objdiff): 73.27%,
   17/46 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a2, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact: 3-callee-save frame wall
   ($16/$17/$31 16-byte vs 8-byte slot packing) + the float-const loads scheduled
   across the reloaded +0x34/+0x0/+0x4 pointer reads.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiSpriteElementInit, run_cmp_235FE8_iso.sh): offset-correctness oracle
   confirms the +0x34 block lands zeroed, pos={100,100}, scale={64,64}, +0x38=0. */
void GuiSpriteElementInit(GuiElement *e, s32 tag, GuiPool *pool) {
    f32 *vec, *pos, *scale;
    GuiElementBaseInit(e, tag, pool);
    if (pool != 0) {
        vec = GuiPlacementNew(0x10, GuiPoolAlloc(*(GuiPool **)((char *)e + 0x2C)));
        *(f32 **)((char *)e + 0x34) = vec;
        vec[1] = 0.0f; vec[2] = 0.0f; vec[3] = 0.0f; vec[0] = 0.0f;
    }
    (*(f32 **)((char *)e + 0x34))[0] = 0.0f;
    (*(f32 **)((char *)e + 0x34))[1] = 0.0f;
    e->pos[0] = 100.0f;
    e->pos[1] = 100.0f;
    e->scale[0] = 64.0f;
    e->scale[1] = 64.0f;
    *(s32 *)((char *)e + 0x38) = 0;
}
#endif

/* GuiSpriteGetTextureVec: return the sprite's texture-vec pointer (+0x34). */
f32 *GuiSpriteGetTextureVec(GuiElement *e) {
    return *(f32 **)((char *)e + 0x34);
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003375C8);

/* func_003375D0: install the D_1ADA18 vtable at p+0x30; if the pool at p+0x2C is
 * live and the node slot p+0x38 is still empty, free the node *(p+0x34) back to
 * it (func_00337D78); then run the shared ctor func_00336F00(p, flag). */
/* GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003375D0)
S136OS_SLOT(func_003375D0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003375D0, unit objdiff): 91.67%,
   2/25 insns differ. Residual: SCHED (prologue/epilogue or delay-slot order only; sched1 ON/OFF and sched2 OFF RUN, none reproduce it).
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* Byte-exact on the s136os arm (task #1309; FACT #8830 measured it on the
   blanket object only, because the solo compile met func_00337D78 undeclared:
   its prototype, like D_1ADA18's declaration, lived only inside another
   function's guarded arm, so both are repeated here). cc1 2.9 cannot
   reproduce the 3-callee-save packed frame. */
extern void func_00337D78(void *pool, void **node);
extern void *D_1ADA18;
void func_003375D0(void *p, s32 flag) {
    *(void **)((char *)p + 0x30) = &D_1ADA18;
    if (*(s32 *)((char *)p + 0x2C) != 0 && *(s32 *)((char *)p + 0x38) == 0) {
        /* free node *(p+0x34) back to the pool at *(p+0x2C) (a0 = *(p+0x2C),
           not p - the bnel that would set a0=p is nullified on this path) */
        func_00337D78(*(void **)((char *)p + 0x2C), *(void ***)((char *)p + 0x34));
    }
    ((void (*)(void *, s32))func_00336F00)(p, flag);
}
#endif

/* func_00337630: draw one GUI sprite element. Bails immediately if the element
 * is hidden (its visibility scalar at *(e->0x10) is 0). When the global tint
 * override g_guiTintEnabled is set, the sprite's primitive-block colour word
 * (*(e->0xC) -> [0]) has its low 24 bits (RGB) replaced from g_guiTintRgb while
 * the top 8 bits (alpha) are preserved. It then replicates that colour word
 * across a 4-vertex stack quad, converts the texture coords (*(e->0x34) -> [0,1])
 * to integer via func_0028EDF0, and emits the quad through func_0028F2C0 using
 * the element's screen position (e->pos = e->0x0) and scale (e->0x4). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337630);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00337630, unit objdiff): 74.58%,
   39/70 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* WALL + LIVE-STATE: functional-equivalent #else routed to tester-EE (cannot be
   standalone cmp-oracle'd - it drives the live GS quad-emit path through
   func_0028EDF0/func_0028F2C0 and reads the g_guiTint* render globals). cc1
   walls: the visibility early-out is a `bc1tl` branch-likely (delay slot
   nullified on the taken/return path), the coord conversions are raw `cvt.w.s`,
   and the tint globals are %gp_rel small-data loads. */
extern s32 g_guiTintEnabled;          /* D_1AD9CC: non-zero -> apply RGB tint */
extern u32 g_guiTintRgb;              /* D_1AD9D0: 0x00RRGGBB tint colour */
extern s32 func_0028EDF0(s32 u, s32 v);
extern void func_0028F2C0(s32 handle, s32 x0, s32 y0, s32 x1, s32 y1, void *quad);
void func_00337630(GuiElement *e) {
    f32 *primColorBlock;
    f32 *texCoord;
    f32 *pos;
    f32 *scale;
    s32 quad[4];
    s32 handle;

    if (*e->visible == 0.0f) {
        return;
    }
    if (g_guiTintEnabled) {
        u32 *prim = *(u32 **)((char *)e + 0x0C);
        *prim = (*prim & 0xFF000000) | (g_guiTintRgb & 0x00FFFFFF);
    }
    primColorBlock = *(f32 **)((char *)e + 0x0C);
    texCoord = *(f32 **)((char *)e + 0x34);
    quad[0] = *(s32 *)primColorBlock;
    quad[1] = *(s32 *)primColorBlock;
    quad[2] = *(s32 *)primColorBlock;
    quad[3] = *(s32 *)primColorBlock;
    handle = func_0028EDF0((s32)texCoord[0], (s32)texCoord[1]);
    scale = *(f32 **)((char *)e + 0x04);
    pos = *(f32 **)((char *)e + 0x00);
    func_0028F2C0(handle, (s32)pos[0], (s32)pos[1], (s32)scale[0], (s32)scale[1], quad);
}
#endif

/* GuiSpriteSetTexture: set a sprite element's texture coordinate pair.
 *
 * e - GUI element; the coordinate vector lives behind the pointer at e+0x34.
 * u, v - integer coordinates, converted to float (cvt.s.w) before storing.
 * Returns nothing.
 *
 * Non-obvious: the vector pointer at e+0x34 is loaded SEPARATELY for each of
 * the two stores rather than once into a temp. Writing it as two independent
 * dereferences is load-bearing -- it is what keeps the second `lw` alive
 * instead of letting it be commoned into the first.
 *
 * Functional equivalence was additionally confirmed on real R5900 hardware by
 * the cmp oracle (cmp_GuiSpriteSetTexture, run_cmp_suite.sh): the offset+width
 * oracle shows (f32)u and (f32)v landing at [0] and [1] of the vector, with
 * [2] and [3] left at the sentinel and the e+0x34 pointer itself preserved. */
void GuiSpriteSetTexture(GuiElement *e, s32 u, s32 v) {
    f32 fu = (f32)u;
    f32 fv = (f32)v;
    (*(f32 **)((char *)e + 0x34))[0] = fu;
    (*(f32 **)((char *)e + 0x34))[1] = fv;
}

/* func_00337758: return the first float of the vector at p+0x34 converted to
 * int (cvt.w.s). */
s32 func_00337758(void *p) {
    f32 *vec = *(f32 **)((char *)p + 0x34);
    return (s32)vec[0];
}

/* GuiElementInitTypeC: identical to TypeB but installs the TypeC vtable
 * (D_1AD9F8) at +0x30. Return value (p) unused by callers. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiElementInitTypeC);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiElementInitTypeC, unit objdiff): 24.23%,
   13/13 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x10' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; 2-callee-save frame wall
   (same install-then-overwrite shape as GuiElementInitTypeB).
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiElementInitTypeC, run_cmp_235FE8_iso.sh): offset oracle confirms +0x30
   == &D_1AD9F8 and every other element byte stays the 0xAA sentinel. */
void GuiElementInitTypeC(void *p) {
    extern void *D_1AD9F8;
    *(void **)((char *)p + 0x30) = &D_1AD9F8;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003377A8);

/* GuiTextElementInit: run the base GuiElement init (forwarding its own tag/pool
 * arguments unchanged), then set up a text element: install the D_263B10
 * glyph/format table at +0x34, mark it active (+0x38 = 1, 64-bit), clear the
 * text handle (+0x40), reset the scale vector (*(e+0x4)) to {1.0, 1.0}, and seed
 * the text params: +0x54 = 1, +0x4C = 0x200, +0x50 = 0.7f, +0x44 = 1, +0x48 = 0.
 * (The asm leaves a1/a2 untouched across the GuiElementBaseInit call, i.e. this
 * forwards the tag and pool it was called with.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiTextElementInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiTextElementInit, unit objdiff): 62.26%,
   19/36 insns differ. Residual: UNKNOWN-lw (first differing insn: 'lw a1, 0x4(s0)' vs 'addiu v0, v0, %lo(D_263B10)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed two-save
   frame wall (target uses a -0x10 frame with s0@0x0/ra@0x8; this cc1 emits a
   -0x20 frame) plus the -fno-gcse double-reload of *(e+0x4) collapses. 49%.
   cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiTextElementInit, run_cmp_235FE8_iso.sh): offset-correctness oracle
   confirms +0x34=&D_263B10, +0x38=(s64)1 (both words), +0x40=0, scale={1,1},
   +0x54=1, +0x4C=0x200, +0x50=0.7f (0x3F333333), +0x44=1, +0x48=0; base init
   forwards the tag to +0x28. */
extern void *D_263B10;
extern void GuiElementBaseInit(GuiElement *e, s32 tag, GuiPool *pool);
void GuiTextElementInit(GuiElement *e, s32 tag, GuiPool *pool) {
    f32 *scale;
    GuiElementBaseInit(e, tag, pool);
    *(void **)((char *)e + 0x34) = &D_263B10;
    *(s64 *)((char *)e + 0x38) = 1;
    *(s32 *)((char *)e + 0x40) = 0;
    scale = *(f32 **)((char *)e + 0x4);
    scale[0] = 1.0f;
    scale = *(f32 **)((char *)e + 0x4);
    scale[1] = 1.0f;
    *(s32 *)((char *)e + 0x54) = 1;
    *(s32 *)((char *)e + 0x4C) = 0x200;
    *(f32 *)((char *)e + 0x50) = 0.7f;
    *(s32 *)((char *)e + 0x44) = 1;
    *(s32 *)((char *)e + 0x48) = 0;
}
#endif

/* func_00337830: install the D_1AD9F8 vtable at p+0x30, then call func_00336F00(p). */
void func_00337830(void *p) {
    *(void **)((char *)p + 0x30) = &D_1AD9F8;
    func_00336F00(p);
    __asm__ __volatile__("");
}

/* GuiElementSetTextFlag: store the flag word at +0x44. */
void GuiElementSetTextFlag(GuiElement *e, s32 flag) {
    *(s32 *)((char *)e + 0x44) = flag;
}

/* GuiElementSetText: store the text handle at +0x40. */
void GuiElementSetText(GuiElement *e, s32 text) {
    *(s32 *)((char *)e + 0x40) = text;
}

/* GuiTextElementMeasure: measure the text element's string - forwards the text
 * handle (+0x40), a -1 max-width sentinel, the +0x34 field, and the scale.x
 * (*(scale+0)) to the shared text-measure helper func_0027F858. */
extern s32 func_0027F858(s32 text, s32 maxWidth, s32 arg2, f32 scaleX);
s32 GuiTextElementMeasure(GuiElement *e) {
    return func_0027F858(*(s32 *)((char *)e + 0x40), -1,
                         *(s32 *)((char *)e + 0x34), e->scale[0]);
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337898);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiTextElementDraw);

/* GuiListSetItemCount: set the meter's MAXIMUM (full-scale value) at +0x40.
 * GuiListSetScrollPos divides by it, so it is a range, not a number of items
 * (FACT #8723); the matched name is historical and kept. */
void GuiListSetItemCount(GuiElement *e, s32 count) {
    *(s32 *)((char *)e + 0x40) = count;
}

/* GuiListSetScrollPos: set the bar's fill length. Given a value `pos`, clamp it
 * to the maximum at +0x40, then write the fill length through the +0x04
 * scale-vector pointer as
 *   fill = (clamp / totalRows) * trackLength        (trackLength = +0x3C)
 * The element is a horizontal fill/meter bar, not a scrolling list (FACT
 * #8658): +0x3C is the bar's full width in pixels and callers pass values such
 * as g_sfxVolume / g_musicVolume; the "row" names below are historical.
 * If the maximum is zero the fill collapses to 0. All four int->float
 * conversions are unsigned (the original emits the (u32) widening idiom).
 *   e   - the list element, read through GuiListView (trackLength +0x3C,
 *         rowCount +0x40)
 *   pos - the value to show (clamped to +0x40)
 *
 * Byte-exact on sdk29 (task #1053), EE arm below. It depends on asm_unit.sh's
 * cvt.s.w pin (task #1053): cc1 leaves the first `b` slot empty and GNU as
 * would move the cvt.s.w into it; the ROM keeps `cvt.s.w; b; nop`. Levers
 * (task #1025):
 *   - the clamp is an if/else, which gives the ROM's `daddu; movz`;
 *   - the row count is re-read (rowCount again, not totalRows), as the ROM
 *     reloads +0x40 for the divide. Load-bearing: reusing totalRows there
 *     differs from the ROM in 36/72 words (task #1129);
 *   - the fields are reached by casting `e` at each access. The named fields
 *     are sha-identical to the raw-offset spelling, but a `GuiListView *list`
 *     local is NOT: cc1 keeps it in its own register (`move t0,a0`, +1 word)
 *     because `e` is still live for e->scale (task #1129);
 *   - the unsigned->float widening of the row count is spelt by hand, with an
 *     empty volatile asm heading the negative arm (a SCHEDULING DEVICE, empty
 *     template, emits nothing: RULING #8483's class) so cc1 does not hoist the
 *     `srl` into the `b` slot;
 *   - the ROM's two-nop pad before the div.s is an inline noreorder asm tied
 *     to the divisor and the track length (a SCHEDULING DEVICE, RULING #8435);
 *   - an empty volatile asm after the divide (a SCHEDULING DEVICE, RULING
 *     #8483's class) keeps div.s out of the bltz slot and keeps the ratio in
 *     $f1. */
/* The list subtype's view of a GuiElement: only the two fields the scroll
 * code reads are named (include/gui.h: ext3C / textOrCount40). Pure type info. */
typedef struct GuiListView {
    GuiElement base;
    u8 pad14[0x3C - sizeof(GuiElement)];
    /* 0x3C */ u32 trackLength;  /* full bar width in pixels */
    /* 0x40 */ u32 rowCount;     /* meter maximum (GuiListSetItemCount); the
                                    field name is historical, not a row count */
} GuiListView;
#ifndef TARGET_NATIVE
void GuiListSetScrollPos(GuiElement *e, s32 pos) {
    u32 totalRows = ((GuiListView *)e)->rowCount;
    u32 clamp;
    f32 rowsF, clampF, rowsDivisor, ratio;
    f32 *fill;
    u32 trackLength;

    if (totalRows < (u32)pos) {
        clamp = totalRows;
    } else {
        clamp = pos;
    }
    if ((s32)totalRows >= 0) {
        rowsF = (f32)(s32)totalRows;
    } else {
        __asm__ __volatile__("");  /* SCHEDULING DEVICE (#8483 class), see above */
        rowsF = (f32)(s32)((totalRows & 1) | (totalRows >> 1));
        rowsF = rowsF + rowsF;
    }
    if (rowsF == 0.0f) {
        e->scale[0] = 0.0f;
        return;
    }
    fill = e->scale;
    clampF = (f32)clamp;
    rowsDivisor = (f32)((GuiListView *)e)->rowCount;  /* re-read, see above */
    trackLength = ((GuiListView *)e)->trackLength;
    __asm__ __volatile__(".set noreorder\n\tnop\n\tnop\n\t.set reorder"
                         : "+f"(rowsDivisor) : "r"(trackLength));  /* SCHEDULING DEVICE (#8435): ROM's pad */
    ratio = clampF / rowsDivisor;
    __asm__ __volatile__("");  /* SCHEDULING DEVICE (#8483 class), see above */
    fill[0] = ratio * (f32)trackLength;
}
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiListSetScrollPos, unit objdiff): 76.01%,
   30/78 insns differ. Residual: UNKNOWN-lw + movn/movz (first differing insn: '' vs 'lw a3, 0x3c(a0)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier.
   That was the engine96 arm; the sdk29 EE arm above is byte-exact (task #1053). */
/* Portable arm: same computation, without the EE scheduling devices. */
void GuiListSetScrollPos(GuiElement *e, s32 pos) {
    GuiListView *list = (GuiListView *)e;
    s32 totalRows = (s32)list->rowCount;
    s32 trackLength = (s32)list->trackLength;
    s32 clamp = totalRows;
    if ((u32)totalRows >= (u32)pos) {
        clamp = pos; /* clamp = min(totalRows, pos), unsigned */
    }
    if ((f32)(u32)totalRows == 0.0f) {
        e->scale[0] = 0.0f;
        return;
    }
    e->scale[0] = ((f32)(u32)clamp / (f32)(u32)totalRows) * (f32)(u32)trackLength;
}
#endif

/* func_00337B68: store an int at +0x38. */
void func_00337B68(GuiElement *e, s32 v) {
    *(s32 *)((char *)e + 0x38) = v;
}

/* GuiListSetVisibleRows: set the bar's HEIGHT in pixels: store `rows` as a
 * float in scale.y, through the +0x4 scale-vector pointer. The meter draw
 * 0x337350 uses pos.y + scale.y as the bottom edge of both bar rectangles, and
 * callers pass 16 or 5 (FACT #8723). It is not a row count; the matched name and
 * the parameter name are historical and kept. */
void GuiListSetVisibleRows(GuiElement *e, s32 rows) {
    e->scale[1] = (f32)rows;
}

/* func_00337B88: store an int at +0x44. */
void func_00337B88(GuiElement *e, s32 v) {
    *(s32 *)((char *)e + 0x44) = v;
}

/* A font atlas's glyph lookup table: a 32-bit glyph count at +0x18 followed by
 * an inline array of {codepoint, glyphValue} pairs starting at +0x1C. Only the
 * lookup-relevant tail is typed (the +0x00..+0x18 header is opaque here). The
 * `value` field doubles as a load-time-relative offset that GuiFontAtlasRelocate
 * rebases to an absolute pointer (atlas base + offset). */
typedef struct GuiFontGlyph {
    /* 0x00 */ s32 codepoint;
    /* 0x04 */ s32 value;
} GuiFontGlyph;
typedef struct GuiFontAtlas {
    /* 0x00 */ char header[0x18];
    /* 0x18 */ s32 glyphCount;
    /* 0x1C */ GuiFontGlyph glyphs[1];
} GuiFontAtlas;

/* GuiFontAtlasRelocate: walk the glyph table and rebase each glyph's `value`
 * field from a load-relative offset to an absolute pointer (atlas base + the
 * stored offset), handing the rebased pointer to func_003014A8 (per-glyph
 * fixup/register hook). Runs once after the atlas blob is loaded into memory. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiFontAtlasRelocate)
S136OS_SLOT(GuiFontAtlasRelocate);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiFontAtlasRelocate, unit objdiff): 72.31%,
   10/29 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s2, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* On cc1 2.9 not byte-exact: the body is instruction-for-instruction
   identical (tools/ee/match.sh), but this 4-GPR-save method hits the 8-byte
   callee-save slot stride vs the pinned-cc1 16-byte stride (frame 0x20 w/ sd at
   0x0/0x8/0x10/0x18; our cc1 emits frame 0x40 w/ 16-byte stride).
   MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (FACT #8810; census FACT #8830).
   GUARD (task #1272): on EE this C is the image's body, compiled alone by the
   s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
   S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
   that skips the splice drops the function. On native it is plain C, as before. */
extern void func_003014A8(void *p);
void GuiFontAtlasRelocate(GuiFontAtlas *atlas) {
    s32 count = atlas->glyphCount;
    GuiFontGlyph *g = atlas->glyphs;
    s32 i;
    for (i = 0; i < count; i++) {
        g[i].value = (s32)((char *)g[i].value + (s32)atlas);
        func_003014A8((void *)g[i].value);
    }
}
#endif

/* GuiFontAtlasLookupGlyph: linear-scan the atlas glyph table for the entry whose
 * codepoint matches `codepoint`, returning its glyph value (0 if not found).
 * Hoisting the glyph base into a local pointer is load-bearing for the match:
 * it makes cc1 keep the index-addressed scan (entries + i*8) the original uses
 * rather than folding +0x1C into each load displacement.
 *
 * Under TARGET_NATIVE only, the definition is weak so the cmp harness's
 * deterministic GuiFontAtlasLookupGlyph mock (a strong def in cmp_235FE8.c) wins
 * the link without a multiple-definition error, while the host functional-equiv
 * harness (no mock) still gets this body. The matched (#ifndef TARGET_NATIVE)
 * build is unaffected - no attribute, so the symbol binding and bytes are
 * unchanged. */
#ifdef TARGET_NATIVE
__attribute__((weak))
#endif
s32 GuiFontAtlasLookupGlyph(GuiFontAtlas *atlas, s32 codepoint) {
    s32 count = atlas->glyphCount;
    GuiFontGlyph *glyphs = atlas->glyphs;
    s32 i = 0;
    s32 result = 0;
    while (i < count) {
        if (glyphs[i].codepoint == codepoint) {
            result = glyphs[i].value;
            break;
        }
        i++;
    }
    return result;
}

/* func_00337C48: no-op stub (empty body - registered/overridable hook). */
void func_00337C48(void) {
}

/* GuiPlacementNew: placement-new passthrough - return the supplied buffer. */
void *GuiPlacementNew(s32 size, void *buf) {
    return buf;
}

/* func_00337C58: identity passthrough - return the first arg. */
void *func_00337C58(void *p) {
    return p;
}

/* rodata assert strings shared by the GUI pool routines:
 *   D_1ADA78 = source file path, D_1ADA98 = elemKind-bound predicate (GuiPoolInit),
 *   D_1ADAC0 = capacity predicate (GuiPoolAlloc). */
extern const char D_1ADA78[];
extern const char D_1ADA98[];
extern const char D_1ADAC0[];

/* GuiPoolInit: initialise a GUI fixed-size pool header.
 *   pool->base     (+0x00) = storage   (a2)
 *   pool->capacity (+0x04) = byteLimit  (a3)
 *   pool->elemSize (+0x08) = elemSize   (a1, asserted >= 4 - a node must be big
 *                                        enough to hold the free-list next ptr)
 *   cursor/count/freeList (+0x0C/+0x10/+0x14) zeroed.
 * An elemSize < 4 trips AssertFail. The asm `sltiu $2,a1,4; beqz $2,stores`
 * SKIPS the assert when (elemSize < 4) is FALSE (>= 4) and FALLS THROUGH to it
 * when (elemSize < 4) is TRUE, so the assert fires for elemSize in [0,4).
 * MATCHED on the s136os arm: the 4-GPR 8-byte-stride save layout is SN 2.95.3
 * v1.36 -fopt-stack's (FACT #8810).
 * GUARD (task #1313): on EE this C is the image's body, compiled alone by the
 * s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
 * S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
 * that skips the splice drops the function. On native it is plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiPoolInit)
S136OS_SLOT(GuiPoolInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiPoolInit, unit objdiff): 59.42%,
   18/37 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s0, 0x0(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* cmp-oracle VALIDATED bit-exact vs the original .s on real R5900
   (cmp_GuiPoolInit, run_cmp_235FE8_iso.sh): field-offset oracle + an AssertFail
   call-count probe confirm every field lands at its offset AND the bound fires
   on exactly the elemSize<4 side. (The earlier `>= 4` form was direction-
   inverted - it asserted on the valid large-size case; the oracle caught it.) */
void GuiPoolInit(GuiPool *pool, s32 elemSize, void *storage, u32 byteLimit) {
    if ((u32)elemSize < 4) {
        AssertFail(D_1ADA78, 0x25, D_1ADA98);
    }
    pool->base = (char *)storage;
    pool->capacity = byteLimit;
    pool->elemSize = elemSize;
    /* SN 1.36's scheduler emits the LAST of these three independent stores
     * first, so this order yields the ROM's +0x10, +0x14, +0x0C (task #1313). */
    pool->freeList = 0;
    pool->cursor = 0;
    pool->count = 0;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337CE0);

/* GuiPoolAlloc: allocate one node from a GUI fixed-size pool.
 *   +0x00 base    pointer to the backing storage
 *   +0x04 capacity (byte limit)
 *   +0x08 elemSize
 *   +0x0C cursor   bump offset into the backing storage
 *   +0x10 count    live allocation count
 *   +0x14 freeList head of the singly-linked free list (next ptr at node+0)
 * If the free list is non-empty, pop its head; otherwise bump-allocate
 * base+cursor, advancing cursor by elemSize. Overflowing the capacity trips an
 * assert and returns NULL. */
void *GuiPoolAlloc(GuiPool *pool) {
    void *node = pool->freeList;
    if (node != 0) {
        pool->freeList = *(void **)node;
        pool->count++;
        return node;
    }
    if (pool->cursor + pool->elemSize <= pool->capacity) {
        char *result = pool->base + pool->cursor;
        pool->cursor += pool->elemSize;
        pool->count++;
        return result;
    }
    AssertFail(D_1ADA78, 0x53, D_1ADAC0);
    return 0;
}

/* func_00337D78: push a node onto the pool free-list at p+0x14 and decrement
 * the live count at p+0x10. */
/* Byte-exact under the canonical engine flags. */
void func_00337D78(void *pool, void **node) {
    *node = *(void **)((char *)pool + 0x14);
    *(void **)((char *)pool + 0x14) = node;
    *(s32 *)((char *)pool + 0x10) -= 1;
}

/* func_00337D98: return the small-data global D_1ADAF0. */
extern s32 D_1ADAF0;
/* func_00337D98: accessor - return the D_1ADAF0 small-data global. */
s32 func_00337D98(void) {
    return D_1ADAF0;
}

/*
 * func_00337DA0: bounds-checked lookup into the 4-entry table D_1ADAD8.
 *
 *   idx  table index
 *   ->   D_1ADAD8[idx] when idx < 4, else 0
 *
 * The ROM takes the table base with one `addiu $2, $gp, %gp_rel(D_1ADAD8)`,
 * i.e. its compiler held the table as -G8 small data. A 16-byte declaration is
 * not small to cc1, which then splits the address into %hi/%lo; declaring it
 * 8 bytes on the EE arm reproduces the ROM (task #948). D_1ADAD8 is referenced
 * nowhere else in this unit.
 */
#ifndef TARGET_NATIVE
extern s32 D_1ADAD8[2];
#else
extern s32 D_1ADAD8[4];
#endif
s32 func_00337DA0(u32 idx) {
    if (idx < 4) {
        return D_1ADAD8[idx];
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00337DC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003380B8);

/* func_00338A80: init two embedded type-B GUI elements (at p+0x10 and p+0x5C),
 * return the object. */
#if defined(S136OS_func_00338A80) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_00338A80; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 13/13 words + 2 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_00338A80 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_00338A80(void *p) {
    GuiElementInitTypeB((char *)p + 0x10);
    GuiElementInitTypeB((char *)p + 0x5C);
    return p;
}
#else
S136OS_SLOT(func_00338A80);
#endif

/* func_00338AB8(self, pool): construct a two-glyph GUI widget (an on/off toggle
 * chip). Clears its state fields (byte +0xA8; words +0x1A8/+0x1AC/+0x1C8/+0x1CC = 0,
 * +0x1C4 = 2, +0x1B0 = 1, +0x1D4 = 0x28) and zeroes the 0x78-byte block at +0x1D8.
 * If a pool is given, allocates four 16-byte vec4 slots (GuiPoolAlloc +
 * GuiPlacementNew) for the widget's size (+0x0 = {255,108,0,0}), colour
 * (+0x8 = {0x60442D00,..}), position (+0x4) and a fourth vector (+0xC). Then inits
 * the two child glyph elements at +0x10 (tag D_1ADB20, glyph 0x57, alpha 0.02,
 * colour 0x55F0C070) and +0x5C (tag D_1ADB30, glyph 0x58, colour 0x55F0C070), and
 * finishes by writing the +0xC vector's colour word 0x80F0F0F0, clearing +0x1B8,
 * and zeroing the +0x4 vector. Faithful TARGET_NATIVE #else (engine 2.96 = no
 * byte-match). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338AB8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00338AB8, unit objdiff): 57.99%,
   122/159 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x40' vs 'addiu sp, sp, -0x30').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 D_1ADB20[], D_1ADB30[];
extern char *g_guiInstance;
void func_00338AB8(void *self, GuiPool *pool) {
    u8  *w = (u8 *)self;
    u32 *o;

    *(u8 *)(w + 0xA8)   = 0;
    *(u32 *)(w + 0x1A8) = 0;
    *(u32 *)(w + 0x1AC) = 0;
    *(u32 *)(w + 0x1B0) = 1;
    *(u32 *)(w + 0x1C4) = 2;
    *(u32 *)(w + 0x1C8) = 0;
    *(u32 *)(w + 0x1CC) = 0;
    *(u32 *)(w + 0x1D4) = 0x28;
    memset(w + 0x1D8, 0, 0x78);

    if (pool != 0) {
        o = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(f32 *)(o + 0) = 255.0f;
        *(f32 *)(o + 1) = 108.0f;
        o[2] = 0; o[3] = 0;
        *(u32 **)(w + 0x0) = o;

        o = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        o[0] = 0x60442D00; o[1] = 0; o[2] = 0; o[3] = 0;
        *(u32 **)(w + 0x8) = o;

        o = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(u32 **)(w + 0x4) = o;
        o[0] = 0; o[1] = 0; o[2] = 0; o[3] = 0;

        o = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(u32 **)(w + 0xC) = o;
        o[0] = 0; o[1] = 0; o[2] = 0; o[3] = 0;
    }

    GuiElementInit((GuiElement *)(w + 0x10), (s32)D_1ADB20, pool);
    GuiElementSetGlyph((GuiElement *)(w + 0x10), (s32)g_guiInstance + 0x8710, 0x57);
    GuiElementSetAlpha((GuiElement *)(w + 0x10), 0.02f);
    GuiElementSetVisible((GuiElement *)(w + 0x10), 1);
    *GuiElementGetColor((GuiElement *)(w + 0x10)) = 0x55F0C070;

    GuiElementInit((GuiElement *)(w + 0x5C), (s32)D_1ADB30, pool);
    GuiElementSetGlyph((GuiElement *)(w + 0x5C), (s32)g_guiInstance + 0x8710, 0x58);
    GuiElementSetVisible((GuiElement *)(w + 0x5C), 1);
    *GuiElementGetColor((GuiElement *)(w + 0x5C)) = 0x55F0C070;

    *(u32 *)(*(u32 **)(w + 0xC)) = (s32)0x80F0F0F0;
    *(u32 *)(w + 0x1B8) = 0;
    *(u32 *)(*(u32 **)(w + 0x4)) = 0;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338CD0);

/* func_00338CD8: configure a scrolling-list widget.
 *  - Store the row-count (a1) at +0x1C4 and the page-size (a2) at +0x1B8.
 *  - Seed the widget's pos vector (*(p+0x0)) with (x,y), clear the +0x1A7 byte
 *    flag, and set the +0x1B0 "active" flag.
 *  - Build the embedded sub-list at p+0xA8: record its address at +0x1D8, run
 *    func_0027F7F8(p+0xA8, -1) and keep its handle at +0x1D4.
 *  - Initialise the widget's scale vector (*(p+0x4)) x-component to 1.0. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00338CD8)
S136OS_SLOT(func_00338CD8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00338CD8, unit objdiff): 50.30%,
   20/33 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd ra, 0x8(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* On cc1 2.9 not byte-exact: reloaded-pointer CSE
   (the original reloads *(p+0x0) before each pos store) + jal-delay-slot store
   scheduling wall.
   MATCHED on the s136os arm: SN 2.95.3 v1.36 -fopt-stack compiles this body
   byte-exact (FACT #8810; census FACT #8830).
   GUARD (task #1272): on EE this C is the image's body, compiled alone by the
   s136os arm (row in tools/ee/s136os_functions.txt) and spliced over
   S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a build
   that skips the splice drops the function. On native it is plain C, as before. */
extern s32 func_0027F7F8(void *p, s32 mode);
void func_00338CD8(void *p, s32 a1, f32 x, f32 y, s32 a2) {
    void *sub = (char *)p + 0xA8;
    *(s32 *)((char *)p + 0x1C4) = a1;
    (*(f32 **)p)[0] = x;
    (*(f32 **)p)[1] = y;
    *(u8 *)((char *)p + 0x1A7) = 0;
    *(s32 *)((char *)p + 0x1B8) = a2;
    *(s32 *)((char *)p + 0x1D8) = (s32)sub;
    *(s32 *)((char *)p + 0x1D4) = func_0027F7F8(sub, -1);
    *(s32 *)((char *)p + 0x1B0) = 1;
    (*(f32 **)((char *)p + 0x4))[0] = 1.0f;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338D48);

/* Init a widget sub-block: copy a 0xFF-byte template into p+0x2084, then run the
 * setup helper func_00338D48 on p+0x1FDC with kind 2, a caller id (or the 0x168
 * default when id == -1), and fixed 255.0/80.0 params. */
/* GUARD (task #1338): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1338 lever): the +0x1FDC box pointer is formed
 * before the first call, as the ROM folds it into the saved $16. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00338F18)
S136OS_SLOT(func_00338F18);
#else
/* engine96 probe (task #466, cc1 2.96, unit objdiff): 60.12%; not the image arm (see GUARD). */
void func_00338F18(void *p, const void *src, s32 id) {
    s32 kind = (id != -1) ? id : 0x168;
    char *box = (char *)p + 0x1FDC;
    func_00115AC0((char *)p + 0x2084, src, 0xFF);
    func_00338D48(box, 2, 255.0f, 80.0f, kind);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338F80);

/* func_00338F88(a1, layout, color, val, font, reposFlag, drawFlag, sub, n9, n10,
 * n11): format + draw a bordered text box. Builds the caption into a 768-byte
 * scratch buffer by selecting one of seven format strings (D_1ADBA8/98/90/70/58/
 * 48/38) from the n9/n10/n11/sub argument combination (page/count indicators),
 * then draws the text with the font selected by `font` (1=DrawFont1TextBox
 * func_00280B48, 0=DrawFont2 func_00280BB8, 2=DrawFont3 func_00280C28). If
 * reposFlag is set, vertically re-centers the box (fields at +0x0/+0x2/+0xA from
 * height +0xE and g_screenHeight). Computes the border rect from the box
 * half-extents (+0xC width, +0xE height, centered on +0x8/+0xA) padded 10x5, draws
 * it (DrawTextBoxBorder func_0027F208 + the frame func_00339398), redraws the text,
 * and restores the layout flag word (+0x12). The +0x12 flag word is toggled with
 * bits 3/4 (|=3, |=4, &=~4) across the passes. Faithful TARGET_NATIVE #else (engine
 * 2.96 = no byte-match); all-integer args (no float-arg reordering); the one
 * mixed-arg call (func_00339398) is asm-verified. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00338F88);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00338F88, unit objdiff): 50.33%,
   217/303 insns differ. Residual: UNKNOWN-daddu + gp/abs-mixed symbol (first differing insn: '' vs 'daddu s0, a5, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00280B48(void *layout, u32 color, const char *text, s32 flag);
extern void func_00280BB8(void *layout, u32 color, const char *text, s32 flag);
extern void func_00280C28(void *layout, u32 color, const char *text, s32 flag);
extern void func_0027F208(s32 top, s32 bottom, s32 left, s32 right, s32 thickness,
                          u32 color);
extern void func_00339398(f32 x0, f32 y0, f32 x1, f32 y1, s32 a1, s32 thickness,
                          u32 color);
extern s32 g_screenHeight;
extern u8 D_1ADB38[], D_1ADB48[], D_1ADB58[], D_1ADB70[], D_1ADB90[], D_1ADB98[],
          D_1ADBA8[];
void func_00338F88(void *a1, void *layout, u32 color, s32 val, s32 font,
                   s32 reposFlag, s32 drawFlag, s32 sub, s32 n9, s32 n10, s32 n11) {
    s16 *ps = (s16 *)layout;
    char buf[768];
    s16 savedFlags = ps[9];
    s32 dx, dy, top, bottom, left, right;

    ps[9] |= 3;

    if (n9 == 0) {
        func_00115DA8(buf, (char *)D_1ADBA8, val);
    } else if (n10 == 0) {
        if (sub == 0) func_00115DA8(buf, (char *)D_1ADB98, val, 1, 1, n9);
        else          func_00115DA8(buf, (char *)D_1ADB90, val, 1, n9);
    } else if (n11 == 0) {
        if (sub == 0) func_00115DA8(buf, (char *)D_1ADB70, val, 1, 1, n9);
        else          func_00115DA8(buf, (char *)D_1ADB58, val, 1, n9, n10);
    } else if (sub == 0) {
        func_00115DA8(buf, (char *)D_1ADB48, val, 1, 1, n9, 1, n10);
    } else {
        func_00115DA8(buf, (char *)D_1ADB38, val, 1, n9, 1);
    }
    ps[9] |= 4;

    if (font == 1)      func_00280B48(layout, color, buf, drawFlag);
    else if (font == 0) func_00280BB8(layout, color, buf, drawFlag);
    else if (font == 2) func_00280C28(layout, color, buf, drawFlag);

    if (reposFlag != 0) {
        s16 h = ps[7];
        s16 y = (s16)((g_screenHeight - h) >> 1);
        ps[1] = y + h;
        ps[5] = y + (h >> 1);
        ps[0] = y;
    }

    if (font == 1)      func_00280B48(layout, color, buf, drawFlag);
    else if (font == 0) func_00280BB8(layout, color, buf, drawFlag);
    else if (font == 2) func_00280C28(layout, color, buf, drawFlag);

    dx = (ps[6] >> 1) + 10;
    dy = (ps[7] >> 1) + 5;
    right  = ps[4] + dx;
    bottom = ps[5] + dy;
    top    = ps[5] - dy;
    left   = ps[4] - dx;
    ps[9] &= (s16)0xFFFB;
    func_0027F208(top, bottom, left, right, 0x60, 0x60442D00);
    func_00339398((f32)top, (f32)bottom, (f32)left, (f32)right, (s32)a1, 0x60,
                  0x55F0C070);

    if (font == 1)      func_00280B48(layout, color, buf, drawFlag);
    else if (font == 0) func_00280BB8(layout, color, buf, drawFlag);
    else if (font == 2) func_00280C28(layout, color, buf, drawFlag);

    ps[9] = savedFlags;
}
#endif

/* func_00339398(x0, y0, x1, y1, base, packByte, color): draw a 6-quad rounded
 * frame (corner + edge textures) around the rect (x0,y0)-(x1,y1). Overrides the
 * camera projection scale to 0.62 (BuildCameraProjection) for the duration, packs
 * packByte into the top byte of `color`, and emits 6 textured quads via
 * func_003017F8: two corner columns (texture base+0x222C, half-height
 * ((y0-x0... see below))) and four edge pieces (texture base+0x2230), with the
 * sx/syg sign pair (+/-1) selecting each quad's mirror orientation.
 *
 * func_003017F8 consumes only $a0/$a1 and $f12-$f16 (FACT #8918), so the native
 * declaration above GuiSpriteElementDraw is the 7-param form and this caller calls
 * it directly. (It used to cast to a 7-arg pointer type because that declaration
 * carried two dead pointer params; task #1311 removed them.)
 * Faithful TARGET_NATIVE #else (engine 2.96 = no byte-match). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339398);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00339398, unit objdiff): 61.12%,
   134/174 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x80' vs 'addiu sp, sp, -0x70').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern char *g_guiInstance;
extern u8 g_sceneActorMobys[];
extern void BuildCameraProjection(void);
void func_00339398(f32 x0, f32 y0, f32 x1, f32 y1, s32 base, s32 packByte,
                   u32 color) {
    f32 *projScale = (f32 *)(g_sceneActorMobys + 0x674 + 0xB0);
    f32  saved = *projScale;
    if (g_guiInstance != 0) {
        f32 syg = 1.0f;                 /* 0.9f when func_00336C10() == 1 */
        f32 hCorner, hEdge;
        s32 texCorner, texEdge;
        *projScale = 0.62f;
        BuildCameraProjection();
        hCorner = ((y0 - x0) - 26.0f) * 0.008928572f * 0.31100002f + 0.0375f;
        if (func_00336C10() == 1) {
            syg = 0.9f;
        }
        color = ((s32)((u32)packByte << 0x18)) | color;
        texCorner = *(s32 *)(base + 0x222C);
        func_003017F8(texCorner, color, x1, x0, -1.0f, syg, hCorner);
        func_003017F8(texCorner, color, y1, x0,  1.0f, syg, hCorner);
        hEdge = ((y1 - x1) - 150.0f) * 0.003267974f * 0.43300003f + 0.102f;
        texEdge = *(s32 *)(base + 0x2230);
        func_003017F8(texEdge, color, x1 + 16.0f, x0, -1.0f, 1.0f, hEdge);
        func_003017F8(texEdge, color, y1 - 16.0f, x0,  1.0f, 1.0f, hEdge);
        func_003017F8(texEdge, color, x1 + 16.0f, y0, -1.0f, -1.0f, hEdge);
        func_003017F8(texEdge, color, y1 - 16.0f, y0,  1.0f, -1.0f, hEdge);
        *projScale = saved;
        BuildCameraProjection();
    }
}
#endif

/* Format a localized caption into a widget sub-block: sprintf the text of
 * localized string `strId` (used as the format) with `fmtArg` into p+0x1E34,
 * then run the setup helper func_00338CD8 on p+0x1D8C with kind 2, a caller id
 * (or the 0x78 default when id == -1), and fixed 255.0/135.0 params. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003395F0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_00338CD8(void *p, s32 a1, f32 x, f32 y, s32 a2);
/* (end of this body's declarations) */
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003395F0, unit objdiff): 62.82%,
   28/37 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0x30' vs 'addiu sp, sp, -0x20').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else.
   SCREEN-EXACT on the s136os arm (SN 2.95.3 v1.36 -fopt-stack; task #1389,
   masked word screen + relocation compare, NOT vmu): a CANDIDATE, not a match.
   Lever (the one that closed its sibling func_00339678, task #1338): the
   +0x1D8C box pointer is formed before the first call, so cc1 folds it into
   the saved $16 as the ROM does. Relocation note: the ROM's sprintf call
   resolves to the same address as func_00115DA8 (both PROVIDEd on EE). */
void func_003395F0(void *p, s32 strId, s32 fmtArg, s32 id) {
    s32 kind = (id != -1) ? id : 0x78;
    char *box = (char *)p + 0x1D8C;
    func_00115DA8((char *)p + 0x1E34, (const char *)GetLocalizedString(strId), fmtArg);
    func_00338CD8(box, 2, 255.0f, 135.0f, kind);
}
#endif

/* Sibling of func_00338F18: copy a 0xFE-byte template into p+0x1E34, then run
 * func_00338D48 on p+0x1D8C with kind 2, a caller id (or the 0xB4 default when
 * id == -1), and fixed 255.0/123.0 params. */
/* GUARD (task #1338): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1338 lever): the +0x1D8C box pointer is formed
 * before the first call, as the ROM folds it into the saved $16. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00339678)
S136OS_SLOT(func_00339678);
#else
/* engine96 probe (task #466, cc1 2.96, unit objdiff): 60.12%; not the image arm (see GUARD). */
void func_00339678(void *p, const void *src, s32 id) {
    s32 kind = (id != -1) ? id : 0xB4;
    char *box = (char *)p + 0x1D8C;
    func_00115AC0((char *)p + 0x1E34, src, 0xFE);
    func_00338D48(box, 2, 255.0f, 123.0f, kind);
}
#endif

/* func_003396E0: no-op stub (empty body - registered/overridable hook). */
void func_003396E0(void) {
}

/* func_003396E8: map a small selector to a scale constant - selector 0 -> 0.7,
 * 1 -> 0.8, anything else -> 1.0. The first argument is ignored.
 *   unused - ignored
 *   sel    - selector
 *   returns the scale in $f0
 *
 * Byte-exact on sdk29 (task #1025), EE arm below. The ROM is a two-case switch
 * lowered as `sel == 1`, then `sel < 2` (slti), then `sel != 0`, with each case
 * returning on its own. The gotos spell that tree; a C `switch` makes cc1 2.9
 * test 0 first. Each return carries a tied empty volatile asm on the result, a
 * SCHEDULING DEVICE (emits nothing): without it cc1 puts the li.s into the
 * `j $31` slot, where the ROM leaves `jr $31; nop` after each lui/ori/mtc1.
 * The same asm without `volatile` does not hold (measured). */
#ifndef TARGET_NATIVE
f32 func_003396E8(s32 unused, s32 sel) {
    f32 r;
    if (sel == 1) goto one;
    if (sel >= 2) goto other;
    if (sel != 0) goto other;
    r = 0.7f;
    __asm__ __volatile__("" : "+f"(r));  /* scheduling device, see above */
    return r;
one:
    r = 0.8f;
    __asm__ __volatile__("" : "+f"(r));  /* scheduling device, see above */
    return r;
other:
    r = 1.0f;
    __asm__ __volatile__("" : "+f"(r));  /* scheduling device, see above */
    return r;
}
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003396E8, unit objdiff): 10.71%,
   21/27 insns differ. Residual: UNKNOWN-lui (first differing insn: '' vs 'lui at, 0x3f4c').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier.
   That was the engine96 arm; the sdk29 EE arm above is byte-exact (task #1025). */
/* Portable arm: same mapping, without the EE scheduling device. */
f32 func_003396E8(s32 unused, s32 sel) {
    if (sel == 1) {
        return 0.8f;
    }
    if (sel == 0) {
        return 0.7f;
    }
    return 1.0f;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339740);

/* func_00339A80: no-op stub (empty body - registered/overridable hook). */
void func_00339A80(void) {
}

/* Refresh a widget with a live sub-value (*(w+0x4)[0] != 0): dispatch on its
 * mode (+0x1C8) — mode 0 runs the sub-updater func_00339740, mode 2 the (empty)
 * func_00339A80 — then always run the (empty) tail func_003396E0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339A88);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00339A88, unit objdiff): 73.47%,
   13/32 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu v1, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00339A88(void *w) {
    f32 *p = *(f32 **)((char *)w + 0x4);
    if (p[0] == 0.0f) {
        return;
    }
    switch (*(s32 *)((char *)w + 0x1C8)) {
    case 0: func_00339740(w); break;
    case 2: func_00339A80();  break;
    default: break;
    }
    func_003396E0();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339B00);

/* func_00339F90: no-op stub (empty body - registered/overridable hook). */
void func_00339F90(void) {
}

/* func_00339F98: temporarily override a camera-projection parameter, rebuild the
 * projection, run one per-mode scene update, then restore the parameter and
 * rebuild again. *(w+0x4) is a curve/animator whose first float gates the whole
 * op: if it is 0 the widget is inactive and nothing happens. The overridden
 * parameter is a float at g_sceneActorMobys+0x674+0xB0 (an unnamed projection
 * sub-struct co-located with the scene-cast region); it is forced to 0.62 for
 * the duration. Mode = *(w+0x1C8): 0 -> func_00339B00(w) (the full per-frame
 * update); 2 -> func_00339F90() (stub hook); any other value -> no update. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00339F98);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00339F98, unit objdiff): 68.45%,
   26/47 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x30' vs 'addiu sp, sp, -0x20').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 g_sceneActorMobys[];
extern void func_00339B00(void *w);
extern void BuildCameraProjection(void);
void func_00339F98(void *w) {
    f32 *camParam;
    f32 saved;
    s32 mode;

    if (*(f32 *)(*(void **)((char *)w + 0x4)) == 0.0f) {
        return;
    }
    camParam = (f32 *)(g_sceneActorMobys + 0x674 + 0xB0);
    saved = *camParam;
    *camParam = 0.62f; /* 0x3F1EB852 */
    BuildCameraProjection();

    mode = *(s32 *)((char *)w + 0x1C8);
    if (mode == 0) {
        func_00339B00(w);
    } else if (mode == 2) {
        func_00339F90();
    }

    *camParam = saved;
    BuildCameraProjection();
}
#endif

/* func_0033A048: init the screen's eight embedded sub-elements at their fixed
 * offsets (four type-B then four type-C), return the object. */
#if defined(S136OS_func_0033A048) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033A048; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 26/26 words + 8 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033A048 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033A048(void *p) {
    GuiElementInitTypeB((char *)p + 0x8);
    GuiElementInitTypeB((char *)p + 0x54);
    GuiElementInitTypeB((char *)p + 0xA0);
    GuiElementInitTypeB((char *)p + 0xEC);
    GuiElementInitTypeC((char *)p + 0x138);
    GuiElementInitTypeC((char *)p + 0x190);
    GuiElementInitTypeC((char *)p + 0x1E8);
    GuiElementInitTypeC((char *)p + 0x240);
    return p;
}
#else
S136OS_SLOT(func_0033A048);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A0B0);

/* GuiConfirmPopupInit: construct the confirm popup (4 button-icon elements + 4
 * text rows). Pool -> alloc the 0x10-byte placement record (+0x4) and zero it
 * (the +0x0 pool store + the +0x2D8=1 "active" flag are unconditional). Record
 * seeded 256x198. Init four icon elements (+0x8/+0x54/+0xA0/+0xEC from
 * D_1ADBE8/BF0/BF8/C00) and four text elements (+0x138/+0x190/+0x1E8/+0x240 from
 * D_1ADC08/C10/C18/C28); mark +0x1E8 as a text element. Colour them, assign
 * glyphs 0x6B..0x6E from the atlas (g_guiInstance+0x8710), set the three visible
 * text rows (localized 0x2C5B / 0x2BE5, plus the in-place buffer at +0x298), then
 * run one GuiConfirmPopupTick(w, 0) to lay it out. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiConfirmPopupInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiConfirmPopupInit, unit objdiff): 49.55%,
   143/207 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu v0, zero, 0x1' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void GuiConfirmPopupTick(void *w, s32 arg2);
extern char *g_guiInstance;
extern u8 D_1ADBE8[], D_1ADBF0[], D_1ADBF8[], D_1ADC00[];
extern u8 D_1ADC08[], D_1ADC10[], D_1ADC18[], D_1ADC28[];
void GuiConfirmPopupInit(void *w, GuiPool *pool) {
    GuiElement *icon0 = (GuiElement *)((char *)w + 0x8);
    GuiElement *icon1 = (GuiElement *)((char *)w + 0x54);
    GuiElement *icon2 = (GuiElement *)((char *)w + 0xA0);
    GuiElement *icon3 = (GuiElement *)((char *)w + 0xEC);
    GuiElement *text0 = (GuiElement *)((char *)w + 0x138);
    GuiElement *text1 = (GuiElement *)((char *)w + 0x190);
    GuiElement *text2 = (GuiElement *)((char *)w + 0x1E8);
    GuiElement *text3 = (GuiElement *)((char *)w + 0x240);
    void *rec;

    *(GuiPool **)((char *)w + 0x0) = pool;
    *(s32 *)((char *)w + 0x2D8) = 1; /* unconditional (beqz delay slot) */
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x4) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    rec = *(void **)((char *)w + 0x4);
    *(f32 *)((char *)rec + 0x0) = 256.0f;
    *(f32 *)((char *)rec + 0x4) = 198.0f;

    GuiElementInit(icon0, (s32)D_1ADBE8, pool);
    GuiElementInit(icon1, (s32)D_1ADBF0, pool);
    GuiElementInit(icon2, (s32)D_1ADBF8, pool);
    GuiElementInit(icon3, (s32)D_1ADC00, pool);
    GuiTextElementInit(text0, (s32)D_1ADC08, pool);
    GuiTextElementInit(text1, (s32)D_1ADC10, pool);
    GuiTextElementInit(text2, (s32)D_1ADC18, pool);
    GuiTextElementInit(text3, (s32)D_1ADC28, pool);
    GuiElementSetTextFlag(text2, 1);

    *GuiElementGetColor(icon0) = 0x60442D00;
    *GuiElementGetColor(icon1) = 0x60241700;
    *GuiElementGetColor(icon2) = 0x55F0C070;
    *GuiElementGetColor(icon3) = 0x55F0C070;
    *GuiElementGetColor(text0) = (s32)0x80F0F0F0;
    *GuiElementGetColor(text1) = (s32)0x80F0F0F0;
    *GuiElementGetColor(text2) = (s32)0x80F0F0F0;

    GuiElementSetGlyph(icon0, (s32)(g_guiInstance + 0x8710), 0x6B);
    GuiElementSetGlyph(icon1, (s32)(g_guiInstance + 0x8710), 0x6C);
    GuiElementSetGlyph(icon2, (s32)(g_guiInstance + 0x8710), 0x6D);
    GuiElementSetGlyph(icon3, (s32)(g_guiInstance + 0x8710), 0x6E);

    GuiElementSetText(text0, GetLocalizedString(0x2C5B));
    GuiElementSetText(text1, GetLocalizedString(0x2BE5));
    GuiElementSetText(text2, (s32)((char *)w + 0x298));

    GuiConfirmPopupTick(w, 0);
}
#endif

/* GuiConfirmPopupTick: lay out the confirm popup's seven visible sub-elements
 * relative to the placement record at *(w+0x4), then bump the current cutscene
 * unlock record. The four icon rows (+0x8/+0x54/+0xA0/+0xEC) sit at the record
 * origin; the three text rows (+0x138/+0x190/+0x1E8) are offset by the fixed
 * (x,y) pairs D_1ADC38/D_1ADC40/D_1ADC48. func_002E0010 refreshes the in-place
 * level-name buffer at +0x298 for g_mapCurrentLevel. The tail touches the same
 * per-cutscene unlock block at (g_health+0x464) as MenuCutsceneUnlockCurrent
 * (1CA080), but a distinct slot (+0x1C8/+0x1CC/+0x1D0): while g_gameTime>=11 it
 * increments the saturating u16 play-count at +0x1C8, always raises the u32
 * high-water mark at +0x1CC toward g_gsPixelOffsetY+0x3C, and sets this
 * g_playerProgress slot's bit (plus the 0x80000000 sentinel) in the seen-mask
 * at +0x1D0. arg2 is unused (the ABI-uniform second tick parameter). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiConfirmPopupTick);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiConfirmPopupTick, unit objdiff): 26.00%,
   177/193 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x90' vs 'addiu sp, sp, -0x60').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_002E0010(void *dst, s32 level);
extern f32 D_1ADC38[2], D_1ADC40[2], D_1ADC48[2];
extern s32 g_health;            /* 0x18C2EC - base of the per-cutscene unlock records at +0x464 */
extern s32 g_gameTime;          /* 0x1B1608 - global frame counter */
extern s32 g_mapCurrentLevel;   /* 0x1C5150 - current map level id */
extern s32 g_gsPixelOffsetY;    /* 0x1A7354 - play-count source at +0x3C */
extern s32 g_playerProgress;    /* 0x1A79F8 - current progress slot (seen-mask bit index) */
void GuiConfirmPopupTick(void *w, s32 arg2) {
    GuiElement *icon0 = (GuiElement *)((char *)w + 0x8);
    GuiElement *icon1 = (GuiElement *)((char *)w + 0x54);
    GuiElement *icon2 = (GuiElement *)((char *)w + 0xA0);
    GuiElement *icon3 = (GuiElement *)((char *)w + 0xEC);
    GuiElement *text0 = (GuiElement *)((char *)w + 0x138);
    GuiElement *text1 = (GuiElement *)((char *)w + 0x190);
    GuiElement *text2 = (GuiElement *)((char *)w + 0x1E8);
    f32 *origin = *(f32 **)((char *)w + 0x4);
    u8 *rec = (u8 *)&g_health + 0x464;
    s32 target;

    (void)arg2;

    GuiElementSetPos(icon0, origin[0], origin[1], 0.0f, 0.0f);
    GuiElementSetPos(icon1, origin[0], origin[1], 0.0f, 0.0f);
    GuiElementSetPos(icon2, origin[0], origin[1], 0.0f, 0.0f);
    GuiElementSetPos(icon3, origin[0], origin[1], 0.0f, 0.0f);
    GuiElementSetPos(text0, D_1ADC38[0] + origin[0], D_1ADC38[1] + origin[1], 0.0f, 0.0f);
    GuiElementSetPos(text1, D_1ADC40[0] + origin[0], D_1ADC40[1] + origin[1], 0.0f, 0.0f);
    GuiElementSetPos(text2, D_1ADC48[0] + origin[0], D_1ADC48[1] + origin[1], 0.0f, 0.0f);

    func_002E0010((char *)w + 0x298, g_mapCurrentLevel);

    if (g_gameTime >= 11 && *(u16 *)(rec + 0x1C8) <= 0xFFFE) {
        *(u16 *)(rec + 0x1C8) = (u16)(*(u16 *)(rec + 0x1C8) + 1);
    }
    target = *(s32 *)((u8 *)&g_gsPixelOffsetY + 0x3C);
    if ((u32)*(s32 *)(rec + 0x1CC) < (u32)target) {
        *(s32 *)(rec + 0x1CC) = target;
    }
    *(u32 *)(rec + 0x1D0) |= (1u << g_playerProgress) | 0x80000000u;
}
#endif

/* Draw a confirm popup when it's active (+0x2D8 != 0): four sprite sub-elements
 * (+0x8/+0x54/+0xA0/+0xEC) and three text sub-elements (+0x138/+0x190/+0x1E8). */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiConfirmPopupDraw)
S136OS_SLOT(GuiConfirmPopupDraw);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiConfirmPopupDraw, unit objdiff): 87.20%,
   4/26 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd ra, 0x8(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
void GuiConfirmPopupDraw(void *w) {
    if (*(s32 *)((char *)w + 0x2D8) != 0) {
        GuiSpriteElementDraw((char *)w + 0x8);
        GuiSpriteElementDraw((char *)w + 0x54);
        GuiSpriteElementDraw((char *)w + 0xA0);
        GuiSpriteElementDraw((char *)w + 0xEC);
        GuiTextElementDraw((char *)w + 0x138);
        GuiTextElementDraw((char *)w + 0x190);
        GuiTextElementDraw((char *)w + 0x1E8);
    }
}
#endif

/* func_0033A640: init the embedded dialog-box (p+0x8) and a type-B element
 * (p+0x2D8), return the object. */
#if defined(S136OS_func_0033A640) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033A640; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 13/13 words + 2 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033A640 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033A640(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    GuiElementInitTypeB((char *)p + 0x2D8);
    return p;
}
#else
S136OS_SLOT(func_0033A640);
#endif

/* func_0033A678: construct a bordered dialog/popup widget. If a pool is given,
 * allocate its 0x10-byte placement record (+0x324) and zero it. Seed the panel
 * element (+0x8) size to 256x212, init its border art from D_1ADC50, then set
 * its bounds (64, -164, 10, 136, 10, 99). Init the icon GuiElement (+0x2D8) from
 * D_1ADC60, force its packed colour to 0x70FFFEED, and give it glyph 0xD6 from
 * the shared atlas (g_guiInstance+0x8710). Finally run func_0033A7E0(w) (the
 * show hook) and clear +0x330. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A678);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033A678, unit objdiff): 68.55%,
   43/84 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s2, 0x10(sp)' vs 'sd s1, 0x8(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* GuiElementInit / GuiElementGetColor / GuiElementSetGlyph are defined earlier
 * in this unit; only the not-yet-defined callees need forward decls here. */
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
void GuiDialogBoxSetBounds(void *p, f32 a, f32 b, f32 c, f32 d, f32 e, f32 f);
extern void func_0033A7E0(void *w);
extern char *g_guiInstance;
extern u8 D_1ADC50[8];
extern u8 D_1ADC60[];
void func_0033A678(void *w, GuiPool *pool) {
    GuiElement *icon = (GuiElement *)((char *)w + 0x2D8);
    void *obj;
    s32 *color;

    /* +0x0 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x324) = obj;
        *(s32 *)((char *)obj + 0x0) = 0;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
    }

    obj = *(void **)((char *)w + 0x324);
    *(f32 *)((char *)obj + 0x0) = 256.0f;
    *(f32 *)((char *)obj + 0x4) = 212.0f;

    *(s32 *)((char *)w + 0x328) = 0;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADC50);
    GuiDialogBoxSetBounds((char *)w + 0x8, 64.0f, -164.0f, 10.0f, 136.0f, 10.0f, 99.0f);

    /* the "tag"/"codepoint" params are s32 by declaration but carry data
     * addresses here (the original passes the pointer through an int slot). */
    GuiElementInit(icon, (s32)D_1ADC60, pool);
    color = GuiElementGetColor(icon);
    *color = 0x70FFFEED;
    GuiElementSetGlyph(icon, (s32)(g_guiInstance + 0x8710), 0xD6);

    func_0033A7E0(w);
    *(s32 *)((char *)w + 0x330) = 0;
}
#endif

/* func_0033A7A8: set the +0x330 flag to 1. */
void func_0033A7A8(void *p) {
    *(s32 *)((char *)p + 0x330) = 1;
}

/* func_0033A7B8: forward p+0x8 to func_0033BE60 (the wrapper sets only $a0; the
 * callee's second slot is left untouched). */
void func_0033A7B8(void *p) {
    ((void (*)(void *))func_0033BE60)((char *)p + 0x8);
    __asm__ __volatile__("");
}

/* func_0033A7D8: store an int at +0x328. */
void func_0033A7D8(void *p, s32 v) {
    *(s32 *)((char *)p + 0x328) = v;
}

/* func_0033A7E0: seed this confirm-dialog screen's text - set the dialog box
 * (p+0x8) title/body/footer to localized strings 0x2C34/0x2C32/0x2BE5, zero its
 * scale, and clear the +0x32C "text ready" flag (the sibling of func_0033A860,
 * which raises that flag with the 0x2C33 title variant). */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033A7E0)
S136OS_SLOT(func_0033A7E0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033A7E0, unit objdiff): 73.28%,
   16/36 insns differ. Residual: UNKNOWN-addiu (first differing insn: '' vs 'addiu a0, zero, 0x2c34').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* WALL on cc1 2.9/2.96 only (s136os is byte-exact, GUARD above): 4-callee-save frame
   wall. */
extern s32 GetLocalizedString(s32 id);
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
extern void GuiDialogBoxSetScale(void *p, f32 scale);
void func_0033A7E0(void *p) {
    s32 t0 = GetLocalizedString(0x2C34);
    s32 t1 = GetLocalizedString(0x2C32);
    s32 t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)p + 0x8, t0, t1, t2);
    GuiDialogBoxSetScale((char *)p + 0x8, 0.0f);
    *(s32 *)((char *)p + 0x32C) = 0;
}
#endif

/* func_0033A860: populate this confirm-dialog screen's text - set the dialog box
 * (p+0x8) title/body/footer to localized strings 0x2C33/0x2C32/0x2BE5, zero its
 * scale, raise the +0x32C "text ready" flag, and return 1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A860);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033A860, unit objdiff): 67.30%,
   22/38 insns differ. Residual: UNKNOWN-addiu (first differing insn: '' vs 'addiu a0, zero, 0x2c33').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; 4-callee-save frame
   wall. */
extern s32 GetLocalizedString(s32 id);
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
extern void GuiDialogBoxSetScale(void *p, f32 scale);
s32 func_0033A860(void *p) {
    s32 t0 = GetLocalizedString(0x2C33);
    s32 t1 = GetLocalizedString(0x2C32);
    s32 t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)p + 0x8, t0, t1, t2);
    GuiDialogBoxSetScale((char *)p + 0x8, 0.0f);
    *(s32 *)((char *)p + 0x32C) = 1;
    return 1;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A8E8);

/* Update a pulsing list-cursor widget: place its frame sub-element (+0x8) at the
 * tracked anchor (*(w+0x324)) via func_0033C060 + func_0033BE70, tick the colour
 * pulse when dpad up/down is held, set the cursor sprite's (+0x2D8) colour to the
 * pulsed blend of 0x60442D00/0x70FFFEED, then position that sprite at the anchor
 * offset by fixed constants plus a per-frame-counter (w+0x328) Y step. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A8F0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033A8F0, unit objdiff): 80.11%,
   33/71 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s1, 0x8(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
s32 func_0033A8F0(void *w) {
    void *sub = (char *)w + 0x8;
    f32 *anchor = *(f32 **)((char *)w + 0x324);
    s32 counter;
    f32 *a;

    func_0033C060(sub, anchor[0], anchor[1]);
    func_0033BE70(sub, 0);   /* 2nd arg (flags) ignored by the callee */
    if (g_padButtonsPressed & 0x5000) {
        func_002AA3F0(0, 0, 1, 0, 1);
    }
    *GuiElementGetColor((GuiElement *)((char *)w + 0x2D8)) =
        func_002AA3F0(0x60442D00, 0x70FFFEED, 0x14, 0, 0);

    counter = *(s32 *)((char *)w + 0x328);
    a = *(f32 **)((char *)w + 0x324);
    GuiElementSetPos((GuiElement *)((char *)w + 0x2D8),
                     a[0] + (f32)D_1ADC68,
                     a[1] + (f32)(counter * D_1ADC70) + (f32)D_1ADC6C,
                     0.0f, 0.0f);
    return 0;
}
#endif

/* func_0033A9F8: draw this confirm-dialog screen. Sets the dialog box's (p+0x8)
 * +0x2C4 footer-visible flag to 1 when any global menu lock (D_1A8C88 / D_1A8C8C)
 * is engaged or the local +0x330 flag is set; additionally sets it to 1 (with
 * arg 1) when the +0x32C "text ready" flag equals 1. Then draws the box
 * (func_0033BF90) and the background sprite at p+0x2D8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033A9F8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033A9F8, unit objdiff): 85.82%,
   15/36 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s1, 0x8(sp)' vs 'sd s0, 0x0(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; branch-likely (beql)
   guard + reloaded-flag CSE wall. */
extern s32 D_1A8C88, D_1A8C8C;
extern void func_0033BE68(void *p, s32 v);
void func_0033A9F8(void *p) {
    void *box = (char *)p + 0x8;
    if (D_1A8C88 != 0 || D_1A8C8C != 0 || *(s32 *)((char *)p + 0x330) != 0) {
        func_0033BE68(box, 0);
    }
    if (*(s32 *)((char *)p + 0x32C) == 1) {
        func_0033BE68(box, 1);
    }
    func_0033BF90(box);
    GuiSpriteElementDraw((char *)p + 0x2D8);
}
#endif

/* func_0033AA80: init the level-info panel's thirteen embedded sub-elements at
 * their fixed offsets (four type-B at p+0x8/+0x54/+0xA0/+0xEC, then nine type-C
 * at +0x138/+0x190/+0x1E8/+0x240/+0x298/+0x2F0/+0x348/+0x3A0/+0x3F8), return the
 * object. */
#if defined(S136OS_func_0033AA80) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033AA80; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 36/36 words + 13 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033AA80 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033AA80(void *p) {
    GuiElementInitTypeB((char *)p + 0x8);
    GuiElementInitTypeB((char *)p + 0x54);
    GuiElementInitTypeB((char *)p + 0xA0);
    GuiElementInitTypeB((char *)p + 0xEC);
    GuiElementInitTypeC((char *)p + 0x138);
    GuiElementInitTypeC((char *)p + 0x190);
    GuiElementInitTypeC((char *)p + 0x1E8);
    GuiElementInitTypeC((char *)p + 0x240);
    GuiElementInitTypeC((char *)p + 0x298);
    GuiElementInitTypeC((char *)p + 0x2F0);
    GuiElementInitTypeC((char *)p + 0x348);
    GuiElementInitTypeC((char *)p + 0x3A0);
    GuiElementInitTypeC((char *)p + 0x3F8);
    return p;
}
#else
S136OS_SLOT(func_0033AA80);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033AB10);

/* GuiLevelInfoPanelInit(self, pool): construct the galactic-map level-info panel.
 * Stores the pool at +0x0, sets the +0x490 "active" flag, and (if a pool is given)
 * allocates one 16-byte size vector at +0x4 seeded to {256.0, 198.0}. Builds four
 * header glyph elements (+0x8/+0x54/+0xA0/+0xEC; tags D_1ADBE8/F0/F8/D_1ADC00;
 * glyphs 0x5F-0x62; colours 0x60442D00/0x60241700/0x55F0C070/0x55F0C070) and nine
 * text rows (+0x138..+0x3F8; tags D_1ADC08/80/90/98/A0/A8/10/18/28; all white
 * 0x80F0F0F0). Rows 2-9 get text-flag 0 except the last (+0x3F8) which gets flag 2.
 * Seven rows are seeded with localized strings (0x2BFB/0x2BF3/0x3129/0x2BFF/0x2C00/
 * 0x2C01/0x2BE5); row +0x3A0 points at the in-struct buffer +0x450. Sets the byte
 * flag D_1ADC78 = 0x12 and tail-calls GuiLevelInfoPanelTick. Faithful TARGET_NATIVE
 * #else (engine 2.96 = no byte-match); no pos/scale calls (no element-last risk). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiLevelInfoPanelInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiLevelInfoPanelInit, unit objdiff): 46.36%,
   237/336 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu v0, zero, 0x1' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern char *g_guiInstance;
extern u8 D_1ADBE8[], D_1ADBF0[], D_1ADBF8[], D_1ADC00[], D_1ADC08[], D_1ADC80[],
          D_1ADC90[], D_1ADC98[], D_1ADCA0[], D_1ADCA8[], D_1ADC10[], D_1ADC18[],
          D_1ADC28[], D_1ADC78[8];
s32 GuiLevelInfoPanelTick(void *w);
void GuiLevelInfoPanelInit(void *self, GuiPool *pool) {
    u8 *s = (u8 *)self;
    GuiElement *h0 = (GuiElement *)(s + 0x8);
    GuiElement *h1 = (GuiElement *)(s + 0x54);
    GuiElement *h2 = (GuiElement *)(s + 0xA0);
    GuiElement *h3 = (GuiElement *)(s + 0xEC);
    GuiElement *r0 = (GuiElement *)(s + 0x138);
    GuiElement *r1 = (GuiElement *)(s + 0x190);
    GuiElement *r2 = (GuiElement *)(s + 0x1E8);
    GuiElement *r3 = (GuiElement *)(s + 0x240);
    GuiElement *r4 = (GuiElement *)(s + 0x298);
    GuiElement *r5 = (GuiElement *)(s + 0x2F0);
    GuiElement *r6 = (GuiElement *)(s + 0x348);
    GuiElement *r7 = (GuiElement *)(s + 0x3A0);
    GuiElement *r8 = (GuiElement *)(s + 0x3F8);
    u32 *vec;

    *(GuiPool **)(s + 0x0) = pool;
    *(s32 *)(s + 0x490) = 1;
    if (pool != 0) {
        vec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(u32 **)(s + 0x4) = vec;
        vec[0] = 0; vec[1] = 0; vec[2] = 0; vec[3] = 0;
    }
    vec = *(u32 **)(s + 0x4);
    *(f32 *)&vec[0] = 256.0f;
    *(f32 *)&vec[1] = 198.0f;

    GuiElementInit(h0, (s32)D_1ADBE8, pool);
    GuiElementInit(h1, (s32)D_1ADBF0, pool);
    GuiElementInit(h2, (s32)D_1ADBF8, pool);
    GuiElementInit(h3, (s32)D_1ADC00, pool);

    GuiTextElementInit(r0, (s32)D_1ADC08, pool);
    GuiTextElementInit(r1, (s32)D_1ADC80, pool);
    GuiTextElementInit(r2, (s32)D_1ADC90, pool);
    GuiTextElementInit(r3, (s32)D_1ADC98, pool);
    GuiTextElementInit(r4, (s32)D_1ADCA0, pool);
    GuiTextElementInit(r5, (s32)D_1ADCA8, pool);
    GuiTextElementInit(r6, (s32)D_1ADC10, pool);
    GuiTextElementInit(r7, (s32)D_1ADC18, pool);
    GuiTextElementInit(r8, (s32)D_1ADC28, pool);

    GuiElementSetTextFlag(r1, 0);
    GuiElementSetTextFlag(r2, 0);
    GuiElementSetTextFlag(r3, 0);
    GuiElementSetTextFlag(r4, 0);
    GuiElementSetTextFlag(r5, 0);
    GuiElementSetTextFlag(r6, 0);
    GuiElementSetTextFlag(r7, 0);
    GuiElementSetTextFlag(r8, 2);

    *GuiElementGetColor(h0) = 0x60442D00;
    *GuiElementGetColor(h1) = 0x60241700;
    *GuiElementGetColor(h2) = 0x55F0C070;
    *GuiElementGetColor(h3) = 0x55F0C070;
    *GuiElementGetColor(r0) = (s32)0x80F0F0F0;
    *GuiElementGetColor(r1) = (s32)0x80F0F0F0;
    *GuiElementGetColor(r2) = (s32)0x80F0F0F0;
    *GuiElementGetColor(r3) = (s32)0x80F0F0F0;
    *GuiElementGetColor(r4) = (s32)0x80F0F0F0;
    *GuiElementGetColor(r5) = (s32)0x80F0F0F0;
    *GuiElementGetColor(r6) = (s32)0x80F0F0F0;
    *GuiElementGetColor(r7) = (s32)0x80F0F0F0;
    *GuiElementGetColor(r8) = (s32)0x80F0F0F0;

    GuiElementSetGlyph(h0, (s32)g_guiInstance + 0x8710, 0x5F);
    GuiElementSetGlyph(h1, (s32)g_guiInstance + 0x8710, 0x60);
    GuiElementSetGlyph(h2, (s32)g_guiInstance + 0x8710, 0x61);
    GuiElementSetGlyph(h3, (s32)g_guiInstance + 0x8710, 0x62);

    D_1ADC78[0] = 0x12;
    GuiElementSetText(r0, GetLocalizedString(0x2BFB));
    GuiElementSetText(r1, GetLocalizedString(0x2BF3));
    GuiElementSetText(r2, GetLocalizedString(0x3129));
    GuiElementSetText(r3, GetLocalizedString(0x2BFF));
    GuiElementSetText(r4, GetLocalizedString(0x2C00));
    GuiElementSetText(r5, GetLocalizedString(0x2C01));
    GuiElementSetText(r6, GetLocalizedString(0x2BE5));
    GuiElementSetText(r7, (s32)(s + 0x450));

    GuiLevelInfoPanelTick(self);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033AF68);

/* GuiLevelInfoPanelTick: per-frame layout for the galactic-map level-info panel.
 * Scales the six "value row" elements (0x190..0x348) uniformly to
 * (D_1ADCD8, D_1ADCDC), then positions all thirteen sub-elements relative to the
 * panel origin (the vector at w[0x4]): the four header/icon rows (0x8/0x54/0xA0/
 * 0xEC) sit on the origin, the label row (0x138) and the value column share the
 * layout offsets D_1ADCB8/D_1ADCC0/D_1ADCC8/D_1ADCD0, with the six value rows
 * (0x190..0x348) stepped 18px apart down the column (0/18/36/54/72/90 off the
 * D_1ADCC0 base, plus +8px on the last row's x). Refreshes the map thumbnail
 * (func_002E0010 at 0x450 for the current level), and sets the panel caption
 * (0x3F8) from g_levelSelectEntries[currentLevel].valueStrId — localized, or the
 * D_1ADC60 placeholder glyph when that id is negative. Returns 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiLevelInfoPanelTick);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiLevelInfoPanelTick, unit objdiff): 48.12%,
   271/340 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0xe0' vs 'addiu sp, sp, -0x70').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern f32 D_1ADCB8[2], D_1ADCC0[2], D_1ADCC8[2], D_1ADCD0[2];
extern f32 D_1ADCD8, D_1ADCDC;
extern u8 g_mapVertexData[], g_levelSelectEntries[], D_1ADC60[];
s32 GuiLevelInfoPanelTick(void *w) {
    f32 *origin = *(f32 **)((char *)w + 0x4);
    s32 level = *(s32 *)(g_mapVertexData + 0x230);
    s32 valueStrId;

    GuiElementSetScale((GuiElement *)((char *)w + 0x190), D_1ADCD8, D_1ADCDC, 0.0f, 0.0f);
    GuiElementSetScale((GuiElement *)((char *)w + 0x1E8), D_1ADCD8, D_1ADCDC, 0.0f, 0.0f);
    GuiElementSetScale((GuiElement *)((char *)w + 0x240), D_1ADCD8, D_1ADCDC, 0.0f, 0.0f);
    GuiElementSetScale((GuiElement *)((char *)w + 0x298), D_1ADCD8, D_1ADCDC, 0.0f, 0.0f);
    GuiElementSetScale((GuiElement *)((char *)w + 0x2F0), D_1ADCD8, D_1ADCDC, 0.0f, 0.0f);
    GuiElementSetScale((GuiElement *)((char *)w + 0x348), D_1ADCD8, D_1ADCDC, 0.0f, 0.0f);

    GuiElementSetPos((GuiElement *)((char *)w + 0x8),   origin[0], origin[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x54),  origin[0], origin[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0xA0),  origin[0], origin[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0xEC),  origin[0], origin[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x138), D_1ADCB8[0] + origin[0], D_1ADCB8[1] + origin[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x190), D_1ADCC0[0] + origin[0], D_1ADCC0[1] + origin[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x1E8), D_1ADCC0[0] + origin[0], D_1ADCC0[1] + origin[1] + 18.0f, 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x240), D_1ADCC0[0] + origin[0], D_1ADCC0[1] + origin[1] + 36.0f, 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x298), D_1ADCC0[0] + origin[0], D_1ADCC0[1] + origin[1] + 54.0f, 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x2F0), D_1ADCC0[0] + origin[0], D_1ADCC0[1] + origin[1] + 72.0f, 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x348), D_1ADCC0[0] + 8.0f + origin[0], D_1ADCC0[1] + origin[1] + 90.0f, 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x3A0), D_1ADCC8[0] + origin[0], D_1ADCC8[1] + origin[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x3F8), D_1ADCD0[0] + origin[0], D_1ADCD0[1] + origin[1], 0.0f, 0.0f);

    func_002E0010((char *)w + 0x450, level);

    valueStrId = ((s32 *)g_levelSelectEntries)[*(s32 *)(g_mapVertexData + 0x230) * 2 + 1];
    if (valueStrId < 0) {
        GuiElementSetText((GuiElement *)((char *)w + 0x3F8), (s32)D_1ADC60);
    } else {
        GuiElementSetText((GuiElement *)((char *)w + 0x3F8), GetLocalizedString(valueStrId));
    }
    return 0;
}
#endif

/* func_0033B428: draw the map-screen panel - only when the +0x490 "panel built"
 * flag is set. Draws the two background sprites (p+0x8, p+0x54), pushes two GS
 * register packets (TEST/0x42=0x44 then SCISSOR/0x47=0xB), renders the map layer
 * (MapDraw(0,1) + func_002DBC98(0)), draws the two foreground sprites (p+0xA0,
 * p+0xEC) and the nine text labels at +0x138 (stride 0x58). */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033B428)
S136OS_SLOT(func_0033B428);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033B428, unit objdiff): 85.00%,
   8/51 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* WALL on cc1 2.9/2.96 only (s136os is byte-exact, GUARD above): 2-callee-save frame +
   branch-likely (beql) guard wall. */
/* GS A+D reg-write: the DATA is a 64-bit register value. Widen it to u64 for the
 * native/#else build (prevents silent truncation of bits >=32); matching-build
 * decl kept verbatim (byte-neutral). */
#ifdef TARGET_NATIVE
extern void AppendGsRegPacket(s32 reg, u64 val);
#else
extern void AppendGsRegPacket(s32 reg, s32 val);
#endif
extern void MapDraw(s32 a, s32 b);
extern void func_002DBC98(s32 a);
extern void GuiTextElementDraw(void *e);
void func_0033B428(void *p) {
    if (*(s32 *)((char *)p + 0x490) != 0) {
        GuiSpriteElementDraw((char *)p + 0x8);
        GuiSpriteElementDraw((char *)p + 0x54);
        AppendGsRegPacket(0x42, 0x44);
        AppendGsRegPacket(0x47, 0xB);
        MapDraw(0, 1);
        func_002DBC98(0);
        GuiSpriteElementDraw((char *)p + 0xA0);
        GuiSpriteElementDraw((char *)p + 0xEC);
        GuiTextElementDraw((char *)p + 0x138);
        GuiTextElementDraw((char *)p + 0x190);
        GuiTextElementDraw((char *)p + 0x1E8);
        GuiTextElementDraw((char *)p + 0x240);
        GuiTextElementDraw((char *)p + 0x298);
        GuiTextElementDraw((char *)p + 0x2F0);
        GuiTextElementDraw((char *)p + 0x348);
        GuiTextElementDraw((char *)p + 0x3A0);
        GuiTextElementDraw((char *)p + 0x3F8);
    }
}
#endif

/* func_0033B4E8: init the map-screen panel's six embedded sub-elements - four
 * type-B elements (p+0x4, stride 0x4C), the D_1ADA18 widget at +0x134, and one
 * type-C element at +0x170; return the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B4E8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033B4E8, unit objdiff): 58.34%,
   26/30 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x30' vs 'addiu sp, sp, -0x10').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; 4-callee-save frame
   wall. */
void *func_0033B4E8(void *p) {
    GuiElementInitTypeB((char *)p + 0x4);
    GuiElementInitTypeB((char *)p + 0x50);
    GuiElementInitTypeB((char *)p + 0x9C);
    GuiElementInitTypeB((char *)p + 0xE8);
    func_003374D8((char *)p + 0x134);
    GuiElementInitTypeC((char *)p + 0x170);
    return p;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B560);

/* GuiProgressBarWidgetInit: construct a progress-bar widget. Clears +0x208 and
 * records the pool at +0x210 (unconditional). If a pool is given, allocate its
 * 0x10-byte placement record (+0x20C) and zero it. The bar's fill float (rec+4)
 * is derived from a config float at g_swapGadgetItemIndex+0x8A: fill =
 * (s32)(cfg*300 + 0.5) rounded, with rec+0 fixed at 70. Init the sprite element
 * (+0x134) from D_1ADC60, scale it 24x24, colour 0x60F0F0B0, and set +0x214=65.
 * Init the text element (+0x170) from D_1ADC60, colour 0x80F0F0F0, scale 0.7. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiProgressBarWidgetInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiProgressBarWidgetInit, unit objdiff): 51.24%,
   77/110 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s3, 0x18(sp)' vs 'sd s1, 0x8(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern s32 g_swapGadgetItemIndex;
extern u8 D_1ADC60[];
void GuiProgressBarWidgetInit(void *w, GuiPool *pool) {
    GuiElement *sprite = (GuiElement *)((char *)w + 0x134);
    GuiElement *text = (GuiElement *)((char *)w + 0x170);
    void *rec;
    s32 *color;
    f32 cfg;

    *(s32 *)((char *)w + 0x208) = 0;
    /* +0x210 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x210) = pool;
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x20C) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    cfg = *(f32 *)((char *)&g_swapGadgetItemIndex + 0x8A);
    rec = *(void **)((char *)w + 0x20C);
    *(f32 *)((char *)rec + 0x0) = 70.0f;
    *(f32 *)((char *)rec + 0x4) = (f32)(s32)(cfg * 300.0f + 0.5f);

    GuiSpriteElementInit(sprite, (s32)D_1ADC60, pool);
    GuiElementSetScale(sprite, 24.0f, 24.0f, 0.0f, 0.0f);
    color = GuiElementGetColor(sprite);
    *color = 0x60F0F0B0;
    *(f32 *)((char *)w + 0x214) = 65.0f;

    GuiTextElementInit(text, (s32)D_1ADC60, pool);
    color = GuiElementGetColor(text);
    *color = (s32)0x80F0F0F0;
    GuiElementSetScale(text, 0.7f, 0.7f, 0.0f, 0.0f);
}
#endif

extern f32 D_1ADCE0, D_1ADCE4;

/**
 * When the widget's +0x21C flag is set, position its embedded element at
 * p+0x170 relative to the anchor vector *(p+0x20C):
 * x = anchor[0] + D_1ADCE0, y = anchor[1] + D_1ADCE4, z = w = 0.
 *
 * p: owning widget. No return value.
 *
 * Matched byte-exact on sdk29 (cc1 2.9, -O2 -G8 -fno-gcse -fno-strict-aliasing,
 * task #851). The empty asm after GuiElementSetPos suppresses cc1's sibling
 * call so the ROM's jal + frame is reproduced; without it the arm is 77.89%
 * and one word short.
 */
void func_0033B6D0(void *p) {
    if (*(s32 *)((char *)p + 0x21C) != 0) {
        f32 *anchor = *(f32 **)((char *)p + 0x20C);
        GuiElementSetPos((GuiElement *)((char *)p + 0x170),
                         anchor[0] + D_1ADCE0, anchor[1] + D_1ADCE4,
                         0.0f, 0.0f);
    }
    __asm__ __volatile__("");
}

/* func_0033B720(w): render a radial/carousel item selector. No-op unless
 * w[0x87] is set. First lays out the two static frame elements (w+0x50, w+0x9C)
 * at the placement anchor (*(f32*)w[0x83]) with the shared frame scale
 * (D_1ADCF8/D_1ADCFC) and dispatches each one's draw method (obj+0xC, arg =
 * elem + obj[+8]). Then, for each of the w[0x82] carousel entries (entry table at
 * *w, stride 0x18), places item element w+0x134 on a circle of radius w[0x85]
 * (x += D_1ADCE8 + r*sin, y += D_1ADCEC + D_1ADCF0*r*cos, angle at entry+0xC),
 * sets its sprite (entry[0]/entry[1]) and colour (0x80808080 if state==1 else
 * entry+8), visibility (state!=2) and draws it. The currently-selected entry
 * (index == w[0x86]) additionally scales the label element (w+0x4) and, unless
 * hidden (state!=2), positions it (D_1ADD00/D_1ADD04 offsets), sets its text
 * (entry+0x14, or localized entry+0x10 if 0) and draws it; then dispatches the
 * label's draw method. Faithful TARGET_NATIVE #else (engine 2.96 = no
 * byte-match). GuiElementSetPos/SetScale element+arg order asm-verified. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033B720);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033B720, unit objdiff): 66.28%,
   153/210 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s2, 0x10(sp)' vs 'sd s0, 0x0(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern f32 func_00283B30(f32 x);   /* SinfVu0 */
extern f32 func_00283B48(f32 x);   /* CosfVu0 */
extern s32 D_1ADCE8, D_1ADCEC;     /* int, converted to float */
extern f32 D_1ADCF0, D_1ADCF8, D_1ADCFC, D_1ADD00, D_1ADD04;
void func_0033B720(void *w) {
    s32 *p = (s32 *)w;
    GuiElement *frame0 = (GuiElement *)((char *)p + 0x50);
    GuiElement *frame1 = (GuiElement *)((char *)p + 0x9C);
    f32 *anchor;
    s32 obj;

    if (p[0x87] == 0) {
        return;
    }

    anchor = *(f32 **)&p[0x83];
    GuiElementSetPos(frame0, anchor[0], anchor[1], 0.0f, 0.0f);
    GuiElementSetScale(frame0, D_1ADCF8, D_1ADCFC, 0.0f, 0.0f);
    obj = p[0x20];
    (*(void (**)(void *))(obj + 0xC))((char *)frame0 + *(s16 *)(obj + 8));

    anchor = *(f32 **)&p[0x83];
    GuiElementSetPos(frame1, anchor[0], anchor[1], 0.0f, 0.0f);
    GuiElementSetScale(frame1, D_1ADCF8, D_1ADCFC, 0.0f, 0.0f);
    obj = p[0x33];
    (*(void (**)(void *))(obj + 0xC))((char *)frame1 + *(s16 *)(obj + 8));

    if (p[0x82] > 0) {
        GuiElement *item = (GuiElement *)((char *)p + 0x134);
        s32 *state = &p[0x72];
        s32 index = 0;
        s32 off = 0;
        do {
            u8 *entry = (u8 *)(*p) + off;
            f32 r = *(f32 *)&p[0x85];
            f32 rx = r * func_00283B30(*(f32 *)(entry + 0xC));
            f32 ry = r * func_00283B48(*(f32 *)(entry + 0xC));
            s32 *color;

            GuiSpriteSetTexture(item, *(s32 *)entry, *(s32 *)(entry + 4));
            anchor = *(f32 **)&p[0x83];
            GuiElementSetPos(item, anchor[0] + (f32)D_1ADCE8 + rx,
                             anchor[1] + (f32)D_1ADCEC + D_1ADCF0 * ry, 0.0f, 0.0f);
            color = GuiElementGetColor(item);
            *color = (*state == 1) ? (s32)0x80808080 : *(s32 *)(entry + 8);
            GuiElementSetVisible(item, *state != 2);
            func_00337630(item);

            if (index == p[0x86]) {
                GuiElementSetScale((GuiElement *)((char *)p + 0x4), D_1ADCF8,
                                   D_1ADCFC, 0.0f, 0.0f);
                if (*state != 2) {
                    s32 str;
                    anchor = *(f32 **)&p[0x83];
                    GuiElementSetPos((GuiElement *)((char *)p + 0x4),
                                     anchor[0] + D_1ADD00 + rx,
                                     anchor[1] + D_1ADD04 + D_1ADCF0 * ry,
                                     0.0f, 0.0f);
                    str = *(s32 *)(entry + 0x14);
                    if (str == 0) {
                        str = GetLocalizedString(*(s16 *)(entry + 0x10));
                    }
                    GuiElementSetText((GuiElement *)((char *)p + 0x170), str);
                    GuiTextElementDraw((char *)p + 0x170);
                }
                obj = p[0xD];
                (*(void (**)(void *))(obj + 0xC))((char *)p + 0x4 + *(s16 *)(obj + 8));
            }

            index++;
            state++;
            off += 0x18;
        } while (index < p[0x82]);
    }
}
#endif

/* func_0033BA10: store a float at +0x214. */
void func_0033BA10(void *p, f32 v) {
    *(f32 *)((char *)p + 0x214) = v;
}

/* func_0033BA18: store v into the int array at p+0x1C8, element index idx. */
void func_0033BA18(void *p, s32 idx, s32 v) {
    s32 *row = (s32 *)((char *)p + idx * 4);
    row[0x72] = v;
}

/* func_0033BA28: clamp - if idx is below the count at p+0x208, store it at
 * p+0x218. */
void func_0033BA28(void *p, s32 idx) {
    if (idx < *(s32 *)((char *)p + 0x208)) {
        *(s32 *)((char *)p + 0x218) = idx;
    }
}

/* func_0033BA40: store an int at +0x21C. */
void func_0033BA40(void *p, s32 v) {
    *(s32 *)((char *)p + 0x21C) = v;
}

/* func_0033BA48: build a four-glyph button row + count its menu entries. For each
 * of the four slot elements (w+0x4, stride 0x4C): if the slot's glyph id
 * srcGlyphs[i] is non-zero, init the element (D_1ADC60, pool at +0x210), set its
 * colour from D_1ADD08[i], and assign the glyph from the shared atlas
 * (g_guiInstance+0x8710); otherwise assign glyph 0 and hide it. Then store the
 * entries pointer at +0x0, and count its valid entries (stride 0x18, terminator
 * field +4 == -1, capped at 0x10) into +0x208, zeroing the +0x1C8 row array as it
 * goes. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033BA48);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern char *g_guiInstance;
/* (end of this body's declarations) */
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033BA48, unit objdiff): 66.12%,
   60/98 insns differ. Residual: UNKNOWN-daddu + gp/abs-mixed symbol (first differing insn: '' vs 'daddu s2, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 D_1ADC60[];
extern s32 D_1ADD08[2];
void func_0033BA48(void *w, s32 *srcGlyphs, void *entries) {
    s32 i, idx, off;
    s32 *row;

    for (i = 0; i <= 3; i++) {
        GuiElement *elem = (GuiElement *)((char *)w + 0x4 + i * 0x4C);
        if (srcGlyphs[i] != 0) {
            GuiElementInit(elem, (s32)D_1ADC60, *(GuiPool **)((char *)w + 0x210));
            *GuiElementGetColor(elem) = D_1ADD08[i];
            GuiElementSetGlyph(elem, (s32)(g_guiInstance + 0x8710), srcGlyphs[i]);
        } else {
            GuiElementSetGlyph(elem, (s32)(g_guiInstance + 0x8710), 0);
            GuiElementSetVisible(elem, 0);
        }
    }

    *(s32 *)((char *)w + 0x208) = 0;
    *(void **)((char *)w + 0x0) = entries;

    if (*(s32 *)((char *)entries + 4) != -1) {
        idx = 0;
        off = 0;
        row = (s32 *)((char *)w + 0x1C8);
        *row = 0;
        for (;;) {
            off += 0x18;
            row++;
            *(s32 *)((char *)w + 0x208) += 1;
            if (*(s32 *)((char *)entries + off + 4) == -1) {
                break;
            }
            idx++;
            if (idx >= 0x10) {
                break;
            }
            *row = 0;
        }
    }
}
#endif

/* GuiDialogBoxInitElements: build the dialog-box sub-elements over p - 5 TypeB
 * border elements (p+0xC, stride 0x4C) then 3 TypeC text rows (p+0x198, +0x1F0,
 * +0x248). All field writes happen inside the TypeB/TypeC ctors. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxInitElements);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiDialogBoxInitElements, unit objdiff): 66.52%,
   23/33 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x30' vs 'addiu sp, sp, -0x20').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void GuiDialogBoxInitElements(void *p) {
    char *base = (char *)p;
    s32 i;
    for (i = 0; i < 5; i++) {
        GuiElementInitTypeB(base + 0xC + i * 0x4C);
    }
    GuiElementInitTypeC(base + 0x198);
    GuiElementInitTypeC(base + 0x1F0);
    GuiElementInitTypeC(base + 0x248);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033BC18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxInitBorder);

/* func_0033BE60: store an int at +0x2C0. */
void func_0033BE60(void *p, s32 v) {
    *(s32 *)((char *)p + 0x2C0) = v;
}

/* func_0033BE68: store an int at +0x2C4. */
void func_0033BE68(void *p, s32 v) {
    *(s32 *)((char *)p + 0x2C4) = v;
}

/* func_0033BE70: lay out the dialog box's five body rows then (when the +0x2C0
 * "show border labels" flag is set) its three border-relative text labels.
 *  - For each of the five rows i (element at p+0xC + i*0x4C, row-active flag is
 *    the halfword at p+0x188 + i*2): when active, anchor the row at the dialog's
 *    own vector (*(p+0x0)) origin and apply the shared alpha at *(p+0x2A4).
 *  - When *(p+0x2C0) is set, place the title (p+0x198), body (p+0x248) and
 *    footer (p+0x1F0) at the origin offset by the (x,y) bound pairs at
 *    +0x2A8/+0x2AC, +0x2B8/+0x2BC and +0x2B0/+0x2B4 respectively.
 * The flags second argument is unused by this method. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033BE70);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033BE70, unit objdiff): 73.47%,
   37/81 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x40' vs 'addiu sp, sp, -0x30').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; 4-callee-save + $f20
   saved-FPR frame wall. */
void func_0033BE70(void *p, s32 flags) {
    s32 i;
    f32 *origin;
    GuiElement *row = (GuiElement *)((char *)p + 0xC);
    s16 *active = (s16 *)((char *)p + 0x188);
    (void)flags;
    for (i = 0; i <= 4; i++) {
        if (active[i] != 0) {
            origin = *(f32 **)p;
            GuiElementSetPos(row, origin[0], origin[1], 0.0f, 0.0f);
            GuiElementSetAlpha(row, *(f32 *)((char *)p + 0x2A4));
        }
        row = (GuiElement *)((char *)row + 0x4C);
    }
    if (*(s32 *)((char *)p + 0x2C0) != 0) {
        origin = *(f32 **)p;
        GuiElementSetPos((GuiElement *)((char *)p + 0x198),
                         origin[0] + *(f32 *)((char *)p + 0x2A8),
                         origin[1] + *(f32 *)((char *)p + 0x2AC), 0.0f, 0.0f);
        origin = *(f32 **)p;
        GuiElementSetPos((GuiElement *)((char *)p + 0x248),
                         origin[0] + *(f32 *)((char *)p + 0x2B8),
                         origin[1] + *(f32 *)((char *)p + 0x2BC), 0.0f, 0.0f);
        origin = *(f32 **)p;
        GuiElementSetPos((GuiElement *)((char *)p + 0x1F0),
                         origin[0] + *(f32 *)((char *)p + 0x2B0),
                         origin[1] + *(f32 *)((char *)p + 0x2B4), 0.0f, 0.0f);
    }
}
#endif

/* func_0033BF90: draw the dialog box's five body rows then its three border
 * text labels.
 *  - For each of the five rows i: when the row-active halfword at p+0x188 + i*2
 *    is set, dispatch the row object at *(p+0x3C + i*0x4C) through its vtable -
 *    call (*obj->draw)(rowBase + obj->offset) where rowBase = p+0xC + i*0x4C,
 *    obj->offset is the halfword at obj+0x8 and obj->draw the fn-ptr at obj+0xC.
 *  - When *(p+0x2C0) is set draw the title (p+0x198); additionally when
 *    *(p+0x2C4) is set draw the body (p+0x248) - but only if *(p+0x2CC) is set -
 *    and the footer (p+0x1F0) whenever *(p+0x2C8) is set. */
/* GUARD (task #1338): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1338 lever): the row pointer stepped before the
 * object pointer at the loop join, and +0x2C8 tested directly rather than
 * through a cached local. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033BF90)
S136OS_SLOT(func_0033BF90);
#else
/* engine96 probe (task #466, cc1 2.96, unit objdiff): 89.90%; not the image arm (see GUARD). */
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact. */
extern void GuiTextElementDraw(void *e);
void func_0033BF90(void *p) {
    s32 i;
    s16 *active = (s16 *)((char *)p + 0x188);
    char *rowBase = (char *)p + 0xC;
    char *obj = (char *)p + 0x3C;
    for (i = 0; i <= 4; i++) {
        if (active[i] != 0) {
            void *o = *(void **)obj;
            s16 off = *(s16 *)((char *)o + 0x8);
            void (*draw)(void *) = *(void (**)(void *))((char *)o + 0xC);
            draw(rowBase + off);
        }
        rowBase += 0x4C;
        obj += 0x4C;
    }
    if (*(s32 *)((char *)p + 0x2C0) != 0) {
        GuiTextElementDraw((char *)p + 0x198);
        if (*(s32 *)((char *)p + 0x2C4) != 0) {
            if (*(s32 *)((char *)p + 0x2CC) != 0) {
                GuiTextElementDraw((char *)p + 0x248);
            }
            if (*(s32 *)((char *)p + 0x2C8) != 0) {
                GuiTextElementDraw((char *)p + 0x1F0);
            }
        }
    }
}
#endif

/* func_0033C060: write two floats through *(p+0x0) at +0/+4, re-reading the
 * pointer between stores. */
/* Byte-exact under the canonical engine flags (-fno-strict-aliasing restores
   the pointer reload between the two stores). */
void func_0033C060(void *p, f32 a, f32 b) {
    (*(f32 **)p)[0] = a;
    (*(f32 **)p)[1] = b;
}

/**
 * GuiDialogBoxSetBounds: write six dialog-box bound floats into the contiguous
 * f32 block at +0x2A8..+0x2BC. Args 1,2 land at +0x2A8/+0x2AC, args 3,4 at
 * +0x2B8/+0x2BC, and args 5,6 at +0x2B0/+0x2B4 between them — the mapping is
 * not in offset order. Seven callers pass e.g. (64,-164,10,136,10,99) and
 * (0,-143,0,114,0,140); which pair is which quantity is not established here.
 *
 * The stores are in ARGUMENT order, and that is what byte-matches. Do NOT
 * rewrite them into the order the .s listing shows: cc1 reorders within the
 * block — it emits the last store first and sinks the second-to-last into the
 * `jr` delay slot — so feeding it the emitted order produces a DIFFERENT
 * emitted order. Measured by words: listing order 1 of 7, argument order 7 of 7.
 * See docs/WORKFLOW.md, "Do not copy the emitted instruction order back into
 * the C".
 *
 * MATCHED: byte-identical, confirmed by two independent instruments — a
 * unit-level objdiff_build.sh rebuild, and a disassembly of the boot ELF at
 * 0x0033C078. The function is 7 words (glabel..endlabel); the `nop` at
 * 0x0033C094 is padding after endlabel, not part of it.
 *
 * A standalone diff.sh percentage is not sufficient evidence on its own: it
 * over-reports, has been seen to under-report on this unit, and scored the
 * broken listing-order form at "94.86%" when by words it is 1 of 7 — only the
 * `jr` survives, the six stores being rotated by one.
 *
 * RELOC PRECONDITION: target_relocs == base_relocs == 0 (/19147). That
 * equality is trivially satisfied -- this leaf's operands are all immediate
 * offsets on $4, so neither side carries a relocation -- so it rules out the
 * reloc-normalisation over-report but is not itself the evidence; the byte
 * comparison and the ROM anchor are.
 */
void GuiDialogBoxSetBounds(void *p, f32 a, f32 b, f32 c, f32 d, f32 e, f32 f) {
    *(f32 *)((char *)p + 0x2A8) = a;
    *(f32 *)((char *)p + 0x2AC) = b;
    *(f32 *)((char *)p + 0x2B8) = c;
    *(f32 *)((char *)p + 0x2BC) = d;
    *(f32 *)((char *)p + 0x2B0) = e;
    *(f32 *)((char *)p + 0x2B4) = f;
}

/* GuiDialogBoxSetText3: assign the three dialog-box text labels - the title
 * (p+0x198 <- t0), the body (p+0x248 <- t1) and the prompt (p+0x1F0 <- t2). */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiDialogBoxSetText3)
S136OS_SLOT(GuiDialogBoxSetText3);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiDialogBoxSetText3, unit objdiff): 63.14%,
   10/26 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* WALL on cc1 2.9/2.96 only (s136os is byte-exact, GUARD above): 3-callee-save frame
   wall. */
extern void GuiElementSetText(GuiElement *e, s32 text);
void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2) {
    GuiElementSetText((GuiElement *)((char *)p + 0x198), t0); /* title */
    GuiElementSetText((GuiElement *)((char *)p + 0x248), t1); /* body */
    GuiElementSetText((GuiElement *)((char *)p + 0x1F0), t2); /* footer */
}
#endif

/* GuiDialogBoxSetScale: store the dialog-box scale float at +0x2A4. */
void GuiDialogBoxSetScale(void *p, f32 scale) {
    *(f32 *)((char *)p + 0x2A4) = scale;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C0F8);

/* func_0033C100: init the screen's ten embedded sub-elements at their fixed
 * offsets (five type-B at p+0/+0x4C/+0x98/+0xE4/+0x130, three type-C at
 * +0x1D8/+0x230/+0x288, then the two D_1ADA18 widgets at +0x2E0/+0x31C), return
 * the object. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C100);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033C100, unit objdiff): 64.46%,
   22/41 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x30' vs 'addiu sp, sp, -0x10').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; 4-callee-save frame
   wall. */
void *func_0033C100(void *p) {
    GuiElementInitTypeB(p);
    GuiElementInitTypeB((char *)p + 0x4C);
    GuiElementInitTypeB((char *)p + 0x98);
    GuiElementInitTypeB((char *)p + 0xE4);
    GuiElementInitTypeB((char *)p + 0x130);
    GuiElementInitTypeC((char *)p + 0x1D8);
    GuiElementInitTypeC((char *)p + 0x230);
    GuiElementInitTypeC((char *)p + 0x288);
    func_003374D8((char *)p + 0x2E0);
    func_003374D8((char *)p + 0x31C);
    return p;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C1A8);

/* GuiIconListScreenInit: construct an icon-list screen — three header buttons
 * (+0x0/+0x4C/+0x98), three list-icon elements (+0xE4/+0x130/+0x17C, a stride-0x4C
 * run the original inits/colours/glyphs in loops), three text rows (+0x1D8/+0x288/
 * +0x230), and two sprites (+0x2E0/+0x31C). Pool -> alloc the 0x10-byte placement
 * record (+0x1C8), +0x1CC=pool + +0x358=1 unconditional; record 255x207. Header
 * buttons coloured 0x60442D00/0x60241700/0x55F0C070 + glyphs 0x87/0x88/0x89; the
 * three icons coloured 0x80FFDE8D + glyph 0x8A (middle icon +0x130 y-flipped via
 * scale 1,-1); text rows 0x80F0F0F0 (two right-flagged); big sprite +0x2E0
 * coloured 0x70A0C0C0 textured 0xE99A(frame 0xA) scaled 240x194, small sprite
 * +0x31C scaled 32x32. Labels localized 0x2BF6/0x2BE5/0x2C0B. Closed by
 * func_0033C588(w, 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiIconListScreenInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiIconListScreenInit, unit objdiff): 44.85%,
   209/301 insns differ. Residual: UNKNOWN-daddu + gp/abs-mixed symbol (first differing insn: '' vs 'daddu s2, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_0033C588(void *w, s32 flag);
extern char *g_guiInstance;
extern u8 D_1ADBE8[], D_1ADBF0[], D_1ADBF8[], D_1ADC00[], D_1ADD30[];
extern u8 D_1ADD38[], D_1ADD40[], D_1ADD48[], D_1ADD50[];
void GuiIconListScreenInit(void *w, GuiPool *pool) {
    GuiElement *e0 = (GuiElement *)((char *)w + 0x0);
    GuiElement *e1 = (GuiElement *)((char *)w + 0x4C);
    GuiElement *e2 = (GuiElement *)((char *)w + 0x98);
    GuiElement *icon0 = (GuiElement *)((char *)w + 0xE4);
    GuiElement *icon1 = (GuiElement *)((char *)w + 0x130);
    GuiElement *icon2 = (GuiElement *)((char *)w + 0x17C);
    GuiElement *t1D8 = (GuiElement *)((char *)w + 0x1D8);
    GuiElement *t288 = (GuiElement *)((char *)w + 0x288);
    GuiElement *t230 = (GuiElement *)((char *)w + 0x230);
    GuiElement *sprBig = (GuiElement *)((char *)w + 0x2E0);
    GuiElement *sprSmall = (GuiElement *)((char *)w + 0x31C);
    void *rec;

    /* +0x1CC = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x1CC) = pool;
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x1C8) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    *(s32 *)((char *)w + 0x358) = 1;
    rec = *(void **)((char *)w + 0x1C8);
    *(f32 *)((char *)rec + 0x0) = 255.0f;
    *(f32 *)((char *)rec + 0x4) = 207.0f;

    GuiElementInit(e0, (s32)D_1ADBE8, pool);
    GuiElementInit(e1, (s32)D_1ADBF0, pool);
    GuiElementInit(e2, (s32)D_1ADBF8, pool);
    GuiElementInit(icon0, (s32)D_1ADC00, pool);
    GuiElementInit(icon1, (s32)D_1ADC00, pool);
    GuiElementInit(icon2, (s32)D_1ADC00, pool);
    GuiTextElementInit(t1D8, (s32)D_1ADD30, pool);
    GuiTextElementInit(t288, (s32)D_1ADD38, pool);
    GuiTextElementInit(t230, (s32)D_1ADD40, pool);
    GuiElementSetTextFlag(t288, 0);
    GuiElementSetTextFlag(t230, 0);
    GuiSpriteElementInit(sprBig, (s32)D_1ADD48, pool);
    GuiSpriteElementInit(sprSmall, (s32)D_1ADD50, pool);

    *GuiElementGetColor(e0) = 0x60442D00;
    *GuiElementGetColor(e1) = 0x60241700;
    *GuiElementGetColor(e2) = 0x55F0C070;
    *GuiElementGetColor(icon0) = (s32)0x80FFDE8D;
    *GuiElementGetColor(icon1) = (s32)0x80FFDE8D;
    *GuiElementGetColor(icon2) = (s32)0x80FFDE8D;
    *GuiElementGetColor(sprBig) = 0x70A0C0C0;
    *GuiElementGetColor(t1D8) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t288) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t230) = (s32)0x80F0F0F0;

    GuiElementSetGlyph(e0, (s32)(g_guiInstance + 0x8710), 0x87);
    GuiElementSetGlyph(e1, (s32)(g_guiInstance + 0x8710), 0x88);
    GuiElementSetGlyph(e2, (s32)(g_guiInstance + 0x8710), 0x89);
    GuiElementSetGlyph(icon0, (s32)(g_guiInstance + 0x8710), 0x8A);
    GuiElementSetGlyph(icon1, (s32)(g_guiInstance + 0x8710), 0x8A);
    GuiElementSetGlyph(icon2, (s32)(g_guiInstance + 0x8710), 0x8A);

    GuiElementSetScale(icon1, 1.0f, -1.0f, 0.0f, 0.0f);
    GuiSpriteSetTexture(sprBig, 0xE99A, 0xA);
    GuiElementSetScale(sprBig, 240.0f, 194.0f, 0.0f, 0.0f);
    GuiElementSetScale(sprSmall, 32.0f, 32.0f, 0.0f, 0.0f);

    *(s32 *)((char *)w + 0x1D0) = 0;
    GuiElementSetText(t1D8, GetLocalizedString(0x2BF6));
    GuiElementSetText(t288, GetLocalizedString(0x2BE5));
    GuiElementSetText(t230, GetLocalizedString(0x2C0B));

    func_0033C588(w, 0);
}
#endif

/* func_0033C580: a bare `daddu $2,$4,$0` fall-through fragment (NO jr $31) -
 * a handwritten stub-table entry, not a real C function. WALL: stays asm. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C580);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C588);

/* func_0033C958(self): render the galaxy/level-select map screen. Draws the three
 * frame sprites (self+0x0/+0x4C/+0x98), three text labels (self+0x1D8/+0x230/
 * +0x288) and a sprite (self+0x2E0). Then for each of the 21 level markers, places
 * the shared marker element (self+0x31C) at the anchor (*(self+0x1C8)) offset by
 * the marker's table entry (D_265100, stride 0x10: s32 dx, s32 dy, s32 texIdx,
 * f32 scale), applies the icon (0xE99E) + scale, tints it visited (0x7029A1FF) vs
 * unvisited (0x70A0C0C0) from g_levelVisitedMarkers, and draws it if the level is
 * available (g_abLevelAvailableFlags). Finally, if a planet is selected
 * (g_mapVertexData+0x230 < 21), dispatches the 3 route sub-elements (self+0x114
 * table, stride 0x4C, base self+0xE4) via their draw methods and draws two
 * highlight line-pairs (func_00290320) around the selected planet's screen rect
 * (func_00336C18 element pos). Faithful TARGET_NATIVE #else (engine 2.96 = no
 * byte-match); GuiElementSetPos/SetScale element+arg order asm-verified. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033C958);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033C958, unit objdiff): 52.80%,
   231/298 insns differ. Residual: UNKNOWN-sd + movn/movz (first differing insn: 'sd s2, 0x10(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00290320(s32 x0, s32 y0, s32 x1, s32 y1, u32 color, s32 flag);
extern u8 D_265100[], g_levelVisitedMarkers[], g_abLevelAvailableFlags[],
          g_mapVertexData[];
void func_0033C958(void *self) {
    u8 *p = (u8 *)self;
    GuiElement *marker = (GuiElement *)(p + 0x31C);
    s32 i;

    GuiSpriteElementDraw(p);
    GuiSpriteElementDraw(p + 0x4C);
    GuiSpriteElementDraw(p + 0x98);
    GuiTextElementDraw(p + 0x1D8);
    GuiTextElementDraw(p + 0x230);
    GuiTextElementDraw(p + 0x288);
    func_00337630((GuiElement *)(p + 0x2E0));

    for (i = 0; i < 0x15; i++) {
        u8  *tbl = D_265100 + i * 0x10;
        f32 *anchor = *(f32 **)(p + 0x1C8);
        GuiElementSetPos(marker, (f32)*(s32 *)(tbl + 0) + anchor[0],
                         (f32)*(s32 *)(tbl + 4) + anchor[1], 0.0f, 0.0f);
        GuiSpriteSetTexture(marker, 0xE99E, *(s32 *)(tbl + 8));
        GuiElementSetScale(marker, *(f32 *)(tbl + 0xC), *(f32 *)(tbl + 0xC),
                           0.0f, 0.0f);
        *GuiElementGetColor(marker) =
            (g_levelVisitedMarkers[i] == 0) ? (s32)0x70A0C0C0 : (s32)0x7029A1FF;
        if (g_abLevelAvailableFlags[i] != 0) {
            func_00337630(marker);
        }
    }

    if (*(s32 *)(g_mapVertexData + 0x230) < 0x15) {
        GuiElement *e0E4 = (GuiElement *)(p + 0xE4);
        GuiElement *e130 = (GuiElement *)(p + 0x130);
        GuiElement *e17C = (GuiElement *)(p + 0x17C);
        s32 *desc = (s32 *)(p + 0x114);
        u8  *base = p + 0xE4;
        s32  n = 2;
        f32 *ra, *rb;

        do {
            s32 obj = *desc;
            n--;
            desc += 0x13;
            (*(void (**)(void *))(obj + 0xC))(base + *(s16 *)(obj + 8));
            base += 0x4C;
        } while (n > -1);

        ra = func_00336C18(e0E4); rb = func_00336C18(e130);
        func_00290320((s32)(ra[0] + 1.0f), (s32)(rb[1] + 6.0f),
                      (s32)(ra[0] + 1.0f), (s32)(ra[1] - 4.0f), 0x180000FF, 0);
        ra = func_00336C18(e17C);
        func_00290320((s32)(ra[0] + 8.0f), (s32)(ra[1] + 2.0f),
                      (s32)(ra[0] + 304.0f), (s32)(ra[1] + 2.0f), 0x180000FF, 0);
        ra = func_00336C18(e0E4); rb = func_00336C18(e130);
        func_00290320((s32)(ra[0] + 1.0f), (s32)(rb[1] + 6.0f),
                      (s32)(ra[0] + 1.0f), (s32)(ra[1] - 4.0f), 0x180000FF, 0);
        ra = func_00336C18(e17C);
        func_00290320((s32)(ra[0] + 8.0f), (s32)(ra[1] + 2.0f),
                      (s32)(ra[0] + 304.0f), (s32)(ra[1] + 2.0f), 0x180000FF, 0);
    }
}
#endif

/* func_0033CD80: init the embedded dialog-box (at p+0x8), return the object. */
#if defined(S136OS_func_0033CD80) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033CD80; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 11/11 words + 1 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033CD80 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033CD80(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#else
S136OS_SLOT(func_0033CD80);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033CDB0);

/* GuiDialogBoxVariantCInit: dialog constructor (variant C, sibling of
 * GuiQuitDialogInit). Panel (+0x8) 255x195, border from D_1ADD98, text rows =
 * strings 0x307B/0x2BE4/0x2BE5, bounds (0,-143,0,114,0,125), scale 0.65. Clears
 * +0x2D8 and +0x2D4, then runs func_0033CEE0(w, 0) (its body builder). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiDialogBoxVariantCInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiDialogBoxVariantCInit, unit objdiff): 68.04%,
   30/85 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a1, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern s32 func_0033CEE0(void *w, s32 flag);
extern u8 D_1ADD98[8];
void GuiDialogBoxVariantCInit(void *w, GuiPool *pool) {
    void *obj;
    s32 t0, t1, t2;

    /* +0x0 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2DC) = obj;
        *(s32 *)((char *)obj + 0x0) = 0;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
    }

    obj = *(void **)((char *)w + 0x2DC);
    *(f32 *)((char *)obj + 0x0) = 255.0f;
    *(f32 *)((char *)obj + 0x4) = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADD98);

    t0 = GetLocalizedString(0x307B);
    t1 = GetLocalizedString(0x2BE4);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 125.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.65f);
    *(s32 *)((char *)w + 0x2D8) = 0;
    *(s32 *)((char *)w + 0x2D4) = 0;
    func_0033CEE0(w, 0);
}
#endif

/* func_0033CEE0: variant-C dialog handler — a 2D map-pan controller. Re-anchors
 * the dialog box (+0x8) via func_0033BE70/func_0033C060. On confirm (0x40) with
 * selection +0x2D8 == 0, plays sound 4. On any D-pad bit (0xF000), nudges the GS
 * screen-context pan offsets: Y (g_gsScreenContext+0x16A) -1 on 0x1000 (clamp
 * >= -0x20) / +1 on 0x4000 (clamp <= 0x20); X (+0x168) -1 on 0x8000 (clamp
 * >= -0x28) / +1 on 0x2000 (clamp <= 0x28). If the pan actually moved, plays
 * sound 4, then rebuilds the display env via func_002857F0(). Returns bit 6 of
 * flags. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033CEE0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033CEE0, unit objdiff): 25.09%,
   113/137 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x20' vs 'addiu sp, sp, -0x30').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 g_gsScreenContext[];
extern void func_002857F0(void);
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);
s32 func_0033CEE0(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    f32 *anchor;
    s16 *panX = (s16 *)(g_gsScreenContext + 0x168);
    s16 *panY = (s16 *)(g_gsScreenContext + 0x16A);
    s16 origX, origY;

    func_0033BE70(box, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033C060(box, anchor[0], anchor[1]);

    if ((flags & 0x40) && *(s32 *)((char *)w + 0x2D8) == 0) {
        PlayGlobalSound(4, 0, 0);
    }

    if (flags & 0xF000) {
        origX = *panX;
        origY = *panY;
        if (flags & 0x1000) {          /* pan up: Y - 1, clamp to -0x20 */
            s16 v = (s16)(*panY - 1);
            *panY = v;
            if (v < -0x20) *panY = -0x20;
        }
        if (flags & 0x4000) {          /* pan down: Y + 1, clamp to 0x20 */
            s16 v = (s16)(*panY + 1);
            *panY = v;
            if (v >= 0x21) *panY = 0x20;
        }
        if (flags & 0x8000) {          /* pan left: X - 1, clamp to -0x28 */
            s16 v = (s16)(*panX - 1);
            *panX = v;
            if (v < -0x28) *panX = -0x28;
        }
        if (flags & 0x2000) {          /* pan right: X + 1, clamp to 0x28 */
            s16 v = (s16)(*panX + 1);
            *panX = v;
            if (v >= 0x29) *panX = 0x28;
        }
        if (origX != *panX || origY != *panY) {
            PlayGlobalSound(4, 0, 0);
        }
        func_002857F0();
    }

    return (flags >> 6) & 1;
}
#endif

/* func_0033D070: draw the progressive-scan option screen. Lays out the box body
 * (func_0033BF90(e+8)), formats the D_1ADDB0 template with the fixed label 0x307B
 * plus the on/off id from D_1ADDA8 selected by g_bProgressiveScan into a scratch
 * buffer, and draws it centered at (g_screenWidth/2, 0x8C) in colour 0x80F0F0F0
 * (func_002801B8). Then lays out a text box (func_00280C98) at a GUI-scaled
 * position (scale*170, scale*379 rounded, from the config float at
 * g_swapGadgetItemIndex+0x8A) and draws the help string 0x3136 into it
 * (func_00280B48). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D070);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033D070, unit objdiff): 30.48%,
   83/104 insns differ. Residual: UNKNOWN-addiu + movn/movz, gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0xa0' vs 'addiu sp, sp, -0x80').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_002801B8(s32 x, s32 y, u32 color, const char *text, s32 flag);
extern void func_00280C98(void *layout, s32 x, s32 y, s32 a, s32 b, s32 c, s32 d,
                          s32 e, s32 f);
extern void func_00280B48(void *layout, u32 color, const char *text, s32 flag);
extern u8 D_1ADDA8[], D_1ADDB0[];
extern s32 g_screenWidth, g_bProgressiveScan, g_swapGadgetItemIndex;
void func_0033D070(void *e) {
    char buf[64];
    u8 layout[32];
    s32 s1, s2;
    f32 scale;

    func_0033BF90((char *)e + 8);

    s1 = GetLocalizedString(0x307B);
    s2 = GetLocalizedString(*(s32 *)(D_1ADDA8 + (g_bProgressiveScan ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADDB0, (const char *)s1, (const char *)s2);
    func_002801B8(g_screenWidth >> 1, 0x8C, 0x80F0F0F0, buf, -1);

    scale = *(f32 *)((char *)&g_swapGadgetItemIndex + 0x8A);
    func_00280C98(layout, (s32)(scale * 170.0f + 0.5f),
                  (s32)(scale * 379.0f + 0.5f), 0x2C, 0x1D4, 0xFE, 0xDE, 0x14, 3);
    func_00280B48(layout, 0x80F0F0F0, (const char *)GetLocalizedString(0x3136), -1);
}
#endif

/* func_0033D1C0: init the embedded dialog-box (at p+0x8), return the object
 * (identical body to func_0033CD80). */
#if defined(S136OS_func_0033D1C0) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033D1C0; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 12/12 words + 1 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033D1C0 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033D1C0(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#else
S136OS_SLOT(func_0033D1C0);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D1F0);

/* func_0033D1F8: construct a dialog-box screen (GuiQuitDialogInit-family). Pool ->
 * alloc the 0x10-byte placement record (+0x2DC) and zero it (+0x0 pool store
 * unconditional). Panel (+0x8) seeded 255x195, border art from D_1ADDB8, three
 * text rows = localized 0x307A/0x2BE4/0x2BE5, bounds (0,-143,0,114,0,140), scale
 * 0.65. Clear +0x2D8, then func_0033D320(w, 0) (its re-layout builder). */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): the dialog-box family form:
 * SetText3 prototyped, block zeroed 4,8,C,0, colour vector re-read per store. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033D1F8)
S136OS_SLOT(func_0033D1F8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033D1F8, unit objdiff): 70.34%,
   28/83 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a1, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
s32 func_0033D320(void *p, s32 flags);
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern u8 D_1ADDB8[8];
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
void func_0033D1F8(void *w, GuiPool *pool) {
    void *obj;
    s32 t0, t1, t2;

    /* +0x0 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2DC) = obj;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
        *(s32 *)((char *)obj + 0x0) = 0;
    }

    (*(f32 **)((char *)w + 0x2DC))[0] = 255.0f;
    (*(f32 **)((char *)w + 0x2DC))[1] = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADDB8);

    t0 = GetLocalizedString(0x307A);
    t1 = GetLocalizedString(0x2BE4);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.65f);
    *(s32 *)((char *)w + 0x2D8) = 0;
    func_0033D320(w, 0);
}
#endif

/* func_0033D320: re-layout this dialog-box screen and (on first open) chime.
 *  - Run the dialog-box layout func_0033BE70 on the embedded box at p+0x8.
 *  - Re-feed it the two floats of the vector at *(p+0x2DC) via func_0033C060.
 *  - When (flags & 0x40) is set AND the +0x2D8 "already opened" latch is still
 *    clear, toggle the global one-shot flag D_1A7BB9, play the open chime
 *    (PlayGlobalSound(4,0,0)) and rebuild the camera projection.
 * Returns bit 6 of the flags argument. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D320);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033D320, unit objdiff): 89.45%,
   13/42 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s0, 0x0(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame +
   branch-likely (beql) guard wall. */
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);
extern void BuildCameraProjection(void);
extern u8 D_1A7BB9;
s32 func_0033D320(void *p, s32 flags) {
    void *box = (char *)p + 0x8;
    f32 *v;
    func_0033BE70(box, flags);
    v = *(f32 **)((char *)p + 0x2DC);
    func_0033C060(box, v[0], v[1]);
    if ((flags & 0x40) != 0 && *(s32 *)((char *)p + 0x2D8) == 0) {
        D_1A7BB9 = (D_1A7BB9 < 1);
        PlayGlobalSound(4, 0, 0);
        BuildCameraProjection();
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033D3C0: draw a two-part formatted caption. Lays out the box body
 * (func_0033BF90(e+8)), formats the D_1ADDB0 template with two localized strings
 * (the fixed 0x307A plus one of the two ids in D_1ADDA8 selected by the one-shot
 * flag D_1A7BB9) into a scratch buffer, then draws it at (0x100, 0xAA) in colour
 * 0x80F0F0F0 via func_002801B8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D3C0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033D3C0, unit objdiff): 50.07%,
   36/49 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0xb0' vs 'addiu sp, sp, -0x90').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_002801B8(s32 x, s32 y, u32 color, const char *text, s32 flag);
extern u8 D_1ADDA8[], D_1ADDB0[], D_1A7BB9;
void func_0033D3C0(void *e) {
    char buf[128];
    s32 s1, s2;

    func_0033BF90((char *)e + 8);

    s1 = GetLocalizedString(0x307A);
    s2 = GetLocalizedString(*(s32 *)(D_1ADDA8 + (D_1A7BB9 ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADDB0, (const char *)s1, (const char *)s2);
    func_002801B8(0x100, 0xAA, 0x80F0F0F0, buf, -1);
}
#endif

/* func_0033D478: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#if defined(S136OS_func_0033D478) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033D478; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 12/12 words + 1 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033D478 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033D478(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#else
S136OS_SLOT(func_0033D478);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D4A8);

/* func_0033D4B0: dialog-box screen constructor (func_0033D1F8 family). Panel
 * 255x195, border D_1ADDC8, text rows localized 0x2C2E/0x2BE4/0x2BE5, bounds
 * (0,-143,0,114,0,140), scale 0.65, then func_0033D5D8(w, 0). */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): the dialog-box family form:
 * SetText3 prototyped, block zeroed 4,8,C,0, colour vector re-read per store. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033D4B0)
S136OS_SLOT(func_0033D4B0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033D4B0, unit objdiff): 70.34%,
   28/83 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a1, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
s32 func_0033D5D8(void *w, s32 flags);
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern u8 D_1ADDC8[8];
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
void func_0033D4B0(void *w, GuiPool *pool) {
    void *obj;
    s32 t0, t1, t2;

    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2DC) = obj;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
        *(s32 *)((char *)obj + 0x0) = 0;
    }

    (*(f32 **)((char *)w + 0x2DC))[0] = 255.0f;
    (*(f32 **)((char *)w + 0x2DC))[1] = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADDC8);

    t0 = GetLocalizedString(0x2C2E);
    t1 = GetLocalizedString(0x2BE4);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.65f);
    *(s32 *)((char *)w + 0x2D8) = 0;
    func_0033D5D8(w, 0);
}
#endif

/* func_0033D5D8: the settings-menu re-layout + input handler (closer of
 * func_0033D4B0). Re-anchors the dialog box (+0x8) via func_0033BE70/func_0033C060.
 * Left (0x1000) / right (0x4000) cycle the selection +0x2D8 through 0..3
 * ((sel+3)%4 / (sel+1)%4) with move sound 3. On confirm (0x40, sound 4) it toggles
 * the option for the current row: 0 -> word D_1A7BAC, 1 -> word D_1A7BB0,
 * 2 -> D_1A7BB4 = (D_1A7BB4+1)%3, 3 -> byte D_1A7BB9 + BuildCameraProjection().
 * Returns bit 6 of flags. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D5D8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033D5D8, unit objdiff): 73.80%,
   49/112 insns differ. Residual: UNKNOWN-sd + movn/movz (first differing insn: 'sd s0, 0x0(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void BuildCameraProjection(void);
extern s32 D_1A7BAC, D_1A7BB0, D_1A7BB4;
extern u8 D_1A7BB9;
s32 func_0033D5D8(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    f32 *anchor;
    s32 sel;

    func_0033BE70(box, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033C060(box, anchor[0], anchor[1]);

    if (flags & 0x1000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 3) % 4;
    } else if (flags & 0x4000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 1) % 4;
    } else if (flags & 0x40) {
        PlayGlobalSound(4, 0, 0);
        sel = *(s32 *)((char *)w + 0x2D8);
        if (sel == 0) {
            D_1A7BAC = (D_1A7BAC == 0) ? 1 : 0;
        } else if (sel == 1) {
            D_1A7BB0 = (D_1A7BB0 == 0) ? 1 : 0;
        } else if (sel == 2) {
            D_1A7BB4 = (D_1A7BB4 + 1) % 3;
        } else if (sel == 3) {
            D_1A7BB9 = (D_1A7BB9 == 0) ? 1 : 0;
            BuildCameraProjection();
        }
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033D780(self): render a 4-row settings screen (self[0x2D8] = selected row,
 * drawn 0x80D0D0D0 vs 0x70808080). Each row: a localized label + a toggle/option
 * string chosen from a config table by a flag, formatted (func_00115DA8 / D_1ADDB0)
 * and centred (func_00280250) at per-row coords. Rows 0/1 use string IDs
 * D_1ADDF8[0/1] gated by D_1A7BAC / D_1A7BB0; row 2 uses (D_1ADE00..)[D_1A7BB4];
 * row 3 uses D_1ADDA8[0/1] gated by D_1A7BB9. Labels 0x2C78/0x2C77/0x2DAE/0x307A.
 * Ghidra-verified. Faithful TARGET_NATIVE #else (engine 2.96 = no byte-match). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033D780);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033D780, unit objdiff): 55.36%,
   162/189 insns differ. Residual: UNKNOWN-addiu + movn/movz, gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x110' vs 'addiu sp, sp, -0x100').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8   D_1ADDF8[], D_1ADE00[], D_1ADE08[], D_1ADDA8[], D_1ADDB0[];
extern s32  D_1A7BAC, D_1A7BB0, D_1A7BB4;
extern u8   D_1A7BB9;
extern s32  D_1ADDD8, D_1ADDDC, D_1ADDE0, D_1ADDE4, D_1ADDE8, D_1ADDEC, D_1ADDF0, D_1ADDF4;
extern void func_00280250(s32 x, s32 y, u32 colour, void *str, s32 flag);

void func_0033D780(void *self) {
    u8  *s = (u8 *)self;
    char buf[0x80];
    u8   cfg1[0x14];
    u8   cfg2[8];
    s32  sel = *(s32 *)(s + 0x2D8);
    s32  label, val;
    u32  colour;
    s32  idx;

    func_0033BF90(s + 8);
    memcpy(cfg1 + 0x00, D_1ADDF8, 8);
    memcpy(cfg1 + 0x08, D_1ADE00, 8);
    memcpy(cfg1 + 0x10, D_1ADE08, 4);
    memcpy(cfg2, D_1ADDA8, 8);

    colour = (sel == 0) ? 0x80D0D0D0 : 0x70808080;
    label = GetLocalizedString(0x2C78);
    idx = (D_1A7BAC == 0) ? 0 : 4;
    val = GetLocalizedString(*(s32 *)(cfg1 + idx));
    func_00115DA8(buf, (char *)D_1ADDB0, (char *)label, (char *)val);
    func_00280250(D_1ADDD8, D_1ADDDC, colour, buf, -1);

    colour = (sel == 1) ? 0x80D0D0D0 : 0x70808080;
    label = GetLocalizedString(0x2C77);
    idx = (D_1A7BB0 == 0) ? 0 : 4;
    val = GetLocalizedString(*(s32 *)(cfg1 + idx));
    func_00115DA8(buf, (char *)D_1ADDB0, (char *)label, (char *)val);
    func_00280250(D_1ADDE0, D_1ADDE4, colour, buf, -1);

    colour = (sel == 2) ? 0x80D0D0D0 : 0x70808080;
    label = GetLocalizedString(0x2DAE);
    val = GetLocalizedString(*(s32 *)(cfg1 + 8 + D_1A7BB4 * 4));
    func_00115DA8(buf, (char *)D_1ADDB0, (char *)label, (char *)val);
    func_00280250(D_1ADDE8, D_1ADDEC, colour, buf, -1);

    colour = (sel == 3) ? 0x80D0D0D0 : 0x70808080;
    label = GetLocalizedString(0x307A);
    idx = (D_1A7BB9 == 0) ? 0 : 4;
    val = GetLocalizedString(*(s32 *)(cfg2 + idx));
    func_00115DA8(buf, (char *)D_1ADDB0, (char *)label, (char *)val);
    func_00280250(D_1ADDF0, D_1ADDF4, colour, buf, -1);
}
#endif

/* func_0033DA00: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#if defined(S136OS_func_0033DA00) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033DA00; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 11/11 words + 1 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033DA00 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033DA00(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#else
S136OS_SLOT(func_0033DA00);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DA30);

/* GuiDialogBoxVariantBInit: dialog constructor (variant B, sibling of
 * GuiQuitDialogInit). Panel (+0x8) 255x195, border from D_1ADE10, text rows =
 * strings 0x2C29/0x2BE4/0x2BE5, bounds (0,-143,0,114,0,140), scale 0.2. Clears
 * +0x2D8, then runs func_0033DB60(w, 0) (its body builder). */
/* GUARD (task #1335): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever, e6e3b6fda): the dialog-box family
 * form: SetText3 prototyped, block zeroed 4,8,C,0, colour vector re-read per
 * store. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiDialogBoxVariantBInit)
S136OS_SLOT(GuiDialogBoxVariantBInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiDialogBoxVariantBInit, unit objdiff): 70.33%,
   29/83 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a1, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern s32 func_0033DB60(void *w, s32 flag);
extern u8 D_1ADE10[8];
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
void GuiDialogBoxVariantBInit(void *w, GuiPool *pool) {
    void *obj;
    s32 t0, t1, t2;

    /* +0x0 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2DC) = obj;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
        *(s32 *)((char *)obj + 0x0) = 0;
    }

    (*(f32 **)((char *)w + 0x2DC))[0] = 255.0f;
    (*(f32 **)((char *)w + 0x2DC))[1] = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADE10);

    t0 = GetLocalizedString(0x2C29);
    t1 = GetLocalizedString(0x2BE4);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.2f);
    *(s32 *)((char *)w + 0x2D8) = 0;
    func_0033DB60(w, 0);
}
#endif

/* func_0033DB60: verbatim twin of func_0033E8B0 (variant-B dialog option handler).
 * Re-anchors the dialog box (+0x8) via func_0033BE70/func_0033C060, then on L/R
 * (0x1000|0x4000) plays move sound 3 + cycles selection +0x2D8 = (sel+1)%2; on
 * confirm (0x40) plays sound 4 + toggles the per-row option flag (D_1A7B9D for
 * row 0, D_1A7B9C for row 1). Returns bit 6 of flags. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DB60);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033DB60, unit objdiff): 78.65%,
   25/69 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s0, 0x0(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 D_1A7B9C, D_1A7B9D;
s32 func_0033DB60(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    f32 *anchor;
    s32 sel;

    func_0033BE70(box, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033C060(box, anchor[0], anchor[1]);

    if ((flags & 0x1000) || (flags & 0x4000)) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 1) % 2;
    } else if (flags & 0x40) {
        PlayGlobalSound(4, 0, 0);
        sel = *(s32 *)((char *)w + 0x2D8);
        if (sel == 0) {
            D_1A7B9D = (D_1A7B9D == 0) ? 1 : 0;
        } else if (sel == 1) {
            D_1A7B9C = (D_1A7B9C == 0) ? 1 : 0;
        }
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033DC68: draw a two-row toggle menu (each row a formatted caption). Lays
 * out the box body (func_0033BF90(w+8)), then draws two rows via func_002801B8:
 * row 0 at (0x100, 0x96) = D_1ADDB0 template with strings 0x2C75 + the D_1ADDA8 id
 * picked by flag D_1A7B9D; row 1 at (0x100, 0xBE) with 0x2C74 + the id picked by
 * D_1A7B9C. The currently-selected row (*(w+0x2D8)) is drawn bright (0x80D0D0D0),
 * the other dimmed (0x70808080). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DC68);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033DC68, unit objdiff): 41.15%,
   94/104 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0xe0' vs 'addiu sp, sp, -0xd0').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_002801B8(s32 x, s32 y, u32 color, const char *text, s32 flag);
extern u8 D_1ADDA8[], D_1ADDB0[], D_1A7B9C, D_1A7B9D;
void func_0033DC68(void *w) {
    char buf[128];
    s32 sel = *(s32 *)((char *)w + 0x2D8);
    u32 color;
    s32 s0, s1;

    func_0033BF90((char *)w + 0x8);

    color = (sel == 0) ? 0x80D0D0D0 : 0x70808080;
    s0 = GetLocalizedString(0x2C75);
    s1 = GetLocalizedString(*(s32 *)(D_1ADDA8 + (D_1A7B9D ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADDB0, (const char *)s0, (const char *)s1);
    func_002801B8(0x100, 0x96, color, buf, -1);

    color = (sel == 1) ? 0x80D0D0D0 : 0x70808080;
    s0 = GetLocalizedString(0x2C74);
    s1 = GetLocalizedString(*(s32 *)(D_1ADDA8 + (D_1A7B9C ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADDB0, (const char *)s0, (const char *)s1);
    func_002801B8(0x100, 0xBE, color, buf, -1);
}
#endif

/* func_0033DDC8: init the embedded dialog-box (p+0x8) and two list-row elements
 * (p+0x2DC, p+0x324), return the object. */
#if defined(S136OS_func_0033DDC8) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033DDC8; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 15/15 words + 3 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033DDC8 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033DDC8(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    GuiListRowElementInit((char *)p + 0x2DC);
    GuiListRowElementInit((char *)p + 0x324);
    return p;
}
#else
S136OS_SLOT(func_0033DDC8);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DE08);

/* func_0033DE10: dialog-box screen with TWO scrolling lists (record at +0x36C).
 * Panel 255x195, border D_1ADE20, text rows localized 0x2C2D/0x2BE4/0x2BE5, bounds
 * (0,-143,0,114,0,140), scale 0.67. Builds two identical lists (+0x2DC and +0x324),
 * each: GuiListElementInit(0x96 px wide, v34=1, tag D_1ADC60), 0x10 px tall
 * (SetVisibleRows), meter max 0x400 (SetItemCount), value 0, colour pairs (0x55F0C070 x2, 0x60442D00 x2), +0x44=
 * 0x20FFFFFF / +0x38=0. Clear +0x2D8, then func_0033E070(w, 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033DE10);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033DE10, unit objdiff): 70.90%,
   75/170 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s5, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
s32 func_0033E070(void *w, s32 flags);
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern u8 D_1ADE20[8], D_1ADC60[];
void func_0033DE10(void *w, GuiPool *pool) {
    GuiElement *list1 = (GuiElement *)((char *)w + 0x2DC);
    GuiElement *list2 = (GuiElement *)((char *)w + 0x324);
    void *obj;
    s32 t0, t1, t2;

    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x36C) = obj;
        *(s32 *)((char *)obj + 0x0) = 0;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
    }

    obj = *(void **)((char *)w + 0x36C);
    *(f32 *)((char *)obj + 0x0) = 255.0f;
    *(f32 *)((char *)obj + 0x4) = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADE20);

    t0 = GetLocalizedString(0x2C2D);
    t1 = GetLocalizedString(0x2BE4);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);
    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.67f);

    GuiListElementInit(list1, 0x96, 1, (s32)D_1ADC60, pool);
    GuiListSetVisibleRows(list1, 0x10);
    GuiListSetItemCount(list1, 0x400);
    GuiListSetScrollPos(list1, 0);
    GuiListSetColorPair0(list1, 0x55F0C070, 0x55F0C070);
    GuiListSetColorPair1(list1, 0x60442D00, 0x60442D00);
    func_00337B88(list1, 0x20FFFFFF);
    func_00337B68(list1, 0);

    GuiListElementInit(list2, 0x96, 1, (s32)D_1ADC60, pool);
    GuiListSetVisibleRows(list2, 0x10);
    GuiListSetItemCount(list2, 0x400);
    GuiListSetScrollPos(list2, 0);
    GuiListSetColorPair0(list2, 0x55F0C070, 0x55F0C070);
    GuiListSetColorPair1(list2, 0x60442D00, 0x60442D00);
    func_00337B88(list2, 0x20FFFFFF);
    func_00337B68(list2, 0);

    *(s32 *)((char *)w + 0x2D8) = 0;
    func_0033E070(w, 0);
}
#endif

/* func_0033E070: the in-game audio-options menu handler (closer of func_0033DE10 —
 * this is UpdateAudioOptionsMenuInGame). Re-anchors the box + positions the two
 * volume-slider lists (music +0x2DC, sfx +0x324) at the anchor (*(w+0x36C)) plus
 * D_1ADE30..3C. Left/right (0x1000/0x4000) cycle the selected row +0x2D8 through
 * 0..2 ((sel+2)%3 / (sel+1)%3) with sound 3. Holding up (0x8000) / down (0x2000)
 * steps the selected slider by 6, clamped to [0, 0x400] (row 0 = g_musicVolume,
 * row 1 = g_sfxVolume). Confirm (0x40) on row 2 toggles g_audioStereoMode + pushes
 * it to the driver (func_00132938), other rows play sound 5. Always mirrors the
 * volumes into the list scroll positions and recomputes the channel mix
 * (func_002E5698). Returns bit 6 of flags. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E070);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033E070, unit objdiff): 47.20%,
   139/206 insns differ. Residual: UNKNOWN-addiu + movn/movz, gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x30' vs 'addiu sp, sp, -0x40').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00132938(s32 stereoOff);
extern void func_002E5698(void);
extern s32 g_padButtonsHeld, g_musicVolume, g_sfxVolume, g_audioStereoMode;
extern f32 D_1ADE30, D_1ADE34, D_1ADE38, D_1ADE3C;
s32 func_0033E070(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    GuiElement *musicList = (GuiElement *)((char *)w + 0x2DC);
    GuiElement *sfxList = (GuiElement *)((char *)w + 0x324);
    f32 *anchor;
    s32 sel, held, v;

    func_0033BE70(box, flags);
    anchor = *(f32 **)((char *)w + 0x36C);
    func_0033C060(box, anchor[0], anchor[1]);
    anchor = *(f32 **)((char *)w + 0x36C);
    GuiElementSetPos(musicList, anchor[0] + D_1ADE30, anchor[1] + D_1ADE34, 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x36C);
    GuiElementSetPos(sfxList, anchor[0] + D_1ADE38, anchor[1] + D_1ADE3C, 0.0f, 0.0f);

    if (flags & 0x1000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 2) % 3;
    } else if (flags & 0x4000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 1) % 3;
    } else {
        held = g_padButtonsHeld;
        sel = *(s32 *)((char *)w + 0x2D8);
        if (held & 0x8000) {
            if (sel == 0) {
                v = g_musicVolume - 6;
                g_musicVolume = (v < 0) ? 0 : v;
            } else if (sel == 1) {
                v = g_sfxVolume - 6;
                g_sfxVolume = (v < 0) ? 0 : v;
            }
        } else if (held & 0x2000) {
            if (sel == 0) {
                v = g_musicVolume + 6;
                g_musicVolume = (v > 0x400) ? 0x400 : v;
            } else if (sel == 1) {
                v = g_sfxVolume + 6;
                g_sfxVolume = (v > 0x400) ? 0x400 : v;
            }
        } else if (flags & 0x40) {
            if (sel == 2) {
                PlayGlobalSound(4, 0, 0);
                g_audioStereoMode = (g_audioStereoMode == 0) ? 1 : 0;
                func_00132938(g_audioStereoMode ^ 1);
            } else {
                PlayGlobalSound(5, 0, 0);
            }
        }
    }

    GuiListSetScrollPos(sfxList, g_sfxVolume);
    GuiListSetScrollPos(musicList, g_musicVolume);
    func_002E5698();
    return (flags >> 6) & 1;
}
#endif

/* func_0033E308(self): render the audio-settings screen — three rows (self[0x2D8]
 * = selected index; the selected row is drawn in 0x80D0D0D0, others 0x70808080).
 * Rows 0/1: localized label (string 0x2DA5/0x2DA6 via GetLocalizedString + the
 * func_00115DA8 formatter with D_1ADE48) right-aligned at (0x100, 0x91/0xBE) with a
 * meter bar (func_00337350 on self+0x2DC/+0x324). Row 2: label 0x2DA7 + the
 * stereo/mono string (D_1ADE40[g_audioStereoMode?4:0]) formatted with D_1ADDB0,
 * centred at (0x100, 0xE1). Ghidra-verified. Faithful TARGET_NATIVE #else. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E308);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s32 g_audioStereoMode;
extern void func_00337350(void *self);
extern u8 D_1ADDB0[];
/* (end of this body's declarations) */
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033E308, unit objdiff): 50.65%,
   89/115 insns differ. Residual: UNKNOWN-sd + movn/movz (first differing insn: 'sd s1, 0x98(sp)' vs 'sd s0, 0x90(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00280120(s32 x, s32 y, u32 colour, void *str, s32 flag); /* draw right-aligned */
extern void func_00280250(s32 x, s32 y, u32 colour, void *str, s32 flag); /* draw centred */
extern u8   D_1ADE40[], D_1ADE48[];

void func_0033E308(void *self) {
    u8  *s = (u8 *)self;
    u8   buf[0x80];
    u8   modeStrs[8];
    s32  sel = *(s32 *)(s + 0x2D8);
    s32  str;
    u32  colour;

    func_0033BF90(s + 8);

    colour = (sel == 0) ? 0x80D0D0D0 : 0x70808080;
    memcpy(modeStrs, D_1ADE40, 8);
    str = GetLocalizedString(0x2DA5);
    func_00115DA8((char *)buf, (char *)D_1ADE48, (char *)str);
    func_00280120(0x100, 0x91, colour, buf, -1);
    func_00337350(s + 0x2DC);

    colour = (sel == 1) ? 0x80D0D0D0 : 0x70808080;
    str = GetLocalizedString(0x2DA6);
    func_00115DA8((char *)buf, (char *)D_1ADE48, (char *)str);
    func_00280120(0x100, 0xBE, colour, buf, -1);
    func_00337350(s + 0x324);

    colour = (sel == 2) ? 0x80D0D0D0 : 0x70808080;
    {
        s32   label = GetLocalizedString(0x2DA7);
        s32   idx = (g_audioStereoMode == 0) ? 0 : 4;
        str = GetLocalizedString(*(s32 *)(modeStrs + idx));
        func_00115DA8((char *)buf, (char *)D_1ADDB0, (char *)label, (char *)str);
        func_00280250(0x100, 0xE1, colour, buf, -1);
    }
}
#endif

/* func_0033E488: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#if defined(S136OS_func_0033E488) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033E488; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 11/11 words + 1 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033E488 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033E488(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#else
S136OS_SLOT(func_0033E488);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E4B8);

/* func_0033E4C0: dialog-box screen constructor (twin of func_0033D1F8). Panel
 * 255x195, border D_1ADE50, text rows localized 0x2C2F/0x2BE4/0x2BE5, bounds
 * (0,-143,0,114,0,140), scale 0.3, then func_0033E5E8(w, 0). */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): the dialog-box family form:
 * SetText3 prototyped, block zeroed 4,8,C,0, colour vector re-read per store. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033E4C0)
S136OS_SLOT(func_0033E4C0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033E4C0, unit objdiff): 70.33%,
   29/83 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a1, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
s32 func_0033E5E8(void *p, s32 flags);
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern u8 D_1ADE50[8];
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
void func_0033E4C0(void *w, GuiPool *pool) {
    void *obj;
    s32 t0, t1, t2;

    /* +0x0 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2DC) = obj;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
        *(s32 *)((char *)obj + 0x0) = 0;
    }

    (*(f32 **)((char *)w + 0x2DC))[0] = 255.0f;
    (*(f32 **)((char *)w + 0x2DC))[1] = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADE50);

    t0 = GetLocalizedString(0x2C2F);
    t1 = GetLocalizedString(0x2BE4);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.3f);
    *(s32 *)((char *)w + 0x2D8) = 0;
    func_0033E5E8(w, 0);
}
#endif

/* func_0033E5E8: re-layout this dialog-box screen and (on first open) chime.
 *  - Run the dialog-box layout func_0033BE70 on the embedded box at p+0x8.
 *  - Re-feed it the two floats of the vector at *(p+0x2DC) via func_0033C060.
 *  - When (flags & 0x40) is set AND the +0x2D8 "already opened" latch is still
 *    clear, play the open chime (PlayGlobalSound(4,0,0)) and toggle the global
 *    one-shot flag D_1A7B9E (set it to !D_1A7B9E).
 * Returns bit 6 of the flags argument. */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): PlayGlobalSound declared s32, as
 * defined in 1DFF80.cpp. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033E5E8)
S136OS_SLOT(func_0033E5E8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033E5E8, unit objdiff): 90.74%,
   16/39 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s0, 0x0(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact: 3-callee-save frame +
   branch-likely (beql) guard wall. */
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);  /* returns s32 (1DFF80.cpp) */
extern u8 D_1A7B9E;
s32 func_0033E5E8(void *p, s32 flags) {
    void *box = (char *)p + 0x8;
    f32 *v;
    func_0033BE70(box, flags);
    v = *(f32 **)((char *)p + 0x2DC);
    func_0033C060(box, v[0], v[1]);
    if ((flags & 0x40) != 0 && *(s32 *)((char *)p + 0x2D8) == 0) {
        PlayGlobalSound(4, 0, 0);
        D_1A7B9E = (D_1A7B9E < 1);
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033E680: draw a two-part formatted caption (sibling of func_0033D3C0),
 * with a state-dependent colour: 0x80D0D0D0 when *(w+0x2D8)==0 (dimmed) else
 * 0x70808080. Lays out the box body (func_0033BF90(w+8)), formats D_1ADDB0 with
 * the fixed string 0x2C2F plus the D_1ADDA8 id selected by flag D_1A7B9E, draws it
 * at (0x100, 0xAA). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E680);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern u8 D_1A7B9E;
/* (end of this body's declarations) */
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033E680, unit objdiff): 50.77%,
   45/57 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0xc0' vs 'addiu sp, sp, -0xa0').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_002801B8(s32 x, s32 y, u32 color, const char *text, s32 flag);
extern u8 D_1ADDA8[], D_1ADDB0[];
void func_0033E680(void *w) {
    char buf[128];
    s32 s1, s2;
    u32 color;

    func_0033BF90((char *)w + 0x8);

    color = (*(s32 *)((char *)w + 0x2D8) == 0) ? 0x80D0D0D0 : 0x70808080;
    s1 = GetLocalizedString(0x2C2F);
    s2 = GetLocalizedString(*(s32 *)(D_1ADDA8 + (D_1A7B9E ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADDB0, (const char *)s1, (const char *)s2);
    func_002801B8(0x100, 0xAA, color, buf, -1);
}
#endif

/* GuiQuitDialogInitElements: init the embedded dialog-box (at p+0x8), return the
 * object (identical body to func_0033CD80). */
#if defined(S136OS_GuiQuitDialogInitElements) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_GuiQuitDialogInitElements; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 12/12 words + 1 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_GuiQuitDialogInitElements engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *GuiQuitDialogInitElements(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#else
S136OS_SLOT(GuiQuitDialogInitElements);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E780);

/* GuiQuitDialogInit: construct the quit-confirmation dialog. If a pool is given,
 * allocate its 0x10-byte placement record (+0x2DC) and zero it. Seed the panel
 * (+0x8) to 255x195, init border art from D_1ADE60, set its three text rows to
 * localized strings 0x2BF7/0x2BE4/0x2BE5, bounds (0,-143,0,114,0,140), scale 0.4.
 * Clear +0x2D8 and run func_0033E8B0(w, 0) (the option-list builder). */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): the dialog-box family form:
 * SetText3 prototyped, block zeroed 4,8,C,0, colour vector re-read per store. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GuiQuitDialogInit)
S136OS_SLOT(GuiQuitDialogInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiQuitDialogInit, unit objdiff): 70.33%,
   29/83 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a1, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern s32 func_0033E8B0(void *w, s32 flag);
extern u8 D_1ADE60[8];
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
void GuiQuitDialogInit(void *w, GuiPool *pool) {
    void *obj;
    s32 t0, t1, t2;

    /* +0x0 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2DC) = obj;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
        *(s32 *)((char *)obj + 0x0) = 0;
    }

    (*(f32 **)((char *)w + 0x2DC))[0] = 255.0f;
    (*(f32 **)((char *)w + 0x2DC))[1] = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADE60);

    t0 = GetLocalizedString(0x2BF7);
    t1 = GetLocalizedString(0x2BE4);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.4f);
    *(s32 *)((char *)w + 0x2D8) = 0;
    func_0033E8B0(w, 0);
}
#endif

/* func_0033E8B0: the quit-dialog option-list handler. Re-lays-out the dialog box
 * (+0x8) at its anchor (*(w+0x2DC)) via func_0033BE70/func_0033C060, then reacts to
 * the input mask (flags): on left/right (0x1000|0x4000) play the move sound and
 * cycle the selection +0x2D8 between 0 and 1; on confirm (0x40) play the confirm
 * sound and toggle the option flag for the current row (D_1A7BBA for row 0,
 * D_1A7BBB for row 1). Returns bit 6 of flags (the confirm/accept bit). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E8B0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033E8B0, unit objdiff): 78.65%,
   25/69 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s0, 0x0(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 D_1A7BBB;
s32 func_0033E8B0(void *w, s32 flags) {
    void *box = (char *)w + 0x8;
    f32 *anchor;
    s32 sel;

    func_0033BE70(box, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033C060(box, anchor[0], anchor[1]);

    if ((flags & 0x1000) || (flags & 0x4000)) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x2D8) = (*(s32 *)((char *)w + 0x2D8) + 1) % 2;
    } else if (flags & 0x40) {
        PlayGlobalSound(4, 0, 0);
        sel = *(s32 *)((char *)w + 0x2D8);
        if (sel == 0) {
            D_1A7BBA = (D_1A7BBA == 0) ? 1 : 0;
        } else if (sel == 1) {
            D_1A7BBB = (D_1A7BBB == 0) ? 1 : 0;
        }
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033E9B8: draw the two-line widescreen/mode toggle caption on dialog `w`.
 * Opens a text batch (func_0027F7A0), then draws two localized lines centred at
 * x=0x100 (y=0x96 and y=0xBE). Each line is sprintf'd from the shared format
 * D_1ADDB0 with a fixed label (GetLocalizedString(0x2C30) / (0x3079)) and a
 * value string chosen from the 2-entry id table D_1ADDA8 by the toggle byte
 * (D_1A7BBA for line 1, D_1A7BBB for line 2). The currently-selected line
 * (*(w+0x2D8): 0 or 1) is drawn bright (0x80D0D0D0), the other dimmed
 * (0x70808080). Closes the batch (func_0027F790). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033E9B8);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern u8 D_1ADDA8[], D_1ADDB0[];
extern u8 D_1A7BBB;
/* (end of this body's declarations) */
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033E9B8, unit objdiff): 33.26%,
   101/112 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0xe0' vs 'addiu sp, sp, -0xd0').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_0027F7A0(void);
extern void func_0027F790(void);
void func_0033E9B8(void *w) {
    char buf[128];
    s32 sel = *(s32 *)((char *)w + 0x2D8);
    s32 label, value;
    u32 color;

    func_0033BF90((char *)w + 0x8);
    func_0027F7A0();

    /* line 1 (y=0x96), highlighted when line 0 is selected */
    color = (sel == 0) ? 0x80D0D0D0 : 0x70808080;
    label = GetLocalizedString(0x2C30);
    value = GetLocalizedString(*(s32 *)&D_1ADDA8[D_1A7BBA ? 4 : 0]);
    func_00115DA8(buf, (const char *)D_1ADDB0, (const char *)label, (const char *)value);
    func_002801B8(0x100, 0x96, color, buf, -1);

    /* line 2 (y=0xBE), highlighted when line 1 is selected */
    color = (sel == 1) ? 0x80D0D0D0 : 0x70808080;
    label = GetLocalizedString(0x3079);
    value = GetLocalizedString(*(s32 *)&D_1ADDA8[D_1A7BBB ? 4 : 0]);
    func_00115DA8(buf, (const char *)D_1ADDB0, (const char *)label, (const char *)value);
    func_002801B8(0x100, 0xBE, color, buf, -1);

    func_0027F790();
}
#endif

/* func_0033EB20: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#if defined(S136OS_func_0033EB20) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033EB20; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 12/12 words + 1 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033EB20 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033EB20(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#else
S136OS_SLOT(func_0033EB20);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EB50);

/* func_0033EB58: dialog-box screen constructor (func_0033D1F8 family). Panel
 * 255x195, border D_1ADE70, text rows localized 0x2BF7/0x2BE4/0x2BE5, bounds
 * (0,-143,0,114,0,140), scale 0.3, then func_0033EC80(w, 0). */
/* GUARD (task #1335): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever, e6e3b6fda): the dialog-box family
 * form: SetText3 prototyped, block zeroed 4,8,C,0, colour vector re-read per
 * store. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033EB58)
S136OS_SLOT(func_0033EB58);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033EB58, unit objdiff): 70.33%,
   29/83 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a1, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
s32 func_0033EC80(void *w, s32 flags);
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern u8 D_1ADE70[8];
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
void func_0033EB58(void *w, GuiPool *pool) {
    void *obj;
    s32 t0, t1, t2;

    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2DC) = obj;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
        *(s32 *)((char *)obj + 0x0) = 0;
    }

    (*(f32 **)((char *)w + 0x2DC))[0] = 255.0f;
    (*(f32 **)((char *)w + 0x2DC))[1] = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADE70);

    t0 = GetLocalizedString(0x2BF7);
    t1 = GetLocalizedString(0x2BE4);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.3f);
    *(s32 *)((char *)w + 0x2D8) = 0;
    func_0033EC80(w, 0);
}
#endif

/* Position a widget sub-element (+0x8) at its anchor (*(w+0x2DC)); when the
 * confirm bit (arg & 0x40) is set and the widget isn't already busy (+0x2D8 == 0),
 * play the confirm sound and toggle the widescreen flag D_1A7BBA. Returns bit 6
 * of the input flags. */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033EC80)
S136OS_SLOT(func_0033EC80);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033EC80, unit objdiff): 90.74%,
   16/39 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s0, 0x0(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
s32 func_0033EC80(void *w, s32 flags) {
    void *sub = (char *)w + 0x8;
    f32 *anchor;

    func_0033BE70(sub, flags);   /* 2nd arg ignored by the callee */
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033C060(sub, anchor[0], anchor[1]);
    if ((flags & 0x40) && *(s32 *)((char *)w + 0x2D8) == 0) {
        PlayGlobalSound(4, 0, 0);
        D_1A7BBA = (D_1A7BBA == 0) ? 1 : 0;
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033ED18: draw a two-part formatted caption (twin of func_0033D3C0), with
 * the fixed string 0x2C30, the D_1ADDA8 id selected by flag D_1A7BBA, colour
 * 0x80F0F0F0, drawn at (0x100, 0xAA). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033ED18);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033ED18, unit objdiff): 50.07%,
   36/49 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0xb0' vs 'addiu sp, sp, -0x90').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_002801B8(s32 x, s32 y, u32 color, const char *text, s32 flag);
extern u8 D_1ADDA8[], D_1ADDB0[];
void func_0033ED18(void *e) {
    char buf[128];
    s32 s1, s2;

    func_0033BF90((char *)e + 0x8);

    s1 = GetLocalizedString(0x2C30);
    s2 = GetLocalizedString(*(s32 *)(D_1ADDA8 + (D_1A7BBA ? 4 : 0)));
    func_00115DA8(buf, (const char *)D_1ADDB0, (const char *)s1, (const char *)s2);
    func_002801B8(0x100, 0xAA, 0x80F0F0F0, buf, -1);
}
#endif

/* func_0033EDD0: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#if defined(S136OS_func_0033EDD0) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033EDD0; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 12/12 words + 1 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033EDD0 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033EDD0(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#else
S136OS_SLOT(func_0033EDD0);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EE00);

/* func_0033EE08: dialog-box screen constructor (func_0033D1F8 family). Panel
 * 255x195, border D_1ADE80, text rows localized 0x2BF7/0x2BE4/0x2BE5, bounds
 * (0,-143,0,114,0,140), scale 0.4, then func_0033EF30(w, 0). */
/* GUARD (task #1335): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever, e6e3b6fda): the dialog-box family
 * form: SetText3 prototyped, block zeroed 4,8,C,0, colour vector re-read per
 * store. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033EE08)
S136OS_SLOT(func_0033EE08);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033EE08, unit objdiff): 70.33%,
   29/83 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a1, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
s32 func_0033EF30(void *w, s32 flags);
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern u8 D_1ADE80[8];
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
void func_0033EE08(void *w, GuiPool *pool) {
    void *obj;
    s32 t0, t1, t2;

    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2DC) = obj;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
        *(s32 *)((char *)obj + 0x0) = 0;
    }

    (*(f32 **)((char *)w + 0x2DC))[0] = 255.0f;
    (*(f32 **)((char *)w + 0x2DC))[1] = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADE80);

    t0 = GetLocalizedString(0x2BF7);
    t1 = GetLocalizedString(0x2BE4);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.4f);
    *(s32 *)((char *)w + 0x2D8) = 0;
    func_0033EF30(w, 0);
}
#endif

/* Sibling of func_0033EC80: position a sub-element (+0x8) at its anchor
 * (*(w+0x2DC)); on the confirm bit (arg & 0x40) while not busy (+0x2D8 == 0),
 * play the confirm sound + toggle D_1A7BBA. Returns bit 6 of the flags. */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033EF30)
S136OS_SLOT(func_0033EF30);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033EF30, unit objdiff): 90.74%,
   16/39 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s0, 0x0(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
s32 func_0033EF30(void *w, s32 flags) {
    void *sub = (char *)w + 0x8;
    f32 *anchor;

    func_0033BE70(sub, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033C060(sub, anchor[0], anchor[1]);
    if ((flags & 0x40) && *(s32 *)((char *)w + 0x2D8) == 0) {
        PlayGlobalSound(4, 0, 0);
        D_1A7BBA = (D_1A7BBA == 0) ? 1 : 0;
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033EFC8: init the embedded dialog-box (at p+0x8), return the object
 * (byte-identical body to func_0033CD80). */
#if defined(S136OS_func_0033EFC8) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033EFC8; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 12/12 words + 1 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033EFC8 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033EFC8(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    return p;
}
#else
S136OS_SLOT(func_0033EFC8);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033EFF8);

/* func_0033F000: dialog-box screen constructor (func_0033D1F8 family). Panel
 * 255x195, border D_1ADE90, text rows localized 0x2BF7/0x2BE4/0x2BE5, bounds
 * (0,-143,0,114,0,140), scale 0.4, then func_0033F128(w, 0). */
/* GUARD (task #1335): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever, e6e3b6fda): the dialog-box family
 * form: SetText3 prototyped, block zeroed 4,8,C,0, colour vector re-read per
 * store. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033F000)
S136OS_SLOT(func_0033F000);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033F000, unit objdiff): 70.33%,
   29/83 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a1, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
s32 func_0033F128(void *w, s32 flags);
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern u8 D_1ADE90[8];
extern void GuiDialogBoxSetText3(void *p, s32 t0, s32 t1, s32 t2);
void func_0033F000(void *w, GuiPool *pool) {
    void *obj;
    s32 t0, t1, t2;

    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2DC) = obj;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
        *(s32 *)((char *)obj + 0x0) = 0;
    }

    (*(f32 **)((char *)w + 0x2DC))[0] = 255.0f;
    (*(f32 **)((char *)w + 0x2DC))[1] = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADE90);

    t0 = GetLocalizedString(0x2BF7);
    t1 = GetLocalizedString(0x2BE4);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.4f);
    *(s32 *)((char *)w + 0x2D8) = 0;
    func_0033F128(w, 0);
}
#endif

/* Verbatim sibling of func_0033EC80/func_0033EF30: sub-element position (+0x8 at
 * anchor *(w+0x2DC)) + confirm-sound/D_1A7BBA-toggle on (arg & 0x40) while not
 * busy (+0x2D8 == 0). Returns bit 6 of the flags. */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033F128)
S136OS_SLOT(func_0033F128);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033F128, unit objdiff): 90.74%,
   16/39 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s0, 0x0(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
s32 func_0033F128(void *w, s32 flags) {
    void *sub = (char *)w + 0x8;
    f32 *anchor;

    func_0033BE70(sub, flags);
    anchor = *(f32 **)((char *)w + 0x2DC);
    func_0033C060(sub, anchor[0], anchor[1]);
    if ((flags & 0x40) && *(s32 *)((char *)w + 0x2D8) == 0) {
        PlayGlobalSound(4, 0, 0);
        D_1A7BBA = (D_1A7BBA == 0) ? 1 : 0;
    }
    return (flags >> 6) & 1;
}
#endif

/* func_0033F1C0: init the embedded dialog-box (p+0x8) and the GuiWidget at
 * p+0x2E0 (func_00348BD0), return the object. */
#if defined(S136OS_func_0033F1C0) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033F1C0; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 14/14 words + 2 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033F1C0 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033F1C0(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    func_00348BD0((char *)p + 0x2E0);
    return p;
}
#else
S136OS_SLOT(func_0033F1C0);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F1F8);

/* func_0033F200: dialog-box screen constructor with an embedded list (record at
 * +0x3BC). Panel 255x195, border D_1ADEA0, text rows localized 0x2C36/0x2C0B/
 * 0x2BE5, bounds (0,-143,0,114,0,140), scale 0.4, clear +0x2D8. Then builds the
 * list element (+0x2E0): func_00348BF8(pool), func_00348E10(-0x53, 0x20),
 * func_00348E20(1), and runs func_0033F3F0(w, 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F200);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033F200, unit objdiff): 73.73%,
   46/98 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s4, 0x20(sp)' vs 'sd s3, 0x18(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
s32 func_0033F3F0(void *w, s32 arg1);
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern void func_00348BF8(void *w, void *pool);
extern void func_00348E20(void *w, s32 v);
extern u8 D_1ADEA0[8];
void func_0033F200(void *w, GuiPool *pool) {
    void *obj;
    void *list = (char *)w + 0x2E0;
    s32 t0, t1, t2;

    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x3BC) = obj;
        *(s32 *)((char *)obj + 0x0) = 0;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
    }

    obj = *(void **)((char *)w + 0x3BC);
    *(f32 *)((char *)obj + 0x0) = 255.0f;
    *(f32 *)((char *)obj + 0x4) = 195.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADEA0);

    t0 = GetLocalizedString(0x2C36);
    t1 = GetLocalizedString(0x2C0B);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, t1, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    GuiDialogBoxSetScale((char *)w + 0x8, 0.4f);
    *(s32 *)((char *)w + 0x2D8) = 0;

    func_00348BF8(list, pool);
    func_00348E10(list, -0x53, 0x20);
    func_00348E20(list, 1);
    func_0033F3F0(w, 0);
}
#endif

/* func_0033F360: call func_0033F398(p) to get a row index, then return the
 * row's address: base (+0x3B8) + index * 0x14 (20-byte stride). */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033F360)
S136OS_SLOT(func_0033F360);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033F360, unit objdiff): 66.08%,
   8/15 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu v1, zero, 0x14' vs 'sll v1, v0, 2').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* WALL on cc1 2.9/2.96 only (s136os is byte-exact, GUARD above): 2-callee-save frame
   wall. */
void func_0033F398(void *p);
s32 func_0033F360(void *p) {
    s32 idx = ((s32 (*)(void *))func_0033F398)(p);
    return *(s32 *)((char *)p + 0x3B8) + idx * 0x14;
}
#endif

/* func_0033F398: forward p+0x2E0 to func_00348E60. */
void func_0033F398(void *p) {
    func_00348E60((char *)p + 0x2E0);
    __asm__ __volatile__("");
}

/* func_0033F3B8: init the GuiWidget at p+0x2E0, then record the row-data
 * pointer (arg) at p+0x3B8. */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033F3B8)
S136OS_SLOT(func_0033F3B8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033F3B8, unit objdiff): 64.79%,
   10/16 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* WALL on cc1 2.9/2.96 only (s136os is byte-exact, GUARD above): 3-callee-save frame
   wall ($16/$17/$31 16-byte vs 8-byte slot packing). */
extern void GuiMenuListSetRows(void *p, void *records);
void func_0033F3B8(void *p, void *records) {
    GuiMenuListSetRows((char *)p + 0x2E0, records);
    *(void **)((char *)p + 0x3B8) = records;
}
#endif

/* Reposition two linked sub-elements to the tracked anchor (*(w+0x3BC)): run the
 * pre-step func_0033BE70 on the inner element (+0x8), place it at the anchor via
 * func_0033C060, set the outer element's (+0x2E0) value from arg1, then place it
 * at the anchor via GuiMenuListSetOrigin. The anchor is re-read for each placement.
 * Returns bit 6 of arg1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F3F0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033F3F0, unit objdiff): 53.50%,
   35/40 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x30' vs 'addiu sp, sp, -0x20').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
s32 func_0033F3F0(void *w, s32 arg1) {
    f32 *anchor;
    func_0033BE70((char *)w + 0x8, arg1);   /* 2nd arg (flags) is ignored by the callee */
    anchor = *(f32 **)((char *)w + 0x3BC);
    func_0033C060((char *)w + 0x8, anchor[0], anchor[1]);
    GuiMenuListHandleInput((char *)w + 0x2E0, arg1);
    anchor = *(f32 **)((char *)w + 0x3BC);
    GuiMenuListSetOrigin((char *)w + 0x2E0, anchor[0], anchor[1]);
    return (arg1 >> 6) & 1;
}
#endif

/* func_0033F478: tear down the embedded dialog-box (p+0x8), then reconfigure the
 * GuiWidget at p+0x2E0 with the per-language resource D_1ADEB0[g_currentLanguage]
 * (func_00348E58) and re-init it (GuiMenuListDraw). */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): the +0x2E0 element pointer is
 * formed after the first call, as the ROM reuses $16 for it. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033F478)
S136OS_SLOT(func_0033F478);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033F478, unit objdiff): 64.67%,
   11/23 insns differ. Residual: UNKNOWN-daddu (first differing insn: 'daddu s0, a0, zero' vs 'addiu s0, a0, 0x2e0').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact: gp_rel(D_1ADEB0) vs
   absolute(g_currentLanguage) addressing mix + 2-callee-save frame wall. */
extern u8 g_currentLanguage;
extern s32 D_1ADEB0[2];
void func_0033F478(void *p) {
    void *w;
    func_0033BF90((char *)p + 0x8);
    w = (char *)p + 0x2E0;
    func_00348E58(w, D_1ADEB0[g_currentLanguage]);
    GuiMenuListDraw(w);
}
#endif

/* func_0033F4D0: init the embedded dialog-box (p+0x8) and a type-C element
 * (p+0x2E0), return the object. */
#if defined(S136OS_func_0033F4D0) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033F4D0; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 14/14 words + 2 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033F4D0 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033F4D0(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    GuiElementInitTypeC((char *)p + 0x2E0);
    return p;
}
#else
S136OS_SLOT(func_0033F4D0);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F508);

/* func_0033F510: a wider dialog-box screen constructor (record at +0x2D8, panel
 * 255x208, border D_1ADEC8). Two-row text (localized 0x2CB7 title, empty middle,
 * 0x2BE5 footer), bounds (-128,-183,112,110,102,110), no scale, then
 * func_0033F610(w, 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F510);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033F510, unit objdiff): 68.88%,
   32/73 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s0, 0x0(sp)' vs 'sd s1, 0x8(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
s32 func_0033F610(void *p, s32 flags);
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern u8 D_1ADEC8[8];
void func_0033F510(void *w, GuiPool *pool) {
    void *obj;
    s32 t0, t2;

    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2D8) = obj;
        *(s32 *)((char *)obj + 0x0) = 0;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
    }

    obj = *(void **)((char *)w + 0x2D8);
    *(f32 *)((char *)obj + 0x0) = 255.0f;
    *(f32 *)((char *)obj + 0x4) = 208.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADEC8);

    t0 = GetLocalizedString(0x2CB7);
    t2 = GetLocalizedString(0x2BE5);
    GuiDialogBoxSetText3((char *)w + 0x8, t0, 0, t2);

    GuiDialogBoxSetBounds((char *)w + 0x8, -128.0f, -183.0f, 112.0f, 110.0f, 102.0f, 110.0f);
    func_0033F610(w, 0);
}
#endif

/* func_0033F610: forward the embedded dialog-box (p+0x8) to func_0033BE70, then
 * push the two floats from the vector at *(p+0x2D8) into it via func_0033C060.
 * Returns bit 6 of the flags argument. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F610);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033F610, unit objdiff): 52.52%,
   15/28 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s2, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
/* TODO(match): functional equivalent - not byte-exact; 3-callee-save frame
   wall. */
extern void func_0033BE70(void *p, s32 flags);
extern void func_0033C060(void *p, f32 a, f32 b);
s32 func_0033F610(void *p, s32 flags) {
    f32 *v;
    func_0033BE70((char *)p + 0x8, flags);
    v = *(f32 **)((char *)p + 0x2D8);
    func_0033C060((char *)p + 0x8, v[0], v[1]);
    return (flags >> 6) & 1;
}
#endif

/* func_0033F670: forward p+0x8 to func_0033BF90. */
void func_0033F670(void *p) {
    func_0033BF90((char *)p + 0x8);
    __asm__ __volatile__("");
}

/* func_0033F690: init the screen's thirteen embedded sub-elements (the dialog
 * box at +0x8, the D_1ADA18 widget at +0x2DC, three type-C, one type-B, then
 * seven more type-C), return the object. */
#if defined(S136OS_func_0033F690) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_0033F690; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 33/33 words + 12 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033F690 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_0033F690(void *p) {
    GuiDialogBoxInitElements((char *)p + 0x8);
    func_003374D8((char *)p + 0x2DC);
    GuiElementInitTypeC((char *)p + 0x318);
    GuiElementInitTypeC((char *)p + 0x370);
    GuiElementInitTypeC((char *)p + 0x3C8);
    GuiElementInitTypeB((char *)p + 0x420);
    GuiElementInitTypeC((char *)p + 0x470);
    GuiElementInitTypeC((char *)p + 0x4C8);
    GuiElementInitTypeC((char *)p + 0x520);
    GuiElementInitTypeC((char *)p + 0x578);
    GuiElementInitTypeC((char *)p + 0x5D0);
    GuiElementInitTypeC((char *)p + 0x628);
    return p;
}
#else
S136OS_SLOT(func_0033F690);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033F718);

/* GuiStatsPanelScreenInit: construct the stats panel (a bordered dialog box, a
 * column of label/value text rows, a button element, and a sprite). Pool -> alloc
 * the 0x10-byte placement record (+0x2D8, +0x0=pool unconditional); record seeded
 * 255x200; border art from D_1ADED8. Seven D_1ADC60 text rows + one D_1ADC60
 * button element (+0x420) are inited, coloured 0x80F0F0F0 (button 0x55F0C070),
 * their labels set (localized 0x333E/0x2BE5/0x3129/0x2C67 + the in-place buffers
 * at +0x680), row +0x578 scaled 0.9; the button gets glyph 0x5D. A sprite (+0x2DC)
 * is textured 0xEAA2 scaled 32x32 colour 0x60F0F0B0. Three more value rows
 * (+0x470/D_1ADEE8, +0x4C8/D_1ADEF0, +0x520/D_1ADEF8) are inited/coloured and
 * pointed at the +0x6C0/+0x6D8/+0x6F0 buffers. Finally func_0033FAB0(w, 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiStatsPanelScreenInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiStatsPanelScreenInit, unit objdiff): 61.44%,
   166/264 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s7, 0x58(sp)' vs 'sd s2, 0x30(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_0033FAB0(void *w, s32 flag);
extern void GuiDialogBoxInitBorder(void *w, void *pool, void *borderCfg);
extern char *g_guiInstance;
extern u8 D_1ADED8[8], D_1ADC60[], D_1ADEE8[], D_1ADEF0[], D_1ADEF8[];
void GuiStatsPanelScreenInit(void *w, GuiPool *pool) {
    GuiElement *t318 = (GuiElement *)((char *)w + 0x318);
    GuiElement *t370 = (GuiElement *)((char *)w + 0x370);
    GuiElement *t3C8 = (GuiElement *)((char *)w + 0x3C8);
    GuiElement *t628 = (GuiElement *)((char *)w + 0x628);
    GuiElement *t5D0 = (GuiElement *)((char *)w + 0x5D0);
    GuiElement *e420 = (GuiElement *)((char *)w + 0x420);
    GuiElement *t578 = (GuiElement *)((char *)w + 0x578);
    GuiElement *sprite = (GuiElement *)((char *)w + 0x2DC);
    GuiElement *t470 = (GuiElement *)((char *)w + 0x470);
    GuiElement *t4C8 = (GuiElement *)((char *)w + 0x4C8);
    GuiElement *t520 = (GuiElement *)((char *)w + 0x520);
    void *rec;

    /* +0x0 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x0) = pool;
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2D8) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    rec = *(void **)((char *)w + 0x2D8);
    *(f32 *)((char *)rec + 0x0) = 255.0f;
    *(f32 *)((char *)rec + 0x4) = 200.0f;
    GuiDialogBoxInitBorder((char *)w + 0x8, pool, D_1ADED8);

    GuiTextElementInit(t318, (s32)D_1ADC60, pool);
    GuiTextElementInit(t370, (s32)D_1ADC60, pool);
    GuiTextElementInit(t3C8, (s32)D_1ADC60, pool);
    GuiTextElementInit(t628, (s32)D_1ADC60, pool);
    GuiTextElementInit(t5D0, (s32)D_1ADC60, pool);
    GuiElementInit(e420, (s32)D_1ADC60, pool);
    GuiTextElementInit(t578, (s32)D_1ADC60, pool);

    GuiElementSetText(t5D0, (s32)((char *)w + 0x680));
    GuiElementSetText(t578, GetLocalizedString(0x333E));
    GuiElementSetScale(t578, 0.9f, 0.9f, 0.0f, 0.0f);
    GuiElementSetText(t318, GetLocalizedString(0x2BE5));
    GuiElementSetText(t370, GetLocalizedString(0x3129));
    GuiElementSetText(t3C8, GetLocalizedString(0x2C67));
    GuiElementSetTextFlag(t318, 0);
    GuiElementSetTextFlag(t370, 0);
    GuiElementSetTextFlag(t3C8, 0);

    *GuiElementGetColor(t318) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t370) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t3C8) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t628) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t5D0) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t578) = (s32)0x80F0F0F0;
    *GuiElementGetColor(e420) = 0x55F0C070;

    GuiSpriteElementInit(sprite, (s32)D_1ADC60, pool);
    GuiSpriteSetTexture(sprite, 0xEAA2, 0);
    GuiElementSetScale(sprite, 32.0f, 32.0f, 0.0f, 0.0f);
    *GuiElementGetColor(sprite) = 0x60F0F0B0;
    GuiElementSetGlyph(e420, (s32)(g_guiInstance + 0x8710), 0x5D);

    GuiTextElementInit(t470, (s32)D_1ADEE8, pool);
    GuiTextElementInit(t4C8, (s32)D_1ADEF0, pool);
    GuiTextElementInit(t520, (s32)D_1ADEF8, pool);
    *GuiElementGetColor(t470) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t4C8) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t520) = (s32)0x80F0F0F0;
    GuiElementSetText(t470, (s32)((char *)w + 0x6C0));
    GuiElementSetText(t4C8, (s32)((char *)w + 0x6D8));
    GuiElementSetText(t520, (s32)((char *)w + 0x6F0));

    func_0033FAB0(w, 0);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FAB0);

/* func_0033FCE8: stats-panel value-row layout. Positions four value elements at
 * fixed data-driven offsets from the shared anchor record *(w+0x2D8): +0x578 at
 * (D_1ADF50, D_1ADF54), +0x470 at (D_1ADF38, D_1ADF3C), +0x4C8 at
 * (D_1ADF40, D_1ADF44), and +0x520 at (D_1ADF48, D_1ADF4C). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FCE8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033FCE8, unit objdiff): 54.53%,
   49/70 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s0, 0x0(sp)' vs 'sd s1, 0x8(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern f32 D_1ADF38, D_1ADF3C, D_1ADF40, D_1ADF44;
extern f32 D_1ADF48, D_1ADF4C, D_1ADF50, D_1ADF54;
void func_0033FCE8(void *w) {
    f32 *anchor = *(f32 **)((char *)w + 0x2D8);

    GuiElementSetPos((GuiElement *)((char *)w + 0x578), D_1ADF50 + anchor[0], D_1ADF54 + anchor[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x470), D_1ADF38 + anchor[0], D_1ADF3C + anchor[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x4C8), D_1ADF40 + anchor[0], D_1ADF44 + anchor[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x520), D_1ADF48 + anchor[0], D_1ADF4C + anchor[1], 0.0f, 0.0f);
}
#endif

/* func_0033FDD8: draw the map screen's planet bolt-stats line. Only runs for a
 * valid selected planet (g_mapVertexData+0x230 in [1,20]). First formats the
 * planet name (localized 0x2C51) into the +0x6C0 buffer via D_1ADBA8 and draws
 * row +0x470; then reformats it as the platinum-bolt tally
 * (CountPlatinumBolts / func_002B1D18 table lookup, via D_1ADF70), repositions
 * row +0x470 (x = pos.x + D_1ADF68, y = pos.y) right-aligned (flag 2), and draws
 * rows +0x470 and +0x578. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FDD8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033FDD8, unit objdiff): 57.83%,
   56/82 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x40' vs 'addiu sp, sp, -0x30').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern s32 CountPlatinumBolts(s32 group);
extern s32 func_002B1D18(s32 idx);
extern u8 g_mapVertexData[], D_1ADBA8[], D_1ADF70[];
extern s32 D_1ADF68;
void func_0033FDD8(void *w) {
    s32 idx = *(s32 *)(g_mapVertexData + 0x230);

    if ((u32)(idx - 1) < 0x14) {
        char *buf = (char *)w + 0x6C0;
        GuiElement *row = (GuiElement *)((char *)w + 0x470);
        f32 *pos;
        s32 platinum, extra;

        func_00115DA8(buf, (const char *)D_1ADBA8, (const char *)GetLocalizedString(0x2C51));
        GuiElementSetTextFlag(row, 0);
        GuiTextElementDraw(row);

        platinum = CountPlatinumBolts(*(s32 *)(g_mapVertexData + 0x230));
        extra = func_002B1D18(*(s32 *)(g_mapVertexData + 0x230));
        func_00115DA8(buf, (const char *)D_1ADF70, platinum, extra);

        pos = func_00336C18(row);
        GuiElementSetPos(row, pos[0] + (f32)D_1ADF68, pos[1], 0.0f, 0.0f);
        GuiElementSetTextFlag(row, 2);
        GuiTextElementDraw(row);
        GuiTextElementDraw((GuiElement *)((char *)w + 0x578));
    }
}
#endif

/* func_0033FEF8: draw one composite screen - run the body builder at p+0x8, blit
 * the menu backdrop (func_002DBC98(0)), draw its three header text rows, the
 * inner panel (func_00337630 at p+0x2DC), a sprite (p+0x420), two footer text
 * rows, then the trailing builder func_0033FDD8(p).
 * BYTE-EXACT under the ENGINE compiler (#259): ee-gcc 2.96 + the engine
 * pipeline reproduces all 28 ROM words and all 10 relocations, once the
 * trailing `__asm__ __volatile__("")` suppresses the sibling call into
 * func_0033FDD8. Build it with
 * `tools/ee/diff96.sh usa text/235FE8 func_0033FEF8 <this file>`.
 *
 * INCLUDE_ASM is retained for the ordinary ee-gcc 2.9 unit build, which hits
 * the 16-byte-slot-vs-8-byte-slot frame wall (the whole 2-GPR-save class in
 * this unit): 5 prologue/epilogue words differ, 99.46% at the unit objdiff
 * gate. Without the guard the same build folds the tail call and reads
 * 93.75% -- the number the previous note recorded. */
/* Draw a composite dialog widget: run its background/frame sub-draws, then draw
 * its text sub-elements (offsets 0x318/0x370/0x3C8/0x628/0x5D0), a sprite
 * (0x420) and two helper sub-draws (0x2DC frame, and the tail func_0033FDD8). */
/* GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_0033FEF8 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0033FEF8)
S136OS_SLOT(func_0033FEF8);
#else
void func_0033FEF8(void *e) {
    func_0033BF90((char *)e + 0x8);
    func_002DBC98(0);
    GuiTextElementDraw((char *)e + 0x318);
    GuiTextElementDraw((char *)e + 0x370);
    GuiTextElementDraw((char *)e + 0x3C8);
    func_00337630((GuiElement *)((char *)e + 0x2DC));
    GuiSpriteElementDraw((char *)e + 0x420);
    GuiTextElementDraw((char *)e + 0x628);
    GuiTextElementDraw((char *)e + 0x5D0);
    func_0033FDD8(e);
    __asm__ __volatile__("");
}
#endif

/* func_0033FF68(self): initialise a composite GUI panel — a fixed layout of child
 * elements at self+offset. Eleven type-B elements (self+0, then stride 0x4C to
 * +0x214, a list-row at +0x260, then +0x2A8/+0x2F4), a stride-0x4C run of 8 more
 * type-B (self+0x340..), one at +0x5A0, a stride-0x3C run of 8 func_003374D8
 * elements (self+0x5EC..), one at +0x7D0, then six type-C (self+0x828/0x880/
 * 0x8D8/0x930/0x988/0x9E0). Returns self. Faithful TARGET_NATIVE #else. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_0033FF68);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_0033FF68, unit objdiff): 63.50%,
   72/94 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x60' vs 'addiu sp, sp, -0x30').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void *func_0033FF68(void *self) {
    u8 *s = (u8 *)self;
    u8 *p;
    s32 i;

    GuiElementInitTypeB(s + 0x000);
    GuiElementInitTypeB(s + 0x04C);
    GuiElementInitTypeB(s + 0x098);
    GuiElementInitTypeB(s + 0x0E4);
    GuiElementInitTypeB(s + 0x130);
    GuiElementInitTypeB(s + 0x17C);
    GuiElementInitTypeB(s + 0x1C8);
    GuiElementInitTypeB(s + 0x214);
    GuiListRowElementInit(s + 0x260);
    GuiElementInitTypeB(s + 0x2A8);
    GuiElementInitTypeB(s + 0x2F4);

    p = s + 0x340;
    for (i = 7; i != -1; i--) {
        GuiElementInitTypeB(p);
        p += 0x4C;
    }
    GuiElementInitTypeB(s + 0x5A0);

    p = s + 0x5EC;
    for (i = 7; i != -1; i--) {
        func_003374D8(p);
        p += 0x3C;
    }
    func_003374D8(s + 0x7D0);
    GuiElementInitTypeC(s + 0x828);
    GuiElementInitTypeC(s + 0x880);
    GuiElementInitTypeC(s + 0x8D8);
    GuiElementInitTypeC(s + 0x930);
    GuiElementInitTypeC(s + 0x988);
    GuiElementInitTypeC(s + 0x9E0);

    return self;
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003400D8);

/* GuiQuickSelectWheelInit(self, pool): construct the planet/level-select wheel
 * screen. Stores the pool (+0x810), allocates a size vector at +0x80C = {250, 200},
 * and builds the static chrome: header/help glyph elements (+0x0/+0x4C/+0xE4/+0x130/
 * +0x98/+0x17C/+0x1C8/+0x214, glyphs 0x63/0x64/0x66/0x65/0x29/0x5D/0x23/0x5E), four
 * text rows (+0x8D8/+0x930/+0x988/+0x9E0) and three centered marker glyphs (+0x7D0,
 * +0x2A8, +0x2F4, +0x5A0) with their colours/scales/positions. Then lays out the 8
 * wheel slots: each slot gets a label element (+0x340 + i*0x4C, positioned at the
 * anchor +113/-72) and a sprite icon (+0x5EC + i*0x3C) placed on a circle of radius
 * ~82x76 centered at (0x15C,0x6D) via WrapAnglePiSum + Sin/Cos of angle i*pi/4-pi.
 * Shares the first slot's scale vectors into the remaining 7 (two loops, strides
 * 0x4C/0x3C). Finishes the title/help text (localized 0x2BE9/0x2BF1/0x2BE5), the
 * bracket labels (+0x988/+0x9E0 scaled 1.25) and the item list (+0x260: 5 rows, 100
 * items, colour pairs), then tail-calls GuiQuickSelectWheelTick. Faithful
 * TARGET_NATIVE #else (engine 2.96 = no byte-match); all 12 GuiElementSetPos/SetScale
 * element+arg orders + the trig layout asm-verified per-call. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiQuickSelectWheelInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiQuickSelectWheelInit, unit objdiff): 53.04%,
   520/720 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x100' vs 'addiu sp, sp, -0xb0').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern char *g_guiInstance;
extern f32 func_00283B30(f32 x);   /* SinfVu0 */
extern f32 func_00283B48(f32 x);   /* CosfVu0 */
extern f32 WrapAnglePiSum(f32 angle, f32 halfPi);
extern u8 D_1ADBE8[], D_1ADBF0[], D_1ADBF8[], D_1ADC00[], D_1ADF78[], D_1ADF80[],
          D_1ADD38[], D_1ADF88[], D_1ADF90[], D_1ADF98[], D_1ADFA0[], D_1ADFA8[],
          D_1ADFB0[], D_1ADFB8[], D_1ADFC8[], D_1ADFD8[], D_1ADFE8[], D_1ADFF8[],
          D_1ADD30[], D_1AE008[], D_1AE018[], D_1ADB14[8], D_1ADB1C[8];
s32 GuiQuickSelectWheelTick(void *w, s32 flag);
void GuiQuickSelectWheelInit(void *self, GuiPool *pool) {
    u8 *p = (u8 *)self;
    s32 i;
    f32 *vec;

    *(GuiPool **)(p + 0x810) = pool;
    if (pool != 0) {
        vec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(f32 **)(p + 0x80C) = vec;
        vec[0] = 0; vec[1] = 0; vec[2] = 0; vec[3] = 0;
    }
    *(s32 *)(p + 0xAB8) = 1;
    vec = *(f32 **)(p + 0x80C);
    vec[0] = 250.0f;
    vec[1] = 200.0f;
    *(s32 *)(p + 0x814) = 0;
    *(s32 *)(p + 0x818) = 0;
    *(s32 *)(p + 0x81C) = 0;

    GuiElementInit((GuiElement *)(p + 0x0),   (s32)D_1ADBE8, pool);
    GuiElementInit((GuiElement *)(p + 0x4C),  (s32)D_1ADBF0, pool);
    GuiElementInit((GuiElement *)(p + 0xE4),  (s32)D_1ADBF8, pool);
    GuiElementInit((GuiElement *)(p + 0x130), (s32)D_1ADC00, pool);
    GuiElementInit((GuiElement *)(p + 0x98),  (s32)D_1ADF78, pool);
    GuiTextElementInit((GuiElement *)(p + 0x8D8), (s32)D_1ADF80, pool);
    GuiTextElementInit((GuiElement *)(p + 0x930), (s32)D_1ADD38, pool);
    GuiTextElementInit((GuiElement *)(p + 0x988), (s32)D_1ADF88, pool);
    GuiTextElementInit((GuiElement *)(p + 0x9E0), (s32)D_1ADF90, pool);
    GuiElementInit((GuiElement *)(p + 0x17C), (s32)D_1ADF98, pool);
    GuiElementInit((GuiElement *)(p + 0x1C8), (s32)D_1ADFA0, pool);
    GuiElementInit((GuiElement *)(p + 0x214), (s32)D_1ADFA8, pool);
    GuiSpriteElementInit((GuiElement *)(p + 0x7D0), (s32)D_1ADFB0, pool);
    GuiElementSetScale((GuiElement *)(p + 0x7D0), 32.0f, 32.0f, 0.0f, 0.0f);

    *GuiElementGetColor((GuiElement *)(p + 0x0))   = 0x60442D00;
    *GuiElementGetColor((GuiElement *)(p + 0x4C))  = 0x60241700;
    *GuiElementGetColor((GuiElement *)(p + 0xE4))  = 0x55F0C070;
    *GuiElementGetColor((GuiElement *)(p + 0x98))  = 0x60241700;
    *GuiElementGetColor((GuiElement *)(p + 0x130)) = 0x55F0C070;
    *GuiElementGetColor((GuiElement *)(p + 0x17C)) = 0x55F0C070;
    *GuiElementGetColor((GuiElement *)(p + 0x1C8)) = 0x55F0C070;
    *GuiElementGetColor((GuiElement *)(p + 0x214)) = 0x70FFFEED;
    *GuiElementGetColor((GuiElement *)(p + 0x7D0)) = 0x60F0F0B0;
    *GuiElementGetColor((GuiElement *)(p + 0x8D8)) = (s32)0x80F0F0F0;
    *GuiElementGetColor((GuiElement *)(p + 0x930)) = (s32)0x80F0F0F0;
    *GuiElementGetColor((GuiElement *)(p + 0x988)) = (s32)0x80F0F0F0;
    *GuiElementGetColor((GuiElement *)(p + 0x9E0)) = (s32)0x80F0F0F0;

    GuiElementSetGlyph((GuiElement *)(p + 0x0),   (s32)g_guiInstance + 0x8710, 99);
    GuiElementSetGlyph((GuiElement *)(p + 0x4C),  (s32)g_guiInstance + 0x8710, 100);
    GuiElementSetGlyph((GuiElement *)(p + 0x98),  (s32)g_guiInstance + 0x8710, 0x29);
    GuiElementSetGlyph((GuiElement *)(p + 0xE4),  (s32)g_guiInstance + 0x8710, 0x66);
    GuiElementSetGlyph((GuiElement *)(p + 0x130), (s32)g_guiInstance + 0x8710, 0x65);
    GuiElementSetGlyph((GuiElement *)(p + 0x17C), (s32)g_guiInstance + 0x8710, 0x5D);
    GuiElementSetGlyph((GuiElement *)(p + 0x1C8), (s32)g_guiInstance + 0x8710, 0x23);
    GuiElementSetGlyph((GuiElement *)(p + 0x214), (s32)g_guiInstance + 0x8710, 0x5E);

    GuiElementInit((GuiElement *)(p + 0x2A8), (s32)D_1ADFB8, pool);
    GuiElementSetGlyph((GuiElement *)(p + 0x2A8), (s32)g_guiInstance + 0x8710, 7);
    GuiElementSetScale((GuiElement *)(p + 0x2A8), 1.0f, 1.0f, 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)(p + 0x2A8), 125.0f, 208.0f, 0.0f, 0.0f);
    *GuiElementGetColor((GuiElement *)(p + 0x2A8)) = 0x60442D00;

    GuiElementInit((GuiElement *)(p + 0x2F4), (s32)D_1ADFC8, pool);
    GuiElementSetGlyph((GuiElement *)(p + 0x2F4), (s32)g_guiInstance + 0x8710, 6);
    GuiElementSetScale((GuiElement *)(p + 0x2F4), 1.0f, 1.0f, 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)(p + 0x2F4), 125.0f, 208.0f, 0.0f, 0.0f);
    *GuiElementGetColor((GuiElement *)(p + 0x2F4)) = 0x55F0C070;

    GuiElementInit((GuiElement *)(p + 0x5A0), (s32)D_1ADFD8, pool);
    GuiElementSetGlyph((GuiElement *)(p + 0x5A0), (s32)g_guiInstance + 0x8710, 0x10);
    GuiElementSetPos((GuiElement *)(p + 0x5A0), 125.0f, 208.0f, 0.0f, 0.0f);
    *GuiElementGetColor((GuiElement *)(p + 0x5A0)) = (s32)0x80FFDE8D;
    GuiElementSetVisible((GuiElement *)(p + 0x5A0), 1);

    *(s32 *)(p + 0x7CC) = 0;
    for (i = 0; i < 8; i++) {
        char buf[16];
        GuiElement *label = (GuiElement *)(p + i * 0x4C + 0x340);
        GuiElement *icon  = (GuiElement *)(p + i * 0x3C + 0x5EC);
        f32 *anchor, angle, s, c;

        func_00115DA8(buf, (char *)D_1ADFE8, i);
        GuiElementInit(label, (s32)buf, pool);
        GuiElementSetGlyph(label, (s32)g_guiInstance + 0x8710, i + 8);
        GuiElementSetScale(label, 1.0f, 1.0f, 0.0f, 0.0f);
        anchor = *(f32 **)(p + 0x80C);
        GuiElementSetPos(label, anchor[0] + 113.0f, anchor[1] - 72.0f, 0.0f, 0.0f);
        *GuiElementGetColor(label) = 0x60442D00;

        func_00115DA8(buf, (char *)D_1ADFF8, i);
        GuiSpriteElementInit(icon, (s32)buf, pool);
        GuiElementSetVisible(icon, 0);
        angle = WrapAnglePiSum((f32)i * 2.0f * 0.3926991f - 3.1415927f, 1.5707964f);
        s = func_00283B30(angle);
        c = func_00283B48(angle);
        GuiElementSetPos(icon, (f32)((s32)(s * 82.14f) + 0x15C),
                         (f32)((s32)(c * 76.442f) + 0x6D), 0.0f, 0.0f);
        GuiElementSetScale(icon, 32.0f, 32.0f, 0.0f, 0.0f);
        *GuiElementGetColor(icon) = 0x60F0F0B0;
    }

    {
        u8 *la = p + 0x38C;
        u8 *lb = p + 0x628;
        s32 k = 6;
        do {
            k--;
            GuiElementShareScaleVec((GuiElement *)la,
                GuiElementGetScaleVec((GuiElement *)(p + 0x340)));
            la += 0x4C;
            GuiElementShareScaleVec((GuiElement *)lb,
                GuiElementGetScaleVec((GuiElement *)(p + 0x5EC)));
            lb += 0x3C;
        } while (k > -1);
    }
    GuiElementShareScaleVec((GuiElement *)(p + 0x2A8),
        GuiElementGetScaleVec((GuiElement *)(p + 0x340)));

    GuiTextElementInit((GuiElement *)(p + 0x828), (s32)D_1ADD30, pool);
    *(s32 *)(p + 0x828 + 0x54) = 0;
    GuiTextElementInit((GuiElement *)(p + 0x880), (s32)D_1AE008, pool);
    *GuiElementGetColor((GuiElement *)(p + 0x828)) = (s32)0x80F0F0F0;
    *GuiElementGetColor((GuiElement *)(p + 0x880)) = (s32)0x80F0F0F0;
    GuiElementSetText((GuiElement *)(p + 0x828), GetLocalizedString(0x2BE9));
    *(u8 *)(p + 0xA78) = 0;
    GuiElementSetText((GuiElement *)(p + 0x880), (s32)(p + 0xA78));
    GuiElementSetText((GuiElement *)(p + 0x8D8), GetLocalizedString(0x2BF1));
    GuiElementSetText((GuiElement *)(p + 0x930), GetLocalizedString(0x2BE5));
    GuiElementSetText((GuiElement *)(p + 0x988), *(s32 *)D_1ADB14);
    GuiElementSetText((GuiElement *)(p + 0x9E0), *(s32 *)D_1ADB1C);
    GuiElementSetScale((GuiElement *)(p + 0x988), 1.25f, 1.25f, 0.0f, 0.0f);
    GuiElementSetScale((GuiElement *)(p + 0x9E0), 1.25f, 1.25f, 0.0f, 0.0f);

    GuiListElementInit((GuiElement *)(p + 0x260), 0x20, 0, (s32)D_1AE018, pool);
    GuiListSetVisibleRows((GuiElement *)(p + 0x260), 5);
    GuiListSetItemCount((GuiElement *)(p + 0x260), 100);
    GuiListSetScrollPos((GuiElement *)(p + 0x260), 0);
    GuiListSetColorPair0((GuiElement *)(p + 0x260), 0x6049C1FF, 0x60001EFF);
    GuiListSetColorPair1((GuiElement *)(p + 0x260), 0x50F0C070, 0x50F0C070);
    func_00337B88((GuiElement *)(p + 0x260), (s32)0x80000000);
    func_00337B68((GuiElement *)(p + 0x260), 0);
    GuiQuickSelectWheelTick(self, 0);
}
#endif
/* GuiQuickSelectWheelTick: per-frame update for the quick-select weapon wheel `w`.
 * Layout: the static frame + the six petal decorations (w+0x828/0x8D8/0x930/0x988/
 * 0x9E0 and the labels at +0x2A8/0x2F4/0x5A0) are pinned at fixed offsets
 * (D_1AE020..D_1AE048) relative to the placement anchor (*(w+0x80C)); w+0x0/0x4C/
 * 0xE4/0x130 sit on the anchor itself.
 * Slots: for each of the 8 wheel positions i, the slot's entry (stride 0x1C) is read
 * from the quick-select table at *(g_hudMobySpawnStart+0x2C); entry[0] is the petal
 * texture (0 -> hide the petal at w+0x5EC + i*0x3C, else show + set it), func_00337DC8
 * (entry[0x18]) yields three colors (petal RGB + the paired GuiList's colour pair),
 * and the list element (func_0034F300(g_guiInstance+0x36F28) + i*0x48) is shown with
 * meter max 0x64 / value 0x64 (full). For an owned weapon (g_weaponTable[slot]
 * non-empty and either +0x6C or +4 set) the meter max becomes
 * weaponTable[slot][0x6C] and, when that is >0, its value becomes
 * g_weaponXp[id] >> 5.
 * Input: flag 0x4 = rotate to previous slot, 0x8 = next, over the 8-slot cursor at
 * w+0x7CC (wrapping, with the wheel-turn cue PlayGlobalSound(3)); otherwise the d-pad
 * is dispatched by mode w+0x81C to func_003418D8 (0) or func_00341A80 (1).
 * Finally: if D_138180[0x1C4] & 0xC pulse the confirm highlight, then refresh the
 * cursor sprite (w+0x5A0) colour (func_002AA3F0) and alpha (= (f32)cursor index).
 * Returns 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiQuickSelectWheelTick);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiQuickSelectWheelTick, unit objdiff): 46.91%,
   401/486 insns differ. Residual: UNKNOWN-addiu + movn/movz, gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x120' vs 'addiu sp, sp, -0x80').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern char *g_guiInstance;
extern u8 g_hudMobySpawnStart[];        /* +0x2C -> quick-select entry table ptr */
extern s32 g_weaponXp[];                /* itemId -> accumulated weapon XP */
extern u8 D_138180[];                   /* HUD/input state; +0x1C4 = button bits */
extern f32 D_1AE020[2], D_1AE028[2], D_1AE030[2], D_1AE038[2], D_1AE040[2], D_1AE048[2];
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);
extern char *func_0034F300(char *ctx);              /* -> quick-select GuiList array base */
extern void func_00337DC8(s32 itemId, s32 *petalRgb, s32 *listC0, s32 *listC1);
void func_003418D8(void *w, s32 flags);             /* grid d-pad handler (mode 0) */
void func_00341A80(void *w, s32 flags);             /* grid d-pad handler (mode 1) */

s32 GuiQuickSelectWheelTick(void *w, s32 flag) {
    f32 *base = *(f32 **)((char *)w + 0x80C);
    void *tbl;
    s32 i;

    GuiElementSetPos((GuiElement *)((char *)w + 0x0),   base[0], base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x4C),  base[0], base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0xE4),  base[0], base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x130), base[0], base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x828), D_1AE020[0] + base[0], D_1AE020[1] + base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x8D8), D_1AE030[0] + base[0], D_1AE030[1] + base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x930), D_1AE028[0] + base[0], D_1AE028[1] + base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x988), D_1AE038[0] + base[0], D_1AE038[1] + base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x9E0), D_1AE040[0] + base[0], D_1AE040[1] + base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x2A8), D_1AE048[0] + base[0], D_1AE048[1] + base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x2F4), D_1AE048[0] + base[0], D_1AE048[1] + base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x5A0), D_1AE048[0] + base[0], D_1AE048[1] + base[1], 0.0f, 0.0f);

    tbl = *(void **)(g_hudMobySpawnStart + 0x2C);
    if (tbl != 0) {
        char *petal = (char *)w + 0x5EC;
        for (i = 0; i < 8; i++, petal += 0x3C) {
            char *entry = *(char **)tbl + i * 0x1C;
            s32 itemId = *(s32 *)(entry + 0x18);
            s32 *color;
            char *list;
            s32 petalRgb, listC0, listC1;
            s32 slot;

            if (*(s32 *)(entry + 0) == 0) {
                GuiElementSetVisible((GuiElement *)petal, 0);
            } else {
                GuiElementSetVisible((GuiElement *)petal, 1);
                GuiSpriteSetTexture((GuiElement *)petal, *(s32 *)(entry + 0), 0);
            }

            list = func_0034F300(g_guiInstance + 0x36F28) + i * 0x48;
            GuiElementSetVisible((GuiElement *)list, 0);

            petalRgb = listC0 = listC1 = 0;
            func_00337DC8(itemId, &petalRgb, &listC0, &listC1);

            color = GuiElementGetColor((GuiElement *)petal);
            *color = (*color & (s32)0xFF000000) | (petalRgb & 0x00FFFFFF);

            list = func_0034F300(g_guiInstance + 0x36F28) + i * 0x48;
            GuiElementSetVisible((GuiElement *)list, 1);
            GuiListSetColorPair0((GuiElement *)list, listC0, listC1);
            GuiListSetItemCount((GuiElement *)list, 0x64);
            GuiListSetScrollPos((GuiElement *)list, 0x64);

            slot = g_itemEquippedSlot[itemId];
            if (*(s32 *)&g_weaponTable[slot * 0xE0 + 0] == 0)
                continue;
            if (*(s32 *)&g_weaponTable[slot * 0xE0 + 0x6C] == 0 &&
                g_weaponTable[slot * 0xE0 + 4] == 0)
                continue;

            GuiListSetItemCount((GuiElement *)list, *(s32 *)&g_weaponTable[slot * 0xE0 + 0x6C]);

            color = GuiElementGetColor((GuiElement *)petal);
            *color = (*color & (s32)0xFF000000) | (petalRgb & 0x00FFFFFF);

            slot = g_itemEquippedSlot[itemId];
            if (*(s32 *)&g_weaponTable[slot * 0xE0 + 0x6C] <= 0)
                continue;

            GuiListSetScrollPos((GuiElement *)list, g_weaponXp[itemId] >> 5);
        }
    }

    if (flag & 0x4) {                    /* rotate to previous slot, wrap 0 -> 7 */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x7CC) - 1;
        *(s32 *)((char *)w + 0x7CC) = (v >= 0) ? v : 7;
    } else if (flag & 0x8) {             /* rotate to next slot, wrap 7 -> 0 */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x7CC) + 1;
        *(s32 *)((char *)w + 0x7CC) = (v > 7) ? 0 : v;
    } else {
        switch (*(s32 *)((char *)w + 0x81C)) {
        case 0:
            func_003418D8(w, flag);
            break;
        case 1:
            func_00341A80(w, flag);
            break;
        default:
            break;
        }
    }

    if (*(s32 *)&D_138180[0x1C4] & 0xC) {
        func_002AA3F0(0, 0, 1, 0, 1);
    }

    {
        s32 *cursorColor = GuiElementGetColor((GuiElement *)((char *)w + 0x5A0));
        *cursorColor = func_002AA3F0(0x60442D00, 0x70FFFEED, 0x14, 0, 0);
    }
    GuiElementSetAlpha((GuiElement *)((char *)w + 0x5A0), (f32)*(s32 *)((char *)w + 0x7CC));
    return 0;
}
#endif

/* func_00341160(self): render the quick-select / weapon-grid screen. No-op unless
 * self[+0xAB8] is set. Any of the four shoulder buttons (g_padButtonsPressed &
 * 0xF000) ticks the shared blend counter (func_002AA3F0(0,0,1,1,1)). Recolours the
 * grid cursor (self+0x214) with a pulsing tint, draws the frame sprites + the five
 * text labels (self+0x828..+0x9E0), hides self+0x880, then (re)builds the two grid
 * descriptors (func_00341548 + func_003413A8) and draws the two arrow sprites
 * (self+0x2A8/+0x2F4) twice — once mirrored (scale negated) and once restored. Then
 * it dispatches each of the 8 weapon-grid cells (descriptor table self+0x370 stride
 * 0x4C, element base self+0x340) and each of the 8 quick-select slots (table
 * self+0x61C stride 0x3C, base self+0x5EC) through the cell object's draw method
 * (obj+0xC, arg = base + obj[+8]). Finally draws the highlight sprite (self+0x5A0,
 * colour 0x80FFDE8D) and blits the cursor glyph (func_0034F028). Faithful
 * TARGET_NATIVE #else (engine 2.96 = no byte-match). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341160);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern char *g_guiInstance;
/* (end of this body's declarations) */
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00341160, unit objdiff): 61.92%,
   112/167 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x50' vs 'addiu sp, sp, -0x30').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_0034F028(s32 atlas, s32 x, s32 y, s32 code);
void func_003413A8(void *self);
void func_00341548(void *self);
void func_00341160(void *self) {
    u8 *s = (u8 *)self;
    f32 *sc;

    if (*(s32 *)(s + 0xAB8) == 0) {
        return;
    }

    if (g_padButtonsPressed & 0xF000) {
        func_002AA3F0(0, 0, 1, 1, 1);
    }

    *GuiElementGetColor((GuiElement *)(s + 0x214)) =
        func_002AA3F0(0x60442D00, 0x70FFFEED, 0x14, 1, 0);
    GuiSpriteElementDraw(s);
    GuiSpriteElementDraw(s + 0x4C);
    GuiSpriteElementDraw(s + 0xE4);
    GuiSpriteElementDraw(s + 0x130);
    GuiTextElementDraw(s + 0x828);
    GuiTextElementDraw(s + 0x8D8);
    GuiTextElementDraw(s + 0x930);
    GuiTextElementDraw(s + 0x988);
    GuiTextElementDraw(s + 0x9E0);
    GuiElementSetVisible((GuiElement *)(s + 0x880), 0);
    func_00341548(self);
    func_003413A8(self);

    GuiSpriteElementDraw(s + 0x214);
    GuiSpriteElementDraw(s + 0x2A8);
    GuiSpriteElementDraw(s + 0x2F4);
    sc = GuiElementGetScaleVec((GuiElement *)(s + 0x2A8)); *sc = -*sc;
    sc = GuiElementGetScaleVec((GuiElement *)(s + 0x2F4)); *sc = -*sc;
    GuiSpriteElementDraw(s + 0x2A8);
    GuiSpriteElementDraw(s + 0x2F4);
    sc = GuiElementGetScaleVec((GuiElement *)(s + 0x2A8)); *sc = -*sc;
    sc = GuiElementGetScaleVec((GuiElement *)(s + 0x2F4)); *sc = -*sc;

    {
        u8 *desc = s + 0x370;
        u8 *elem = s + 0x340;
        s32 n = 7;
        do {
            u8 *obj = *(u8 **)desc;
            n--;
            desc += 0x4C;
            (*(void (**)(void *))(obj + 0xC))(elem + *(s16 *)(obj + 8));
            elem += 0x4C;
        } while (n > -1);
    }
    {
        u8 *desc = s + 0x61C;
        u8 *elem = s + 0x5EC;
        s32 n = 7;
        do {
            u8 *obj = *(u8 **)desc;
            n--;
            desc += 0x3C;
            (*(void (**)(void *))(obj + 0xC))(elem + *(s16 *)(obj + 8));
            elem += 0x3C;
        } while (n > -1);
    }

    GuiSpriteElementDraw(s + 0x5A0);
    *GuiElementGetColor((GuiElement *)(s + 0x5A0)) = (s32)0x80FFDE8D;
    func_0034F028((s32)g_guiInstance + 0x36F28, 0xF3, 0x2B, 0x70);
}
#endif

/* func_003413A8(self): build + populate a composite GUI screen descriptor. Copies
 * four 8-byte layout config blocks (D_1AE050/58/050/60/68) to locals, sets a glyph
 * on the self+0x17C element (GuiElementSetGlyph, source = g_guiInstance+0x8710),
 * then assembles a ~0x60-byte parameter block (pointers to the config locals + to
 * self sub-elements at +0x17C/+0x98/+0x214/+0x260/+0x7D0/+0xA78/+0x880, plus fields
 * from self+0x814/+0x818/+0x80C/+0x820 and the self[+0x81C]==0 flag) and hands it
 * to func_003380B8 to instantiate the elements. Struct layout Ghidra-verified.
 * Faithful TARGET_NATIVE #else (engine 2.96 = no byte-match). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003413A8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003413A8, unit objdiff): 23.67%,
   127/129 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0xf0' vs 'addiu sp, sp, -0xa0').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8   D_1AE050[], D_1AE058[], D_1AE060[], D_1AE068[];
extern u8   D_00259F38[];
extern char *g_guiInstance;
extern void func_003380B8(void *params);

void func_003413A8(void *self) {
    u8 *s = (u8 *)self;
    u8  cfg[5][8];
    u8  params[0x60];

    memcpy(cfg[0], D_1AE050, 8);
    memcpy(cfg[1], D_1AE058, 8);
    memcpy(cfg[2], D_1AE050, 8);
    memcpy(cfg[3], D_1AE060, 8);
    memcpy(cfg[4], D_1AE068, 8);

    *(void **)(params + 0x00) = D_00259F38;
    *(u32 *)(params + 0x04)  = 4;
    *(u32 *)(params + 0x08)  = 6;
    *(u32 *)(params + 0x0C)  = *(u32 *)(s + 0x814);
    *(u32 *)(params + 0x10)  = *(u32 *)(s + 0x818);
    *(void **)(params + 0x14) = cfg[0];
    *(void **)(params + 0x18) = cfg[1];
    *(void **)(params + 0x1C) = cfg[2];
    *(void **)(params + 0x20) = cfg[3];
    *(void **)(params + 0x24) = cfg[4];
    *(void **)(params + 0x28) = s + 0x17C;
    *(void **)(params + 0x2C) = s + 0x98;
    *(void **)(params + 0x30) = s + 0x214;
    *(void **)(params + 0x34) = s + 0x260;
    *(u32 *)(params + 0x38)  = 0;
    *(void **)(params + 0x3C) = s + 0x7D0;
    *(void **)(params + 0x40) = s + 0xA78;
    *(void **)(params + 0x44) = s + 0x880;
    *(u32 *)(params + 0x48)  = *(u32 *)(s + 0x80C);
    *(u32 *)(params + 0x4C)  = *(u32 *)(s + 0x820);
    *(u32 *)(params + 0x50)  = 0;
    *(u32 *)(params + 0x54)  = (*(s32 *)(s + 0x81C) == 0);
    *(u32 *)(params + 0x58)  = 0;

    GuiElementSetGlyph((GuiElement *)(s + 0x17C), (s32)g_guiInstance + 0x8710, 0x23);
    func_003380B8(params);
}
#endif

/* func_00341548(self): build + populate the quick-select / weapon-grid composite
 * screen descriptor (sibling of func_003413A8). Copies six 8-byte layout config
 * blocks (D_1AE070/078/070/080/068/088) to locals, sets a glyph on the self+0x17C
 * element (GuiElementSetGlyph, source = g_guiInstance+0x8710, code 0x5D), then
 * assembles a ~0x60-byte parameter block (grid table D_259CC0, cols 3, rows 2,
 * pointers to the config locals + to self sub-elements at +0x17C/+0x214/+0x7D0/
 * +0xA78/+0x880, fields from self+0x814/+0x818/+0x80C/+0x820 and the
 * self[+0x81C]==1 flag) and hands it to func_003380B8 to instantiate the elements.
 * Struct layout asm-verified (struct base sp+0x60). Faithful TARGET_NATIVE #else
 * (engine 2.96 = no byte-match). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341548);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern char *g_guiInstance;
/* (end of this body's declarations) */
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00341548, unit objdiff): 0.00%,
   153/157 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x100' vs 'addiu sp, sp, -0xa0').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8   D_1AE068[], D_1AE070[], D_1AE078[], D_1AE080[], D_1AE088[];
extern u8   D_259CC0[];
void func_00341548(void *self) {
    u8 *s = (u8 *)self;
    u8  cfg[6][8];
    u8  params[0x60];

    memcpy(cfg[0], D_1AE070, 8);
    memcpy(cfg[1], D_1AE078, 8);
    memcpy(cfg[2], D_1AE070, 8);
    memcpy(cfg[3], D_1AE080, 8);
    memcpy(cfg[4], D_1AE068, 8);
    memcpy(cfg[5], D_1AE088, 8);

    *(void **)(params + 0x00) = D_259CC0;
    *(u32 *)(params + 0x04)  = 3;
    *(u32 *)(params + 0x08)  = 2;
    *(u32 *)(params + 0x0C)  = *(u32 *)(s + 0x814);
    *(u32 *)(params + 0x10)  = *(u32 *)(s + 0x818);
    *(void **)(params + 0x14) = cfg[0];
    *(void **)(params + 0x18) = cfg[1];
    *(void **)(params + 0x1C) = cfg[2];
    *(void **)(params + 0x20) = cfg[3];
    *(void **)(params + 0x24) = cfg[4];
    *(void **)(params + 0x28) = s + 0x17C;
    *(u32 *)(params + 0x2C)  = 0;
    *(void **)(params + 0x30) = s + 0x214;
    *(u32 *)(params + 0x34)  = 0;
    *(u32 *)(params + 0x38)  = 1;
    *(void **)(params + 0x3C) = s + 0x7D0;
    *(void **)(params + 0x40) = s + 0xA78;
    *(void **)(params + 0x44) = s + 0x880;
    *(u32 *)(params + 0x48)  = *(u32 *)(s + 0x80C);
    *(u32 *)(params + 0x4C)  = *(u32 *)(s + 0x820);
    *(void **)(params + 0x50) = cfg[5];
    *(u32 *)(params + 0x54)  = (*(s32 *)(s + 0x81C) == 1);
    *(u32 *)(params + 0x58)  = 0;

    GuiElementSetGlyph((GuiElement *)(s + 0x17C), (s32)g_guiInstance + 0x8710, 0x5D);
    func_003380B8(params);
}
#endif

/* func_00341708: assign the weapon under the grid cursor to the quick-select bar.
 * `w` is the weapon-grid widget; `table` (D_00259F38) is its grid-entry array
 * (stride 0xA, +6 = s16 itemId) and `rowStride` the grid's column count. Reads the
 * item at cell (col w+0x814, row w+0x818). If it is empty (0), un-owned
 * (g_inventoryOwned) or its equipped weapon has no icon
 * (g_weaponTable[slot]+0x3C == 0xEA7E sentinel) it just plays the reject cue
 * (PlayGlobalSound 5). Otherwise it scans the 8 quick-select slots
 * (base = *(g_hudMobySpawnStart+0x2C) then one more deref; stride 0x1C, +0x18 =
 * s32 itemId, +0 = icon): if the item already occupies a slot that slot is cleared
 * first, then the item is (re-)placed into the round-robin `next` slot with its
 * weapon icon (g_weaponTable[slot]+0x3C) and the confirm cue (PlayGlobalSound 4).
 * Every path then advances the round-robin cursor w+0x7CC (wrap 7 -> 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341708);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00341708, unit objdiff): 31.22%,
   139/146 insns differ. Residual: UNKNOWN-addiu + movn/movz, gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x10' vs 'addiu sp, sp, -0x20').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 g_hudMobySpawnStart[];        /* +0x2C -> quick-select slot-table ptr */
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);

void func_00341708(void *w, void *table, s32 rowStride) {
    s32 col = *(s32 *)((char *)w + 0x814);
    s32 row = *(s32 *)((char *)w + 0x818);
    s16 item = *(s16 *)((char *)table + (col + row * rowStride) * 0xA + 6);
    s32 cursor = *(s32 *)((char *)w + 0x7CC);

    if (item == 0 || g_inventoryOwned[item] == 0) {
        PlayGlobalSound(5, 0, 0);              /* empty cell / not owned */
    } else {
        s32 slot = g_itemEquippedSlot[item];
        if (*(u16 *)&g_weaponTable[slot * 0xE0 + 0x3C] == 0xEA7E) {
            PlayGlobalSound(5, 0, 0);          /* equipped weapon has no icon */
        } else {
            char *base = *(char **)(*(char **)(g_hudMobySpawnStart + 0x2C));
            s32 next = (cursor == 7) ? 0 : cursor + 1;
            s32 i, found = 8;

            for (i = 0; i < 8; i++) {          /* already in a quick-select slot? */
                if (*(s32 *)(base + i * 0x1C + 0x18) == item) {
                    found = i;
                    break;
                }
            }

            if (found < 8) {                   /* clear its old slot first */
                char *fslot = base + found * 0x1C;
                *(s32 *)(fslot + 0x18) = 0;
                /* itemId was just cleared -> re-reads 0 (slot 0's default icon) */
                *(s32 *)(fslot + 0) =
                    *(u16 *)&g_weaponTable[g_itemEquippedSlot[*(s32 *)(fslot + 0x18)] * 0xE0 + 0x3C];
            }

            {                                  /* (re-)place item into slot `next` */
                char *nslot = base + next * 0x1C;
                *(s32 *)(nslot + 0) =
                    *(u16 *)&g_weaponTable[g_itemEquippedSlot[item] * 0xE0 + 0x3C];
                *(s32 *)(nslot + 0x18) = item;
                *(s32 *)(nslot + 4) = 0;
                PlayGlobalSound(4, 0, 0);      /* confirm cue */
            }
        }
    }

    *(s32 *)((char *)w + 0x7CC) = (cursor + 1 > 7) ? 0 : cursor + 1;   /* advance cursor */
}
#endif

/* func_003418D8: d-pad + confirm handler for the two-region weapon grid `w`
 * (a 4-column x N main grid at col +0x814 / row +0x818, plus a 2-wide sub-region
 * occupying rows 4/5). `flags` (held buttons): up (0x1000) row-- wrapping to 5;
 * down (0x4000) row++ wrapping to 0; left (0x8000) col--, and on underflow drops
 * to col 3 (or, when the row is 4/5, crosses into the sub-region: col 2, row-=4,
 * region flag +0x81C=1); right (0x2000) col++, and on overflow past 4 wraps to
 * col 0 (row 4 -> row 0 + flag; row 5 -> col 1/row 1 + flag). Confirm (0x40)
 * forwards to func_00341708(w, D_00259F38, 4). Each path then refreshes the
 * selected entry +0x820 = D_00259F38[col + row*4].field_0x6. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003418D8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003418D8, unit objdiff): 51.89%,
   102/125 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0x20' vs 'addiu sp, sp, -0x10').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 D_00259F38[];
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);
extern void func_00341708(void *w, void *table, s32 mult);
void func_003418D8(void *w, s32 flags) {
    s32 v, idx;

    if (flags & 0x1000) {          /* up: row-- wrap to 5 */
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x818) - 1;
        *(s32 *)((char *)w + 0x818) = (v >= 0) ? v : 5;
    } else if (flags & 0x4000) {   /* down: row++ wrap to 0 */
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x818) + 1;
        *(s32 *)((char *)w + 0x818) = (v > 5) ? 0 : v;
    } else if (flags & 0x8000) {   /* left: col-- (may cross into sub-region) */
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x814) - 1;
        if (v >= 0) {
            *(s32 *)((char *)w + 0x814) = v;
        } else {
            s32 t = *(s32 *)((char *)w + 0x818) - 4;
            if ((u32)t < 2) {          /* rows 4/5 -> 2-wide sub-region */
                *(s32 *)((char *)w + 0x814) = 2;
                *(s32 *)((char *)w + 0x818) = t;
                *(s32 *)((char *)w + 0x81C) = 1;
            } else {
                *(s32 *)((char *)w + 0x814) = 3;
            }
        }
    } else if (flags & 0x2000) {   /* right: col++ (may wrap/cross region) */
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x814) + 1;
        if (v < 4) {
            *(s32 *)((char *)w + 0x814) = v;
        } else {
            s32 r = *(s32 *)((char *)w + 0x818);
            *(s32 *)((char *)w + 0x814) = 0;
            if (r == 4) {
                *(s32 *)((char *)w + 0x818) = 0;
                *(s32 *)((char *)w + 0x81C) = 1;
            } else if (r == 5) {
                *(s32 *)((char *)w + 0x81C) = 1;
                *(s32 *)((char *)w + 0x814) = 1;
                *(s32 *)((char *)w + 0x818) = 1;
            }
        }
    } else if (flags & 0x40) {     /* confirm */
        func_00341708(w, D_00259F38, 4);
    }

    /* tail (all paths): result entry from the grid table */
    idx = *(s32 *)((char *)w + 0x814) + *(s32 *)((char *)w + 0x818) * 4;
    *(s32 *)((char *)w + 0x820) = *(s16 *)&D_00259F38[idx * 0xA + 0x6];
}
#endif

/* func_00341A80: d-pad + confirm handler for the 3-column weapon grid `w` (sibling
 * of func_003418D8; col +0x814 / row +0x818, table D_259CC0 stride 0xA, idx =
 * col + row*3). up (0x1000) row-- wrapping to 1, but column 0 is single-row so it
 * pins row 0; down (0x4000) row++ wrapping to 0, column 0 pins row 0; left
 * (0x8000) col--, and on underflow (or leaving column 0 from row 1) crosses to
 * col 3 with row += 4 and clears the region flag +0x81C; right (0x2000) col++,
 * and past column 2 wraps to col 0 with row += 4 and flag cleared. Confirm (0x40)
 * forwards to func_00341708(w, D_259CC0, 3). Each path then refreshes +0x820 =
 * D_259CC0[col + row*3].field_0x6. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341A80);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00341A80, unit objdiff): 48.39%,
   97/121 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0x20' vs 'addiu sp, sp, -0x10').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 D_259CC0[];
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);
extern void func_00341708(void *w, void *table, s32 mult);
void func_00341A80(void *w, s32 flags) {
    s32 v, idx;

    if (flags & 0x1000) {          /* up: row-- (column 0 pins row 0) */
        s32 col = *(s32 *)((char *)w + 0x814);
        v = *(s32 *)((char *)w + 0x818) - 1;
        v = (v >= 0) ? v : 1;
        if (col == 0) {
            v = 0;
        }
        *(s32 *)((char *)w + 0x818) = v;
        PlayGlobalSound(3, 0, 0);
    } else if (flags & 0x4000) {   /* down: row++ wrap to 0 (column 0 pins row 0) */
        s32 col = *(s32 *)((char *)w + 0x814);
        v = *(s32 *)((char *)w + 0x818) + 1;
        v = (v >= 2) ? 0 : v;
        if (col == 0) {
            v = 0;
        }
        *(s32 *)((char *)w + 0x818) = v;
        PlayGlobalSound(3, 0, 0);
    } else if (flags & 0x8000) {   /* left: col-- (may cross to the +4 row region) */
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x814) - 1;
        if (v < 0 || (v == 0 && *(s32 *)((char *)w + 0x818) == 1)) {
            *(s32 *)((char *)w + 0x814) = 3;
            *(s32 *)((char *)w + 0x818) = *(s32 *)((char *)w + 0x818) + 4;
            *(s32 *)((char *)w + 0x81C) = 0;
        } else {
            *(s32 *)((char *)w + 0x814) = v;
        }
    } else if (flags & 0x2000) {   /* right: col++ (may cross to the +4 row region) */
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x814) + 1;
        if (v < 3) {
            *(s32 *)((char *)w + 0x814) = v;
        } else {
            *(s32 *)((char *)w + 0x814) = 0;
            *(s32 *)((char *)w + 0x818) = *(s32 *)((char *)w + 0x818) + 4;
            *(s32 *)((char *)w + 0x81C) = 0;
        }
    } else if (flags & 0x40) {     /* confirm */
        func_00341708(w, D_259CC0, 3);
    }

    idx = *(s32 *)((char *)w + 0x814) + *(s32 *)((char *)w + 0x818) * 3;
    *(s32 *)((char *)w + 0x820) = *(s16 *)&D_259CC0[idx * 0xA + 0x6];
}
#endif

/* func_00341C28: init the screen's six embedded sub-elements at their fixed
 * offsets (two type-B, one type-C, one type-B, a list-row, and the D_1ADA18
 * widget), return the object. */
#if defined(S136OS_func_00341C28) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_00341C28; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 20/20 words + 6 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_00341C28 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_00341C28(void *p) {
    GuiElementInitTypeB(p);
    GuiElementInitTypeB((char *)p + 0x4C);
    GuiElementInitTypeC((char *)p + 0x98);
    GuiElementInitTypeB((char *)p + 0xF0);
    GuiListRowElementInit((char *)p + 0x13C);
    func_003374D8((char *)p + 0x184);
    return p;
}
#else
S136OS_SLOT(func_00341C28);
#endif

/* GuiScrollListScreenInit: construct a scroll-list screen (a header button, two
 * navigation buttons, a title text, a sprite, and the scrolling list itself).
 * Inits the header element (+0xF0/D_1ADF78, colour 0x60241700, glyph 0x29) up
 * front, then (pool -> alloc the 0x10-byte placement record at +0x1C0, +0x1C4=pool
 * unconditional, +0x210=1) seeds two button elements (+0x0/D_1ADF98,
 * +0x4C/D_1ADFA0), a title (+0x98/D_1AE008), a sprite (+0x184/D_1ADFB0), colours
 * and glyphs them (0x5D/0x5E; +0x4C hidden), scales the sprite 32x32, sets the
 * title text buffer (+0x1D0), and builds the list at +0x13C:
 * GuiListElementInit(0x20 px wide, tag D_1AE018), 5 px tall (SetVisibleRows),
 * meter max 100 (SetItemCount), value 0, colour pairs (0x6049C1FF/0x60001EFF, 0x50F0C070 x2), +0x44=0x80000000
 * +0x38=0. Finally records +0x220=6 / +0x21C=4 and runs func_00341F40(w,0,0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiScrollListScreenInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiScrollListScreenInit, unit objdiff): 67.42%,
   114/197 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s4, 0x20(sp)' vs 'sd s1, 0x8(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00341F40(void *w, s32 a, s32 b);
extern char *g_guiInstance;
extern u8 D_1ADF78[], D_1ADF98[], D_1ADFA0[], D_1AE008[], D_1ADFB0[], D_1AE018[];
void GuiScrollListScreenInit(void *w, GuiPool *pool) {
    GuiElement *header = (GuiElement *)((char *)w + 0xF0);
    GuiElement *btn0 = (GuiElement *)((char *)w + 0x0);
    GuiElement *btn1 = (GuiElement *)((char *)w + 0x4C);
    GuiElement *title = (GuiElement *)((char *)w + 0x98);
    GuiElement *sprite = (GuiElement *)((char *)w + 0x184);
    GuiElement *list = (GuiElement *)((char *)w + 0x13C);
    void *rec;

    GuiElementInit(header, (s32)D_1ADF78, pool);
    *GuiElementGetColor(header) = 0x60241700;
    GuiElementSetGlyph(header, (s32)(g_guiInstance + 0x8710), 0x29);

    /* +0x1C4 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x1C4) = pool;
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x1C0) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    *(s32 *)((char *)w + 0x210) = 1;
    rec = *(void **)((char *)w + 0x1C0);
    *(s32 *)((char *)rec + 0x0) = 0;
    *(s32 *)((char *)rec + 0x4) = 0;

    GuiElementInit(btn0, (s32)D_1ADF98, pool);
    GuiElementInit(btn1, (s32)D_1ADFA0, pool);
    GuiTextElementInit(title, (s32)D_1AE008, pool);

    *GuiElementGetColor(btn0) = 0x55F0C070;
    *GuiElementGetColor(btn1) = 0x70FFFEED;
    GuiElementSetGlyph(btn0, (s32)(g_guiInstance + 0x8710), 0x5D);
    GuiElementSetGlyph(btn1, (s32)(g_guiInstance + 0x8710), 0x5E);
    GuiElementSetVisible(btn1, 0);

    *(s32 *)((char *)w + 0x214) = 0;
    *(s32 *)((char *)w + 0x218) = 0;
    GuiSpriteElementInit(sprite, (s32)D_1ADFB0, pool);
    *GuiElementGetColor(sprite) = 0x60F0F0B0;
    GuiElementSetScale(sprite, 32.0f, 32.0f, 0.0f, 0.0f);

    *GuiElementGetColor(title) = (s32)0x80F0F0F0;
    *(u8 *)((char *)w + 0x1D0) = 0;
    GuiElementSetText(title, (s32)((char *)w + 0x1D0));
    *(f32 *)((char *)w + 0x1CC) = 0.0f;
    *(f32 *)((char *)w + 0x1C8) = 0.0f;
    *(s32 *)((char *)w + 0x224) = 0;

    GuiListElementInit(list, 0x20, 0, (s32)D_1AE018, pool);
    GuiListSetVisibleRows(list, 5);
    GuiListSetItemCount(list, 0x64);
    GuiListSetScrollPos(list, 0);
    GuiListSetColorPair0(list, 0x6049C1FF, 0x60001EFF);
    GuiListSetColorPair1(list, 0x50F0C070, 0x50F0C070);
    func_00337B88(list, (s32)0x80000000);
    func_00337B68(list, 0);

    *(s32 *)((char *)w + 0x220) = 6;
    *(s32 *)((char *)w + 0x21C) = 4;
    func_00341F40(w, 0, 0);
}
#endif

/* func_00341F40: 2D grid-cursor move + selection readout. `flags` selects a
 * direction (0x1000 up / 0x4000 down on the row axis +0x218 wrapping mod
 * rowCount +0x220; 0x8000 left / 0x2000 right on the col axis +0x214 wrapping mod
 * colCount +0x21C), each accompanied by the cursor sound (PlayGlobalSound(3,0,0)).
 * Then re-positions the cursor element (*(w+0x1C0)) and, when `table` is non-null,
 * records the s16 at table[col + row*colCount].field_0x6 into +0x224. Called with
 * table==0 at init (a no-op early-out). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00341F40);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00341F40, unit objdiff): 75.61%,
   53/102 insns differ. Residual: UNKNOWN-sd + movn/movz (first differing insn: 'sd s0, 0x10(sp)' vs 'sd s2, 0x20(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00341F40(void *w, s32 flags, s32 table) {
    f32 offset[2];
    GuiElement *cursor;
    s32 col, row, idx;

    /* zeroed 8-byte (Vec2) position offset, added to the cursor element's pos */
    memset(offset, 0, 8);
    if (table == 0) {
        return;
    }

    *(s32 *)((char *)w + 0x224) = 0;
    cursor = *(GuiElement **)((char *)w + 0x1C0);
    GuiElementSetPos((GuiElement *)w,
                     offset[0] + *(f32 *)((char *)cursor + 0x0),
                     offset[1] + *(f32 *)((char *)cursor + 0x4),
                     0.0f, 0.0f);

    if (flags & 0x1000) {                       /* up: row-- wrap to rowCount-1 */
        s32 cur;
        PlayGlobalSound(3, 0, 0);
        cur = *(s32 *)((char *)w + 0x218) - 1;
        if (cur < 0) {
            cur = *(s32 *)((char *)w + 0x220) - 1;
        }
        *(s32 *)((char *)w + 0x218) = cur;
    } else if (flags & 0x4000) {                /* down: row++ wrap to 0 */
        s32 cur;
        PlayGlobalSound(3, 0, 0);
        cur = *(s32 *)((char *)w + 0x218) + 1;
        if (cur >= *(s32 *)((char *)w + 0x220)) {
            cur = 0;
        }
        *(s32 *)((char *)w + 0x218) = cur;
    } else if (flags & 0x8000) {                /* left: col-- wrap to colCount-1 */
        s32 cur;
        PlayGlobalSound(3, 0, 0);
        cur = *(s32 *)((char *)w + 0x214) - 1;
        if (cur < 0) {
            cur = *(s32 *)((char *)w + 0x21C) - 1;
        }
        *(s32 *)((char *)w + 0x214) = cur;
    } else if (flags & 0x2000) {                /* right: col++ wrap to 0 */
        s32 cur;
        PlayGlobalSound(3, 0, 0);
        cur = *(s32 *)((char *)w + 0x214) + 1;
        if (cur >= *(s32 *)((char *)w + 0x21C)) {
            cur = 0;
        }
        *(s32 *)((char *)w + 0x214) = cur;
    }

    row = *(s32 *)((char *)w + 0x218);
    col = *(s32 *)((char *)w + 0x214);
    idx = col + row * *(s32 *)((char *)w + 0x21C);
    *(s32 *)((char *)w + 0x224) =
        *(s16 *)((char *)table + idx * 0xA + 0x6);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003420C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003420D0);

/* GuiIconScreenInit: construct an icon screen (five button-glyph elements, a
 * text row, and a sprite). Pool -> alloc the 0x10-byte placement record (+0x228)
 * and zero it (the +0x22C pool store + the +0x230=1 flag are unconditional).
 * Record seeded 251x170. Init five elements (+0x0/D_1ADBE8, +0x98/D_1ADBF0,
 * +0x4C/D_1ADBF8, +0xE4/D_1ADC00, +0x130/D_1ADF98), the text row (+0x1D0/D_1ADC08),
 * the sprite (+0x17C/D_1AE098), and the sub-block (+0x238) via func_00348BF8(pool).
 * Colour all seven, alpha three buttons (+0x0/+0x4C/+0xE4) to 1.0, assign glyphs
 * 0x91/0x94/0x92/0x93/0x95, scale the label 1.0x1.05, set its text (localized
 * 0x2BEB), texture the sprite (0xEA9D) scaled 32x32, and clear the +0x1B8 block. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiIconScreenInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiIconScreenInit, unit objdiff): 61.77%,
   148/228 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x70' vs 'addiu sp, sp, -0x60').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00348BF8(void *w, void *pool);
extern char *g_guiInstance;
extern u8 D_1ADBE8[], D_1ADBF0[], D_1ADBF8[], D_1ADC00[], D_1ADF98[];
extern u8 D_1ADC08[], D_1AE098[];
void GuiIconScreenInit(void *w, GuiPool *pool) {
    GuiElement *e0 = (GuiElement *)((char *)w + 0x0);
    GuiElement *e1 = (GuiElement *)((char *)w + 0x98);
    GuiElement *e2 = (GuiElement *)((char *)w + 0x4C);
    GuiElement *e3 = (GuiElement *)((char *)w + 0xE4);
    GuiElement *e4 = (GuiElement *)((char *)w + 0x130);
    GuiElement *text = (GuiElement *)((char *)w + 0x1D0);
    GuiElement *sprite = (GuiElement *)((char *)w + 0x17C);
    void *sub = (char *)w + 0x238;
    void *rec;

    /* +0x22C = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x22C) = pool;
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x228) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    *(s32 *)((char *)w + 0x230) = 1;
    rec = *(void **)((char *)w + 0x228);
    *(f32 *)((char *)rec + 0x0) = 251.0f;
    *(f32 *)((char *)rec + 0x4) = 170.0f;

    GuiElementInit(e0, (s32)D_1ADBE8, pool);
    GuiElementInit(e1, (s32)D_1ADBF0, pool);
    GuiElementInit(e2, (s32)D_1ADBF8, pool);
    GuiElementInit(e3, (s32)D_1ADC00, pool);
    GuiElementInit(e4, (s32)D_1ADF98, pool);
    GuiTextElementInit(text, (s32)D_1ADC08, pool);
    GuiSpriteElementInit(sprite, (s32)D_1AE098, pool);
    func_00348BF8(sub, pool);

    *GuiElementGetColor(e0) = 0x60442D00;
    *GuiElementGetColor(e1) = (s32)0x80FFDE8D;
    *GuiElementGetColor(e2) = 0x55F0C070;
    *GuiElementGetColor(e3) = 0x55F0C070;
    *GuiElementGetColor(e4) = 0x70FFFEED;
    *GuiElementGetColor(text) = (s32)0x80F0F0F0;
    *GuiElementGetColor(sprite) = 0x60F0F0B0;

    GuiElementSetAlpha(e0, 1.0f);
    GuiElementSetAlpha(e2, 1.0f);
    GuiElementSetAlpha(e3, 1.0f);

    GuiElementSetGlyph(e0, (s32)(g_guiInstance + 0x8710), 0x91);
    GuiElementSetGlyph(e1, (s32)(g_guiInstance + 0x8710), 0x94);
    GuiElementSetGlyph(e2, (s32)(g_guiInstance + 0x8710), 0x92);
    GuiElementSetGlyph(e3, (s32)(g_guiInstance + 0x8710), 0x93);
    GuiElementSetGlyph(e4, (s32)(g_guiInstance + 0x8710), 0x95);

    GuiElementSetScale(e4, 1.0f, 1.05f, 0.0f, 0.0f);
    GuiElementSetText(text, GetLocalizedString(0x2BEB));
    GuiSpriteSetTexture(sprite, 0xEA9D, 0);
    memset((char *)w + 0x1B8, 0, 0xC);
    GuiElementSetScale(sprite, 32.0f, 32.0f, 0.0f, 0.0f);
}
#endif

/* func_00342450: write three ints at +0x318/+0x310/+0x314 in that source order. */
#if defined(MATCH_func_00342450) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_00342450; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 4/4 words + 0 relocs against the ROM.
 * The INCLUDE_ASM below still feeds the 2.9 link in build.sh, which defines no MATCH_. */
void func_00342450(void *p, s32 a, s32 b, s32 c) {
    *(s32 *)((char *)p + 0x318) = c;
    *(s32 *)((char *)p + 0x310) = a;
    *(s32 *)((char *)p + 0x314) = b;
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342450);
#endif

/* func_00342460: store an int at +0x230. */
void func_00342460(void *p, s32 v) {
    *(s32 *)((char *)p + 0x230) = v;
}

/* func_00342468: look up an entry from the D_265250 table for column
 * sel=*(p+0x31C): table[sel*8 + count] where count=*(p+sel*4+0x1B8). For the
 * special column 2, when the extras flag is clear and the +0x1C0 counter has
 * reached 3+, the entry is suppressed (returns 0). */
/*
 * Byte-exact on sdk29 (task #948). The table is 8 entries per column, and the
 * ROM computes `table + count*4 + sel*32` with `p` as the FIRST addu operand of
 * `p + sel*4`: indexing a [][8] table by (sel, counts[sel]) gives exactly that.
 * The movz and the absolute table address were never walls.
 */
extern s32 D_265250[][8];
extern u8 g_miscExtras;
s32 func_00342468(void *p) {
    s32 sel = *(s32 *)((char *)p + 0x31C);
    s32 *counts = (s32 *)((char *)p + 0x1B8);
    s32 result = D_265250[sel][counts[sel]];
    if (sel == 2 && g_miscExtras == 0 && *(s32 *)((char *)p + 0x1C0) >= 3) {
        result = 0;
    }
    return result;
}

/* func_003424C8(screen): one-past-the-end row address for the currently
 * selected column of a multi-column screen. The selected column index lives at
 * +0x31C; each column has a record base pointer (+0x310 / +0x314 / +0x318) and
 * a parallel row count (+0x1B8 / +0x1BC / +sel*4+0x1B8). Returns
 * base + count * 0x14 (0x14 = the record stride). Columns 0 and 1 are spelled
 * out because the ROM special-cases them with constant offsets; any other
 * index goes through the parallel +0x1B8 array.
 *
 * Load-bearing for the byte match: `sel * 4` is computed into its own
 * statement temp rather than written inline in the address expression. Folded
 * back inline, cc1 2.9's expander evaluates the complex operand first and
 * emits `addu $3,$3,$4` where the ROM has `addu $3,$4,$3` — the operand-order
 * (expand-swap) tax, and the only word that differed before the hoist.
 * (Identity: func_003424C8, 0x003424C8; name frozen by the .s glabel.) */
s32 func_003424C8(void *screen) {
    s32 sel = *(s32 *)((char *)screen + 0x31C);
    s32 selOfs;
    if (sel == 0) {
        return *(s32 *)((char *)screen + 0x310) +
               *(s32 *)((char *)screen + 0x1B8) * 0x14;
    }
    if (sel == 1) {
        return *(s32 *)((char *)screen + 0x314) +
               *(s32 *)((char *)screen + 0x1BC) * 0x14;
    }
    selOfs = sel * 4;
    return *(s32 *)((char *)screen + 0x318) +
           *(s32 *)((char *)screen + selOfs + 0x1B8) * 0x14;
}

/* func_00342520: select active column `mode` (0..2) on the multi-column screen
 * `w`. Loads the column's config value (D_1AE0A0[mode] -> +0x1C4) and table row
 * (&D_2652B0[mode*0x20] -> +0x1C8), sets the column icon glyph (+0x1CC: 7/8/6),
 * (re)inits the row widget at +0x238 with the column's item count
 * (*(w+mode*4+0x1B8)) and its record pointer (*(w+0x310/314/318)), sets the
 * caption text (+0x1D0) to the column's localized string (0x2BEB/0x2BEC/0x3094)
 * and the three element alphas (w, +0x4C, +0xE4) to mode's value (0/1/2), and
 * records the selection at +0x31C. mode>=3 (unsigned) is a no-op. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342520);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00342520, unit objdiff): 14.15%,
   103/112 insns differ. Residual: UNKNOWN-addiu + movn/movz, gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x30' vs 'addiu sp, sp, -0x40').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern s32 D_1AE0A0[2];
extern u8 D_2652B0[];
extern void func_00348E50(void *w, s32 v);
void func_00342520(void *w, s32 mode) {
    f32 alpha;
    s32 iconId, strId;
    void *records;

    if ((u32)mode >= 3) {
        return;   /* only columns 0, 1, 2 */
    }

    *(s32 *)((char *)w + 0x1C4) = D_1AE0A0[mode];
    *(void **)((char *)w + 0x1C8) = &D_2652B0[mode * 0x20];

    if (mode == 0) {
        iconId = 7; alpha = 0.0f; strId = 0x2BEB;
        records = *(void **)((char *)w + 0x310);
    } else if (mode == 1) {
        iconId = 8; alpha = 1.0f; strId = 0x2BEC;
        records = *(void **)((char *)w + 0x314);
    } else {   /* mode == 2 */
        iconId = 6; alpha = 2.0f; strId = 0x3094;
        records = *(void **)((char *)w + 0x318);
    }
    *(s32 *)((char *)w + 0x1CC) = iconId;

    func_00348E50((char *)w + 0x238, *(s32 *)((char *)w + mode * 4 + 0x1B8));
    GuiMenuListSetRows((char *)w + 0x238, records);

    GuiElementSetText((GuiElement *)((char *)w + 0x1D0), GetLocalizedString(strId));

    GuiElementSetAlpha((GuiElement *)w, alpha);
    GuiElementSetAlpha((GuiElement *)((char *)w + 0x4C), alpha);
    GuiElementSetAlpha((GuiElement *)((char *)w + 0xE4), alpha);

    *(s32 *)((char *)w + 0x31C) = mode;
}
#endif

extern f32 D_1AE0B0, D_1AE0B4, D_1AE0B8;

/**
 * Position the embedded element at p+0x130 for the selected column, relative
 * to the anchor vector *(p+0x228): with sel = *(s32 *)(p+0x31C) and
 * col = *(s32 *)(p+0x1B8+sel*4), x = D_1AE0B0 + anchor[0],
 * y = D_1AE0B4 + D_1AE0B8 * (float)col + anchor[1], z = w = 0.
 *
 * p: owning widget. No return value.
 *
 * Matched byte-exact on sdk29 (cc1 2.9, -O2 -G8 -fno-gcse -fno-strict-aliasing,
 * task #858). The empty asm after GuiElementSetPos suppresses cc1's sibling
 * call so the ROM's jal + frame is reproduced; without it the arm is 57.12%
 * and four words short. The column slot is formed as p + sel*4 FIRST
 * (`addu v1,v1,v0`); indexing from p+0x1B8 instead gives `addu v0,v0,v1` and a
 * different load order (86.92%, 6/26 words).
 */
void func_00342670(void *p) {
    char *col = (char *)p;
    f32 *anchor;
    f32 colValue;

    col += *(s32 *)((char *)p + 0x31C) * 4;
    anchor = *(f32 **)((char *)p + 0x228);
    colValue = (f32)*(s32 *)(col + 0x1B8);
    GuiElementSetPos((GuiElement *)((char *)p + 0x130), D_1AE0B0 + anchor[0],
                     (D_1AE0B4 + D_1AE0B8 * colValue) + anchor[1], 0.0f, 0.0f);
    __asm__ __volatile__("");
}

/* func_003426D8: no-op stub (empty body - registered/overridable hook). Its
 * caller func_003427D0 passes the widget in $a0 (ROM 0x342928), so the hook
 * takes it; the empty body ignores it. */
void func_003426D8(void *w) {
}

/* Update a selectable list widget: position its element (+0x238) at the tracked
 * anchor (*(w+0x228)) offset by D_1AE0C0/C4, apply the size params D_1AE0C8/CC,
 * advance its selection by the input mask, then record the element's new state
 * into the per-row table (w+0x1B8)[w+0x31C] and pulse the colour when it changed. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003426E0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003426E0, unit objdiff): 75.29%,
   42/65 insns differ. Residual: UNKNOWN-lwc1 (first differing insn: 'lwc1 fv0f, %gp_rel(D_1AE0C0)(gp)' vs 'lwc1 fa0, %gp_rel(D_1AE0C0)(gp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_003426E0(void *w, s32 inputMask) {
    void *sub = (char *)w + 0x238;
    s32 *table = (s32 *)((char *)w + 0x1B8);
    f32 *anchor = *(f32 **)((char *)w + 0x228);
    s32 idx;
    s32 oldVal;

    GuiMenuListSetOrigin(sub, anchor[0] + D_1AE0C0, anchor[1] + D_1AE0C4);
    func_00348E10(sub, (s32)D_1AE0C8, (s32)D_1AE0CC);
    GuiMenuListHandleInput(sub, inputMask);
    idx = *(s32 *)((char *)w + 0x31C);
    oldVal = table[idx];
    table[idx] = func_00348E60(sub);
    idx = *(s32 *)((char *)w + 0x31C);
    if (oldVal != table[idx]) {
        func_002AA3F0(0, 0, 1, 0, 1);
    }
}
#endif

/* Lay out + refresh a composite list widget: position its five sub-elements
 * (root, +0x98, +0x4C, +0xE4, +0x1D0) at the tracked anchor (*(w+0x228)) each
 * offset by its own D_1AE0Dx/Ex/Fx pair; set the highlight sprite's (+0x130)
 * colour to the pulsed blend; run the row updater (func_003426E0), the selected-
 * row positioner (func_00342670) and a no-op hook (func_003426D8, passed w); return the current
 * row's recorded state ((w+0x1B8)[w+0x31C]). */
/* GUARD (task #1338): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1338 lever): func_003426E0 prototyped in the
 * arm (it was an implicit int in the solo TU), func_003426D8 called with w as
 * the ROM does, and the result slot formed as w + sel*4 first. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003427D0)
S136OS_SLOT(func_003427D0);
#else
/* engine96 probe (task #466, cc1 2.96, unit objdiff): 74.21%; not the image arm (see GUARD). */
extern void func_003426E0(void *w, s32 inputMask);
s32 func_003427D0(void *w, s32 inputMask) {
    f32 *anchor;

    anchor = *(f32 **)((char *)w + 0x228);
    GuiElementSetPos((GuiElement *)w, D_1AE0D0 + anchor[0], D_1AE0D4 + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x228);
    GuiElementSetPos((GuiElement *)((char *)w + 0x98), D_1AE0E0 + anchor[0], D_1AE0E4 + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x228);
    GuiElementSetPos((GuiElement *)((char *)w + 0x4C), D_1AE0D8 + anchor[0], D_1AE0DC + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x228);
    GuiElementSetPos((GuiElement *)((char *)w + 0xE4), D_1AE0E8 + anchor[0], D_1AE0EC + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x228);
    GuiElementSetPos((GuiElement *)((char *)w + 0x1D0), D_1AE0F0 + anchor[0], D_1AE0F4 + anchor[1], 0.0f, 0.0f);

    *GuiElementGetColor((GuiElement *)((char *)w + 0x130)) =
        func_002AA3F0(0x60442D00, 0x70FFFEED, 0x14, 0, 0);

    func_003426E0(w, inputMask);
    func_00342670(w);
    func_003426D8(w);
    w = (char *)w + *(s32 *)((char *)w + 0x31C) * 4;
    return *(s32 *)((char *)w + 0x1B8);
}
#endif

/* func_00342958: forward p+0x130 to GuiSpriteElementDraw. */
void func_00342958(void *p) {
    GuiSpriteElementDraw((char *)p + 0x130);
    __asm__ __volatile__("");
}

/* func_00342978: forward p+0x238 to GuiMenuListDraw. */
void func_00342978(void *p) {
    GuiMenuListDraw((char *)p + 0x238);
    __asm__ __volatile__("");
}

/* func_00342998: immediate-mode draw of the active column's icon row on screen
 * `w`. For each of the *(w+0x1CC) items, reuse the shared icon element (w+0x17C):
 * position it as a vertical list from the anchor (*(w+0x228)) — x = anchor.x +
 * D_1AE0F8, y = anchor.y + D_1AE0FC + D_1AE100*i — pick its texture from the
 * column's entry table (entry = *(w+0x1C8))[i] keyed on the selected column
 * (*(w+0x31C)), tint the item that matches the column's current index
 * (*(w+selCol*4+0x1B8)) dim (0x70FFFEED) and the rest bright (0x60F0F0B0), and
 * submit it (func_00337630). Empty column (count<=0) draws nothing. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342998);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00342998, unit objdiff): 55.38%,
   91/121 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0x40' vs 'addiu sp, sp, -0x50').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern f32 D_1AE0F8, D_1AE0FC, D_1AE100;
void func_00342998(void *w) {
    GuiElement *icon = (GuiElement *)((char *)w + 0x17C);
    s32 count = *(s32 *)((char *)w + 0x1CC);
    s32 i;

    for (i = 0; i < count; i++) {
        f32 *anchor = *(f32 **)((char *)w + 0x228);
        s32 selCol = *(s32 *)((char *)w + 0x31C);
        s32 *table = *(s32 **)((char *)w + 0x1C8);
        s32 entry = table[i];
        s32 *color;

        GuiElementSetPos(icon,
                         anchor[0] + D_1AE0F8,
                         (anchor[1] + D_1AE0FC) + D_1AE100 * (f32)i,
                         0.0f, 0.0f);

        if (selCol == 2) {
            if (entry == 5) {
                GuiSpriteSetTexture(icon, 0xEA9E, 5);
            } else {
                GuiSpriteSetTexture(icon, entry, 0);
            }
        } else if (selCol == 0) {
            if (entry == 6) {
                GuiSpriteSetTexture(icon, 0xEA97, 0);
            } else {
                GuiSpriteSetTexture(icon, *(s32 *)((char *)w + 0x1C4), entry);
            }
        } else {
            GuiSpriteSetTexture(icon, *(s32 *)((char *)w + 0x1C4), entry);
        }

        color = GuiElementGetColor(icon);
        *color = (i == *(s32 *)((char *)w + selCol * 4 + 0x1B8)) ? 0x70FFFEED
                                                                : 0x60F0F0B0;
        func_00337630(icon);
    }
}
#endif

/* Draw a composite widget only when its enabled flag (+0x230) is set: four
 * sprite sub-elements (base, +0x98, +0x4C, +0xE4), a text sub-element (+0x1D0),
 * and three helper sub-draws (func_00342958/78/98). */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00342B30)
S136OS_SLOT(func_00342B30);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00342B30, unit objdiff): 88.15%,
   4/28 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
void func_00342B30(void *w) {
    if (*(s32 *)((char *)w + 0x230) != 0) {
        GuiSpriteElementDraw(w);
        GuiSpriteElementDraw((char *)w + 0x98);
        GuiSpriteElementDraw((char *)w + 0x4C);
        GuiSpriteElementDraw((char *)w + 0xE4);
        GuiTextElementDraw((char *)w + 0x1D0);
        func_00342958(w);
        func_00342978(w);
        func_00342998(w);
    }
}
#endif

/* func_00342BA0: init the GuiWidget at p+0x10 and two type-B elements
 * (p+0xEC, p+0x138), return the object. */
#if defined(S136OS_func_00342BA0) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_00342BA0; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 15/15 words + 3 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_00342BA0 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_00342BA0(void *p) {
    func_00348BD0((char *)p + 0x10);
    GuiElementInitTypeB((char *)p + 0xEC);
    GuiElementInitTypeB((char *)p + 0x138);
    return p;
}
#else
S136OS_SLOT(func_00342BA0);
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342BE0);

/* func_00342BE8: store an int at +0x8. */
void func_00342BE8(void *p, s32 v) {
    *(s32 *)((char *)p + 0x8) = v;
}

/* GuiHelpPromptWidgetInit: construct the help-prompt widget (two button-glyph
 * elements + a label list). Pool -> alloc the 0x10-byte placement record (+0x0)
 * and zero it (the +0x4 pool store is unconditional). Seed the record to 250x120.
 * Init two icon elements (+0xEC from D_1ADBE8, +0x138 from D_1ADBF0), colour them
 * 0x60442D00 / 0x55F0C070, give them glyphs 0x73 / 0x74 from the shared atlas
 * (g_guiInstance+0x8710), and set both alphas to D_1AE104. Then init the label
 * block (+0x10) via func_00348BF8(pool) + func_00348E10(0xB, 0x3C), and run
 * func_00342DF8(w, 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiHelpPromptWidgetInit);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern char *g_guiInstance;
/* (end of this body's declarations) */
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiHelpPromptWidgetInit, unit objdiff): 66.35%,
   72/102 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x40' vs 'addiu sp, sp, -0x30').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00348BF8(void *w, void *pool);
extern u8 D_1ADBE8[];
extern u8 D_1ADBF0[];
extern f32 D_1AE104;
s32 func_00342DF8(void *p, s32 inputMask);
void GuiHelpPromptWidgetInit(void *w, GuiPool *pool) {
    GuiElement *e0 = (GuiElement *)((char *)w + 0xEC);
    GuiElement *e1 = (GuiElement *)((char *)w + 0x138);
    void *obj;
    s32 *color;

    /* +0x4 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x4) = pool;
    if (pool != 0) {
        obj = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x0) = obj;
        *(s32 *)((char *)obj + 0x0) = 0;
        *(s32 *)((char *)obj + 0x4) = 0;
        *(s32 *)((char *)obj + 0x8) = 0;
        *(s32 *)((char *)obj + 0xC) = 0;
    }

    obj = *(void **)((char *)w + 0x0);
    *(f32 *)((char *)obj + 0x0) = 250.0f;
    *(f32 *)((char *)obj + 0x4) = 120.0f;

    GuiElementInit(e0, (s32)D_1ADBE8, pool);
    GuiElementInit(e1, (s32)D_1ADBF0, pool);

    color = GuiElementGetColor(e0);
    *color = 0x60442D00;
    color = GuiElementGetColor(e1);
    *color = 0x55F0C070;

    GuiElementSetGlyph(e0, (s32)(g_guiInstance + 0x8710), 0x73);
    GuiElementSetGlyph(e1, (s32)(g_guiInstance + 0x8710), 0x74);

    GuiElementSetAlpha(e0, D_1AE104);
    GuiElementSetAlpha(e1, D_1AE104);

    func_00348BF8((char *)w + 0x10, pool);
    func_00348E10((char *)w + 0x10, 0xB, 0x3C);
    func_00342DF8(w, 0);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342D60);

/* func_00342D68: call func_00342DA0(p) to get a row index, then return the
 * row's address: base (+0xE8) + index * 0x14 (20-byte stride). */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00342D68)
S136OS_SLOT(func_00342D68);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00342D68, unit objdiff): 66.08%,
   8/15 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu v1, zero, 0x14' vs 'sll v1, v0, 2').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* WALL on cc1 2.9/2.96 only (s136os is byte-exact, GUARD above): 2-callee-save frame
   wall. */
void func_00342DA0(void *p);
s32 func_00342D68(void *p) {
    s32 idx = ((s32 (*)(void *))func_00342DA0)(p);
    return *(s32 *)((char *)p + 0xE8) + idx * 0x14;
}
#endif

/* func_00342DA0: forward p+0x10 to func_00348E60. */
void func_00342DA0(void *p) {
    func_00348E60((char *)p + 0x10);
    __asm__ __volatile__("");
}

/* func_00342DC0: init the GuiWidget at p+0x10, then record the row-data
 * pointer (arg) at p+0xE8. */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00342DC0)
S136OS_SLOT(func_00342DC0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00342DC0, unit objdiff): 64.79%,
   10/16 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* WALL on cc1 2.9/2.96 only (s136os is byte-exact, GUARD above): 3-callee-save frame
   wall. */
void func_00342DC0(void *p, void *records) {
    GuiMenuListSetRows((char *)p + 0x10, records);
    *(void **)((char *)p + 0xE8) = records;
}
#endif

/* func_00342DF8: advance the GuiWidget at p+0x10 by the input mask (GuiMenuListHandleInput),
 * then re-feed
 * it the two floats from the vector at *(p+0x0) via GuiMenuListSetOrigin. Returns 0. */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): GuiMenuListHandleInput declared
 * s32, as defined in 248B50.c. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00342DF8)
S136OS_SLOT(func_00342DF8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00342DF8, unit objdiff): 75.05%,
   14/21 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd s1, 0x8(sp)' vs 'sd s0, 0x0(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* cc1 2.9 (not the image arm; see GUARD) is not byte-exact: 3-callee-save frame
   wall. */
s32 func_00342DF8(void *p, s32 inputMask) {
    void *w = (char *)p + 0x10;
    GuiMenuListHandleInput(w, inputMask);   /* forwards its own $5 (inputMask) — asm sets $5 nowhere */
    {
        f32 *v = *(f32 **)p;
        GuiMenuListSetOrigin(w, v[0], v[1]);
    }
    return 0;
}
#endif

/* Draw a vertical menu list: for each of func_00348E68 rows, position the two
 * per-row sprites (+0xEC frame, +0x138 text) at the anchor (*(w+0)) offset by
 * D_1AE10C (x) and D_1AE108*row (y), colour the text sprite the pulsed selected
 * colour when func_00342DA0 says this row is selected else the static 0x55F0C070,
 * and draw both; then run the menu-list draw. Pulses the counter when dpad held. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00342E48);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00342E48, unit objdiff): 71.59%,
   66/113 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x50' vs 'addiu sp, sp, -0x60').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00342E48(void *w) {
    void *list = (char *)w + 0x10;
    s32 count = func_00348E68(list);
    s32 i;

    if (g_padButtonsPressed & 0x5000) {
        func_002AA3F0(0, 0, 1, 0, 1);
    }
    for (i = 0; i < count; i++) {
        f32 *anchor;
        s32 *color;

        anchor = *(f32 **)((char *)w + 0x0);
        GuiElementSetPos((GuiElement *)((char *)w + 0xEC),
                         anchor[0] + D_1AE10C, anchor[1] + D_1AE108 * (f32)i, 0.0f, 0.0f);
        anchor = *(f32 **)((char *)w + 0x0);
        GuiElementSetPos((GuiElement *)((char *)w + 0x138),
                         anchor[0] + D_1AE10C, anchor[1] + D_1AE108 * (f32)i, 0.0f, 0.0f);
        color = GuiElementGetColor((GuiElement *)((char *)w + 0x138));
        if (((s32 (*)(void *))func_00342DA0)(w) == i) {
            *color = func_002AA3F0(0x20FFFEED, 0x70FFFEED, 0x14, 0, 0);
        } else {
            *color = 0x55F0C070;
        }
        GuiSpriteElementDraw((char *)w + 0xEC);
        GuiSpriteElementDraw((char *)w + 0x138);
    }
    GuiMenuListDraw(list);
}
#endif

/* func_00342FD8: init the screen's six embedded sub-elements (four type-B, the
 * D_1ADA18 widget at +0x130, and the GuiWidget at +0x188), return the object. */
#if defined(S136OS_func_00342FD8) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_00342FD8; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 20/20 words + 6 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_00342FD8 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_00342FD8(void *p) {
    GuiElementInitTypeB(p);
    GuiElementInitTypeB((char *)p + 0x4C);
    GuiElementInitTypeB((char *)p + 0x98);
    GuiElementInitTypeB((char *)p + 0xE4);
    func_003374D8((char *)p + 0x130);
    func_00348BD0((char *)p + 0x188);
    return p;
}
#else
S136OS_SLOT(func_00342FD8);
#endif

/* GuiIconScreenInit2: construct an icon screen (two button-glyph elements, a
 * label element, and a sprite). Pool -> alloc the 0x10-byte placement record
 * (+0x180) and zero it (the +0x184 pool store + the +0x170=1 flag are
 * unconditional). Record seeded 253x200. Init element +0x0 (D_1ADBE8), +0x4C
 * (D_1ADBF8), +0xE4 (D_1ADF98), and sprite +0x130 (D_1AE098); init the sub-block
 * +0x188 via func_00348BF8(pool). Colour the four elements, alpha the two button
 * glyphs to 1.0, assign glyphs 0xCD/0xCE/0x95 from the atlas, scale the label
 * 1.0x1.05, texture the sprite (0xEA9D) scaled 32x32. Finally write the descriptor
 * (+0x174=0xEAA6, +0x178=&D_1AE110, +0x17C=4) after clearing +0x16C, and hand the
 * sub-block its value via func_00348E50. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiIconScreenInit2);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern char *g_guiInstance;
/* (end of this body's declarations) */
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiIconScreenInit2, unit objdiff): 54.75%,
   133/182 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s5, 0x28(sp)' vs 'sd s0, 0x0(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00348BF8(void *w, void *pool);
extern void func_00348E50(void *w, s32 v);
extern u8 D_1ADBE8[], D_1ADBF8[], D_1ADF98[], D_1AE098[], D_1AE110[8];
void GuiIconScreenInit2(void *w, GuiPool *pool) {
    GuiElement *e0 = (GuiElement *)((char *)w + 0x0);
    GuiElement *e1 = (GuiElement *)((char *)w + 0x4C);
    GuiElement *e2 = (GuiElement *)((char *)w + 0xE4);
    GuiElement *sprite = (GuiElement *)((char *)w + 0x130);
    void *sub = (char *)w + 0x188;
    void *rec;

    /* +0x184 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x184) = pool;
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x180) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    *(s32 *)((char *)w + 0x170) = 1;
    rec = *(void **)((char *)w + 0x180);
    *(f32 *)((char *)rec + 0x0) = 253.0f;
    *(f32 *)((char *)rec + 0x4) = 200.0f;

    GuiElementInit(e0, (s32)D_1ADBE8, pool);
    GuiElementInit(e1, (s32)D_1ADBF8, pool);
    GuiElementInit(e2, (s32)D_1ADF98, pool);
    GuiSpriteElementInit(sprite, (s32)D_1AE098, pool);
    func_00348BF8(sub, pool);

    *GuiElementGetColor(e0) = 0x60442D00;
    *GuiElementGetColor(e1) = 0x55F0C070;
    *GuiElementGetColor(e2) = 0x70FFFEED;
    *GuiElementGetColor(sprite) = 0x60F0F0B0;

    GuiElementSetAlpha(e0, 1.0f);
    GuiElementSetAlpha(e1, 1.0f);

    GuiElementSetGlyph(e0, (s32)(g_guiInstance + 0x8710), 0xCD);
    GuiElementSetGlyph(e1, (s32)(g_guiInstance + 0x8710), 0xCE);
    GuiElementSetGlyph(e2, (s32)(g_guiInstance + 0x8710), 0x95);

    GuiElementSetScale(e2, 1.0f, 1.05f, 0.0f, 0.0f);
    GuiSpriteSetTexture(sprite, 0xEA9D, 0);
    memset((char *)w + 0x16C, 0, 4);
    GuiElementSetScale(sprite, 32.0f, 32.0f, 0.0f, 0.0f);

    *(s32 *)((char *)w + 0x174) = 0xEAA6;
    *(void **)((char *)w + 0x178) = D_1AE110;
    *(s32 *)((char *)w + 0x17C) = 4;
    func_00348E50(sub, *(s32 *)((char *)w + 0x16C));
}
#endif

/*
 * func_00343290: cache the row-data pointer at p+0x260 then init the GuiWidget
 * at p+0x188 with it (GuiMenuListSetRows(p+0x188, records)).
 *   p       - the owning menu widget
 *   records - row-data array, stored and passed through unchanged
 *
 * Byte-exact on sdk29 (task #978). The ROM interleaves its prologue with the
 * two argument copies: `addiu sp; move v1,a0; sd ra; move v0,a1`. Plain C
 * gives cc1 both copies ahead of `sd ra` (FACT #8195, the "prologue sd ra vs
 * arg copy" residual). A volatile asm that reads `self` pins the $a0 copy
 * before the barrier and the laundered `rows` copy after it, which is the
 * ROM's order; the store then lands in the jal slot. The trailing empty asm
 * keeps the call from becoming a sibling-call `j`.
 */
void func_00343290(void *p, void *records) {
    char *self = p;
    void *rows;
    __asm__ __volatile__("" : : "r"(self));
    rows = records;
    __asm__("" : "+r"(rows));
    *(void **)(self + 0x260) = rows;
    GuiMenuListSetRows(self + 0x188, records);
    __asm__ __volatile__("");
}

/* func_003432B8: store an int at +0x170. */
void func_003432B8(void *p, s32 v) {
    *(s32 *)((char *)p + 0x170) = v;
}

/* func_003432C0: return base (p+0x260) + index (p+0x16C) * 0x14 - a pointer
 * into a 20-byte-stride array. */
s32 func_003432C0(void *p) {
    return *(s32 *)((char *)p + 0x260) + *(s32 *)((char *)p + 0x16C) * 0x14;
}

extern f32 D_1AE150, D_1AE154, D_1AE158;

/**
 * Position the list's embedded element at p+0xE4 for the current row, relative
 * to the anchor vector *(p+0x180): x = D_1AE150 + anchor[0],
 * y = D_1AE154 + D_1AE158 * (float)row + anchor[1] with row = *(s32 *)(p+0x16C),
 * z = w = 0.
 *
 * p: owning list widget. No return value.
 *
 * Matched byte-exact on sdk29 (cc1 2.9, -O2 -G8 -fno-gcse -fno-strict-aliasing,
 * task #851). The empty asm after GuiElementSetPos suppresses cc1's sibling
 * call so the ROM's jal + frame is reproduced; without it the arm is 18.64%.
 */
void func_003432D8(void *p) {
    f32 *anchor = *(f32 **)((char *)p + 0x180);
    f32 row = (f32)*(s32 *)((char *)p + 0x16C);
    f32 x = D_1AE150 + anchor[0];
    f32 y = (D_1AE154 + D_1AE158 * row) + anchor[1];
    GuiElementSetPos((GuiElement *)((char *)p + 0xE4), x, y, 0.0f, 0.0f);
    __asm__ __volatile__("");
}

/* func_00343330: no-op stub (empty body - registered/overridable hook). */
void func_00343330(void) {
}

/* Update a scrollable list element: position its sub-element (+0x188) at the
 * tracked anchor (*(w+0x180)) + D_1AE160/164, apply the size params D_1AE168/16C,
 * advance its selection by the input mask, and record the resulting state at
 * (w+0x16C). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343338);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00343338, unit objdiff): 88.50%,
   21/40 insns differ. Residual: UNKNOWN-lwc1 (first differing insn: 'lwc1 fv0f, %gp_rel(D_1AE160)(gp)' vs 'lwc1 fa0, %gp_rel(D_1AE160)(gp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00343338(void *w, s32 inputMask) {
    void *sub = (char *)w + 0x188;
    f32 *anchor = *(f32 **)((char *)w + 0x180);
    GuiMenuListSetOrigin(sub, anchor[0] + D_1AE160, anchor[1] + D_1AE164);
    func_00348E10(sub, (s32)D_1AE168, (s32)D_1AE16C);
    GuiMenuListHandleInput(sub, inputMask);
    *(s32 *)((char *)w + 0x16C) = func_00348E60(sub);
}
#endif

/* func_003433D0: position + fade the two-element header group, then run its
 * sub-builders. Places element w at the anchor (*(w+0x180)) plus (D_1AE170,
 * D_1AE174) and element +0x4C at (D_1AE178, D_1AE17C). Fades both to the alpha
 * D_1AE180[func_00348E68(w+0x188) - 1] (list-row-count driven). Runs
 * func_00343338(w, inputMask), func_003432D8(w), func_00343330(), and returns the
 * cached value at +0x16C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003433D0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003433D0, unit objdiff): 65.39%,
   46/70 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x40' vs 'addiu sp, sp, -0x30').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern f32 D_1AE170, D_1AE174, D_1AE178, D_1AE17C;
extern f32 D_1AE180[2];
s32 func_003433D0(void *w, s32 inputMask) {
    f32 *anchor = *(f32 **)((char *)w + 0x180);
    f32 alpha;
    s32 idx;

    GuiElementSetPos((GuiElement *)w, D_1AE170 + anchor[0], D_1AE174 + anchor[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x4C), D_1AE178 + anchor[0], D_1AE17C + anchor[1], 0.0f, 0.0f);

    idx = func_00348E68((char *)w + 0x188) - 1;
    alpha = D_1AE180[idx];
    GuiElementSetAlpha((GuiElement *)w, alpha);
    GuiElementSetAlpha((GuiElement *)((char *)w + 0x4C), alpha);

    func_00343338(w, inputMask);
    func_003432D8(w);
    func_00343330();

    return *(s32 *)((char *)w + 0x16C);
}
#endif

/* Draw a pulsing sprite sub-element (+0xE4): when dpad up/down is pressed
 * (0x5000), tick the color-pulse counter; then set the sprite's colour to the
 * pulsed blend of 0x60442D00 and 0x70FFFEED (period 0x14) and draw it. */
/* GUARD (task #1325): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Byte-exact on that arm (task #1325 lever): g_padButtonsPressed read through
 * the g_padButtonsPressedSplit addressing device (RULING #8620). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003434C8)
S136OS_SLOT(func_003434C8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003434C8, unit objdiff): 74.40%,
   14/39 insns differ. Residual: UNKNOWN-lui (first differing insn: 'lui v1, %hi(g_padButtonsPressed)' vs 'lui v0, %hi(g_padButtonsPressed)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* g_padButtonsPressedSplit: a second C name for g_padButtonsPressed (same
 * assembler symbol via the asm label). The file-scope `.extern ,16` makes a
 * plain read of the real name an assembler-expanded `lui rX; lw rX` macro;
 * `section(".data")` tells cc1 -G8 this spelling is NOT small data, so cc1
 * splits the address itself (`lui $3` ... `lw $2,%lo($3)`), as the ROM does.
 * An ADDRESSING-MODEL DEVICE (RULING #8620): it moves no data and emits nothing;
 * the relocation names g_padButtonsPressed. */
#ifndef TARGET_NATIVE
extern s32 g_padButtonsPressedSplit __asm__("g_padButtonsPressed") __attribute__((section(".data")));
#else
#define g_padButtonsPressedSplit g_padButtonsPressed
#endif
void func_003434C8(void *w) {
    if (g_padButtonsPressedSplit & 0x5000) {
        func_002AA3F0(0, 0, 1, 0, 1);
    }
    *GuiElementGetColor((GuiElement *)((char *)w + 0xE4)) =
        func_002AA3F0(0x60442D00, 0x70FFFEED, 0x14, 0, 0);
    GuiSpriteElementDraw((char *)w + 0xE4);
}
#endif

/* func_00343558: forward p+0x188 to GuiMenuListDraw. */
void func_00343558(void *p) {
    GuiMenuListDraw((char *)p + 0x188);
    __asm__ __volatile__("");
}

/* func_00343578: draw the row of icon sprites. Loops i over *(w+0x17C) entries:
 * positions the shared sprite element (+0x130) at the anchor (*(w+0x180)) plus
 * (D_1AE198, D_1AE19C + D_1AE1A0*i), sets its texture (id 0xEA9E for i==3 else
 * 0xEAA6 — also recorded at +0x174, frame = *(w+0x178)[i]), and draws it via
 * func_00337630. Bails immediately if the count is <= 0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343578);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00343578, unit objdiff): 18.24%,
   76/85 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0x40' vs 'addiu sp, sp, -0x50').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern f32 D_1AE198, D_1AE19C, D_1AE1A0;
void func_00343578(void *w) {
    GuiElement *e = (GuiElement *)((char *)w + 0x130);
    s32 count = *(s32 *)((char *)w + 0x17C);
    s32 i;

    for (i = 0; i < count; i++) {
        f32 *anchor = *(f32 **)((char *)w + 0x180);
        s32 *frames = *(s32 **)((char *)w + 0x178);
        s32 texId = (i == 3) ? 0xEA9E : 0xEAA6;

        GuiElementSetPos(e, anchor[0] + D_1AE198,
                         (anchor[1] + D_1AE19C) + D_1AE1A0 * (f32)i, 0.0f, 0.0f);
        *(s32 *)((char *)w + 0x174) = texId;
        GuiSpriteSetTexture(e, texId, frames[i]);
        func_00337630(e);
    }
}
#endif

/* func_00343668: when the +0x170 flag is set, draw the two sprite elements
 * (p+0x0 and p+0x4C) and run the three sub-draws (func_003434C8/00343558/
 * 00343578). */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00343668)
S136OS_SLOT(func_00343668);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00343668, unit objdiff): 84.76%,
   4/22 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s0, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
/* WALL on cc1 2.9/2.96 only (s136os is byte-exact, GUARD above): 2-callee-save frame +
   branch-likely guard wall. */
extern void func_003434C8(void *p);
extern void func_00343558(void *p);
extern void func_00343578(void *p);
void func_00343668(void *p) {
    if (*(s32 *)((char *)p + 0x170) != 0) {
        GuiSpriteElementDraw(p);
        GuiSpriteElementDraw((char *)p + 0x4C);
        func_003434C8(p);
        func_00343558(p);
        func_00343578(p);
    }
}
#endif

/* func_003436C0: identity passthrough - return the first arg. */
void *func_003436C0(void *p) {
    return p;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003436C8);

/* func_003436D0: map four control floats to a clamped byte value (an interpolated
 * intensity/alpha, max `hi`). `a0` is passed by the sole caller (func_00343AF8) but
 * unused here. Runs the unnamed fixed-point helper cluster twice: reduce(x) =
 * func_001234F0(x, float->int) then func_00123028/func_00122A98 (int wrap: if the
 * div result is negative, re-wrap) then func_00123298 (int->float). First pass
 * feeds reduce(c-b) through func_00122B00 with reduce(d); the (c-a) span both
 * clamps that result (min) and normalises it (ratio = min/(c-a)); second pass runs
 * reduce(ratio) -> func_00123298 -> f. Result = (s32)((1.0 - f) * 255.0), clamped
 * so it never exceeds `hi`. Exact helper semantics untraced (kept as func_ names). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003436D0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003436D0, unit objdiff): 75.32%,
   46/77 insns differ. Residual: UNKNOWN-addiu + movn/movz (first differing insn: 'addiu sp, sp, -0x40' vs 'addiu sp, sp, -0x30').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern s32   func_001234F0(f32 x);
extern s32   func_00123028(s32 a, s32 b);
extern s32   func_00122A98(s32 a, s32 b);
extern s32   func_00122B00(s32 a, s32 b);
extern f32   func_00123298(s32 a);

s32 func_003436D0(void *a0, s32 hi, f32 a, f32 b, f32 c, f32 d) {
    s32 n;
    f32 f, span;
    s32 result;

    (void)a0;

    /* first reduce pass on (c - b) */
    n = func_001234F0(c - b);
    if (func_00123028(n, 0) < 0) {
        n = func_00122A98(0, n);
    }
    n = func_00122B00(n, func_001234F0(d));
    f = func_00123298(n);

    /* clamp to the (c - a) span, then normalise into [0,1] */
    span = c - a;
    if (span < f) {
        f = span;
    }
    f = f / span;

    /* second reduce pass on the normalised ratio */
    n = func_001234F0(f);
    if (func_00123028(n, 0) < 0) {
        n = func_00122A98(0, n);
    }
    f = func_00123298(n);

    result = (s32)((1.0f - f) * 255.0f);
    if (hi < result) {
        result = hi;
    }
    return result;
}
#endif

/* Construct/reset a widget: if given a node pool, record it (+0x8C) and alloc +
 * placement-new the element's primary block (stored at +0x0, zeroed 4 words),
 * then clear the 0x64-byte state region (+0x28) and seed the fixed fields —
 * scale defaults 16.0 at +0x4/+0xC, everything else zero. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003437F0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003437F0, unit objdiff): 78.53%,
   14/42 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu a0, a1, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_003437F0(void *w, GuiPool *pool) {
    /* +0x8C = pool is written UNCONDITIONALLY: the original's `sw $4,0x8C($16)`
       sits in the beqz delay slot, so it runs even on the pool==0 reset path
       (storing 0). The memset below clears +0x28..+0x8C exclusive, so +0x8C is
       not otherwise zeroed — the write must stay above the guard. */
    *(GuiPool **)((char *)w + 0x8C) = pool;
    if (pool != 0) {
        s32 *blk;
        blk = (s32 *)GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)w = blk;
        blk[0] = 0; blk[1] = 0; blk[2] = 0; blk[3] = 0;
    }
    memset((char *)w + 0x28, 0, 0x64);
    *(s32 *)((char *)w + 0x94) = 0;
    *(f32 *)((char *)w + 0xC) = 16.0f;
    *(s32 *)((char *)w + 0x10) = 0;
    *(s32 *)((char *)w + 0x20) = 0;
    *(s32 *)((char *)w + 0x24) = 0;
    *(s32 *)((char *)w + 0x14) = 0;
    *(f32 *)((char *)w + 0x4) = 16.0f;
    *(s32 *)((char *)w + 0x8) = 0;
    *(s32 *)((char *)w + 0x18) = 0;
    *(s32 *)((char *)w + 0x1C) = 0;
    *(s32 *)((char *)w + 0x90) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343888);

/* func_00343AD0(view): the currently highlighted row id of a list view, or 0
 * when the view is empty. +0x10 is the live row count and +0x14 the highlighted
 * row index into the row-id array at +0x28; an empty view (count == 0) returns
 * 0 without reading the index. Returns the row id (an s32).
 *
 * Load-bearing for the byte match: the scaled index is computed into its own
 * statement temp. Written inline as `p + p[0x14]*4 + 0x28` the cc1 2.9 expander
 * evaluates the complex operand first and emits `addu $3,$3,$4` against the
 * ROM's `addu $3,$4,$3` — the same expand-swap tax as func_003424C8, and the
 * only differing word before the hoist.
 * (Identity: func_00343AD0, 0x00343AD0; name frozen by the .s glabel.) */
s32 func_00343AD0(void *view) {
    s32 rowOfs;
    if (*(s32 *)((char *)view + 0x10) == 0) {
        return 0;
    }
    rowOfs = *(s32 *)((char *)view + 0x14) * 4;
    return *(s32 *)((char *)view + rowOfs + 0x28);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343AF8);

/* func_00343E80(view, records): build the list view's visible row set. Walk
 * `records` (stride 0xA, signed item-id at +0x6) up to records+0xFA; for each
 * owned item (g_inventoryOwned[id] != 0) append the id to the view's row array
 * at +0x28 (capped at 0x19 entries, count at +0x10), then set the initial
 * highlighted row +0x14 = count/2 and run layout func_00343888(view, 0).
 * BLOCKED: the body needs a 2-arg (view, records) prototype, but the
 * already-matched thin forwarder func_00344458 relies on func_00343E80 being
 * seen as 1-arg (file-scope proto at line 89, visible under TARGET_NATIVE too)
 * so it passes its own $a1 through untouched. A #else 2-arg definition cascades
 * an arity fix through func_00344458 and up; left INCLUDE_ASM to preserve the
 * forwarder's byte-match. (Body is otherwise an integer ownership-filter loop.) */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343E80);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343F30);

/* func_00343F38: store two floats at +0x8 and +0xC. */
void func_00343F38(void *p, f32 a, f32 b) {
    *(f32 *)((char *)p + 0x8) = a;
    *(f32 *)((char *)p + 0xC) = b;
}

/* func_00343F48: store a float at +0x4. */
void func_00343F48(void *p, f32 v) {
    *(f32 *)((char *)p + 0x4) = v;
}

/* func_00343F50: as func_0033C060: write two floats through *(p+0x0). */
/* Byte-exact under the canonical engine flags (pointer reload as in
   func_0033C060). */
void func_00343F50(void *p, f32 a, f32 b) {
    (*(f32 **)p)[0] = a;
    (*(f32 **)p)[1] = b;
}

/* func_00343F68: clear the int at +0x90. */
void func_00343F68(void *p) {
    *(s32 *)((char *)p + 0x90) = 0;
}

/* func_00343F70: return the int at +0x10. */
s32 func_00343F70(void *p) {
    return *(s32 *)((char *)p + 0x10);
}

/* func_00343F78: draw the two-layer highlight box behind selection `w`. Both
 * rects are built from the base position (*(w+0)): the inner fill rect spans
 * (base + D_1AE1D0/D_1AE1D4) .. (base + D_1AE1D8/D_1AE1DC) in a fixed colour
 * (0x60241700); the outer rect expands that by +/-D_1AE1E0 in x and +/-D_1AE1E4
 * in y and is drawn first in a pulsing highlight colour
 * (func_002AA3F0(0x60241700, 0x55F0C070, ...)). Both are submitted to the GS
 * rect primitive func_00285EF8 clipped to g_screenWidth x g_screenHeight. The
 * D_1AE1Dx/Ex offsets are stored as integers (cvt.s.w) and added as floats. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00343F78);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00343F78, unit objdiff): 36.02%,
   116/124 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x30' vs 'addiu sp, sp, -0x20').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern s32 g_screenWidth, g_screenHeight;
extern s32 D_1AE1D0, D_1AE1D4, D_1AE1D8, D_1AE1DC, D_1AE1E0, D_1AE1E4;
extern void func_00285EF8(s32 x0, s32 y0, s32 x1, s32 y1, s32 sw, s32 sh, u32 color);
void func_00343F78(void *w) {
    f32 *base = *(f32 **)((char *)w + 0x0);
    f32 bx = base[0];
    f32 by = base[1];
    u32 pulsed;

    /* outer border rect (expanded by D_1AE1E0/E4), pulsing highlight colour */
    pulsed = func_002AA3F0(0x60241700, 0x55F0C070, 0x14, 0, 0);
    func_00285EF8((s32)(((f32)D_1AE1D0 + bx) - (f32)D_1AE1E0),
                  (s32)(((f32)D_1AE1D4 + by) - (f32)D_1AE1E4),
                  (s32)(((f32)D_1AE1D8 + bx) + (f32)D_1AE1E0),
                  (s32)(((f32)D_1AE1DC + by) + (f32)D_1AE1E4),
                  g_screenWidth, g_screenHeight, pulsed);

    /* inner fill rect, fixed colour */
    func_00285EF8((s32)((f32)D_1AE1D0 + bx),
                  (s32)((f32)D_1AE1D4 + by),
                  (s32)((f32)D_1AE1D8 + bx),
                  (s32)((f32)D_1AE1DC + by),
                  g_screenWidth, g_screenHeight, 0x60241700);
}
#endif

/* func_00344110: init the screen's eight embedded sub-elements at their fixed
 * offsets (three type-B, the D_1ADA18 widget, three type-C, and the identity
 * widget func_003436C0), return the object. */
#if defined(S136OS_func_00344110) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_00344110; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 24/24 words + 8 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_00344110 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
extern void *func_003436C0(void *p);
void *func_00344110(void *p) {
    GuiElementInitTypeB(p);
    GuiElementInitTypeB((char *)p + 0x4C);
    GuiElementInitTypeB((char *)p + 0x98);
    func_003374D8((char *)p + 0xE4);
    GuiElementInitTypeC((char *)p + 0x1B0);
    GuiElementInitTypeC((char *)p + 0x208);
    GuiElementInitTypeC((char *)p + 0x260);
    func_003436C0((char *)p + 0x2C8);
    return p;
}
#else
S136OS_SLOT(func_00344110);
#endif

/* GuiTitledSpriteScreenInit: construct a titled-sprite screen (three button
 * glyphs, three text rows, a sprite, and an embedded sub-widget). Pool -> alloc
 * the 0x10-byte placement record (+0x2B8), +0x2BC=pool + +0x2C0=1 unconditional.
 * Record seeded 251x210. Init buttons (+0x0/D_1ADBE8, +0x4C/D_1ADBF0,
 * +0x98/D_1ADBF8), text rows (+0x1B0/D_1ADC08, +0x208/D_1ADC60, +0x260/D_1ADC60),
 * and the sprite (+0xE4/D_1AE098). Colour them, glyph the buttons 0x99/0x9A/0x9B,
 * set the two in-place text buffers (+0x120, +0x160) and the localized row 0x2BE5
 * (right-aligned via text flag 2). func_003444D0(w,0), scale the sprite 32x32,
 * then build the sub-widget at +0x2C8 (func_003437F0 + place 0,40 + scale 32),
 * clear +0x360, and run func_003446B8(w,0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiTitledSpriteScreenInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiTitledSpriteScreenInit, unit objdiff): 59.83%,
   128/214 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s7, 0x48(sp)' vs 'sd s1, 0x18(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_003444D0(void *p, u32 idx);
extern void func_003446B8(void *w, s32 flag);
extern char *g_guiInstance;
extern u8 D_1ADBE8[], D_1ADBF0[], D_1ADBF8[], D_1ADC08[], D_1ADC60[], D_1AE098[];
void GuiTitledSpriteScreenInit(void *w, GuiPool *pool) {
    GuiElement *btn0 = (GuiElement *)((char *)w + 0x0);
    GuiElement *btn1 = (GuiElement *)((char *)w + 0x4C);
    GuiElement *btn2 = (GuiElement *)((char *)w + 0x98);
    GuiElement *txt0 = (GuiElement *)((char *)w + 0x1B0);
    GuiElement *txt1 = (GuiElement *)((char *)w + 0x208);
    GuiElement *txt2 = (GuiElement *)((char *)w + 0x260);
    GuiElement *sprite = (GuiElement *)((char *)w + 0xE4);
    void *sub = (char *)w + 0x2C8;
    void *rec;

    /* +0x2BC = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x2BC) = pool;
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x2B8) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    *(s32 *)((char *)w + 0x2C0) = 1;
    rec = *(void **)((char *)w + 0x2B8);
    *(f32 *)((char *)rec + 0x0) = 251.0f;
    *(f32 *)((char *)rec + 0x4) = 210.0f;

    GuiElementInit(btn0, (s32)D_1ADBE8, pool);
    GuiElementInit(btn1, (s32)D_1ADBF0, pool);
    GuiElementInit(btn2, (s32)D_1ADBF8, pool);
    GuiTextElementInit(txt0, (s32)D_1ADC08, pool);
    GuiTextElementInit(txt1, (s32)D_1ADC60, pool);
    GuiTextElementInit(txt2, (s32)D_1ADC60, pool);
    GuiSpriteElementInit(sprite, (s32)D_1AE098, pool);

    *GuiElementGetColor(btn0) = 0x60442D00;
    *GuiElementGetColor(btn1) = 0x60241700;
    *GuiElementGetColor(btn2) = 0x55F0C070;
    *GuiElementGetColor(txt0) = (s32)0x80F0F0F0;
    *GuiElementGetColor(txt2) = (s32)0x80F0F0F0;
    *GuiElementGetColor(txt1) = (s32)0x80F0F0F0;
    *GuiElementGetColor(sprite) = (s32)0x80F0F0F0;

    GuiElementSetGlyph(btn0, (s32)(g_guiInstance + 0x8710), 0x99);
    GuiElementSetGlyph(btn1, (s32)(g_guiInstance + 0x8710), 0x9A);
    GuiElementSetGlyph(btn2, (s32)(g_guiInstance + 0x8710), 0x9B);

    GuiElementSetText(txt0, (s32)((char *)w + 0x120));
    GuiElementSetText(txt1, (s32)((char *)w + 0x160));
    GuiElementSetText(txt2, GetLocalizedString(0x2BE5));
    GuiElementSetTextFlag(txt2, 2);

    func_003444D0(w, 0);
    GuiElementSetScale(sprite, 32.0f, 32.0f, 0.0f, 0.0f);

    func_003437F0(sub, pool);
    func_00343F38(sub, 0.0f, 40.0f);
    func_00343F48(sub, 32.0f);
    *(s32 *)((char *)w + 0x360) = 0;
    func_003446B8(w, 0);
}
#endif

/* func_00344458: forward p+0x2C8 to func_00343E80. */
void func_00344458(void *p) {
    func_00343E80((char *)p + 0x2C8);
    __asm__ __volatile__("");
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344478);

/* func_00344480: forward p+0x2C8 to func_00343AD0. */
void func_00344480(void *p) {
    func_00343AD0((char *)p + 0x2C8);
    __asm__ __volatile__("");
}

/* func_003444A0: forward p+0x2C8 to func_00343F68. */
void func_003444A0(void *p) {
    func_00343F68((char *)p + 0x2C8);
    __asm__ __volatile__("");
}

/* func_003444C0: store an int at +0x360. */
void func_003444C0(void *p, s32 v) {
    *(s32 *)((char *)p + 0x360) = v;
}

/* func_003444C8: return the int at +0x360. */
s32 func_003444C8(void *p) {
    return *(s32 *)((char *)p + 0x360);
}

/* func_003444D0: if idx is in range (<4) record it at p+0x2C4, then reset the
 * field at p+0x1A0 to zero. */
void func_003444D0(void *p, u32 idx) {
    if (idx < 4) {
        *(s32 *)((char *)p + 0x2C4) = idx;
    }
    *(s32 *)((char *)p + 0x1A0) = 0;
}

/* Position or hide a widget sub-element (+0x208): if the gate func_00343F70
 * (queried on +0x2C8) returns 0, hide it; otherwise place it at the tracked
 * anchor (*(w+0x2B8)) offset by the fixed constants D_1AE1E8/D_1AE1EC. */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003444E8)
S136OS_SLOT(func_003444E8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003444E8, unit objdiff): 82.64%,
   9/30 insns differ. Residual: UNKNOWN-b (first differing insn: 'b 0xe4e4' vs 'b 0xdb34').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
void func_003444E8(void *w) {
    if (func_00343F70((char *)w + 0x2C8) == 0) {
        GuiElementSetVisible((GuiElement *)((char *)w + 0x208), 0);
    } else {
        f32 *pos = *(f32 **)((char *)w + 0x2B8);
        GuiElementSetPos((GuiElement *)((char *)w + 0x208),
                         D_1AE1E8 + pos[0], D_1AE1EC + pos[1], 0.0f, 0.0f);
    }
}
#endif

/* func_00344558: build the titled-sprite's caption label. Shows element +0x1B0,
 * then formats a two-part string into a scratch buffer according to the mode at
 * *(w+0x2C4) (0 -> string 0x2BE7, 1 -> 0x2BE8, 2 -> 0x2BEA; each combined with the
 * suffix 0x2BEB via the D_1AE1F8 format). If the result fits (< 0x31 chars) it is
 * copied into +0x120; otherwise the +0x120 field gets the overflow template
 * D_1ADBA8 (localized string 0). Finally positions +0x1B0 at the anchor
 * (*(w+0x2B8)) plus (D_1AE1F0, D_1AE1F4). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344558);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00344558, unit objdiff): 12.55%,
   99/113 insns differ. Residual: UNKNOWN-sd + movn/movz, gp/abs-mixed symbol (first differing insn: 'sd s2, 0x110(sp)' vs 'sd s1, 0x108(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern s32 func_001157AC(const char *s); /* SDK strlen */
extern u8 D_1AE1F8[], D_1ADBA8[];
extern f32 D_1AE1F0, D_1AE1F4;
void func_00344558(void *w) {
    char buf[256];
    s32 mode = *(s32 *)((char *)w + 0x2C4);
    f32 *anchor;

    GuiElementSetVisible((GuiElement *)((char *)w + 0x1B0), 1);

    if (mode == 0 || mode == 1 || mode == 2) {
        s32 strId = (mode == 0) ? 0x2BE7 : (mode == 1) ? 0x2BE8 : 0x2BEA;
        func_00115DA8(buf, (const char *)D_1AE1F8,
                      (const char *)GetLocalizedString(strId),
                      (const char *)GetLocalizedString(0x2BEB));
    }

    if (func_001157AC(buf) < 0x31) {
        func_00115AC0((char *)w + 0x120, buf, 0x31);
    } else {
        func_00115DA8((char *)w + 0x120, (const char *)D_1ADBA8,
                      (const char *)GetLocalizedString(0));
    }

    anchor = *(f32 **)((char *)w + 0x2B8);
    GuiElementSetPos((GuiElement *)((char *)w + 0x1B0),
                     D_1AE1F0 + anchor[0], D_1AE1F4 + anchor[1], 0.0f, 0.0f);
}
#endif

/* func_003446B8: lay out the titled-sprite sub-widget. Reads the anchor position
 * record at *(w+0x2B8) (float x at [0], y at [1]) and positions five child
 * elements at fixed data-driven offsets from it: the root (w) at
 * (D_1AE200+x, D_1AE204+y), +0x4C at (D_1AE208, D_1AE20C), +0x98 at
 * (D_1AE210, D_1AE214), +0x260 at (D_1AE218, D_1AE21C), and the sub-element
 * +0x2C8 via func_00343F50 at (D_1AE220, D_1AE224). Then runs the two builders
 * func_00344558(w)/func_003444E8(w) and records func_00343888(sub, flag) at
 * +0x1A0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003446B8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003446B8, unit objdiff): 53.67%,
   73/98 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s0, 0x0(sp)' vs 'sd s1, 0x8(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00344558(void *w);
extern s32 func_00343888(void *w, s32 flags);
extern f32 D_1AE200, D_1AE204, D_1AE208, D_1AE20C, D_1AE210, D_1AE214;
extern f32 D_1AE218, D_1AE21C, D_1AE220, D_1AE224;
void func_003446B8(void *w, s32 flag) {
    GuiElement *sub = (GuiElement *)((char *)w + 0x2C8);
    f32 *src = *(f32 **)((char *)w + 0x2B8);

    GuiElementSetPos((GuiElement *)w, D_1AE200 + src[0], D_1AE204 + src[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x4C), D_1AE208 + src[0], D_1AE20C + src[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x98), D_1AE210 + src[0], D_1AE214 + src[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x260), D_1AE218 + src[0], D_1AE21C + src[1], 0.0f, 0.0f);
    func_00343F50(sub, D_1AE220 + src[0], D_1AE224 + src[1]);

    func_00344558(w);
    func_003444E8(w);
    *(s32 *)((char *)w + 0x1A0) = func_00343888(sub, flag);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344800);

/* func_00344808: draw the titled-sprite widget. Bails if hidden (*(w+0x2C0)==0).
 * Draws three sprite elements (w, +0x4C, +0x98) and the title text (+0x1B0), lays
 * out the sub-widget (func_00343AF8(+0x2C8)), then resolves the currently-selected
 * weapon's caption: slot = g_itemEquippedSlot[func_00343AD0(+0x2C8)]; captionId =
 * g_weaponTable[slot*0xE0 + 8]; sets +0x208's text to that localized string and
 * draws +0x208 and +0x260. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344808);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00344808, unit objdiff): 78.85%,
   23/50 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s2, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00343AF8(void *w);
void func_00344808(void *w) {
    s32 slot;
    s32 captionId;

    if (*(s32 *)((char *)w + 0x2C0) == 0) {
        return;
    }
    GuiSpriteElementDraw(w);
    GuiSpriteElementDraw((char *)w + 0x4C);
    GuiSpriteElementDraw((char *)w + 0x98);
    GuiTextElementDraw((char *)w + 0x1B0);
    func_00343AF8((char *)w + 0x2C8);

    slot = g_itemEquippedSlot[func_00343AD0((char *)w + 0x2C8)];
    captionId = *(s32 *)(g_weaponTable + slot * 0xE0 + 8);
    GuiElementSetText((GuiElement *)((char *)w + 0x208), GetLocalizedString(captionId));

    GuiTextElementDraw((char *)w + 0x208);
    GuiTextElementDraw((char *)w + 0x260);
}
#endif

/* func_003448C0: init the info-panel screen's fourteen embedded sub-elements at
 * their fixed offsets (six type-B at p+0x0/+0x4C/+0x98/+0xE4/+0x130/+0x17C, two
 * type-C at +0x1C8/+0x220, the D_1ADA18 widget at +0x278, then five more type-C
 * at +0x2B8/+0x310/+0x368/+0x3C0/+0x418), return the object. */
#if defined(S136OS_func_003448C0) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_003448C0; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 36/36 words + 14 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_003448C0 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_003448C0(void *p) {
    GuiElementInitTypeB(p);
    GuiElementInitTypeB((char *)p + 0x4C);
    GuiElementInitTypeB((char *)p + 0x98);
    GuiElementInitTypeB((char *)p + 0xE4);
    GuiElementInitTypeB((char *)p + 0x130);
    GuiElementInitTypeB((char *)p + 0x17C);
    GuiElementInitTypeC((char *)p + 0x1C8);
    GuiElementInitTypeC((char *)p + 0x220);
    func_003374D8((char *)p + 0x278);
    GuiElementInitTypeC((char *)p + 0x2B8);
    GuiElementInitTypeC((char *)p + 0x310);
    GuiElementInitTypeC((char *)p + 0x368);
    GuiElementInitTypeC((char *)p + 0x3C0);
    GuiElementInitTypeC((char *)p + 0x418);
    return p;
}
#else
S136OS_SLOT(func_003448C0);
#endif

/* GuiInfoPanelScreenInit: construct the info panel — six button-glyph elements
 * (+0x0/+0x4C/+0x98/+0xE4/+0x130/+0x17C), seven text rows, and a sprite (+0x278).
 * Pool -> alloc the 0x10-byte placement record (+0x4A0, +0x4A4=pool + +0x4A8=1
 * unconditional); record seeded 250x207. Elements art D_1ADBE8/BF0/BF8/C00/DF98/
 * DFA0; text rows from D_1ADEE8/EF0/EF8/DD38/AE228/DD30/AE008. Colours the buttons
 * (0x60442D00/0x60241700/0x55F0C070 x3/0x70FFFEED) and text rows (0x80F0F0F0),
 * glyphs the buttons 0x9E..0xA1/0x5D/0x5E, row +0x310 scaled 0.9 (flag 1), sets
 * the localized/in-place row texts, scales the sprite 32x32 (colour 0x60F0F0B0),
 * records +0x4AC/+0x4B0/+0x4B4=0 / +0x4B8=3, and finally func_003453D0(w, 0) (the
 * weapon-select body builder). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiInfoPanelScreenInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiInfoPanelScreenInit, unit objdiff): 46.67%,
   281/369 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x90' vs 'addiu sp, sp, -0x80').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_003453D0(void *w, void *arg1);
extern char *g_guiInstance;
extern u8 D_1ADBE8[], D_1ADBF0[], D_1ADBF8[], D_1ADC00[], D_1ADF98[], D_1ADFA0[];
extern u8 D_1ADEE8[], D_1ADEF0[], D_1ADEF8[], D_1ADD38[], D_1AE228[], D_1ADD30[];
extern u8 D_1AE008[], D_1ADFB0[];
void GuiInfoPanelScreenInit(void *w, GuiPool *pool) {
    GuiElement *e0 = (GuiElement *)((char *)w + 0x0);
    GuiElement *e1 = (GuiElement *)((char *)w + 0x4C);
    GuiElement *e2 = (GuiElement *)((char *)w + 0x98);
    GuiElement *e3 = (GuiElement *)((char *)w + 0xE4);
    GuiElement *e4 = (GuiElement *)((char *)w + 0x130);
    GuiElement *e5 = (GuiElement *)((char *)w + 0x17C);
    GuiElement *t368 = (GuiElement *)((char *)w + 0x368);
    GuiElement *t3C0 = (GuiElement *)((char *)w + 0x3C0);
    GuiElement *t418 = (GuiElement *)((char *)w + 0x418);
    GuiElement *t2B8 = (GuiElement *)((char *)w + 0x2B8);
    GuiElement *t310 = (GuiElement *)((char *)w + 0x310);
    GuiElement *t1C8 = (GuiElement *)((char *)w + 0x1C8);
    GuiElement *t220 = (GuiElement *)((char *)w + 0x220);
    GuiElement *sprite = (GuiElement *)((char *)w + 0x278);
    void *rec;

    /* +0x4A4 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x4A4) = pool;
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x4A0) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    *(s32 *)((char *)w + 0x4A8) = 1;
    rec = *(void **)((char *)w + 0x4A0);
    *(f32 *)((char *)rec + 0x0) = 250.0f;
    *(f32 *)((char *)rec + 0x4) = 207.0f;

    GuiElementInit(e0, (s32)D_1ADBE8, pool);
    GuiElementInit(e1, (s32)D_1ADBF0, pool);
    GuiElementInit(e2, (s32)D_1ADBF8, pool);
    GuiElementInit(e3, (s32)D_1ADC00, pool);
    GuiElementInit(e4, (s32)D_1ADF98, pool);
    GuiElementInit(e5, (s32)D_1ADFA0, pool);
    GuiTextElementInit(t368, (s32)D_1ADEE8, pool);
    GuiTextElementInit(t3C0, (s32)D_1ADEF0, pool);
    GuiTextElementInit(t418, (s32)D_1ADEF8, pool);
    GuiTextElementInit(t2B8, (s32)D_1ADD38, pool);
    GuiTextElementInit(t310, (s32)D_1AE228, pool);
    GuiTextElementInit(t1C8, (s32)D_1ADD30, pool);
    GuiTextElementInit(t220, (s32)D_1AE008, pool);

    *GuiElementGetColor(e0) = 0x60442D00;
    *GuiElementGetColor(e1) = 0x60241700;
    *GuiElementGetColor(e2) = 0x55F0C070;
    *GuiElementGetColor(e3) = 0x55F0C070;
    *GuiElementGetColor(e5) = 0x70FFFEED;
    *GuiElementGetColor(e4) = 0x55F0C070;
    *GuiElementGetColor(t2B8) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t310) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t1C8) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t220) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t368) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t3C0) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t418) = (s32)0x80F0F0F0;

    GuiElementSetTextFlag(t2B8, 0);
    GuiElementSetTextFlag(t310, 1);
    GuiElementSetScale(t310, 0.9f, 0.9f, 0.0f, 0.0f);

    GuiElementSetGlyph(e0, (s32)(g_guiInstance + 0x8710), 0x9E);
    GuiElementSetGlyph(e1, (s32)(g_guiInstance + 0x8710), 0x9F);
    GuiElementSetGlyph(e2, (s32)(g_guiInstance + 0x8710), 0xA0);
    GuiElementSetGlyph(e3, (s32)(g_guiInstance + 0x8710), 0xA1);
    GuiElementSetGlyph(e4, (s32)(g_guiInstance + 0x8710), 0x5D);
    GuiElementSetGlyph(e5, (s32)(g_guiInstance + 0x8710), 0x5E);

    *(s32 *)((char *)w + 0x4AC) = 0;
    *(s32 *)((char *)w + 0x4B0) = 0;
    *(s32 *)((char *)w + 0x4B8) = 3;
    GuiSpriteElementInit(sprite, (s32)D_1ADFB0, pool);
    *GuiElementGetColor(sprite) = 0x60F0F0B0;
    GuiElementSetScale(sprite, 32.0f, 32.0f, 0.0f, 0.0f);

    /* +0x1C8/+0x220 colours are re-written here (idempotent, matches original). */
    *GuiElementGetColor(t1C8) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t220) = (s32)0x80F0F0F0;
    GuiElementSetText(t1C8, GetLocalizedString(0x2BEA));
    GuiElementSetText(t220, GetLocalizedString(0x2BEF));
    GuiElementSetText(t368, (s32)((char *)w + 0x470));
    GuiElementSetText(t3C0, (s32)((char *)w + 0x480));
    GuiElementSetText(t418, (s32)((char *)w + 0x490));
    GuiElementSetTextFlag(t368, 0);
    GuiElementSetTextFlag(t3C0, 0);
    GuiElementSetTextFlag(t418, 0);
    *(s32 *)((char *)w + 0x4B4) = 0;
    GuiElementSetText(t2B8, GetLocalizedString(0x2BE5));

    func_003453D0(w, 0);
}
#endif

/* func_00344E08: weapon-select mode-0 (2x2 grid) input handler. The 2nd arg is the
 * D-pad input mask (threaded as void* through func_003453D0). Left (0x1000): column
 * +0x4B0=1, page +0x4B8=2. Right (0x4000): column 0, page 2. Up (0x8000): row
 * +0x4AC--, on underflow clamp to 0 + set page +0x4B8=1. Down (0x2000): row++, on
 * reaching 2 wrap to 0 + page 1 + column 0. Each plays move sound 3. Finally looks
 * up the selected row in the 0xA-byte-stride table D_1AA830 and records its +6
 * halfword field at +0x4B4. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344E08);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00344E08, unit objdiff): 64.16%,
   41/76 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd ra, 0x8(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 D_1AA830[];
void func_00344E08(void *w, void *a) {
    s32 flags = (s32)a;
    s32 idx;

    if (flags & 0x1000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4B0) = 1;
        *(s32 *)((char *)w + 0x4B8) = 2;
    } else if (flags & 0x4000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4B0) = 0;
        *(s32 *)((char *)w + 0x4B8) = 2;
    } else if (flags & 0x8000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4AC) = *(s32 *)((char *)w + 0x4AC) - 1;
        if (*(s32 *)((char *)w + 0x4AC) < 0) {
            *(s32 *)((char *)w + 0x4AC) = 0;
            *(s32 *)((char *)w + 0x4B8) = 1;
        }
    } else if (flags & 0x2000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4AC) = *(s32 *)((char *)w + 0x4AC) + 1;
        if (*(s32 *)((char *)w + 0x4AC) >= 2) {
            *(s32 *)((char *)w + 0x4AC) = 0;
            *(s32 *)((char *)w + 0x4B8) = 1;
            *(s32 *)((char *)w + 0x4B0) = 0;
        }
    }

    idx = *(s32 *)((char *)w + 0x4AC);
    *(s32 *)((char *)w + 0x4B4) = *(s16 *)(D_1AA830 + idx * 0xA + 6);
}
#endif

/* func_00344F18: weapon-select mode-2 (2-col x 4-row grid) input handler. 2nd arg
 * = D-pad mask. Adjusts column +0x4B0 (L/R) and row +0x4AC (up/down) with wrap
 * across the grid edges, flipping page +0x4B8 on the wrap-arounds; each direction
 * plays sound 3. The shared wrap exits carry path-specific constants (inlined
 * here). Finally records the (row + col*4)'th 0xA-stride entry's +6 halfword from
 * D_25E308 at +0x4B4. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00344F18);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00344F18, unit objdiff): 38.98%,
   82/119 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd ra, 0x8(sp)' vs 'daddu s0, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 D_25E308[];
void func_00344F18(void *w, void *a) {
    s32 flags = (s32)a;
    s32 col, row, idx;

    if (flags & 0x1000) {            /* LEFT: col-- */
        PlayGlobalSound(3, 0, 0);
        col = *(s32 *)((char *)w + 0x4B0) - 1;
        *(s32 *)((char *)w + 0x4B0) = col;
        if (col < 0) {
            row = *(s32 *)((char *)w + 0x4AC);
            if (row < 2) {
                *(s32 *)((char *)w + 0x4B0) = 0;
                *(s32 *)((char *)w + 0x4B8) = 0;
            } else if (row == 2) {
                *(s32 *)((char *)w + 0x4B0) = 1;
            } else {
                *(s32 *)((char *)w + 0x4B0) = 0;
                *(s32 *)((char *)w + 0x4B8) = 1;
                *(s32 *)((char *)w + 0x4AC) = 0;
            }
        }
    } else if (flags & 0x4000) {     /* RIGHT: col++ */
        PlayGlobalSound(3, 0, 0);
        col = *(s32 *)((char *)w + 0x4B0) + 1;
        *(s32 *)((char *)w + 0x4B0) = col;
        if (col >= 2) {
            row = *(s32 *)((char *)w + 0x4AC);
            if (row < 2) {
                *(s32 *)((char *)w + 0x4B0) = 0;
                *(s32 *)((char *)w + 0x4B8) = 0;
            } else if (row == 2) {
                *(s32 *)((char *)w + 0x4B0) = 0;
            } else {
                *(s32 *)((char *)w + 0x4B0) = 0;
                *(s32 *)((char *)w + 0x4B8) = 3;
                *(s32 *)((char *)w + 0x4AC) = 0;
            }
        }
    } else if (flags & 0x8000) {     /* UP: row-- */
        PlayGlobalSound(3, 0, 0);
        row = *(s32 *)((char *)w + 0x4AC) - 1;
        *(s32 *)((char *)w + 0x4AC) = row;
        if (row < 0) {
            *(s32 *)((char *)w + 0x4AC) = 3;
        }
    } else if (flags & 0x2000) {     /* DOWN: row++ */
        PlayGlobalSound(3, 0, 0);
        row = *(s32 *)((char *)w + 0x4AC) + 1;
        *(s32 *)((char *)w + 0x4AC) = row;
        if (row >= 4) {
            *(s32 *)((char *)w + 0x4AC) = 0;
            *(s32 *)((char *)w + 0x4B8) = 3;
            *(s32 *)((char *)w + 0x4B0) = 0;
        }
    }

    col = *(s32 *)((char *)w + 0x4B0);
    row = *(s32 *)((char *)w + 0x4AC);
    idx = row + col * 4;
    *(s32 *)((char *)w + 0x4B4) = *(s16 *)(D_25E308 + idx * 0xA + 6);
}
#endif

/* func_00345080: weapon-select mode-3 (2-col x 2-row grid) input handler. 2nd arg
 * = D-pad mask. Adjusts column +0x4B0 (L/R) and row +0x4AC (up/down) with
 * wrap/page-flip (+0x4B8) at the edges; each direction plays sound 3. Records the
 * flat index row + col*2 directly at +0x4B4 (no table). Note the LEFT/col==0/row>=2
 * path faithfully leaves +0x4B0 at -1 (the original reuses the just-stored col-1
 * for the index rather than re-clamping). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00345080);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00345080, unit objdiff): 69.62%,
   54/91 insns differ. Residual: UNKNOWN-sd + movn/movz (first differing insn: 'sd ra, 0x8(sp)' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00345080(void *w, void *a) {
    s32 flags = (s32)a;
    s32 col, row;

    if (flags & 0x1000) {            /* LEFT: col-- */
        PlayGlobalSound(3, 0, 0);
        col = *(s32 *)((char *)w + 0x4B0) - 1;
        *(s32 *)((char *)w + 0x4B0) = col;
        if (col < 0) {               /* col was 0 */
            row = *(s32 *)((char *)w + 0x4AC);
            if (row == 0) {
                *(s32 *)((char *)w + 0x4B8) = 2;
                *(s32 *)((char *)w + 0x4B0) = 1;
                *(s32 *)((char *)w + 0x4AC) = 3;
            } else if (row == 1) {
                *(s32 *)((char *)w + 0x4B0) = 1;
            }
            /* row >= 2: +0x4B0 stays -1 (faithful reused-col exit) */
        }
    } else if (flags & 0x4000) {     /* RIGHT: col++ wrap 0<->1 */
        PlayGlobalSound(3, 0, 0);
        col = *(s32 *)((char *)w + 0x4B0) + 1;
        if (col > 1) col = 0;
        *(s32 *)((char *)w + 0x4B0) = col;
    } else if (flags & 0x8000) {     /* UP: row-- */
        PlayGlobalSound(3, 0, 0);
        row = *(s32 *)((char *)w + 0x4AC) - 1;
        *(s32 *)((char *)w + 0x4AC) = row;
        if (row < 0) {
            *(s32 *)((char *)w + 0x4AC) = 3;
            *(s32 *)((char *)w + 0x4B0) = 1;
            *(s32 *)((char *)w + 0x4B8) = 2;
        }
    } else if (flags & 0x2000) {     /* DOWN: row++ wrap at 2 */
        PlayGlobalSound(3, 0, 0);
        row = *(s32 *)((char *)w + 0x4AC) + 1;
        *(s32 *)((char *)w + 0x4AC) = row;
        if (row >= 2) {
            *(s32 *)((char *)w + 0x4AC) = 0;
        }
    }

    col = *(s32 *)((char *)w + 0x4B0);
    row = *(s32 *)((char *)w + 0x4AC);
    *(s32 *)((char *)w + 0x4B4) = row + col * 2;
}
#endif

/* func_003451B8: weapon-select mode-1 (single-row) input handler. 2nd arg = D-pad
 * mask. Each direction sets a fixed selection state and plays move sound 3:
 * left (0x1000) -> page +0x4B8=2, col +0x4B0=1, row +0x4AC=3; right (0x4000) ->
 * page 2, row 3, col 0; up (0x8000) -> page 0, row 1, col 0; down (0x2000) ->
 * col 0, page 3, row 0. Records the fixed table halfword D_1AA856 at +0x4B4. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003451B8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003451B8, unit objdiff): 81.20%,
   27/60 insns differ. Residual: UNKNOWN-sd (first differing insn: 'sd ra, 0x8(sp)' vs 'daddu s0, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern s16 D_1AA856;
void func_003451B8(void *w, void *a) {
    s32 flags = (s32)a;

    if (flags & 0x1000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4B8) = 2;
        *(s32 *)((char *)w + 0x4B0) = 1;
        *(s32 *)((char *)w + 0x4AC) = 3;
    } else if (flags & 0x4000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4B8) = 2;
        *(s32 *)((char *)w + 0x4AC) = 3;
        *(s32 *)((char *)w + 0x4B0) = 0;
    } else if (flags & 0x8000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4B8) = 0;
        *(s32 *)((char *)w + 0x4AC) = 1;
        *(s32 *)((char *)w + 0x4B0) = 0;
    } else if (flags & 0x2000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4B0) = 0;
        *(s32 *)((char *)w + 0x4B8) = 3;
        *(s32 *)((char *)w + 0x4AC) = 0;
    }

    *(s32 *)((char *)w + 0x4B4) = D_1AA856;
}
#endif

/* func_00345298: read the 16-bit field_0x42 of the weapon variant currently
 * selected on the weapon-select screen `w`, dispatched by screen mode (+0x4B8):
 *   mode 0: id = selection (+0x4B4) -> generic owned-variant lookup.
 *   mode 1: fixed item 6 (only if owned).
 *   mode 2: id = selection; the item-7 special case (id==7, owns item 0x31 but
 *           not item 7) reads the 0x31 variant instead, else the generic lookup.
 *   mode 3: category-table entry D_1AE238[selection], returned directly.
 *   otherwise, or unowned: 0.
 * The owned-variant lookup mirrors func_003453D0:
 * g_weaponTable[g_itemEquippedSlot[id]*0xE0 + 0x42] (signed 16-bit). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00345298);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00345298, unit objdiff): 11.86%,
   97/101 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x10' vs '').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern s32 D_1AE238[];
s32 func_00345298(void *w) {
    s32 mode = *(s32 *)((char *)w + 0x4B8);
    s32 id;

    if (mode == 1) {
        if (g_inventoryOwned[6] == 0) {
            return 0;
        }
        return *(s16 *)&g_weaponTable[g_itemEquippedSlot[6] * 0xE0 + 0x42];
    }
    if (mode == 3) {
        return D_1AE238[*(s32 *)((char *)w + 0x4B4)];
    }
    if (mode == 0) {
        id = *(s32 *)((char *)w + 0x4B4);
    } else if (mode == 2) {
        id = *(s32 *)((char *)w + 0x4B4);
        if (id == 7 && g_inventoryOwned[0x31] != 0 && g_inventoryOwned[7] == 0) {
            return *(s16 *)&g_weaponTable[g_itemEquippedSlot[0x31] * 0xE0 + 0x42];
        }
    } else {
        return 0;   /* mode < 0 or mode >= 4 */
    }

    /* generic owned-variant lookup on the selected id */
    if (g_inventoryOwned[id] == 0) {
        return 0;
    }
    return *(s16 *)&g_weaponTable[g_itemEquippedSlot[id] * 0xE0 + 0x42];
}
#endif

/* Build the weapon-select screen. Reset the selection (+0x4B4), clear the caption
 * id (D_1ADAF0=-1), apply the equipped-armor upgrade slot, then dispatch on the
 * screen mode (+0x4B8) to the per-mode sub-builder. Recompute the caption id from
 * either the D_1AE248 category table (mode ended at 3) or the owned weapon's
 * nameStringId (g_inventoryOwned -> g_weaponTable[g_itemEquippedSlot]+0x6). Lay out
 * the nine panel sub-elements at the anchor (*(w+0x4A0)) + their D_1AE2xx offsets,
 * pulse + colour the caption sprite (+0x17C), and draw the panel tail. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003453D0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003453D0, unit objdiff): 52.79%,
   201/277 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x40' vs 'addiu sp, sp, -0x20').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_003453D0(void *w, void *arg1) {
    s32 mode, result, idx;
    f32 *anchor;

    *(s32 *)((char *)w + 0x4B4) = 0;
    D_1ADAF0 = -1;
    SetWeaponUpgradeSlot(6, g_equippedArmor);

    mode = *(s32 *)((char *)w + 0x4B8);
    if (mode == 1)      func_003451B8(w, arg1);
    else if (mode == 0) func_00344E08(w, arg1);
    else if (mode == 2) func_00344F18(w, arg1);
    else if (mode == 3) func_00345080(w, arg1);
    result = *(s32 *)((char *)w + 0x4B8);

    if (result != 3) {
        idx = *(s32 *)((char *)w + 0x4B4);
        if (g_inventoryOwned[idx] != 0) {
            D_1ADAF0 = *(s16 *)&g_weaponTable[g_itemEquippedSlot[idx] * 0xE0 + 0x6];
        }
    } else {
        D_1ADAF0 = D_1AE248[*(s32 *)((char *)w + 0x4B4)];
    }

    anchor = *(f32 **)((char *)w + 0x4A0);
    GuiElementSetPos((GuiElement *)w, D_1AE258 + anchor[0], D_1AE25C + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x4A0);
    GuiElementSetPos((GuiElement *)((char *)w + 0x4C), D_1AE278 + anchor[0], D_1AE27C + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x4A0);
    GuiElementSetPos((GuiElement *)((char *)w + 0x98), D_1AE260 + anchor[0], D_1AE264 + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x4A0);
    GuiElementSetPos((GuiElement *)((char *)w + 0xE4), D_1AE268 + anchor[0], D_1AE26C + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x4A0);
    GuiElementSetPos((GuiElement *)((char *)w + 0x130), D_1AE270 + anchor[0], D_1AE274 + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x4A0);
    GuiElementSetPos((GuiElement *)((char *)w + 0x1C8), D_1AE280[0] + anchor[0], D_1AE280[1] + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x4A0);
    GuiElementSetPos((GuiElement *)((char *)w + 0x2B8), D_1AE288 + anchor[0], D_1AE28C + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x4A0);
    GuiElementSetPos((GuiElement *)((char *)w + 0x220), D_1AE290 + anchor[0], D_1AE294 + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x4A0);
    GuiElementSetPos((GuiElement *)((char *)w + 0x17C), D_1AE290 + anchor[0], D_1AE294 + anchor[1], 0.0f, 0.0f);

    if (g_padButtonsPressed & 0xF000) {
        func_002AA3F0(0, 0, 1, 0, 1);
    }
    *GuiElementGetColor((GuiElement *)((char *)w + 0x17C)) =
        func_002AA3F0(0x60442D00, 0x70FFFEED, 0x14, 0, 0);
    func_003457A0(w);
}
#endif

/* func_003457A0: weapon-panel tail layout. Positions four elements at fixed
 * data-driven offsets from the shared anchor record *(w+0x4A0): +0x368 at
 * (D_1AE298, D_1AE29C), +0x3C0 at (D_1AE2A0, D_1AE2A4), +0x418 at
 * (D_1AE2A8, D_1AE2AC), and +0x310 at (D_1AE2B0, D_1AE2B4). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003457A0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003457A0, unit objdiff): 54.53%,
   49/70 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s0, 0x0(sp)' vs 'sd s1, 0x8(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern f32 D_1AE298, D_1AE29C, D_1AE2A0, D_1AE2A4;
extern f32 D_1AE2A8, D_1AE2AC, D_1AE2B0, D_1AE2B4;
void func_003457A0(void *w) {
    f32 *anchor = *(f32 **)((char *)w + 0x4A0);

    GuiElementSetPos((GuiElement *)((char *)w + 0x368), D_1AE298 + anchor[0], D_1AE29C + anchor[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x3C0), D_1AE2A0 + anchor[0], D_1AE2A4 + anchor[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x418), D_1AE2A8 + anchor[0], D_1AE2AC + anchor[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x310), D_1AE2B0 + anchor[0], D_1AE2B4 + anchor[1], 0.0f, 0.0f);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00345890);

/*
 * func_00345F00 / func_00345FF0 / func_003460E0 / func_003461D0 — four sibling
 * GUI-screen builders. Each fills a 0x5C-byte screen descriptor on the stack
 * from the owning object `obj`'s config fields plus a per-builder label const
 * and a 7-entry block of layout globals, then hands the descriptor to
 * func_003380B8 (the populator that instantiates the screen's GUI elements).
 * The four differ ONLY in their label const + their global block; the descriptor
 * shape and the obj field offsets are identical. Save-wall blocked for matching
 * ($16-$19/$31 packed 8-byte); provided as TARGET_NATIVE #else arms, cmp-oracle'd
 * (cmp_235FE8_iso.c mocks func_003380B8 and byte-compares the built descriptor
 * vs the original asm).
 */
typedef struct GuiScreenDesc {
    void *label;        /* 0x00  &labelConst */
    s32   cfg0;         /* 0x04  globalBlock[0] (by value) */
    s32   cfg1;         /* 0x08  globalBlock[1] (by value) */
    s32   objCfg4AC;    /* 0x0C  obj[0x4AC] */
    s32   objCfg4B0;    /* 0x10  obj[0x4B0] */
    void *layout0;      /* 0x14  &globalBlock[2] */
    void *layout1;      /* 0x18  &globalBlock[3] */
    void *layout2;      /* 0x1C  &globalBlock[4] */
    void *layout3;      /* 0x20  &globalBlock[5] */
    void *layout4;      /* 0x24  &globalBlock[6] */
    void *child130;     /* 0x28  obj+0x130 */
    s32   _z2C;         /* 0x2C  0 */
    void *child17C;     /* 0x30  obj+0x17C */
    s32   _z34;         /* 0x34  0 */
    s32   _z38;         /* 0x38  0 */
    void *child278;     /* 0x3C  obj+0x278 */
    s32   _z40;         /* 0x40  0 */
    void *child220;     /* 0x44  obj+0x220 */
    s32   objCfg4A0;    /* 0x48  obj[0x4A0] */
    s32   objCfg4B4;    /* 0x4C  obj[0x4B4] */
    s32   _z50;         /* 0x50  0 */
    s32   typeActive;   /* 0x54  obj[0x4B8] == this builder's screen-type index */
    s32   enable;       /* 0x58  1 */
} GuiScreenDesc;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(GuiScreenDesc) == 0x5C, "GuiScreenDesc must be 0x5C under ILP32");
#endif

extern void func_003380B8(void *desc);   /* GUI screen populator (consumer) */

/* per-builder label consts (address-taken) */
extern u8 D_25E308[], D_1AA850[], D_1AA890[], D_1AA830[];
/* per-builder 7-global layout blocks: first two read by VALUE, last five by ADDRESS */
extern s32 D_1AE2F0, D_1AE2F4; extern u8 D_1AE2F8[8], D_1AE300[8], D_1AE308[8], D_1AE310[8], D_1AE318[8];
extern s32 D_1AE320, D_1AE324; extern u8 D_1AE328[8], D_1AE330[8], D_1AE338[8], D_1AE340[8], D_1AE348[8];
extern s32 D_1AE350, D_1AE354; extern u8 D_1AE358[8], D_1AE360[8], D_1AE368[8], D_1AE370[8], D_1AE378[8];
extern s32 D_1AE380, D_1AE384; extern u8 D_1AE388[8], D_1AE390[8], D_1AE398[8], D_1AE3A0[8], D_1AE3A8[8];

#ifdef TARGET_NATIVE
/* Shared descriptor builder (the four siblings are one shape; see doc above). */
static void GuiBuildScreenDesc(u8 *obj, void *label, s32 cfg0, s32 cfg1,
                               void *l0, void *l1, void *l2, void *l3, void *l4,
                               s32 typeIndex) {
    GuiScreenDesc d;
    d.label = label;
    d.cfg0 = cfg0;
    d.cfg1 = cfg1;
    d.objCfg4AC = *(s32 *)(obj + 0x4AC);
    d.objCfg4B0 = *(s32 *)(obj + 0x4B0);
    d.layout0 = l0; d.layout1 = l1; d.layout2 = l2; d.layout3 = l3; d.layout4 = l4;
    d.child130 = obj + 0x130;
    d._z2C = 0;
    d.child17C = obj + 0x17C;
    d._z34 = 0;
    d._z38 = 0;
    d.child278 = obj + 0x278;
    d._z40 = 0;
    d.child220 = obj + 0x220;
    d.objCfg4A0 = *(s32 *)(obj + 0x4A0);
    d.objCfg4B4 = *(s32 *)(obj + 0x4B4);
    d._z50 = 0;
    d.typeActive = (*(s32 *)(obj + 0x4B8) == typeIndex);
    d.enable = 1;
    func_003380B8(&d);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00345F00);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00345F00, unit objdiff): 25.22%,
   58/60 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x90' vs 'lw a2, %gp_rel(D_1AE2F0)(gp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00345F00(void *obj) {
    GuiBuildScreenDesc((u8 *)obj, D_25E308, D_1AE2F0, D_1AE2F4,
                       D_1AE2F8, D_1AE300, D_1AE308, D_1AE310, D_1AE318, 2);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00345FF0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00345FF0, unit objdiff): 25.22%,
   58/60 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x90' vs 'lw a2, %gp_rel(D_1AE320)(gp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00345FF0(void *obj) {
    GuiBuildScreenDesc((u8 *)obj, D_1AA850, D_1AE320, D_1AE324,
                       D_1AE328, D_1AE330, D_1AE338, D_1AE340, D_1AE348, 1);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003460E0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003460E0, unit objdiff): 25.22%,
   58/60 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x90' vs 'lw a2, %gp_rel(D_1AE350)(gp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_003460E0(void *obj) {
    GuiBuildScreenDesc((u8 *)obj, D_1AA890, D_1AE350, D_1AE354,
                       D_1AE358, D_1AE360, D_1AE368, D_1AE370, D_1AE378, 3);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003461D0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003461D0, unit objdiff): 25.05%,
   57/59 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x90' vs 'lw a2, %gp_rel(D_1AE380)(gp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_003461D0(void *obj) {
    GuiBuildScreenDesc((u8 *)obj, D_1AA830, D_1AE380, D_1AE384,
                       D_1AE388, D_1AE390, D_1AE398, D_1AE3A0, D_1AE3A8, 0);
}
#endif

/* func_003462C0: draw a map/info screen. Bails if hidden (*(w+0x4A8)==0). Draws
 * four sprite elements (w/+0x4C/+0x98/+0xE4) and three text rows (+0x1C8, +0x2B8,
 * +0x1C8 again), hides +0x220, then runs the five sub-builders func_003461D0/
 * 00345FF0/00345F00/003460E0/00345890 and draws the detail sprite (+0x17C). */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_003462C0)
S136OS_SLOT(func_003462C0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003462C0, unit objdiff): 85.71%,
   6/45 insns differ. Residual: UNKNOWN-daddu (first differing insn: '' vs 'daddu s1, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
void func_003461D0(void *w);
void func_00345FF0(void *w);
void func_00345F00(void *w);
void func_003460E0(void *w);
void func_00345890(void *w);
void func_003462C0(void *w) {
    if (*(s32 *)((char *)w + 0x4A8) == 0) {
        return;
    }
    GuiSpriteElementDraw(w);
    GuiSpriteElementDraw((char *)w + 0x4C);
    GuiSpriteElementDraw((char *)w + 0x98);
    GuiSpriteElementDraw((char *)w + 0xE4);
    GuiTextElementDraw((char *)w + 0x1C8);
    GuiTextElementDraw((char *)w + 0x2B8);
    GuiTextElementDraw((char *)w + 0x1C8);
    GuiElementSetVisible((GuiElement *)((char *)w + 0x220), 0);
    func_003461D0(w);
    func_00345FF0(w);
    func_00345F00(w);
    func_003460E0(w);
    func_00345890(w);
    GuiSpriteElementDraw((char *)w + 0x17C);
}
#endif

/* Construct a composite widget: init six TypeB sub-elements (base, +0x4C, +0x98,
 * +0xE4, +0x130, +0x17C), two TypeC (+0x1C8, +0x220), a sub-list (+0x278), then
 * five more TypeC (+0x2B8, +0x310, +0x368, +0x3C0, +0x418); returns the widget. */
#if defined(S136OS_func_00346368) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_00346368; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 36/36 words + 14 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_00346368 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_00346368(void *w) {
    GuiElementInitTypeB(w);
    GuiElementInitTypeB((char *)w + 0x4C);
    GuiElementInitTypeB((char *)w + 0x98);
    GuiElementInitTypeB((char *)w + 0xE4);
    GuiElementInitTypeB((char *)w + 0x130);
    GuiElementInitTypeB((char *)w + 0x17C);
    GuiElementInitTypeC((char *)w + 0x1C8);
    GuiElementInitTypeC((char *)w + 0x220);
    func_003374D8((char *)w + 0x278);
    GuiElementInitTypeC((char *)w + 0x2B8);
    GuiElementInitTypeC((char *)w + 0x310);
    GuiElementInitTypeC((char *)w + 0x368);
    GuiElementInitTypeC((char *)w + 0x3C0);
    GuiElementInitTypeC((char *)w + 0x418);
    return w;
}
#else
S136OS_SLOT(func_00346368);
#endif

/* GuiMapScreenInit: construct the map screen — six button-glyph elements
 * (+0x0/+0x4C/+0x98/+0xE4/+0x130/+0x17C), seven text rows, and a sprite (+0x278).
 * Pool -> alloc the 0x10-byte placement record (+0x470), +0x474=pool + +0x4F8=1
 * unconditional; record seeded 250x207. Button art D_1ADBE8/BF0/BF8/C00/DF98/DFA0;
 * text rows D_1ADD40/DD38/AE3B0/AE3C0/AE3D0/DD30/AE008. Buttons coloured
 * (0x60442D00/0x55F0C070 x3/0x70FFFEED/0x60241700) + glyphs 0x59..0x5E; +0x17C
 * hidden; text rows 0x80F0F0F0; state fields +0x4FC/+0x500/+0x508/+0x504=0. Sprite
 * scaled 32x32 (0x60F0F0B0). Row texts localized (0x2BE8/0x2BE5/0x2BF1/0x2BED/
 * 0x2BEE/0x2BF0) + the empty +0x4B8 buffer; two rows right-flagged. Closed by
 * GuiMapScreenTick(w, 0, &prevSel) — the 3rd arg is a throwaway out-pointer the
 * tick writes the previous list-selection index into (traced: it only writes
 * *(out+0), never reads it; the caller discards it). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiMapScreenInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiMapScreenInit, unit objdiff): 49.94%,
   225/349 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s7, 0x68(sp)' vs 'sd s2, 0x40(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void GuiMapScreenTick(void *w, s32 mode, s32 *out);
extern char *g_guiInstance;
extern u8 D_1ADBE8[], D_1ADBF0[], D_1ADBF8[], D_1ADC00[], D_1ADF98[], D_1ADFA0[];
extern u8 D_1ADD40[], D_1ADD38[], D_1AE3B0[], D_1AE3C0[], D_1AE3D0[], D_1ADD30[];
extern u8 D_1AE008[], D_1ADFB0[];
void GuiMapScreenInit(void *w, GuiPool *pool) {
    GuiElement *e0 = (GuiElement *)((char *)w + 0x0);
    GuiElement *e1 = (GuiElement *)((char *)w + 0x4C);
    GuiElement *e2 = (GuiElement *)((char *)w + 0x98);
    GuiElement *e3 = (GuiElement *)((char *)w + 0xE4);
    GuiElement *e4 = (GuiElement *)((char *)w + 0x130);
    GuiElement *e5 = (GuiElement *)((char *)w + 0x17C);
    GuiElement *t2B8 = (GuiElement *)((char *)w + 0x2B8);
    GuiElement *t310 = (GuiElement *)((char *)w + 0x310);
    GuiElement *t368 = (GuiElement *)((char *)w + 0x368);
    GuiElement *t3C0 = (GuiElement *)((char *)w + 0x3C0);
    GuiElement *t418 = (GuiElement *)((char *)w + 0x418);
    GuiElement *t1C8 = (GuiElement *)((char *)w + 0x1C8);
    GuiElement *t220 = (GuiElement *)((char *)w + 0x220);
    GuiElement *sprite = (GuiElement *)((char *)w + 0x278);
    void *rec;
    s32 prevSel; /* GuiMapScreenTick out-param (discarded) */

    /* +0x474 = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x474) = pool;
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x470) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    *(s32 *)((char *)w + 0x4F8) = 1;
    rec = *(void **)((char *)w + 0x470);
    *(f32 *)((char *)rec + 0x0) = 250.0f;
    *(f32 *)((char *)rec + 0x4) = 207.0f;

    GuiElementInit(e0, (s32)D_1ADBE8, pool);
    GuiElementInit(e1, (s32)D_1ADBF0, pool);
    GuiElementInit(e2, (s32)D_1ADBF8, pool);
    GuiElementInit(e3, (s32)D_1ADC00, pool);
    GuiElementInit(e4, (s32)D_1ADF98, pool);
    GuiElementInit(e5, (s32)D_1ADFA0, pool);
    GuiTextElementInit(t2B8, (s32)D_1ADD40, pool);
    GuiTextElementInit(t310, (s32)D_1ADD38, pool);
    GuiTextElementInit(t368, (s32)D_1AE3B0, pool);
    GuiTextElementInit(t3C0, (s32)D_1AE3C0, pool);
    GuiTextElementInit(t418, (s32)D_1AE3D0, pool);
    GuiTextElementInit(t1C8, (s32)D_1ADD30, pool);
    GuiTextElementInit(t220, (s32)D_1AE008, pool);

    *GuiElementGetColor(e0) = 0x60442D00;
    *GuiElementGetColor(e2) = 0x55F0C070;
    *GuiElementGetColor(e3) = 0x55F0C070;
    *GuiElementGetColor(e4) = 0x55F0C070;
    *GuiElementGetColor(e5) = 0x70FFFEED;
    *GuiElementGetColor(e1) = 0x60241700;
    *GuiElementGetColor(t310) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t2B8) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t368) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t3C0) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t418) = (s32)0x80F0F0F0;

    GuiElementSetGlyph(e0, (s32)(g_guiInstance + 0x8710), 0x59);
    GuiElementSetGlyph(e2, (s32)(g_guiInstance + 0x8710), 0x5B);
    GuiElementSetGlyph(e3, (s32)(g_guiInstance + 0x8710), 0x5C);
    GuiElementSetGlyph(e4, (s32)(g_guiInstance + 0x8710), 0x5D);
    GuiElementSetGlyph(e5, (s32)(g_guiInstance + 0x8710), 0x5E);
    GuiElementSetGlyph(e1, (s32)(g_guiInstance + 0x8710), 0x5A);

    GuiElementSetVisible(e5, 0);
    *(s32 *)((char *)w + 0x4FC) = 0;
    *(s32 *)((char *)w + 0x500) = 0;
    *(s32 *)((char *)w + 0x508) = 0;
    GuiSpriteElementInit(sprite, (s32)D_1ADFB0, pool);
    *GuiElementGetColor(sprite) = 0x60F0F0B0;
    GuiElementSetScale(sprite, 32.0f, 32.0f, 0.0f, 0.0f);

    *GuiElementGetColor(t1C8) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t220) = (s32)0x80F0F0F0;
    GuiElementSetText(t1C8, GetLocalizedString(0x2BE8));
    *(u8 *)((char *)w + 0x4B8) = 0;
    GuiElementSetText(t220, (s32)((char *)w + 0x4B8));
    *(s32 *)((char *)w + 0x504) = 0;
    GuiElementSetText(t310, GetLocalizedString(0x2BE5));
    GuiElementSetText(t2B8, GetLocalizedString(0x2BF1));
    GuiElementSetTextFlag(t310, 1);
    GuiElementSetTextFlag(t2B8, 1);
    GuiElementSetText(t368, GetLocalizedString(0x2BED));
    GuiElementSetText(t3C0, GetLocalizedString(0x2BEE));
    GuiElementSetText(t418, GetLocalizedString(0x2BF0));

    GuiMapScreenTick(w, 0, &prevSel);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00346878);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00346878, unit objdiff): 34.25%,
   164/206 insns differ. Residual: UNKNOWN-daddu + movn/movz (first differing insn: '' vs 'daddu s0, a0, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8   D_259CC0[];              /* 2D grid entry table, stride 0xA, +6 = s16 item id */
extern u8   g_menuTransitionMode[];  /* 0x1F27DC - +0x24 receives the activated item id */
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);
/*
 * func_00346878(w, flags) — the mode-0 sub-builder: a 2D-grid cursor/confirm
 * handler for the D_259CC0 select grid `w` (3 columns wide; grid index =
 * w+0x4FC column [0..2] + 3 * w+0x500 row, record stride 0xA). Held-button
 * `flags` move the cursor, each with the wheel cue PlayGlobalSound(3):
 *   0x1000 up    -> row--, underflow clamps row to 0 and sets w+0x508=1
 *   0x4000 down  -> from row 0: enter row 1 (seeding col if col==0); from row 1:
 *                   set w+0x508=3, back to row 0 and col--
 *   0x8000 left  -> col-- with wrap to 2 (row 0 keeps col 0, row!=0 wraps at 0)
 *   0x2000 right -> col++ with wrap (>2 -> 0 on row 0, -> 1 on row!=0)
 *   0x40  confirm-> owned highlighted item: accept cue (4), record in
 *                   g_menuTransitionMode+0x24 and w+0x504; else deny cue (5)
 * Every path then refreshes w+0x504 and, for an owned item, its weapon-name
 * caption id D_1ADAF0 = g_weaponTable[g_itemEquippedSlot[entry]*0xE0 + 6].
 */
void func_00346878(void *w, s32 flags) {
    s16 entry;
    s32 idx;

    if (flags & 0x1000) {
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x500) - 1;
        *(s32 *)((char *)w + 0x500) = v;
        if (v < 0) {
            *(s32 *)((char *)w + 0x500) = 0;
            *(s32 *)((char *)w + 0x508) = 1;
        }
    } else if (flags & 0x4000) {
        s32 s500;
        PlayGlobalSound(3, 0, 0);
        s500 = *(s32 *)((char *)w + 0x500);
        if (s500 == 0) {
            if (*(s32 *)((char *)w + 0x4FC) == 0) {
                *(s32 *)((char *)w + 0x500) = 1;
                *(s32 *)((char *)w + 0x4FC) = 1;
            } else {
                *(s32 *)((char *)w + 0x500) = 1;
            }
        } else if (s500 == 1) {
            *(s32 *)((char *)w + 0x508) = 3;
            *(s32 *)((char *)w + 0x500) = 0;
            *(s32 *)((char *)w + 0x4FC) -= 1;
        }
    } else if (flags & 0x8000) {
        s32 c;
        PlayGlobalSound(3, 0, 0);
        c = *(s32 *)((char *)w + 0x4FC) - 1;
        if (*(s32 *)((char *)w + 0x500) == 0) {
            *(s32 *)((char *)w + 0x4FC) = (c >= 0) ? c : 2;
        } else {
            *(s32 *)((char *)w + 0x4FC) = (c > 0) ? c : 2;
        }
    } else if (flags & 0x2000) {
        s32 c;
        PlayGlobalSound(3, 0, 0);
        c = *(s32 *)((char *)w + 0x4FC) + 1;
        if (*(s32 *)((char *)w + 0x500) == 0) {
            *(s32 *)((char *)w + 0x4FC) = (c <= 2) ? c : 0;
        } else {
            *(s32 *)((char *)w + 0x4FC) = (c <= 2) ? c : 1;
        }
    } else if (flags & 0x40) {
        s16 sel;
        idx = *(s32 *)((char *)w + 0x4FC) + 3 * *(s32 *)((char *)w + 0x500);
        sel = *(s16 *)&D_259CC0[idx * 0xA + 6];
        if (sel == 0 || g_inventoryOwned[sel] == 0) {
            PlayGlobalSound(5, 0, 0);
        } else {
            PlayGlobalSound(4, 0, 0);
            *(s32 *)&g_menuTransitionMode[0x24] = sel;
            *(s32 *)((char *)w + 0x504) = sel;
        }
    }

    idx = *(s32 *)((char *)w + 0x4FC) + 3 * *(s32 *)((char *)w + 0x500);
    entry = *(s16 *)&D_259CC0[idx * 0xA + 6];
    *(s32 *)((char *)w + 0x504) = entry;
    if (entry != 0 && g_inventoryOwned[entry] != 0) {
        D_1ADAF0 = *(s16 *)&g_weaponTable[g_itemEquippedSlot[entry] * 0xE0 + 6];
    }
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00346AF8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00346AF8, unit objdiff): 31.61%,
   120/156 insns differ. Residual: UNKNOWN-sd + gp/abs-mixed symbol (first differing insn: 'sd s0, 0x0(sp)' vs 'sd s1, 0x8(sp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8   D_1AA7F8[];              /* select-screen entry table, stride 0xA, +6 = s16 item id */
extern u8   g_menuTransitionMode[];  /* 0x1F27DC - +0x30 receives the activated item id */
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);
/*
 * func_00346AF8(w, flags) — the mode-3 sub-builder: a two-axis cursor/confirm
 * handler for the D_1AA7F8 select screen `w`. `flags` (held-button bits) drives
 * the cursor index at w+0x4FC and the sub-state at w+0x500/+0x508, each with the
 * wheel-turn cue PlayGlobalSound(3):
 *   0x1000 -> advance cursor, set sub-state 0x500=1 / 0x508=0
 *   0x4000 -> advance cursor, set sub-state 0x508=1 / 0x500=0
 *   0x8000 -> retreat cursor (wrap to 1 on underflow)
 *   0x2000 -> advance cursor (wrap to 0 at 2)
 *   0x40   -> confirm: for the highlighted owned item play the accept cue (4),
 *             record it in g_menuTransitionMode+0x30 and w+0x504; otherwise the
 *             deny cue (5).
 * Every path then refreshes the highlighted entry (w+0x504) and, for an owned
 * item, its weapon-name caption id D_1ADAF0 = g_weaponTable[slot*0xE0 + 6].
 */
void func_00346AF8(void *w, s32 flags) {
    s16 entry;

    if (flags & 0x1000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x500) = 1;
        *(s32 *)((char *)w + 0x508) = 0;
        *(s32 *)((char *)w + 0x4FC) += 1;
    } else if (flags & 0x4000) {
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x508) = 1;
        *(s32 *)((char *)w + 0x500) = 0;
        *(s32 *)((char *)w + 0x4FC) += 1;
    } else if (flags & 0x8000) {
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x4FC) - 1;
        *(s32 *)((char *)w + 0x4FC) = v;
        if (v < 0) {
            *(s32 *)((char *)w + 0x4FC) = 1;
        }
    } else if (flags & 0x2000) {
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x4FC) + 1;
        *(s32 *)((char *)w + 0x4FC) = v;
        if (v >= 2) {
            *(s32 *)((char *)w + 0x4FC) = 0;
        }
    } else if (flags & 0x40) {
        s16 sel = *(s16 *)&D_1AA7F8[*(s32 *)((char *)w + 0x4FC) * 0xA + 6];
        if (sel == 0 || g_inventoryOwned[sel] == 0) {
            PlayGlobalSound(5, 0, 0);
        } else {
            PlayGlobalSound(4, 0, 0);
            *(s32 *)&g_menuTransitionMode[0x30] = sel;
            *(s32 *)((char *)w + 0x504) = sel;
        }
    }

    entry = *(s16 *)&D_1AA7F8[*(s32 *)((char *)w + 0x4FC) * 0xA + 6];
    *(s32 *)((char *)w + 0x504) = entry;
    if (entry != 0 && g_inventoryOwned[entry] != 0) {
        D_1ADAF0 = *(s16 *)&g_weaponTable[g_itemEquippedSlot[entry] * 0xE0 + 6];
    }
}
#endif

/* func_00346CD8: d-pad + confirm handler for the weapon-select screen `w`.
 * `flags` (held-button bits) drives the single-axis cursor at +0x4FC over the
 * 3-entry D_1AA8B8 table: up (0x1000) decrements clamped at 0 (and resets the
 * +0x500 sub-state, sets +0x508=3); down (0x4000) just resets +0x500/+0x508;
 * left (0x8000) decrements wrapping to 2; right (0x2000) increments wrapping to
 * 0. Confirm (0x40): if the cursor's entry is owned, play the accept sound and
 * toggle it as the selected weapon (g_menuScreenBlock+0x44) while refreshing the
 * caption id D_1ADAF0; otherwise play the deny sound. Every path then refreshes
 * +0x504 = current entry and, when owned, the caption id from
 * g_weaponTable[g_itemEquippedSlot[entry]*0xE0 + 0x6]. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00346CD8);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00346CD8, unit objdiff): 13.55%,
   162/186 insns differ. Residual: UNKNOWN-addiu + movn/movz, gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x20' vs 'addiu sp, sp, -0x30').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern u8 D_1AA8B8[];
extern u8 g_menuScreenBlock[];
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);
void func_00346CD8(void *w, s32 flags) {
    s32 entry;

    if (flags & 0x1000) {          /* up: decrement, clamp at 0 */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x500) = 0;
        *(s32 *)((char *)w + 0x508) = 3;
        v = *(s32 *)((char *)w + 0x4FC) - 1;
        if (v < 0) {
            v = 0;
        }
        *(s32 *)((char *)w + 0x4FC) = v;
    } else if (flags & 0x4000) {   /* down: reset sub-state only */
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x500) = 0;
        *(s32 *)((char *)w + 0x508) = 0;
    } else if (flags & 0x8000) {   /* left: decrement, wrap to 2 */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x4FC) - 1;
        if (v < 0) {
            v = 2;
        }
        *(s32 *)((char *)w + 0x4FC) = v;
    } else if (flags & 0x2000) {   /* right: increment, wrap to 0 */
        s32 v;
        PlayGlobalSound(3, 0, 0);
        v = *(s32 *)((char *)w + 0x4FC) + 1;
        if (v >= 3) {
            v = 0;
        }
        *(s32 *)((char *)w + 0x4FC) = v;
    } else if (flags & 0x40) {     /* confirm: toggle-select the entry */
        entry = *(s16 *)&D_1AA8B8[*(s32 *)((char *)w + 0x4FC) * 0xA + 0x6];
        if (entry != 0 && g_inventoryOwned[entry] != 0) {
            s32 slot = g_itemEquippedSlot[*(s32 *)((char *)w + 0x504)];
            PlayGlobalSound(4, 0, 0);
            D_1ADAF0 = *(s16 *)&g_weaponTable[slot * 0xE0 + 0x6];
            if (*(s32 *)((char *)g_menuScreenBlock + 0x44) == entry) {
                *(s32 *)((char *)g_menuScreenBlock + 0x44) = 0;
            } else {
                *(s32 *)((char *)g_menuScreenBlock + 0x44) = entry;
            }
        } else {
            PlayGlobalSound(5, 0, 0);
        }
    }

    /* tail (all paths): refresh the current entry + caption id */
    entry = *(s16 *)&D_1AA8B8[*(s32 *)((char *)w + 0x4FC) * 0xA + 0x6];
    *(s32 *)((char *)w + 0x504) = entry;
    if (entry != 0 && g_inventoryOwned[entry] != 0) {
        D_1ADAF0 = *(s16 *)&g_weaponTable[g_itemEquippedSlot[entry] * 0xE0 + 0x6];
    }
}
#endif

/* GuiMapScreenTick: per-frame tick for the map screen `w`. Latches the current
 * mode-state (w+0x508) into *out, clears the transient at w+0x504, resets the
 * caption id (D_1ADAF0=-1), then forwards (w, mode) to the per-state sub-builder
 * — 0 -> func_00346878, 1 -> func_00346CD8, 3 -> func_00346AF8 (other states do
 * nothing). A d-pad press (g_padButtonsPressed & 0xF000) fires a UI feedback
 * pulse via func_002AA3F0; the caption sprite colour (+0x17C) is refreshed to a
 * pulsing blend. Finally lays out 11 panel sub-elements at the anchor record
 * *(w+0x470) plus their fixed (x,y) offset pairs (D_1AE3D8.. and the stack-copied
 * D_1AE280 for the +0x1C8 element). Mirrors the weapon-screen builder
 * func_003453D0. Returns nothing (the register-0 return is discarded). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiMapScreenTick);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiMapScreenTick, unit objdiff): 59.50%,
   166/241 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x40' vs 'addiu sp, sp, -0x20').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00346878(void *w, s32 flags);   /* mode-0 sub-builder */
extern void func_00346AF8(void *w, s32 flags);   /* mode-3 sub-builder */
extern f32 D_1AE3D8, D_1AE3DC, D_1AE3E0, D_1AE3E4, D_1AE3E8, D_1AE3EC;
extern f32 D_1AE3F0, D_1AE3F4, D_1AE3F8, D_1AE3FC;
extern f32 D_1AE400, D_1AE404, D_1AE408, D_1AE40C, D_1AE410, D_1AE414;
extern f32 D_1AE418, D_1AE41C, D_1AE420, D_1AE424;
void GuiMapScreenTick(void *w, s32 mode, s32 *out) {
    s32 state = *(s32 *)((char *)w + 0x508);
    f32 *anchor;

    *out = state;
    *(s32 *)((char *)w + 0x504) = 0;
    D_1ADAF0 = -1;

    switch (state) {
        case 0: func_00346878(w, mode); break;
        case 1: func_00346CD8(w, mode); break;
        case 3: func_00346AF8(w, mode); break;
        default: break;
    }

    if (g_padButtonsPressed & 0xF000) {
        func_002AA3F0(0, 0, 1, 0, 1);
    }
    *GuiElementGetColor((GuiElement *)((char *)w + 0x17C)) =
        func_002AA3F0(0x60442D00, 0x70FFFEED, 0x14, 0, 0);

    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)w, D_1AE3D8 + anchor[0], D_1AE3DC + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)((char *)w + 0x4C), D_1AE3F8 + anchor[0], D_1AE3FC + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)((char *)w + 0x98), D_1AE3E0 + anchor[0], D_1AE3E4 + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)((char *)w + 0xE4), D_1AE3E8 + anchor[0], D_1AE3EC + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)((char *)w + 0x130), D_1AE3F0 + anchor[0], D_1AE3F4 + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)((char *)w + 0x1C8), D_1AE280[0] + anchor[0], D_1AE280[1] + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)((char *)w + 0x310), D_1AE408 + anchor[0], D_1AE40C + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)((char *)w + 0x2B8), D_1AE400 + anchor[0], D_1AE404 + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)((char *)w + 0x368), D_1AE410 + anchor[0], D_1AE414 + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)((char *)w + 0x3C0), D_1AE418 + anchor[0], D_1AE41C + anchor[1], 0.0f, 0.0f);
    anchor = *(f32 **)((char *)w + 0x470);
    GuiElementSetPos((GuiElement *)((char *)w + 0x418), D_1AE420 + anchor[0], D_1AE424 + anchor[1], 0.0f, 0.0f);
}
#endif

/* Family-2 screen descriptor (0x5C bytes), shared by func_00347228 / func_00347348
 * / func_00347450. Distinct field layout from the func_00345F00 four. */
typedef struct GuiScreenDesc2 {
    void *label;        /* 0x00  &labelConst */
    s32   cfg0;         /* 0x04  block[0] (by value) */
    s32   cfg1;         /* 0x08  block[1] (by value) */
    s32   objCfg4FC;    /* 0x0C  obj[0x4FC] */
    s32   objCfg500;    /* 0x10  obj[0x500] */
    void *layout0;      /* 0x14  &block[2] */
    void *layout1;      /* 0x18  &block[3] */
    void *layout2;      /* 0x1C  &block[4] */
    void *layout3;      /* 0x20  &block[5] */
    void *layout4;      /* 0x24  &block[6] */
    void *child130;     /* 0x28  obj+0x130 */
    s32   _z2C;         /* 0x2C  0 */
    void *child17C;     /* 0x30  obj+0x17C */
    s32   _z34;         /* 0x34  0 */
    s32   typeIndex;    /* 0x38  this builder's screen-type index (literal) */
    void *child278;     /* 0x3C  obj+0x278 */
    void *child4B8;     /* 0x40  obj+0x4B8 */
    void *child220;     /* 0x44  obj+0x220 */
    s32   objCfg470;    /* 0x48  obj[0x470] */
    s32   objCfg504;    /* 0x4C  obj[0x504] */
    s32   _z50;         /* 0x50  0 for most builders; func_00347228 packs a scratch ptr here */
    s32   typeActive;   /* 0x54  obj[0x508] == typeIndex */
    s32   enable;       /* 0x58  1 */
} GuiScreenDesc2;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(GuiScreenDesc2) == 0x5C, "GuiScreenDesc2 must be 0x5C under ILP32");
#endif

/*
 * func_00347228 — screen builder, family-2 variant. Same GuiScreenDesc2 shape as
 * func_00347348/450 but: (1) screen-type index 0, so typeActive = (obj[0x508]==0);
 * (2) it uses the D_1AE428 global block (cfg0/cfg1 by value, layout0..4 = the five
 * &D_1AE430..450 addresses) and label const D_259CC0; (3) the distinguishing trait
 * — desc[0x50] carries a pointer to a local 8-byte copy of the D_1AE088 scratch
 * buffer (the original does an unaligned ldl/ldr -> sdl/sdr 8-byte copy onto the
 * frame, then stores its address into the descriptor). Save-wall blocked for
 * matching (frame 0xB0, six callee-saves at 8-byte spacing); TARGET_NATIVE #else
 * arm, cmp-oracle'd via cmp_GuiScreenBuilders2 (the scratch ptr is a stack address,
 * volatile across asm-vs-C — func_003380B8 dereferences the pointed-to bytes, so the
 * downstream effect is identical as long as those 8 bytes equal D_1AE088's).
 */
extern u8 D_1AE088[];               /* 8-byte scratch source */
extern u8 D_259CC0[];               /* family-2 label const for the index-0 screen */
extern s32 D_1AE428, D_1AE42C;      /* cfg0, cfg1 (read by value) */
extern u8 D_1AE430[8], D_1AE438[8], D_1AE440[8], D_1AE448[8], D_1AE450[8];  /* layout0..4 */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00347228);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00347228, unit objdiff): 0.00%,
   103/105 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0xb0' vs 'addiu sp, sp, -0x80').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00347228(void *obj_) {
    u8 *obj = (u8 *)obj_;
    GuiScreenDesc2 d;
    u8 scratch[8];
    s32 i;

    /* unaligned 8-byte copy of D_1AE088 (the original uses ldl/ldr -> sdl/sdr) */
    for (i = 0; i < 8; i++) scratch[i] = D_1AE088[i];

    d.label = D_259CC0;
    d.cfg0 = D_1AE428;
    d.cfg1 = D_1AE42C;
    d.objCfg4FC = *(s32 *)(obj + 0x4FC);
    d.objCfg500 = *(s32 *)(obj + 0x500);
    d.layout0 = D_1AE430; d.layout1 = D_1AE438; d.layout2 = D_1AE440;
    d.layout3 = D_1AE448; d.layout4 = D_1AE450;
    d.child130 = obj + 0x130;
    d._z2C = 0;
    d.child17C = obj + 0x17C;
    d._z34 = 0;
    d.typeIndex = 0;
    d.child278 = obj + 0x278;
    d.child4B8 = obj + 0x4B8;
    d.child220 = obj + 0x220;
    d.objCfg470 = *(s32 *)(obj + 0x470);
    d.objCfg504 = *(s32 *)(obj + 0x504);
    d._z50 = (s32)scratch;                      /* desc[0x50] = ptr to the local scratch copy (ILP32) */
    d.typeActive = (*(s32 *)(obj + 0x508) == 0);
    d.enable = 1;
    func_003380B8(&d);
}
#endif

/*
 * func_00347348 / func_00347450 — two more screen builders ("family-2" shape, a
 * distinct field layout from the func_00345F00 four). Each fills a 0x5C-byte
 * descriptor from the owning object + a per-builder label const and 7-global
 * block plus a screen-type index, then hands it to func_003380B8. The index
 * appears literally at desc[0x38] AND drives the desc[0x54] match flag
 * (obj[0x508] == index): 348 -> 3, 450 -> 1. (func_00347228 is the same family
 * but routes a pointed-to local D_1AE088 scratch buffer through desc[0x50]; left
 * INCLUDE_ASM for a separate pass.) Save-wall blocked for matching; provided as
 * TARGET_NATIVE #else arms, cmp-oracle'd (cmp_GuiScreenBuilders2). The
 * GuiScreenDesc2 layout is defined above func_00347228.
 */

/* family-2 label consts (absolute) + 7-global blocks (first two read by value) */
extern u8 D_1AA7F8[], D_1AA8B8[];
extern s32 D_1AE458, D_1AE45C; extern u8 D_1AE460[8], D_1AE468[8], D_1AE470[8], D_1AE478[8], D_1AE480[8];
extern s32 D_1AE488, D_1AE48C; extern u8 D_1AE490[8], D_1AE498[8], D_1AE4A0[8], D_1AE4A8[8], D_1AE4B0[8];

#ifdef TARGET_NATIVE
static void GuiBuildScreenDesc2(u8 *obj, void *label, s32 cfg0, s32 cfg1,
                                void *l0, void *l1, void *l2, void *l3, void *l4,
                                s32 typeIndex) {
    GuiScreenDesc2 d;
    d.label = label;
    d.cfg0 = cfg0;
    d.cfg1 = cfg1;
    d.objCfg4FC = *(s32 *)(obj + 0x4FC);
    d.objCfg500 = *(s32 *)(obj + 0x500);
    d.layout0 = l0; d.layout1 = l1; d.layout2 = l2; d.layout3 = l3; d.layout4 = l4;
    d.child130 = obj + 0x130;
    d._z2C = 0;
    d.child17C = obj + 0x17C;
    d._z34 = 0;
    d.typeIndex = typeIndex;
    d.child278 = obj + 0x278;
    d.child4B8 = obj + 0x4B8;
    d.child220 = obj + 0x220;
    d.objCfg470 = *(s32 *)(obj + 0x470);
    d.objCfg504 = *(s32 *)(obj + 0x504);
    d._z50 = 0;
    d.typeActive = (*(s32 *)(obj + 0x508) == typeIndex);
    d.enable = 1;
    func_003380B8(&d);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00347348);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00347348, unit objdiff): 22.83%,
   64/66 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0xa0' vs 'lw a2, %gp_rel(D_1AE458)(gp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00347348(void *obj) {
    GuiBuildScreenDesc2((u8 *)obj, D_1AA7F8, D_1AE458, D_1AE45C,
                        D_1AE460, D_1AE468, D_1AE470, D_1AE478, D_1AE480, 3);
}
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00347450);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00347450, unit objdiff): 23.95%,
   61/63 insns differ. Residual: UNKNOWN-addiu (first differing insn: 'addiu sp, sp, -0x90' vs 'lw a2, %gp_rel(D_1AE488)(gp)').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
void func_00347450(void *obj) {
    GuiBuildScreenDesc2((u8 *)obj, D_1AA8B8, D_1AE488, D_1AE48C,
                        D_1AE490, D_1AE498, D_1AE4A0, D_1AE4A8, D_1AE4B0, 1);
}
#endif

/* func_00347550: draw the weapon-grid screen. Bails if hidden (*(w+0x4F8)==0).
 * Draws four sprite elements (w/+0x4C/+0x98/+0xE4) and a text row (+0x1C8), runs
 * the three family-2 sub-builders func_00347228/00347348/00347450, then draws the
 * detail sprite (+0x17C) and five more text rows (+0x310/+0x2B8/+0x368/+0x3C0/
 * +0x418). */
/* GUARD (task #1273): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810/#8830; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00347550)
S136OS_SLOT(func_00347550);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00347550, unit objdiff): 94.87%,
   2/40 insns differ. Residual: SCHED (prologue/epilogue or delay-slot order only; sched1 ON/OFF and sched2 OFF RUN, none reproduce it).
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact on that arm. */
void func_00347550(void *w) {
    if (*(s32 *)((char *)w + 0x4F8) == 0) {
        return;
    }
    GuiSpriteElementDraw(w);
    GuiSpriteElementDraw((char *)w + 0x4C);
    GuiSpriteElementDraw((char *)w + 0x98);
    GuiSpriteElementDraw((char *)w + 0xE4);
    GuiTextElementDraw((char *)w + 0x1C8);
    func_00347228(w);
    func_00347348(w);
    func_00347450(w);
    GuiSpriteElementDraw((char *)w + 0x17C);
    GuiTextElementDraw((char *)w + 0x310);
    GuiTextElementDraw((char *)w + 0x2B8);
    GuiTextElementDraw((char *)w + 0x368);
    GuiTextElementDraw((char *)w + 0x3C0);
    GuiTextElementDraw((char *)w + 0x418);
}
#endif

/* Construct a list-style composite widget: six TypeB sub-elements (base..+0x17C),
 * a list-row (+0x1C8), three TypeC (+0x210, +0x268, +0x2C0), a sub-list (+0x318),
 * then three more TypeB (+0x354, +0x3A0, +0x3EC); returns the widget. */
#if defined(S136OS_func_003475F0) || defined(TARGET_NATIVE)
/* Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_003475F0; task #466):
 * unit objdiff 100.00% and verify_match_unit.sh 36/36 words + 14 relocs against the ROM.
 * GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_003475F0 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
void *func_003475F0(void *w) {
    GuiElementInitTypeB(w);
    GuiElementInitTypeB((char *)w + 0x4C);
    GuiElementInitTypeB((char *)w + 0x98);
    GuiElementInitTypeB((char *)w + 0xE4);
    GuiElementInitTypeB((char *)w + 0x130);
    GuiElementInitTypeB((char *)w + 0x17C);
    GuiListRowElementInit((char *)w + 0x1C8);
    GuiElementInitTypeC((char *)w + 0x210);
    GuiElementInitTypeC((char *)w + 0x268);
    GuiElementInitTypeC((char *)w + 0x2C0);
    func_003374D8((char *)w + 0x318);
    GuiElementInitTypeB((char *)w + 0x354);
    GuiElementInitTypeB((char *)w + 0x3A0);
    GuiElementInitTypeB((char *)w + 0x3EC);
    return w;
}
#else
S136OS_SLOT(func_003475F0);
#endif

/* GuiWeaponGridScreenInit: construct the weapon-grid screen — eight header/detail
 * button-glyph elements, a scrolling list (+0x1C8), a sprite (+0x318), and three
 * text rows (+0x210/+0x268/+0x2C0). Pool -> alloc the 0x10-byte placement record
 * (+0x438), +0x43C=pool + +0x4C0=1 unconditional; record 250x207. Buttons art
 * D_1ADBE8/BF0/FA0/C00/FA8/AE4B8/DF98/AE4C0 + a title +0x354 (D_1ADF78); the list
 * built via GuiListElementInit(tag D_1AE018) with 5 rows / 100 items / two colour
 * pairs + the two field leaves. Buttons coloured + glyphed (0x21..0x2C), title
 * scaled 1x1, sprite +0x318 (D_1ADFB0) coloured 0x60F0F0B0 scaled 32x32. Three
 * D_1ADC08 text rows coloured 0x80F0F0F0 (two right-flagged; +0x2C0 gets +0x308=1
 * / +0x30C=0xC8), texts localized 0x2BE7 + the in-place buffers +0x440/+0x480.
 * Closed by GuiWeaponGridTick(w, 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiWeaponGridScreenInit);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiWeaponGridScreenInit, unit objdiff): 46.59%,
   282/391 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x90' vs 'addiu sp, sp, -0x80').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
s32 GuiWeaponGridTick(void *w, s32 flag);
extern char *g_guiInstance;
extern u8 D_1ADBE8[], D_1ADBF0[], D_1ADFA0[], D_1ADC00[], D_1ADFA8[], D_1AE4B8[];
extern u8 D_1AE018[], D_1ADF98[], D_1AE4C0[], D_1ADF78[], D_1ADFB0[], D_1ADC08[];
void GuiWeaponGridScreenInit(void *w, GuiPool *pool) {
    GuiElement *e0 = (GuiElement *)((char *)w + 0x0);
    GuiElement *e1 = (GuiElement *)((char *)w + 0x4C);
    GuiElement *e2 = (GuiElement *)((char *)w + 0x130);
    GuiElement *e3 = (GuiElement *)((char *)w + 0x98);
    GuiElement *e4 = (GuiElement *)((char *)w + 0x3A0);
    GuiElement *e5 = (GuiElement *)((char *)w + 0x3EC);
    GuiElement *list = (GuiElement *)((char *)w + 0x1C8);
    GuiElement *e6 = (GuiElement *)((char *)w + 0xE4);
    GuiElement *e7 = (GuiElement *)((char *)w + 0x17C);
    GuiElement *sprite = (GuiElement *)((char *)w + 0x318);
    GuiElement *e8 = (GuiElement *)((char *)w + 0x354);
    GuiElement *t0 = (GuiElement *)((char *)w + 0x210);
    GuiElement *t1 = (GuiElement *)((char *)w + 0x268);
    GuiElement *t2 = (GuiElement *)((char *)w + 0x2C0);
    void *rec;

    /* +0x43C = pool is stored unconditionally (beqz delay slot). */
    *(GuiPool **)((char *)w + 0x43C) = pool;
    if (pool != 0) {
        rec = GuiPlacementNew(0x10, GuiPoolAlloc(pool));
        *(void **)((char *)w + 0x438) = rec;
        *(s32 *)((char *)rec + 0x0) = 0;
        *(s32 *)((char *)rec + 0x4) = 0;
        *(s32 *)((char *)rec + 0x8) = 0;
        *(s32 *)((char *)rec + 0xC) = 0;
    }

    *(s32 *)((char *)w + 0x4C0) = 1;
    rec = *(void **)((char *)w + 0x438);
    *(f32 *)((char *)rec + 0x0) = 250.0f;
    *(f32 *)((char *)rec + 0x4) = 207.0f;

    GuiElementInit(e0, (s32)D_1ADBE8, pool);
    GuiElementInit(e1, (s32)D_1ADBF0, pool);
    GuiElementInit(e2, (s32)D_1ADFA0, pool);
    GuiElementInit(e3, (s32)D_1ADC00, pool);
    GuiElementInit(e4, (s32)D_1ADFA8, pool);
    GuiElementInit(e5, (s32)D_1AE4B8, pool);

    GuiListElementInit(list, 0x20, 0, (s32)D_1AE018, pool);
    GuiListSetVisibleRows(list, 5);
    GuiListSetItemCount(list, 0x64);
    GuiListSetScrollPos(list, 0);
    GuiListSetColorPair0(list, 0x6049C1FF, 0x60001EFF);
    GuiListSetColorPair1(list, 0x50F0C070, 0x50F0C070);
    func_00337B88(list, (s32)0x80000000);
    func_00337B68(list, 0);

    GuiElementInit(e6, (s32)D_1ADF98, pool);
    GuiElementInit(e7, (s32)D_1AE4C0, pool);

    *GuiElementGetColor(e0) = 0x60442D00;
    *GuiElementGetColor(e1) = 0x55F0C070;
    *GuiElementGetColor(e2) = (s32)0x80FFDE8D;
    *GuiElementGetColor(e3) = 0x55F0C070;
    *GuiElementGetColor(e6) = (s32)0x80FFDE8D;
    *GuiElementGetColor(e7) = 0x60241700;
    *GuiElementGetColor(e4) = 0x60442D00;
    *GuiElementGetColor(e5) = 0x55F0C070;

    GuiElementSetGlyph(e0, (s32)(g_guiInstance + 0x8710), 0x21);
    GuiElementSetGlyph(e1, (s32)(g_guiInstance + 0x8710), 0x22);
    GuiElementSetGlyph(e2, (s32)(g_guiInstance + 0x8710), 0x28);
    GuiElementSetGlyph(e3, (s32)(g_guiInstance + 0x8710), 0x23);
    GuiElementSetGlyph(e6, (s32)(g_guiInstance + 0x8710), 0x27);
    GuiElementSetGlyph(e7, (s32)(g_guiInstance + 0x8710), 0x2A);
    GuiElementSetGlyph(e4, (s32)(g_guiInstance + 0x8710), 0x2C);
    GuiElementSetGlyph(e5, (s32)(g_guiInstance + 0x8710), 0x2B);

    *(s32 *)((char *)w + 0x4C4) = 0;
    *(s32 *)((char *)w + 0x4C8) = 0;
    GuiSpriteElementInit(sprite, (s32)D_1ADFB0, pool);
    *GuiElementGetColor(sprite) = 0x60F0F0B0;
    GuiElementSetScale(sprite, 32.0f, 32.0f, 0.0f, 0.0f);

    GuiElementInit(e8, (s32)D_1ADF78, pool);
    *GuiElementGetColor(e8) = 0x60241700;
    GuiElementSetGlyph(e8, (s32)(g_guiInstance + 0x8710), 0x29);
    GuiElementSetScale(e8, 1.0f, 1.0f, 0.0f, 0.0f);

    GuiTextElementInit(t0, (s32)D_1ADC08, pool);
    GuiTextElementInit(t1, (s32)D_1ADC08, pool);
    GuiElementSetTextFlag(t1, 1);
    GuiTextElementInit(t2, (s32)D_1ADC08, pool);
    GuiElementSetTextFlag(t2, 1);
    *(s32 *)((char *)w + 0x308) = 1;
    *(s32 *)((char *)w + 0x30C) = 0xC8;

    *GuiElementGetColor(t0) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t1) = (s32)0x80F0F0F0;
    *GuiElementGetColor(t2) = (s32)0x80F0F0F0;

    GuiElementSetText(t0, GetLocalizedString(0x2BE7));
    GuiElementSetText(t1, (s32)((char *)w + 0x440));
    GuiElementSetText(t2, (s32)((char *)w + 0x480));

    GuiWeaponGridTick(w, 0);
}
#endif

/* GuiWeaponGridTick: per-frame update for the 6x4 weapon-select grid widget `w`.
 * `flag` is the input-button bitmask for this frame. D-pad bits move the cursor
 * (0x1000 left / 0x4000 right over the 6 columns at +0x4C8; 0x8000 up / 0x2000
 * down over the 4 rows at +0x4C4, each wrapping), playing the move cue and, on a
 * real cell change, kicking the highlight pulse (func_002AA3F0). The selected
 * cell maps through D_00259F38[cell] -> item id -> equipped variant slot in
 * g_weaponTable. All nine sub-elements are repositioned relative to the placement
 * record (+0x438); the cursor (+0xE4) additionally steps 53.5px per row / 47px
 * per column. For an owned item it formats the localized name (+0x480) and the
 * ammo/upgrade line (+0x440, split "cur/cap" at 1000 via func_00115DA8); an
 * unowned cell clears both text buffers. When 0x40 (confirm) is held on an owned
 * item it plays the accept cue, records the item in g_menuTransitionMode+0x24 and
 * returns the selected cell index; otherwise returns -1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", GuiWeaponGridTick);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern u8 D_00259F38[];
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);
extern u8 D_1ADBA8[];
/* (end of this body's declarations) */
/* engine96 probe (task #466, cc1 2.96 via MATCH_GuiWeaponGridTick, unit objdiff): 39.99%,
   396/479 insns differ. Residual: UNKNOWN-addiu + movn/movz, gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0x100' vs 'addiu sp, sp, -0x70').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern s32 g_weaponAmmo[];              /* 0x139688 - s32 current ammo per item id */
extern u8 g_inventoryNewFlag[];         /* 0x1A7B38 - per-item "newly acquired" flag */
extern u8 g_menuTransitionMode[];       /* 0x1F27DC - +0x24 receives the activated item id */
extern f32 D_1AE4D0[2], D_1AE4D8[2], D_1AE4E0[2], D_1AE4E8[2]; /* {x,y} element offset pairs */
extern u8 D_1AE4F0[], D_1AE4F8[], D_1AE500[], D_1AE510[], D_1AE2D0[]; /* sprintf format strings */

s32 GuiWeaponGridTick(void *w, s32 flag) {
    f32 *base = *(f32 **)((char *)w + 0x438);
    s32 oldCell = *(s32 *)((char *)w + 0x4C4) + *(s32 *)((char *)w + 0x4C8) * 4;
    s32 newCell;
    s32 selRow, selCol;
    s32 result = -1;

    if (flag & 0x1000) {
        s32 v = *(s32 *)((char *)w + 0x4C8) - 1;
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4C8) = (v > -1) ? v : 5;
    } else if (flag & 0x4000) {
        s32 v = *(s32 *)((char *)w + 0x4C8) + 1;
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4C8) = (5 < v) ? 0 : v;
    } else if (flag & 0x8000) {
        s32 v = *(s32 *)((char *)w + 0x4C4) - 1;
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4C4) = (v > -1) ? v : 3;
    } else if (flag & 0x2000) {
        s32 v = *(s32 *)((char *)w + 0x4C4) + 1;
        PlayGlobalSound(3, 0, 0);
        *(s32 *)((char *)w + 0x4C4) = (3 < v) ? 0 : v;
    }

    selRow = *(s32 *)((char *)w + 0x4C4);
    selCol = *(s32 *)((char *)w + 0x4C8);
    newCell = selRow + selCol * 4;
    if (oldCell != newCell) {
        func_002AA3F0(0, 0, 1, 0, 1);
    }

    {
        s32 *cursorColor = GuiElementGetColor((GuiElement *)((char *)w + 0xE4));
        *cursorColor = func_002AA3F0(0x60442D00, 0x70FFFEED, 0x14, 0, 0);
    }

    GuiElementSetPos((GuiElement *)((char *)w + 0x0),   base[0], base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x4C),  base[0], base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x130), base[0], base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x98),  base[0], base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0xE4),
                     D_1AE4D0[0] + base[0] + (f32)selRow * 53.5f,
                     D_1AE4D0[1] + base[1] + (f32)selCol * 47.0f, 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x17C), base[0], base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x210),
                     D_1AE4D8[0] + base[0], D_1AE4D8[1] + base[1], 0.0f, 0.0f);

    D_1ADAF0 = -1;
    if (newCell >= 0) {
        s16 gridItemId = *(s16 *)((char *)D_00259F38 + newCell * 10 + 6);
        u8 slot = g_itemEquippedSlot[gridItemId];

        if (g_weaponTable[slot * 0xE0 + 4] != 0) {
            g_inventoryOwned[gridItemId] = 1;
            g_inventoryNewFlag[gridItemId] = 1;
        }

        if (g_inventoryOwned[gridItemId] != 0) {
            char *namebuf = (char *)w + 0x480;
            char *statusbuf = (char *)w + 0x440;
            s16 nameId = *(s16 *)&g_weaponTable[slot * 0xE0 + 6];
            s32 nameStrArg = *(s32 *)&g_weaponTable[slot * 0xE0 + 8];
            s32 ammo;
            u16 capacity;

            D_1ADAF0 = nameId;
            func_00115DA8(namebuf, (const char *)GetLocalizedString(nameStrArg));

            ammo = g_weaponAmmo[gridItemId];
            capacity = *(u16 *)&g_weaponTable[slot * 0xE0 + 0x88];
            if (capacity != 0) {
                u16 upgrade = *(u16 *)&g_weaponTable[slot * 0xE0 + 0x8E];
                char *p = statusbuf;
                p += func_00115DA8(p, (const char *)D_1AE4F0, (const char *)GetLocalizedString(0x2C5F));
                if (ammo < 1000)
                    p += func_00115DA8(p, (const char *)D_1AE4F8, ammo);
                else
                    p += func_00115DA8(p, (const char *)D_1AE500, ammo / 1000, ammo % 1000);
                if (upgrade < 1000)
                    p += func_00115DA8(p, (const char *)D_1AE2D0, upgrade);
                else
                    p += func_00115DA8(p, (const char *)D_1AE510, upgrade / 1000, upgrade % 1000);
            } else {
                func_00115DA8(statusbuf, (const char *)D_1ADBA8, (const char *)GetLocalizedString(0x2C4F));
            }

            if (flag & 0x40) {
                PlayGlobalSound(4, 0, 0);
                result = newCell;
                *(s32 *)&g_menuTransitionMode[0x24] = gridItemId;
            }
        } else {
            *((char *)w + 0x440) = 0;
            *((char *)w + 0x480) = 0;
            if (flag & 0x40) {
                PlayGlobalSound(5, 0, 0);
            }
        }
    }

    GuiElementSetPos((GuiElement *)((char *)w + 0x268),
                     D_1AE4E0[0] + base[0], D_1AE4E0[1] + base[1], 0.0f, 0.0f);
    GuiElementSetPos((GuiElement *)((char *)w + 0x2C0),
                     D_1AE4E8[0] + base[0], D_1AE4E8[1] + base[1], 0.0f, 0.0f);

    return result;
}
#endif

/* func_003481E0: redraw the "command"/status icon triplet for one HUD item. The
 * item's state byte is looked up in g_itemStateFlags via a stride-10 descriptor
 * table (D_00259F38[idx].flags, idx = *(p+0x4C4) + *(p+0x4C8)*4). All three sub-
 * elements (icon p+0x318, left p+0x3A0, right p+0x3EC) start hidden and are
 * positioned relative to the item's base vector (*(p+0x438)); the icon falls back
 * to the default position (D_1AE518/D_1AE51C). If flag bit0 is set the icon uses
 * sprite slot 0 + caption 0x3065; else if bit2, slot 1 + caption 0x3064. Bit4 adds
 * a second (offset-by-0x2D-and-D_1AE530) row with slot 2 + caption 0x3066. Captions
 * are drawn auto-fit-width-centered at scale 0.7 (func_00280440), colour 0x80F0F0F0,
 * max width 0xAF. func_00337630 flushes the icon sprite. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_003481E0);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_003481E0, unit objdiff): 49.74%,
   249/330 insns differ. Residual: UNKNOWN-lui + gp/abs-mixed symbol (first differing insn: 'lui a3, %hi(D_1AE518)' vs 'daddu a1, zero, zero').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00280440(f32 scale, s32 x, s32 y, u32 color, const char *text,
                          s32 flag, s32 maxWidth);
extern s32 D_1AE518, D_1AE51C;
extern f32 D_1AE520, D_1AE524, D_1AE528, D_1AE52C, D_1AE530, D_1AE538, D_1AE53C,
           D_1AE540, D_1AE544;
extern u8 D_00259F38[], g_itemStateFlags[];
void func_003481E0(void *p) {
    GuiElement *icon = (GuiElement *)((char *)p + 0x318);
    GuiElement *left = (GuiElement *)((char *)p + 0x3A0);
    GuiElement *right = (GuiElement *)((char *)p + 0x3EC);
    s32 defX = D_1AE518;
    s32 defY = D_1AE51C;
    s32 idx = *(s32 *)((char *)p + 0x4C4) + *(s32 *)((char *)p + 0x4C8) * 4;
    s16 flagIdx = *(s16 *)(D_00259F38 + idx * 10 + 6);
    f32 *base;
    s32 str;

    GuiElementSetVisible(icon, 0);
    GuiElementSetVisible(left, 0);
    GuiElementSetVisible(right, 0);

    base = *(f32 **)((char *)p + 0x438);
    GuiElementSetPos(left, base[0] + D_1AE528, base[1] + D_1AE52C, 0.0f, 0.0f);
    base = *(f32 **)((char *)p + 0x438);
    GuiElementSetPos(right, base[0] + D_1AE520, base[1] + D_1AE524, 0.0f, 0.0f);
    GuiElementSetPos(icon, (f32)defX, (f32)defY, 0.0f, 0.0f);

    if (g_itemStateFlags[flagIdx] & 1) {
        GuiSpriteSetTexture(icon, 0xEA98, 0);
        GuiElementSetVisible(icon, 1);
        GuiElementSetVisible(left, 1);
        GuiElementSetVisible(right, 1);
        GuiSpriteElementDraw(left);
        GuiSpriteElementDraw(right);
        base = *(f32 **)((char *)p + 0x438);
        str = GetLocalizedString(0x3065);
        func_00280440(0.7f, (s32)(base[0] + D_1AE538), (s32)(base[1] + D_1AE53C),
                      0x80F0F0F0, (const char *)str, -1, 0xAF);
    } else if (g_itemStateFlags[flagIdx] & 4) {
        GuiSpriteSetTexture(icon, 0xEA98, 1);
        GuiElementSetVisible(icon, 1);
        GuiElementSetVisible(left, 1);
        GuiElementSetVisible(right, 1);
        GuiSpriteElementDraw(left);
        GuiSpriteElementDraw(right);
        base = *(f32 **)((char *)p + 0x438);
        str = GetLocalizedString(0x3064);
        func_00280440(0.7f, (s32)(base[0] + D_1AE538), (s32)(base[1] + D_1AE53C),
                      0x80F0F0F0, (const char *)str, -1, 0xAF);
    }

    func_00337630(icon);

    if (g_itemStateFlags[flagIdx] & 0x10) {
        GuiSpriteSetTexture(icon, 0xEA98, 2);
        GuiElementSetPos(icon, (f32)defX, (f32)(defY + 0x2D), 0.0f, 0.0f);
        GuiElementSetVisible(icon, 1);
        GuiElementSetVisible(left, 1);
        GuiElementSetVisible(right, 1);
        base = *(f32 **)((char *)p + 0x438);
        GuiElementSetPos(left, base[0] + D_1AE528, base[1] + D_1AE52C + D_1AE530,
                         0.0f, 0.0f);
        base = *(f32 **)((char *)p + 0x438);
        GuiElementSetPos(right, base[0] + D_1AE520, base[1] + D_1AE524 + D_1AE530,
                         0.0f, 0.0f);
        GuiSpriteElementDraw(left);
        GuiSpriteElementDraw(right);
        base = *(f32 **)((char *)p + 0x438);
        str = GetLocalizedString(0x3066);
        func_00280440(0.7f, (s32)(base[0] + D_1AE540), (s32)(base[1] + D_1AE544),
                      0x80F0F0F0, (const char *)str, -1, 0xAF);
        func_00337630(icon);
    }
}
#endif

/* func_00348628(self): render the weapon quick-select grid (6 rows x 4 cols). No-op
 * unless self[+0x4C0] is set. Draws the frame sprites, then for each grid cell walks
 * the entry table (D_00259F38, stride 0x14 shorts, +6 = itemId; row stride 0x28
 * bytes): positions the cell's four sub-elements (+0x354 backdrop, +0x318 icon,
 * +0x1C8 ammo/xp list, +0x98 highlight) at the placement anchor (*(self+0x438))
 * offset by the per-row/col layout constants (D_1AE708-714, 53.5/47.0 icon step,
 * qword D_1AE548/550 for the list, qword D_1AE558/560 for the highlight). For an
 * owned item (g_inventoryOwned) it sets the icon (g_weaponTable[slot]+0x3C), and if
 * that isn't the empty sentinel 0xEA7E, wires the ammo list (count g_weaponTable+0x6C,
 * scroll g_weaponXp>>5, colours via func_00337DC8; count<1 -> 100/100). Tints the
 * backdrop for a dry weapon (g_weaponAmmo==0 & g_weaponTable+0x88) and highlights the
 * currently-activated item (func_0026F7A8(g_menuTransitionMode+0x24)). Finishes with
 * the header/label elements + func_003481E0. Faithful TARGET_NATIVE #else (engine
 * 2.96 = no byte-match); all 4 GuiElementSetPos element+arg orders + the qword-split
 * int/float constants asm-verified per-call. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/235FE8", func_00348628);
#else
/* engine96 probe (task #466, cc1 2.96 via MATCH_func_00348628, unit objdiff): 49.86%,
   328/410 insns differ. Residual: UNKNOWN-addiu + gp/abs-mixed symbol (first differing insn: 'addiu sp, sp, -0xe0' vs 'addiu sp, sp, -0x90').
   Levers RUN on the whole unit: -fno-strict-aliasing, per-symbol gp/abs pins, sibcall barrier;
   not byte-exact, so the arm stays #else. */
extern void func_00337DC8(s32 itemId, s32 *petalRgb, s32 *listC0, s32 *listC1);
extern s16 func_0026F7A8(s32 activatedId);
extern s32 g_weaponXp[], g_weaponAmmo[];
extern u8 g_menuTransitionMode[], D_00259F38[];
extern s32 D_1AE548, D_1AE54C, D_1AE550, D_1AE554, D_1AE700, D_1AE704, D_1AE708,
           D_1AE70C;
extern f32 D_1AE558, D_1AE55C, D_1AE560, D_1AE564, D_1AE710, D_1AE714;
void func_00348628(void *self) {
    u8 *p = (u8 *)self;
    GuiElement *back = (GuiElement *)(p + 0x354);
    GuiElement *high = (GuiElement *)(p + 0x98);
    GuiElement *list = (GuiElement *)(p + 0x1C8);
    GuiElement *icon = (GuiElement *)(p + 0x318);
    s32 row, col;

    if (*(s32 *)(p + 0x4C0) == 0) {
        return;
    }

    GuiSpriteElementDraw(p);
    GuiSpriteElementDraw(p + 0x4C);
    GuiSpriteElementDraw(p + 0x130);

    for (row = 0; row < 6; row++) {
        s16 *grid = (s16 *)(D_00259F38 + 6 + row * 0x28);
        for (col = 0; col < 4; col++, grid += 5) {
            s32  item = grid[0];
            f32 *anchor;

            *GuiElementGetColor(back) = 0x60241700;
            anchor = *(f32 **)(p + 0x438);
            GuiElementSetPos(back, D_1AE710 * (f32)col + (f32)D_1AE708 + anchor[0],
                             D_1AE714 * (f32)row + (f32)D_1AE70C + anchor[1],
                             0.0f, 0.0f);
            *GuiElementGetColor(high) = 0x30F0C070;
            *GuiElementGetColor(back) = 0x60241700;
            GuiElementSetVisible(list, 0);
            GuiElementSetVisible(icon, 0);

            if (g_inventoryOwned[item] != 0) {
                s32 slot = g_itemEquippedSlot[item];
                s32 tex  = *(u16 *)&g_weaponTable[slot * 0xE0 + 0x3C];

                GuiSpriteSetTexture(icon, tex, 0);
                anchor = *(f32 **)(p + 0x438);
                GuiElementSetPos(icon, (f32)col * 53.5f + (f32)D_1AE700 + anchor[0],
                                 (f32)row * 47.0f + (f32)D_1AE704 + anchor[1],
                                 0.0f, 0.0f);
                if (tex == 0xEA7E) {
                    *GuiElementGetColor(icon) = (s32)0x80808080;
                } else {
                    s32 petalRgb = 0, listC0 = 0, listC1 = 0;
                    GuiListSetItemCount(list,
                                        *(s32 *)&g_weaponTable[slot * 0xE0 + 0x6C]);
                    GuiListSetScrollPos(list, g_weaponXp[item] >> 5);
                    anchor = *(f32 **)(p + 0x438);
                    GuiElementSetPos(list,
                                     (f32)(D_1AE550 * col + D_1AE548) + anchor[0],
                                     (f32)(D_1AE554 * row + D_1AE54C) + anchor[1],
                                     0.0f, 0.0f);
                    GuiElementSetVisible(list, 1);
                    func_00337DC8(item, &petalRgb, &listC0, &listC1);
                    GuiListSetColorPair0(list, petalRgb, listC0);
                    *GuiElementGetColor(icon) = listC1;
                    if (*(s32 *)&g_weaponTable[slot * 0xE0 + 0x6C] < 1) {
                        GuiListSetItemCount(list, 100);
                        GuiListSetScrollPos(list, 100);
                    }
                }

                if (g_weaponAmmo[item] == 0 &&
                    *(s16 *)&g_weaponTable[g_itemEquippedSlot[item] * 0xE0 + 0x88]
                        != 0) {
                    *GuiElementGetColor(back) = 0x60202080;
                }
                GuiElementSetVisible(icon, 1);
                if (func_0026F7A8(*(s32 *)&g_menuTransitionMode[0x24]) == item) {
                    *GuiElementGetColor(high) = (s32)0x80FFDE8D;
                }
            }

            anchor = *(f32 **)(p + 0x438);
            GuiElementSetPos(high, D_1AE558 * (f32)col + D_1AE560 + anchor[0],
                             D_1AE55C * (f32)row + D_1AE564 + anchor[0],
                             0.0f, 0.0f);
            GuiSpriteElementDraw(back);
            func_00337630(icon);
            func_00337350(list);
            GuiSpriteElementDraw(high);
        }
    }

    GuiSpriteElementDraw(p + 0xE4);
    GuiSpriteElementDraw(p + 0x17C);
    GuiTextElementDraw(p + 0x210);
    GuiTextElementDraw(p + 0x268);
    GuiTextElementDraw(p + 0x2C0);
    func_003481E0(self);
}
#endif

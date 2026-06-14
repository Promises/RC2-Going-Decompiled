#include "common.h"

/*
 * text/188470 — EU (SCES_516.07) twin of USA text/188580 (camera-aux helper
 * TU). Ported in Phase B (2026-06-14): the matched C bodies are region-agnostic
 * (objdiff masks gp/reloc deltas); only the referenced global NAMES are
 * retargeted to their EU addresses. Like its USA sibling this is an original
 * separate TU built at nonzero -G: its small-data globals (D_1A8B10/D_1A8B14)
 * are accessed via %gp_rel($gp); the matcher builds THIS unit at -O2 -G8 (see
 * the per-unit GFLAG override in tools/ee/objdiff_build.sh). See the USA unit
 * src/usa/text/188580.c for the full -G8 extern-sizing rules.
 */

/* Anonymous .bss state block — EU address 0x1BAC80 (USA twin: D_001BAC00 at
 * 0x1BAC00). The EU asm reaches its fields off g_nVendorBuyQuantity+0x8A38
 * (0x1B2248+0x8A38 == 0x1BAC80). +0x40 is compared against mode/state 7 and
 * +0x58 stepped 1->2 by the helpers below. Field semantics not yet traced. */
typedef struct {
    /* 0x00 */ u8 unk00[0x40];
    /* 0x40 */ s32 unk40; /* mode/state id; 7 gates the helpers below */
    /* 0x44 */ u8 unk44[0x14];
    /* 0x58 */ s32 unk58; /* sub-step: 1 -> 2 handshake */
    /* 0x5C */ u8 unk5C[0x28];
    /* 0x84 */ s16 unk84; /* counter/flag, reset by func_00288798 */
} UnkCamAuxState;
extern UnkCamAuxState D_001BAC80;

/* Small-data globals (gp-relative; complete <=8-byte declarations so -G8
 * places them in small data). EU twins of USA D_1A8A60 / D_1A8A64. Semantics
 * not yet traced. */
extern s32 D_1A8B10;
extern s32 D_1A8B14;

/* func_002884F0: USA twin func_00288600 (camera-matrix rebuild from an angle
 * pair). Held INCLUDE_ASM in USA by the proven 16-byte callee-save-slot layout
 * wall; same wall applies in EU. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188470", func_002884F0);

/* func_00288638: USA twin func_00288748 (arcmin->rad camera roll step). Same
 * 16-byte save-slot wall — held INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188470", func_00288638);

/* func_002886B0: USA twin func_002887C0 (re-anchor camera, sibling-call tail).
 * Same 16-byte save-slot wall + sibling-call-optimised tail — held INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188470", func_002886B0);

/** If the 0x1BAC80 block is in state 7 with sub-step 1, advance it to 2.
 *  (USA twin: func_00288840.) */
void func_00288730(void) {
    UnkCamAuxState *state = &D_001BAC80;

    if (state->unk40 == 7 && state->unk58 == 1) {
        state->unk58 = 2;
    }
}

/** True while the 0x1BAC80 block is in state 7. (USA twin: func_00288870.) */
s32 func_00288760(void) {
    return D_001BAC80.unk40 == 7;
}

/** Set D_1A8B10 to 3. (USA twin: func_00288888.) */
void func_00288778(void) {
    D_1A8B10 = 3;
}

/** Read the 0x1BAC80 block's +0x84 counter/flag. (USA twin: func_00288898.) */
s16 func_00288788(void) {
    return D_001BAC80.unk84;
}

/** Clear the 0x1BAC80 block's +0x84 counter/flag. (USA twin: func_002888A8.) */
void func_00288798(void) {
    D_001BAC80.unk84 = 0;
}

/** Read the D_1A8B14 flag. (USA twin: func_002888B8.) */
s32 func_002887A8(void) {
    return D_1A8B14;
}

/** Raise the D_1A8B14 flag. (USA twin: func_002888C0.) */
void func_002887B0(void) {
    D_1A8B14 = 1;
}

/** Clear the D_1A8B14 flag. (USA twin: func_002888D0.) */
void func_002887C0(void) {
    D_1A8B14 = 0;
}

#include "common.h"

/*
 * text/188580 — camera-aux helper TU (carved out of the .text asm segment on
 * 2026-06-10, TU-cluster scan candidate B). This is an original separate
 * translation unit built at nonzero -G: its small-data globals (sdata cluster
 * D_1A8A60..D_1A8AE0) are accessed uniformly via %gp_rel($gp), which -G0
 * cannot express. The matcher builds THIS unit at -O2 -G8 (see the per-unit
 * GFLAG override in tools/ee/objdiff_build.sh / diff.sh / build.sh); every
 * other text unit stays -O2 -G0.
 *
 * -G8 rule for externs in this file: a complete extern object of size <= 8
 * bytes is placed in small data (gp-relative access); anything that the
 * original accesses with absolute %hi/%lo pairs must be declared with an
 * incomplete array type (extern T sym[];) or a complete type > 8 bytes so
 * cc1 cannot prove it small.
 */

/* Anonymous .bss state block at 0x1BAC00 (sized in symbol_addrs so the asm
 * resolves its fields to this symbol). +0x40 is compared against mode/state 7
 * and +0x58 stepped 1->2 by the helpers below; also driven by the screen/
 * cinematic code around func_00287140. Field semantics not yet traced. */
typedef struct {
    /* 0x00 */ u8 unk00[0x40];
    /* 0x40 */ s32 unk40; /* mode/state id; 7 gates the helpers below */
    /* 0x44 */ u8 unk44[0x14];
    /* 0x58 */ s32 unk58; /* sub-step: 1 -> 2 handshake */
    /* 0x5C */ u8 unk5C[0x28];
    /* 0x84 */ s16 unk84; /* counter/flag, reset by func_002888A8 */
} UnkCamAuxState;
extern UnkCamAuxState D_001BAC00;

/* Small-data globals (gp-relative; complete <=8-byte declarations so -G8
 * places them in small data). Semantics not yet traced. */
extern s32 D_1A8A60;
extern s32 D_1A8A64;

/* func_00288600: camera-matrix rebuild from an angle pair (saves s0..s5+ra,
 * 8-byte-packed at sp+0xB0..0xE0) — blocked by the proven 16-byte
 * callee-save-slot layout wall (see cod/0321A0 snd_SetupDmaTransfer): this
 * cc1 reserves a 16-byte stack slot per callee save, the original packs them
 * 8-byte. Blocks every multi-save function in this unit. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188580", func_00288600);

/* func_00288748: arcmin->rad camera roll step scaled by D_1A8A80 (saves
 * s0/s1/ra packed at sp+0x40/0x48/0x50) — same 16-byte save-slot wall. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188580", func_00288748);

/* func_002887C0: re-anchor camera at D_001B1750, apply arg via func_00283670,
 * scale by (f32)D_1A8A78 — body reproduces 1:1 (best attempt 54%: every call/
 * operand right) but saves s0/s1/ra packed at sp+0x0/0x8/0x10 in a 0x20 frame;
 * this cc1 builds a 0x40 frame with 16-byte save slots AND sibling-call-
 * optimises the void tail call to `j func_00283670` where the original keeps
 * jal+epilogue. Same 16-byte save-slot wall as cod/0321A0. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188580", func_002887C0);

/** If the 0x1BAC00 block is in state 7 with sub-step 1, advance it to 2. */
void func_00288840(void) {
    UnkCamAuxState *state = &D_001BAC00;

    if (state->unk40 == 7 && state->unk58 == 1) {
        state->unk58 = 2;
    }
}

/** True while the 0x1BAC00 block is in state 7. */
s32 func_00288870(void) {
    return D_001BAC00.unk40 == 7;
}

/** Set D_1A8A60 to 3. */
void func_00288888(void) {
    D_1A8A60 = 3;
}

/** Read the 0x1BAC00 block's +0x84 counter/flag. */
s16 func_00288898(void) {
    return D_001BAC00.unk84;
}

/** Clear the 0x1BAC00 block's +0x84 counter/flag. */
void func_002888A8(void) {
    D_001BAC00.unk84 = 0;
}

/** Read the D_1A8A64 flag. */
s32 func_002888B8(void) {
    return D_1A8A64;
}

/** Raise the D_1A8A64 flag. */
void func_002888C0(void) {
    D_1A8A64 = 1;
}

/** Clear the D_1A8A64 flag. */
void func_002888D0(void) {
    D_1A8A64 = 0;
}

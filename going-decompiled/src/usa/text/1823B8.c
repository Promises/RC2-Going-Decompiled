#include "common.h"

extern u64 GetUiTextureTex0(s32 id);
extern s32 D_239A90[];

/* GS sprite-primitive descriptor built by the UI texture-quad setup below:
 * 0x70 reserved, 0x78 = TEX0 (texture-buffer reg from GetUiTextureTex0),
 * 0x80 = TEX1, 0x88 = CLAMP (wrap/clamp mode packed from the per-format table
 * at D_239A90, stride 0x14). */
typedef struct UiSpritePacket {
    u8  pad0[0x70];
    u64 unk70;
    u64 tex0;
    u64 tex1;
    u64 clamp;
} UiSpritePacket;

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282438);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282790);

/* Initialise a UI sprite packet's texture/clamp fields. Looks up the texture
 * buffer (TEX0) for texId, sets a fixed TEX1, and builds the CLAMP register
 * from the format-table entry D_239A90[idx] OR'd with the caller's wrap mode
 * (placed in the upper 32 bits).
 *
 * NEAR-MISS (correct C, not byte-exact): walls the 8-byte-packed callee-save
 * layout (pinned cc1 reserves 16 bytes/save; original packs 4 saves at 8-byte
 * spacing in a 0x20 frame) plus the TEX1 constant lowering (original emits
 * ori/dsll32/ori, ee-gcc emits a single dli). Preserved as the native body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282798);
#else
void func_00282798(UiSpritePacket *p, s32 texId, s32 idx, u64 wrapMode) {
    s32 *e = &D_239A90[idx * 5];
    p->tex0  = GetUiTextureTex0(texId);
    p->tex1  = ((u64)0xFF90 << 32) | 0x260;
    p->unk70 = 0;
    p->clamp = (u64)e[0]
             | ((u64)e[1] << 2)
             | ((u64)e[2] << 4)
             | ((u64)e[3] << 6)
             | (wrapMode << 32);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282838);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282A48);

/* func_00282A78: empty/no-op leaf (original compiles to jr ra; nop). */
void func_00282A78(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282A80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", func_00282E50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1823B8", DrawGlowSprites);

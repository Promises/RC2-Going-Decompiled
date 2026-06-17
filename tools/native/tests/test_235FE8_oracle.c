/*
 * test_235FE8_oracle.c — worked example of the PCSX2 golden-trace pattern for a
 * struct-mutator (GuiElementSetPos). This is the TEMPLATE for turning a captured
 * trace into a native test; see tools/native/ORACLE.md for the capture recipe.
 *
 * The pointer-graph-rebuild pattern (no emulator, no fixed address map):
 *   - PCSX2 capture gives you, at function ENTRY: a0 (the object addr), the
 *     float args, and the bytes of every block the function reaches through its
 *     pointers; and at RETURN: the bytes of every block it mutated.
 *   - You don't replay PS2 addresses. You rebuild the SAME pointer graph with
 *     native allocations (object + its pointee blocks wired together), seed it
 *     with the captured INPUT bytes/args, call the function, and CHECK the
 *     mutated blocks against the captured OUTPUT bytes.
 *
 * Works for functions whose footprint is "pointer args + their pointees".
 * Functions that touch absolute globals need those mapped too (deferred).
 *
 * NOTE: the values below are a SYNTHETIC stand-in for a real capture (clearly
 * marked) — they demonstrate the transcription pattern. Replace with captured
 * numbers once PCSX2 capture is wired up.
 */
#include "common.h"
#include "native_test.h"

/* Mirror of the GuiElement layout defined inside text/235FE8.c (the struct is
 * unit-local, not yet in a shared header — only the touched fields matter). */
typedef struct GuiElement {
    /* 0x00 */ f32 *pos;
    /* 0x04 */ f32 *scale;
    /* 0x08 */ s32 unk08;
    /* 0x0C */ s32 *color;
    /* 0x10 */ f32 *visible;
} GuiElement;

extern void GuiElementSetPos(GuiElement *e, f32 x, f32 y, f32 z, f32 w);

int main(void) {
    /* --- captured ENTRY state (SYNTHETIC placeholder) ------------------- */
    /* a0 -> GuiElement; e->pos -> a 4-float block (the "pos" pointee). */
    f32 pos_block[4] = {-1, -1, -1, -1};   /* pos block bytes BEFORE the call */
    GuiElement e = {0};
    e.pos = pos_block;                      /* rebuild the pointer graph */
    f32 in_x = 12.0f, in_y = -3.5f, in_z = 0.0f, in_w = 1.0f;  /* $f12..$f15 */

    /* --- run the function under test ----------------------------------- */
    GuiElementSetPos(&e, in_x, in_y, in_z, in_w);

    /* --- captured RETURN state (SYNTHETIC placeholder) ------------------ */
    /* pos block bytes AFTER the call, as PCSX2 would have dumped them. */
    CHECK_FEQ(pos_block[0], 12.0f, 1e-6f);
    CHECK_FEQ(pos_block[1], -3.5f, 1e-6f);
    CHECK_FEQ(pos_block[2], 0.0f, 1e-6f);
    CHECK_FEQ(pos_block[3], 1.0f, 1e-6f);

    return TEST_SUMMARY();
}

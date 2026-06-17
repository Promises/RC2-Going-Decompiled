/*
 * test_235FE8.c — functional tests for pure helpers in text/235FE8.
 *
 * Hand-authored golden cases (oracle = the documented behavior in the unit;
 * later these get cross-checked against PCSX2-captured traces). Proves the
 * native harness end-to-end: pure-math asserts + a mocked callee + --gc-sections
 * pulling in only the reached functions.
 */
#include "common.h"
#include "native_test.h"

/* Defined in the unit under test. */
extern void func_00336918(void *self, f32 t, f32 *dst, f32 *a, f32 *b);
extern void func_00336AC8(f32 *src, f32 t, f32 *dst);
extern void func_00336A28(f32 *src, f32 t, f32 *dst);

/* --- mock for func_00336A28's only callee ------------------------------- */
static f32 g_herm_args[5];
static int g_herm_called = 0;
f32 GuiHermiteInterp(f32 t, f32 c0, f32 c1, f32 c2, f32 c3) {
    g_herm_called++;
    g_herm_args[0] = t; g_herm_args[1] = c0; g_herm_args[2] = c1;
    g_herm_args[3] = c2; g_herm_args[4] = c3;
    return 42.5f; /* sentinel: prove the result is forwarded to *dst */
}

int main(void) {
    /* func_00336918: dst = (1-t)*a + t*b, componentwise. t=0.25 -> it=0.75. */
    {
        f32 a[4] = {1, 2, 3, 4}, b[4] = {5, 6, 7, 8}, dst[4] = {0};
        func_00336918((void *)0, 0.25f, dst, a, b);
        CHECK_FEQ(dst[0], 2.0f, 1e-6f);  /* .75*1 + .25*5 */
        CHECK_FEQ(dst[1], 3.0f, 1e-6f);  /* .75*2 + .25*6 */
        CHECK_FEQ(dst[2], 4.0f, 1e-6f);  /* .75*3 + .25*7 */
        CHECK_FEQ(dst[3], 5.0f, 1e-6f);  /* .75*4 + .25*8 */
    }

    /* func_00336AC8: a[i] at src[2,4,6,8], b[i] at src[3,5,7,9]; t=0.5. */
    {
        f32 src[10] = {0, 0, 10, 20, 30, 40, 50, 60, 70, 80};
        f32 dst[4] = {0};
        func_00336AC8(src, 0.5f, dst);
        CHECK_FEQ(dst[0], 15.0f, 1e-6f);  /* .5*10 + .5*20 */
        CHECK_FEQ(dst[1], 35.0f, 1e-6f);  /* .5*30 + .5*40 */
        CHECK_FEQ(dst[2], 55.0f, 1e-6f);  /* .5*50 + .5*60 */
        CHECK_FEQ(dst[3], 75.0f, 1e-6f);  /* .5*70 + .5*80 */
    }

    /* func_00336A28: forwards GuiHermiteInterp(t, 0, src[2], src[3], 1) -> *dst. */
    {
        f32 src[4] = {9, 9, 0.3f, 0.7f}, dst = 0;
        func_00336A28(src, 0.6f, &dst);
        CHECK_EQ(g_herm_called, 1);
        CHECK_FEQ(g_herm_args[0], 0.6f, 1e-6f);  /* t        */
        CHECK_FEQ(g_herm_args[1], 0.0f, 1e-6f);  /* c0 = 0   */
        CHECK_FEQ(g_herm_args[2], 0.3f, 1e-6f);  /* c1 = src[2] */
        CHECK_FEQ(g_herm_args[3], 0.7f, 1e-6f);  /* c2 = src[3] */
        CHECK_FEQ(g_herm_args[4], 1.0f, 1e-6f);  /* c3 = 1   */
        CHECK_FEQ(dst, 42.5f, 1e-6f);            /* result forwarded */
    }

    return TEST_SUMMARY();
}

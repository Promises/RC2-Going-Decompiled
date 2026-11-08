/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00419860, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* L30's functions at their v2.00 places. */
extern void func_L30_0037CDB0(int, int);
extern void func_L30_002EF590(int, int, int, int, unsigned long);
extern void func_L30_0041A6A0(void *, int, float, float, float, float, float, int);
/* The boot's GS screen context (core, symbol_addrs), seen through upstream's
 * view of the two screen halfwords at +0x150. */
extern struct { char pad[0x150]; short x, y; } g_gsScreenContext;

/**
 * func_L30_00419860 - draw the actor's fading overlay and pulse its light.
 * @obj: the actor; its state block is at +0x68 (halfword timer at +0,
 *       halfword length at +2, light range at +8..+0xC).
 *
 * In the last frame of the run (length - timer < 2, flag +4 set) and with
 * +0x1C set, it sets up a 0x42/0x48 primitive (func_L30_0037CDB0) and fills
 * the screen with colour 0x80808080 (func_L30_002EF590 with the
 * g_gsScreenContext width and height). With +0x18 set, a light of kind 0x3C
 * at the actor's position is emitted through func_L30_0041A6A0, its range
 * blended from +8 to +0xC by 0.1 per frame over the last 10 frames, or by
 * (timer - 0x2D) * 0.1 while the timer is 0x2E..0x31.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0040E660.c, function
 * func_L30_004191E0 (v1.01 address), author commit 8741cd07 (bigbeno37;
 * `git blame -M -C` gives all 18 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_004191E0 -> func_L30_00419860 (our split name);
 *   - D_001A6480 -> g_gsScreenContext (the boot's symbol_addrs name at
 *     exactly 0x001A6480; upstream's struct view kept);
 *   - func_002FC560 -> func_L30_0037CDB0, func_0027DE50 ->
 *     func_L30_002EF590, func_L05_0042F4E8 -> func_L30_0041A6A0 (L30's
 *     copies at their v2.00 places);
 *   - NOTE #13281 codes this entry `l` (`long long`->`long`), a FILE-level
 *     attribution: this body and its declarations hold no `long long` (their
 *     `unsigned long` is upstream's own spelling, kept), so no respelling is
 *     made;
 *   - `#include "include_asm.h"` and the file's other declarations dropped.
 */
void func_L30_00419860(char *obj) {
    char *st = *(char **)(obj + 0x68);
    if (*(short *)(st + 2) - *(short *)st < 2 && *(short *)(st + 4)) {
        if (*(int *)(st + 0x1C)) {
            func_L30_0037CDB0(0x42, 0x48);
            func_L30_002EF590(0, g_gsScreenContext.y, 0, g_gsScreenContext.x, (unsigned long)(int)0x80808080);
        }
    }
    if (*(int *)(st + 0x18)) {
        if (*(short *)(st + 2) - *(short *)st < 10) {
            float t = (*(short *)(st + 2) - *(short *)st) * 0.1f;
            func_L30_0041A6A0(obj + 0x10, 0x3C, *(float *)(st + 8) + (*(float *)(st + 12) - *(float *)(st + 8)) * t, 0.5f, 0.0f, 0.0f, 0.0f, 0);
        } else if ((unsigned short)(*(unsigned short *)st - 0x2E) < 4) {
            float t = (*(short *)st - 0x2D) * 0.1f;
            func_L30_0041A6A0(obj + 0x10, 0x3C, *(float *)(st + 8) + (*(float *)(st + 12) - *(float *)(st + 8)) * t, 0.5f, 0.0f, 0.0f, 0.0f, 0);
        }
    }
}

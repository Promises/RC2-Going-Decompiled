/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00430FD0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack
 * (upstream builds its file at -G8), plain as.
 */

typedef unsigned char u8;
typedef signed int s32;

extern void func_L18_00309640(void *arg0, void *arg1, void *arg2);
extern void func_L18_00309670(void *arg0, void *arg1, void *arg2);
extern void func_L18_00309928(void *arg0, void *arg1, float arg2);
extern float func_L18_00309730(void *arg0, void *arg1);
extern s32 func_L18_00338D30(s32 arg0, s32 arg1, float arg2);
extern void func_L18_00341A78(void *arg0, s32 arg1, float arg2, float arg3);
extern u8 g_cameraPos[];
extern float D_L18_001B14C0 __attribute__((sda));
extern float D_L18_001B14C4 __attribute__((sda));
extern int D_L18_001B14C8 __attribute__((sda));
extern float D_L18_001B14CC __attribute__((sda));

/**
 * func_L18_00430FD0 - play a camera-facing cue when an offset point is
 * far enough from the camera line.
 *
 * @a0: the object; its matrix block is at +0xC0 and its position at +0x10.
 * @a1: the offset to transform.
 *
 * Transforms @a1 by the object's matrix (func_L18_00309B08), adds the
 * position, takes the vector to g_cameraPos and normalises it, and reads the
 * distance t (func_L18_00309730) against the negated matrix row. When t
 * exceeds D_L18_001B14CC it calls func_L18_00341A78 at the point with the
 * handle func_L18_00338D30(D_L18_001B14C8 & 0xFFFFFF, D_L18_001B14C8, t*t)
 * and the two factors D_L18_001B14C4 and D_L18_001B14C0.
 * Return: nothing.
 *
 * Non-obvious: func_L18_00309B08 has no prototype here, as upstream left it
 * (an implicit declaration); the bytes are measured with it so.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_003F80A0-g8.c,
 * function func_L18_0042FAE0 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_0042FAE0 -> func_L18_00430FD0 (our split name at the v2.00 place);
 *   - D_001C97C0 -> g_cameraPos: the name symbol_addrs/L18_boot_names.txt gives at
 *   exactly 0x001B9740 (relverify: inside D_L18_001B2B30 +0x6C10); NAMES rule,
 *   task #2646 class A2;
 *   - D_L18_001B1520 -> D_L18_001B14C0 (our split label at the v2.00 address);
 *   - D_L18_001B1524 -> D_L18_001B14C4 (our split label at the v2.00 address);
 *   - D_L18_001B1528 -> D_L18_001B14C8 (our split label at the v2.00 address);
 *   - D_L18_001B152C -> D_L18_001B14CC (our split label at the v2.00 address);
 *   - func_00283098 -> func_L18_00309B08 (the v2.00 relocation target, our name there);
 *   - func_003088F0 -> func_L18_00309640 (the v2.00 relocation target, our name there);
 *   - func_00308920 -> func_L18_00309670 (the v2.00 relocation target, our name there);
 *   - func_003089E0 -> func_L18_00309730 (the v2.00 relocation target, our name there);
 *   - func_00308BD8 -> func_L18_00309928 (the v2.00 relocation target, our name there);
 *   - func_00337AD8 -> func_L18_00338D30 (the v2.00 relocation target, our name there);
 *   - func_00340828 -> func_L18_00341A78 (the v2.00 relocation target, our name there);
 *   - upstream's `__attribute__((sda))` on the four D_L18_001B14C* externs kept
 *   verbatim (the measured bytes are with it in place);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them).
 */
void func_L18_00430FD0(u8 *a0, void *a1) {
    u8 buf[0x30];
    u8 *s0;
    u8 *s2 = a0 + 0xC0;
    float t;
    func_L18_00309B08(buf, a1, s2);
    func_L18_00309640(buf, a0 + 0x10, buf);
    s0 = buf + 0x10;
    func_L18_00309670(s0, g_cameraPos, buf);
    func_L18_00309928(s0, s0, 1.0f);
    func_L18_00309928(buf + 0x20, s2, -1.0f);
    t = func_L18_00309730(s0, buf + 0x20);
    if (D_L18_001B14CC < t) {
        func_L18_00341A78(buf, func_L18_00338D30(D_L18_001B14C8 & 0xFFFFFF,
                                         D_L18_001B14C8, t * t),
                      D_L18_001B14C4, D_L18_001B14C0);
    }
}

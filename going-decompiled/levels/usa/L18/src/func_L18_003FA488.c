/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003FA488, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned int u32;
typedef int s32;
extern void func_L18_003083C8(void *arg0, s32 arg1, u32 arg2, s32 arg3, float arg4,
                          float arg5, float arg6);
extern void func_L18_0032D808(s32 arg0, s32 arg1, void *arg2);

/**
 * func_L18_003FA488 - draw two glow effects at an object's point 5.
 *
 * @a0: the object; func_L18_0032D808(a0, 5, buf) gives the point.
 *
 * Calls func_L18_003083C8 at that point twice: size 0x2E colour 0x80FFFFFF,
 * then size 0xC colour 0x80F0F020, both with mode 1 and scale 0.25.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_003F80A0.c,
 * function func_L18_003F9018 (v1.01 address), author commit fa8d6672 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_003F9018 -> func_L18_003FA488 (our split name at the v2.00 place);
 *   - func_00281EB8 -> func_L18_003083C8 (the v2.00 relocation target, our name there);
 *   - func_0029FCC0 -> func_L18_0032D808 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them).
 */
void func_L18_003FA488(u8 *a0) {
    u8 buf[0x10];
    func_L18_0032D808((s32)a0, 5, buf);
    func_L18_003083C8(buf, 0x2E, 0x80FFFFFFu, 1, 0.25f, 0.0f, 0.0f);
    func_L18_003083C8(buf, 0xC, 0x80F0F020u, 1, 0.25f, 0.0f, 0.0f);
}

/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x0035C9C8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;

extern char *func_L18_0034C390(s32);
extern s32 rand(void);
extern s32 func_L18_0030A7A8(float);
extern float func_L18_00309C38(float);
extern float func_L18_00309C50(float);
extern u8 *D_L18_001FF454;

/**
 * func_L18_0035C9C8 - spawn an orbiting record and place it on its circle.
 *
 * Allocates a record of type 0x65 (func_L18_0034C390). If one was given, it
 * fills: byte 3 = 0x48 when @p4 is non-null else 0x44, byte 2 from the table
 * D_L18_001FF454[@p3], byte 8 a random value, byte 9 a random value from
 * func_L18_0030A7A8(1.0) + 0x40, word 4 = @p2 and +0xC = @f3 * 210000. The
 * orbit block at +0x20 takes the centre @p0, radius @f0, angle @f1, rate @f2,
 * step @p1 and +0x10 = @f4 * 210000. The position (+0x10/+0x14/+0x18) is the
 * centre plus radius * func_L18_00309C38/func_L18_00309C50 of the angle on x/z,
 * with the centre's y.
 * Return: the record, or NULL when none was free.
 *
 * Non-obvious: x uses @f1 directly while z re-reads the stored angle; the
 * word stores of the pointers are through u32 casts, as upstream wrote them.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_002E68B0.c,
 * function func_L18_0035B6B8 (v1.01 address), author commit 5030340d (Flavius); its
 * parameter order was changed later by a0ffc4d1 (Flavius), which this text carries.
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_0035B6B8 -> func_L18_0035C9C8 (our split name at the v2.00 place);
 *   - D_L18_001FF4D4 -> D_L18_001FF454: an address-named extern (RULING #13291 section 2),
 *   inside the larger split label D_L18_001F0000 (+0xF454); no name exists at that exact address
 *   in the boot splits, symbol_addrs.txt, L18_boot_names.txt or the level's
 *   split labels (grep with firing controls, task #2662);
 *   - func_001163B0 -> rand (our boot name at that address);
 *   - func_00283180 -> func_L18_00309C38 (the v2.00 relocation target, our name there);
 *   - func_00283198 -> func_L18_00309C50 (the v2.00 relocation target, our name there);
 *   - func_00283CF0 -> func_L18_0030A7A8 (the v2.00 relocation target, our name there);
 *   - func_002B9030 -> func_L18_0034C390 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them);
 *   - upstream's function comment folded into this one.
 */
char *func_L18_0035C9C8(char *p0, float f0, float f1, float f2, float f3, float f4, s16 p1,
                         char *p2, s32 p3, char *p4)
{
    char *s0 = func_L18_0034C390(0x65);
    char *s1 = s0 + 0x20;

    if (s0 != 0) {
        *(u8 *)(s0 + 3) = (p4 != 0) ? 0x48 : 0x44;
        *(u8 *)(s0 + 1) = 0;
        *(u8 *)(s0 + 2) = D_L18_001FF454[p3];
        *(u8 *)(s0 + 8) = rand();
        *(u8 *)(s0 + 9) = func_L18_0030A7A8(1.0f) + 0x40;
        *(u32 *)(s0 + 4) = p2;
        *(float *)(s0 + 0xC) = f3 * 210000.0f;
        *(u32 *)(s1 + 0) = p0;
        *(float *)(s1 + 4) = f0;
        *(float *)(s1 + 8) = f1;
        *(float *)(s1 + 0xC) = f2;
        *(u16 *)(s1 + 0x14) = p1;
        *(float *)(s1 + 0x10) = f4 * 210000.0f;
        *(float *)(s0 + 0x10) = *(float *)(*(char **)(s1 + 0)) +
                                *(float *)(s1 + 4) * func_L18_00309C38(f1);
        *(float *)(s0 + 0x18) = *(float *)(*(char **)(s1 + 0) + 8) +
                                *(float *)(s1 + 4) * func_L18_00309C50(*(float *)(s1 + 8));
        *(float *)(s0 + 0x14) = *(float *)(*(char **)(s1 + 0) + 4);
    }
    return s0;
}

/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003CDD20, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

typedef int TI128 __attribute__((mode(TI)));
typedef union {
    TI128 q;
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    } f;
} Vec4;
extern void func_L17_002F07C0(void *, void *);
extern void func_L17_002EFCC8(void *, void *, void *);
extern void func_L17_002F01A0(void *, void *, void *);
extern f32 func_L17_002EFBD8(f32);

/**
 * func_L17_003CDD20 - is a point inside the object's box on its local x/y?
 *
 * @a0: the object (position +0x10, matrix +0xC0).
 * @a1: the point, a 16-byte vector.
 * @f1: the bound on local y.
 * @f2: the bound on local x.
 * Return: 1 when |x| < @f2 and |y| < @f1, else 0.
 *
 * It builds a matrix from +0xC0 (func_L17_002F07C0), takes the point
 * relative to the position (func_L17_002EFCC8), turns it by that matrix
 * (func_L17_002F01A0) and compares the absolute values (func_L17_002EFBD8).
 *
 * Non-obvious: the bounds come in swapped, the second float for x.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_003C7C70.c, function func_L17_003CC798 (v1.01 address), author commit 4541c4ce (ReaperOfVeriod).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_003CC798 -> func_L17_003CDD20 (our split name at the v2.00 place);
 *   - func_00282C48 -> func_L17_002EFBD8 (the v2.00 relocation target, our name there);
 *   - func_00282CF0 -> func_L17_002EFCC8 (the v2.00 relocation target, our name there);
 *   - func_002830C0 -> func_L17_002F01A0 (the v2.00 relocation target, our name there);
 *   - func_00283698 -> func_L17_002F07C0 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
s32 func_L17_003CDD20(u8 *a0, TI128 *a1, f32 f1, f32 f2) {
    Vec4 v[1];
    Vec4 mat[4];
    Vec4 w[1];
    Vec4 out[1];

    v[0].q = *a1;
    func_L17_002F07C0(mat, a0 + 0xC0);
    func_L17_002EFCC8(&out[0].q, &v[0].q, a0 + 0x10);
    w[0].q = out[0].q;
    func_L17_002F01A0(&w[0].q, &w[0].q, mat);
    if (func_L17_002EFBD8(w[0].f.x) < f2) {
        if (func_L17_002EFBD8(w[0].f.y) < f1)
            return 1;
    }
    return 0;
}

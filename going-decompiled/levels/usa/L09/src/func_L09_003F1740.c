/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003F1740, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The level's copy of a boot helper, named by its v2.00 place in L09. */
extern void func_L09_00317AF8(unsigned char *, int, int);

/**
 * func_L09_003F1740 - arm the object's sub-object and start it.
 *
 * Writes the sub-object's angle 10.0f (+0x20), halfword 10 (+0x24), byte 3
 * (+0x28), scale 1.0f (+0x30), live byte 1 (+0x2F), copies its word +0xA4
 * to +0x8C, then runs the resident starter func_L09_00317AF8(a0, 0, 1).
 *
 * @a0: the object; its sub-object is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_002F8BA0.c, function func_L09_003F02D0 (v1.01
 * address), author commit d5fd91e5 (Louis-Philippe Le Sieur). Adopted with
 * the authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM
 * but for names (no device); the respellings, each to the target of the
 * v2.00 disc's own relocation at this place (task #2561 relverify):
 *   - func_L09_003F02D0 -> func_L09_003F1740 (our split name at the v2.00 place);
 *   - func_002A7750_3 -> func_L09_00317AF8;
 *   - `#include "common.h"` dropped: only the declarations this function uses
 *     are kept, and upstream's function comment is folded into this one.
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the whole upstream file fails on this arm; this
 * function's own text has no `long long`, so alone in its TU it needs no
 * respelling.
 */
void func_L09_003F1740(unsigned char *a0) {
    unsigned char *state = *(unsigned char **)(a0 + 0x68);

    *(float *)(state + 0x20) = 10.0f;
    *(short *)(state + 0x24) = 10;
    *(unsigned char *)(state + 0x28) = 3;
    *(float *)(state + 0x30) = 1.0f;
    *(unsigned char *)(state + 0x2F) = 1;
    *(int *)(state + 0x8C) = *(int *)(state + 0xA4);
    func_L09_00317AF8(a0, 0, 1);
}

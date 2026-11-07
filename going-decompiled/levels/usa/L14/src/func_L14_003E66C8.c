/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E66C8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/* Upstream include/common.h's own definition of the one typedef the body uses
 * (their `#include "common.h"` is not carried; nothing else of it is used
 * here). */
typedef signed short       s16;

/**
 * func_L14_003E66C8 - pack the object's key block into its target's state.
 *
 * @arg0: the object; its state block pointer is at +0x68.
 * Return: nothing.
 *
 * Non-obvious: state +0x30 points at the target (checked for null) and the
 * target's +0x68 at its state block. Two 24-bit colours are built from the
 * block's six halfwords at +0x40..+0x4A (each sign-extended by the s16 cast,
 * masked to 8 bits by the shift) and written with the packed flag +0x18, the
 * enabled word +0x3C and the two colour words +0x48/+0x4C.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c, function
 * func_L14_003E51B0 (v1.01 address), author commit ee3fee45 (llesieur99; the
 * definition line blames ee3fee45 too). Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for
 * names; edits:
 *   - name func_L14_003E51B0 -> func_L14_003E66C8 (our split name at the
 *     v2.00 place);
 *   - the typedef s16 written out with upstream include/common.h's own
 *     definition (the body uses it; `#include "common.h"` dropped);
 *   - the file's other functions and their externs not carried.
 */
void func_L14_003E66C8(char *arg0) {
    char *v0 = *(char **)(arg0 + 0x68);
    char *v1 = *(char **)(v0 + 0x30);

    if (v1 != 0) {
        char *v2 = *(char **)(v1 + 0x68);

        *(int *)(v2 + 0x48) = ((unsigned int)*(s16 *)(v0 + 0x44) << 16)
                              | ((unsigned int)*(s16 *)(v0 + 0x42) << 8)
                              | (unsigned int)*(s16 *)(v0 + 0x40);
        *(int *)(v2 + 0x4C) = ((unsigned int)*(s16 *)(v0 + 0x4A) << 16)
                              | ((unsigned int)*(s16 *)(v0 + 0x48) << 8)
                              | (unsigned int)*(s16 *)(v0 + 0x46);
        *(int *)(v2 + 0x3C) = 1;
        *(int *)(v2 + 0x34) = *(int *)(v0 + 0x18);
    }
}

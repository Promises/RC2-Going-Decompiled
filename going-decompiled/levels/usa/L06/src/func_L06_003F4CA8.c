/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003F4CA8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern int func_L06_0035CAF8(int a0, int a1);

/**
 * func_L06_003F4CA8 - count a spawner's live children of class 0x75E
 * and find a slot to reuse.
 * @a0: unused.
 * @a1: the spawner block: 20 child moby pointers at +0xA0, a handle at +0x84,
 *      a count byte at +0x81.
 *
 * Seeds the count byte with func_L06_0035CAF8(handle, -1), then for each slot
 * adds one per child that is non-null, of class 0x75E (+0xAA) and whose state
 * byte (+0x20) is neither 0xFE nor 0xFD.
 * Return: the index of the LAST slot that failed that test, or -1 when all
 * 20 hold live children.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0033BDC0.c, function
 * func_L06_003F3760 (v1.01 address), author commit 52e801be (llesieur99;
 * `git blame -M -C` and `git log -S` agree). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2653 relverify by name); edits:
 *   - func_L06_003F3760 -> func_L06_003F4CA8 (our split name at the v2.00
 *     place)
 *   - func_002A8508 -> func_L06_0035CAF8 (callees respelled to L06's own
 *     copies at their v2.00 places; level-copy callee names stay
 *     func_L06_<addr>)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
int func_L06_003F4CA8(int a0, char *a1)
{
    char *s0 = a1;
    int free_slot = -1;
    int i;

    *(unsigned char *)(s0 + 0x81) = func_L06_0035CAF8(*(int *)(s0 + 0x84), -1);
    for (i = 0; i < 20; i++) {
        char *m = ((char **)(s0 + 0xA0))[i];

        if (m != 0 && *(short *)(m + 0xAA) == 0x75E
            && *(unsigned char *)(m + 0x20) != 0xFE && *(unsigned char *)(m + 0x20) != 0xFD)
            (*(unsigned char *)(s0 + 0x81))++;
        else
            free_slot = i;
    }
    return free_slot;
}

/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003C73A8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm ee-gcc 2.9-ee-991111b
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

extern char g_soundBankHandlesBlk[];

/**
 * func_L17_003C73A8 - is @a0 the object the shared record currently names?
 *
 * @a0: an object pointer, compared as an int.
 * Return: 1 or 0.
 *
 * Reads g_soundBankHandlesBlk. When its word +0x229C is 3 or its word
 * +0x2294 is 0x1B, the answer is whether the word +0x588 equals @a0.
 * Otherwise it is 0 when the halfword +0x34E is non-zero, else whether the
 * word +0x33C equals @a0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_0032C070.c, function func_L17_003C5E20 (v1.01 address), author commit d078020b (Louis-Philippe Le Sieur).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_003C5E20 -> func_L17_003C73A8 (our split name at the v2.00 place);
 *   - D_00189E20 -> g_soundBankHandlesBlk (our boot name at that address);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
int func_L17_003C73A8(int a0)
{
    char *base = g_soundBankHandlesBlk;

    if (*(int *)(base + 0x229C) == 3 || *(int *)(base + 0x2294) == 0x1B) {
        if (*(int *)(base + 0x588) != a0)
            goto zero;
        return 1;
    }
    if (*(short *)(base + 0x34E) != 0)
        goto zero;
    if (*(int *)(base + 0x33C) == a0)
        return 1;
zero:
    return 0;
}

/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00457DD0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L11's own function and its copy of boot code, at their v2.00 places. */
extern int func_L11_004430B8(char *p, int id);
extern int func_L11_0033DAC8(char *, int, float, float, int, int);

/**
 * func_L11_00457DD0 - keep the actor's tracking flag and target id current
 * (the variant with its state at +0x240..+0x256).
 *
 * @arg0: the actor; its state block is at +0x68, its stage byte at +0x20.
 *
 * Below stage 4 it clears state+0x240 and the tracking flag state+0x256.
 * When tracking already, or when func_L11_004430B8 accepts the actor's group
 * and id (then it sets tracking), it stores
 * func_L11_0033DAC8(arg0, state+0x24C, 100.0f, 100.0f, 0, 0) at state+0x254
 * and copies state+0x220 to state+0x240. Otherwise it clears tracking.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00453358.c, function
 * func_L11_00456378 (v1.01 address), author commit 5964c620 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged (verbatim but for
 * names; no device); edits:
 *   - name func_L11_00456378 -> func_L11_00457DD0 (our split name at the
 *     v2.00 place);
 *   - func_L11_00441868 -> func_L11_004430B8 and func_002B3BA8_6378 ->
 *     func_L11_0033DAC8: the targets of the v2.00 ROM's own relocations
 *     (relverify, task #2553). Upstream's _6378 suffix only gave this
 *     function's prototype its own name;
 *   - the audit's `long long` -> `long` respelling (RULING #13197 class,
 *     needed because cc1 s136 refuses the file's other `long long` uses) is
 *     NOT applied: this one-function TU carries no `long long`;
 *   - `#include "common.h"` dropped (no typedef of it is used here); the
 *     upstream one-line comment ("same body as func_L02_003F16F0") not
 *     carried; the file's other functions and their externs not carried.
 *     As upstream, an int is passed for func_L11_004430B8's pointer
 *     parameter.
 */
void func_L11_00457DD0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x20) < 4) {
        *(int *)(state + 0x240) = 0;
        goto done;
    }

    if (*(short *)(state + 0x256) == 1) {
        *(short *)(state + 0x254) =
            func_L11_0033DAC8(arg0, *(int *)(state + 0x24C), 100.0f, 100.0f, 0, 0);
        *(int *)(state + 0x240) = *(int *)(state + 0x220);
    } else {
        if (func_L11_004430B8(*(int *)(arg0 + 0xB8), *(short *)(arg0 + 0xAA)) == 0)
            goto done;
        *(short *)(state + 0x256) = 1;
        *(short *)(state + 0x254) =
            func_L11_0033DAC8(arg0, *(int *)(state + 0x24C), 100.0f, 100.0f, 0, 0);
        *(int *)(state + 0x240) = *(int *)(state + 0x220);
    }
    return;

done:
    *(short *)(state + 0x256) = 0;
}

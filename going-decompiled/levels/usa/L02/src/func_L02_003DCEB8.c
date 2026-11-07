/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003DCEB8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm the SDK 2.9 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/**
 * func_L02_003DCEB8 - test whether the actor's linked object is weak enough to
 * finish.
 *
 * @arg0: the actor; its state block is at +0x68.
 *
 * The state names the other object at +0x264. With no object, or an object
 * whose byte +0x20 is 0xFE, 0xFD or below 3, the answer is 0. Otherwise it is
 * 1 when the state's float at +0x20 is below
 * (float)half(+0x24) * (3 - (float)half(+0x282)) * (1/3).
 * Return: 1 or 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003DB310.c,
 * function func_L02_003DB950 (v1.01 address), author commit 773eafc2 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003DB950 -> func_L02_003DCEB8 (our split name at
 *     the v2.00 place);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
int func_L02_003DCEB8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *other = *(char **)(state + 0x264);
    unsigned char kind;
    int result = 0;

    if (other == 0)
        goto done;
    kind = *(unsigned char *)(other + 0x20);
    if (kind == 0xFE)
        goto done;
    if (kind == 0xFD)
        goto done;
    if (kind < 3)
        goto done;
    if (*(float *)(state + 0x20) <
        (float)*(short *)(state + 0x24) *
            (3.0f - (float)*(short *)(state + 0x282)) * 0.33333334f)
        result = 1;
done:
    return result;
}

/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FD670, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm the SDK 2.9 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef int s32;

/**
 * func_L02_003FD670 - claim one tick of the timer for a class.
 *
 * @a0: the actor; its state block is at +0x68.
 * @a1: the class: 0x9CA, 0xB22, 0xC3B or 0xC3E name the counters at state
 *      +0x50, +0x54, +0x58 and +0x5C; 0x9F8, 0xA44 and anything else give 0.
 *
 * A counter above 0 is decremented.
 * Return: 1 when a tick was claimed, else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003F5430.c,
 * function func_L02_003FC0D0 (v1.01 address), author commit 455cde53 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003FC0D0 -> func_L02_003FD670 (our split name at
 *     the v2.00 place);
 *   - s32 typedef carried from the file's own;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
s32 func_L02_003FD670(char *a0, s32 a1) {
    char *p = *(char **)(a0 + 0x68);

    switch (a1) {
    case 0x9CA:
        if (*(s32 *)(p + 0x50) > 0) {
            *(s32 *)(p + 0x50) -= 1;
            return 1;
        }
        break;
    case 0xB22:
        if (*(s32 *)(p + 0x54) > 0) {
            *(s32 *)(p + 0x54) -= 1;
            return 1;
        }
        break;
    case 0xC3B:
        if (*(s32 *)(p + 0x58) > 0) {
            *(s32 *)(p + 0x58) -= 1;
            return 1;
        }
        break;
    case 0xC3E:
        if (*(s32 *)(p + 0x5C) > 0) {
            *(s32 *)(p + 0x5C) -= 1;
            return 1;
        }
        break;
    case 0x9F8:
    case 0xA44:
        return 0;
    default:
        break;
    }
    return 0;
}

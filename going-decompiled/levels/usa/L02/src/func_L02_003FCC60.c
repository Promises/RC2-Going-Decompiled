/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FCC60, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef short s16;
typedef int s32;

extern int func_L02_003D8D20(char *);
extern void func_L02_003D8DD0(char *, char *);

/**
 * func_L02_003FCC60 - fill the list's current slot with an object and count it.
 *
 * @a0: the actor; its state block is at +0x68, a list of slots at +0xA8
 *      indexed by the word at +0xD8.
 * @a1: the object.
 *
 * When the current slot holds an entry that func_L02_003D8D20 accepts, the
 * object goes to it (func_L02_003D8DD0); for class (+0xAA) 0x9CA, 0xB22,
 * 0xC3B or 0xC3E the matching counter at state +0x90..+0x9C is bumped and the
 * object's state float +0x20 set to 1.
 * Return: 1 when the slot took the object, else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003F5430.c,
 * function func_L02_003FB6C0 (v1.01 address), author commit 455cde53 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003FB6C0 -> func_L02_003FCC60 (our split name at
 *     the v2.00 place);
 *   - func_L02_003D77B8 -> func_L02_003D8D20 and func_L02_003D7868 ->
 *     func_L02_003D8DD0 (v2.00 places);
 *   - s16/s32 typedefs carried from the file's own;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
s32 func_L02_003FCC60(char *a0, char *a1) {
    char *s0 = *(char **)(a0 + 0x68);
    char **list = (char **)(s0 + 0xA8);
    char **slot = list + *(s32 *)(s0 + 0xD8);
    char *e = *slot;

    if (e != 0) {
        if (func_L02_003D8D20(e) != 0) {
            char **cur = list + *(s32 *)(s0 + 0xD8);

            func_L02_003D8DD0(*cur, a1);
            switch (*(s16 *)(a1 + 0xAA)) {
            case 0x9CA:
                *(s32 *)(s0 + 0x90) += 1;
                *(float *)(*(char **)(a1 + 0x68) + 0x20) = 1.0f;
                break;
            case 0xB22:
                *(s32 *)(s0 + 0x94) += 1;
                *(float *)(*(char **)(a1 + 0x68) + 0x20) = 1.0f;
                break;
            case 0xC3B:
                *(s32 *)(s0 + 0x98) += 1;
                *(float *)(*(char **)(a1 + 0x68) + 0x20) = 1.0f;
                break;
            case 0xC3E:
                *(s32 *)(s0 + 0x9C) += 1;
                *(float *)(*(char **)(a1 + 0x68) + 0x20) = 1.0f;
                break;
            default:
                return 1;
            }
            return 1;
        }
    }
    return 0;
}

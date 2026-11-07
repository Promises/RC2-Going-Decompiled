/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E6730, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/**
 * func_L14_003E6730 - on an odd key index, hand both linked objects their key
 * delta and colours.
 *
 * @self: the object; its state block pointer is at +0x68, the key index
 *        +0x16 and the key word +0x14 of that block.
 * Return: nothing.
 *
 * Non-obvious: only an odd index (of the 8-entry ring at state +0x16, mod 8)
 * acts. It reads the ring entries index and index+1 as the keys a and b,
 * then for each of the two object pointers at state +0x28 and +0x2C (null
 * checked separately): writes the deltas (+0x14 = key word - a, +0x18 = a -
 * b), two packed RGB words built from state +0x40..+0x4A with the 0x80000000
 * flag bit (at +0x30 = +0x38 and +0x34 = +0x3C), and a float 1.0 at +0xC.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c, function
 * func_L14_003E5218 (v1.01 address), author commit 4912fc03 (bigbeno37; it
 * replaced the function's INCLUDE_ASM marker with this body; `git log
 * --reverse -S` names 564fc202/bigmak94 instead because that commit added a
 * same-text extern declaration of the name earlier in the file - the
 * FACT #37795 class). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_003E5218 -> func_L14_003E6730 (our split name at the
 *     v2.00 place);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing here uses them).
 */
void func_L14_003E6730(char *self) {
    char *p = *(char **)(self + 0x68);
    short index = *(short *)(p + 0x16);
    if ((short)(index % 2) == 1) {
        int a = ((short *)p)[index];
        int b = ((short *)p)[(index + 1) % 8];
        char *obj;
        char *d;
        int green;
        obj = *(char **)(p + 0x28);
        if (obj) {
            d = *(char **)(obj + 0x68);
            *(int *)(d + 0x14) = *(short *)(p + 0x14) - a;
            *(int *)(d + 0x18) = a - b;
            green = (*(short *)(p + 0x42) << 8) | 0x80000000;
            *(int *)(d + 0x30) = *(int *)(d + 0x38) = (*(short *)(p + 0x44) << 16) | green | *(short *)(p + 0x40);
            green = (*(short *)(p + 0x48) << 8) | 0x80000000;
            *(int *)(d + 0x34) = *(int *)(d + 0x3C) = (*(short *)(p + 0x4A) << 16) | green | *(short *)(p + 0x46);
            *(float *)(d + 0xC) = 1.0f;
        }
        obj = *(char **)(p + 0x2C);
        if (obj) {
            d = *(char **)(obj + 0x68);
            *(int *)(d + 0x14) = *(short *)(p + 0x14) - a;
            *(int *)(d + 0x18) = a - b;
            green = (*(short *)(p + 0x42) << 8) | 0x80000000;
            *(int *)(d + 0x30) = *(int *)(d + 0x38) = (*(short *)(p + 0x44) << 16) | green | *(short *)(p + 0x40);
            green = (*(short *)(p + 0x48) << 8) | 0x80000000;
            *(int *)(d + 0x34) = *(int *)(d + 0x3C) = (*(short *)(p + 0x4A) << 16) | green | *(short *)(p + 0x46);
            *(float *)(d + 0xC) = 1.0f;
        }
    }
}

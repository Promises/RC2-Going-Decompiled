/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E65D0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* Upstream include/common.h's own definitions of the three typedefs the body
 * uses (their `#include "common.h"` is not carried; nothing else of it is
 * used here). */
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef signed short       s16;

extern void func_L14_00323250();

/**
 * func_L14_003E65D0 - keep a ring cursor ahead of its write index and restage
 * the object accordingly.
 *
 * @object: the object; its state block pointer is at +0x68, its stage byte
 *          +0x20.
 * Return: nothing.
 *
 * Non-obvious: the state block is walked as an s16 array; slots 10 and 11 are
 * the read and write cursors of an 8-entry ring. While the read cursor lags
 * the write cursor it advances (mod 8). An even cursor puts the object in
 * stage 1, an odd one in stage 2, each only when the stage byte differs
 * (func_L14_00323250).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c, function
 * func_L14_003E50B8 (v1.01 address), author commit b9d9d0d4 (bigbeno37; it
 * replaced the function's INCLUDE_ASM marker with this body; `git log
 * --reverse -S` names 564fc202/bigmak94 instead because that commit added a
 * same-text extern declaration of the name earlier in the file - the
 * FACT #37795 class). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_003E50B8 -> func_L14_003E65D0 (our split name at the
 *     v2.00 place);
 *   - func_321F58 -> func_L14_00323250 (the level's own copy of boot
 *     func_002A7750 at its L14 place);
 *   - the typedefs u8/u16/s16 written out with upstream include/common.h's
 *     own definitions (the body uses them; `#include "common.h"` dropped);
 *   - the file's other functions and their externs not carried.
 */
void func_L14_003E65D0(char *object)
{
    s16 *ring = *(s16 **)(object + 0x68);
    while (ring[10] < ring[ring[11]]) {
        ring[11] = (ring[11] + 1) % 8;
    }
    if (((u16)ring[11] & 1) == 0) {
        if (*(u8 *)(object + 0x20) != 1)
            func_L14_00323250(object, 1, -1);
    } else {
        if (*(u8 *)(object + 0x20) != 2)
            func_L14_00323250(object, 2, -1);
    }
}

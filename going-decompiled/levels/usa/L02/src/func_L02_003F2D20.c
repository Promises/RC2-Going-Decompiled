/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F2D20, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;

/* L02's copies of boot helpers, at their v2.00 places in this level. */
extern void func_L02_0031F998(char *, int, int);
extern void func_L02_00333AC0(u8 *, float, float, float, float);
extern void func_L02_00317418(void *, u8, u8, u8);

/**
 * func_L02_003F2D20 - bring the actor (back) into play.
 *
 * @arg0: the actor; its state block is at +0x68.
 * @arg1: stored as the halfword at state +0x4F0.
 *
 * Resets the state's float +0x20 from its halfword +0x24, clears +0x98, runs
 * func_L02_00333AC0 with four constants, moves to state 3
 * (func_L02_0031F998(arg0, 3, 0)), copies the word at +0x10 of the object at
 * +0x24 to +0x98, makes the actor visible (+0x31 = 1, flags |= 0x1000, low
 * bits cleared) and does the same for the two sub-objects at state +0x4C0.
 * A pending colour (+0x60) is cleared and applied with
 * func_L02_00317418(arg0, r, g, b) from +0x64..+0x66, setting +0x62.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003DB310.c,
 * function func_L02_003F17B0 (v1.01 address), author commit e80a585a (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003F17B0 -> func_L02_003F2D20 (our split name at
 *     the v2.00 place);
 *   - func_002B60B0 -> func_L02_00333AC0, func_002A0488 ->
 *     func_L02_00317418 and func_002A7750 -> func_L02_0031F998 (L02's
 *     copies);
 *   - the file's second, unprototyped `extern void func_002A7750();`
 *     not carried (the prototype it composes with is);
 *   - u8/u16 typedefs written here (upstream takes them from common.h);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L02_003F2D20(char *arg0, int arg1) {
    char *state = *(char **)(arg0 + 0x68);
    int i;

    *(float *)(state + 0x20) = (float)*(short *)(state + 0x24);
    *(u16 *)(state + 0x98) = 0;
    func_L02_00333AC0((u8 *)arg0, 0.004444444552063942f, 0.13333334028720856f,
                  0.0011635529808700085f, 0.10471977293491364f);
    func_L02_0031F998(arg0, 3, 0);
    *(int *)(arg0 + 0x98) = *(int *)(*(char **)(arg0 + 0x24) + 0x10);
    *(u8 *)(state + 0x7A) = 0;
    *(u8 *)(arg0 + 0x31) = 1;
    *(u8 *)(arg0 + 0x7F) = 0;
    *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 0x1000) & 0xFFB8;
    {
        char **slot = (char **)(state + 0x4C0);

        for (i = 1; i >= 0; i--) {
            if (*slot != 0) {
                *(u16 *)(*slot + 0x34) &= 0xFFFE;
                *(u8 *)(*slot + 0x31) = 1;
                *(int *)(*slot + 0x98) = *(int *)(*(int *)(*slot + 0x24) + 0x10);
            }
            slot++;
        }
    }
    {
        short flag = *(short *)(state + 0x60);

        *(u16 *)(state + 0x4F0) = arg1;
        *(u16 *)(state + 0x4EA) = 0;
        if (flag != 0) {
            *(short *)(state + 0x60) = 0;
            func_L02_00317418(arg0, *(u8 *)(state + 0x64), *(u8 *)(state + 0x65),
                          *(u8 *)(state + 0x66));
            *(short *)(state + 0x62) = 1;
        }
    }
}

/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x003ECF28, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L07_00311AE0(int, int, int);

extern int func_L07_00311BF0(int, int);

extern void func_L07_002E2C98(char *, int, int, char *, int, int, int);

/**
 * func_L07_003ECF28 - arm the object's mode eight and hand it to the shared
 * launcher.
 *
 * func_L07_00311AE0(@arg, 8, -1) sets the mode. The object's u16 flags at
 * +0x34 lose 0x41 and gain 0x1000, and its +0x98 word takes the +0x10 word of
 * its class data (+0x24). A random value from func_L07_00311BF0(0x2D, 0x4B)
 * then goes, with the caller's four arguments and the object's +0xF0 block,
 * to func_L07_002E2C98.
 *
 * @arg: the object.
 * @a1, @a2, @a3, @a4: passed through to func_L07_002E2C98.
 * Return: none.
 *
 * Non-obvious: the twin of func_L07_003E86C0, with mode eight instead of
 * ten.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_00389EB8.c, function func_L07_003EBBE8
 * (v1.01 address), author commit 91e04732 (llesieur99), by blame -M -C of the
 * definition (all 10 lines; no rename-copy found: it differs from
 * func_L07_003E7380 in the mode constant).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2656 relverify by
 * name):
 *   - func_L07_003EBBE8 -> func_L07_003ECF28 (our split name);
 *   - func_002A7750 -> func_L07_00311AE0, func_002A7820 -> func_L07_00311BF0,
 *     func_L07_002E2090 -> func_L07_002E2C98 (L07's own copies at the v2.00
 *     places; upstream's `int` first parameter for func_L07_00311AE0 kept,
 *     so @arg converts implicitly as upstream has it);
 *   - `#include "common.h"`/`#include "include_asm.h"` and the file's other
 *     declarations dropped: this function uses no common.h typedef; and
 *     upstream's one-line comment folded into this one.
 */
void func_L07_003ECF28(char *arg, int a1, int a2, int a3, int a4)
{
    int r;

    func_L07_00311AE0(arg, 8, -1);
    *(unsigned short *)(arg + 0x34) = (*(unsigned short *)(arg + 0x34) & 0xFFBE) | 0x1000;
    *(int *)(arg + 0x98) = *(int *)(*(char **)(arg + 0x24) + 0x10);
    r = func_L07_00311BF0(0x2D, 0x4B);
    func_L07_002E2C98(arg, a1, a2, arg + 0xF0, r, a3, a4);
}

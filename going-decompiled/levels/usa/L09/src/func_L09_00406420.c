/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00406420, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef int s32;
typedef float f32;

extern int func_L09_00328FE8(char *, char *);
extern int func_L09_00329040(char *, float *, float);

/**
 * func_L09_00406420 - route a query either to the level's dialogue path or
 * through its weighted path.
 *
 * With @a2 zero the pair (@a0, @a1) goes to func_L09_00328FE8 unchanged;
 * otherwise func_L09_00329040 receives the state's float at +0x33C (state is
 * at *(char **)(a0+0x68)) as the weight for @a1.
 *
 * @a0: the object.
 * @a1: the query payload.
 * @a2: nonzero to take the weighted path.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00404F90 (v1.01
 * address), author commit 264cb2be (llesieur99), with later body changes
 * a541245e (the signature and the func_L01_00321178 call) and 3638916f (the
 * func_002B4FF0 call), both Louis-Philippe Le Sieur, the same account
 * (`git blame` of the definition lines). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_00404F90 -> func_L09_00406420 (our split name);
 *   - func_L01_00321178 -> func_L09_00328FE8 and
 *     func_002B4FF0 -> func_L09_00329040 (L09's own copies at the v2.00
 *     places);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_00406420(char *a0, char *a1, s32 a2) {
    char *p = *(char **)(a0 + 0x68);
    if (a2 == 0) {
        func_L09_00328FE8(a0, a1);
    } else {
        func_L09_00329040(a0, (float *)a1, *(f32 *)(p + 0x33C));
    }
}

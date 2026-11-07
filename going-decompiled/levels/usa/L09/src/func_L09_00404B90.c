/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00404B90, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef int s32;
typedef unsigned char u8;

extern int func_L09_003195F0(float x, float y, float z, void *object, int bone, int mode, int first, int second, int third);

/**
 * func_L09_00404B90 - while the object is in state 7, sparks four of its
 * bones through the spark thrower with the controller's byte as the second
 * coordinate.
 *
 * The state is at *(char **)(a0+0x68). When the object's state byte at +0x20
 * is 7, four sparks are thrown (bones 0..3) through
 * func_L09_003195F0(0.17f, state's u8 at +0x34, 1.0f, @a0, bone, 0x4000001,
 * 0, 1, 0).
 *
 * @a0: the object.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_00403700.c, function func_L09_00403700 (v1.01
 * address), author commit 3a4945e2 (Flavius), which finished upstream's
 * near-miss draft nonmatching/levels/func_L09_00403700.c (96fa46bb,
 * Louis-Philippe Le Sieur; 1d0b1d4c, ReaperOfVeriod) and deleted it in the
 * same commit: `git blame -C -C` traces the signature, the locals and the
 * state test (5 of the 9 definition lines) to that draft, and the loop and
 * call to 3a4945e2 (RULING #13427). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_00403700 -> func_L09_00404B90 (our split name);
 *   - func_002A8D70 -> func_L09_003195F0 (L09's own copy);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_00404B90(char *a0) {
    char *s2 = *(char **)(a0 + 0x68);
    s32 i;

    if (*(u8 *)(a0 + 0x20) == 7) {
        for (i = 0; i < 4; i++)
            func_L09_003195F0(0.17f, *(u8 *)(s2 + 0x34), 1.0f, a0, i, 0x4000001, 0, 1, 0);
    }
}

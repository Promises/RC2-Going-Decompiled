/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x0037A3F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

extern char *D_L07_001AEBC0 __attribute__((sda));

extern float func_L07_002E6088(float a0, float a1);

extern void func_L07_00339EF0(int a0, int a1, char *a2, void *a3, int a4, float f12);

extern void func_L07_002E5778(char *a0, void *a1);

/**
 * func_L07_0037A3F0 - sample a path at a wrapped distance and hand the result
 * to the state's +0x20 consumer.
 *
 * The state block is the +0x68 pointer of the object D_L07_001AEBC0 points
 * at. The distance is func_L07_002E6088(@a0 + 45, 1000), passed in $f12 to
 * func_L07_00339EF0 together with the state's +0x14 word, 1, the state's
 * +0x50 block, a local result word and 0. That local word then goes to
 * func_L07_002E5778 with the state's +0x20 block.
 *
 * @a0: the base distance; 45 is added before the 1000 wrap.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_002BE770-g8.c, function func_L07_00379180
 * (v1.01 address), author commit ee3fee45 (llesieur99), by blame -M -C of the
 * definition (all 7 lines; no draft found by `log -S`, no rename-copy found).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2656 relverify by
 * name):
 *   - func_L07_00379180 -> func_L07_0037A3F0 (our split name);
 *   - D_L07_001AEC08 -> D_L07_001AEBC0 (our committed split label at the
 *     v2.00 target; upstream's `sda` marker kept, inert at -G8 where a 4-byte
 *     object is $gp-relative by its size alone);
 *   - func_2E5480 -> func_L07_002E6088, func_338D20 -> func_L07_00339EF0,
 *     func_2E4B70 -> func_L07_002E5778 (L07's own copies at the v2.00
 *     places);
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped: this function uses no common.h typedef.
 */
void func_L07_0037A3F0(float a0) {
    char *s0 = *(char **)(D_L07_001AEBC0 + 0x68);
    int v;

    func_L07_00339EF0(*(int *)(s0 + 0x14), 1, s0 + 0x50, &v, 0, func_L07_002E6088(a0 + 45.0f, 1000.0f));
    func_L07_002E5778(s0 + 0x20, &v);
}

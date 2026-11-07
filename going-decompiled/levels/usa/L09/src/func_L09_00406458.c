/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00406458, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned short u16;
typedef int s32;
typedef float f32;

extern void func_L09_002EAD98(char *p);

extern void func_L09_003206A0(char *a0, char *a1, s32 a2, f32 f12, f32 f13);

extern void func_L09_00323718(char *a0, char *a1, s32 a2, s32 a3, f32 f12);

/**
 * func_L09_00406458 - tick the state's two RNG cells, count the tick, and
 * feed both derived views back into the effect builders.
 *
 * The state is at *(char **)(a0+0x68). The RNG cells at state+0x330 and
 * state+0x332 are ticked through func_L09_002EAD98; the object's u16 counter
 * at +0x96 is incremented unless it already holds 0xFFFF; then the builder
 * pair runs: func_L09_003206A0(@a0, state+0x280, 0, 0.03f, 0.3f) and
 * func_L09_00323718(@a0, state+0x360, 0, 1, 1.5f).
 *
 * @a0: the object.
 * Return: none.
 *
 * Non-obvious: the counter saturates at 0xFFFF rather than wrapping. Twin of
 * func_L09_00404668 (which ticks the +0x26C pair). No codegen or addressing
 * device is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00404FC8 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_00404FC8 -> func_L09_00406458 (our split name);
 *   - func_2EA110 -> func_L09_002EAD98, func_31F500 -> func_L09_003206A0,
 *     func_322578 -> func_L09_00323718 (L09's own copies at the v2.00
 *     places);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_00406458(char *a0) {
    char *s1 = *(char **)(a0 + 0x68);
    u16 n;
    func_L09_002EAD98(s1 + 0x330);
    func_L09_002EAD98(s1 + 0x332);
    n = *(u16 *)(a0 + 0x96);
    if (n < 0xFFFF) {
        *(u16 *)(a0 + 0x96) = n + 1;
    }
    func_L09_003206A0(a0, s1 + 0x280, 0, 0.03f, 0.3f);
    func_L09_00323718(a0, s1 + 0x360, 0, 1, 1.5f);
}

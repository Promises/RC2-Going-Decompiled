/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x0038D8B8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef int s32;
extern s32 func_L13_0032B7C0(u8 *, s32);
extern s32 func_L13_00325BD0(u8 *, s32, s32);
extern void func_L13_003387A8(s32, s32);
extern char g_heroPos[];

/**
 * func_L13_0038D8B8 - toggle an object between two states as the hero
 * enters one of two volumes.
 *
 * @a0: the object; its state block is at +0x68.
 * Return: nothing.
 *
 * State 0 sets +0x30 to 0xFF and goes to state 1 through
 * func_L13_00325BD0(a0, 1, -1). In state 1, when func_L13_0032B7C0(g_heroPos,
 * id 0) reports non-zero, it calls func_L13_003387A8(word 4, word 6) and goes
 * to state 2; state 2 is the mirror with id 1, words 5 and 7, back to 1.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/13_boldan/L13_00306F38.c, function func_L13_0038C470 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L13_0038C470 -> func_L13_0038D8B8 (our split name at the v2.00 place);
 *   - D_00189EA0 -> g_heroPos (our boot name at that address);
 *   - func_3248E0 -> func_L13_00325BD0 (the v2.00 relocation target, our name there);
 *   - func_32A4D0 -> func_L13_0032B7C0 (the v2.00 relocation target, our name there);
 *   - func_337400 -> func_L13_003387A8 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L13_0038D8B8(u8 *a0) {
    s32 *ptr = *(s32 **)(a0 + 0x68);
    switch (a0[0x20]) {
    case 0:
        a0[0x30] = 0xFF;
        func_L13_00325BD0(a0, 1, -1);
        break;
    case 1:
        if (func_L13_0032B7C0(g_heroPos, ptr[0]) != 0) {
            func_L13_003387A8(ptr[4], ptr[6]);
            func_L13_00325BD0(a0, 2, -1);
        }
        break;
    case 2:
        if (func_L13_0032B7C0(g_heroPos, ptr[1]) != 0) {
            func_L13_003387A8(ptr[5], ptr[7]);
            func_L13_00325BD0(a0, 1, -1);
        }
        break;
    }
}

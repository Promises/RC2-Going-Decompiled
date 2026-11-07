/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x004461B0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_00330820(int, int, int);
extern void func_L20_00441AB0(void);  /* ADDED PROTOTYPE */
extern void func_L20_002FB898(void (*)(void), int);

/**
 * func_L20_004461B0 - move an object to state 0x12 and register
 * func_L20_00441AB0 as a callback for it through func_L20_002FB898.
 *
 * @arg0: the object, passed as an int.
 * Return: none.
 *
 * Non-obvious: func_L20_00330820(arg0, 0x12, -1) runs first.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0.c (blob c24c6906), function
 * func_L20_00444C68 (v1.01 address), author commit db93b56d (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, no device: the statements are verbatim but for names, and
 * one extern prototype is added for func_L20_00441AB0, whose definition
 * upstream has earlier in the same file (a one-function TU must declare it).
 * Respellings, each to the target of the v2.00 disc's own relocation at this
 * place (relverify by name on this object, task #2566):
 *   - func_L20_00444C68 -> func_L20_004461B0 (our split name at the v2.00
 *     place);
 *   - func_002A7750 -> func_L20_00330820 (the level's own copy at its L20
 *     place);
 *   - func_L00_002E6EF8 -> func_L20_002FB898 (the level's own copy at its L20
 *     place);
 *   - func_L20_00440570 -> func_L20_00441AB0 (its v2.00 split name);
 *   - `extern void func_L20_00441AB0(void);` added (marked ADDED PROTOTYPE) in
 *     place of upstream's in-file definition of func_L20_00440570;
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
void func_L20_004461B0(int arg0) {
    func_L20_00330820(arg0, 0x12, -1);
    func_L20_002FB898(func_L20_00441AB0, arg0);
}

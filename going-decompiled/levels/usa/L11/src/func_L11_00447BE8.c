/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x00447BE8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the boot's
 * ee-gcc 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;

/* The level's copy of the shared helper at 0x00333F00. */
extern int func_L11_00333F00(void *, int);

/**
 * func_L11_00447BE8 - pass the state's word at +0x1C4 to the shared helper
 * after the caller's argument.
 *
 * @object: the object; its state is at +0x68.
 * @arg: passed through as the helper's first argument.
 * Return: func_L11_00333F00(@arg, state+0x1C4 word).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_004461C0 (v1.01
 * address), author commit 6b943916 (Louis-Philippe Le Sieur; the name first
 * appears in the file at 264cb2be, llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_004461C0 -> func_L11_00447BE8 (our split name at the v2.00 place);
 *   - func_L00_003206B8 -> func_L11_00333F00 (upstream names the L00 copy;
 *     L11 calls its own);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the u8
 *     typedef common.h supplied is written here.
 */
int func_L11_00447BE8(u8 *object, u8 *arg) {
    u8 *state = *(u8 **)(object + 0x68);
    return func_L11_00333F00(arg, *(int *)(state + 0x1C4));
}

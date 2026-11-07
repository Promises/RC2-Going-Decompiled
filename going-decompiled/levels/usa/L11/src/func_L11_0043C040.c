/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x0043C040, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the boot's
 * ee-gcc 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/**
 * func_L11_0043C040 - is the object's state byte past the spawn states?
 *
 * @p: the object.
 * Return: 1 when the byte at +0x20 is 4 or more, else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_00432628.c, function func_L11_0043A800 (v1.01
 * address), author commit fa8d6672 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device):
 *   - func_L11_0043A800 -> func_L11_0043C040 (our split name at the v2.00 place);
 *   - `#include "common.h"` dropped (nothing in this function used it).
 */
int func_L11_0043C040(char *p)
{
    return *(unsigned char *)(p + 0x20) >= 4;
}

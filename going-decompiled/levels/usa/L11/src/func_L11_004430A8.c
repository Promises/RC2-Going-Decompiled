/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x004430A8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the boot's
 * ee-gcc 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef signed int s32;

/**
 * func_L11_004430A8 - read the state's group word at +0xC0.
 *
 * @obj: the object; its state is at +0x68.
 * Return: the word at state+0xC0 (0xFF means "no group").
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_00441858 (v1.01
 * address), author commit 431ac503 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device):
 *   - func_L11_00441858 -> func_L11_004430A8 (our split name at the v2.00 place);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the u8/s32
 *     typedefs common.h supplied are written here.
 */
s32 func_L11_004430A8(u8 *obj) {
    return *(s32 *)(*(u8 **)(obj + 0x68) + 0xC0);
}

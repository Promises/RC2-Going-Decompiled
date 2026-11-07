/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044A330, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef signed int s32;

/* The level's copies of two boot helpers at these v2.00 places, declared
   unprototyped as upstream does. */
extern void func_L11_0032DBF8();
extern void func_L11_0032D918();

/**
 * func_L11_0044A330 - put the object in mode 4 or 5 by whether its state
 * links an object.
 *
 * Linked (state+0x354 non-zero): func_L11_0032DBF8(a0, 4, 2) and, unless the
 * byte at +0x43 is already 0, func_L11_0032D918(a0, 0, 0, 10). Otherwise
 * func_L11_0032DBF8(a0, 5, 2) and, unless +0x43 is already 1,
 * func_L11_0032D918(a0, 1, 0, 10).
 *
 * @a0: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention; changed upstream since
 * b2a35115), file src/levels/11_joba/L11_004465F8.c, function
 * func_L11_00448908 (v1.01 address), author commit fa8d6672 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device); the respellings, each to the
 * target of the v2.00 disc's own relocation at this place (task #2555
 * relverify):
 *   - func_L11_00448908 -> func_L11_0044A330 (our split name at the v2.00 place);
 *   - func_32C9E0 -> func_L11_0032DBF8, func_32C700 -> func_L11_0032D918;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the u8/s32
 *     typedefs common.h supplied are written here.
 */
void func_L11_0044A330(char *a0)
{
    if (*(s32 *)(*(char **)(a0 + 0x68) + 0x354) != 0) {
        func_L11_0032DBF8(a0, 4, 2);
        if (*(u8 *)(a0 + 0x43) != 0)
            func_L11_0032D918(a0, 0, 0, 0xA);
    } else {
        func_L11_0032DBF8(a0, 5, 2);
        if (*(u8 *)(a0 + 0x43) != 1)
            func_L11_0032D918(a0, 1, 0, 0xA);
    }
}

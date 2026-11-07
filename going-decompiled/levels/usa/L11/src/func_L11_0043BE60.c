/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x0043BE60, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The level's copy of the effect spawner at 0x0032F698. */
extern void func_L11_0032F698(char *p, char *q, int a, float b, float c, float d, int e, int f, int g);

/**
 * func_L11_0043BE60 - bump the object's hit counter and spawn its effect.
 *
 * Increments the 16-bit counter at +0x96 unless it is already 0xFFFF, then
 * calls func_L11_0032F698 at the object's position (+0x10) with mask
 * 0x4000000 and scales 1.2, 1.0, 1.0.
 *
 * @p: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_00432628.c, function func_L11_0043A620 (v1.01
 * address), author commit fa8d6672 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_0043A620 -> func_L11_0043BE60 (our split name at the v2.00 place);
 *   - func_32E480 -> func_L11_0032F698;
 *   - `#include "common.h"` dropped (nothing in this function used it).
 */
void func_L11_0043BE60(char *p)
{
    unsigned short count = *(unsigned short *)(p + 0x96);

    if (count <= 0xFFFE)
        *(unsigned short *)(p + 0x96) = count + 1;
    func_L11_0032F698(p, p + 0x10, 0x4000000, 1.2f, 1.0f, 1.0f, 0, 1, 0);
}

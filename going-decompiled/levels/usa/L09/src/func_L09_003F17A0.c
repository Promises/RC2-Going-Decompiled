/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003F17A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The level's copies of boot helpers, named by their v2.00 places in L09. */
extern int func_L09_002EAD98(unsigned short *);
extern void func_L09_002EADC8(unsigned short *);

/**
 * func_L09_003F17A0 - count a hit and run the sub-object's two-call teardown.
 *
 * Increments the object's 16-bit counter (+0x96) while it is below 0xFFFF,
 * then calls func_L09_002EAD98 on the sub-object's halfword +0x92 and
 * func_L09_002EADC8 on its halfword +0x90.
 *
 * @a0: the object; its sub-object is at +0x68.
 *
 * Non-obvious: the guard is an unsigned `x < 0xFFFFu`, which cc1 prints as
 * `0xFFFE < x` with the branch inverted.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_002F8BA0.c, function func_L09_003F0330 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2561 relverify):
 *   - func_L09_003F0330 -> func_L09_003F17A0 (our split name at the v2.00 place);
 *   - func_00282978 -> func_L09_002EAD98, func_002829A8 -> func_L09_002EADC8;
 *   - `#include "common.h"` dropped: only the declarations this function uses
 *     are kept, and upstream's function comment is folded into this one.
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the whole upstream file fails on this arm; this
 * function's own text has no `long long`, so alone in its TU it needs no
 * respelling.
 */
void func_L09_003F17A0(unsigned char *a0) {
    unsigned char *s0 = *(unsigned char **)(a0 + 0x68);
    if (*(unsigned short *)(a0 + 0x96) < 0xFFFFu) {
        *(unsigned short *)(a0 + 0x96) = *(unsigned short *)(a0 + 0x96) + 1;
    }
    func_L09_002EAD98((unsigned short *)(s0 + 0x92));
    func_L09_002EADC8((unsigned short *)(s0 + 0x90));
}

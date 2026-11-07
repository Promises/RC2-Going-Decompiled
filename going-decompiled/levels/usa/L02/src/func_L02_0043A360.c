/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0043A360,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0043A360 - restarts the shared effect object the level keeps at
 * 0x1BE200: reads the effect block pointer from +0x198 of that block and runs
 * the five shared restart/reset entries on it twice, in retail's order.
 *
 * Return: nothing.
 *
 * Non-obvious: the pointer at +0x198 is re-read for every call, as retail
 * does; the block address is an address-named extern (RULING #13291 §2): the
 * level ELF values D_L02_001BE200 from its name; the target lies inside the
 * committed split label D_L02_001B3930.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_00438B28 (v1.01 address), author commit
 * 91e04732 (llesieur99). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it);
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_00438B28 -> func_L02_0043A360,
 *     func_L02_003DAE60 -> func_L02_003DC3C8,
 *     func_L02_003DB158 -> func_L02_003DC6C0,
 *     func_L02_003DAF90 -> func_L02_003DC4F8,
 *     func_L02_003DB030 -> func_L02_003DC598;
 *   - kept address-named (RULING #13291 §2; the target lies inside the
 *     larger split label D_L02_001B3930): D_L02_001BE200 keeps its spelling.
 */
extern char D_L02_001BE200[];
extern void func_L02_003DC3C8(void *, int);
extern void func_L02_003DC6C0(void *, int);
extern void func_L02_003DC4F8(void *);
extern void func_L02_003DC598(void *);

/* restarts the shared effect object the level keeps at 0x1BE200 */
void func_L02_0043A360(void) {
    char *p = D_L02_001BE200;

    func_L02_003DC3C8(*(void **)(p + 0x198), 0);
    func_L02_003DC3C8(*(void **)(p + 0x198), 1);
    func_L02_003DC6C0(*(void **)(p + 0x198), 5);
    func_L02_003DC6C0(*(void **)(p + 0x198), 6);
    func_L02_003DC4F8(*(void **)(p + 0x198));
    func_L02_003DC598(*(void **)(p + 0x198));
}

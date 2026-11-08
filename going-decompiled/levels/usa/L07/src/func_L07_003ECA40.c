/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x003ECA40, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L07_002E4B88(char *);

extern void func_L07_0031DBA0(char *, char *, int, int, float);

/* A level word inside the larger split label g_modelTexRegs (+0x180):
 * address-named (RULING #13291 section 2). */
extern int D_L07_001C9700;

/**
 * func_L07_003ECA40 - release the two state slots, step the counter, run the
 * optional pass and fire the object's effect.
 *
 * The two slots at +0x2D4 and +0x2D8 of the state block (the object's +0x68
 * pointer) are each passed to func_L07_002E4B88, and the object's u16 counter
 * at +0x96 is incremented unless it is already 0xFFFF. When the level word
 * D_L07_001C9700 is nonzero, func_L07_003ECB78 runs on the object. Finally
 * func_L07_0031DBA0 gets the object, the state's +0x230 block, 5, 1 and 2.5.
 *
 * @arg: the object.
 * Return: none.
 *
 * Non-obvious: func_L07_003ECB78 has no prototype in scope (implicitly
 * declared), as upstream has it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_00389EB8.c, function func_L07_003EB700
 * (v1.01 address), author commit 91e04732 (llesieur99), by blame -M -C of the
 * definition (all 14 lines; no rename-copy found).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2656 relverify by
 * name):
 *   - func_L07_003EB700 -> func_L07_003ECA40 (our split name);
 *   - func_00282948 -> func_L07_002E4B88, func_002B0990 -> func_L07_0031DBA0,
 *     func_L07_003EB838 -> func_L07_003ECB78 (L07's own copies at the v2.00
 *     places);
 *   - D_L07_001C9780 -> D_L07_001C9700: address-named (RULING #13291
 *     section 2); no boot dlabel, boot operand, symbol_addrs entry,
 *     L07_boot_names entry or L07 split label names that address (task
 *     #2656 grep);
 *   - `#include "common.h"`/`#include "include_asm.h"` and the file's other
 *     declarations dropped: this function uses no common.h typedef.
 */
void func_L07_003ECA40(char *arg)
{
    char *o = *(char **)(arg + 104);
    unsigned short v;

    func_L07_002E4B88(o + 0x2D4);
    func_L07_002E4B88(o + 0x2D8);
    v = *(unsigned short *)(arg + 0x96);
    if (v <= 0xFFFE)
        *(unsigned short *)(arg + 0x96) = v + 1;
    if (D_L07_001C9700 != 0)
        func_L07_003ECB78(arg);
    func_L07_0031DBA0(arg, o + 0x230, 5, 1, 2.5f);
}

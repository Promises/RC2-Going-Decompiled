/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x0044C948, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern char g_heroPos[];
extern char D_L06_001B1398[];
extern char D_L06_001395CF;
extern void func_L06_002E3FF0(char *);
extern int func_L06_003622D0(char *, int);
extern void func_L06_0035B8C8(char *, int, int);

/**
 * func_L06_0044C948 - one-shot trigger: when the hero reaches the moby's
 * target, print a level string and set a flag byte.
 * @arg0: the moby; idle when its state byte +0x20 is 0; the first word of its
 *        pvar block (+0x68) is a target handle (-1: none).
 *
 * Fires only while idle with a valid handle and when
 * func_L06_003622D0(g_heroPos, handle) is nonzero. It passes the level
 * string D_L06_001B1398 to func_L06_002E3FF0, sets the byte
 * D_L06_001395CF to 1 and calls func_L06_0035B8C8(@arg0, 1, -1).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0041DED8.c, function
 * func_L06_0044B350 (v1.01 address), author commit a2622a00 (Louis-Philippe
 * Le Sieur; `git blame -M -C` and `git log -S` agree), which replaced
 * llesieur99's nonmatching draft nonmatching/levels/func_L06_0044B350.c
 * (6aa8c748, 2026-10-12) and kept its lines: `func_2E3EF0(D_L06_001B13F0);`
 * and `D_L06_001395CF = 1;` survive (`blame -C -C -C10`; RULING #13427 rev
 * 2). Upstream's func_L06_0044B350 is token-identical but for names to
 * func_L06_0044B2D8, written in the same commit by the same author, so the
 * rename-copy rule (RULING #13427 rev 4) adds no other writer. Adopted with
 * the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (BYTE-IDENTICAL), each name to the target of the
 * v2.00 disc's own relocation at this place (task #2653 relverify by name);
 * edits:
 *   - func_L06_0044B350 -> func_L06_0044C948 (our split name at the v2.00
 *     place)
 *   - D_L06_001395CF kept address-named (RULING #13291 section 2: no boot
 *     dlabel, boot operand, symbol_addrs entry or L06_boot_names.txt entry
 *     names 0x001395CF; it lies inside g_quickSelectMap)
 *   - D_L06_00189EA0 -> g_heroPos (the boot name at 0x00189EA0)
 *   - D_L06_001B13F0 -> D_L06_001B1398 (our split .lit label at the v2.00
 *     target)
 *   - func_2E3EF0 -> func_L06_002E3FF0, func_35A550 -> func_L06_0035B8C8,
 *     func_360F58 -> func_L06_003622D0 (callees respelled to L06's own
 *     copies at their v2.00 places; level-copy callee names stay
 *     func_L06_<addr>)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
void func_L06_0044C948(char *arg0)
{
    int h;

    if (*(unsigned char *)(arg0 + 0x20) != 0)
        return;
    h = *(int *)(*(int *)(arg0 + 0x68));
    if (h == -1)
        return;
    if (func_L06_003622D0(g_heroPos, h) == 0)
        return;
    func_L06_002E3FF0(D_L06_001B1398);
    D_L06_001395CF = 1;
    func_L06_0035B8C8(arg0, 1, -1);
}

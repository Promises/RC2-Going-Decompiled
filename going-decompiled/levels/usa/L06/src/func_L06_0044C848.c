/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x0044C848, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern char g_heroPos[];
extern char D_L06_001B12D8[];
extern char D_L06_001B1318[];
/* The two flags the re-arm sets, at 0x001395B8. */
typedef struct {
    char pad_000[0x13];
    char flag_013;
    char flag_014;
} L06ArmFlags;

extern L06ArmFlags D_1395B8;
extern void func_L06_002E3FF0(char *);
extern int func_L06_003622D0(char *, int);
extern void func_L06_0035B8C8(char *, int, int);

/**
 * func_L06_0044C848 - one-shot trigger: when the hero reaches the moby's
 * target, print two level strings and set two flag bytes.
 * @arg0: the moby; idle when its state byte +0x20 is 0; the first word of its
 *        pvar block (+0x68) is a target handle (-1: none).
 *
 * Fires only while idle with a valid handle and when
 * func_L06_003622D0(g_heroPos, handle) is nonzero. It passes the level
 * strings D_L06_001B12D8 and D_L06_001B1318 to func_L06_002E3FF0, sets the
 * bytes D_1395B8+0x13 and +0x14 (0x001395CB, 0x001395CC, through upstream's
 * L06ArmFlags view) to 1 and calls func_L06_0035B8C8(@arg0, 1, -1).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0041DED8.c, function
 * func_L06_0044B250 (v1.01 address), author commit a2622a00 (Louis-Philippe
 * Le Sieur; `git blame -M -C` and `git log -S` agree), which replaced
 * llesieur99's nonmatching draft nonmatching/levels/func_L06_0044B250.c
 * (6aa8c748, 2026-10-12) and kept its lines: the two
 * `func_2E3EF0(D_L06_001B1330/1370);` calls survive (`blame -C -C -C10`;
 * RULING #13427 rev 2). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). VERBATIM but for names (BYTE-IDENTICAL),
 * each name to the target of the v2.00 disc's own relocation at this place
 * (task #2653 relverify by name); edits:
 *   - func_L06_0044B250 -> func_L06_0044C848 (our split name at the v2.00
 *     place)
 *   - D_L06_001395B8 -> D_1395B8 (the name L06_boot_names.txt and the boot
 *     asm operands give at exactly 0x001395B8; the A2 rule, as L02 did)
 *   - D_L06_00189EA0 -> g_heroPos (the boot name at 0x00189EA0)
 *   - D_L06_001B1330 -> D_L06_001B12D8, D_L06_001B1370 -> D_L06_001B1318
 *     (our split .lit labels at the v2.00 targets)
 *   - func_2E3EF0 -> func_L06_002E3FF0, func_35A550 -> func_L06_0035B8C8,
 *     func_360F58 -> func_L06_003622D0 (callees respelled to L06's own
 *     copies at their v2.00 places; level-copy callee names stay
 *     func_L06_<addr>)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
void func_L06_0044C848(char *arg0)
{
    int h;

    if (*(unsigned char *)(arg0 + 0x20) != 0)
        return;
    h = *(int *)(*(int *)(arg0 + 0x68));
    if (h == -1)
        return;
    if (func_L06_003622D0(g_heroPos, h) == 0)
        return;
    func_L06_002E3FF0(D_L06_001B12D8);
    func_L06_002E3FF0(D_L06_001B1318);
    D_1395B8.flag_013 = 1;
    D_1395B8.flag_014 = 1;
    func_L06_0035B8C8(arg0, 1, -1);
}

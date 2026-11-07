/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00456830, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L11's own function and its copies of boot code, at their v2.00 places. */
extern void func_L11_0032DBF8(void *a0, int a1, int a2);
extern void func_L11_0032D918(void *a0, int a1, int a2, int a3);
extern void func_L11_00325748(void *a0);
extern void func_L11_004431C8(char *p, int id);
extern void func_L11_00374BB0(int index);
extern void func_L11_00374F58(int index, int value, char *object, int kind);
extern void func_L11_0042E300(char *, int, int, int, int, int, int, int, float, float, int,
                         int, int, int);

/* The boot's 0x70-byte slot table (core, below 0x1A7C80). */
extern char g_listenerPosHistory[];

/**
 * func_L11_00456830 - release the actor's slot in the shared table and drop
 * it to the idle flag block.
 *
 * @p: the actor (type 0xDF7); its state block q is at +0x68.
 *
 * If the slot at q+0x1DC is not -1 and its g_listenerPosHistory entry (0x70
 * bytes) points back at p (+0x88) with a nonzero +0x74,
 * func_L11_00374BB0(slot) releases it; the slot becomes -1. A tracking actor
 * (q+0x1EC == 1) is untracked through func_L11_004431C8. If byte +0x31 is
 * set, it calls func_L11_0042E300(p + 0x10, ...) and
 * func_L11_00374F58(0, 0, p, 0xDF7). Then func_L11_0032D918(p, 0, 0, 3),
 * func_L11_00325748(p) and func_L11_0032DBF8(p, 1, 0) run, and the +0x34
 * flags, +0x98, +0x31, q+0x1EC and q+0x1C4 are reset.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00453358.c, function
 * func_L11_00454DD8 (v1.01 address), author commit daf0e4a1 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged (verbatim but for
 * names; no device); edits:
 *   - name func_L11_00454DD8 -> func_L11_00456830 (our split name at the
 *     v2.00 place);
 *   - D_00188660 -> g_listenerPosHistory (the boot's name, same address);
 *   - func_002E59F8 -> func_L11_00374BB0, func_L11_00441978 ->
 *     func_L11_004431C8, func_0032C700 -> func_L11_0042E300, func_002E5DA0 ->
 *     func_L11_00374F58, func_002A7470 -> func_L11_0032D918, func_002A05A8 ->
 *     func_L11_00325748, func_002A7750 -> func_L11_0032DBF8: the targets of
 *     the v2.00 ROM's own relocations (relverify, task #2553);
 *   - the audit's `long long` -> `long` respelling (RULING #13197 class,
 *     needed because cc1 s136 refuses the file's other `long long` uses) is
 *     NOT applied: this one-function TU carries no `long long`;
 *   - `#include "common.h"` dropped (no typedef of it is used here); the
 *     upstream one-line comment folded into this one; the file's other
 *     functions and their externs not carried.
 */
void func_L11_00456830(char *p)
{
    char *q = *(char **)(p + 0x68);
    int i = *(int *)(q + 0x1DC);
    char *entry;

    if (i != -1) {
        entry = g_listenerPosHistory + i * 0x70;
        if (*(char **)(entry + 0x88) == p && *(unsigned char *)(entry + 0x74) != 0)
            func_L11_00374BB0(i);
    }
    *(int *)(q + 0x1DC) = -1;
    if (*(short *)(q + 0x1EC) == 1) {
        *(short *)(q + 0x1EC) = 0;
        func_L11_004431C8(*(char **)(p + 0xB8), *(short *)(p + 0xAA));
    }
    if (*(unsigned char *)(p + 0x31) != 0) {
        func_L11_0042E300(p + 0x10, 0, 10, 30, 0, 10, 45, 1, 0.3f, 0.0f, 0, 0, 0, 0);
        func_L11_00374F58(0, 0, p, 0xDF7);
    }
    func_L11_0032D918(p, 0, 0, 3);
    func_L11_00325748(p);
    func_L11_0032DBF8(p, 1, 0);
    *(unsigned short *)(p + 0x34) = (*(unsigned short *)(p + 0x34) & 0xEFFF) | 0x47;
    *(int *)(p + 0x98) = 0;
    *(unsigned char *)(p + 0x31) = 0;
    *(short *)(q + 0x1EC) = 0;
    *(int *)(q + 0x1C4) = 0;
}

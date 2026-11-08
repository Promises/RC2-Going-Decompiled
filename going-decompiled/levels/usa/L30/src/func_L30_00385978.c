/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00385978, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* The hero's position (boot core, symbol_addrs). */
extern char g_heroPos[];
/* L30's copy of the boot's volume probe, at its v2.00 place. */
extern int func_L30_00327920(char *, int);

/**
 * func_L30_00385978 - pick the first of the actor's 27 volumes the hero is
 * in.
 * @a0: the actor; its state block is at +0x68 and opens with 27 volume
 *      handles (-1 = empty).
 *
 * The first live handle for which func_L30_00327920(g_heroPos, handle) is
 * non-zero is stored at state +0x140 and its index at +0x144.
 * Return: that handle, or -1 when the hero is in none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0032AAF0.c, function
 * func_L30_00384768 (v1.01 address), author commit 6b21de6b (Louis-Philippe
 * Le Sieur; `git blame -M -C` gives all 15 definition lines to it, `git log
 * -S` agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_00384768 -> func_L30_00385978 (our split name);
 *   - D_00189EA0 -> g_heroPos (the boot's symbol_addrs name at exactly
 *     0x00189EA0);
 *   - func_002ACBC8 -> func_L30_00327920 (L30's copy at its v2.00 place);
 *   - the file's `#include` lines and other declarations dropped; upstream's
 *     comment folded into this one.
 */
int func_L30_00385978(char *a0) {
    char *q = *(char **)(a0 + 0x68);
    int i;

    for (i = 0; i < 27; i++) {
        int h = *(int *)(q + i * 4);

        if (h != -1 && func_L30_00327920(g_heroPos, h) != 0) {
            *(int *)(q + 0x140) = *(int *)(q + i * 4);
            *(int *)(q + 0x144) = i;
            return *(int *)(q + i * 4);
        }
    }
    return -1;
}

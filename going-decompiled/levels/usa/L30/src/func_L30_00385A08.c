/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00385A08, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* L30's default text-box rectangle and per-volume message table, at their
 * v2.00 split labels. */
extern char D_L30_001AEC48[];
extern char D_L30_00295AF0[];
/* L30's functions at their v2.00 places. */
extern int func_L30_002FC000(int);
extern void func_L30_00314318(void *, unsigned int, int, int, int, int, int, int, int, int);
typedef struct {
    float x;
    float y;
    float z;
    float u;
    float v;
    float w;
} Vec6;
/* The boot's camera/control block (core, symbol_addrs g_soundBankHandlesBlk
 * at exactly 0x00189E20), seen through upstream's view of its mode halfword
 * (+0x1E0) and flag byte (+0x22BC). */
typedef struct {
    unsigned char pad[0x1E0];
    unsigned short mode;
    unsigned char pad2[0x1FB0 - 0x1E2];
    float f1FB0;
    float f1FB4;
    unsigned char pad3[0x22BC - 0x1FB8];
    unsigned char flag;
} D00189E20_View;
extern D00189E20_View g_soundBankHandlesBlk;

/**
 * func_L30_00385A08 - show the message of the actor's current volume on
 * screen.
 * @a0: the actor; its state block is at +0x68 (the chosen volume index at
 *      +0x144).
 *
 * Sets the control block's mode halfword (+0x1E0) to 5 and its flag byte
 * (+0x22BC) to 1, copies the six-float rectangle D_L30_001AEC48 to a local,
 * looks up the volume's message D_L30_00295AF0[index] through
 * func_L30_002FC000 and shows it with func_L30_00314318 in colour 0x80F0F0F0.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/us2/levels/30_insomniac_museum/L30_00385A08.c, function
 * func_US2_L30_00385A08 (v2.00 address), author commit cde7e4dd (Flavius;
 * `git blame -M -C` gives all 11 definition lines to it, `git log -S`
 * agrees), a copy of llesieur99's func_L30_003847F8 (the same function at
 * its v1.01 place) in src/levels/30_insomniac_museum (author commit
 * 431ac503, earlier): token-identical, data names included (RULING #13427
 * rev 4, validator #2680). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_US2_L30_00385A08 -> func_L30_00385A08 (our split name, same
 *     address);
 *   - D_00189E20 -> g_soundBankHandlesBlk (the boot's symbol_addrs name at
 *     exactly 0x00189E20; upstream's view type kept);
 *   - D_001AEC98 -> D_L30_001AEC48, D_00295BF0 -> D_L30_00295AF0 (our split
 *     labels at the v2.00 targets);
 *   - func_2FB3C8 -> func_L30_002FC000, func_3132D8 -> func_L30_00314318
 *     (L30's functions at their v2.00 places);
 *   - `#include "include_asm.h"` dropped.
 */
void func_L30_00385A08(char *a0) {
    char *st = *(char **)(a0 + 0x68);
    Vec6 buf;
    int v;

    g_soundBankHandlesBlk.mode = 5;
    g_soundBankHandlesBlk.flag = 1;
    buf = *(Vec6 *)D_L30_001AEC48;
    v = func_L30_002FC000(*(int *)(D_L30_00295AF0 + *(int *)(st + 0x144) * 4));
    func_L30_00314318(&buf, 0x80F0F0F0, v, 1, 0, -1, 0, 0, 0, 0);
}

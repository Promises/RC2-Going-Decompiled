/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003C7400, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern char g_soundBankHandlesBlk[];
extern char D_L17_001FE880[];
extern char D_L17_00297070[];
extern void func_L17_002F0178(void *, void *, void *);
extern void func_L17_002EFC98(void *, void *, void *);

/**
 * func_L17_003C7400 - transform the seven points of the record named by
 * the state's id at +0x128 into the object's frame.
 *
 * @a2: the object; its state block is at +0x68.
 * Return: nothing.
 *
 * The same steps as func_L17_003C47D0, with the id read from the state's
 * +0x128 and the source points from D_L17_00297070: an id of -1 or a record
 * (D_L17_001FE880[id]) whose first word is not 7 returns at once; each of
 * the seven points goes through func_L17_002F0178 with the object's matrix
 * (+0xC0), then func_L17_002EFC98 with its position (+0x10); the id's low
 * halfword goes to the halfword +0x24FE of g_soundBankHandlesBlk.
 *
 * Non-obvious: the same int/pointer mix as its twin, with the same cc1
 * warning; the text is kept as written.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_0032C070.c, function func_L17_003C5E78 (v1.01 address), author commit 91e04732 (llesieur99); 5030340d (Flavius) later changed only one
 * callee's name on one line of it, so it is not credited.
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_003C5E78 -> func_L17_003C7400 (our split name at the v2.00 place);
 *   - D_00189E20 -> g_soundBankHandlesBlk (our boot name at that address);
 *   - D_L17_00296FF0 -> D_L17_00297070 (our split label at the v2.00 address);
 *   - func_00283098 -> func_L17_002F0178 (the v2.00 relocation target, our name there);
 *   - func_2EEEB8 -> func_L17_002EFC98 (the v2.00 relocation target, our name there);
 *   - D_L17_001FE880 kept: upstream's name already states the v2.00 address
 *   (0x001FE880, inside D_L17_001F0000 (+0xE880)); no name exists at that exact address
 *   (same grep); address-named (RULING #13291 section 2);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L17_003C7400(char *a2)
{
    char *s5;
    char *s0;
    char *s2;
    char *s4;
    char *s3;
    char *base;
    int v1;
    int i;

    s5 = *(char **)(a2 + 0x68);
    v1 = *(int *)(s5 + 0x128);
    if (v1 == -1)
        return;
    v1 = *(int *)(D_L17_001FE880 + v1 * 4);
    if (*(int *)v1 != 7)
        return;
    s3 = a2 + 0xC0;
    s4 = a2 + 0x10;
    s2 = D_L17_00297070;
    s0 = v1 + 0x10;
    for (i = 6; i >= 0; i--) {
        func_L17_002F0178(s0, s2, s3);
        s2 += 0x10;
        func_L17_002EFC98(s0, s0, s4);
        s0 += 0x10;
    }
    base = g_soundBankHandlesBlk;
    *(unsigned short *)(base + 0x24FE) = *(unsigned short *)(s5 + 0x128);
}

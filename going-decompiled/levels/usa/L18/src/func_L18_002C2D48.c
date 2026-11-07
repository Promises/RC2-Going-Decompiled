/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x002C2D48, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef short s16;
extern char g_soundBankHandlesBlk[];
extern u8 g_itemEquippedSlot[];
extern char D_L18_0026BC70[];
extern int func_L18_002CB240(int);
typedef struct {
    u8 pad[0x28];
    int v;
} WeaponEntry;

/**
 * func_L18_002C2D48 - pick the screen id for the player's current mode.
 *
 * @a0: a gate; when it is non-null the equipped item's word may answer.
 * Return: 0xD when the mode byte (block +0x22B4) is 4; else, when @a0 is
 * non-null, the gate byte +0x22BA is set and the halfword +0x24EC is clear,
 * the word at +0x28 of the 224-byte entry of D_L18_0026BC70 for the item in
 * g_itemEquippedSlot[func_L18_002CB240(0)]; else 0x51 when the word +0x24CC
 * is 1, else 0x67 when the byte +0x1492 is set, else 0.
 *
 * Non-obvious: the block is the boot's large record named
 * g_soundBankHandlesBlk at its start; the offsets read are well past the
 * sound-bank handles. @a0 is reused to hold the block on the fallback path.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_002C2D48.c,
 * function func_L18_002C2D48 (v1.01 address), author commit 7b6cc769 (agent-lvl7-08).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_002C2D48 -> func_L18_002C2D48 (our split name at the v2.00 place);
 *   - D_00139568 -> g_itemEquippedSlot (our boot name at that address);
 *   - D_00189E20 -> g_soundBankHandlesBlk (our boot name at that address);
 *   - D_0026BCF0 -> D_L18_0026BC70 (our split label at the v2.00 address);
 *   - func_002CB1C8 -> func_L18_002CB240 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them);
 *   - upstream's function comment folded into this one.
 */
int func_L18_002C2D48(char *a0)
{
    char *s0 = g_soundBankHandlesBlk;

    if (*(u8 *)(s0 + 0x22B4) == 4)
        return 0xD;
    if (a0 != 0 && *(u8 *)(s0 + 0x22BA) != 0 && *(s16 *)(s0 + 0x24EC) == 0) {
        int idx = func_L18_002CB240(0);
        return ((WeaponEntry *)(D_L18_0026BC70 + g_itemEquippedSlot[idx] * 224))->v;
    }
    a0 = g_soundBankHandlesBlk;
    if (*(int *)(a0 + 0x24CC) != 1)
        return *(u8 *)(a0 + 0x1492) ? 0x67 : 0;
    return 0x51;
}

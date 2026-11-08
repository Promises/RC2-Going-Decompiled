/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003F4BD8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern int func_L06_0035DF18(char *a0, int a1, int a2);
extern float func_L06_0035DF78(char *a0, int a1);
extern void func_L06_00360BA0(char *a0, char *a1);
extern void func_L06_0035ADB8(char *a0, int a1, int a2);
extern void func_L06_00360C60(char *a0, char *a1);

/**
 * func_L06_003F4BD8 - apply this frame's hit amount to an object's
 * health and play the matching reaction.
 * @a0: the moby.
 * @a1: its state block (health float at +0x20, reaction record at +0x50).
 *
 * t = func_L06_0035DF78(@a0, func_L06_0035DF18(@a0, 0xC30000, 0)). When t > 0
 * it is subtracted from the health; while health stays above 0 the reaction
 * byte +0x57 is set to 0x5A, otherwise func_L06_0035ADB8(@a0, 0, 3) runs, the
 * moby state byte +0x20 becomes 2 and the reaction byte 0x78; either way
 * func_L06_00360BA0(@a0, @a1 + 0x50) plays it. Always ends by setting @a0's
 * +0xA8 to 0xFF and calling func_L06_00360C60(@a0, @a1 + 0x50).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0033BDC0.c, function
 * func_L06_003F3690 (v1.01 address), author commit 52e801be (llesieur99;
 * `git blame -M -C` and `git log -S` agree. The token-identical PAL copy
 * func_PAL_L06_003F6F58 is later, Flavius 7a2f01ec 2026-10-21, so it does
 * not change the credit). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). VERBATIM but for names (BYTE-IDENTICAL),
 * each name to the target of the v2.00 disc's own relocation at this place
 * (task #2653 relverify by name); edits:
 *   - func_L06_003F3690 -> func_L06_003F4BD8 (our split name at the v2.00
 *     place)
 *   - func_002A6C58 -> func_L06_0035ADB8, func_002A9640 ->
 *     func_L06_0035DF18, func_002A96A0 -> func_L06_0035DF78, func_002AB800
 *     -> func_L06_00360BA0, func_002AB8C0 -> func_L06_00360C60 (callees
 *     respelled to L06's own copies at their v2.00 places; level-copy
 *     callee names stay func_L06_<addr>)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
void func_L06_003F4BD8(char *a0, char *a1)
{
    float t = func_L06_0035DF78(a0, func_L06_0035DF18(a0, 0xC30000, 0));

    if (t > 0.0f) {
        float left = *(float *)(a1 + 0x20) - t;

        *(float *)(a1 + 0x20) = left;
        if (left > 0.0f) {
            *(unsigned char *)(a1 + 0x57) = 0x5A;
            func_L06_00360BA0(a0, a1 + 0x50);
        } else {
            func_L06_0035ADB8(a0, 0, 3);
            *(unsigned char *)(a0 + 0x20) = 2;
            *(unsigned char *)(a1 + 0x57) = 0x78;
            func_L06_00360BA0(a0, a1 + 0x50);
        }
    }
    *(unsigned char *)(a0 + 0xA8) = 0xFF;
    func_L06_00360C60(a0, a1 + 0x50);
}

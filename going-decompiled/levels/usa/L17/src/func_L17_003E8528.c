/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003E8528, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm ee-gcc 2.9-ee-991111b
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

typedef signed int s32;

extern s32 g_mobyTableBase __attribute__((section(".sdata")));

/**
 * func_L17_003E8528 - give up the leader slot func_L17_003E83F0 elects.
 *
 * @a0: a moby.
 * Return: nothing.
 *
 * When the shared leader index (state +0x720 points to it) equals this
 * moby's index ((a0 - g_mobyTableBase) >> 8), it sets the index to -1 and
 * the shared phase word (state +0x748 points to it) to 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_003C7C70.c, function func_L17_003E6F20 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_L17_003E6F20 -> func_L17_003E8528 (our split name at the v2.00 place);
 *   - D_L17_001B2C1C -> g_mobyTableBase: the name symbol_addrs/L17_boot_names.txt gives at
 *   exactly 0x001B2C1C, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *   g_mobyTableBase declaration (RULING #8620 ADDRESSING-MODEL DEVICE, EE arm
 *   only; this TU has no other arm): our cc1 then prints the one-instruction
 *   load macro, which GNU as expands at -G0 to the adjacent absolute lui/lw
 *   pair retail has (cc1 schedules it as one instruction). Ablation (task
 *   #2667): with upstream's `sda` spelling 7 of 14 masked words
 *   differ. It moves no data and emits
 *   nothing;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L17_003E8528(s32 a0)
{
    s32 obj = *(s32 *)(a0 + 0x68);
    s32 *p = *(s32 **)(obj + 0x720);

    if (*p == (a0 - g_mobyTableBase) >> 8) {
        *p = -1;
        *(s32 *)(*(s32 *)(obj + 0x748)) = 0;
    }
}

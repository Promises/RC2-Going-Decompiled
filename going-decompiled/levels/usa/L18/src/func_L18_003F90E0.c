/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003F90E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef signed int s32;

extern void func_L18_003F9148(s32);
extern void func_L18_003F9368(s32);
extern void func_L18_003F9510(s32);
extern void func_L18_003F9F08(s32);
extern void func_L18_003FA078(s32);

/**
 * func_L18_003F90E0 - per-frame dispatcher of one Damosel object.
 *
 * @a0: the object (as an int, upstream's spelling).
 *
 * State byte (+0x20) 0 runs the set-up func_L18_003F9148 and returns; state
 * 1 does nothing; any other state runs the four updates func_L18_003F9368,
 * func_L18_003F9510, func_L18_003F9F08 and func_L18_003FA078 in that order.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_002E68B0.c,
 * function func_L18_003F7C70 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_003F7C70 -> func_L18_003F90E0 (our split name at the v2.00 place);
 *   - func_3F7CD8 -> func_L18_003F9148 (the v2.00 relocation target, our name there);
 *   - func_3F7EF8 -> func_L18_003F9368 (the v2.00 relocation target, our name there);
 *   - func_3F80A0 -> func_L18_003F9510 (the v2.00 relocation target, our name there);
 *   - func_3F8A98 -> func_L18_003F9F08 (the v2.00 relocation target, our name there);
 *   - func_3F8C08 -> func_L18_003FA078 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them).
 */
void func_L18_003F90E0(s32 a0)
{
    u8 v = *(u8 *)(a0 + 0x20);

    if (v == 0) {
        func_L18_003F9148(a0);
        return;
    }
    if (v == 1)
        return;
    func_L18_003F9368(a0);
    func_L18_003F9510(a0);
    func_L18_003F9F08(a0);
    func_L18_003FA078(a0);
}

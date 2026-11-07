/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x0043B4B8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef signed int s32;

extern float func_L18_0030A650(float, float);
extern int D_L18_001C9CC0;
extern s32 func_L18_0037C590(void *, s32);
extern s32 func_L18_0037C8D8(s32, s32, void *);
extern void *func_L18_00338E08(void *, s32, s32);
extern float func_L18_00338E68(void *, void *);
extern void func_L18_0033B248(void *, void *);
extern void func_L18_0033B308(void *, void *);
extern void func_L18_003368C0(void *, int, int);
extern int func_L18_0037C590(void *,int),func_L18_0037C8D8(int,int,void *);

/**
 * func_L18_0043B4B8 - spinning damageable Damosel object.
 *
 * @obj: the object; its state block is at +0x68.
 *
 * Does nothing while D_L18_001C9CC0 is set. State 0 sets the halfword +0x24
 * to 18 and the health (+0x20) to 18.0, then state 1. State 1 keeps a sound
 * running (func_L18_0037C590 / func_L18_0037C8D8 on the handle +0x68),
 * turns +0xF8 by rate (+0x64) * 0.000290888 (one arc-minute in radians)
 * through func_L18_0030A650, takes the damage of any hit of class mask
 * 0x230000 (func_L18_00338E08 / func_L18_00338E68) off the health with a
 * red flash colour (0xFF, 0x40, 0) passed to func_L18_0033B248, then calls
 * func_L18_0033B308 every pass and sets byte +0xA8 to 0xFF.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_00433460.c,
 * function func_L18_00439FC8 (v1.01 address), author commit 11a4bac2
 * (Jacob), which closed llesieur99's near-miss draft from ee3fee45 (its lines
 * survive up to the f > 0 block; Jacob moved the func_002AB8C0 call after it
 * and wrote the tail) (RULING #13427 rev 2).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_00439FC8 -> func_L18_0043B4B8 (our split name at the v2.00 place);
 *   - D_L18_001C9D40 -> D_L18_001C9CC0: an address-named extern (RULING #13291 section 2),
 *   inside the larger split label D_L18_001B2B30 (+0x17190); no name exists at that exact address
 *   in the boot splits, symbol_addrs.txt, L18_boot_names.txt or the level's
 *   split labels (grep with firing controls, task #2662);
 *   - func_00283B98 -> func_L18_0030A650 (the v2.00 relocation target, our name there);
 *   - func_002A7750 -> func_L18_003368C0 (the v2.00 relocation target, our name there);
 *   - func_002A9640 -> func_L18_00338E08 (the v2.00 relocation target, our name there);
 *   - func_002A96A0 -> func_L18_00338E68 (the v2.00 relocation target, our name there);
 *   - func_002AB800 -> func_L18_0033B248 (the v2.00 relocation target, our name there);
 *   - func_002AB8C0 -> func_L18_0033B308 (the v2.00 relocation target, our name there);
 *   - func_002E59B0 -> func_L18_0037C590 (the v2.00 relocation target, our name there);
 *   - func_002E5CF8 -> func_L18_0037C8D8 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them), and 8 repeated identical extern line(s) of the upstream file collapsed to one.
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the upstream file as a whole fails on this arm;
 * this function's own text has no `long long` (nor s64/u64), so alone in its
 * TU it needs no respelling, and none is made.
 */
void func_L18_0043B4B8(u8 *obj) {
    u8 *state;
    u8 *p;
    float f;

    if (D_L18_001C9CC0 != 0) return;
    state = *(u8 **)(obj + 0x68);
    switch (*(u8 *)(obj + 0x20)) {
    case 0:
        *(short *)(state + 0x24) = 0x12;
        *(float *)(state + 0x20) = 18.0f;
        func_L18_003368C0(obj, 1, -1);
        return;
    case 1:
        if (func_L18_0037C590(obj, *(s32 *)(state + 0x68)) == 0) {
            *(s32 *)(state + 0x68) = func_L18_0037C8D8(0, 4, obj);
        }
        p = state + 0x50;
        *(float *)(obj + 0xF8) = func_L18_0030A650(*(float *)(obj + 0xF8),
                                               *(float *)(state + 0x64) * 0.000290888245f);
        f = func_L18_00338E68(obj, func_L18_00338E08(obj, 0x230000, 0));
        if (f > 0.0f) {
            *(u8 *)(state + 0x57) = 0xFF;
            *(u8 *)(state + 0x58) = 0x40;
            *(u8 *)(state + 0x59) = 0;
            func_L18_0033B248(obj, p);
            *(float *)(state + 0x20) = *(float *)(state + 0x20) - f;
        }
        func_L18_0033B308(obj, p);
        *(u8 *)(obj + 0xA8) = 0xFF;
        return;
    }
}

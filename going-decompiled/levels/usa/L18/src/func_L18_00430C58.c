/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00430C58, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;
extern void func_L18_003368C0(u8 *arg0, s32 arg1, s32 arg2);

/**
 * func_L18_00430C58 - in state 1, arm the object and move it to state 3.
 *
 * @a0: the object; its class record is at +0x24.
 *
 * Only when the state byte (+0x20) is 1: clears bits 0x41 of the flags
 * halfword (+0x34) and sets 0x1000, copies the class record's word 4 to
 * +0x98, and calls func_L18_003368C0(a0, 3, -1).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_003F80A0.c,
 * function func_L18_0042F768 (v1.01 address), author commit fa8d6672 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_0042F768 -> func_L18_00430C58 (our split name at the v2.00 place);
 *   - func_002A7750 -> func_L18_003368C0 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them).
 */
void func_L18_00430C58(u8 *a0) {
    s32 *p;
    if (*(u8 *)(a0 + 0x20) != 1) return;
    p = *(s32 **)(a0 + 0x24);
    *(u16 *)(a0 + 0x34) = (*(u16 *)(a0 + 0x34) & 0xFFBE) | 0x1000;
    *(s32 *)(a0 + 0x98) = p[4];
    func_L18_003368C0(a0, 3, -1);
}

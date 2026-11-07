/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00435618, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef signed int s32;

extern float func_L18_0030A650(float, float);
extern float func_L18_00309C38(float);
extern float func_L18_00309C50(float);

/**
 * func_L18_00435618 - advance an object's two pairs of oscillators.
 *
 * @obj: the object; its state block is at +0x68.
 *
 * For each of two groups (0xA0 bytes apart from state +0x610) and each of
 * its two 0x10-byte slots, advances the three phases (+0x40/+0x44/+0x48) by
 * their rates (+0x60/+0x64/+0x68) through func_L18_0030A650, then sets
 * +0x20 = func_L18_00309C38(phase0) * amp0, +0x24 =
 * func_L18_00309C50(phase1) * amp1 and +0x28 = -func_L18_00309C50(phase2)
 * * amp2 (amplitudes +0x80/+0x84/+0x88).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_00433460.c,
 * function func_L18_00434128 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_00434128 -> func_L18_00435618 (our split name at the v2.00 place);
 *   - func_00283180 -> func_L18_00309C38 (the v2.00 relocation target, our name there);
 *   - func_00283198 -> func_L18_00309C50 (the v2.00 relocation target, our name there);
 *   - func_00283B98 -> func_L18_0030A650 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them), and 1 repeated identical extern line(s) of the upstream file collapsed to one.
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the upstream file as a whole fails on this arm;
 * this function's own text has no `long long` (nor s64/u64), so alone in its
 * TU it needs no respelling, and none is made.
 */
void func_L18_00435618(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 i;
    for (i = 0; i < 2; i++) {
        u8 *p = &state[i * 0xA0 + 0x610];
        s32 j;
        for (j = 1; j >= 0; j--) {
            float x;
            *(float *)(p + 0x40) = func_L18_0030A650(*(float *)(p + 0x40), *(float *)(p + 0x60));
            *(float *)(p + 0x44) = func_L18_0030A650(*(float *)(p + 0x44), *(float *)(p + 0x64));
            *(float *)(p + 0x48) = func_L18_0030A650(*(float *)(p + 0x48), *(float *)(p + 0x68));
            x = func_L18_00309C38(*(float *)(p + 0x40));
            *(float *)(p + 0x20) = x * *(float *)(p + 0x80);
            *(float *)(p + 0x24) = func_L18_00309C50(*(float *)(p + 0x44)) * *(float *)(p + 0x84);
            *(float *)(p + 0x28) = -func_L18_00309C50(*(float *)(p + 0x48)) * *(float *)(p + 0x88);
            p += 0x10;
        }
    }
}

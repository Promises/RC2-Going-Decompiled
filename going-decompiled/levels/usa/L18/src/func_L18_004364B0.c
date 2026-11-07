/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x004364B0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;

extern float g_cameraPos[];
extern float func_L18_00336A28(float, float);
extern void func_L18_0032D808(void *, int, void *);
extern void func_L18_00309670(float *, const float *, const float *);
extern void func_L18_00309928(char *, char *, float);
extern void func_L18_00309640(char *, char *, char *);
extern int func_L18_003369D0(int, int);
extern float func_L18_00336AF0(void);
extern void func_L18_00436348(void *, int, float, int, float, int, int);

/**
 * func_L18_004364B0 - spawn two glow sprites in front of an object, toward
 * the camera.
 *
 * @obj: the object; func_L18_0032D808(obj, 0, v) gives its point.
 *
 * Moves the point 0.2 of the way along the flattened (z = 0) vector to
 * g_cameraPos, then calls func_L18_00436348 twice there: colour 0x807070,
 * size half of func_L18_00336A28(0.5, 0.8), and colour 0x802020, size 0.56,
 * each with a random value from func_L18_003369D0 and func_L18_00336AF0.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_00433460.c,
 * function func_L18_00434FC0 (v1.01 address), author commit da4beea9
 * (bigbeno37), which closed llesieur99's near-miss draft from ee3fee45; the
 * draft's lines survive except the two closing calls, which da4beea9 wrote
 * (RULING #13427 rev 2).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_00434FC0 -> func_L18_004364B0 (our split name at the v2.00 place);
 *   - D_L18_001B97C0 -> g_cameraPos: the name symbol_addrs/L18_boot_names.txt gives at
 *   exactly 0x001B9740 (relverify: inside D_L18_001B2B30 +0x6C10); NAMES rule,
 *   task #2646 class A2;
 *   - func_00282CC0 -> func_L18_00309640 (the v2.00 relocation target, our name there);
 *   - func_00282CF0 -> func_L18_00309670 (the v2.00 relocation target, our name there);
 *   - func_00282F20 -> func_L18_00309928 (the v2.00 relocation target, our name there);
 *   - func_0029FCC0 -> func_L18_0032D808 (the v2.00 relocation target, our name there);
 *   - func_002A7820 -> func_L18_003369D0 (the v2.00 relocation target, our name there);
 *   - func_002A7878 -> func_L18_00336A28 (the v2.00 relocation target, our name there);
 *   - func_002A7940 -> func_L18_00336AF0 (the v2.00 relocation target, our name there);
 *   - func_L02_003FFC48 -> func_L18_00436348 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them), and 4 repeated identical extern line(s) of the upstream file collapsed to one.
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the upstream file as a whole fails on this arm;
 * this function's own text has no `long long` (nor s64/u64), so alone in its
 * TU it needs no respelling, and none is made.
 */
void func_L18_004364B0(u8 *obj) {
    float v[4];
    float w[4];
    float a;

    a = func_L18_00336A28(0.5f, 0.8f);
    func_L18_0032D808(obj, 0, v);
    a = a * 0.5f;
    func_L18_00309670(w, g_cameraPos, v);
    w[2] = 0.0f;
    func_L18_00309928((char *)w, (char *)w, 0.2f);
    func_L18_00309640((char *)v, (char *)v, (char *)w);
    func_L18_00436348(v, 0x807070, a, func_L18_003369D0(0x50, 0x60), func_L18_00336AF0(), 0x31, 2);
    func_L18_00436348(v, 0x802020, 0.56f, func_L18_003369D0(0x16, 0x32), func_L18_00336AF0(), 0xB, 2);
}

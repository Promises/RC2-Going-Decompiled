/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003FF0E8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The level's copies of boot helpers, named by their v2.00 places in L09. */
extern float func_L09_0031B048(unsigned char *, unsigned char *, float, float, float, float);
extern int func_L09_0033FDA8(char *, char *, char *, char *, int, float);
/* A table of record pointers at 0x001FE640, inside our split label
   D_L09_001F0000: an address-named extern (RULING #13291 §2), valued from its
   address by level_elf. */
extern unsigned int D_L09_001FE640[];

/**
 * func_L09_003FF0E8 - sample the sub-object through the shared record
 * starter and ramp the object's three colour channels toward the result.
 *
 * Calls func_L09_0033FDA8 on the record D_L09_001FE640[state+0x318], which
 * writes three floats into a local buffer and may update the state's +0x324
 * word and +0x3EC float. Each float is then ramped into the channels at
 * +0x390/+0x394/+0x398 (velocity at +0x3A0/+0x3A4/+0x3A8) at 0.02 per frame
 * through func_L09_0031B048. When the state's halfword +0x32A is clear and
 * the starter reported a live record, the saved +0x324 word and +0x3EC float
 * are put back and the +0x388 angle is negated.
 *
 * @a0: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_002F8BA0.c, function func_L09_003FDC58 (v1.01
 * address), author commit ee3fee45 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED (names only,
 * no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2561 relverify):
 *   - func_L09_003FDC58 -> func_L09_003FF0E8 (our split name at the v2.00 place);
 *   - D_L09_001FE6C0 -> D_L09_001FE640: an address-named extern inside
 *     D_L09_001F0000 (RULING #13291 §2);
 *   - func_002AA3A8 -> func_L09_0031B048, func_L00_00341820 -> func_L09_0033FDA8;
 *   - `#include "common.h"` dropped: only the declarations this function uses
 *     are kept, and upstream's function comment is folded into this one.
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the whole upstream file fails on this arm; this
 * function's own text has no `long long`, so alone in its TU it needs no
 * respelling.
 */
void func_L09_003FF0E8(unsigned char *a0) {
    unsigned char *s0 = *(unsigned char **)(a0 + 0x68);
    float buf[4];
    float step = 0.02f;
    float keep = *(float *)(s0 + 0x3EC);
    int save = *(int *)(s0 + 0x324);
    int live = func_L09_0033FDA8(
        (char *)D_L09_001FE640[*(int *)(s0 + 0x318)], (char *)buf,
        (char *)(s0 + 0x324), (char *)(s0 + 0x3EC), *(short *)(s0 + 0x32A),
        *(float *)(s0 + 0x388));
    func_L09_0031B048((char *)(s0 + 0x390), (char *)(s0 + 0x3A0), buf[0], step, 0.2f,
                  0.0f);
    func_L09_0031B048((char *)(s0 + 0x394), (char *)(s0 + 0x3A4), buf[1], step, 0.2f,
                  0.0f);
    func_L09_0031B048((char *)(s0 + 0x398), (char *)(s0 + 0x3A8), buf[2], step, 0.2f,
                  0.0f);
    if (*(short *)(s0 + 0x32A) == 0 && live != 0) {
        *(float *)(s0 + 0x3EC) = keep;
        *(int *)(s0 + 0x324) = save;
        *(float *)(s0 + 0x388) = -*(float *)(s0 + 0x388);
    }
}

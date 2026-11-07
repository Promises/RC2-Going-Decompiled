/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003CA110, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* EE `long` is 64-bit; RULING #13197 spells it s64 in adopted bodies. */
typedef long s64;

/* The level's copies of boot helpers, named by their v2.00 places in L09. */
extern float func_L09_002EC100(float, float);
extern void func_L09_00320B58();
extern void func_L09_00321068(unsigned char *, unsigned char *);

/**
 * func_L09_003CA110 - two-state gate on the record's state byte (+0x20).
 *
 * State 0: as long as the 64-bit word the sub-object opens with is not -1,
 * the record is stamped 0xFF (+0x30), the shared eight-argument starter
 * func_L09_00320B58 is run on the sub-object's track block (+0x30), the
 * sub-object's byte +0xC5A is set and the record flips to state 1.
 * State 1: the sub-object's first float (+8) is scaled by 2*pi/21600 (a
 * per-frame 1/100 s turn rate) and folded into the record's angle (+0xF0)
 * through func_L09_002EC100, then the track block is stepped again by
 * func_L09_00321068. Any other state does nothing.
 *
 * @a0: the record; its sub-object is at +0x68.
 *
 * Non-obvious: the -1 test reads the sub-object's first EIGHT bytes as one
 * 64-bit word (an `ld`), not its first int.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_002F8BA0.c, function func_L09_003C8CD8 (v1.01
 * address), author commit 431ac503 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED (one type
 * respelling, no other device); the edits, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2561 relverify):
 *   - func_L09_003C8CD8 -> func_L09_003CA110 (our split name at the v2.00 place);
 *   - func_00283B98 -> func_L09_002EC100, func_L00_00323448 -> func_L09_00320B58,
 *     func_L00_00323958 -> func_L09_00321068;
 *   - `*(long long *)s1` is written `*(s64 *)s1`, with `typedef long s64` here
 *     (RULING #13197; NOTE #13281's `long long`->`long` device): on this
 *     arm the upstream spelling fails in cc1 with "unsupported wide integer
 *     operation" (measured on this TU, task #2561), while EE `long` is the
 *     64-bit word the ROM's `ld` reads;
 *   - `#include "common.h"` dropped: only the declarations this function uses
 *     are kept, and upstream's function comment is folded into this one.
 */
void func_L09_003CA110(unsigned char *a0) {
    unsigned char *s0 = a0;
    unsigned char *s1 = *(unsigned char **)(s0 + 0x68);
    switch (s0[0x20]) {
    case 0:
        if (*(s64 *)s1 == -1) {
            break;
        }
        {
            unsigned char stamp = 0xFF;
            *(unsigned char *)(s0 + 0x30) = stamp;
        }
        func_L09_00320B58(s0, s1 + 0x30, 0x64, *(int *)(s1 + 4), *(int *)s1,
                          -1, 7, 0);
        s1[0xC5A] = 1;
        s0[0x20] = 1;
        break;
    case 1:
        *(float *)(s0 + 0xF0) = func_L09_002EC100(
            *(float *)(s0 + 0xF0), *(float *)(s1 + 8) * 0.000290888245f);
        func_L09_00321068(s0, s1 + 0x30);
        break;
    }
}

/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x00413F50,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_00413F50 - throws two sparks from a point 1.5 in front of the
 * actor (carried by the matrix at +0xC0, then moved to the position at
 * +0x10): the first with a velocity drawn between the first two range words
 * per frame (no spread in this level's table), the second from a random
 * point within 1.5 of the first, falling at a speed drawn between the last
 * two.
 *
 * @a0: the actor object.
 * Return: nothing.
 *
 * Non-obvious: the effect's tuning (four resident tables at +0x40 apart in
 * this build) is copied onto the stack first, and the first point is zeroed
 * through TI128 so cc1 clears all four words with one store.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_00412808 (v1.01 address), author commit
 * ce9897f7 (bigmak94). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it); the
 *     u8 typedef line and the file's TI128/Spark typedef block are written
 *     out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_00412808 -> func_L02_00413F50,
 *     D_L02_001B1C10 -> D_L02_001B1C50,
 *     D_L02_001B1C20 -> D_L02_001B1C60,
 *     D_L02_001B1C28 -> D_L02_001B1C68,
 *     D_L02_001B1C30 -> D_L02_001B1C70,
 *     func_00283098 -> func_L02_002F3238,
 *     func_002A7820 -> func_L02_0031FAA8,
 *     func_002A77E0 -> func_L02_0031FA68,
 *     func_002C3150 -> func_L02_00342128,
 *     func_00282C88 -> func_L02_002F2D20,
 *     func_00282CC0 -> func_L02_002F2D58,
 *     func_002A7A00 -> func_L02_0031FD50,
 *     func_002A7878 -> func_L02_0031FB00.
 */
typedef unsigned char u8;
typedef int TI128 __attribute__((mode(TI)));

typedef struct { float f[4]; } SparkRange;
typedef struct { float f[2]; } SparkSizes;
typedef struct { int i[2]; } SparkColours;
typedef struct { u8 b[4]; } SparkKinds;
extern SparkRange D_L02_001B1C50;
extern SparkSizes D_L02_001B1C60;
extern SparkColours D_L02_001B1C68;
extern SparkKinds D_L02_001B1C70;
extern void func_L02_002F3238(void *, void *, void *);
extern int func_L02_0031FAA8(int, int);
extern int func_L02_0031FA68(int);
extern void *func_L02_00342128(void *, void *, int, int, int, int, float);
extern void func_L02_002F2D20(void *);
extern void func_L02_002F2D58(void *, void *, void *);
extern void func_L02_0031FD50(void *, float, float);
extern float func_L02_0031FB00(float, float);

void func_L02_00413F50(u8 *a0) {
    SparkRange range = D_L02_001B1C50;
    SparkSizes sizes = D_L02_001B1C60;
    SparkColours colours = D_L02_001B1C68;
    SparkKinds kinds = D_L02_001B1C70;
    float vel[4];
    float front[4];
    float spot[4];

    *(TI128 *)front = 0;
    front[2] = 1.5f;
    func_L02_002F3238(front, front, a0 + 0xC0);
    func_L02_002F2D58(front, front, a0 + 0x10);
    func_L02_0031FD50(vel, range.f[0] * 0.016666668f, range.f[1] * 0.016666668f);
    func_L02_00342128(front, vel, colours.i[0], (u8)func_L02_0031FAA8(kinds.b[0], kinds.b[1]),
                  (u8)func_L02_0031FA68(0xFF), 0, sizes.f[0]);
    func_L02_002F2D20(vel);
    func_L02_0031FD50(spot, 0.0f, 1.5f);
    func_L02_002F2D58(spot, spot, front);
    vel[2] = -(func_L02_0031FB00(range.f[2], range.f[3]) * 0.016666668f);
    func_L02_00342128(spot, vel, colours.i[1], (u8)func_L02_0031FAA8(kinds.b[2], kinds.b[3]),
                  (u8)func_L02_0031FA68(0xFF), 0, sizes.f[1]);
}

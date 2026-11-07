/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x0043A758, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;

/* A path stores its point count in a 16-byte header, followed by vectors. */
struct RotorPath {
    int count;
    char pad04[12];
    float points[0][4];
};

/* The rotor's state (the object's field 0x68): a 13-point chain, the ten records it drives
   and the path it follows. Only the fields these functions use are named. */
struct RotorState {
    /* 0x000 */ char pad00[0x35];
    /* 0x035 */ u8 f35;
    /* 0x036 */ char pad36[0x1A];
    /* 0x050 */ float aim[4];
    /* 0x060 */ int f60;
    /* 0x064 */ char pad64[0x9C];
    /* 0x100 */ float points[13][4];
    /* 0x1D0 */ float oldPoints[13][4];
    /* 0x2A0 */ char pad2A0[0x10];
    /* 0x2B0 */ int choices[4][4];
    /* 0x2F0 */ char pad2F0[0x30];
    /* 0x320 */ int pathIndex;
    /* 0x324 */ struct RotorPath *path;
    /* 0x328 */ char pad328[8];
    /* 0x330 */ int records[10];
    /* 0x358 */ int linked;
    /* 0x35C */ char pad35C[0xC];
    /* 0x368 */ int projectileClass;
    /* 0x36C */ char pad36C[4];
    /* 0x370 */ s16 pathPoint;
    /* 0x372 */ char pad372[2];
    /* 0x374 */ s16 projectileIndex;
    /* 0x376 */ s16 projectileDelay;
    /* 0x378 */ s16 ropeCount;
    /* 0x37A */ u16 toggle;
    /* 0x37C */ char pad37C[4];
    /* 0x380 */ u8 disabled[10];
};

/* A basis vector whose fourth word is cleared as an integer. */
struct RotorAxis {
    float x, y, z;
    int w;
};

/* A rotor moby, and each 256-byte record of the level's table has the same layout. */
struct RotorObject {
    /* 0x00 */ char pad00[0x10];
    /* 0x10 */ float pos[4];
    /* 0x20 */ u8 mode;
    /* 0x21 */ char pad21[0x13];
    /* 0x34 */ u16 flags;
    /* 0x36 */ char pad36[0x32];
    /* 0x68 */ struct RotorState *state;
    /* 0x6C */ char pad6C[0x4C];
    /* 0xB8 */ int fB8;
    /* 0xBC */ char padBC[4];
    /* 0xC0 */ struct RotorAxis basis[3];
};

/* The level's path table. 0x001FF600 lies inside the split data label
   D_L11_001F0000, so it is an address-named extern (RULING #13291 §2): the
   level ELF values it from its name. */
extern struct RotorPath *D_L11_001FF600[];
/* The level's boot copies of the random-integer and vector-distance helpers. */
extern int func_L11_0032DCC8(int);
extern float func_L11_003013F0(void *, void *);

/**
 * func_L11_0043A758 - pick the rotor's next path by mode, then find the path
 * point nearest the rotor.
 *
 * Flips the state's toggle bit. Mode 2 takes the fixed path
 * D_L11_001FF600[state->pathIndex]; modes 3 and 4 pick one of four candidate
 * path ids at random (func_L11_0032DCC8(4)) from choices row 0/1 (mode 3) or
 * 2/3 (mode 4), the row chosen by the new toggle value. Any other mode keeps
 * the current path. Then it measures the distance (func_L11_003013F0) from
 * obj->pos to point 0 and to each of the first count-1 points, and stores the
 * index of the nearest one in state->pathPoint.
 *
 * @obj: the rotor moby; its state is at +0x68.
 *
 * Non-obvious: the loop starts at point 0 again (so point 0 is measured
 * twice) and never looks at the last point, as the ROM does.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_00432628.c, function func_L11_00438F18 (v1.01
 * address), author commit b4b0fe9b (bigbeno37). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_00438F18 -> func_L11_0043A758 (our split name at the v2.00 place);
 *   - D_L11_001FF580 -> D_L11_001FF600 (address-named, inside D_L11_001F0000);
 *   - func_002A77E0 -> func_L11_0032DCC8, func_00282E80 -> func_L11_003013F0
 *     (the level's boot copies at those places);
 *   - `#include "common.h"` dropped: the u8/u16/s16 typedefs it supplied are
 *     written here, and only the declarations this function uses are kept;
 *   - the opening brace moved onto the signature line (whitespace only), so
 *     level_gate's SEEDED C selftest arm, which finds the body by that line,
 *     can seed this first row.
 */
void func_L11_0043A758(struct RotorObject *obj) {
    struct RotorState *state = obj->state;
    struct RotorPath *path;
    float best;
    float d;
    int pick;
    int i;
    int toggled;

    toggled = state->toggle ^ 1;
    state->toggle = toggled;
    switch (obj->mode) {
    case 2:
        state->path = D_L11_001FF600[state->pathIndex];
        break;
    case 3:
        if (toggled != 0)
            path = D_L11_001FF600[state->choices[0][func_L11_0032DCC8(4)]];
        else
            path = D_L11_001FF600[state->choices[1][func_L11_0032DCC8(4)]];
        state->path = path;
        break;
    case 4:
        if (toggled != 0)
            path = D_L11_001FF600[state->choices[2][func_L11_0032DCC8(4)]];
        else
            path = D_L11_001FF600[state->choices[3][func_L11_0032DCC8(4)]];
        state->path = path;
        break;
    }
    pick = 0;
    best = func_L11_003013F0(obj->pos, state->path->points[0]);
    for (i = 0; i < state->path->count - 1; i++) {
        d = func_L11_003013F0(obj->pos, state->path->points[i]);
        if (d < best) {
            best = d;
            pick = i;
        }
    }
    state->pathPoint = pick;
}

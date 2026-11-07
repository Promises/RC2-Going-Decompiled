/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x00454850, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
extern void func_L11_0032D918(char *, int, int, int);
extern void func_L11_0032DBF8(char *, int, int);
extern void func_L11_00325748(char *);
extern void func_L11_00441978(int, int);
extern void func_L11_0042E300(char *, int, int, int, int, int, int, int,
                         float, float, int, int, int, int);
extern void func_L11_00374F58(int, int, char *, int);
extern void func_L11_0032DFB0(void *, float, float);
extern void func_L11_003011E0(void *, void *, void *);
extern void func_L11_00339740(char *, int, void *, float, float, float, float, int);

/**
 * func_L11_00454850 - tear a target-moby (class 0xD37) down.
 *
 * Clears the +0x2D2 latch and the +0x2C4 word. A still-visible object spawns
 * the death effect (func_L11_0042E300), the class-0xDF7 helper
 * (func_L11_00374F58) and a 0xFF0 impulse packet along the state's +0xD0
 * direction (func_L11_0032DFB0, func_L11_003011E0, func_L11_00339740); the
 * object is then hidden exactly as in func_L11_0044DF28 (group 1/0, +0x34
 * keeps 0x47 and loses 0x1000).
 *
 * @object: the moby.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8.c, function func_L11_00452DF8 (v1.01
 * address), author commit 5acf9869 (Flavius). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_00452DF8 -> func_L11_00454850 (our split name at the v2.00
 *     place);
 *   - func_00282CC0 -> func_L11_003011E0;
 *   - func_002A05A8 -> func_L11_00325748;
 *   - func_002A7470 -> func_L11_0032D918;
 *   - func_002A7750 -> func_L11_0032DBF8;
 *   - func_002A7A00 -> func_L11_0032DFB0;
 *   - func_002B0A68 -> func_L11_00339740;
 *   - func_002E5DA0 -> func_L11_00374F58;
 *   - func_0032C700 -> func_L11_0042E300;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here.
 *
 */
void func_L11_00454850(char *object) {
    char *state = *(char **)(object + 0x68);
    float direction[4];

    *(short *)(state + 0x2D2) = 0;
    *(int *)(state + 0x2C4) = 0;
    if (*(u8 *)(object + 0x31) != 0) {
        func_L11_0042E300(object + 0x10, 0, 5, 5, 25, 20, 10, 1,
                     0.5f, 0.0f, 1, 0, 0, 0);
        func_L11_00374F58(0, 0, object, 0xDF7);
        func_L11_0032DFB0(direction, 0.083333335816860199f, 0.11666667461395264f);
        func_L11_003011E0(direction, direction, state + 0xD0);
        func_L11_00339740(object, 0xFF0, direction, 3.0f, 6.0f, 3.0f, 6.0f, 0xFF);
    }
    func_L11_0032D918(object, 0, 0, 3);
    func_L11_00325748(object);
    func_L11_0032DBF8(object, 1, 0);
    *(u8 *)(object + 0x31) = 0;
    *(int *)(object + 0x98) = 0;
    *(u16 *)(object + 0x34) = (*(u16 *)(object + 0x34) & 0xEFFF) | 0x47;
}

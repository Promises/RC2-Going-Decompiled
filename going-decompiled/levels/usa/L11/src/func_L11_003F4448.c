/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x003F4448, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 16 of 84
 * .text words differ from the disc, relocated fields masked, and .text is
 * 0x150 bytes, not the place's 0x160, and it emits a .lit4 pool of 8 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

typedef struct {
    u8 pad0[0x10];
    f32 x;
    f32 y;
    f32 z;
    u8 pad1C[0x8];
    u8 *model;
    u8 pad28[0x9];
    u8 b31;
    u8 pad32[2];
    u16 flags;
    u8 pad36[0x49];
    u8 b7F;
    u8 pad80[0x18];
    u32 w98;
    u8 pad9C[0x1C];
    s32 slot;
    u8 padBC[0x4];
    u8 mtx[0x30];
    s32 rotW;
    f32 rotX;
    f32 rotZ;
} L11Moby;
extern void func_L11_0032DBF8(u8 *arg0, s32 arg1, s32 arg2);
extern void func_L11_00325678(void *, int, int, int);
extern int func_L11_0033C680(char *, char *, char *);
extern f32 func_L11_00301808(f32 a0);
extern f32 func_L11_00301820(f32 a0);
extern f32 func_L11_003022A8(f32 a0, f32 a1);
extern void func_L11_00301A78(void *a0, void *a1);
extern f32 *func_L11_00443038(s32 a0);
extern f32 D_L11_001B0508 __attribute__((sda));
extern f32 D_L11_001B04F0 __attribute__((sda));

/**
 * func_L11_003F4448 - reset an orbiting Joba object and place it on its circle
 * around its anchor.
 *
 * @arg0: the object (L11Moby view; +0x68 its state block).
 * Return: none.
 *
 * Non-obvious: It clears and sets the object's flags, re-arms its model word
 * (+0x98 from model+0x10), and updates the collision record
 * (func_L11_0033C680). A pending event (+0x70) is replayed to
 * func_L11_00325678. The animation is set (func_L11_0032DBF8(obj, 2, 2)) and
 * the orbit angle (+0x250) reset to 0. The object is then put at its anchor
 * (func_L11_00443038(slot)) plus D_L11_001B0508 times
 * func_L11_00301808/func_L11_00301820 of that angle, at height D_L11_001B04F0,
 * rotation x -pi/2 and z pi + angle (func_L11_003022A8), and its matrix is
 * rebuilt (func_L11_00301A78). The anchor is looked up twice, as upstream
 * writes it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_003F2F38-g8.c (blob 7209f689), function
 * func_L11_003F2F38 (v1.01 address), author commit eabc4759 (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device). Respellings, each to the target of the
 * v2.00 disc's own relocation at this place (relverify by name on this object,
 * task #2578):
 *   - func_L11_003F2F38 -> func_L11_003F4448 (our split name at the v2.00
 *     place);
 *   - D_L11_001B0540 -> D_L11_001B04F0 (our label at that address);
 *   - D_L11_001B0558 -> D_L11_001B0508 (our label at that address);
 *   - func_00283180 -> func_L11_00301808 (the level's own copy at its L11
 *     place);
 *   - func_00283198 -> func_L11_00301820 (the level's own copy at its L11
 *     place);
 *   - func_002833F0 -> func_L11_00301A78 (the level's own copy at its L11
 *     place);
 *   - func_00283B98 -> func_L11_003022A8 (the level's own copy at its L11
 *     place);
 *   - func_002A0488 -> func_L11_00325678 (the level's own copy at its L11
 *     place);
 *   - func_002A7750 -> func_L11_0032DBF8 (the level's own copy at its L11
 *     place);
 *   - func_002B3380 -> func_L11_0033C680 (the level's own copy at its L11
 *     place);
 *   - func_L11_004417E8 -> func_L11_00443038 (its v2.00 split name);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_003F4448(u8 *arg0) {
    L11Moby *m = (L11Moby *)arg0;
    u8 *s1 = *(u8 **)(arg0 + 0x68);
    f32 *p;
    f32 f20;
    f32 y;
    u32 w;

    *(f32 *)(s1 + 0x20) = (f32)*(s16 *)(s1 + 0x24);
    w = *(u32 *)(m->model + 0x10);
    m->flags &= 0xFFFE;
    m->b31 = 1;
    m->w98 = w;
    *(u8 *)(s1 + 0x2E) = 1;
    m->b7F = 0;
    m->flags = (m->flags & 0xFFB9) | 0x1000;
    *(s32 *)(s1 + 0x60) = 0;
    func_L11_0033C680((char *)m, (char *)(s1 + 0x50), (char *)(s1 + 0x20));
    if (*(s16 *)(s1 + 0x70) != 0) {
        *(s16 *)(s1 + 0x70) = 0;
        func_L11_00325678(m, *(u32 *)(s1 + 0x280), *(u32 *)(s1 + 0x284),
                      *(u32 *)(s1 + 0x288));
        *(s16 *)(s1 + 0x72) = 1;
    }
    func_L11_0032DBF8(arg0, 2, 2);
    *(s32 *)(s1 + 0x250) = 0;
    f20 = func_L11_00301808(*(f32 *)(s1 + 0x250));
    p = func_L11_00443038(m->slot);
    m->x = D_L11_001B0508 * f20 + p[0];
    f20 = func_L11_00301820(*(f32 *)(s1 + 0x250));
    p = func_L11_00443038(m->slot);
    y = D_L11_001B0508 * f20 + p[1];
    m->z = D_L11_001B04F0;
    m->rotX = -1.5707965f;
    m->rotW = 0;
    m->y = y;
    m->rotZ = func_L11_003022A8(3.1415927f, *(f32 *)(s1 + 0x250));
    func_L11_00301A78(m->mtx, &m->rotW);
}

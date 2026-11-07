/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0040E8C8,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0040E8C8 - lets go of the slot the actor holds (+0x19C of its
 * state) when that slot's record still names it and is live, ends the state
 * at +0x1B8 when it is 1 (func_L02_003FD750), starts the effect pair when
 * the actor is shown (+0x31), then clears the state and the shared fields as
 * func_L02_0040D0C8 does.
 *
 * @a0: the actor object.
 * Return: nothing.
 *
 * Non-obvious: the slot table is the boot's g_listenerPosHistory (0x188660,
 * 0x70-byte records); the slot index is re-read from state+0x19C into a
 * register of its own before the release test.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_0040D180 (v1.01 address), author commit
 * 24d977dd (bigmak94; the commit that introduced the body — e80a585a added
 * only the declaration and the INCLUDE_ASM marker). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but
 * for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it); the
 *     u8/u16/s16/u32/s32 typedef lines are written out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_0040D180 -> func_L02_0040E8C8,
 *     D_L02_00188660 -> g_listenerPosHistory (the boot's name, same address),
 *     func_002E59F8 -> func_L02_00367AF8,
 *     func_L02_003FC1B0 -> func_L02_003FD750,
 *     func_0032C700 -> func_L02_00420A08,
 *     func_002E5DA0 -> func_L02_00367EA0,
 *     func_002A7470 -> func_L02_0031F6B8,
 *     func_002A05A8 -> func_L02_003174E8,
 *     func_002A7750 -> func_L02_0031F998 (upstream declares it with an empty
 *       parameter list in this unit; kept verbatim).
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;

extern char g_listenerPosHistory[];
extern int func_L02_00367AF8(int);
extern int func_L02_003FD750(void *, int);
extern void func_L02_00420A08(void *, int, int, int, int, int, int, int, float, float,
                          int, int, int, int);
extern void func_L02_00367EA0(int, int, void *, int);
extern void func_L02_0031F6B8(void *, int, int, int);
extern void func_L02_003174E8(void *);
extern void func_L02_0031F998();

void func_L02_0040E8C8(u8 *a0) {
    u8 *s1 = *(u8 **)(a0 + 0x68);
    s32 slot = *(s32 *)(s1 + 0x19C);

    if (slot != -1) {
        u8 *rec = (u8 *)g_listenerPosHistory + slot * 0x70;

        if (*(u8 **)(rec + 0x88) == a0 && rec[0x74] != 0)
            func_L02_00367AF8(slot);
    }
    *(s32 *)(s1 + 0x19C) = -1;
    if (*(s16 *)(s1 + 0x1B8) == 1) {
        *(u16 *)(s1 + 0x1B8) = 0;
        func_L02_003FD750(*(void **)(a0 + 0xB8), *(s16 *)(a0 + 0xAA));
    }
    if (a0[0x31] != 0) {
        func_L02_00420A08(a0 + 0x10, 0, 0xA, 0x1E, 0, 0xA, 0x2D, 1, 0.3f, 0.0f, 0, 0, 0, 0);
        func_L02_00367EA0(0, 0, a0, 0xC3E);
    }
    *(u16 *)(s1 + 0x1B8) = 0;
    *(s32 *)(s1 + 0x184) = 0;
    func_L02_0031F6B8(a0, 0, 0, 3);
    func_L02_003174E8(a0);
    func_L02_0031F998(a0, 1, 0);
    a0[0x31] = 0;
    *(u32 *)(a0 + 0x98) = 0;
    *(u16 *)(a0 + 0x34) = (*(u16 *)(a0 + 0x34) & 0xEFFF) | 0x47;
}

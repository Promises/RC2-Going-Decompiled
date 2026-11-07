/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0040D0C8,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0040D0C8 - arms the actor: its two effect tracks, then the shared
 * state resets.
 *
 * @a0: the actor object.
 * Return: nothing.
 *
 * Non-obvious: the spark burst reads its transform matrix from a0+0x10 and
 * rotates the velocity by the state's +0xD0 matrix; the effect start and the
 * counter reset happen only while the actor is shown (+0x31).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_0040B980 (v1.01 address), author commit
 * e80a585a (Flavius). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it); the
 *     u8/u16/u32 typedef lines are written out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_0040B980 -> func_L02_0040D0C8,
 *     func_0032C700 -> func_L02_00420A08,
 *     func_002E5DA0 -> func_L02_00367EA0,
 *     func_002A7A00 -> func_L02_0031FD50,
 *     func_00282CC0 -> func_L02_002F2D58,
 *     func_002B0A68 -> func_L02_0032C360,
 *     func_002A7470 -> func_L02_0031F6B8,
 *     func_002A05A8 -> func_L02_003174E8,
 *     func_002A7750 -> func_L02_0031F998 (upstream declares it with an empty
 *       parameter list in this unit; kept verbatim).
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

extern void func_L02_00420A08(void *, int, int, int, int, int, int, int, float, float,
                          int, int, int, int);
extern void func_L02_00367EA0(int, int, void *, int);
extern void func_L02_0031FD50(void *, float, float);
extern void func_L02_002F2D58(void *, void *, void *);
extern void func_L02_0032C360(void *, int, void *, float, float, float, float, int);
extern void func_L02_0031F6B8(void *, int, int, int);
extern void func_L02_003174E8(void *);
extern void func_L02_0031F998();

/* arms the actor: its two effect tracks, then the shared state resets */
void func_L02_0040D0C8(u8 *a0) {
    u8 *s1 = *(u8 **)(a0 + 0x68);
    float buf[3];

    *(u16 *)(s1 + 0x2E2) = 0;
    *(u32 *)(s1 + 0x2D4) = 0;
    if (a0[0x31]) {
        func_L02_00420A08(a0 + 0x10, 0, 5, 5, 0x19, 0x14, 0xA, 1, 0.5f, 0.0f,
                      1, 0, 0, 0);
        func_L02_00367EA0(0, 0, a0, 0xC3E);
        func_L02_0031FD50(buf, 0.083333335816860199f, 0.11666667461395264f);
        func_L02_002F2D58(buf, buf, s1 + 0xD0);
        func_L02_0032C360(a0, 0xFF, buf, 3.0f, 6.0f, 3.0f, 6.0f, 0xFF);
    }
    func_L02_0031F6B8(a0, 0, 0, 3);
    func_L02_003174E8(a0);
    func_L02_0031F998(a0, 1, 0);
    *(u8 *)(a0 + 0x31) = 0;
    *(u32 *)(a0 + 0x98) = 0;
    *(u16 *)(a0 + 0x34) = (*(u16 *)(a0 + 0x34) & 0xEFFF) | 0x47;
}

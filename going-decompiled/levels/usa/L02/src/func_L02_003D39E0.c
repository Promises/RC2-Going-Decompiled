/* Level 2 (Maktar Nebula), USA v2.00: the USA-only copy of the entity-state
 * reinit, adopted from OpenRAC (row L02-D, task #2560). Compiled ALONE as its
 * own TU and linked at its one place, 0x003D39E0, in place of its split asm;
 * the row is going-decompiled/levels/usa/L02/config/L02_c.tsv, arm
 * tools/ee/ee_cc1.sh s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_003D39E0 - reinit an entity: copy its target vector, re-seed its
 * effect timers, clear +0x98 and flag 0x1000 of the halfword at +0x34, then
 * restart (or release) its two handlers.
 *
 * @self: the entity object; @flag1/@flag2 select release (0) over restart for
 * the two handlers.
 *
 * Non-obvious: the vector copy moves 16 bytes through a local via TI128
 * (cc1 emits lq/sq on the copy); the timer seed writes three floats in a
 * single statement; when flag2 is set the handler func_L02_003D38E8 is called
 * directly, otherwise its address is handed to func_L02_0040EF00.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/us2/levels/02_maktar_nebula/
 * L02_003D39E0.c, function func_US2_L02_003D39E0 (v2.00 place, upstream name
 * carries it), author commit ac7e1a2e (bigbeno37). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but
 * for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it);
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_US2_L02_003D39E0 -> func_L02_003D39E0,
 *     func_L02_003D1ED0 -> func_L02_003D3428,
 *     func_L02_003D2390 -> func_L02_003D38E8,
 *     func_L02_003FC248 -> func_L02_003FD7E8,
 *     func_L01_003E9CF8 -> func_L02_0040EF00,
 *     func_00282A60 -> func_L02_002F2AB0,
 *     func_0029F978 -> func_L02_00316598,
 *     func_002A7A00 -> func_L02_0031FD50,
 *     func_002B0A68 -> func_L02_0032C360.
 */
typedef int TI128 __attribute__((mode(TI)));

extern void func_L02_003D3428(char *, TI128 *);
extern void func_L02_003D38E8(char *);
extern void func_L02_003FD7E8(char *, int, int);
extern void func_L02_0040EF00(char *, int, void *);
extern void func_L02_002F2AB0(void *, int, int);
extern void func_L02_00316598(char *, int, void *);
extern void func_L02_0031FD50(char *, float, float);
extern void func_L02_0032C360(char *, int, char *, float, float, float, float, int);

void func_L02_003D39E0(char *self, int flag1, int flag2) {
    char *state = *(char **)(self + 0x68);
    float vec[4];

    if (*(char **)(state + 0x2E4) != 0) {
        *(TI128 *)vec = *(TI128 *)(*(char **)(state + 0x2E4) + 0x10);
        func_L02_003D3428(self, (TI128 *)vec);
    }
    if (*(unsigned char *)(state + 0x291) == 0) {
        func_L02_002F2AB0(state + 0x290, 0, 0x40);
        func_L02_00316598(self, 3, state + 0x290);
        *(float *)(state + 0x2B0) = *(float *)(state + 0x2B4) = *(float *)(state + 0x2B8) = 0.0001f;
        *(unsigned char *)(state + 0x292) = 1;
        func_L02_0031FD50((char *)vec, 0.016666668f, 0.050000004f);
        func_L02_0032C360(self, 0x20, (char *)vec, 3.0f, 6.0f, 3.0f, 6.0f, 0xFF);
    }
    *(int *)(self + 0x98) = 0;
    *(unsigned short *)(self + 0x34) &= 0xEFFF;
    if (flag1 == 0)
        func_L02_003FD7E8(self, 0, -1);
    if (flag2 == 0)
        func_L02_0040EF00(self, 0, func_L02_003D38E8);
    else
        func_L02_003D38E8(self);
}

/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x0043C368, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The level's copies of boot helpers, named by their v2.00 places in L11. */
extern void func_L11_0042E300(char *, int, int, int, int, int, int, int, float, float, int, int, int, int);
extern void func_L11_00374F58(int, int, void *, int);
extern void func_L11_0032D918(void *, int, int, int);
extern void func_L11_00325748(char *);
extern void func_L11_0032DBF8(void *, int, int);
extern void func_L11_0032DFB0(float *out, float a, float b);
extern float func_L11_0032DD60(float a, float b);
extern void func_L11_00339740(char *self, int mask, float *arg2, unsigned char arg3,
                              float f0, float f1, float f2, float f3);
extern int func_L11_00374EB0(int index, int a1, char *a2);
extern void func_L11_00324870(char *list, char *node);

/**
 * func_L11_0043C368 - retire the object: run its jitter effect while its
 * flag byte is set, then shut it down and unlink it from its two lists.
 *
 * Does nothing while the state byte at +0x20 is 0. When the byte at +0x31 is
 * set it builds a random vector (func_L11_0032DFB0, then a random z from
 * func_L11_0032DD60) and passes it to func_L11_00339740, then calls
 * func_L11_00374EB0(1, 0, obj), func_L11_0042E300 at the position (+0x10) and
 * func_L11_00374F58(0, 0, obj, 0xDF7). In every case it then calls
 * func_L11_0032DBF8(obj, 1, 0), func_L11_0032D918(obj, 0, 0, 3) and
 * func_L11_00325748(obj), clears +0x31 and +0x98, sets the flag bits 0x47 and
 * clears 0x1000 at +0x34, and removes it from the state's lists at +0x220 and
 * +0x260 if it is linked there (+0x221 / +0x261 non-zero).
 *
 * @obj: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_00432628.c, function func_L11_0043AB28 (v1.01
 * address), author commit 5acf9869 (Flavius). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_0043AB28 -> func_L11_0043C368 (our split name at the v2.00 place);
 *   - func_002A7A00 -> func_L11_0032DFB0, func_002A7878 -> func_L11_0032DD60,
 *     func_002B0A68 -> func_L11_00339740, func_002E5CF8 -> func_L11_00374EB0,
 *     func_0032C700 -> func_L11_0042E300, func_002E5DA0 -> func_L11_00374F58,
 *     func_002A7750 -> func_L11_0032DBF8, func_002A7470 -> func_L11_0032D918,
 *     func_002A05A8 -> func_L11_00325748, func_0029F9F0 -> func_L11_00324870;
 *   - `#include "common.h"` dropped (nothing in this function used it); the
 *     prototypes are the file's own declarations of those names, and every
 *     other declaration of the file is omitted.
 */
void func_L11_0043C368(char *obj)
{
    char *state;
    unsigned short flags;
    float v[3];

    if (*(unsigned char *)(obj + 0x20) == 0)
        return;

    if (*(unsigned char *)(obj + 0x31) != 0) {
        float s = 1.0f / 6.0f;

        func_L11_0032DFB0(v, s, 1.0f / 3.0f);
        v[2] = func_L11_0032DD60(1.0f / 12.0f, s);
        func_L11_00339740(obj, 3, v, 0xFF, 3.0f, 6.0f, 3.0f, 6.0f);
        func_L11_00374EB0(1, 0, obj);
        func_L11_0042E300(obj + 0x10, 1, 0x19, 0xA, 0xA, 5, 0, 1, 1.0f, 20.0f, 0, 0, 0, 0);
        func_L11_00374F58(0, 0, obj, 0xDF7);
    }

    func_L11_0032DBF8(obj, 1, 0);
    func_L11_0032D918(obj, 0, 0, 3);
    func_L11_00325748(obj);

    state = *(char **)(obj + 0x68);
    flags = *(unsigned short *)(obj + 0x34);
    *(unsigned char *)(obj + 0x31) = 0;
    *(int *)(obj + 0x98) = 0;
    *(unsigned short *)(obj + 0x34) = (flags & 0xEFFF) | 0x47;

    if (*(unsigned char *)(state + 0x221))
        func_L11_00324870(obj, state + 0x220);
    if (*(unsigned char *)(state + 0x261))
        func_L11_00324870(obj, state + 0x260);
}

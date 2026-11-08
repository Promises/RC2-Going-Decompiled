/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00418BF0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned short u16;

/* L30's functions at their v2.00 places. */
extern char *func_L30_00317AD8(int);
extern void func_L30_0032D158(void *, void *);
extern void func_L30_002F6258(void *, void *);

/**
 * func_L30_00418BF0 - spawn the actor's companion moby of class 0xB28 and
 * remember it.
 * @a0: the actor; its state block is at +0x68.
 *
 * func_L30_00317AD8(0xB28) spawns the moby; on failure nothing else happens.
 * Otherwise func_L30_0032D158 ties it to @a0, func_L30_002F6258 derives its
 * +0xF0 block from its matrix at +0xC0, bit 0x100 of its flags halfword
 * (+0x34) is set and the moby is stored at state +0x50.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0040E660.c, function
 * func_L30_00418570 (v1.01 address), author commit ee3fee45 (llesieur99;
 * `git blame -M -C` gives 10 of the 11 definition lines to it, the 11th, a
 * blank line, to upstream's root commit 7fccf735; `git log -S` agrees).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names, each name to the target of the v2.00
 * disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_00418570 -> func_L30_00418BF0 (our split name);
 *   - func_316A50 -> func_L30_00317AD8, func_32BFF0 -> func_L30_0032D158,
 *     func_2F56B0 -> func_L30_002F6258 (L30's copies at their v2.00 places);
 *   - NOTE #13281 codes this entry `l` (`long long`->`long`), a FILE-level
 *     attribution (the file fails in cc1 on another function): this body and
 *     its declarations hold no `long long`, so no respelling is made;
 *   - `#include "include_asm.h"` and the file's other declarations dropped;
 *     the file's own u16 typedef is kept.
 */
void func_L30_00418BF0(char *a0) {
    char *st = *(char **)(a0 + 0x68);
    char *o;

    o = func_L30_00317AD8(0xB28);
    if (o == 0) return;
    func_L30_0032D158(o, a0);
    func_L30_002F6258(o + 0xC0, o + 0xF0);
    *(u16 *)(o + 0x34) |= 0x100;
    *(char **)(st + 0x50) = o;
}

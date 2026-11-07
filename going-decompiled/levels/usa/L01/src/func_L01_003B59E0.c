/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003B59E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed short s16;
typedef unsigned char u8;
typedef unsigned int u32;

extern void memset(void *, int, int);

extern int func_L01_002D6C20(float, char *, int, char *, int);

extern void *D_L01_001C3980[];

/**
 * func_L01_003B59E0 - collect up to 16 live entities of kind 0xA26 into a
 * caller's array.
 *
 * The array at @arg1 is cleared (memset of its first word; the count at @arg2
 * zeroed), func_L01_002D6C20 bounds the candidate count (the 3.5f radius
 * query against the object), and every non-null entity in D_L01_001C3980
 * whose kind word at +0xAA is 0xA26 is activated (+0x9C = 1, the +0xBC byte
 * = 1) and stored into the array until 16 are gathered.
 *
 * @arg0: the object (query centre).
 * @arg1: the caller's entity array (at least 16 slots).
 * @arg2: receives the number of entities stored.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003835D8.c, function func_L01_003B45D8 (v1.01
 * address), author commit c9a5add7 (Flavius), with one body line changed at
 * ec778f01 (Flavius), by blame of the definition (validator #2638; 6d931c1b,
 * named here before, does not touch this function).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2626 relverify by
 * name):
 *   - func_L01_003B45D8 -> func_L01_003B59E0 (our split name);
 *   - func_00115484 -> memset (the boot's core name at 0x00115484; upstream's
 *     own prototype kept, the name measured codegen-neutral on this arm);
 *   - func_00277F48 -> func_L01_002D6C20 (L01's own copy at the v2.00 place);
 *   - D_L01_001C3980 kept address-named (RULING #13291 section 2: the v2.00
 *     target 0x001C3980 lies inside g_pointLights+0xEC0, no split label
 *     starts there);
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped, common.h's typedefs this body uses written out
 *     in the TU instead (validator #2638): this function references no other symbol.
 */
void func_L01_003B59E0(void *arg0, void **arg1, int *arg2) {
    int i;
    int count;
    void *p;

    memset(arg1, 0, 4);
    *arg2 = 0;
    count = func_L01_002D6C20(3.5f, (char *)arg0 + 0x10, 1, (char *)arg0, 0);
    if (count <= 0)
        return;
    for (i = 0; i < count; i++) {
        p = D_L01_001C3980[i];
        if (p == 0)
            continue;
        if (*(s16 *)((u8 *)p + 0xAA) != 0xA26)
            continue;
        *(u32 *)((u8 *)p + 0x9C) = 1;
        *(u8 *)((u8 *)D_L01_001C3980[i] + 0xBC) = 1;
        arg1[*arg2] = D_L01_001C3980[i];
        *arg2 = *arg2 + 1;
        if (*arg2 >= 16)
            break;
    }
}

/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003B6698, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef unsigned char u8;

extern char *g_mobyTableBase __attribute__((sda));

extern u8 *g_mobyGroupLists[];

/**
 * func_L01_003B6698 - flag every entity of kind 0x9A2 that the object's group
 * index selects.
 *
 * The group byte at +0x21 selects an index list in g_mobyGroupLists (0xFF or a
 * null list returns immediately). Each u16 entry addresses a 0x100-byte
 * record of the table g_mobyTableBase at (entry & 0x7FFF) << 8; a record whose
 * visibility byte at +0x20 is non-negative and whose kind at +0xAA is 0x9A2
 * gets the word 1 stored at +0x350 of the record its +0x68 points at. A
 * negative index (the high bit of the u16) ends the walk.
 *
 * @arg0: the object record (only its +0x21 group byte is read).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003835D8-g8.c, function func_L01_003B5290
 * (v1.01 address), author commit 91e04732 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2626 relverify by name):
 *   - func_L01_003B5290 -> func_L01_003B6698 (our split name);
 *   - D_L01_001B255C -> g_mobyTableBase, D_L01_001FB020 -> g_mobyGroupLists
 *     (task #2646): the names symbol_addrs/L01_boot_names.txt gives at
 *     exactly those addresses (the v2.00 targets 0x001B255C and 0x001FB020);
 *     the address-named spelling was a second name for the same address (A2
 *     class, NOTE #13409);
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped, common.h's typedefs this body uses written out
 *     in the TU instead (validator #2638): this function references no other symbol.
 */
void func_L01_003B6698(u8 *arg0) {
    char *a2;
    u8 *p;
    u8 *list;

    if (arg0[0x21] == 0xFF)
        return;
    list = g_mobyGroupLists[arg0[0x21]];
    if (list == 0)
        return;
    p = list;
    a2 = g_mobyTableBase;
    for (;;) {
        char *q = a2 + ((*(unsigned short *)p & 0x7FFF) << 8);

        if (*(signed char *)(q + 0x20) >= 0) {
            if (*(short *)(q + 0xAA) == 0x9A2) {
                *(int *)(*(char **)(q + 0x68) + 0x350) = 1;
            }
        }
        if (*(short *)p < 0)
            break;
        p += 2;
    }
}

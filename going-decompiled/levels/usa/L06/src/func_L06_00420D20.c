/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00420D20, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef unsigned char u8;

extern char *g_mobyTableBase __attribute__((sda));
extern u8 *g_mobyGroupLists[];

/**
 * func_L06_00420D20 - flag the members of a moby's group that are of class
 * 0xBBA.
 * @arg0: the moby; its byte +0x21 is its group index (0xFF: no group).
 *
 * Walks the group's 16-bit id list g_mobyGroupLists[group]; each id, masked to
 * 15 bits, selects a 0x100-byte record of the moby table at g_mobyTableBase.
 * For every live record (signed state byte +0x20 >= 0) of class 0xBBA (+0xAA)
 * the word at +0x258 of its pvar block (+0x68) is set to 1. The id with bit
 * 15 set is the last one examined.
 * Return: none (also when the group is 0xFF or its list is empty).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0041DED8-g8.c, function
 * func_L06_0041F798 (v1.01 address), author commit 431ac503 (llesieur99;
 * `git blame -M -C` and `git log -S` agree); its loop text, every line but
 * the class test and the store, first stands in llesieur99's
 * func_L01_003B5290 (91e04732, 2026-10-12T16:27, earlier the same day; our
 * func_L01_003B6698), from which this body was copied with its class
 * constant and store changed (RULING #13427 rev 2/rev 4, #13541: credited
 * for the lines that survive). Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names (BYTE-
 * IDENTICAL), each name to the target of the v2.00 disc's own relocation at
 * this place (task #2653 relverify by name); edits:
 *   - func_L06_0041F798 -> func_L06_00420D20 (our split name at the v2.00
 *     place)
 *   - D_L06_001B281C -> g_mobyTableBase and D_L06_001FB3A0 ->
 *     g_mobyGroupLists: the names symbol_addrs/L06_boot_names.txt gives at
 *     exactly those addresses (task #2646 A2 rule; upstream's address-named
 *     spelling would be a second name for the same address). Upstream's
 *     `__attribute__((sda))` on g_mobyTableBase kept verbatim: our cc1
 *     ignores it (a warning), and at -G8 the 4-byte extern is addressed gp-
 *     relative either way
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped; the u8 typedef it supplied is spelled here
 */
void func_L06_00420D20(u8 *arg0)
{
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
            if (*(short *)(q + 0xAA) == 0xBBA) {
                *(int *)(*(char **)(q + 0x68) + 0x258) = 1;
            }
        }
        if (*(short *)p < 0)
            break;
        p += 2;
    }
}

/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2581, RULING #13291 row L14-ASM). Compiled ALONE as its own TU and
 * linked at its one place, 0x00429CD8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8 with the row's
 * cc1flags -fno-gcse, assembled by plain GNU as. Measured (task #2581, same
 * TU): without -fno-gcse .text is 0x80 bytes, not the place's 0x7C
 * (level_elf.py cobj refuses); at -G0 it is 0x84. The as column is not
 * load-bearing here: asm_unit closes it too, and so does the sn2952 arm without
 * -fno-gcse; the row keeps the audit's closing arm (NOTE #13281: sdk29, gcse
 * off, plain as).
 */

extern unsigned char *D_L14_001FAFE0[];
extern unsigned char *D_L14_001B251C __attribute__((sda));

/**
 * func_L14_00429CD8 - flag every class 0x112B moby in an object's id list.
 *
 * @a0: the object; byte +0x21 indexes the list table D_L14_001FAFE0 (0xFF =
 *      none).
 * Return: nothing.
 *
 * Non-obvious: Upstream's comment: "Walks the 0x7FFF-masked id list at
 * table[index] and flags class 0x112B entries." Each id selects a 0x100-byte
 * moby record of D_L14_001B251C; a live record (byte +0x20 >= 0) of class
 * 0x112B (+0xAA) gets 1 in the halfword at +0x316 of its state block (+0x68).
 * The list ends at the first id with its top bit set, which is still processed.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own, `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c (blob
 * b6afe825), function func_L14_004286F8 (v1.01 address), author commit 4541c4ce
 * (ReaperOfVeriod). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082, #13291). ADAPTED: built at -G8 where upstream builds the file
 * at -G0 (its stem has no -g8; a tsv gflag setting, not a text edit; measured
 * at -G0, task #2581: .text 0x84 bytes, not 0x7C, cobj refuses), and with
 * -fno-gcse (the row's cc1flags column, a per-row flag as RULING #12178;
 * upstream builds with gcse on); the text is verbatim but for names. Edits,
 * each name to the target of the v2.00 disc's own relocation at this place
 * (relverify by name on this object, task #2581):
 *   - func_L14_004286F8 -> func_L14_00429CD8 (our split name at the v2.00
 *     place);
 *   - D_L14_001B259C_1 -> D_L14_001B251C (address-named, inside
 *     D_L14_001B1D00+0x81C; RULING #13291 section 2);
 *   - D_L14_001FB060 -> D_L14_001FAFE0 (address-named, inside
 *     D_L14_001F0000+0xAFE0; RULING #13291 section 2);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning) and the member closes with it as written;
 *   - `#include "common.h"` dropped (the body uses none of its typedefs);
 *     upstream file-scope declarations the body does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L14_00429CD8(unsigned char *a0)
{
    unsigned char *base;
    unsigned char *p;
    unsigned short *q;
    char *e;

    if (a0[0x21] == 0xFF)
        return;
    p = D_L14_001FAFE0[a0[0x21]];
    if (p == 0)
        return;
    q = (unsigned short *)p;
    base = D_L14_001B251C;
    for (;;) {
        e = (char *)(base + ((*q & 0x7FFF) << 8));
        if ((signed char)e[0x20] >= 0) {
            if (*(signed short *)(e + 0xAA) == 0x112B) {
                *(unsigned short *)(*(unsigned int *)(e + 0x68) + 0x316) = 1;
            }
        }
        if ((signed short)*q < 0)
            break;
        q++;
    }
}

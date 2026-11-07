/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00442A18, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;

/* Id lists, one per class; an element inside L11's larger .bss label
 * D_L11_001F0000, so address-named (RULING #13291 §2). */
extern u8 *D_L11_001FC4A0[];

/* The handlers, at their v2.00 places in L11. */
extern void func_L11_0044B2A0(u8 *);
extern void func_L11_0044DF28(u8 *);
extern void func_L11_00454850(u8 *);
extern void func_L11_00456830(u8 *);
extern void func_L11_00457F98(u8 *);
extern void func_L11_004382B8(u8 *);
extern void func_L11_0043B430(u8 *);
extern void func_L11_0043C368(u8 *);
extern void func_L11_003F45A8(u8 *);

/* The base of the level's 256-byte object records, a word inside the .bss
 * block D_L11_001B2E00 (address-named, RULING #13291 §2), read through an
 * incomplete array. The section attribute is an ADDRESSING-MODEL DEVICE on
 * this declaration (RULING #8620): cc1 prints a one-instruction macro load,
 * which GNU as expands at -G0 into the absolute lui/lw pair; cc1 does not
 * hoist it out of the loop, as the ROM has it. It moves no data and emits
 * nothing. */
extern u8 *D_L11_001B361C[] __attribute__((section(".sdata")));

/**
 * func_L11_00442A18 - hand every entry of the actor's class list to its
 * handler.
 *
 * @obj: the actor; the word at state+0xC0 names its class list (0xFF = none).
 *
 * It walks the class's id list in D_L11_001FC4A0. Each halfword's low 15 bits
 * index a 256-byte record from D_L11_001B361C, and the list ends after the
 * first id with its sign bit set. The record's type halfword at +0xAA picks
 * the handler: 0xD98, 0xDCF, 0xDF4, 0xDF7, 0xDF9, 0xD37, 0xD4A, 0xD4B and
 * 0xB3A each have one; any other type is skipped.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_0043E1A0.c, function
 * func_L11_004411C8 (v1.01 address), author commit f6353318 (DrLegitamate).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L11_004411C8 -> func_L11_00442A18 (our split name at the
 *     v2.00 place);
 *   - D_L11_001FC420 -> D_L11_001FC4A0, and the nine handlers
 *     func_L11_00449878/0044C4F0/00452DF8/00454DD8/00456540/00436A78/
 *     00439BF0/0043AB28/003F3098 -> func_L11_0044B2A0/0044DF28/00454850/
 *     00456830/00457F98/004382B8/0043B430/0043C368/003F45A8: upstream names
 *     state v1.01 addresses, ours are the targets of the v2.00 ROM's own
 *     relocations (relverify, task #2553);
 *   - upstream reads the record base through an asm-label view,
 *     `extern u8 *D_L11_001B359C_base[] __asm__("D_L11_001B359C")
 *     __attribute__((sda))`, because its file also declares
 *     D_L11_001B359C as a scalar. This one-function TU has only the one
 *     declaration, so it is written directly as D_L11_001B361C (the v2.00
 *     target) with the same type: no asm label (no device). The use in the
 *     body is respelled D_L11_001B359C_base[0] -> D_L11_001B361C[0];
 *   - `__attribute__((sda))` -> `section(".sdata")` (RULING #8620);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped; the
 *     typedefs used written out; the upstream comments folded into this one;
 *     the file's other functions and their externs not carried.
 */
void func_L11_00442A18(u8 *obj)
{
    s32 index = *(s32 *)(*(u8 **)(obj + 0x68) + 0xC0);
    u16 *p;
    u8 *list;
    u8 *entry;

    if (index == 0xFF)
        return;
    list = D_L11_001FC4A0[index];
    if (list == 0)
        return;
    p = (u16 *)list;
    do {
        entry = D_L11_001B361C[0] + ((*p & 0x7FFF) << 8);
        switch (*(s16 *)(entry + 0xAA)) {
        case 0xD98:
            func_L11_0044B2A0(entry);
            break;
        case 0xDCF:
            func_L11_0044DF28(entry);
            break;
        case 0xDF4:
            func_L11_00454850(entry);
            break;
        case 0xDF7:
            func_L11_00456830(entry);
            break;
        case 0xDF9:
            func_L11_00457F98(entry);
            break;
        case 0xD37:
            func_L11_004382B8(entry);
            break;
        case 0xD4A:
            func_L11_0043B430(entry);
            break;
        case 0xD4B:
            func_L11_0043C368(entry);
            break;
        case 0xB3A:
            func_L11_003F45A8(entry);
            break;
        }
    } while (*(s16 *)p++ >= 0);
}

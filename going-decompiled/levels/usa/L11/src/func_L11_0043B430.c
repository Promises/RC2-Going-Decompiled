/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x0043B430, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The base of the level's 256-byte object records, a word inside the .bss
 * block D_L11_001B2E00 (address-named, RULING #13291 §2). The section
 * attribute is an ADDRESSING-MODEL DEVICE on this declaration (RULING #8620):
 * cc1 prints a one-instruction macro load instead of splitting %hi/%lo
 * itself, and GNU as expands it at -G0 into the lui/lw pair the ROM has. It
 * moves no data and emits nothing. */
extern int D_L11_001B361C __attribute__((section(".sdata")));

/* L11's copies of boot code, at their v2.00 places in this level. */
extern void func_L11_0032DBF8(void *, int, int);
extern int func_L11_00374B68(char *obj, int a);
extern void func_L11_00374BB0(int a);

/* The boot's 0x70-byte slot table (core, below 0x1A7C80). */
extern char g_listenerPosHistory[];

/* Address of the 256-byte record selected by the word at state+0x358. */
#define REC_L11_359C(o, t) ((*(int *)(*(char **)((o) + 0x68) + 0x358) << 8) + (int)(t))

/**
 * func_L11_0043B430 - release the actor's slot and reset its flags and its
 * record's flags.
 *
 * @obj: the actor; its state block is at +0x68.
 *
 * It calls func_L11_0032DBF8(obj, 1, 0) and resets the actor's flags: the
 * +0x34 halfword gets 0x47 set and 0x1000 cleared, and byte +0x31 and word
 * +0x98 are zeroed. If func_L11_00374B68(obj, slot) is nonzero for the slot
 * at state+0x38C, the slot is released. When the slot is not -1 and its
 * g_listenerPosHistory entry (0x70 bytes) points back at obj (+0x88) with a
 * nonzero +0x74, func_L11_00374BB0(slot) is called first. Then state+0x38C
 * becomes -1. Finally the record named by state+0x358 gets the same flag
 * reset: 0x2, 0x4, 0x40 and 0x1 set and 0x1000 cleared in +0x34, and +0x31
 * and +0x98 zeroed.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00432628.c, function
 * func_L11_00439BF0 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L11_00439BF0 -> func_L11_0043B430 (our split name at the
 *     v2.00 place);
 *   - D_00188660 -> g_listenerPosHistory (the boot's name, same address);
 *   - D_L11_001B359C -> D_L11_001B361C, func_002A7750 -> func_L11_0032DBF8,
 *     func_373978 -> func_L11_00374B68, func_3739C0 -> func_L11_00374BB0: the
 *     targets of the v2.00 ROM's own relocations (relverify, task #2553);
 *   - `__attribute__((sda))` -> `section(".sdata")` (RULING #8620);
 *   - `#include "common.h"` dropped (no typedef of it is used here); the
 *     macro REC_L11_359C kept verbatim; the upstream one-line comment folded
 *     into this one; the file's other functions and their externs not
 *     carried.
 */
void func_L11_0043B430(char *obj)
{
    char *state;
    char *records;

    func_L11_0032DBF8(obj, 1, 0);
    state = *(char **)(obj + 0x68);
    *(unsigned short *)(obj + 0x34) = (*(unsigned short *)(obj + 0x34) & 0xEFFF) | 0x47;
    *(unsigned char *)(obj + 0x31) = 0;
    *(int *)(obj + 0x98) = 0;
    if (func_L11_00374B68(obj, *(int *)(state + 0x38C)) != 0) {
        if (*(int *)(state + 0x38C) != -1) {
            char *entry = g_listenerPosHistory + *(int *)(state + 0x38C) * 0x70;

            if (*(char **)(entry + 0x88) == obj && *(unsigned char *)(entry + 0x74) != 0)
                func_L11_00374BB0(*(int *)(state + 0x38C));
        }
        *(int *)(state + 0x38C) = -1;
    }
    records = (char *)D_L11_001B361C;
    *(unsigned short *)(REC_L11_359C(obj, records) + 0x34) |= 0x2;
    *(unsigned short *)(REC_L11_359C(obj, records) + 0x34) |= 0x4;
    *(unsigned short *)(REC_L11_359C(obj, records) + 0x34) |= 0x40;
    *(unsigned short *)(REC_L11_359C(obj, records) + 0x34) &= 0xEFFF;
    *(unsigned short *)(REC_L11_359C(obj, records) + 0x34) |= 0x1;
    *(unsigned char *)(REC_L11_359C(obj, records) + 0x31) = 0;
    *(int *)(REC_L11_359C(obj, (char *)D_L11_001B361C) + 0x98) = 0;
}

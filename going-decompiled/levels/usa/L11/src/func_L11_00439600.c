/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00439600, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The base of the level's 256-byte object records, L11's boot-name map
 * name at 0x001B361C (task #2646). The section
 * attribute is an ADDRESSING-MODEL DEVICE on this declaration (RULING #8620):
 * cc1 prints a one-instruction macro load instead of splitting %hi/%lo
 * itself, and GNU as expands it at -G0 into the lui/lw pair the ROM has. It
 * moves no data and emits nothing. */
extern int g_mobyTableBase __attribute__((section(".sdata")));

/* L11's copy of the boot's func_002A7470 (record state change); left
 * unprototyped, as upstream declares it. */
extern void func_L11_0032D918();

/**
 * func_L11_00439600 - move the ten records the actor's state lists to state
 * a1.
 *
 * @p:  the actor; its state block is at +0x68.
 * @a1: the wanted value of each record's byte +0x43.
 * @a2, @a3: passed through to func_L11_0032D918.
 *
 * For i = 0..9, the word at state+0x330+4*i indexes a 256-byte record from
 * g_mobyTableBase. Each record whose byte +0x43 differs from a1 is passed to
 * func_L11_0032D918(record, a1, a2, a3).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00432628.c, function
 * func_L11_00437DC0 (v1.01 address), author commit fa8d6672 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L11_00437DC0 -> func_L11_00439600 (our split name at the
 *     v2.00 place);
 *   - D_L11_001B359C -> g_mobyTableBase and func_32C700 -> func_L11_0032D918:
 *     the targets of the v2.00 ROM's own relocations (relverify, task #2553);
 *   - D_L11_001B361C -> g_mobyTableBase (task #2646): the name
 *     symbol_addrs/L11_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `__attribute__((sda))` -> `section(".sdata")` (RULING #8620);
 *   - `#include "common.h"` dropped (no typedef of it is used here); the
 *     upstream one-line comment folded into this one; the file's other
 *     functions and their externs not carried.
 */
void func_L11_00439600(char *p, int a1, int a2, int a3)
{
    int i;

    for (i = 0; i < 10; i++) {
        int offset = i * 4;
        int index = *(int *)(*(char **)(p + 0x68) + offset + 0x330);
        char *record = (char *)(g_mobyTableBase + (index << 8));

        if (*(unsigned char *)(record + 0x43) != a1)
            func_L11_0032D918(record, a1, a2, a3);
    }
}

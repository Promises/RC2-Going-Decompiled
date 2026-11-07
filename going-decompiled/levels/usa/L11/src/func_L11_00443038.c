/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00443038, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SDK 2.9 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef signed int s32;

/* The base of a table of 128-byte entries, a word inside the .bss block
 * D_L11_001B2E00 (address-named, RULING #13291 §2). The section attribute is
 * an ADDRESSING-MODEL DEVICE on this declaration (RULING #8620): cc1 prints
 * a one-instruction macro load instead of splitting %hi/%lo itself, and GNU
 * as expands it at -G0 into the lui/lw pair the ROM has. It moves no data and
 * emits nothing. */
extern u8 *D_L11_001B370C __attribute__((section(".sdata")));

/**
 * func_L11_00443038 - the payload of the 128-byte entry the actor's state
 * selects.
 *
 * @object: the actor; the word at state+0x168 is the entry index.
 * Return: D_L11_001B370C + index * 128 + 0x30.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_0043E1A0.c, function
 * func_L11_004417E8 (v1.01 address), author commit 264cb2be (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L11_004417E8 -> func_L11_00443038 (our split name at the
 *     v2.00 place);
 *   - D_L11_001B368C -> D_L11_001B370C: the target of the v2.00 ROM's own
 *     relocations (relverify, task #2553);
 *   - `__attribute__((sda))` -> `section(".sdata")` (RULING #8620);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped; the
 *     typedefs used written out; the upstream one-line comment folded into
 *     this one; the file's other functions and their externs not carried.
 */
u8 *func_L11_00443038(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);
    return D_L11_001B370C + (*(s32 *)(state + 0x168) << 7) + 0x30;
}

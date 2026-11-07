/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x0042BCF8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef signed char s8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef float f32;

/* Id lists, one per group; an element inside L11's larger .bss label
 * D_L11_001F0000, so address-named (RULING #13291 §2). */
extern void *D_L11_001FC4A0[];
/* The base of the level's 256-byte object records, a word inside the .bss
 * block D_L11_001B2E00 (address-named, RULING #13291 §2). The section
 * attribute is an ADDRESSING-MODEL DEVICE on this declaration (RULING #8620):
 * cc1 prints a one-instruction macro load instead of splitting %hi/%lo
 * itself, and GNU as expands it at -G0 into the lui/lw pair the ROM has. It
 * moves no data and emits nothing. */
extern u8 *D_L11_001B361C[] __attribute__((section(".sdata")));

/**
 * func_L11_0042BCF8 - bump the hit counters of the group's 0xCF6/0xCF5
 * members.
 *
 * @arg0: the actor. Byte +0x21 names its group (0xFF = none); its state block
 *        is at +0x68.
 *
 * The same walk as func_L11_00427310, with the two type tests in the other
 * order. For each live record (byte +0x20 >= 0) of type 0xCF6 or 0xCF5 in the
 * group's id list, it sets the record state's +0x266 halfword to 1 and adds 1
 * to the word at +0x560. A 0xCF5 also gets 1 added at +0x584 when the actor
 * state's float at +0x20 is <= 0.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00425F10.c, function
 * func_L11_0042A4C8 (v1.01 address), author commit ba383701 (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L11_0042A4C8 -> func_L11_0042BCF8 (our split name at the
 *     v2.00 place);
 *   - D_L11_001FC420 -> D_L11_001FC4A0 and D_L11_001B359C -> D_L11_001B361C:
 *     upstream names state v1.01 addresses, ours are the targets of the
 *     v2.00 ROM's own relocations (relverify, task #2553);
 *   - `__attribute__((sda))` -> `section(".sdata")` (RULING #8620);
 *   - `#include "common.h"` dropped; its typedefs written out (the ones
 *     used); the file's other functions and their externs not carried.
 */
void func_L11_0042BCF8(u8 *arg0) {
    void *list;
    u16 *p;
    u8 *rec;
    u8 *owner;
    u8 *base;

    if (arg0[0x21] == 0xFF)
        return;
    list = D_L11_001FC4A0[arg0[0x21]];
    arg0 = *(u8 **)(arg0 + 0x68);
    if (list == 0)
        return;
    base = D_L11_001B361C[0];
    p = list;
    do {
        rec = base + ((*p & 0x7FFF) << 8);
        if (*(s8 *)(rec + 0x20) >= 0 && *(s16 *)(rec + 0xAA) == 0xCF6) {
            owner = *(u8 **)(rec + 0x68);
            *(u16 *)(owner + 0x266) = 1;
            (*(u32 *)(owner + 0x560))++;
        }
        if (*(s8 *)(rec + 0x20) >= 0 && *(s16 *)(rec + 0xAA) == 0xCF5) {
            owner = *(u8 **)(rec + 0x68);
            *(u16 *)(owner + 0x266) = 1;
            (*(u32 *)(owner + 0x560))++;
            if (*(f32 *)(arg0 + 0x20) <= 0.0f)
                (*(u32 *)(owner + 0x584))++;
        }
    } while (*(s16 *)p++ >= 0);
}

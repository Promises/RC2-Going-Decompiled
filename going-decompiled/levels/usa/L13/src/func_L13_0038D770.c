/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x0038D770, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm ee-gcc 2.9-ee-991111b
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8 (upstream builds its file at -G8), plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef struct Node {
    u8 pad0[0x28];
    struct Node *next;
    u8 pad2[0x34 - 0x2C];
    u16 flags;
    u8 pad3[0x98 - 0x36];
    u32 f98;
    u8 pad4[0xAA - 0x9C];
    s16 fAA;
} Node;
extern int D_L13_001A8F00 __attribute__((section(".sdata")));
extern int D_L13_001BD570 __attribute__((section(".data")));
extern Node *D_L13_001B28A8[1] __attribute__((section(".sdata")));

/**
 * func_L13_0038D770 - flag every listed object of four classes when the
 * level reaches a given state.
 *
 * @a0: the object; its state byte is at +0x20.
 * Return: nothing.
 *
 * State 0 sets the byte +0x30 to 0xFF and moves to state 1. In state 1,
 * when D_L13_001A8F00 is 2 and D_L13_001BD570 is 3, it walks the list headed
 * by D_L13_001B28A8[0] (next pointer at +0x28); each node of class (+0xAA)
 * 0x1F4, 0xEA3, 0xEA4 or 0x11EB gets its word +0x98 cleared and its flags
 * halfword +0x34 ORed with 0x41 with bit 0x1000 cleared.
 *
 * Non-obvious: D_L13_001A8F00 and D_L13_001B28A8 are $gp-addressed in
 * retail; with this file at -G8 the respelled `section(".sdata")` gets that
 * and `section(".data")` keeps D_L13_001BD570 absolute (RULING #8620, see the
 * edits).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/13_boldan/L13_00306F38-g8.c, function func_L13_0038C328 (v1.01 address), author commit 09afaadd (Louis-Philippe Le Sieur).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_L13_0038C328 -> func_L13_0038D770 (our split name at the v2.00 place);
 *   - D_001BD5F0 -> D_L13_001BD570: an address-named extern (RULING #13291 section 2) at
 *   0x001BD570, inside the split label D_L13_001B2C30 (+0xA940); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L13_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2672);
 *   - D_L13_001B2928 -> D_L13_001B28A8: an address-named extern (RULING #13291 section 2) at
 *   0x001B28A8, inside the split label D_L13_001B2080 (+0x828); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L13_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2672);
 *   - D_L13_001A8F00 kept: the split label at exactly 0x001A8F00 (the boot-name map's g_nGameState
 *   there is not used, an exact split label wins);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on D_L13_001A8F00 and D_L13_001B28A8 (RULING #8620
 *   ADDRESSING-MODEL DEVICE, EE arm only; this TU has no other arm): our cc1
 *   does not know `sda`/`nosda` (it warns "directive ignored"); with `.sdata` at -G8 the
 *   access is $gp-relative (R_MIPS_GPREL16), as retail's is. It moves no
 *   data and emits nothing;
 *   - `__attribute__((nosda))` -> `__attribute__((section(".data")))` on D_L13_001BD570 (RULING #8620
 *   ADDRESSING-MODEL DEVICE, EE arm only; this TU has no other arm): our cc1
 *   does not know `sda`/`nosda` (it warns "directive ignored"); with `.data` the access
 *   stays an absolute %hi/%lo pair, as retail's is. It moves no
 *   data and emits nothing;
 *   - ablation (task #2672): with upstream's `sda`/`nosda` spelling and nothing
 *   else changed, 26 of 42 masked words differ (0xA8 bytes, not 0xB0);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L13_0038D770(u8 *a0) {
    Node *p;

    switch (a0[0x20]) {
    case 0:
        a0[0x30] = 0xFF;
        a0[0x20] = 1;
        break;
    case 1:
        if (D_L13_001A8F00 == 2 && D_L13_001BD570 == 3) {
            for (p = D_L13_001B28A8[0]; p != 0; p = p->next) {
                s16 k = p->fAA;

                if (k == 0x1F4 || k == 0xEA3 || k == 0xEA4 || k == 0x11EB) {
                    p->f98 = 0;
                    p->flags = (p->flags | 0x41) & 0xEFFF;
                }
            }
        }
        break;
    }
}

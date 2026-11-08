/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00413760, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* EE `long` is 64-bit; RULING #13197 spells it u64 in adopted bodies. */
typedef unsigned long u64;

/* RULING #8620 ADDRESSING-MODEL DEVICE (EE arm): section(".sdata") on this
 * extern declaration makes cc1 print the one-instruction `sw` macro, which GNU
 * as expands absolute at -G0 as retail has it. It moves no data and emits
 * nothing. The word lies inside L30's larger label (g_hudClutSlots+0x10 in
 * L30's boot-name map), so it is address-named (RULING #13291 section 2). */
extern int D_L30_001B28E8 __attribute__((section(".sdata")));
extern short D_18A000;
extern u64 D_138180[];
extern unsigned char D_L30_001B3BB0[];
extern void *D_L30_00260CB0[];
extern int D_L30_00260DB0[][2];
extern void func_L30_002EE448(void *, void *);
extern void func_L30_00325068(void *, int, int);
extern void func_L30_00413C18(char *);
extern void func_L30_00317E40(char *);

/**
 * func_L30_00413760 - per-frame update of the level's value-editing menu
 * (an on-screen tuning list).
 * @obj: the menu moby; its state block (ints: selection, count, scroll top)
 *       is at +0x68.
 *
 * Marks the menu live (D_L30_001B28E8 = 1, D_18A000 = 5) and queues its draw
 * hook func_L30_00413C18 through func_L30_002EE448. Either button of the pad
 * word's 0x9000000000 mask (the 64-bit word at D_138180 +0x1A0) closes the
 * menu (func_L30_00317E40) and returns. Otherwise the up/down bits of the
 * word at +0x1A4 move the selection through func_L30_00325068 and scroll the
 * window, and the left/right bits at +0x1A0 make a step of 1 (5 when a low
 * nibble button is held too), negative for left. At most every 5 frames
 * (frame halfword +0x96) the step is added to the selected entry: the pointer
 * D_L30_00260CB0[sel] is read as int, float (step * 0.01), byte or short by
 * type D_L30_001B3BB0[sel] and clamped to D_L30_00260DB0[sel][0..1].
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0040E660.c, function
 * func_L30_004130E8 (v1.01 address), author commit 93d2ec41 (bigbeno37;
 * `git blame -M -C` gives all 59 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; each name to
 * the target of the v2.00 disc's own relocation at this place (task #2673
 * relverify by name); edits:
 *   - func_L30_004130E8 -> func_L30_00413760 (our split name);
 *   - `extern unsigned long long D_00138180[]` -> `extern u64 D_138180[]`,
 *     with `typedef unsigned long u64` here (RULING #13197, NOTE #13281's
 *     `long long`->`long` device): on this arm the upstream spelling fails in
 *     cc1 with "unsupported wide integer operation" (measured on the file,
 *     task #2673); EE `long` is the 64-bit word the ROM's `ld` reads;
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_L30_001B28E8 declaration (RULING #8620, above);
 *   - D_L30_001B2A68 -> D_L30_001B28E8, D_L30_001B3D30 -> D_L30_001B3BB0,
 *     D_L30_00260E30 -> D_L30_00260CB0, D_L30_00260F30 -> D_L30_00260DB0
 *     (the v2.00 targets, each inside a larger L30 label, so address-named,
 *     RULING #13291 section 2);
 *   - D_00138180 -> D_138180 and D_0018A000 -> D_18A000 (the boot's own
 *     operand spellings of those core addresses);
 *   - func_2EDB20 -> func_L30_002EE448, func_323FB0 -> func_L30_00325068,
 *     func_316DB8 -> func_L30_00317E40, func_L30_004135A0 ->
 *     func_L30_00413C18 (L30's functions at their v2.00 places);
 *   - `#include "include_asm.h"` and the file's other declarations dropped.
 */
void func_L30_00413760(char *obj) {
    int *st = *(int **)(obj + 0x68);
    int step = 0;
    D_L30_001B28E8 = 1;
    D_18A000 = 5;
    func_L30_002EE448(func_L30_00413C18, obj);
    if (D_138180[0x1A0 / 8] & 0x9000000000ULL) {
        func_L30_00317E40(obj);
        return;
    }
    if (((int *)D_138180)[0x1A4 / 4] & 0x4000) {
        func_L30_00325068(st, st[1] - 1, 1);
        if (st[2] < st[0] - 15) st[2]++;
    } else if (((int *)D_138180)[0x1A4 / 4] & 0x1000) {
        func_L30_00325068(st, 0, 1);
        if (st[0] < st[2]) st[2]--;
    } else if (((int *)D_138180)[0x1A0 / 4] & 0x2000) {
        step = (((int *)D_138180)[0x1A0 / 4] & 15) ? 5 : 1;
    } else if (((int *)D_138180)[0x1A0 / 4] & 0x8000) {
        step = (((int *)D_138180)[0x1A0 / 4] & 15) ? -5 : -1;
    }
    if (step && *(unsigned short *)(obj + 0x96) >= 5) {
        *(unsigned short *)(obj + 0x96) = 0;
        switch (D_L30_001B3BB0[st[0]]) {
        case 0: {
            int value = *(int *)D_L30_00260CB0[st[0]];
            value += step;
            if (value > D_L30_00260DB0[st[0]][1]) value = D_L30_00260DB0[st[0]][1];
            else if (value < D_L30_00260DB0[st[0]][0]) value = D_L30_00260DB0[st[0]][0];
            *(int *)D_L30_00260CB0[st[0]] = value;
            break;
        }
        case 1: {
            float value = *(float *)D_L30_00260CB0[st[0]] + step * 0.01f;
            if (value > D_L30_00260DB0[st[0]][1]) value = D_L30_00260DB0[st[0]][1];
            else if (value < D_L30_00260DB0[st[0]][0]) value = D_L30_00260DB0[st[0]][0];
            *(float *)D_L30_00260CB0[st[0]] = value;
            break;
        }
        case 2: {
            unsigned char value = *(unsigned char *)D_L30_00260CB0[st[0]];
            value += step;
            if (value > D_L30_00260DB0[st[0]][1]) value = D_L30_00260DB0[st[0]][1];
            else if (value < D_L30_00260DB0[st[0]][0]) value = D_L30_00260DB0[st[0]][0];
            *(unsigned char *)D_L30_00260CB0[st[0]] = value;
            break;
        }
        case 3: {
            short value = *(short *)D_L30_00260CB0[st[0]];
            value += step;
            if (value > D_L30_00260DB0[st[0]][1]) value = D_L30_00260DB0[st[0]][1];
            else if (value < D_L30_00260DB0[st[0]][0]) value = D_L30_00260DB0[st[0]][0];
            *(short *)D_L30_00260CB0[st[0]] = value;
            break;
        }
        }
    }
    (*(unsigned short *)(obj + 0x96))++;
}

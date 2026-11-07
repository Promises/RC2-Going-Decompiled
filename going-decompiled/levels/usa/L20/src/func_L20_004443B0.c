/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x004443B0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;

extern u16 *D_L20_001FBAA0[];
/* RULING #8620 addressing device (level code has no native arm): upstream's
 * `__attribute__((sda))` is written `section(".sdata")` on these extern
 * DECLARATIONS. It moves no data and emits nothing. cc1 then prints the entity
 * bank's load as the one-instruction `lw $r,SYM` macro, which GNU as expands
 * to an adjacent lui/lw pair (absolute: the bank has no size, so it is not
 * $gp-relative even at -G8); without it cc1 splits %hi/%lo and schedules them
 * apart. The 4-byte bound is $gp-relative by its size either way, and the
 * relocation types are the same in both. Ablation (task #2565): with
 * upstream's `sda` (ignored by this cc1) the function differs from the disc in
 * 2 of 50 words (masked compare). */
extern char *D_L20_001B2F1C[] __attribute__((section(".sdata")));
extern int D_L20_001B2F38 __attribute__((section(".sdata")));
extern void func_L20_003BB6F0(char *);

/**
 * func_L20_004443B0 - reset the entities of a moby's group and run their setup.
 *
 * For every entity of the object's group (state word 0x218; id list
 * D_L20_001FBAA0[group], ids masked with 0x7FFF, 0x100-byte records at
 * D_L20_001B2F1C), sets its halfword +0x36 to 0x7F80 and calls
 * func_L20_003BB6F0 on it.
 *
 * @a0: the moby; its +0x68 points at the state.
 * Return: nothing.
 *
 * Non-obvious: the loop is the usual group walk with its doubled p test: the
 * second test leaves the exit test at the bottom, which gives retail's `b
 * test` entry (upstream's note).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_00442E68-g8.c, function func_L20_00442E68 (v1.01
 * address), author commit 7b31e91f (Fromnowhere). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_00442E68 -> func_L20_004443B0; D_L20_001B2F9C ->
 *     D_L20_001B2F1C; D_L20_001B2FB8 -> D_L20_001B2F38; D_L20_001FBB20 ->
 *     D_L20_001FBAA0; func_L02_003A6F78 -> func_L20_003BB6F0
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))` on
 *     the extern declarations (RULING #8620 addressing device, comment above
 *     them)
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 *   - upstream's comment folded into this one
 */
void func_L20_004443B0(char *a0) {
    char *state = *(char **)(a0 + 0x68);
    u16 *p;
    char *e;

    for (p = D_L20_001FBAA0[*(s32 *)(state + 0x218)];
         (*(s32 *)(state + 0x218) & 0xFF) != 0xFF && *(s32 *)(state + 0x218) <= D_L20_001B2F38 &&
         p != 0 &&
         ((p != 0 && (s16)p[-1] >= 0) || p == D_L20_001FBAA0[*(s32 *)(state + 0x218)]);
         p++) {
        e = D_L20_001B2F1C[0] + ((*p & 0x7FFF) << 8);
        if (e == 0)
            break;
        *(u16 *)(e + 0x36) = 0x7F80;
        func_L20_003BB6F0(e);
    }
}

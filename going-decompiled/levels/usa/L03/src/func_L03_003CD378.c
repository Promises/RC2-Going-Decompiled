/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003CD378, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* Upstream include/common.h's own definitions of the typedefs the body uses. */
typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

/* L03's copies of boot helpers, at their v2.00 places. */
extern s32  func_L03_00361DC8(s32 a0, s32 a1, char *a2);
extern void func_L03_0031CC98(char *a0, s32 a1, s32 a2);
extern f32  func_L03_0031CE60(f32 a0, f32 a1);
extern f32  func_L03_002F0E60(f32 a0, f32 a1);
/* The boot's table at 0x00188660 (core, below 0x1A7C80), indexed here in
 * 0x70-byte entries. */
extern char g_listenerPosHistory[];

/**
 * func_L03_003CD378 - pick the actor's next motion slot.
 *
 * @a0: the actor; its private block pointer is at +0x68, its sub-state byte
 *      at +0x20, its heading at +0xF8.
 * Return: nothing.
 *
 * Upstream's note: case 0 draws a random turn rate, case 1 re-checks the
 * 0x70-stride table entry before refreshing the aim. In detail:
 *   - state 0: switch to state 1 (func_L03_0031CC98(a0, 1, -1)), store
 *     func_L03_0031CE60(60, 90) * 0.000290888245 (degrees per second scaled
 *     to radians per frame) at block+4, and clear the slot index block+8 to -1;
 *   - state 1: when block+0 is set, the entry at block+8 in
 *     g_listenerPosHistory's 0x70-byte table is re-taken
 *     (func_L03_00361DC8(0, 4, a0)) if its owner at +0x88 is no longer this
 *     actor or its byte +0x74 is 0; then the heading at +0xF8 is advanced by
 *     block+4 (func_L03_002F0E60).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_00399230.c, function
 * func_L03_003CBD70 (v1.01 address), author commit 431ac503 (llesieur99;
 * `git blame -M -C` names it for all 26 definition lines at fe6adf9b, and
 * `git log --reverse -S'func_L03_003CBD70('` agrees). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_003CBD70 -> func_L03_003CD378 (our split name at the
 *     v2.00 place);
 *   - D_L03_00188660 -> g_listenerPosHistory (the boot's name at that
 *     address, symbol_addrs.txt and L03's boot-name map);
 *   - four callees as L03's copies, the v2.00 ROM's call targets:
 *     func_360A80 -> func_L03_00361DC8, func_31B920 -> func_L03_0031CC98,
 *     func_31BAE8 -> func_L03_0031CE60, func_2F0000 -> func_L03_002F0E60;
 *   - `#include "common.h"` dropped, its three typedefs written out;
 *     upstream's comment folded into this one.
 */
void func_L03_003CD378(char *a0)
{
    char *s0 = a0;
    char *s1 = *(char **)(s0 + 0x68);

    switch (*(u8 *)(s0 + 0x20)) {
    case 0:
        func_L03_0031CC98(s0, 1, -1);
        *(f32 *)(s1 + 4) = func_L03_0031CE60(60.0f, 90.0f) * 0.000290888245f;
        *(s32 *)(s1 + 8) = -1;
        break;
    case 1: {
        s32 v = *(s32 *)s1;

        if (v != 0) {
            char *p = g_listenerPosHistory + *(s32 *)(s1 + 8) * 0x70;

            if (*(char **)(p + 0x88) != s0 || *(u8 *)(p + 0x74) == 0)
                *(s32 *)(s1 + 8) = func_L03_00361DC8(0, 4, s0);
        }
        *(f32 *)(s0 + 0xF8) = func_L03_002F0E60(*(f32 *)(s0 + 0xF8),
            *(f32 *)(s1 + 4));
        break;
    }
    }
}

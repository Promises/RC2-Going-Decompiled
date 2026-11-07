/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x00428430, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack (upstream
 * builds its file at -G8; the audit's arm for this member is s136).
 */

extern float D_L14_001B1758 __attribute__((sda));
extern float D_L14_001B175C __attribute__((sda));

extern char g_heroPos[];

extern void *func_L14_002DEDE8(int size);

extern void func_L14_002DEE88(void *block);

extern int func_L14_00329290(void *, int);

typedef struct {
    float x;
    float y;
} EffectPair;

/**
 * func_L14_00428430 - keep a two-float pool record alive while any of the
 * owner's four trigger slots points at the hero's zone.
 *
 * @a0: the object; its state block pointer is at +0x68.
 * Return: nothing.
 *
 * Non-obvious: the four slot words at state +0x20 are walked with an
 * incrementing pointer. At the first slot that is set (not -1) and passes
 * func_L14_00329290 against g_heroPos (the hero position vec4, our boot name
 * at 0x189EA0): the pool record at state +0x14 is allocated (5 words) if
 * missing and its two floats set from the level's tuning pair, then the walk
 * returns. If no slot passes, a live record is freed (func_L14_002DEE88) and
 * the pointer cleared. `__attribute__((sda))` is upstream's own text; this
 * cc1 warns about it and the member closes with it as written.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98-g8.c,
 * function func_L14_00426E60 (v1.01 address), author commit 8422a726
 * (Fromnowhere; the definition line blames 8422a726 too). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_00426E60 -> func_L14_00428430 (our split name at the
 *     v2.00 place);
 *   - D_L14_001B17B8 -> D_L14_001B1758 and D_L14_001B17BC -> D_L14_001B175C
 *     (our labels at the v2.00 addresses the relocations imply);
 *   - D_00189EA0 -> g_heroPos (our boot name at that address; L14
 *     core_externs.txt line 243);
 *   - func_L00_002D70A0 -> func_L14_002DEDE8 and func_L00_002D7140 ->
 *     func_L14_002DEE88 (the level's own copies at their L14 places);
 *   - func_002ACBC8 -> func_L14_00329290 (the level's own copy of the boot
 *     function at its L14 place);
 *   - `#include "common.h"` dropped (the typedef EffectPair is upstream's
 *     own text, kept in upstream's order after the externs; the rest of
 *     common.h is unused); the file's other functions and their externs not
 *     carried.
 */
void func_L14_00428430(char *a0)
{
    char *state = *(char **)(a0 + 0x68);
    int *slot = (int *)(state + 0x20);
    int i;

    for (i = 0; i < 4; i++, slot++) {
        if (*slot != -1 && func_L14_00329290(g_heroPos, *slot) != 0) {
            if (*(EffectPair **)(state + 0x14) == 0) {
                *(EffectPair **)(state + 0x14) = func_L14_002DEDE8(5);
            }
            (*(EffectPair **)(state + 0x14))->x = D_L14_001B1758;
            (*(EffectPair **)(state + 0x14))->y = D_L14_001B175C;
            return;
        }
    }

    if (*(EffectPair **)(state + 0x14) != 0) {
        func_L14_002DEE88(*(EffectPair **)(state + 0x14));
        *(EffectPair **)(state + 0x14) = 0;
    }
}

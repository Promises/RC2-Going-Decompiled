/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0044AAC0, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 102 of
 * 123 .text words differ from the disc, relocated fields masked, and .text is
 * 0x1EC bytes, not the place's 0x208, and it emits a .lit4 pool of 8 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

extern float func_L11_00323BF8(char *);
extern void func_L11_00324B40(char *, int, void *);
extern void func_L11_0032F550(void *, void *, void *, void *);
extern void func_L11_00301210(void *, void *, void *);
extern float func_L11_00301340(void *);
extern int func_L11_0032F698(char *, void *, int, float, float, float, int, int, int);
extern int func_L11_00374EB0(int, int, char *);

/**
 * func_L11_0044AAC0 - sweep the actor's beam between its two joints during
 * frames 19 to 31 and hit what is near it.
 *
 * @object: the actor (moby); +0x68 its state block: +0x34 the damage, +0x350 the
 *          target record, +0x368 the once-only sound flag.
 * Return: none.
 *
 * Non-obvious: The animation time comes from func_L11_00323BF8 and the joints
 * 0 and 1 from func_L11_00324B40. It finds the beam's point closest to the
 * target (func_L11_0032F550). The hit strength there is 0.15 + 0.35 * f, where
 * f is that point's distance to joint 1 over the joints' distance, clamped to
 * [0, 1]. The first hit that lands plays sound 5 once (func_L11_00374EB0). A
 * second hit of strength 0.5 is made at joint 1.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_004465F8.c (blob 4b23acda), function
 * func_L11_00449098 (v1.01 address), author commit 5acf9869 (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED: built at -G8 where upstream builds the file at -G0 (its stem has no
 * -g8; a tsv gflag setting, not a text edit; measured at -G0, task #2578:
 * .text 0x1FC bytes, not 0x208 (cobj refuses)); the text is verbatim but for
 * names. Respellings, each to the target of the v2.00 disc's own relocation at
 * this place (relverify by name on this object, task #2578):
 *   - func_L11_00449098 -> func_L11_0044AAC0 (our split name at the v2.00
 *     place);
 *   - func_00282CF0 -> func_L11_00301210 (the level's own copy at its L11
 *     place);
 *   - func_00282DF0 -> func_L11_00301340 (the level's own copy at its L11
 *     place);
 *   - func_0029F530 -> func_L11_00323BF8 (the level's own copy at its L11
 *     place);
 *   - func_0029FCC0 -> func_L11_00324B40 (the level's own copy at its L11
 *     place);
 *   - func_002A8C28 -> func_L11_0032F698 (the level's own copy at its L11
 *     place);
 *   - func_002E5CF8 -> func_L11_00374EB0 (the level's own copy at its L11
 *     place);
 *   - func_L00_0031BDC0 -> func_L11_0032F550 (the level's own copy at its L11
 *     place);
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_0044AAC0(char *object)
{
    float start[4], end[4], closest[4], remaining[4], span[4];
    char *state;
    float time, fraction;

    time = func_L11_00323BF8(object);
    state = *(char **)(object + 0x68);
    if (19.0f <= time && time <= 31.0f) {
        func_L11_00324B40(object, 0, start);
        func_L11_00324B40(object, 1, end);
        func_L11_0032F550(closest, start, end,
                         *(char **)(state + 0x350) + 0x10);
        func_L11_00301210(remaining, closest, end);
        func_L11_00301210(span, start, end);
        fraction = func_L11_00301340(remaining);
        fraction /= func_L11_00301340(span);
        if (fraction < 0.0f)
            fraction = 0.0f;
        if (fraction > 1.0f)
            fraction = 1.0f;
        if (func_L11_0032F698(object, closest, 0x10001,
                         fraction * 0.35f + 0.15f,
                         (float)*(unsigned char *)(state + 0x34), 1.0f, 0, 1, 0)
            && *(short *)(state + 0x368) == 0) {
            func_L11_00374EB0(5, 0, object);
            *(short *)(state + 0x368) = 1;
        }
        func_L11_0032F698(object, end, 0x10001, 0.5f,
                      (float)*(unsigned char *)(state + 0x34), 1.0f, 0, 1, 0);
    }
}

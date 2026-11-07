/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F2298, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

extern void func_L14_002ED778(float);
extern int func_L14_003336B8(char *);
extern void func_L14_002EF2A0(void (*f)(char *), char *arg);
extern float func_L14_003233B8(float, float);
extern void func_L14_003F23B0(char *);

/**
 * func_L14_003F2298 - drift an object and queue its wobble callback.
 *
 * @arg0: the object; its state block pointer is at +0x68, its stage byte
 *        +0x20.
 * Return: nothing.
 *
 * Non-obvious: stage 0 arms the wobble (state +0x10 = 1, sets the colour
 * period with func_L14_002ED778(2048) and starts stage 1). Each tick two
 * drift counters (+0x0, +0x8) advance by their own rates and wrap at 8.0;
 * while func_L14_003336B8 says the hero is close the wobble is disarmed.
 * While armed the object is handed to func_L14_002EF2A0 with
 * func_L14_003F23B0 (the level's own copy, passed by address). Both drift
 * rates are then redrawn with func_L14_003233B8.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c, function
 * func_L14_003F0D50 (v1.01 address), author commit daf0e4a1 (Flavius; the
 * definition line blames daf0e4a1 too). Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for
 * names; edits:
 *   - name func_L14_003F0D50 -> func_L14_003F2298 (our split name at the
 *     v2.00 place);
 *   - func_L00_002E54E0 -> func_L14_002ED778 (the level's own copy at its
 *     L14 place);
 *   - func_002B38B8 -> func_L14_003336B8 (the level's own copy of the boot
 *     function at its L14 place);
 *   - func_L00_002E6EF8 -> func_L14_002EF2A0 (the level's own copy at its
 *     L14 place);
 *   - func_002A7878 -> func_L14_003233B8 (the level's own copy of the boot
 *     function at its L14 place);
 *   - func_L14_003F0E68 -> func_L14_003F23B0 (the level's own copy at its
 *     L14 place; its address is taken as the callback argument);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing else here uses them); upstream's declaration forms
 *     kept as written.
 */
void func_L14_003F2298(char *arg0) {
    char *s2 = arg0;
    char *s1 = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x20) == 0) {
        *(int *)(s1 + 0x10) = 1;
        func_L14_002ED778(2048.0f);
        *(unsigned char *)(arg0 + 0x20) = 1;
    }
    *(float *)(s1 + 0x0) = *(float *)(s1 + 0x0) + *(float *)(s1 + 0x4);
    *(float *)(s1 + 0x8) = *(float *)(s1 + 0x8) + *(float *)(s1 + 0xC);
    if (*(float *)(s1 + 0x0) >= 8.0f) {
        *(float *)(s1 + 0x0) = *(float *)(s1 + 0x0) - 8.0f;
    }
    if (*(float *)(s1 + 0x8) >= 8.0f) {
        *(float *)(s1 + 0x8) = *(float *)(s1 + 0x8) - 8.0f;
    }
    if (func_L14_003336B8(s2) != 0) {
        *(int *)(s1 + 0x10) = 0;
    }
    if (*(int *)(s1 + 0x10) != 0) {
        func_L14_002EF2A0(func_L14_003F23B0, s2);
    }
    *(float *)(s1 + 0x4) = func_L14_003233B8(0.003f, 0.005f);
    *(float *)(s1 + 0xC) = func_L14_003233B8(0.001f, 0.0025f);
}

/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E32E8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

extern void func_L14_00323250();
extern float func_L14_002F7A68(float a, float b);
extern float func_L14_002F7028(float a);

/**
 * func_L14_003E32E8 - move a rising/falling object towards its target height.
 *
 * @arg0: the object; its state block pointer is at +0x68, its stage byte
 *        +0x20 and its height +0x18.
 * Return: nothing.
 *
 * Non-obvious: stage 0 records the object's height in state +0x04, clears
 * state +0x08 and starts stage 1 (func_L14_00323250). Stage 1 eases both the
 * object's height (+0xF8) and state +0x08 towards the block's own values with
 * func_L14_002F7A68, then sets the height to state +0x04 plus the eased
 * amount scaled by state +0x10 and put through func_L14_002F7028.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c, function
 * func_L14_003E1DD0 (v1.01 address), author commit adcbd51e (llesieur99; the
 * definition line blames adcbd51e too). Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for
 * names; edits:
 *   - name func_L14_003E1DD0 -> func_L14_003E32E8 (our split name at the
 *     v2.00 place);
 *   - func_321F58 -> func_L14_00323250 (the level's own copy of boot
 *     func_002A7750 at its L14 place);
 *   - func_2F6C80 -> func_L14_002F7A68 (the level's own copy of boot
 *     func_00283B98 at its L14 place);
 *   - func_2F6240 -> func_L14_002F7028 (the level's own copy of boot
 *     func_00283198 at its L14 place);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing else here uses them); upstream's declaration forms
 *     kept as written.
 */
void func_L14_003E32E8(char *arg0) {
    char *s0 = *(char **)(arg0 + 0x68);
    unsigned char st = *(unsigned char *)(arg0 + 0x20);

    switch (st) {
    case 0:
        *(float *)(s0 + 0x4) = *(float *)(arg0 + 0x18);
        *(int *)(s0 + 0x8) = 0;
        func_L14_00323250(arg0, 1, -1);
        break;
    case 1:
        *(float *)(arg0 + 0xF8) = func_L14_002F7A68(*(float *)(arg0 + 0xF8), *(float *)(s0 + 0x0));
        *(float *)(s0 + 0x8) = func_L14_002F7A68(*(float *)(s0 + 0x8), *(float *)(s0 + 0xC));
        *(float *)(arg0 + 0x18) = *(float *)(s0 + 0x4)
                                  + *(float *)(s0 + 0x10) * func_L14_002F7028(*(float *)(s0 + 0x8));
        break;
    }
}

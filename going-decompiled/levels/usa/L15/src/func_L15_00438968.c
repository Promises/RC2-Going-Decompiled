/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00438968, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/**
 * func_L15_00438968 - initialise a path mover.
 *
 * @a0: the object; its state block is at +0x68.
 * Return: nothing.
 *
 * Sets the owner record's +0x148 to 100 and its +0x154 to the address of
 * the state's +0x278 (set to 10.0), the state's +0x180 to 100.0, the
 * object to state 1 with scale 0.2, and clears the path index and timers.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_004328E8.c, function func_L15_00436BA8 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_00436BA8 -> func_L15_00438968 (our split name at the v2.00 place);
 *   - func_002ED688 -> func_L15_00381760 (the v2.00 relocation target, our name there);
 *   - func_L05_00379C68 -> func_L15_00381700 (the v2.00 relocation target, our name there);
 *   - func_L05_0037AEC0 -> func_L15_00382980 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
extern void func_L15_00438968(char *);
extern int func_L15_00381760(unsigned char *owner);
extern void func_L15_00382980(char *object, int flag);
extern void func_L15_00381700(int arg0, int value);

void func_L15_00438968(char *a0) {
    char *s0 = a0;
    char *s1 = *(char **)(s0 + 0x68);
    char *s2 = (char *)func_L15_00381760((unsigned char *)a0);

    *(int *)(s2 + 0x148) = 100;
    func_L15_00382980(s0, 1);
    *(float *)(s1 + 0x180) = 100.0f;
    *(unsigned char *)(s0 + 0x20) = 1;
    *(unsigned char *)(s0 + 0x30) = 0xFF;
    *(unsigned short *)(s0 + 0x32) = 0x1FF;
    *(unsigned short *)(s0 + 0x34) |= 0x100;
    *(float *)(s0 + 0x2C) = 0.20000000298023224f;
    *(float *)(s1 + 0x278) = 10.0f;
    *(int *)(s1 + 0x26C) = 0;
    *(int *)(s1 + 0x270) = 0;
    *(int *)(s1 + 0x274) = 0;
    *(int *)(s2 + 0x154) = (int)(s1 + 0x278);
    func_L15_00381700((int)s0, 0xFFFF);
}

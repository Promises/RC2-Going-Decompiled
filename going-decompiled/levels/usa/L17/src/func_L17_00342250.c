/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00342250, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern int func_L17_002EF908(char *);
extern int func_L17_002F0EA0(float);
extern float func_L17_002F02C0(float);
extern float func_L17_002EFBD8(float);
extern float func_L17_002F0E90(int);
extern void func_L17_003341F8(char *);

/**
 * func_L17_00342250 - pulse an object's colour byte from its timer ratio.
 *
 * @s1: the object, or NULL (then nothing happens); its sub-block is at +0x20.
 * Return: nothing.
 *
 * When func_L17_002EF908 on +0x24 reports non-zero, the object goes to
 * func_L17_003341F8 and nothing else happens. Otherwise the ratio of the
 * sub-block halfwords +4 and +6 (each through func_L17_002F0E90) is scaled
 * by 6.283186 (2*pi) and passed to func_L17_002F02C0; the word +0xC of the
 * sub-block (func_L17_002F0E90) times that result goes through
 * func_L17_002EFBD8 and func_L17_002F0EA0 into the top byte of the object's
 * word +4, whose low 24 bits are kept.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_0032C070.c, function func_L17_00340EA0 (v1.01 address), author commit 91e04732 (llesieur99); 5030340d (Flavius) later changed only one
 * callee's name on one line of it, so it is not credited.
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_00340EA0 -> func_L17_00342250 (our split name at the v2.00 place);
 *   - func_00282C48 -> func_L17_002EFBD8 (the v2.00 relocation target, our name there);
 *   - func_2EEB28 -> func_L17_002EF908 (the v2.00 relocation target, our name there);
 *   - func_2EF4E0 -> func_L17_002F02C0 (the v2.00 relocation target, our name there);
 *   - func_2F00B0 -> func_L17_002F0E90 (the v2.00 relocation target, our name there);
 *   - func_2F00C0 -> func_L17_002F0EA0 (the v2.00 relocation target, our name there);
 *   - func_332E48 -> func_L17_003341F8 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L17_00342250(char *s1)
{
    float f20;
    float t;
    char *s0;

    if (s1 == 0)
        return;
    s0 = s1 + 0x20;
    if (func_L17_002EF908(s1 + 0x24) != 0) {
        func_L17_003341F8(s1);
        return;
    }
    f20 = func_L17_002F0E90(*(short *)(s0 + 4)) / func_L17_002F0E90(*(short *)(s0 + 6));
    t = func_L17_002F02C0(f20 * 6.283186f);
    *(int *)(s1 + 4) = (*(int *)(s1 + 4) & 0x00FFFFFF)
                       + (func_L17_002F0EA0(func_L17_002EFBD8(func_L17_002F0E90(*(int *)(s0 + 0xC)) * t)) << 24);
}

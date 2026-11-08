/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00438058, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed int s32;

extern int func_L15_00381760(unsigned char *owner);
extern s32 func_L15_003792C8(s32 a, s32 b, void *c);
extern int func_L15_003345A8(int, int);
extern void func_L15_00381A60(void *, int, int, float, int, float, float, float,
                              int, int, int);

/**
 * func_L15_00438058 - fire the object's two shots when its record is ready
 * and its own counter is idle.
 *
 * @a0: the object; its state block is at +0x68.
 * Return: nothing.
 *
 * Needs the record from func_L15_00381760 with a non-zero +0x120 and the
 * state's +0x450 at 0. It calls func_L15_003792C8(2, 0, a0) and fires two
 * shots through func_L15_00381A60 from slots counter * 2 + 4 and + 5, sets +0x450
 * to a random 0x14..0x3C and advances the counter +0x454, wrapping at 3.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_004328E8.c, function func_L15_00436298 (v1.01 address), author commit daf0e4a1 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_00436298 -> func_L15_00438058 (our split name at the v2.00 place);
 *   - func_002A7820 -> func_L15_003345A8 (the v2.00 relocation target, our name there);
 *   - func_002E5CF8 -> func_L15_003792C8 (the v2.00 relocation target, our name there);
 *   - func_002ED688 -> func_L15_00381760 (the v2.00 relocation target, our name there);
 *   - func_L05_00379FA0 -> func_L15_00381A60 (the v2.00 relocation target, our name there);
 *   - upstream's file declares func_L15_003792C8 again further down; the repeat is
 *   dropped, the first declaration kept;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L15_00438058(char *a0) {
    char *s1 = a0;
    char *s0;
    char *v0;
    int v1;

    s0 = *(char **)(s1 + 0x68);
    v0 = (char *)func_L15_00381760(a0);
    if (v0 == 0) {
        return;
    }
    if (*(int *)(v0 + 0x120) == 0) {
        return;
    }
    if (*(int *)(s0 + 0x450) != 0) {
        return;
    }
    func_L15_003792C8(2, 0, s1);
    func_L15_00381A60(s1, 0x600000CF, 1, 1.0f, 0, 2.0f, 1.5f, 1.5f, 0x1E,
                     *(int *)(s0 + 0x454) * 2 + 4, 0);
    func_L15_00381A60(s1, 0x600000CF, 1, 1.0f, 0, 2.0f, 1.5f, 1.5f, 0x1E,
                     *(int *)(s0 + 0x454) * 2 + 5, 0);
    *(int *)(s0 + 0x450) = func_L15_003345A8(0x14, 0x3C);
    v1 = *(int *)(s0 + 0x454) + 1;
    *(int *)(s0 + 0x454) = v1;
    if (v1 >= 3) {
        *(int *)(s0 + 0x454) = 0;
    }
}

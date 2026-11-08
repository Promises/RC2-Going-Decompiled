/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00431AD8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack (upstream builds its file at -G8), plain as.
 */

extern int D_L15_001B14D0 __attribute__((sda));
extern char *func_L15_00336828(char *a0, int a1, int a2);
extern void func_L15_00338D28(char *a0, char *a1);
extern void func_L15_00338C68(char *a0, char *a1);
extern int func_L15_00379370(int index, int value, char *object, int kind);
extern int func_L15_004296B8(void *a0, void *a1, int a2, int a3, int a4, int a5, int a6, int a7,
                             int a8, int a9, int a10, float f12);
extern void func_L15_00433B38(char *a0, int a1, int a2);
extern int func_L15_003837B0(char *a0);
extern void func_L15_00383A90(char *a0);
extern void func_L15_00382900(char *a0);

/**
 * func_L15_00431AD8 - a moby's death timer: count its stored time down and
 * take the moby down at zero.
 *
 * @obj: the moby; its state block is at +0x68.
 * @flag: when 0, func_L15_00382900(obj) is also called at zero.
 * Return: 1 when the timer reached zero this call, else 0.
 *
 * It sets byte +0xA8 to 0xFF and passes the state's +0x10 entry, if any, to
 * func_L15_00338D28. When func_L15_00336828(obj, 0x230000, 0) returns a hit,
 * that entry (if any) gets byte +7 = 0xFF and goes to func_L15_00338C68, the
 * float the state's first word points to is reduced by the hit's +0x2C, and
 * func_L15_00379370 is called with (5, 0, obj, 0x1211) for class (+0xAA)
 * 0x1211, (0xD, 0, obj, 0xFBF) for any class but 0xB73. At or below 0.0 the
 * time is clamped to 0.0, func_L15_004296B8 fires at obj + 0x10,
 * func_L15_00433B38(obj, D_L15_001B14D0 == 0, 0xF) runs, and
 * func_L15_00383A90(obj) follows when func_L15_003837B0(obj) is non-zero.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_00315428-g8.c, function func_L15_0042FFB8 (v1.01 address), author commit 7ac08f9b (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_0042FFB8 -> func_L15_00431AD8 (our split name at the v2.00 place);
 *   - D_L15_001B1520 -> D_L15_001B14D0 (our split label at the v2.00 address);
 *   - func_002A9640 -> func_L15_00336828 (the v2.00 relocation target, our name there);
 *   - func_002AB800 -> func_L15_00338C68 (the v2.00 relocation target, our name there);
 *   - func_002AB8C0 -> func_L15_00338D28 (the v2.00 relocation target, our name there);
 *   - func_002E5DA0 -> func_L15_00379370 (the v2.00 relocation target, our name there);
 *   - func_L05_0037AE40 -> func_L15_00382900 (the v2.00 relocation target, our name there);
 *   - func_L05_0037BBB8 -> func_L15_003837B0 (the v2.00 relocation target, our name there);
 *   - func_L05_0037BE98 -> func_L15_00383A90 (the v2.00 relocation target, our name there);
 *   - func_L05_00421908 -> func_L15_00433B38 (the v2.00 relocation target, our name there);
 *   - func_L05_00424990 -> func_L15_004296B8 (the v2.00 relocation target, our name there);
 *   - upstream's `__attribute__((sda))` on D_L15_001B14D0 kept verbatim:
 *   our cc1 ignores it with a warning and, at -G8, places these 4-byte
 *   objects $gp-relative as retail has them, so it is byte-inert here;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
int func_L15_00431AD8(char *obj, int flag)
{
    char *s0;
    float *s2;
    char *s3;
    char *v;
    short v1;
    int ret = 0;

    s3 = func_L15_00336828(obj, 0x230000, 0);
    *(unsigned char *)(obj + 0xA8) = 0xFF;
    v = *(char **)(obj + 0x68);
    s0 = *(char **)(v + 0x10);
    s2 = *(float **)(v + 0x0);
    if (s0 != 0) {
        func_L15_00338D28(obj, s0);
    }
    if (s3 != 0) {
        if (s0 != 0) {
            *(unsigned char *)(s0 + 7) = 0xFF;
            func_L15_00338C68(obj, s0);
        }
        *s2 = *s2 - *(float *)(s3 + 0x2C);
        v1 = *(short *)(obj + 0xAA);
        if (v1 != 0xB73) {
            if (v1 == 0x1211) {
                func_L15_00379370(5, 0, obj, 0x1211);
            } else {
                func_L15_00379370(0xD, 0, obj, 0xFBF);
            }
        }
        if (*s2 <= 0.0f) {
            *s2 = 0.0f;
            func_L15_004296B8(obj + 0x10, 0, 0x32, 0x1E, 0x1E, 0x1E, 0x28, 1, 1, 0, 0, 0.75f);
            if (D_L15_001B14D0 == 0) {
                func_L15_00433B38(obj, 1, 0xF);
            } else {
                func_L15_00433B38(obj, 0, 0xF);
            }
            if (func_L15_003837B0(obj)) {
                func_L15_00383A90(obj);
            }
            if (flag == 0) {
                func_L15_00382900(obj);
            }
            ret = 1;
        }
    }
    return ret;
}

/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F2E58, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L02's copies of boot helpers and L02 functions, at their v2.00 places. */
extern void func_L02_0031F998(char *, int, int);
extern void func_L02_0031F6B8(char *, int, int, int);
extern void func_L02_003174E8(char *);
extern void func_L02_0031FD50(char *, float, float);
extern void func_L02_0032C360(char *, int, char *, float, float, float, float, int);
extern void func_L02_002F2D58(void *, void *, void *);
extern void func_L02_003FD750(int, int);
extern void func_L02_00420A08(char *, int, int, int, int, int, int, int, float, float, int, int, int, int);
extern void func_L02_00367EA0(int, int, char *, int);

/**
 * func_L02_003F2E58 - take the actor out of play, with its break-up effect.
 *
 * @arg0: the actor; its state block is at +0x68.
 *
 * A claimed timer (+0x4EA == 1) is handed back through
 * func_L02_003FD750(object +0xB8, 0xB22); the claim, +0x4B0 and +0x7A are
 * cleared and +0x2E set to 1. A visible actor (+0x31) spawns its effect
 * (func_L02_00420A08, func_L02_00367EA0 with 0xC3E, and a random vector from
 * func_L02_0031FD50 added to state +0xE0 by func_L02_002F2D58 and passed to
 * func_L02_0032C360). Both sub-objects at state +0x4C0 are hidden, then the
 * actor itself: func_L02_0031F6B8(arg0, 0, 0, 3), func_L02_003174E8,
 * func_L02_0031F998(arg0, 1, 0), +0x31 and +0x98 cleared, flags
 * (flags & 0xEFFF) | 0x47.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003DB310.c,
 * function func_L02_003F18E8 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003F18E8 -> func_L02_003F2E58 (our split name at
 *     the v2.00 place);
 *   - func_31E3C0 -> func_L02_0031F6B8, func_316220 ->
 *     func_L02_003174E8, func_31EA58 -> func_L02_0031FD50, func_32B068
 *     -> func_L02_0032C360, func_2F1F70 -> func_L02_002F2D58,
 *     func_3FC1B0 -> func_L02_003FD750, func_41F230 ->
 *     func_L02_00420A08, func_366BD8 -> func_L02_00367EA0, func_31E6A0
 *     -> func_L02_0031F998 (L02's v2.00 places);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L02_003F2E58(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char buffer[0x10];
    int i;

    if (*(short *)(state + 0x4EA) == 1)
        func_L02_003FD750(*(int *)(arg0 + 0xB8), 0xB22);
    *(short *)(state + 0x4EA) = 0;
    *(int *)(state + 0x4B0) = 0;
    *(unsigned char *)(state + 0x7A) = 0;
    *(unsigned char *)(state + 0x2E) = 1;
    if (*(unsigned char *)(arg0 + 0x31) != 0) {
        func_L02_00420A08(arg0 + 0x10, 0, 0x1E, 5, 5, 5, 0x2D, 1, 0.5f, 0.0f, 1, 0, 0, 0);
        func_L02_00367EA0(0, 0, arg0, 0xC3E);
        func_L02_0031FD50(buffer, 0.033333335071802139f, 0.083333335816860199f);
        func_L02_002F2D58(buffer, buffer, state + 0xE0);
        func_L02_0032C360(arg0, 0x7F, buffer, 3.0f, 6.0f, 3.0f, 6.0f, 0xFF);
    }
    {
        char **slot = (char **)(state + 0x4C0);

        for (i = 1; i >= 0; i--) {
            if (*slot != 0) {
                *(unsigned short *)(*slot + 0x34) |= 1;
                *(unsigned char *)(*slot + 0x31) = 0;
                *(int *)(*slot + 0x98) = 0;
            }
            slot++;
        }
    }
    func_L02_0031F6B8(arg0, 0, 0, 3);
    func_L02_003174E8(arg0);
    func_L02_0031F998(arg0, 1, 0);
    *(unsigned char *)(arg0 + 0x31) = 0;
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned short *)(arg0 + 0x34) =
        (*(unsigned short *)(arg0 + 0x34) & 0xEFFF) | 0x47;
}

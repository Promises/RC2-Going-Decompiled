/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E8B18, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned short u16;

/* L02's copies of boot helpers, at their v2.00 places in this level. */
extern void func_L02_0031F998(char *, int, int);
extern int func_L02_00330440(char *);
extern int func_L02_002F2998(int *);
extern int func_L02_002F3F40(float);
/* The wanted-entry table: three tracks of nine words, a label of L02's .data. */
extern int D_L02_002A5470[3][9];

/* The actor's state, as far as this function goes: three tracks of nine
 * positions each. */
typedef struct {
    char pad0[0x60];
    float pos[3];
    int timer[3];
    int mode[3];
    float speed[3];
    int want[3];
} State003E75A8;

/**
 * func_L02_003E8B18 - advance the actor's three spinning tracks until each
 * stops on its wanted entry.
 *
 * @arg0: the actor; its state block (State003E75A8) is at +0x68.
 *
 * When func_L02_00330440 agrees, the countdowns are set (100 for the middle
 * track, 50 and 180 for the others, swapped when bit 15 of the actor's +0x34
 * is set) and every mode becomes 1. Mode 1 moves the position back by its
 * speed, wrapping into [0, 9), and becomes mode 2 when func_L02_002F2998 says
 * its countdown is over. Mode 2 moves the same way until the entry
 * D_L02_002A5470[track][(int)(position + 9) % 9] is the wanted one and the
 * position is less than 0.2 above a whole number; the mode is then 0. Once all
 * three modes are 0 the actor changes state (func_L02_0031F998 with 4, -1).
 * The all-zero test runs inside the loop, once per track.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003DB310.c,
 * function func_L02_003E75A8 (v1.01 address), author commit b7374531 (bigmak94).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003E75A8 -> func_L02_003E8B18 (our split name at
 *     the v2.00 place);
 *   - func_002A7750 -> func_L02_0031F998, func_002B3868 ->
 *     func_L02_00330440, func_00282948 -> func_L02_002F2998,
 *     func_00283CF0 -> func_L02_002F3F40: L02's copies;
 *   - D_L02_002A53B0 -> D_L02_002A5470 (the v2.00 label);
 *   - the file's second, unprototyped `extern void func_002A7750();`
 *     not carried (the prototype it composes with is);
 *   - u16 typedef written here (upstream takes it from common.h);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L02_003E8B18(char *arg0) {
    State003E75A8 *st = *(State003E75A8 **)(arg0 + 0x68);
    int i;

    if (func_L02_00330440(arg0)) {
        st->timer[1] = 100;
        st->timer[0] = 50;
        st->timer[2] = 180;
        if (*(u16 *)(arg0 + 0x34) & 0x8000) {
            st->timer[0] = 180;
            st->timer[2] = 50;
        }
        st->mode[0] = 1;
        st->mode[1] = 1;
        st->mode[2] = 1;
    }
    for (i = 0; i < 3; i++) {
        switch (st->mode[i]) {
        case 0:
            break;
        case 1:
            st->pos[i] = st->pos[i] - st->speed[i];
            if (st->pos[i] < 0.0f)
                st->pos[i] += 9.0f;
            if (func_L02_002F2998(&st->timer[i]))
                st->mode[i] = 2;
            break;
        case 2:
            st->pos[i] = st->pos[i] - st->speed[i];
            if (st->pos[i] < 0.0f)
                st->pos[i] += 9.0f;
            if (D_L02_002A5470[i][func_L02_002F3F40(st->pos[i] + 9.0f) % 9] == st->want[i]) {
                if (st->pos[i] - (float)func_L02_002F3F40(st->pos[i]) < 0.2f)
                    st->mode[i] = 0;
            }
            break;
        }
        if (st->mode[0] == 0 && st->mode[1] == 0 && st->mode[2] == 0)
            func_L02_0031F998(arg0, 4, -1);
    }
}

/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00438750, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed int s32;
typedef float f32;

extern void func_L15_0035B558(char *, int, int, int, float, int);
extern int func_L15_003850A0(char *);
extern void func_L15_00384EC0(char *, char *);
extern void func_L15_00384978(char *, int);
extern void func_L15_00383D58(char *);
extern void func_L15_00383130(char *, int, int, int);
typedef struct GornPathPoint {
    float position[3];
    float action;
} GornPathPoint;
typedef struct GornPath {
    int count;
    int pad[3];
    GornPathPoint points[1];
} GornPath;
extern char *D_L15_001FE400[];
extern void func_L15_00438968(char *);
extern void func_L15_00438A28(void);
extern int func_L15_00381760(unsigned char *owner);
extern int func_L15_00307608(void *);
extern int func_L15_00308AE0(float);
extern int func_L15_003812F8(void);
extern void func_L15_00381920(void *, void *, int, int);

/**
 * func_L15_00438750 - move an object along its path, pausing where a point
 * asks for it.
 *
 * @object: the object; its state block is at +0x68.
 * Return: nothing.
 *
 * State 0 waits for func_L15_003812F8, initialises (func_L15_00438968) and
 * may add the func_L15_00438A28 hook. State 1 follows path
 * D_L15_001FE400[state +0x268] (a count, then 16-byte points of x, y, z and
 * an action): at each reached point it moves to the next, ends in state 3 at
 * the last, and an odd action pauses it in state 2 for (action >> 1) * 60
 * frames. State 2 waits for the timer at state +0x274.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_004328E8.c, function func_L15_00436990 (v1.01 address), author commit b7357856 (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_00436990 -> func_L15_00438750 (our split name at the v2.00 place);
 *   - D_L15_001FE480 -> D_L15_001FE400: an address-named extern (RULING #13291 section 2) at
 *   0x001FE400, inside the split label D_L15_001F0000 (+0xE400); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L15_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2677);
 *   - func_00282948 -> func_L15_00307608 (the v2.00 relocation target, our name there);
 *   - func_00283CF0 -> func_L15_00308AE0 (the v2.00 relocation target, our name there);
 *   - func_002ED688 -> func_L15_00381760 (the v2.00 relocation target, our name there);
 *   - func_002EE678 -> func_L15_00384978 (the v2.00 relocation target, our name there);
 *   - func_002EE920 -> func_L15_00384EC0 (the v2.00 relocation target, our name there);
 *   - func_002EEB00 -> func_L15_003850A0 (the v2.00 relocation target, our name there);
 *   - func_L00_00367720 -> func_L15_003812F8 (the v2.00 relocation target, our name there);
 *   - func_L05_00379E60 -> func_L15_00381920 (the v2.00 relocation target, our name there);
 *   - func_L05_0037B538 -> func_L15_00383130 (the v2.00 relocation target, our name there);
 *   - func_L05_0037C160 -> func_L15_00383D58 (the v2.00 relocation target, our name there);
 *   - func_L06_00382608 -> func_L15_0035B558 (the v2.00 relocation target, our name there);
 *   - func_L15_00436BA8 -> func_L15_00438968 (the v2.00 relocation target, our name there);
 *   - func_L15_00436C68 -> func_L15_00438A28 (the v2.00 relocation target, our name there);
 *   - upstream's file declares func_L15_00308AE0 again further down; the 2 repeats are
 *   dropped, the first declaration kept;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L15_00438750(char *object) {
    char *mover = object;
    char *state = *(char **)(mover + 0x68);

    switch (*(unsigned char *)(mover + 0x20)) {
    case 0:
        *(unsigned char *)(mover + 0x30) = 0xFF;
        if (func_L15_003812F8() == 0) {
            return;
        }
        func_L15_00438968(mover);
        if (*(int *)(state + 0x264) != 0) {
            func_L15_00381920(mover, (void *)func_L15_00438A28, 0, 0);
        }
        func_L15_0035B558(mover, *(int *)(state + 0x268), 0x8000FF00, 0x81, 0.75f, 1);
        break;
    case 1: {
        GornPath *path;
        char *owner;
        int index;

        func_L15_0035B558(mover, *(int *)(state + 0x268), 0x8000FF00, 0x81, 0.75f, 1);
        owner = (char *)func_L15_00381760(mover);
        path = (GornPath *)D_L15_001FE400[*(int *)(state + 0x268)];
        if (func_L15_003850A0(mover) != 0) {
            index = *(int *)(state + 0x26C);
            *(int *)(state + 0x26C) = index + 1;
            if (index + 1 == path->count - 1) {
                *(unsigned char *)(mover + 0x20) = 3;
            } else {
                float action = path->points[index].action;

                if (action != -1.0f) {
                    if ((func_L15_00308AE0(action) & 1) != 0) {
                        *(int *)(state + 0x274) = (func_L15_00308AE0(action) >> 1) * 0x3C;
                        *(unsigned char *)(mover + 0x20) = 2;
                    }
                }
                func_L15_00384EC0(mover, (char *)&path->points[*(int *)(state + 0x26C)]);
                *(int *)(owner + 0x130) = 0x1E8480;
            }
        }
        func_L15_00384978(mover, 0xC);
        func_L15_00383D58(mover);
        func_L15_00383130(mover, 0, 1, 1);
        break;
    }
    case 2:
        if (func_L15_00307608((int *)(state + 0x274)) != 0) {
            *(unsigned char *)(mover + 0x20) = 1;
        }
        break;
    }
}

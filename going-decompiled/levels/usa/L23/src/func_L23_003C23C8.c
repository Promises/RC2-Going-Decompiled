/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x003C23C8, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef long s64;
typedef unsigned long u64;
extern const int D_L23_001B1638 __attribute__((section(".sdata")));
extern int *D_L23_001B3628 __attribute__((section(".sdata")));
extern int *D_L23_001FF280[];
extern char D_L23_00259548[];
extern char g_heroPos[];
extern int func_L23_003C2238(int arg0, u64 v, int *arg2, int arg3);
extern int func_L23_003C22D8(int arg0, int index, char *arg);
extern void func_L23_003C2388(char *arg0, int arg1);
extern int func_L23_00325618(int);
extern int func_L23_0041B1E0(s64);
extern float func_L23_002F9088(void *, void *);
extern void func_L23_002F8EF8(void *, void *, void *);
extern float func_L23_002F8FD0(void *, void *);

/**
 * func_L23_003C23C8 - picks the object's next path node from the candidates in
 * `mask` (func_L23_003C2238). With no candidate it reverses the direction at
 * state +0x2B2. Mode 0 takes candidate number func_L23_00325618(n). Modes 1-3
 * score every candidate, skipping ones func_L23_0041B1E0 blocks when `check` is
 * set. Mode 1 takes the candidate whose end point is nearest
 * (func_L23_002F9088) to the point at state +0x320. Mode 3 takes the largest,
 * and mode 2 or any other the smallest, func_L23_002F8FD0 of two vectors
 * func_L23_002F8EF8 builds from g_heroPos and from the candidate's end point,
 * each with the object's position (+0x10). With one candidate, it is taken
 * whatever its score. The direction (+1/-1) depends on which end of the path
 * func_L23_003C22D8 says the object is at. Ends in func_L23_003C2388, which
 * records the chosen node.
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * @mask: 64-bit candidate mask.  @mode: selection mode (0-3).
 * @check: nonzero to skip blocked candidates.
 * Return: nothing.
 *
 * Non-obvious: the arrays are C99-style variable-length arrays sized by
 * D_L23_001B1638, read once at entry, as upstream writes them. The +0x2B4 node
 * id is passed to func_L23_003C22D8 as a pointer-typed argument.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_0035CE90.c, function
 * func_L23_003C0F88 (v1.01 address), author commit fa31bfc3 (bigmak94).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; the body's statements unchanged. Edits:
 *   - func_L23_003C0F88 -> func_L23_003C23C8 (our split name at the v2.00
 *     place)
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on
 *     D_L23_001B1638 and D_L23_001B3628 (ablation: without the first, 15 of
 *     293 words differ; without the second, 3) (RULING #8620: an addressing-
 *     model device on an extern DECLARATION, EE only; upstream's patched
 *     compiler reads sda, ours does not)
 *   - `unsigned long long` -> `u64` (RULING #13197: a 64-bit EE integer
 *     spelled s64/u64, local typedefs as common.h's; ablation with `long
 *     long`: 3 of 293 words differ) for the mask, and `long long` -> `s64` for
 *     the masks read and the callee prototype
 *   - D_L23_001B1688 -> D_L23_001B1638; D_L23_001B36A8 -> D_L23_001B3628,
 *     D_L23_001FF300 -> D_L23_001FF280, D_L23_002595C8 -> D_L23_00259548
 *     (address-named externs inside larger split labels, RULING #13291 section
 *     2)
 *   - D_00189EA0 -> g_heroPos (the boot's name, same address)
 *   - func_L23_003C0DF8 -> func_L23_003C2238, func_L23_003C0E98 ->
 *     func_L23_003C22D8, func_L23_003C0F48 -> func_L23_003C2388, func_002A77E0
 *     -> func_L23_00325618, func_L23_00419CE0 -> func_L23_0041B1E0,
 *     func_00282E48 -> func_L23_002F9088, func_00282CF0 -> func_L23_002F8EF8,
 *     func_00282DB0 -> func_L23_002F8FD0
 *   - the opening brace stays on its own line as upstream has it (whitespace)
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the TU
 *     declares what it uses, and upstream's comments are folded into this one
 */
void func_L23_003C23C8(char *arg0, u64 mask, int mode, int check)
{
    char *state = *(char **)(arg0 + 0x68);
    int count = D_L23_001B1638;
    int ids[count];
    int n;
    int id;
    float va[4];
    float vb[4];

    n = func_L23_003C2238((int)arg0, mask, ids, check);
    if (n == 0) {
        int d = *(short *)(state + 0x2B2);

        *(short *)(state + 0x2B2) = -d;
        return;
    }
    if (mode == 0) {
        int res;

        id = ids[func_L23_00325618(n)];
        res = func_L23_003C22D8((int)arg0, id, (char *)(int)*(short *)(state + 0x2B4));
        *(short *)(state + 0x2B2) = res == 0 ? 1 : -1;
        *(short *)(state + 0x2B6) = res;
        func_L23_003C2388(arg0, id);
        return;
    } else {
        char *pos[count];
        int res[count];
        s64 *bits[count];
        int i;
        int best;

        for (i = 0; i < n; i++) {
            int *q;

            id = ids[i];
            res[i] = func_L23_003C22D8((int)arg0, id, (char *)(int)*(short *)(state + 0x2B4));
            q = D_L23_001FF280[D_L23_001B3628[id]];
            if (res[i] == 0) {
                pos[i] = (char *)(q + *q * 4);
                bits[i] = (s64 *)(D_L23_00259548 + 8 + (D_L23_001B3628[id] << 4));
            } else {
                pos[i] = (char *)(q + 4);
                bits[i] = (s64 *)(D_L23_00259548 + (D_L23_001B3628[id] << 4));
            }
        }
        best = 0;
        switch (mode) {
        case 1: {
            float bestDist = 30000.0f;
            int j;

            for (j = 0; j < n; j++) {
                int blocked = 0;
                float d;

                if (check)
                    blocked = func_L23_0041B1E0(*bits[j]);
                d = func_L23_002F9088(pos[j], *(void **)(state + 0x320));
                if ((d < bestDist && !blocked) || n == 1) {
                    best = j;
                    bestDist = d;
                }
            }
            break;
        }
        case 3: {
            float bestDot = -30000.0f;
            int j;

            for (j = 0; j < n; j++) {
                int blocked = 0;
                float d;

                if (check)
                    blocked = func_L23_0041B1E0(*bits[j]);
                func_L23_002F8EF8(va, g_heroPos, arg0 + 0x10);
                func_L23_002F8EF8(vb, pos[j], arg0 + 0x10);
                d = func_L23_002F8FD0(va, vb);
                if ((bestDot < d && !blocked) || n == 1) {
                    best = j;
                    bestDot = d;
                }
            }
            break;
        }
        case 2:
        default: {
            float bestDot = 30000.0f;
            int j;

            for (j = 0; j < n; j++) {
                int blocked = 0;
                float d;

                if (check)
                    blocked = func_L23_0041B1E0(*bits[j]);
                func_L23_002F8EF8(va, g_heroPos, arg0 + 0x10);
                func_L23_002F8EF8(vb, pos[j], arg0 + 0x10);
                d = func_L23_002F8FD0(va, vb);
                if ((d < bestDot && !blocked) || n == 1) {
                    best = j;
                    bestDot = d;
                }
            }
            break;
        }
        }
        if (res[best] == 0) {
            id = ids[best];
            *(short *)(state + 0x2B2) = 1;
        } else {
            id = ids[best];
            *(short *)(state + 0x2B2) = -1;
        }
        *(short *)(state + 0x2B6) = res[best];
        func_L23_003C2388(arg0, id);
    }
}

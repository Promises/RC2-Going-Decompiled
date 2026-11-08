/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003E83F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef signed int s32;

extern s32 g_mobyTableBase __attribute__((section(".sdata")));
extern float func_L17_002EFE58(void *, void *);

/**
 * func_L17_003E83F0 - elect, over two passes, the object nearest a shared
 * point as the group's leader.
 *
 * @object: a moby; its index is its offset from g_mobyTableBase / 0x100.
 * Return: 1 when this object is (or has just become) the leader, else 0.
 *
 * The state block (+0x68) holds pointers to three shared words: the leader
 * index (+0x720), a phase (+0x748) and the best distance (+0x74C). An
 * object whose byte +0x21 is 0xFF returns 1 at once. When there is no live
 * leader (index -1, or its moby is NULL or in state 0xFE/0xFD), this object
 * takes the slot, sets the phase to 1, stores its distance to the point at
 * state+0x240 (func_L17_002EFE58) and returns 0. In phase 1, the leader
 * seeing itself again sets phase 2 and returns 1; any other object closer
 * than the best distance takes the slot and returns 0. In phase 2 only the
 * leader gets 1.
 *
 * Non-obvious: g_mobyTableBase is declared as a word and read as the table's
 * base. Upstream's `sda` on it is respelled `section(".sdata")` (RULING
 * #8620, see the edits).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_003C7C70.c, function func_L17_003E6DE8 (v1.01 address), author commit e4fd2fe8 (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_L17_003E6DE8 -> func_L17_003E83F0 (our split name at the v2.00 place);
 *   - D_L17_001B2C1C -> g_mobyTableBase: the name symbol_addrs/L17_boot_names.txt gives at
 *   exactly 0x001B2C1C, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - func_00282E48 -> func_L17_002EFE58 (the v2.00 relocation target, our name there);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *   g_mobyTableBase declaration (RULING #8620 ADDRESSING-MODEL DEVICE, EE arm
 *   only; this TU has no other arm): our cc1 then prints the one-instruction
 *   load macro, which GNU as expands at -G0 to the adjacent absolute lui/lw
 *   pair retail has (cc1 schedules it as one instruction). Ablation (task
 *   #2667): with upstream's `sda` spelling the object is 0x134 bytes, not 0x138,
 *   and 76 of 77 masked words differ. It moves no data and emits
 *   nothing;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
int func_L17_003E83F0(char *object)
{
    char *base = (char *)g_mobyTableBase;
    int index = (object - base) >> 8;
    char *state = *(char **)(object + 0x68);
    int other;
    char *target;
    float *nearest;
    float *best;
    float distance;

    if (*(u8 *)(object + 0x21) == 0xFF)
        return 1;
    other = **(int **)(state + 0x720);
    if (other == -1 || (target = base + (other << 8)) == 0
        || *(u8 *)(target + 0x20) == 0xFE || *(u8 *)(target + 0x20) == 0xFD) {
        **(int **)(state + 0x720) = index;
        **(int **)(state + 0x748) = 1;
        nearest = *(float **)(state + 0x74C);
        *nearest = func_L17_002EFE58(object + 0x10, state + 0x240);
        return 0;
    }
    if (**(int **)(state + 0x748) == 1) {
        if (index == other) {
            **(int **)(state + 0x748) = 2;
            return 1;
        }
        distance = func_L17_002EFE58(object + 0x10, state + 0x240);
        best = *(float **)(state + 0x74C);
        if (distance < *best) {
            **(int **)(state + 0x720) = index;
            *best = distance;
            return 0;
        }
    }
    if (**(int **)(state + 0x748) == 2 && **(int **)(state + 0x720) == index)
        return 1;
    return 0;
}

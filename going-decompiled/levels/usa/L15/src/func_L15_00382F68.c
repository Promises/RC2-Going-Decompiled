/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00382F68, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
typedef short s16;
extern u32 g_mobyGroupLists[];
extern s32 func_L15_00381760(void *);
extern char *D_L15_001B261C_base[] __asm__("g_mobyTableBase") __attribute__((section(".sdata")));

/**
 * func_L15_00382F68 - count the live members of an object's moby group.
 *
 * @arg0: the object; its byte +0x21 selects the group list.
 * Return: the count.
 *
 * g_mobyGroupLists[group] is a list of u16 moby indices whose last entry has
 * bit 15 set; a zero list counts 0. A moby counts when bit 0x20 of its
 * flags halfword +0x34 is set, its state block (+0x68) holds a non-zero
 * first word, the record func_L15_00381760 returns for it has a non-zero
 * +0x110, and the float that first word points to is above 0.0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_00315428.c, function func_L15_00381CF0 (v1.01 address), author commit b7357856 (bigbeno37), which
 * carried Louis-Philippe Le Sieur's nonmatching draft
 * nonmatching/levels/func_L15_00381CF0.c (author commit d91dba61, earlier)
 * into src: 17 of the 28 definition lines are the draft's text and 11
 * (the float compare, the do/while walk and the return) are bigbeno37's
 * (git blame -M -C), so both are credited (RULING #13427 rev 2, #13541 rev 2).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED, the body's statements unchanged; edits:
 *   - name func_L15_00381CF0 -> func_L15_00382F68 (our split name at the v2.00 place);
 *   - D_L15_001B261C -> g_mobyTableBase: the name symbol_addrs/L15_boot_names.txt gives at
 *   exactly 0x001B259C, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - D_L15_001FB320 -> g_mobyGroupLists: the name symbol_addrs/L15_boot_names.txt gives at
 *   exactly 0x001FB2A0, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - func_002ED688 -> func_L15_00381760 (the v2.00 relocation target, our name there);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on D_L15_001B261C_base (upstream's asm-label view of the
 *   table base, its label respelled to g_mobyTableBase) (RULING #8620
 *   ADDRESSING-MODEL DEVICE, EE arm only; this TU has no other arm): our cc1
 *   does not know `sda`/`nosda` (it warns "directive ignored"); with `.sdata` it prints
 *   the one-instruction access macro, which GNU as expands at -G0 to the
 *   adjacent absolute lui + %lo pair retail has. It moves no
 *   data and emits nothing;
 *   - ablation (task #2677): with upstream's `sda` spelling and nothing
 *   else changed, 49 of 54 masked words differ (0xD8 bytes, not 0xD0);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
s32 func_L15_00382F68(char *arg0) {
    u16 *p;
    char *base;
    char *e;
    char *o;
    u32 slot;
    s32 v;
    s32 count = 0;
    float f;

    slot = g_mobyGroupLists[(u8)arg0[0x21]];
    if (slot == 0)
        return 0;
    p = (u16 *)slot;
    f = 0.0f;
    do {
        base = D_L15_001B261C_base[0];
        e = (char *)(((*p & 0x7FFF) << 8) + (u32)base);
        if ((*(u16 *)(e + 0x34) & 0x20) != 0) {
            o = *(char **)(e + 0x68);
            v = func_L15_00381760(e);
            if (*(s32 *)o != 0 && *(s32 *)(v + 0x110) != 0 &&
                f < *(float *)(*(s32 *)o))
                count++;
        }
    } while (*(s16 *)p++ >= 0);
    return count;
}

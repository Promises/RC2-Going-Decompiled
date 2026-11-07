/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x0044B030, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned short u16;
typedef int TI128 __attribute__((mode(TI)));
extern float D_L20_001B239C __attribute__((sda));
extern void func_L20_00330820(char *, int, int);
extern void func_L20_00302FF0(void *, void *, void *);
extern void func_L20_003032A8(void *dst, void *src, float x);
extern void func_L20_00421D88(char *, char *, void *, float);
extern int func_L20_00332230(char *, char *, int, int, float, float, float, int, int);
extern char g_cameraPos[] __attribute__((nosda));

/**
 * func_L20_0044B030 - combine an object's position with a level point, call
 * func_L20_00421D88 and func_L20_00332230, and move it to state 1.
 *
 * @arg0: the object; +0x10 its position (float +0x18 used), +0x34 its u16
 * flags, +0x98 a word that is cleared.
 * Return: none.
 *
 * Non-obvious: func_L20_00302FF0(&v, g_cameraPos, pos) combines the level
 * point and the position into v, and func_L20_003032A8(&v, &v, 1.0) rewrites
 * v. func_L20_00421D88(arg0, pos, &v, z - 0.5) follows, using the position's
 * float at +0x18. func_L20_00332230 gets 0x10000, 0, 6.0, D_L20_001B239C, 2.0,
 * 1 and 0. The flags then gain 0x41 and lose 0x1000, and
 * func_L20_00330820(arg0, 1, -1) runs last. Upstream's `sda` / `nosda`
 * attributes are kept as written; this cc1 ignores them (warnings), and the
 * member closes with them as written.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_00449178-g8.c (blob 854a7b56), function
 * func_L20_00449AE8 (v1.01 address), author commit e0711884 (ReaperOfVeriod).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_00449AE8 -> func_L20_0044B030 (our split name at the v2.00
 *     place);
 *   - D_L20_001B23EC -> D_L20_001B239C (our label at that address);
 *   - D_L20_001B9FC0 -> g_cameraPos (L20's boot-map name at the v2.00
 *     target 0x001B9F40, inside the split label D_L20_001B32B0 at +0x6C90);
 *   - D_L20_001B9F40 -> g_cameraPos (task #2646): the name
 *     symbol_addrs/L20_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - func_00282CF0 -> func_L20_00302FF0 (the level's own copy at its L20
 *     place);
 *   - func_00282F20 -> func_L20_003032A8 (the level's own copy at its L20
 *     place);
 *   - func_002A7750 -> func_L20_00330820 (the level's own copy at its L20
 *     place);
 *   - func_002A8C28 -> func_L20_00332230 (the level's own copy at its L20
 *     place);
 *   - func_0032DA58 -> func_L20_00421D88 (the level's own copy at its L20
 *     place);
 *   - `__attribute__((sda))` / `nosda` kept as upstream's text: this cc1
 *     ignores it (a warning), and the member closes with it as written;
 *   - `#include "include_asm.h"` dropped; the typedefs this body uses are
 *     written here (or are the file's own); upstream file-scope declarations
 *     it does not use are not carried, the rest keep upstream's order and
 *     text, duplicates included.
 */
void func_L20_0044B030(char *arg0) {
    TI128 v;

    func_L20_00302FF0(&v, g_cameraPos, arg0 + 0x10);
    func_L20_003032A8(&v, &v, 1.0f);
    func_L20_00421D88(arg0, arg0 + 0x10, &v, *(float *)(arg0 + 0x18) - 0.5f);
    func_L20_00332230(arg0, arg0 + 0x10, 0x10000, 0, 6.0f, D_L20_001B239C, 2.0f,
                  1, 0);
    *(int *)(arg0 + 0x98) = 0;
    *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    func_L20_00330820(arg0, 1, -1);
}

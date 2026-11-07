/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x004203A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

typedef struct {
    u8 pad00[0x268];
    f32 phase;
} Vars_L10_0041EE00;

typedef struct {
    u8 pad00[0x10];
    f32 pos[4];
    u8 pad20[0x68 - 0x20];
    Vars_L10_0041EE00 *vars;
    u8 pad6C[0xC0 - 0x6C];
    f32 axis[4];
} Moby_L10_0041EE00;

/* Id lists, one per group: L10's boot-name map name at 0x001FB160. */
extern u16 *g_mobyGroupLists[];
/* The base of the level's 256-byte object records, L10's boot-name map name
   at 0x001B249C. ADDRESSING-MODEL DEVICE (RULING #8620) on this declaration:
   upstream's `sda` (ignored by this cc1) is respelled `section(".sdata")`,
   so cc1 prints a one-instruction macro load that the assembler expands at
   -G0 to the absolute lui/lw pair the ROM has. Ablation (task #2657,
   upstream's own `sda`, same arm and flags): 80 words where the disc has 78,
   73 differing. It moves no data and emits nothing. */
extern char *g_mobyTableBase[] __attribute__((section(".sdata")));

/* The level's copies of boot code, named by their v2.00 places. */
extern void func_L10_003089D0(void *, s32, u32, s32, float, float, float);
extern void func_L10_00309898(void *, void *, void *);
extern void func_L10_00309920(void *, void *, float);
extern float func_L10_00309E88(float);
extern int func_L10_0030A9F8(float);
extern float func_L10_003365A8(void);
extern char *func_L10_0037CCF8(void *);

/**
 * func_L10_004203A0 - draw a pulsing glow at every live object of this
 * object's group.
 *
 * Walks the group's id list (g_mobyGroupLists[@m's byte at +0x21]; ids index
 * 256-byte records from g_mobyTableBase, the list ends after the first id
 * with its sign bit set). For each object whose record from
 * func_L10_0037CCF8 has a non-zero word at +0x110, the glow sits at its
 * position plus a quarter of its axis (+0xC0), with alpha
 * 64 * (func_L10_00309E88(phase) + 1) / 2 over colour 0x63B4FF, drawn by
 * func_L10_003089D0 (kind 0x31, sizes 7.5 and 2.0, a random angle).
 *
 * @m: the object (the group owner).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_0041EE00.c, function
 * func_L10_0041EE00 (v1.01 address), author commit 5e61bf0b (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; the body's statements unchanged, the edits, each name to
 * the target of the v2.00 disc's own relocation at this place (task #2657
 * relverify):
 *   - func_L10_0041EE00 -> func_L10_004203A0 (our split name at the v2.00
 *     place); upstream's struct tags Vars_L10_0041EE00 / Moby_L10_0041EE00
 *     are kept as written;
 *   - D_L10_001FB1E0 -> g_mobyGroupLists, D_L10_001B251C -> g_mobyTableBase
 *     (the names symbol_addrs/L10_boot_names.txt gives at the v2.00 targets
 *     0x001FB160 and 0x001B249C); the task's row also listed address-named
 *     externs (code `o`), none needed now that both carry names (task #2646);
 *   - func_00281EB8 -> func_L10_003089D0, func_00282CC0 -> func_L10_00309898,
 *     func_00282D30 -> func_L10_00309920, func_00283180 -> func_L10_00309E88,
 *     func_00283CF0 -> func_L10_0030A9F8, func_002A7940 -> func_L10_003365A8,
 *     func_002ED688 -> func_L10_0037CCF8 (our split names at the v2.00
 *     targets);
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on g_mobyTableBase's extern declaration (RULING #8620; the audit's
 *     `resp` variant, NOTE #13281);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     upstream's comment folded into this one.
 */
void func_L10_004203A0(u8 *m)
{
    f32 v[4] __attribute__((aligned(16)));
    u16 *first = g_mobyGroupLists[m[0x21]];
    u16 *list;
    Moby_L10_0041EE00 *o;
    Vars_L10_0041EE00 *vars;
    u32 color;

    if (first != 0) {
        list = first;
        do {
            o = (Moby_L10_0041EE00 *)(g_mobyTableBase[0] + ((*list & 0x7FFF) << 8));
            if (*(s32 *)(func_L10_0037CCF8(o) + 0x110) != 0) {
                vars = o->vars;
                func_L10_00309920(v, o->axis, 0.25f);
                func_L10_00309898(v, o->pos, v);
                color = (func_L10_0030A9F8((func_L10_00309E88(vars->phase) + 1.0f) * 0.5f * 64.0f) << 24) | 0x63B4FF;
                func_L10_003089D0(v, 0x31, color, 1, 7.5f, 2.0f, func_L10_003365A8());
            }
        } while ((s16)*list++ >= 0);
    }
}

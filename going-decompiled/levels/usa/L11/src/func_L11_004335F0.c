/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x004335F0, in place
 * of its split asm; the row is config/L11_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

extern void *g_mobyGroupLists[];
extern u8 *g_mobyTableBase;

/**
 * func_L11_004335F0 - flag every record of the object's id list whose class
 * halfword is 0xD07.
 *
 * Walks the id list g_mobyGroupLists[object +0x21] to its end (a negative
 * halfword), indexes each id into the 256-byte record table at
 * g_mobyTableBase, and sets +0x276 of a live (byte +0x20 >= 0) class-0xD07
 * record's state to 1.
 *
 * @arg0: the moby object; reads +0x21 (group id). Returns immediately when it
 * is 0xFF or the group's list is null.
 *
 * Non-obvious: both tables are the names L11's boot-name map gives at their
 * v2.00 addresses (task #2646).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00425F10-g8.c, function func_L11_00431DB0 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - `__attribute__((sda))` kept verbatim (this toolchain ignores it, warning
 *     only; measured inert at this arm and -G flag);
 *   - func_L11_00431DB0 -> func_L11_004335F0 (this function's v2.00 split name);
 *   - D_L11_001B359C -> g_mobyTableBase, D_L11_001FC420 -> g_mobyGroupLists
 *     (upstream's v1.01 names -> L11's boot-map names at the v2.00 targets
 *     0x001B361C, 0x001FC4A0);
 *   - D_L11_001B361C -> g_mobyTableBase, D_L11_001FC4A0 -> g_mobyGroupLists
 *     (task #2646): the names symbol_addrs/L11_boot_names.txt gives at exactly
 *     those addresses; the address-named spelling was a second name for the
 *     same address (A2 class, NOTE #13409).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_004335F0(u8 *arg0) {
    void *list;
    u16 *p;
    u8 *base;

    if (arg0[0x21] == 0xFF)
        return;
    list = g_mobyGroupLists[arg0[0x21]];
    if (list == 0)
        return;
    p = list;
    base = g_mobyTableBase;
    do {
        u8 *e = base + ((*p & 0x7FFF) << 8);

        if (*(s8 *)(e + 0x20) >= 0 && *(s16 *)(e + 0xAA) == 0xD07)
            *(s16 *)(*(u8 **)(e + 0x68) + 0x276) = 1;
    } while (*(s16 *)p++ >= 0);
}

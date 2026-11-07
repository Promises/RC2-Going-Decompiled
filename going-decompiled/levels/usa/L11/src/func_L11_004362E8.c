/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x004362E8, in place
 * of its split asm; the row is config/L11_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

extern void func_L11_00436338(char *p, void *q);
extern void func_L11_00436540(char *p, void *q);

/**
 * func_L11_004362E8 - hand two reads of the state's quadword to the two
 * measured stage helpers.
 *
 * The state's first 16 bytes are read as a TI128, passed by address to
 * func_L11_00436338, re-read, and passed to func_L11_00436540; the second
 * read must observe anything the first helper changed.
 *
 * @p: the moby object; reads +0x68 (state).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00432628.c, function func_L11_00434AA8 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_00434AA8 -> func_L11_004362E8 (this function's v2.00 split name);
 *   - func_L11_00434AF8 -> func_L11_00436338, func_L11_00434D00 -> func_L11_00436540 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_004362E8(char *p)
{
    char *state = *(char **)(p + 0x68);
    TI128 v = *(TI128 *)state;

    func_L11_00436338(p, &v);
    v = *(TI128 *)state;
    func_L11_00436540(p, &v);
}

/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x00439B60, in place
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

extern void func_L11_003014F8(void *, void *, float);
extern void func_L11_003011A8(void *);
extern void func_L11_00301328(void *a, void *b, void *c);
extern void func_L11_003325D0(char *a, char *b);
extern void func_L11_00331800(char *a, char *b, int c, float d, float e, float f, float g);

/**
 * func_L11_00439B60 - build the measured three-axis basis of the object and
 * hand it to the three slots.
 *
 * Scales `direction` to unit length, takes the world-up cross products for the
 * other two axes (each re-normalised), projects the first axis to `out`, and
 * submits out[0..2] to the object's +0xF0/+0xF4/+0xF8 slots against the
 * state's +0x2A0/+0x2A4/+0x2A8 anchors with (a, b, c) as the float tail.
 *
 * @obj: the moby object; reads +0x68 (state), writes +0xF0..+0xF8.
 * @direction: the heading vector (scaled, not assumed unit). @a/@b/@c: the
 * three float tail arguments of the slot submissions.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00432628.c, function func_L11_00438320 (v1.01 address), author commit 431ac503 (llesieur99),
 * which wrote the body; changed in 6b943916 (Louis-Philippe Le Sieur, the same
 * llesieur99 account), which added the `direction` parameter and made the
 * first call func_00282F20((char *)m[0], direction, 1.0f).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_00438320 -> func_L11_00439B60 (this function's v2.00 split name);
 *   - func_00282C88 -> func_L11_003011A8, func_00282DD8 -> func_L11_00301328,
 *   - func_00282F20 -> func_L11_003014F8, func_002AAA00 -> func_L11_00331800,
 *   - func_002AB6D8 -> func_L11_003325D0 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_00439B60(char *obj, char *direction, float a, float b, float c)
{
    char *state = *(char **)(obj + 0x68);
    float m[4][4];
    float out[4];

    func_L11_003014F8((char *)m[0], direction, 1.0f);
    func_L11_003011A8((TI128 *)m[2]);
    m[2][2] = 1.0f;
    func_L11_00301328((char *)m[1], (char *)m[0], (char *)m[2]);
    func_L11_003014F8((char *)m[1], (char *)m[1], 1.0f);
    func_L11_00301328((char *)m[2], (char *)m[1], (char *)m[0]);
    func_L11_003014F8((char *)m[2], (char *)m[2], 1.0f);
    func_L11_003325D0((char *)m[0], (char *)out);
    func_L11_00331800(obj + 0xF0, state + 0x2A0, 0, out[0], a, b, c);
    func_L11_00331800(obj + 0xF4, state + 0x2A4, 0, out[1], a, b, c);
    func_L11_00331800(obj + 0xF8, state + 0x2A8, 0, out[2], a, b, c);
}

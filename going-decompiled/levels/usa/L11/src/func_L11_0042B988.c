/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0042B988, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 104 of
 * 149 .text words differ from the disc, relocated fields masked, and .text is
 * 0x254 bytes, not the place's 0x2C4, and it emits a .lit4 pool of 52 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

typedef unsigned char u8;
typedef float f32;

extern void func_L11_003011E0(f32 *a0, f32 *a1, f32 *a2);
extern float func_L11_00301340(void *);
extern void func_L11_00301210(void *, void *, void *);
extern void func_L11_0033CF50(void *, void *, void *, void *, int, void *, void *, f32, f32);
extern void func_L11_00331800(void *, f32, f32, f32, f32, void *, int);
extern f32 func_L11_00302390(f32, f32);
extern void func_L11_00331348(void *, void *, f32, f32, f32, f32);
extern f32 func_L11_00331048(f32 *, f32, f32);
extern void func_L11_003014F8(void *, void *, f32);
extern int func_L11_0032E668(void *, void *, void *, f32, f32, f32, f32, int);

/**
 * func_L11_0042B988 - steer the object toward a target point and settle on a
 * heading there (the body of func_L11_00426FA0).
 *
 * @object:        the walker (moby); +0x10 position, +0xC0 forward, +0xF8 yaw,
 *                 +0x68 its state block.
 * @target:        the target point.
 * @desiredHeading: the heading to settle on once within @stopDistance.
 * @speed:         the walk speed (per frame speed/60, acceleration speed/1200).
 * @stopDistance:  the stop distance.
 * @margin:        the margin passed to the path query func_L11_0033CF50.
 * Return: 1 when it has stopped and faces @desiredHeading within 5 degrees, or
 * after 30 frames in which the move did not succeed; else 0.
 *
 * Non-obvious: Upstream notes this is the same body as an L04 function, with
 * this level's own callees. Here it also has the same body as
 * func_L11_00426FA0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00425F10.c (blob 2ea06e9e), function
 * func_L11_0042A158 (v1.01 address), author commit 76d5f55b (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED: built at -G8 where upstream builds the file at -G0 (its stem has no
 * -g8; a tsv gflag setting, not a text edit; measured at -G0, task #2578:
 * .text 0x2BC bytes, not 0x2C4 (cobj refuses)); the text is verbatim but for
 * names. Respellings, each to the target of the v2.00 disc's own relocation at
 * this place (relverify by name on this object, task #2578):
 *   - func_L11_0042A158 -> func_L11_0042B988 (our split name at the v2.00
 *     place);
 *   - func_00282CC0 -> func_L11_003011E0 (the level's own copy at its L11
 *     place);
 *   - func_00282CF0 -> func_L11_00301210 (the level's own copy at its L11
 *     place);
 *   - func_00282DF0 -> func_L11_00301340 (the level's own copy at its L11
 *     place);
 *   - func_00282F20 -> func_L11_003014F8 (the level's own copy at its L11
 *     place);
 *   - func_00283C80 -> func_L11_00302390 (the level's own copy at its L11
 *     place);
 *   - func_002A7EA0 -> func_L11_0032E668 (the level's own copy at its L11
 *     place);
 *   - func_002AA2E8 -> func_L11_00331048 (the level's own copy at its L11
 *     place);
 *   - func_002AA548 -> func_L11_00331348 (the level's own copy at its L11
 *     place);
 *   - func_002AAA00 -> func_L11_00331800 (the level's own copy at its L11
 *     place);
 *   - func_L02_0032E998 -> func_L11_0033CF50 (the level's own copy at its L11
 *     place);
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
int func_L11_0042B988(u8 *object, void *target, f32 desiredHeading, f32 speed, f32 stopDistance, f32 margin) {
    f32 delta[4], displacement[4], destination[4];
    f32 angle, frameSpeed = 0.0f;
    u8 *data = *(u8 **)(object + 0x68);
    int done = 0;
    f32 distance, acceleration;
    func_L11_00301210(delta, target, object + 0x10);
    delta[2] = 0.0f;
    distance = func_L11_00301340(delta);
    acceleration = speed * 0.0008333333535119891f;
    if (stopDistance < distance) {
        int collision = *(int *)(data + 0x520);
        angle = 0.0f;
        func_L11_0033CF50(object + 0x10, target, &angle, data + 0x320, collision, data + 0x530, data + 0x534, margin, 0.4f);
        func_L11_00331800(object + 0xF8, angle, 0.006981317885220051f, 0.0034906589426100254f, 0.20943954586982727f, data + 0x27C, 0);
        if (func_L11_00302390(*(f32 *)(object + 0xF8), angle) < 0.523598849773407f) {
            func_L11_00331348(&frameSpeed, data + 0x538, distance, acceleration, acceleration, speed * 0.01666666753590107f);
        } else {
            *(f32 *)(data + 0x538) = 0.0f;
        }
    } else {
        func_L11_00331800(object + 0xF8, desiredHeading, 0.006981317885220051f, 0.2f, 0.20943954586982727f, data + 0x27C, 0);
        func_L11_00331048((f32 *)(data + 0x538), 0.0f, acceleration);
        if (*(f32 *)(data + 0x538) == 0.0f && func_L11_00302390(*(f32 *)(object + 0xF8), desiredHeading) < 0.0872664749622345f) {
            *(f32 *)(data + 0x27C) = 0.0f;
            done = 1;
        }
    }
    func_L11_003014F8(displacement, object + 0xC0, *(f32 *)(data + 0x538));
    func_L11_003011E0(destination, displacement, object + 0x10);
    if (!func_L11_0032E668(object, object + 0x10, destination, 0.75f, 0.75f, 0.4f, 0.523598849773407f, 0))
        *(int *)(data + 0x53C) += 1;
    if (*(int *)(data + 0x53C) >= 30) {
        *(int *)(data + 0x53C) = 0;
        done = 1;
    }
    return done;
}

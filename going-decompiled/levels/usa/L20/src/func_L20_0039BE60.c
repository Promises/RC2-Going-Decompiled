/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x0039BE60, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef struct YeedilEffectState {
    char pad00[0x30];
    int mode;
    int frame;
    char pad38[0x154];
    char *first;
    int unused190;
    char *second;
} YeedilEffectState;
extern YeedilEffectState D_L20_001BDC40;
/* RULING #8620 addressing device (level code has no native arm): upstream's
 * `__attribute__((sda))` is written `section(".sdata")` on these extern
 * DECLARATIONS. It moves no data and emits nothing: cc1 then prints the
 * one-instruction `lw/sw $r,SYM` macro, which GNU as expands to an adjacent
 * lui + lw/sw pair (absolute %hi/%lo, not $gp-relative at -G0), as retail
 * has it. Ablation (task #2565): with upstream's `sda` (ignored by this cc1)
 * the function differs from the disc in 4 of 110 words (masked compare). */
extern int D_L20_001A8F00[] __attribute__((section(".sdata")));
extern void func_L20_0041B550(void *, int, int, int, int, int, int, int,
                         int, int, int, int, float, float);
extern void func_L20_00327768(void *, int, void *);
extern float func_L20_00330988(float, float);
extern float func_L20_00330A50(void);
extern void func_L20_00339838(void *, float, float, float);
extern int func_L20_003308F0(int);
extern void func_L20_0034B690(void *, void *, int, float);

/**
 * func_L20_0039BE60 - update a Yeedil effect emitter (two-state machine in byte +0x20).
 *
 * State 0: sets +0x30 to 0xFF and moves to state 1. State 1: only while
 * D_L20_001A8F00[0] is 2 and the effect state D_L20_001BDC40 is in mode 7. At
 * frame 157 it starts func_L20_0041B550 on the state's second object; in
 * frames 171..319 it takes a point of the state's first object
 * (func_L20_00327768), raises it by 0.03, draws a random speed, heading and
 * tilt (func_L20_00330988 / func_L20_00330A50), builds a velocity
 * (func_L20_00339838) and, when func_L20_003308F0(2) says so, spawns a
 * particle there (func_L20_0034B690) with a random size of 1470..7350.
 *
 * @object: the emitter moby; +0x20 state, +0x30 written.
 * Return: nothing.
 *
 * Non-obvious: the frame window is the single unsigned compare (frame - 171)
 * < 149, as upstream wrote it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_002DEBE0.c, function func_L20_0039AA98 (v1.01
 * address), author commit 5acf9869 (Flavius). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0039AA98 -> func_L20_0039BE60; D_L20_001BDCC0 ->
 *     D_L20_001BDC40; func_0029FCC0 -> func_L20_00327768; func_002A77E0 ->
 *     func_L20_003308F0; func_002A7878 -> func_L20_00330988; func_002A7940 ->
 *     func_L20_00330A50; func_002AF000 -> func_L20_00339838; func_002BE278 ->
 *     func_L20_0034B690; func_0032C700 -> func_L20_0041B550
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))` on
 *     the extern declarations (RULING #8620 addressing device, comment above
 *     them)
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 *   - the file-local YeedilEffectState typedef (upstream's) carried with it
 */
void func_L20_0039BE60(unsigned char *object)
{
    float position[4], velocity[4];
    float speed, heading, tilt;
    YeedilEffectState *state;

    switch (object[0x20]) {
    case 0:
        object[0x30] = 0xFF;
        object[0x20] = 1;
        break;
    case 1:
        if (D_L20_001A8F00[0] != 2)
            break;
        state = &D_L20_001BDC40;
        if (state->mode != 7)
            break;
        if (state->frame == 157)
            func_L20_0041B550(state->second + 0x10, 1, 40, 20, 30, 40, 50, 1,
                          1, 1, 0, 0, 1.5f, 20.0f);
        if ((unsigned int)(state->frame - 171) < 149) {
            func_L20_00327768(state->first, 0, position);
            position[2] += 0.03f;
            speed = func_L20_00330988(0.028333336114883423f, 0.05000000447034836f);
            heading = func_L20_00330A50();
            tilt = func_L20_00330988(0.523598849773407f, 2.6179940700531006f);
            func_L20_00339838(velocity, speed, heading, tilt);
            if (func_L20_003308F0(2))
                func_L20_0034B690(position, velocity, 0, func_L20_00330988(1470.0f, 7350.0f));
        }
        break;
    }
}

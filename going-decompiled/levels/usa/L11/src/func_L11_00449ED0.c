/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00449ED0, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 181 of
 * 216 .text words differ from the disc, relocated fields masked, and .text is
 * 0x360 bytes, not the place's 0x39C, and it emits a .lit4 pool of 28 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;

extern float func_L11_0032DD60(float, float);
extern s32 func_L11_0032DCC8(s32);
typedef struct JobObject JobObject;
typedef struct {
    u8 pad00[0x18];
    float *params;          /* +0x18, points at paramBlock */
    u8 pad1C[4];
    float health;           /* +0x20 */
    u8 pad24[4];
    u8 f28;
    u8 f29;
    u8 pad2A[4];
    u8 f2E;
    u8 pad2F;
    float f30;
    u8 f34;
    u8 pad35[0x48 - 0x35];
    u8 f48;
    u8 pad49;
    u8 f4A;
    u8 pad4B[5];
    u8 hitInfo[0x10];       /* +0x50 */
    u8 f60;
    u8 pad61[8];
    u8 f69;
    u8 pad6A[0x80 - 0x6A];
    float f80;
    float f84;
    u8 pad88[4];
    float f8C;
    u8 pad90[8];
    s32 f98;
    u8 pad9C[0xFC - 0x9C];
    float fFC;
    float paramBlock[5];    /* +0x100 */
    u8 pad114[0x12C - 0x114];
    float f12C;
    float f130;
    float f134;
    u8 pad138[0x1D4 - 0x138];
    float f1D4;
    u8 pad1D8[0x1E6 - 0x1D8];
    u8 f1E6;
    u8 pad1E7;
    u8 f1E8;
    u8 f1E9;
    u8 pad1EA[0x1FC - 0x1EA];
    u16 spawnTimer;         /* +0x1FC */
    u8 pad1FE[0x2F0 - 0x1FE];
    float basis[12];        /* +0x2F0 */
    float anchor[4];        /* +0x320 */
    u8 pad330[0x354 - 0x330];
    JobObject *turret;      /* +0x354 */
    float turnRate;         /* +0x358 */
    u8 pad35C[0x364 - 0x35C];
    s16 fireTimer;          /* +0x364 */
    u8 pad366[0x36E - 0x366];
    s16 f36E;
} JobState;
struct JobObject {
    u8 pad00[0x10];
    float position[4];      /* +0x10 */
    u8 stage;               /* +0x20 */
    u8 pad21[0x31 - 0x21];
    u8 f31;
    u8 pad32[2];
    u16 flags;              /* +0x34 */
    u8 pad36[0x68 - 0x36];
    JobState *state;        /* +0x68 */
    u8 pad6C[3];
    u8 f6F;
    u8 pad70[0x95 - 0x70];
    u8 f95;
    u8 pad96[0xB8 - 0x96];
    s32 fB8;
    u8 padBC[4];
    float rotation[12];     /* +0xC0 */
};
extern s32 func_L11_00302400(float);
extern float func_L11_003013B8(void *, void *);
extern void func_L11_0032FF88(void *);
extern void *func_L11_00330218(void *, s32, s32);
extern s32 func_L11_0033A1D8(void *, void *, s32);
extern void func_L11_00301D20(void *, void *);
extern void func_L11_00338628(void *, void *, s32);
extern s32 func_L11_00447BE8(s32, void *);
extern void func_L11_0032DBF8(void *, s32, s32);
extern char g_cameraPos[];
extern s32 D_L11_001A8FF4;
extern void func_L11_0044B2A0(void *);
extern void func_L11_0044A820(void *);
extern s32 func_L11_0044A708(void *, void *);
extern void func_L11_0044AE20(void *, s32);
extern void func_L11_0044B398(void *, s32, s32);

/**
 * func_L11_00449ED0 - per-frame update of a turret-like enemy: set it up on
 * its first frame, then handle hits, firing, damage and death.
 *
 * @object: the enemy (JobObject view).
 * Return: none.
 *
 * Non-obvious: On stage 0 it seeds its parameter block, including a
 * random-sign turn rate of 10 to 30 degrees. Upstream: the sign is chosen
 * inside the store's own expression, so the stored value does not cross the
 * coin-flip call.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_004465F8-g8.c (blob c5476c37), function
 * func_L11_004484A8 (v1.01 address), author commit 3cd8af2d (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2578):
 *   - func_L11_004484A8 -> func_L11_00449ED0 (our split name at the v2.00
 *     place);
 *   - D_001A8FF4 -> D_L11_001A8FF4 (our label at that address);
 *   - D_L11_001BA8C0 -> g_cameraPos (L11's boot-map name at
 *     the v2.00 target 0x001BA940);
 *   - func_00282E48 -> func_L11_003013B8 (the level's own copy at its L11
 *     place);
 *   - func_00283698 -> func_L11_00301D20 (the level's own copy at its L11
 *     place);
 *   - func_00283CF0 -> func_L11_00302400 (the level's own copy at its L11
 *     place);
 *   - func_002A7750 -> func_L11_0032DBF8 (the level's own copy at its L11
 *     place);
 *   - func_002A77E0 -> func_L11_0032DCC8 (the level's own copy at its L11
 *     place);
 *   - func_002A7878 -> func_L11_0032DD60 (the level's own copy at its L11
 *     place);
 *   - func_002A93B0 -> func_L11_0032FF88 (the level's own copy at its L11
 *     place);
 *   - func_002A9640 -> func_L11_00330218 (the level's own copy at its L11
 *     place);
 *   - func_002AFFD8 -> func_L11_00338628 (the level's own copy at its L11
 *     place);
 *   - func_002B0ED8 -> func_L11_0033A1D8 (the level's own copy at its L11
 *     place);
 *   - func_L11_004461C0 -> func_L11_00447BE8 (its v2.00 split name);
 *   - func_L11_00448CE0 -> func_L11_0044A708 (its v2.00 split name);
 *   - func_L11_00448DF8 -> func_L11_0044A820 (its v2.00 split name);
 *   - func_L11_004493F8 -> func_L11_0044AE20 (its v2.00 split name);
 *   - func_L11_00449878 -> func_L11_0044B2A0 (its v2.00 split name);
 *   - func_L11_00449970 -> func_L11_0044B398 (its v2.00 split name);
 *   - D_L11_001BA940 -> g_cameraPos (task #2646): the name
 *     symbol_addrs/L11_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_00449ED0(JobObject *object)
{
    JobState *state = object->state;
    void *hit;
    float speed;

    if (object->stage == 0) {
        state->params = state->paramBlock;
        state->f48 = func_L11_00302400(15.0f);
        state->f4A = func_L11_00302400(16.0f);
        speed = func_L11_0032DD60(10.0f, 30.0f) * 0.01745329424738884f;
        state->turnRate = func_L11_0032DCC8(2) ? -speed : speed;
        state->f80 = 1.3f;
        state->f84 = 1.3f;
        state->f69 = 2;
        state->f1E9 = 3;
        state->f1D4 = 0.5f;
        state->paramBlock[1] = 0.5f;
        state->paramBlock[2] = 0.5f;
        state->f60 = 16;
        state->f98 = 7;
        state->f8C = 0.4f;
        state->f1E6 = 5;
        state->f1E8 = 2;
        state->paramBlock[0] = 1.3f;
        state->paramBlock[3] = 0.5f;
        state->paramBlock[4] = 1.5707964897155762f;
        state->f130 = 0.523598849773407f;
        state->f134 = 0.01f;
        state->f12C = 0.0055555556900799274f;
        state->f28 = 3;
        state->f29 = 1;
        state->f30 = 0.75f;
        state->fireTimer = 0;
        if (!func_L11_0032DCC8(2))
            object->flags |= 0x8000;
        state->turret = 0;
        func_L11_0044B2A0(object);
    }
    if (state->f36E == 0 && object->stage != 18 && !D_L11_001A8FF4) {
        object->flags |= 0x400;
        if (object->f31 && func_L11_003013B8(object->position, g_cameraPos) < 24.0f) {
            func_L11_0032FF88(object);
            object->f6F = 22;
        }
    } else {
        object->flags &= 0xFBFF;
    }
    if (object->stage >= 3) {
        state->fFC = 1.0f;
        hit = func_L11_00330218(object, 0x10000, 1);
        if (hit) {
            state->fFC = 1.0f;
            if (func_L11_0044A708(object, hit)) {
                state->f60 = 15;
                func_L11_0033A1D8(object, state->hitInfo, 1);
                state->f60 = 16;
                if (state->turret && state->fireTimer == 0 && object->f31) {
                    func_L11_0044AE20(state->turret->position, 2);
                    state->fireTimer = 60;
                }
            } else {
                func_L11_0033A1D8(object, state->hitInfo, 0);
            }
        } else {
            func_L11_0033A1D8(object, state->hitInfo, 0);
        }
        if (state->health <= 0.0f && object->f95 != 7 && object->f95 != 8)
            func_L11_0044B398(object, 1, 0);
        if (state->f36E) {
            func_L11_00301D20(state->basis, object->rotation);
            func_L11_00338628(object->position, state->anchor, 0);
            if (!func_L11_00447BE8(object->fB8, object->position) && object->f95 != 8) {
                state->health = 0;
                func_L11_0032DBF8(object, 18, 8);
            }
        }
        if (object->stage >= 2 && state->f2E == 2)
            func_L11_0044B398(object, 1, 1);
        func_L11_0044A820(object);
    }
}

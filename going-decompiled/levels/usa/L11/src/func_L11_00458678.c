/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00458678, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef signed short s16;
typedef signed int s32;

/* The actor object: position, stage and flags at fixed offsets, its state block behind `state`. */
typedef struct {
    u8 pad00[0x10];
    float position[4];
    u8 stage;
    u8 pad21[0x10];
    u8 active;
    u8 pad32[0x68 - 0x32];
    void *state;
    u8 pad6C[3];
    u8 f6F;
    u8 pad70[0xAA - 0x70];
    s16 id;
} JobaObject;

/* State block of the point-ring effect: up to eight 16-byte points, then its tuning values. */
typedef struct {
    float points[8][4];
    s32 count;
    float minRadius;
    float maxRadius;
    float maxElevation;
    float minElevation;
    float f94;
    float f98;
    float f9C;
    float fA0;
    float fA4;
} JobaRingState;

/* L11's own function and its copies of boot and level-00 code, at their
 * v2.00 places in this level. */
extern float func_L11_003013B8(void *, void *);
extern void func_L11_00337298(void *, float, float, float);
extern void func_L11_003011E0(void *, void *, void *);
extern float func_L11_003022F0(float, float);
extern float func_L11_0032DDC0(float, float);
extern void func_L11_002F9698(void *, char *);
extern float func_L11_003018D0(float a, float b);
extern float func_L11_0032DD60(float a, float b);
extern float func_L11_0032DE28(void);
extern float func_L11_003013F0(void *p, float *v);
extern void func_L11_00301210(float *out, float *a, float *b);
extern void func_L11_004588F8(void);

/**
 * func_L11_00458678 - keep the ring's points inside the configured radial
 * and angular limits.
 *
 * @obj: the ring actor; its JobaRingState is obj->state.
 *
 * Stage 0 (first call): clamp count to 8, move to stage 1, and seed each
 * point with func_L11_00337298(point, r, a, e) around obj->position, where r
 * comes from func_L11_0032DD60(minRadius, maxRadius) and a, e from
 * func_L11_0032DE28().
 * Stage 1: hand func_L11_004588F8 and obj to func_L11_002F9698, then for each
 * point recompute azimuth and elevation (func_L11_003018D0) relative to
 * obj->position, step them with func_L11_003022F0 by f9C and by
 * func_L11_0032DDC0(fA4, fA0) (degrees, times pi/180), clamp the elevation
 * to [minElevation, maxElevation], step the radius by
 * func_L11_0032DDC0(f98/60, f94/60) and clamp it to [minRadius, maxRadius],
 * and rebuild the point with func_L11_00337298 and func_L11_003011E0.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00453358.c, function
 * func_L11_00456C20 (v1.01 address), author commit 3cd8af2d (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged (verbatim but for
 * names; no device); edits:
 *   - name func_L11_00456C20 -> func_L11_00458678 (our split name at the
 *     v2.00 place);
 *   - callees respelled to the targets of the v2.00 ROM's own relocations
 *     (relverify, task #2553): func_00282CC0 -> func_L11_003011E0,
 *     func_00282CF0 -> func_L11_00301210, func_00282E48 -> func_L11_003013B8,
 *     func_00282E80 -> func_L11_003013F0, func_00283248 -> func_L11_003018D0,
 *     func_00283BE0 -> func_L11_003022F0, func_002A7878 -> func_L11_0032DD60,
 *     func_002A78D8 -> func_L11_0032DDC0, func_002A7940 -> func_L11_0032DE28,
 *     func_002AF000 -> func_L11_00337298, func_L00_002E6EF8 ->
 *     func_L11_002F9698, func_L11_00456EA0 -> func_L11_004588F8;
 *   - the audit's `long long` -> `long` respelling (RULING #13197 class,
 *     needed because cc1 s136 refuses the file's other `long long` uses) is
 *     NOT applied: this one-function TU carries no `long long`;
 *   - `#include "common.h"` dropped; the typedefs used written out; the
 *     JobaObject and JobaRingState typedefs carried verbatim; the upstream
 *     one-line comment folded into this one; the file's other functions and
 *     their externs not carried.
 */
void func_L11_00458678(JobaObject *obj)
{
    JobaRingState *state = obj->state;
    float *point;
    int i, j;
    float scratch[4];
    float radius, azimuth, elevation, step;

    if (obj->stage == 0) {
        if (state->count > 8)
            state->count = 8;
        obj->stage = 1;
        for (i = 0; i < state->count; i++) {
            float newRadius, newAzimuth;

            newRadius = func_L11_0032DD60(state->minRadius, state->maxRadius);
            newAzimuth = func_L11_0032DE28();
            func_L11_00337298(state->points[i], newRadius, newAzimuth, func_L11_0032DE28());
            func_L11_003011E0(state->points[i], obj->position, state->points[i]);
        }
    }

    if (obj->stage == 1) {
        func_L11_002F9698(func_L11_004588F8, (char *)obj);
        for (j = 0; j < state->count; j++) {
            point = state->points[j];
            func_L11_00301210(scratch, obj->position, point);
            azimuth = func_L11_003018D0(point[0] - obj->position[0], point[1] - obj->position[1]);
            elevation = func_L11_003018D0(func_L11_003013F0(obj->position, point),
                                      point[2] - obj->position[2]);
            azimuth = func_L11_003022F0(azimuth, state->f9C * 0.017453294f);
            elevation = func_L11_003022F0(elevation,
                                      func_L11_0032DDC0(state->fA4, state->fA0) * 0.017453294f);
            if (state->maxElevation < elevation)
                elevation = state->maxElevation;
            if (elevation < state->minElevation)
                elevation = state->minElevation;

            radius = func_L11_003013B8(obj->position, point);
            step = func_L11_0032DDC0(state->f98 * 0.0166666675f, state->f94 * 0.0166666675f);
            radius += step;
            if (state->maxRadius < radius)
                radius = state->maxRadius;
            if (radius < state->minRadius)
                radius = state->minRadius;

            func_L11_00337298(point, radius, azimuth, elevation);
            func_L11_003011E0(point, obj->position, point);
        }
    }
}

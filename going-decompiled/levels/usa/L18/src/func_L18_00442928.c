/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00442928, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed int s32;

extern void func_L18_00309670(float *, const float *, const float *);
extern void func_L18_00309928(char *, char *, float);
extern s32 func_L18_0037C8D8(s32, s32, void *);
extern void *func_L18_00338E08(void *, s32, s32);
extern float func_L18_00338E68(void *, void *);
extern void func_L18_003368C0(void *, int, int);
extern int func_L18_0030A7A8(float);
extern int func_L18_003458A0(void *);
extern void func_L18_0037C5D8(int);
extern void func_L18_0032C688(void *);
extern void func_L18_00442BB0(void *);
extern void func_L18_00341FB8(void *, int, void *, float, float, float, float, int);
extern void func_L18_00335DB0(void *, int, int);
extern char g_listenerPosHistory[], g_heroPos[];
extern char g_listenerPosHistory[];
typedef struct { char pad[0x74]; unsigned char active; char gap[0x13]; void *owner; } DamoselSoundAnchor;
extern char g_heroPos[];
extern void func_L18_0033B658(void *, void *, void *, float, float, int, int, int,
                         float, float, float, float, int, float, int, int, int, int, int);
extern void func_L18_00341040(void *, void *, void *, void *, float);
extern void func_L18_00335DB0(void *, int, int), func_L18_0032C688(void *);

/**
 * func_L18_00442928 - Damosel damage controller: release its sound anchor,
 * take hits, explode at zero health.
 *
 * @object: the object; its state block is at +0x68 (health +0x20, sound
 * anchor index +0x100).
 *
 * State 0 clears the anchor index, sets flag 0x1000 in +0x34, draws two
 * random bytes (+0x48, +0x4A) and enters state 1. State 1: when
 * func_L18_003458A0 says so, frees the anchor (its 112-byte entry in
 * g_listenerPosHistory, while active and still owned by this object). A hit
 * (func_L18_00338E08, mask 0x10000) from an owner of class 0xB69 or 0xF9E is
 * ignored; any other takes its damage off the health, and at or below zero
 * the object explodes (func_L18_0033B658), stops its sound, is pushed away
 * from g_heroPos (func_L18_00341040, func_L18_00341FB8), is set to state 3
 * and is released (func_L18_0032C688). Without a hit, func_L18_00442BB0 runs.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_00433460.c,
 * function func_L18_00441438 (v1.01 address), author commit 132104a3
 * (Louis-Philippe Le Sieur); b5e84378 (same author) later only respelled one
 * callee name (damoselSetState -> func_002A7750).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_00441438 -> func_L18_00442928 (our split name at the v2.00 place);
 *   - D_00188660 -> g_listenerPosHistory (our boot name at that address);
 *   - D_00189EA0 -> g_heroPos (our boot name at that address);
 *   - func_00282CF0 -> func_L18_00309670 (the v2.00 relocation target, our name there);
 *   - func_00282F20 -> func_L18_00309928 (the v2.00 relocation target, our name there);
 *   - func_00283CF0 -> func_L18_0030A7A8 (the v2.00 relocation target, our name there);
 *   - func_0029F338 -> func_L18_0032C688 (the v2.00 relocation target, our name there);
 *   - func_002A6C58 -> func_L18_00335DB0 (the v2.00 relocation target, our name there);
 *   - func_002A7750 -> func_L18_003368C0 (the v2.00 relocation target, our name there);
 *   - func_002A9640 -> func_L18_00338E08 (the v2.00 relocation target, our name there);
 *   - func_002A96A0 -> func_L18_00338E68 (the v2.00 relocation target, our name there);
 *   - func_002ABBB8 -> func_L18_0033B658 (the v2.00 relocation target, our name there);
 *   - func_002AFEB8 -> func_L18_00341040 (the v2.00 relocation target, our name there);
 *   - func_002B0A68 -> func_L18_00341FB8 (the v2.00 relocation target, our name there);
 *   - func_002B3868 -> func_L18_003458A0 (the v2.00 relocation target, our name there);
 *   - func_002E59F8 -> func_L18_0037C5D8 (the v2.00 relocation target, our name there);
 *   - func_002E5CF8 -> func_L18_0037C8D8 (the v2.00 relocation target, our name there);
 *   - func_L00_00426478 -> func_L18_00442BB0 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them), and 11 repeated identical extern line(s) of the upstream file collapsed to one.
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the upstream file as a whole fails on this arm;
 * this function's own text has no `long long` (nor s64/u64), so alone in its
 * TU it needs no respelling, and none is made.
 */
void func_L18_00442928(unsigned char *object)
{
    char *state = *(char **)(object + 0x68);
    char *hit;
    char *owner;
    int excluded;
    float zero, damage;
    float direction[4];
    switch (object[0x20]) {
    case 0:
        *(int *)(state + 0x100) = -1;
        *(unsigned short *)(object + 0x34) |= 0x1000;
        state[0x48] = func_L18_0030A7A8(15.0f);
        state[0x4a] = func_L18_0030A7A8(6.0f);
        func_L18_003368C0(object, 1, -1);
        break;
    case 1:
        if (func_L18_003458A0(object)) {
            if (*(int *)(state + 0x100) != -1 && ((DamoselSoundAnchor *)(g_listenerPosHistory + *(int *)(state + 0x100) * 112))->owner == object && ((DamoselSoundAnchor *)(g_listenerPosHistory + *(int *)(state + 0x100) * 112))->active)
                func_L18_0037C5D8(*(int *)(state + 0x100));
            *(int *)(state + 0x100) = -1;
        }
        hit = func_L18_00338E08(object, 0x10000, 0);
        if (!hit) goto intact;
        owner = *(char **)(hit + 0x20);
        excluded = 0;
        if (owner && (*(short *)(owner + 0xaa) == 0xb69 || *(short *)(owner + 0xaa) == 0xf9e)) excluded = 1;
        if (excluded) break;
        zero = 0.0f;
        damage = func_L18_00338E68(object, hit);
        object[0xa8] = 255;
        *(float *)(state + 0x20) -= damage;
        if (*(float *)(state + 0x20) <= zero) {
            func_L18_0033B658(object, object + 0x10, 0, zero, zero, 10, 3, 9,
                         4.0f, 2.0f, 9.0f, zero, -1, 15.0f, 1, 1, -1, 0, 0x810001);
            func_L18_0037C8D8(0, 0, object);
            func_L18_00309670(direction, (float *)(object + 0x10), (float *)g_heroPos);
            func_L18_00341040(object, direction, direction, 0, zero);
            func_L18_00309928((char *)direction, (char *)direction, 0.10000001f);
            func_L18_00341FB8(object, 0x1fff, direction, 3.0f, 6.0f, 3.0f, 6.0f, 255);
            func_L18_00335DB0(object, 0, 3);
            func_L18_0032C688(object);
        }
        break;
intact:
        func_L18_00442BB0(object);
        break;
    }
}

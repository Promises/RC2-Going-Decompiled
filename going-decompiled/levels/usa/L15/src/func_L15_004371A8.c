/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x004371A8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed int s32;
typedef float f32;

extern void func_L15_0032B098(unsigned char *, int, unsigned char *);
extern void func_L15_00381730(int, void *);
#define GF(offset) (((GornState *)state)->f[(offset) / 4])
#define GI(offset) (((GornState *)state)->i[(offset) / 4])
typedef union { float f[0x660 / 4]; int i[0x660 / 4]; } GornState;
typedef union { float f[0x80 / 4]; unsigned short h[0x80 / 2]; unsigned char b[0x80]; } GornOwnerFields;
extern void func_L15_003079C8(void *, void *, void *);
extern void func_L15_0032B340(void *, int, void *);
extern void func_L15_00307A08(void *, void *, float);
extern int func_L15_00381760(unsigned char *owner);
extern void func_L15_00382980(char *object, int flag);
extern void func_L15_00381700(int arg0, int value);
extern int func_L15_00308AE0(float);

/**
 * func_L15_004371A8 - initialise a Gorn object: counters, flags, six joint
 * vectors and eight joint records.
 *
 * @object: the object; its state block (0x660 bytes) is at +0x68.
 * Return: nothing.
 *
 * Sets the owner record's +0x148 to (int)150.0, the state's +0x180 to 150.0
 * and the object's state, flags and scale +0x2C (0.675). It reads joints
 * 14..19 into six vectors at state +0x280..+0x3C0 (func_L15_0032B340), makes
 * them relative to the position (the second three also to the first three)
 * divided by the scale, and joints 12..19 into records at state +0x460..
 * +0x620 (func_L15_0032B098), several of whose fields are set from the
 * vectors times 1024.0 or to 1.0. It
 * ends with func_L15_00381700(object, 0xFFFF) and func_L15_00381730(object, 1).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_004328E8.c, function func_L15_004353E8 (v1.01 address), author commit 4912fc03 (bigbeno37), which
 * brought Louis-Philippe Le Sieur's nonmatching draft
 * nonmatching/levels/func_L15_004353E8.c (author commit 4ba068b1, earlier)
 * into src: 91 of the 98 definition lines are that draft's text; 4 are
 * bigbeno37's edits to the draft (54dba8e9), 2 came with 4d6e47bb and 1 with
 * 4912fc03 (git blame -M -C), so both are credited (RULING #13427 rev 2,
 * #13541 rev 2).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_004353E8 -> func_L15_004371A8 (our split name at the v2.00 place);
 *   - func_00282CF0 -> func_L15_003079C8 (the v2.00 relocation target, our name there);
 *   - func_00282D30 -> func_L15_00307A08 (the v2.00 relocation target, our name there);
 *   - func_00283CF0 -> func_L15_00308AE0 (the v2.00 relocation target, our name there);
 *   - func_0029F978 -> func_L15_0032B098 (the v2.00 relocation target, our name there);
 *   - func_0029FC20 -> func_L15_0032B340 (the v2.00 relocation target, our name there);
 *   - func_002ED688 -> func_L15_00381760 (the v2.00 relocation target, our name there);
 *   - func_L05_00379C68 -> func_L15_00381700 (the v2.00 relocation target, our name there);
 *   - func_L05_0037AEC0 -> func_L15_00382980 (the v2.00 relocation target, our name there);
 *   - func_L15_003804B8 -> func_L15_00381730 (the v2.00 relocation target, our name there);
 *   - upstream's file declares func_L15_00308AE0 again further down; the 2 repeats are
 *   dropped, the first declaration kept;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
extern void func_L15_004371A8(char *);

void func_L15_004371A8(char *object)
{
    GornOwnerFields *fields = (GornOwnerFields *)object;
    char *state = *(char **)(object + 0x68);
    char *record;
    char *a = state + 0x2B0;
    char *b = state + 0x2F0;
    char *c = state + 0x330;
    char *d = state + 0x370;
    char *e = state + 0x3B0;
    char *f = state + 0x3F0;
    char *position = object + 0x10;
    record = (char *)func_L15_00381760((unsigned char *)object);
    *(int *)(record + 0x148) = func_L15_00308AE0(150.0f);
    func_L15_00382980(object, 1);
    GF(0x180) = 150.0f;
    object[0x20] = 1;
    *(unsigned char *)(object + 0x30) = 255;
    *(unsigned short *)(object + 0x32) = 0x1FF;
    *(float *)(object + 0x2C) = 0.675f;
    *(unsigned short *)(object + 0x34) |= 0x100;
    *(unsigned short *)(object + 0x7C) = 63;
    GI(0x458) = -1;
    GI(0x268) = 1;
    GI(0x270) = 0;
    GI(0x27C) = 0;
    GI(0x45C) = 0;
    GI(0x278) = 0;
    func_L15_0032B340(object, 14, state + 0x280);
    func_L15_0032B340(object, 16, state + 0x2C0);
    func_L15_0032B340(object, 18, state + 0x300);
    func_L15_0032B340(object, 15, state + 0x340);
    func_L15_0032B340(object, 17, state + 0x380);
    func_L15_0032B340(object, 19, state + 0x3C0);
    func_L15_003079C8(a, a, position);
    func_L15_003079C8(d, d, position);
    func_L15_003079C8(d, d, a);
    func_L15_00307A08(a, a, 1.0f / fields->f[0x2C / 4]);
    func_L15_00307A08(d, d, 1.0f / fields->f[0x2C / 4]);
    func_L15_003079C8(b, b, position);
    func_L15_003079C8(e, e, position);
    func_L15_003079C8(e, e, b);
    func_L15_00307A08(b, b, 1.0f / fields->f[0x2C / 4]);
    func_L15_00307A08(e, e, 1.0f / fields->f[0x2C / 4]);
    func_L15_003079C8(c, c, position);
    func_L15_003079C8(f, f, position);
    func_L15_003079C8(f, f, c);
    func_L15_00307A08(c, c, 1.0f / fields->f[0x2C / 4]);
    func_L15_00307A08(f, f, 1.0f / fields->f[0x2C / 4]);
    func_L15_0032B098((unsigned char *)object, 12, (unsigned char *)(state + 0x460));
    func_L15_0032B098((unsigned char *)object, 13, (unsigned char *)(state + 0x4A0));
    func_L15_0032B098((unsigned char *)object, 14, (unsigned char *)(state + 0x4E0));
    func_L15_0032B098((unsigned char *)object, 15, (unsigned char *)(state + 0x520));
    func_L15_0032B098((unsigned char *)object, 16, (unsigned char *)(state + 0x560));
    func_L15_0032B098((unsigned char *)object, 17, (unsigned char *)(state + 0x5A0));
    func_L15_0032B098((unsigned char *)object, 18, (unsigned char *)(state + 0x5E0));
    func_L15_0032B098((unsigned char *)object, 19, (unsigned char *)(state + 0x620));
    GF(0x510) = GF(0x2B0) * 1024.0f;
    GF(0x514) = GF(0x2B4) * 1024.0f;
    GF(0x518) = GF(0x2B8) * 1024.0f;
    GF(0x550) = GF(0x370) * 1024.0f;
    GF(0x554) = GF(0x374) * 1024.0f;
    GF(0x558) = GF(0x378) * 1024.0f;
    GF(0x590) = GF(0x2F0) * 1024.0f;
    GF(0x594) = GF(0x2F4) * 1024.0f;
    GF(0x598) = GF(0x2F8) * 1024.0f;
    GF(0x5D0) = GF(0x3B0) * 1024.0f;
    GF(0x5D4) = GF(0x3B4) * 1024.0f;
    *(unsigned char *)(state + 0x4E3) = 1;
    *(unsigned char *)(state + 0x523) = 1;
    *(unsigned char *)(state + 0x563) = 1;
    *(unsigned char *)(state + 0x5A3) = 1;
    GF(0x4EC) = 1.0f;
    GF(0x51C) = 1.0f;
    GF(0x52C) = 1.0f;
    GF(0x55C) = 1.0f;
    GF(0x56C) = 1.0f;
    GF(0x59C) = 1.0f;
    GF(0x5AC) = 1.0f;
    GF(0x5D8) = GF(0x3B8) * 1024.0f;
    GF(0x5DC) = 1.0f;
    *(unsigned char *)(state + 0x5E3) = 1;
    GF(0x610) = GF(0x330) * 1024.0f;
    GF(0x614) = GF(0x334) * 1024.0f;
    GF(0x618) = GF(0x338) * 1024.0f;
    GF(0x650) = GF(0x3F0) * 1024.0f;
    GF(0x654) = GF(0x3F4) * 1024.0f;
    GF(0x658) = GF(0x3F8) * 1024.0f;
    *(unsigned char *)(state + 0x623) = 1;
    GF(0x5EC) = 1.0f;
    GF(0x61C) = 1.0f;
    GF(0x62C) = 1.0f;
    GF(0x65C) = 1.0f;
    GI(0x444) = 300;
    GI(0x440) = 0;
    func_L15_00381700((int)object, 0xFFFF);
    func_L15_00381730((int)object, (void *)1);
}

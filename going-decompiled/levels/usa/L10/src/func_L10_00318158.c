/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00318158, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* The level's copies of boot code, named by their v2.00 places. */
extern void func_L10_003098C8(void *, void *, void *);
extern void func_L10_00309B78(void *, void *, float);
extern void func_L10_00317EF0(void);
extern void func_L10_00375330(char *);
extern void func_L10_0033FCE8(char *, char *, int, int, float);
extern void func_L10_0030A7F8(char *, char *);
extern void func_L10_0030A660(char *, char *);
extern void func_L10_0033F1A8(char *, char *, float, float, float);
extern void func_L10_00427670(char *, int, int, int, int, int, int, int, int, int, int, float);
extern int func_L10_00336448(int kind);
extern float func_L10_00336540(float, float);
extern char *func_L10_0041D090(char *, char *, char *, int, int, int, int, int, float);

typedef struct {
    char pad[0x30];
    int mode, timer;
    char gap[0xC];
    short count;
    char gap2[0x146];
    char *objects[3];
} Scene10;
/* The level's scene record at 0x001BD300. No label starts there in our split
   (it lies inside a larger data label), so this extern is ADDRESS-NAMED and
   the level ELF values it from its name (RULING #13291 §2). */
extern Scene10 D_L10_001BD300;
/* Spawn offsets, 32 bytes per entry (our split label). */
extern char D_L10_00296E80[];
/* The save block's first word, the current level number: g_playerProgress
   at 0x001A79F8, the boot's name there and L10's boot-name map's.
   ADDRESSING-MODEL DEVICE (RULING #8620) on this declaration: upstream's
   `sda` (ignored by this cc1) is respelled `section(".sdata")`, so cc1
   prints each load as a one-instruction macro that the assembler expands at
   -G0 to the absolute lui/lw pair the ROM has. Ablation (task #2657,
   upstream's own `sda`, same arm and flags): 231 words where the disc has
   232, 205 differing. It moves no data and emits nothing. */
extern int g_playerProgress __attribute__((section(".sdata")));

/**
 * func_L10_00318158 - per-frame scene update of Hrugis Cloud: object flags,
 * the class 0xFA0 effect and the intro sparks.
 *
 * After func_L10_00317EF0, for each of the scene's objects: class 0xE29 gets
 * flag 0x800 at +0x34, class 0xD54 goes to func_L10_00375330, and class
 * 0xFA0, unless the level number is 0x33, gets func_L10_0033FCE8 on its state
 * (+0x68) + 0xC. Then, only in level 10 with the scene's mode 0, the third
 * object emits: at timer 0x195 two bursts through func_L10_00427670 at
 * offsets func_L10_0033F1A8 computes from the object's matrix; and while the
 * timer is in [0x187, 0x200) or >= 0x327, unless func_L10_00336448(4)
 * fires, one spark from a random entry of D_L10_00296E80, aimed at a jittered
 * destination, with a random sign, through func_L10_0041D090.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/us2/levels/10_hrugis_cloud/L10_00318158.c,
 * function func_US2_L10_00318158 (v2.00 address), author commit 6e87b85c
 * (bigbeno37). Upstream's file comment calls it the v2.00 copy of
 * func_L10_00317528 written as its Feltzin sibling func_US2_L05_00315A58 with
 * a new test of the level number against 0x33. Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged, the edits, each name to the target of the v2.00
 * disc's own relocation at this place (task #2657 relverify):
 *   - func_US2_L10_00318158 -> func_L10_00318158 (our split name, same place);
 *   - func_00282CF0 -> func_L10_003098C8, func_00282F20 -> func_L10_00309B78,
 *     func_L00_002FC980 -> func_L10_00317EF0, func_US2_002E72D8 ->
 *     func_L10_00375330, func_L00_00326048 -> func_L10_0033FCE8,
 *     func_00283AF0 -> func_L10_0030A7F8, func_00283958 -> func_L10_0030A660,
 *     func_L01_0031A278 -> func_L10_0033F1A8, func_L05_00424990 ->
 *     func_L10_00427670, func_002A77E0 -> func_L10_00336448, func_002A78D8
 *     -> func_L10_00336540, func_L05_00419CC8 -> func_L10_0041D090 (our
 *     split names at the v2.00 targets);
 *   - D_001A79F0 -> g_playerProgress (the boot's and L10's boot-name map's
 *     name at 0x001A79F8); D_L10_00296F00 -> D_L10_00296E80 (our split
 *     label); D_L10_001BD380 -> D_L10_001BD300, an address-named extern for a
 *     target inside a larger split data label (RULING #13291 §2);
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on g_playerProgress's extern declaration (RULING #8620; the audit's
 *     `resp` variant, NOTE #13281);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped (the
 *     function uses no typedef of theirs); upstream's comment folded into
 *     this one.
 */
void func_L10_00318158(void)
{
    float pos[4] __attribute__((aligned(16)));
    union {
        float matrix[12];
        struct { float dest[4], delta[4]; } vectors;
    } scratch __attribute__((aligned(16)));
    int i;
    char *obj;
    func_L10_00317EF0();
    for (i = 0; i < D_L10_001BD300.count; i++) {
        char *current = D_L10_001BD300.objects[i];
        if (current && *(short *)(current + 0xAA) == 0xE29)
            *(unsigned short *)(current + 0x34) |= 0x800;
        if (*(short *)(current + 0xAA) == 0xD54) func_L10_00375330(current);
        if (current) {
            int class_id = 0xFA0;
            if (*(short *)(current + 0xAA) == class_id && g_playerProgress != 0x33) {
                char *state = *(char **)(current + 0x68);
                if (state) func_L10_0033FCE8(current, state + 0xC, 0x80AAAAD2, 0x80396464, 0.06108653545379639f);
            }
        }
    }
    if (g_playerProgress != 10 || D_L10_001BD300.mode) return;
    obj = D_L10_001BD300.objects[2];
    if (D_L10_001BD300.timer == 0x195) {
        func_L10_0030A7F8(*(char **)(obj + 0x58) + 0x10, (char *)scratch.matrix);
        func_L10_0030A660((char *)scratch.matrix, obj + 0xC0);
        func_L10_0033F1A8(obj, (char *)pos, 4.0f, 3.0f, 5.0f);
        func_L10_00427670((char *)pos, 1, 25, 20, 20, 20, 45, 1, 1, 1, 0, 1.5f);
        func_L10_0033F1A8(obj, (char *)pos, 9.0f, 4.0f, 4.0f);
        func_L10_00427670((char *)pos, 35, 50, 30, 30, 30, 50, 1, 1, 1, 1, 1.0f);
    }
    if (((unsigned int)(D_L10_001BD300.timer - 0x187) < 0x79 || D_L10_001BD300.timer >= 0x327) && !func_L10_00336448(4)) {
        float sign = 1.0f;
        float *entry;
        if (func_L10_00336448(2)) sign = -1.0f;
        entry = (float *)(D_L10_00296E80 + func_L10_00336448(3) * 32);
        func_L10_0033F1A8(obj, (char *)pos, entry[0], entry[1], entry[2]);
        func_L10_0033F1A8(obj, (char *)scratch.vectors.dest, entry[4], entry[5], entry[6]);
        scratch.vectors.dest[0] += func_L10_00336540(0.5f, 1.5f);
        scratch.vectors.dest[1] += func_L10_00336540(0.5f, 1.5f);
        scratch.vectors.dest[2] += func_L10_00336540(0.5f, 1.5f);
        func_L10_003098C8((char *)scratch.vectors.delta, (char *)scratch.vectors.dest, (char *)pos);
        func_L10_00309B78((char *)scratch.vectors.delta, (char *)scratch.vectors.delta, 1.1666667461395264f);
        func_L10_0041D090(obj, (char *)pos, (char *)scratch.vectors.delta, 0x46BE20, 0x327814, 1, 0, 1, sign);
    }
}

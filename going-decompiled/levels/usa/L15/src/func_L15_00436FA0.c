/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00436FA0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed int s32;
typedef float f32;

extern float func_L15_00308AD0(int);
extern int func_L15_00308AE0(float);
extern float func_L15_00334600(float, float);
extern float func_L15_00308988(float, float);
extern void func_L15_0032B3E0(void *obj, s32 a, void *out);
typedef struct GornEffectCamera {
    char pad00[0x140];
    float position[4];
} GornEffectCamera;
extern GornEffectCamera g_cameraState;
extern unsigned char *D_L15_001FF42C;
extern float func_L15_00308038(float, float);
extern float func_L15_00334660(float, float);
extern void func_L15_0033D718(void *, float, float, float);
extern unsigned char *func_L15_0034E4D0(float, float, float, float,
                                   void *, int, void *, int, int);
extern int func_L15_003345A8(int, int);

/**
 * func_L15_00436FA0 - spawn one particle at a joint of an object, sent off
 * at a random angle around the object-to-camera heading.
 *
 * @object: the object (position +0x10).
 * @joint: the joint whose position seeds the particle.
 * @unused: not read.
 * @radius: scales the scatter, speed and size.
 * Return: nothing.
 *
 * The start point is the joint position plus func_L15_00334660(0, radius)
 * per axis; the heading is func_L15_00308038 of the x/y offset from the
 * object to g_cameraState's position, turned by func_L15_00334660(0, pi/2),
 * with a tilt of func_L15_00334660(0, 1.3963) and a speed between
 * radius/120 and radius/60. A particle from func_L15_0034E4D0 gets byte +3
 * = 0x48, halfword +0xA = func_L15_003345A8(40, 70) and byte +2 from
 * D_L15_001FF42C.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_004328E8.c, function func_L15_004351E0 (v1.01 address), author commit 5acf9869 (Flavius); the
 * definition line was changed later by DrLegitamate (bde5ccd6), which
 * added the `int unused` parameter (git blame -M -C), so both are credited.
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_004351E0 -> func_L15_00436FA0 (our split name at the v2.00 place);
 *   - D_L15_001B9680 -> g_cameraState: the name symbol_addrs/L15_boot_names.txt gives at
 *   exactly 0x001B9600, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - D_L15_001FF4AC -> D_L15_001FF42C: an address-named extern (RULING #13291 section 2) at
 *   0x001FF42C, inside the split label D_L15_001F0000 (+0xF42C); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L15_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2677);
 *   - func_00283248 -> func_L15_00308038 (the v2.00 relocation target, our name there);
 *   - func_00283B98 -> func_L15_00308988 (the v2.00 relocation target, our name there);
 *   - func_00283CE0 -> func_L15_00308AD0 (the v2.00 relocation target, our name there);
 *   - func_00283CF0 -> func_L15_00308AE0 (the v2.00 relocation target, our name there);
 *   - func_0029FCC0 -> func_L15_0032B3E0 (the v2.00 relocation target, our name there);
 *   - func_002A7820 -> func_L15_003345A8 (the v2.00 relocation target, our name there);
 *   - func_002A7878 -> func_L15_00334600 (the v2.00 relocation target, our name there);
 *   - func_002A78D8 -> func_L15_00334660 (the v2.00 relocation target, our name there);
 *   - func_002AF000 -> func_L15_0033D718 (the v2.00 relocation target, our name there);
 *   - func_002BDCE8 -> func_L15_0034E4D0 (the v2.00 relocation target, our name there);
 *   - upstream's file declares func_L15_00308988, func_L15_00308AD0, func_L15_00308AE0, func_L15_0032B3E0, func_L15_00334600 again further down; the 7 repeats are
 *   dropped, the first declaration kept;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L15_00436FA0(char *object, int joint, int unused, float radius) {
    float initial[4], point[4], velocity[4];
    float heading, speed, tilt, yaw;
    unsigned char *particle;
    unsigned char *data;

    func_L15_0032B3E0(object, joint, initial);
    heading = func_L15_00308038(g_cameraState.position[0] - *(float *)(object + 0x10),
                            g_cameraState.position[1] - *(float *)(object + 0x14));
    point[0] = initial[0] + func_L15_00334660(0, radius);
    point[1] = initial[1] + func_L15_00334660(0, radius);
    point[2] = initial[2] + func_L15_00334660(0, radius);
    speed = func_L15_00334600(radius * 0.008333333767950535f,
                          radius * 0.01666666753590107f);
    tilt = func_L15_00334660(0, 1.396263599395752f);
    yaw = func_L15_00308988(heading, func_L15_00334660(0, 1.5707964897155762f));
    func_L15_0033D718(velocity, speed, yaw, tilt);
    particle = func_L15_0034E4D0(0, 1.0f, 1.01f, radius * 210000.0f,
                            point, -1, velocity, 0x307F7F7F, 0);
    if (particle != 0) {
        particle[3] = 0x48;
        *(short *)(particle + 0xA) = func_L15_003345A8(40, 70);
        particle[9] = func_L15_00308AE0(4.0f) + 0x60;
        data = particle + 0x20;
        *(float *)(data + 0x1C) = func_L15_00308AD0(func_L15_003345A8(1, 10));
        *(int *)(data + 4) = 3;
        data[0xA] = 0x7F;
        data[0xB] = particle[0xA];
        particle[2] = *D_L15_001FF42C;
    }
}

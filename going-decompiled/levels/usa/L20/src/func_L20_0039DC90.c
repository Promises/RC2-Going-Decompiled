/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x0039DC90, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_00302FF0(char *local, char *first, char *second);
extern float func_L20_00303680(float a, float b);
extern void func_L20_00303030(char *dst, char *src, float scale);
extern float func_L20_00330988(float a, float b);
extern float func_L20_003309E8(float a, float b);
extern int func_L20_00304128(float f);
extern float func_L20_00304118(int v);
extern float func_L20_00303FD0(float a, float b);
extern void func_L20_00339838(float *p, float a, float b, float c);
extern char *func_L20_0034B100(char *p, float fa, float fb, float fc, float fd, int v, char *local,
                            unsigned int color, int flag);
extern void func_L20_0034ED90(float *out, float *dir, int a, int b, unsigned int color, int id,
                           float fa, float fb, float fc, float fd, float fe);
extern char g_cameraPos[];
extern char g_heroFacingDir[];

/**
 * func_L20_0039DC90 - spawn the 20-particle burst of a Yeedil effect at a point.
 *
 * Takes the heading from g_cameraPos to the point (func_L20_00303680 of
 * their difference). Twenty times: a smoke particle with a random push along
 * g_heroFacingDir, size, speed and lifetime (func_L20_0034B100; its fields
 * +3, +9, +10 and the +0x20 block set when it is created), then a spark
 * (func_L20_0034ED90) with a random direction within 80 degrees of that
 * heading (func_L20_00339838), both jittered around the point.
 *
 * @p: the point (x, y, z).
 * Return: nothing.
 *
 * Non-obvious: every random draw is a separate call, in upstream's order; the
 * float constants are emitted inline (lui/mtc1) at -G0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C8C8.c, function func_L20_0039C8C8 (v1.01
 * address), author commit 8821a1ae (Flavius). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0039C8C8 -> func_L20_0039DC90; D_0018A0E0 ->
 *     g_heroFacingDir; D_L20_001B9FC0 -> g_cameraPos; func_00282CF0 ->
 *     func_L20_00302FF0; func_00282D30 -> func_L20_00303030; func_00283248 ->
 *     func_L20_00303680; func_00283B98 -> func_L20_00303FD0; func_00283CE0 ->
 *     func_L20_00304118; func_00283CF0 -> func_L20_00304128; func_002A7878 ->
 *     func_L20_00330988; func_002A78D8 -> func_L20_003309E8; func_002AF000 ->
 *     func_L20_00339838; func_002BDCE8 -> func_L20_0034B100; func_002C16C0 ->
 *     func_L20_0034ED90
 *   - D_L20_001B9F40 -> g_cameraPos (task #2646): the name
 *     symbol_addrs/L20_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
void func_L20_0039DC90(float *p)
{
    float v[4];
    float dir[4];
    float out[4];
    float ang;
    float scale;
    float mag;
    float angle;
    float f;
    int id;
    int i;
    char *r;
    char *q;

    func_L20_00302FF0((char *)v, g_cameraPos, (char *)p);
    ang = func_L20_00303680(v[0], v[1]);

    for (i = 19; i >= 0; i--) {
        f = func_L20_00330988(0.2f, 3.0f);
        func_L20_00303030((char *)dir, g_heroFacingDir, -(f * 0.01666666753590107f));
        id = func_L20_00304128(func_L20_003309E8(1.0f, 2.0f));
        f = func_L20_00330988(0.3f, 0.75f) * 210000.0f;
        out[0] = p[0] + func_L20_003309E8(0.0f, 0.2f);
        out[1] = p[1] + func_L20_003309E8(0.0f, 0.2f);
        out[2] = p[2] + func_L20_003309E8(0.0f, 0.2f);
        r = func_L20_0034B100((char *)out, 0.0f, 1.0f, 1.01f, f, id, (char *)dir, 0x7F202020, 0);
        if (r != 0) {
            q = r + 32;
            *(unsigned char *)(r + 3) = 0x44;
            *(short *)(r + 10) = 0x5A;
            *(unsigned char *)(r + 9) = func_L20_00304128(4.0f) + 0x40;
            *(float *)(q + 0x1C) = func_L20_00304118(5);
            *(int *)(q + 4) = 3;
            *(unsigned char *)(q + 0xA) = 0x30;
            *(unsigned char *)(q + 0xB) = *(unsigned char *)(r + 10);
        }
        scale = func_L20_00330988(1.0f, 2.0f);
        mag = func_L20_00330988(0.0f, 1.396263599395752f);
        angle = func_L20_00303FD0(ang, func_L20_003309E8(0.0f, 1.396263599395752f));
        func_L20_00339838(dir, scale * 0.01666666753590107f, angle, mag);
        out[0] = p[0] + func_L20_003309E8(0.0f, 0.1f);
        out[1] = p[1] + func_L20_003309E8(0.0f, 0.1f);
        out[2] = p[2] + func_L20_003309E8(0.0f, 0.1f);
        func_L20_0034ED90(out, dir, 30, 127, 0x007F7F7F, id,
                      func_L20_00330988(0.1f, 0.5f) * 210000.0f, 500.0f, 1.0f, 0.0f, 0.0f);
    }
}

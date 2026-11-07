/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x0039F2E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef float f32;

extern void *func_L20_00326280(int);
extern void func_L20_0033FA28(char *, char *);
extern char *func_L20_00339408(char *);
extern void func_L20_00327768(char *, int, char *);
extern f32 func_L20_00303FD0(f32, f32);
extern void func_L20_00328D10(void *);
extern void func_L20_00339450(void *, void *);
extern void func_L20_00339AA0(void *, u32, int, int);
extern void func_L20_00330540(void *, int, int, int);

/**
 * func_L20_0039F2E0 - create (if asked) and refresh a moby's three helper objects.
 *
 * With @arg1 non-zero, any missing helper is spawned: class 0xD0E at state
 * +0x254 (its +0x7C set to 0x3FF) and class 0xD66 at state +0x258 and +0x25C,
 * each through func_L20_00326280 + func_L20_0033FA28; a failed spawn returns
 * 0. Then the main helper is put at the moby's point 0 (func_L20_00327768),
 * its heading set from the moby's and state +0x288 (func_L20_00303FD0),
 * updated (func_L20_00328D10, func_L20_00339450) and given the moby's
 * animation when it differs. Each of the two side helpers is attached to the
 * main one (func_L20_00339AA0, slot 0 or 1, the second with 6) and updated.
 *
 * @arg0: the moby. @arg1: non-zero to spawn missing helpers.
 * Return: 1, or 0 when a spawn failed.
 *
 * Non-obvious: s4 holds a loop counter in the spawn loop and then
 * func_L20_00339408's pointer result (upstream's register-shaped locals,
 * kept).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C448.c, function func_L20_0039DF18 (v1.01
 * address), author commit 9826f942 (ReaperOfVeriod). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0039DF18 -> func_L20_0039F2E0; func_00303270 ->
 *     func_L20_00303FD0; func_00325040 -> func_L20_00326280; func_00326528 ->
 *     func_L20_00327768; func_00327AD0 -> func_L20_00328D10; func_0032F2D0 ->
 *     func_L20_00330540; func_00338198 -> func_L20_00339408; func_003381E0 ->
 *     func_L20_00339450; func_00338830 -> func_L20_00339AA0; func_0033E708 ->
 *     func_L20_0033FA28
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
int func_L20_0039F2E0(char *arg0, int arg1) {
    char *s0;
    char *s1;
    char *s2 = arg0;
    char *s3;
    char *s4;
    int cnt;
    char *p2;
    int six;
    int v;

    s3 = *(char **)(s2 + 0x68);
    if (arg1 != 0) {
        if (*(char **)(s3 + 0x254) == 0) {
            s0 = (char *)func_L20_00326280(0xD0E);
            if (s0 == 0) {
                v = 0;
                goto out;
            }
            func_L20_0033FA28(s0, s2);
            *(u16 *)(s0 + 0x7C) = 0x3FF;
            *(char **)(s3 + 0x254) = s0;
        }
        s4 = 0;
        s1 = s3 + 0x258;
        while ((int)s4 < 2) {
            if (*(u32 *)s1 == 0) {
                s0 = (char *)func_L20_00326280(0xD66);
                if (s0 == 0) {
                    v = 0;
                    goto out;
                }
                func_L20_0033FA28(s0, s2);
                *(u32 *)s1 = (u32)s0;
            }
            s4++;
            s1 += 4;
        }
    }
    s4 = (char *)func_L20_00339408(s2);
    if (*(char **)(s3 + 0x254) != 0) {
        func_L20_00327768(s2, 0, *(char **)(s3 + 0x254) + 0x10);
        *(f32 *)(*(char **)(s3 + 0x254) + 0xF8) = func_L20_00303FD0(*(f32 *)(s2 + 0xF8), *(f32 *)(s3 + 0x288));
        func_L20_00328D10(*(char **)(s3 + 0x254));
        func_L20_00339450(*(char **)(s3 + 0x254), s4);
        if (*(u8 *)(*(char **)(s3 + 0x254) + 0x43) != *(u8 *)(s2 + 0x43)) {
            func_L20_00330540(*(char **)(s3 + 0x254), *(u8 *)(s2 + 0x43), *(u8 *)(s2 + 0x41), 0xA);
        }
    }
    cnt = 0;
    six = 6;
    p2 = s3 + 0x258;
    while (cnt < 2) {
        if (*(u32 *)p2 != 0) {
            func_L20_00339AA0(*(char **)(s3 + 0x254), *(u32 *)p2, cnt, (cnt == 1) ? six : 0);
            func_L20_00339450(*(u32 *)p2, s4);
        }
        cnt++;
        p2 += 4;
    }
    v = 1;
out:
    return v;
}

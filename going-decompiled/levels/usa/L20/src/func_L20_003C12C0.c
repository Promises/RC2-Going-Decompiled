/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003C12C0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef signed short s16;
typedef signed int s32;

extern void func_L20_003265E8(char *);
extern int func_L20_00331830(int, int);
extern int func_L20_00336998(void *, int);
extern void func_L20_00331910(int, int);
extern char g_soundBankHandlesBlk[] __attribute__((nosda));
extern u8 D_0019B2E8[] __attribute__((nosda));
extern u8 D_L20_001B2D60[] __attribute__((nosda));
extern struct { u8 header[0x464]; u8 collected[0]; } D_L20_0021E2F0;
extern struct { u8 header[0x464]; u8 collected[0]; } D_L20_0021EF60;
extern s32 g_playerProgress __attribute__((sda));

/**
 * func_L20_003C12C0 - collect a Yeedil pickup once its conditions hold.
 *
 * Returns early while any of the (up to 8) gate ids at the state's start is
 * not yet satisfied (func_L20_00331830(id, -1)), while the hero's mode is
 * 0x31, or when a radius check (state +0x28) fails. Copies state +0x24 to
 * +0xBC when set. When state +0x20 is 0xFF the pickup is collected: its group
 * byte +0xB0 (+2) is recorded at its index +0xB2 in D_L20_0021E2F0, and also
 * in D_L20_0021EF60 when the group is 0xFF, or when the group is not yet set
 * in D_L20_001B2D60 but is in this planet's row of D_0019B2E8 (row
 * g_playerProgress); then the moby is freed (func_L20_003265E8). Otherwise
 * state +0x20 becomes the moby's state and state +0x2C, when set, is passed
 * to func_L20_00331910(id, 1).
 *
 * @arg0: the pickup moby; its +0x68 points at the state.
 * Return: nothing.
 *
 * Non-obvious: D_0019B2E8 is the boot's g_platinumBoltFlags+0x70, so it is an
 * address-named extern valued from its name (RULING #13291 section 2);
 * upstream's `nosda`/`sda` attribute text is kept (this cc1 ignores it, with
 * a warning).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C448-g8.c, function func_L20_003BFEB8 (v1.01
 * address), author commit 8422a726 (Fromnowhere). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_003BFEB8 -> func_L20_003C12C0; D_00189E20 ->
 *     g_soundBankHandlesBlk; D_001A79F0 -> g_playerProgress; D_L20_001B2DE0 ->
 *     D_L20_001B2D60; D_L20_0021E340 -> D_L20_0021E2F0; D_L20_0021EFB0 ->
 *     D_L20_0021EF60; func_0029F338 -> func_L20_003265E8; func_002A8508 ->
 *     func_L20_00331830; func_002ACBC8 -> func_L20_00336998; func_L02_0031F858
 *     -> func_L20_00331910
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 *   - the file-local u8/s16/s32 typedefs (upstream's) carried with it
 */
void func_L20_003C12C0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    s32 *p = (s32 *)state;
    s32 count;
    s32 c;
    char *hero;

    for (count = 0; count < 8; count++) {
        if (*p != -1) {
            if (func_L20_00331830(*p, -1) != 0) {
                return;
            }
            p++;
        }
    }
    hero = g_soundBankHandlesBlk;
    if (*(s32 *)(hero + 0x2294) == 0x31) {
        return;
    }
    if (*(s32 *)(state + 0x28) != -1 && func_L20_00336998(hero + 0x80, *(s32 *)(state + 0x28)) == 0) {
        return;
    }
    if (*(s32 *)(state + 0x24) != -1) {
        *(u8 *)(arg0 + 0xBC) = *(u8 *)(state + 0x24);
    }
    c = *(s32 *)(state + 0x20);
    if (c == 0xFF) {
        u8 group;

        D_L20_0021E2F0.collected[*(s16 *)(arg0 + 0xB2)] = *(u8 *)(arg0 + 0xB0) + 2;
        if (*(u8 *)(arg0 + 0xB0) == 0xFF) {
            goto record;
        }
        group = *(u8 *)(arg0 + 0xB0);
        if (!(D_L20_001B2D60[group] & 0x80) && (D_0019B2E8[group + (g_playerProgress << 4)] & 0x80)) {
        record:
            D_L20_0021EF60.collected[*(s16 *)(arg0 + 0xB2)] = *(u8 *)(arg0 + 0xB0) + 2;
        }
        func_L20_003265E8(arg0);
        return;
    }
    if (c != -1) {
        *(u8 *)(arg0 + 0x20) = *(u8 *)(state + 0x20);
    }
    if (*(s32 *)(state + 0x2C) != -1) {
        func_L20_00331910(*(s32 *)(state + 0x2C), 1);
    }
}

/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00443E18, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef float f32;
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;

/* A word inside L11's .bss block D_L11_001B39B0 (address-named, RULING
 * #13291 §2). */
extern s32 D_L11_001CAEC0;

/* The two `section(".sdata")` declarations below are an ADDRESSING-MODEL
 * DEVICE (RULING #8620): cc1 prints a one-instruction macro load or store
 * instead of splitting %hi/%lo itself, and GNU as expands it at -G0 into the
 * lui/lw (lui/sw) pair the ROM has. They move no data and emit nothing.
 * D_L11_001B370C is a word inside the .bss block D_L11_001B2E00
 * (address-named, RULING #13291 §2). */
extern u8 *D_L11_001B370C __attribute__((section(".sdata")));

/* A core byte table at 0x1395B8 (inside the boot data split's D_139599
 * extent, not g_quickSelectMap), spelled D_1395B8 as the boot and the L11
 * split asm already spell it (task #2603, FACT #13369). */
extern u8 D_1395B8[];

extern s32 D_L11_001AA170 __attribute__((section(".sdata")));

extern u8 g_heroPos[];

/* Callees at their v2.00 places in L11 (L11's own functions and its copies
 * of boot and shared code). */
extern void func_L11_00447750(u8 *, s32);

extern void func_L11_00442A18(u8 *);

extern void func_L11_0032DBF8(u8 *, s32, s32);

extern void func_L11_002E9298(s32);

extern void func_L11_00316578(s32, void *);

extern void func_L11_003011A8(void *);

extern s32 func_L11_0043F948(s32);

extern void func_L11_002D1A58(s32, s32, s32, s32);

extern void func_L11_0036E3E0(void *, void *);

extern void func_L11_00342748(void *, void *);

/**
 * func_L11_00443E18 - restart the actor's state machine, release the level's
 * helper objects and hand the state's slots to the two level handlers.
 *
 * @obj: the actor; its state block is at +0x68.
 *
 * In order: func_L11_00447750(obj, 0x20) and func_L11_0032DBF8(obj, 1, -1);
 * the handle at state+0x170, if set, goes to func_L11_002E9298 and is
 * cleared; when D_L11_001CAEC0 is set, func_L11_00316578(0, state+0x2F4's
 * pointer). A 4-float buffer is filled by func_L11_003011A8, its [2] set to
 * 2.16 or -2.52 by D_1395B8[func_L11_0043F948(1)], and passed with the
 * 128-byte entry of D_L11_001B370C that state+0x15C (or +0x158) selects, at
 * +0x30, to func_L11_002D1A58. Then func_L11_0036E3E0(g_heroPos,
 * g_heroPos + 0x10), func_L11_00442A18(obj) and func_L11_00342748(state+0x2F4,
 * state+0x2FC pointers) run, and D_L11_001AA170 is set to -1.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_004425C8.c, function
 * func_L11_004425C8 (v1.01 address), author commit 3cd8af2d (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L11_004425C8 -> func_L11_00443E18 (our split name at the
 *     v2.00 place);
 *   - D_00189EA0 -> g_heroPos (the boot's name, same address);
 *     D_L11_001AA170 unchanged;
 *   - D_001395B8 -> D_1395B8: the same address, respelled to the boot's and
 *     the L11 split asm's existing name for it (task #2603, FACT #13369);
 *   - D_L11_001CAE40 -> D_L11_001CAEC0, D_L11_001B368C -> D_L11_001B370C,
 *     func_L11_00445D28 -> func_L11_00447750, func_L11_004411C8 ->
 *     func_L11_00442A18, func_L11_0043E108 -> func_L11_0043F948: upstream
 *     names state v1.01 addresses, ours are the targets of the v2.00 ROM's
 *     own relocations (relverify, task #2553);
 *   - callees named for other programs' addresses respelled to L11's copies,
 *     the targets of the same relocations: func_002A7750 ->
 *     func_L11_0032DBF8, func_L00_002D7140 -> func_L11_002E9298,
 *     func_L00_00303530 -> func_L11_00316578, func_00282C88 ->
 *     func_L11_003011A8, func_L00_002B9D60 -> func_L11_002D1A58,
 *     func_002DF3F8 -> func_L11_0036E3E0, func_L01_00323310 ->
 *     func_L11_00342748;
 *   - `__attribute__((sda))` -> `section(".sdata")` (RULING #8620) on two
 *     externs;
 *   - the upstream function comment folded into this one; the trailing blank
 *     line inside the body dropped.
 */
void func_L11_00443E18(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    f32 buf[4];

    func_L11_00447750(obj, 0x20);
    func_L11_0032DBF8(obj, 1, -1);
    if (*(s32 *)(state + 0x170) != 0) {
        func_L11_002E9298(*(s32 *)(state + 0x170));
        *(s32 *)(state + 0x170) = 0;
    }
    if (D_L11_001CAEC0 != 0) {
        func_L11_00316578(0, *(void **)(state + 0x2F4));
    }
    func_L11_003011A8(buf);
    if (D_1395B8[func_L11_0043F948(1)] != 0) {
        buf[2] = 2.16f;
        func_L11_002D1A58((s32)(D_L11_001B370C + (*(s32 *)(state + 0x15C) << 7)) + 0x30, (s32)buf, 0, 1);
    } else {
        buf[2] = -2.52f;
        func_L11_002D1A58((s32)(D_L11_001B370C + (*(s32 *)(state + 0x158) << 7)) + 0x30, (s32)buf, 0, 1);
    }
    func_L11_0036E3E0(g_heroPos, g_heroPos + 0x10);
    func_L11_00442A18(obj);
    func_L11_00342748(*(void **)(state + 0x2F4), *(void **)(state + 0x2FC));
    D_L11_001AA170 = -1;
}

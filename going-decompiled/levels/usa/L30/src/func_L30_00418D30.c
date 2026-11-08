/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00418D30, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* Upstream's declarations, respelled to the v2.00 targets: L30's functions
 * at their places, the boot's g_heroPos and its operand spelling D_138320
 * (a 64-bit pad word: EE `long`, upstream's own spelling, kept), seventeen
 * name strings at their L30 split labels, and the glow table, a word inside a
 * larger L30 label (g_mobyGlowWorkBuf+0x32E0 in L30's boot-name map), so
 * address-named (RULING #13291 section 2). */
extern char *func_L30_00413B08(void *);
extern void func_L30_00413BA0(void *, void *, void *, int, int, int);
extern char g_heroPos[];
extern long D_138320;
extern int func_L30_003221C8(int, int);
extern void func_L30_00322088(void *, int, int);
extern int func_L30_00327920(void *, int);
extern int func_L30_0032C268(int, int, int);
extern int func_L30_0032D1D8(void *);
extern unsigned char *func_L30_0033FA68(void *, void *, int, int, int, int, int, float, float, float, int);
extern unsigned char *D_L30_001FF7C0[];
extern char D_L30_001B1FC0[];
extern char D_L30_001B1FD0[];
extern char D_L30_001B1FE0[];
extern char D_L30_001B1FF0[];
extern char D_L30_001B2000[];
extern char D_L30_001B2008[];
extern char D_L30_001B2010[];
extern char D_L30_001B2018[];
extern char D_L30_001B2028[];
extern char D_L30_001B2038[];
extern char D_L30_001B2048[];
extern char D_L30_001B2058[];
extern char D_L30_001B2068[];
extern char D_L30_001B2070[];
extern char D_L30_001B2080[];
extern char D_L30_001B2090[];
extern char D_L30_001B20A0[];
typedef struct { char pad[0x4c]; short minimum, maximum; } MuseumParticleBounds;

/**
 * func_L30_00418D30 - the museum's tunable particle-fountain actor.
 * @obj: the actor; its state block is at +0x68 and holds the fountain's
 *       settings (MuseumParticleBounds: spawn-delay bounds at +0x4C/+0x4E).
 *
 * State 0 (byte +0x20) clamps the delay bounds, draws a first delay
 * (func_L30_003221C8) into +0xC and moves to state 1. State 1: when the
 * hero is in the actor's volume (+0x48, func_L30_00327920) it shows message
 * 0x1338 (func_L30_0032C268(1, 0x1338, 1)) and, when bits 0 and 1 of the
 * pad word D_138320 are both set, moves to state 2. State 2 opens the value-editing menu once
 * (func_L30_0032D1D8 / func_L30_00413B08, kept at +0x3C) with seventeen
 * entries (func_L30_00413BA0: setting, name string, type, min, max), clears
 * the message and goes back to state 1 when the menu moby is gone or dying.
 * In every state the delay counts down and, at 0, a particle is spawned
 * (func_L30_0033FA68) with the settings' colour, sizes (scaled by 210000)
 * and texture D_L30_001FF7C0[set][index], its bytes 9 and 2 set.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/us2/levels/30_insomniac_museum/L30_00418D30-cc.c, function
 * func_US2_L30_00418D30 (v2.00 address), author commit cb63d202 (bigbeno37;
 * `git blame -M -C` gives all 65 definition lines to it, `git log -S`
 * agrees). Its body is a copy of the same author's v1.01 function
 * func_L30_004186B0 (src/levels/30_insomniac_museum/L30_0040E660.c, body by
 * 96f56db3, bigbeno37, from bigbeno37's draft feaab5f5), identical but for
 * the name and one constant (0x27E6 there, 0x1338 here); RULINGs #13427
 * rev 4 and #13541 rev 2 credit that writer too, the same person. Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names, each name to the target of the v2.00 disc's own
 * relocation at this place (task #2673 relverify by name):
 *   - func_US2_L30_00418D30 -> func_L30_00418D30 (our split name, same
 *     address);
 *   - D_00138320 -> D_138320 (the boot's own operand spelling of that core
 *     address); D_00189EA0 -> g_heroPos (the boot's symbol_addrs name);
 *   - D_L30_001B2110..D_L30_001B21F0 -> D_L30_001B1FC0..D_L30_001B20A0
 *     (each 0x150 lower: our split labels at the v2.00 targets);
 *     D_L30_001FF940 -> D_L30_001FF7C0 (the v2.00 target, address-named);
 *   - func_002A7750 -> func_L30_00322088, func_002A7820 -> func_L30_003221C8,
 *     func_002ACBC8 -> func_L30_00327920, func_002B0CE0 -> func_L30_0032C268,
 *     func_002B3868 -> func_L30_0032D1D8, func_002C65A0 -> func_L30_0033FA68,
 *     func_L30_00413490 -> func_L30_00413B08, func_L30_00413528 ->
 *     func_L30_00413BA0 (L30's functions at their v2.00 places);
 *   - the file's comments, and its u16 typedef (unused here), dropped.
 */
void func_L30_00418D30(char *obj) {
    char *st = *(char **)(obj + 0x68);
    MuseumParticleBounds *bounds = (MuseumParticleBounds *)st;
    unsigned char *particle;

    switch ((unsigned char)obj[0x20]) {
    case 0:
        if (bounds->maximum < 0) bounds->maximum = 0;
        if (bounds->minimum > bounds->maximum) bounds->minimum = bounds->maximum;
        else if (bounds->minimum < 0) bounds->minimum = 0;
        *(float *)(st + 0xC) = func_L30_003221C8(bounds->minimum, bounds->maximum);
        func_L30_00322088(obj, 1, -1);
        break;
    case 1:
        if (*(int *)(st + 0x48) != -1 && func_L30_00327920(g_heroPos, *(int *)(st + 0x48))) {
            func_L30_0032C268(1, 0x1338, 1);
            if ((D_138320 & 3) == 3) func_L30_00322088(obj, 2, -1);
        }
        break;
    case 2:
        if (func_L30_0032D1D8(obj)) {
            *(char **)(st + 0x3C) = func_L30_00413B08(obj);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x00, D_L30_001B1FC0, 1, -1000, 1000);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x04, D_L30_001B1FD0, 1, -1000, 1000);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x08, D_L30_001B1FE0, 1, -1000, 1000);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x10, D_L30_001B1FF0, 0, 1, 1000);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x14, D_L30_001B2000, 0, 0, 255);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x18, D_L30_001B2008, 0, 0, 255);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x1C, D_L30_001B2010, 0, 0, 255);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x20, D_L30_001B2018, 0, -1000, 1000);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x24, D_L30_001B2028, 0, 0, 127);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x28, D_L30_001B2038, 0, 0, 127);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x2C, D_L30_001B2048, 1, 0, 1000);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x30, D_L30_001B2058, 1, 0, 1000);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x34, D_L30_001B2068, 1, -1000, 1000);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x40, D_L30_001B2070, 0, 0, 103);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x44, D_L30_001B2080, 0, 0, 20);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x4C, D_L30_001B2090, 3, 0, 1000);
            func_L30_00413BA0(*(char **)(st + 0x3C), st + 0x4E, D_L30_001B20A0, 3, 0, 1000);
        }
        func_L30_0032C268(1, 0, 1);
        if (*(char **)(st + 0x3C) == 0
            || *(short *)(*(char **)(st + 0x3C) + 0xAA) != 0x118A
            || (unsigned char)(*(char **)(st + 0x3C))[0x20] == 0xFE
            || (unsigned char)(*(char **)(st + 0x3C))[0x20] == 0xFD) {
            func_L30_00322088(obj, 1, -1);
        }
        break;
    }

    if ((*(float *)(st + 0xC))-- == 0.0f) {
        if (bounds->minimum > bounds->maximum) bounds->minimum = bounds->maximum;
        else if (bounds->minimum < 0) bounds->minimum = 0;
        *(float *)(st + 0xC) = func_L30_003221C8(bounds->minimum, bounds->maximum);
        particle = func_L30_0033FA68(obj + 0x10, st, *(int *)(st + 0x10),
                                 (*(int *)(st + 0x1C) << 16) | (*(int *)(st + 0x18) << 8) | *(int *)(st + 0x14),
                                 *(int *)(st + 0x20), *(int *)(st + 0x24), *(int *)(st + 0x28),
                                 *(float *)(st + 0x2C) * 210000.0f, *(float *)(st + 0x30) * 210000.0f,
                                 *(float *)(st + 0x34), *(int *)(st + 0x38));
        if (particle) {
            particle[9] = 0xFF;
            particle[2] = D_L30_001FF7C0[*(int *)(st + 0x40)][*(int *)(st + 0x44)];
        }
    }
}

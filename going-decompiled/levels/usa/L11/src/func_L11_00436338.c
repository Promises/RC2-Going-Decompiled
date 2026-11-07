/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00436338, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 115 of
 * 128 .text words differ from the disc, relocated fields masked, and .text is
 * 0x200 bytes, not the place's 0x208, and it emits a .lit4 pool of 4 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

/* RULING #8620 addressing device, EE arm only (level code has no native arm): a
 * section attribute on an extern DECLARATION moves no data and emits nothing.
 * Ablation (task #2578): with upstream's `sda`/`nosda` text instead, same
 * length, level .text 48 differing bytes, first at 0x0043643C. */
typedef unsigned int u32;

extern void func_L11_00301210(void *dst, void *a, void *b);
extern void func_L11_003014F8(void *dst, void *src, float scale);
typedef int TI128 __attribute__((mode(TI)));
typedef struct {
    TI128 position[4];
    u32 color[4];
    float uv[4][2];
    unsigned long primitive, texture, mode, registers;
} EffectQuad;
typedef struct {
    TI128 saved;
    EffectQuad quad;
    float a[4];
    float b[4];
    float c[4];
    TI128 corner;
} EffectQuadFrame;
extern void func_L11_003011E0(void *, void *, void *);
extern void func_L11_00301328(void *, void *, void *);
extern unsigned long func_L11_002F8F30(int);
extern void func_L11_002FD670(void *, void *, int);
extern char g_cameraPos[];
extern int D_L11_001B11DC __attribute__((section(".sdata")));
extern int D_L11_001B11E4 __attribute__((section(".sdata")));
extern float D_L11_001B11F0[] __attribute__((section(".sdata")));
extern float D_L11_001B11F4[] __attribute__((section(".sdata")));

/**
 * func_L11_00436338 - draw the first stage of the beam between the state's two
 * end points as one textured quad.
 *
 * @obj: the beam object (moby); +0x68 its state block, whose +0x00 and +0x10 are
 *       the two end points.
 * @q:   a 16-byte value, copied into the frame and otherwise unused.
 * Return: none.
 *
 * Non-obvious: Each end's offset toward the level's anchor point
 * g_cameraPos is crossed with the span (func_L11_00301328) and scaled by
 * 0.1. The four corners are each end plus and minus its offset. The quad takes
 * texture 7 (func_L11_002F8F30), the colour D_L11_001B11E4 at every corner,
 * and its UVs from the level's table at D_L11_001B11F0. It is submitted with
 * func_L11_002FD670. `unsigned long` in the quad's GS words is 64 bits on the
 * EE; level code has no native arm.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00432628-g8.c (blob debf25a2), function
 * func_L11_00434AF8 (v1.01 address), author commit 9ad4d690 (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED: the section respelling below; otherwise verbatim but for names.
 * Respellings, each to the target of the v2.00 disc's own relocation at this
 * place (relverify by name on this object, task #2578):
 *   - func_L11_00434AF8 -> func_L11_00436338 (our split name at the v2.00
 *     place);
 *   - D_L11_001B11FC -> D_L11_001B11DC (our label at that address);
 *   - D_L11_001B1204 -> D_L11_001B11E4 (our label at that address);
 *   - D_L11_001B1210 -> D_L11_001B11F0 (our label at that address);
 *   - D_L11_001B1214 -> D_L11_001B11F4 (our label at that address);
 *   - D_L11_001BA8C0 -> g_cameraPos (L11's boot-map name at
 *     the v2.00 target 0x001BA940);
 *   - func_0027C878 -> func_L11_002F8F30 (the level's own copy at its L11
 *     place);
 *   - func_00280B90 -> func_L11_002FD670 (the level's own copy at its L11
 *     place);
 *   - func_00282CC0 -> func_L11_003011E0 (the level's own copy at its L11
 *     place);
 *   - func_00282CF0 -> func_L11_00301210 (the level's own copy at its L11
 *     place);
 *   - func_00282DD8 -> func_L11_00301328 (the level's own copy at its L11
 *     place);
 *   - func_00282F20 -> func_L11_003014F8 (the level's own copy at its L11
 *     place);
 *   - D_L11_001BA940 -> g_cameraPos (task #2646): the name
 *     symbol_addrs/L11_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on 4
 *     extern declaration(s) (RULING #8620: upstream's patched cc1 reads sda,
 *     ours does not);
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_00436338(char *obj, void *q)
{
    EffectQuadFrame f;
    char *st;
    int i;

    f.saved = *(TI128 *)q;
    st = *(char **)(obj + 0x68);
    func_L11_00301210(f.c, st, st + 0x10);
    func_L11_00301210(f.a, st, g_cameraPos);
    func_L11_00301210(f.b, st + 0x10, g_cameraPos);
    func_L11_00301328(f.a, f.a, f.c);
    func_L11_003014F8(f.a, f.a, 0.1f);
    func_L11_00301328(f.b, f.b, f.c);
    func_L11_003014F8(f.b, f.b, 0.1f);
    f.quad.texture = func_L11_002F8F30(7);
    f.quad.mode = 0x0000ff9000000260UL;
    f.quad.registers = ((unsigned long)D_L11_001B11DC << 32) | 0x48;
    f.quad.primitive = 0;
    f.quad.color[0] = f.quad.color[1] = f.quad.color[2] = f.quad.color[3] = D_L11_001B11E4;
    for (i = 0; i < 4; i++) {
        f.quad.uv[i][0] = D_L11_001B11F0[i * 2];
        f.quad.uv[i][1] = D_L11_001B11F4[i * 2];
    }
    func_L11_00301210(&f.corner, st, f.a);
    f.quad.position[0] = f.corner;
    func_L11_003011E0(&f.corner, st, f.a);
    f.quad.position[1] = f.corner;
    func_L11_00301210(&f.corner, st + 0x10, f.b);
    f.quad.position[2] = f.corner;
    func_L11_003011E0(&f.corner, st + 0x10, f.b);
    f.quad.position[3] = f.corner;
    func_L11_002FD670(&f.quad, 0, 0);
}

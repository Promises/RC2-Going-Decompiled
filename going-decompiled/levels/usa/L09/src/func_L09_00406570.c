/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00406570, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef int Copy128 __attribute__((mode(TI)));
typedef union { Copy128 q; float f[4]; } Vector16;

typedef int s32;
typedef unsigned char u8;
typedef float f32;

/* The hero's position record: the boot's g_heroPos at 0x00189EA0 (upstream's
 * address-named extern spelled D_00189EA0). */
extern char g_heroPos[];

extern void func_L09_002EB0F8(void *, void *, void *);
extern void func_L09_002EB128(void *, void *, void *);
extern void func_L09_002EB3D8(void *, void *, float);
extern void func_L09_002EB5B8(void *, void *, void *);
extern void func_L09_00316FE8(char *, int, int);
extern int func_L09_0035D9A8(int, int, char *);
extern void func_L09_0031D518(char *, void *, int, float, float);
extern void func_L09_003238A8(char *, int, float *, unsigned char, float, float, float, float);
extern void func_L09_0030D900(void *);

/**
 * func_L09_00406570 - burst the object: retune it, take it out of play, and
 * throw its break effect.
 *
 * The object goes to animation state 3 via func_L09_00316FE8(@object, 0, 3),
 * is detached by func_L09_0035D9A8(1, 0, @object), and its break impulse is
 * built: the offset vector (zeroed, then 0.75f at f[2]) is rotated into world
 * space at the object's +0xC0 matrix by func_L09_002EB5B8 and centred on the
 * object's origin (+0x10) by func_L09_002EB0F8; the object is pushed with
 * func_L09_0031D518(@object, &position, -1, 0.333f, 0). The impulse vector
 * then starts from the origin-to-g_heroPos direction (func_L09_002EB128),
 * has f[2] zeroed, and is scaled by 0.10000001f through func_L09_002EB3D8.
 * Finally func_L09_003238A8 throws the visible burst (bone 0x1F, alpha 255,
 * the 3.0f/6.0f pairs) and func_L09_0030D900 closes the sequence.
 *
 * @object: the object (its origin view is at +0x10, state at +0x68).
 * Return: none.
 *
 * Non-obvious: the local `zero` is set to 0.0f through an intermediate so the
 * same constant register feeds three calls. No codegen or addressing device
 * is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_004050E0.c, function func_L09_004050E0 (v1.01
 * address), author commit 3dc9b1fa (Louis-Philippe Le Sieur). Adopted with
 * the authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM
 * but for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's
 * own relocation at this place (task #2562 relverify by name):
 *   - func_L09_004050E0 -> func_L09_00406570 (our split name);
 *   - D_00189EA0 -> g_heroPos (the boot's name at 0x00189EA0);
 *   - func_002A6C58 -> func_L09_00316FE8, func_002E5CF8 -> func_L09_0035D9A8,
 *     func_00283098 -> func_L09_002EB5B8, func_00282CC0 -> func_L09_002EB0F8,
 *     func_002AC728 -> func_L09_0031D518, func_00282CF0 -> func_L09_002EB128,
 *     func_00282F20 -> func_L09_002EB3D8, func_002B0A68 -> func_L09_003238A8,
 *     func_0029F338 -> func_L09_0030D900 (L09's own copies at the v2.00
 *     places);
 *   - `#include "include_asm.h"` dropped (nothing in the function used it);
 *     Copy128 and Vector16 are upstream's own file-scope typedefs, kept
 *     verbatim.
 */
void func_L09_00406570(char *object)
{
    Vector16 position, offset, impulse;
    char *origin = object + 0x10;
    float zero;
    func_L09_00316FE8(object, 0, 3);
    zero = 0.0f;
    func_L09_0035D9A8(1, 0, object);
    offset.q = 0;
    offset.f[2] = 0.75f;
    func_L09_002EB5B8(&position, &offset, object + 0xC0);
    func_L09_002EB0F8(&position, &position, origin);
    func_L09_0031D518(object, &position, -1, 0.333f, zero);
    func_L09_002EB128(&impulse, origin, g_heroPos);
    impulse.f[2] = zero;
    func_L09_002EB3D8(&impulse, &impulse, 0.10000001f);
    func_L09_003238A8(object, 0x1F, impulse.f, 255, 3.0f, 6.0f, 3.0f, 6.0f);
    func_L09_0030D900(object);
}

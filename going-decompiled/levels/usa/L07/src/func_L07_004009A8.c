/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x004009A8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;

/* A core flag byte (below 0x1A7C80) with no label of its own: address-named,
 * 2 bytes into the boot's core.lit dlabel D_1A7BD6. ADDRESSING-MODEL DEVICE
 * (RULING #8620, EE arm only): `.sdata` on the extern declaration makes cc1
 * print the one-instruction `lbu` macro, which GNU as expands absolute
 * (`lui` / `lbu %lo`) at -G0, as retail has it. It moves no data and emits
 * nothing. */
extern u8 D_001A7BD8 __attribute__((section(".sdata")));

extern void func_L07_004009E0(u8 *a0);

/**
 * func_L07_004009A8 - set the object's 0x2 flag while a core flag is up,
 * otherwise run the object's update.
 *
 * When the core flag byte D_001A7BD8 is nonzero, bit 0x2 is ORed into the
 * object's u16 flags at +0x34 and nothing else runs. Otherwise the object is
 * handed to func_L07_004009E0, the function that follows this one.
 *
 * @a0: the object.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_003F4578.c, function func_L07_003FF618
 * (v1.01 address), author commit adcbd51e (llesieur99), by blame -M -C of the
 * definition (all 8 lines; no draft found by `log -S`, no rename-copy found).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged; edits, each name to the
 * target of the v2.00 disc's own relocation at this place (task #2656
 * relverify by name):
 *   - func_L07_003FF618 -> func_L07_004009A8 (our split name);
 *   - D_001A7BD0 -> D_001A7BD8: the address the disc's relocations imply is
 *     0x1A7BD8, 8 bytes past upstream's name; address-named, since no boot
 *     dlabel, boot operand, symbol_addrs entry, L07_boot_names entry or L07
 *     split label names that address (task #2656 grep);
 *   - func_L01_003C91C8 -> func_L07_004009E0 (L07's own copy at the v2.00
 *     place);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_001A7BD8 declaration (RULING #8620; upstream's `sda` is a marker for
 *     its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped, common.h's two typedefs this body uses written
 *     out in the TU instead.
 */
void func_L07_004009A8(u8 *a0)
{
    if (D_001A7BD8 != 0) {
        *(u16 *)(a0 + 0x34) |= 0x2;
        return;
    }
    func_L07_004009E0(a0);
}

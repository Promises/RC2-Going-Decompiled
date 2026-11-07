/* Level 08 (Tabora), USA v2.00: a level-only function built from C (task
 * #2627, RULING #13291 row L08-G1). Compiled ALONE as its own TU and linked at
 * its one place, 0x003D9DE8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L08/config/L08_c.tsv, the arm sdk29
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G8.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef int s32;

typedef struct {
    u8 pad00[0x20];
    u8 f20;
    u8 f21;
    u8 pad22[0x68 - 0x22];
    void *f68;
    u8 pad6C[0x95 - 0x6C];
    u8 f95;
    u16 f96;
    u8 pad98[0xBC - 0x98];
    u8 fBC;
    u8 padBD[0x100 - 0xBD];
} Ent;

/* the level's object-handle table: a pointer global holding the base of
 * 0x100-byte entries. L08's boot-name map name at 0x001B29DC
 * (symbol_addrs/L08_boot_names.txt). Upstream's `__attribute__((sda))` is kept verbatim; our
 * stock cc1 warns `attribute directive ignored`, byte-inert here (masked
 * compare diff 0). */
extern Ent *g_mobyTableBase __attribute__((sda));

/**
 * func_L08_003D9DE8 - are a handle's entry faces unset or gone? (second of
 * three identical ROM copies.)
 *
 * @a0: the entity handle.
 * Return: 1 when the handle is -1, or when the entry is unset/dead (its
 * class byte at +0x20 is 0, 0xFE or 0xFD); else 0.
 *
 * Upstream carries this body three times (func_L08_003D8948, func_L08_003D8990
 * and func_L08_004229F8); the ROM holds all three copies, so all three are
 * adopted, each at its own place.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/08_tabora/L08_003D8348-g8.c, function
 * func_L08_003D8990 (v1.01 address), author commit 91e04732 (llesieur99, the
 * same account as Louis-Philippe Le Sieur). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL:
 * VERBATIM but for names; edits:
 *   - name func_L08_003D8990 -> func_L08_003D9DE8 (our split name at the
 *     v2.00 place);
 *   - D_L08_001B29DC -> g_mobyTableBase (task #2646): upstream's spelling,
 *     which states the v2.00 target address 0x001B29DC, respelled to the
 *     name symbol_addrs/L08_boot_names.txt gives at exactly that address;
 *     the address-named spelling was a second name for the same address
 *     (A2 class, NOTE #13409);
 *   - the file's `#include "include_asm.h"` line, the other functions and
 *     their externs not carried (nothing else here uses them); the u8/u16/
 *     s16/s32 typedefs and the Ent struct kept.
 */
/* the entry's two faces are either unset or already gone */
s32 func_L08_003D9DE8(s32 a0)
{
    Ent *p;
    s32 r;

    if (a0 != -1) {
        p = g_mobyTableBase + a0;
        r = 0;
        if (p == 0 || p->f20 == 0xFE || p->f20 == 0xFD) r = 1;
        return r;
    }
    return 1;
}

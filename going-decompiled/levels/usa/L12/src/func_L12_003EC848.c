/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003EC848, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef signed short s16;
typedef unsigned int u32;

/* Upstream's own macro (its common.h), kept as written: a section attribute
   on an extern declaration places nothing; it keeps g_heroState's load the
   absolute %hi/%lo pair the ROM has instead of a $gp access at -G8. */
#define NOT_SDA __attribute__((section(".data")))

/* The subtitle history (src/game/00289178.c): per record, its play count, the time it was last
   played and the levels it was played in (bit 31: played at all). */
typedef struct {
    unsigned short count, pad;
    unsigned int time, levels;
} SubtitleHistory;
/* The level's copies of boot code, named by their v2.00 places. */
extern void func_L12_002F8958(int,int);

/* ADDRESSING-MODEL DEVICE (RULING #8620) on the declarations of
   g_inventoryOwned, g_playerProgress and g_mobyTableBase: upstream's `sda`
   (ignored by this cc1) is respelled `section(".sdata")`, so cc1 prints the
   one-instruction load macro that GNU as expands to the absolute %hi/%lo
   pair the ROM has for g_inventoryOwned and g_playerProgress (validator
   #2675: 67 and 18 words differ without each). On g_mobyTableBase it is
   inert at -G8 (GPREL16 either way; 0 words), kept as upstream's own text.
   It moves no data and emits nothing.
   Ablation (task #2670, upstream's own `sda`, same arm and flags): 153 words
   against the ROM's 150, 75 of them differing. */
extern u8 g_heroPos[];
extern SubtitleHistory D_18C958[];
extern u8 g_inventoryOwned[] __attribute__((section(".sdata")));
extern u32 D_1A7390;
extern int g_playerProgress[] __attribute__((section(".sdata")));
extern int g_heroState NOT_SDA;
extern int func_L12_00325270(void *,int);
extern void func_L12_002F2190(void *);
extern u8 *g_mobyTableBase __attribute__((section(".sdata")));
extern void func_L12_0031F2D8(void *,int,int);

/**
 * func_L12_003EC848 - play the level's tutorial subtitle lines, each gated on
 * its record in the subtitle history.
 *
 * State 0 (the state byte at +0x20): sets +0x30 to 255, the countdown at
 * vars+0x10 to -1 and moves the object to state 1 (func_L12_0031F2D8).
 * State 1, each update:
 *   - when the hero is in the volume named by vars+0 (func_L12_00325270),
 *     the hero's +0x540 word is set and record 121 has never played, plays
 *     line 0xA22 (func_L12_002F8958, the record index its second argument);
 *   - when the hero is in the volume at vars+4 and inventory slot 0x33 is
 *     empty: if record 92 has played, plays 0xA05 once more than 0x438 ticks
 *     (or never) after its last play time, else only refreshes that time;
 *     if it has not, counts it played, stamps the time and marks this level
 *     (bit g_playerProgress[0], plus bit 31) in its level mask;
 *   - when the hero is in the volume at vars+8 and the moby that vars+0xC
 *     indexes is a live class-0xFB8 one, arms the countdown to 0x2A30 on
 *     first sight; when the countdown has run out (0) and record 93 has not
 *     played in this level, plays 0xA06;
 *   - when inventory slot 7 is owned and the hero's state is not 0x6F, plays
 *     0xA28 unless record 127 has played in this level;
 *   - steps the countdown (func_L12_002F2190) while it is not negative.
 *
 * @o: the object; its vars block is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_00398E50-g8.c, function
 * func_L12_003EB470 (v1.01 address), author commit b9d9d0d4 (bigbeno37). The
 * earlier nonmatching drafts eb4ef01f (DrLegitamate) and 526dab78
 * (Louis-Philippe Le Sieur) leave no line in this body (0 of 37 non-trivial
 * lines verbatim in either; `git blame -M -C -C -C` at the pin gives all 55
 * lines to b9d9d0d4), so they carry no credit (RULING #13427 rev 2). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED; the body's statements unchanged, the edits, each name to the target
 * of the v2.00 disc's own relocation at this place (task #2670 relverify):
 *   - func_L12_003EB470 -> func_L12_003EC848 (our split name at the v2.00 place);
 *   - func_00282948 -> func_L12_002F2190, func_00288E18 -> func_L12_002F8958,
 *     func_002A7750 -> func_L12_0031F2D8, func_002ACBC8 -> func_L12_00325270
 *     (our split names at the v2.00 targets);
 *   - D_00189EA0 -> g_heroPos, D_0018C0B4 -> g_heroState, D_001A79F0 ->
 *     g_playerProgress (0x001A79F8), D_001A7AF8 -> g_inventoryOwned
 *     (0x001A7B00): the boot's names (symbol_addrs/usa);
 *   - D_0018C958 -> D_18C958 and D_001A7390 -> D_1A7390: the boot asm's own
 *     operand spellings of those core addresses (no boot label starts there);
 *   - D_L12_001B2E9C -> g_mobyTableBase, the name the level's boot-name map
 *     gives at the v2.00 target 0x001B2E1C (inside a larger split data label,
 *     so no address-named extern is needed);
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on the declarations of g_inventoryOwned, g_playerProgress and
 *     g_mobyTableBase (RULING #8620; the audit's `resp` variant, NOTE #13281);
 *   - `#include "common.h"` dropped: the typedefs and the NOT_SDA macro used
 *     are written here and only this function's declarations kept; the
 *     duplicate copy of upstream's comment above the function folded into
 *     this one.
 */
void func_L12_003EC848(u8 *o) {
    u8 *d = *(u8 **)(o + 0x68);

    switch (o[0x20]) {
    case 0:
        o[0x30] = 255;
        *(int *)(d + 0x10) = -1;
        func_L12_0031F2D8(o, 1, -1);
        break;
    case 1:
        if (func_L12_00325270(g_heroPos, *(int *)d) && *(int *)(g_heroPos + 0x540)
            && !D_18C958[121].count)
            func_L12_002F8958(0xA22, 121);
        if (func_L12_00325270(g_heroPos, *(int *)(d + 4)) && !g_inventoryOwned[0x33]) {
            if (D_18C958[92].count) {
                u32 now = D_1A7390;
                u32 old = D_18C958[92].time;

                if (now - old > 0x438 || old == 0)
                    func_L12_002F8958(0xA05, 92);
                else if (old < now)
                    D_18C958[92].time = now;
            } else {
                if (D_18C958[92].count < 65535)
                    D_18C958[92].count++;
                if (D_18C958[92].time < D_1A7390)
                    D_18C958[92].time = D_1A7390;
                D_18C958[92].levels = (D_18C958[92].levels | (1u << g_playerProgress[0])) | 0x80000000u;
            }
        }
        if (func_L12_00325270(g_heroPos, *(int *)(d + 8))) {
            u8 *p = g_mobyTableBase + (*(int *)(d + 0xC) << 8);

            if (p && *(s16 *)(p + 0xAA) == 0xFB8 && p[0x20] != 254 && p[0x20] != 253) {
                if (*(int *)(d + 0x10) < 0)
                    *(int *)(d + 0x10) = 0x2A30;
                else if (!*(int *)(d + 0x10)) {
                    u32 seen = D_18C958[93].levels & (1 << g_playerProgress[0]);

                    if (!seen)
                        func_L12_002F8958(0xA06, 93);
                }
            }
        }
        if (g_inventoryOwned[7] && g_heroState != 0x6F) {
            u32 seen = D_18C958[127].levels & (1 << g_playerProgress[0]);

            if (!seen)
                func_L12_002F8958(0xA28, 127);
        }
        if (*(int *)(d + 0x10) >= 0)
            func_L12_002F2190(d + 0x10);
        break;
    }
}

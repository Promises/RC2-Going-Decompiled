/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F01C8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef struct {
    float x, y, z, w;
} V4;

typedef struct {
    int count;
    int pad[3];
    V4 pts[1];
} Entry;

typedef struct {
    int id;
    char pad04[0x2C];
    int idx;
    float t;
} State;

/* The level's path table, a word inside L02's larger .bss label D_L02_001F0000,
 * so address-named (RULING #13291 §2). */
extern char *D_L02_001FF1C0[];
/* The hero's position (boot core, below 0x1A7C80). */
extern char g_heroPos[];
/* L02's copy of a boot distance helper. */
extern float func_L02_002F2F50(char *, char *);

/**
 * func_L02_003F01C8 - step the actor's place on its path towards the hero.
 *
 * @arg0: the actor; its state block (State) is at +0x68.
 *
 * The state's id selects an Entry of D_L02_001FF1C0 (a count and 16-byte
 * points). It measures the hero's distance to the current point, then to the
 * next one: if that is nearer, the point's w is added to t (capped at 14) and
 * idx moves on. Otherwise it tries the previous point the same way, taking w
 * off t (floored at 0) and moving idx back.
 * Return: the distance to the point the actor ends on.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003DB310.c,
 * function func_L02_003EEC58 (v1.01 address), author commit eef580e5 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003EEC58 -> func_L02_003F01C8 (our split name at
 *     the v2.00 place);
 *   - D_00189EA0 -> g_heroPos (our name at 0x00189EA0);
 *   - func_00282E80 -> func_L02_002F2F50 (L02's copy);
 *   - D_L02_001FF1C0 unchanged: the v2.00 relocation targets the same
 *     address, kept address-named (RULING #13291 §2);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
float func_L02_003F01C8(char *arg0) {
    State *state = *(State **)(arg0 + 0x68);
    Entry *entry = (Entry *)D_L02_001FF1C0[state->id];
    float cur = func_L02_002F2F50(g_heroPos, (char *)&entry->pts[state->idx]);
    float d;

    if (state->idx < entry->count - 1) {
        d = func_L02_002F2F50(g_heroPos, (char *)&entry->pts[state->idx + 1]);
        if (d < cur) {
            state->t += entry->pts[state->idx].w;
            if (state->t > 14.0f)
                state->t = 14.0f;
            state->idx += 1;
            return d;
        }
    }
    if (state->idx > 0) {
        d = func_L02_002F2F50(g_heroPos, (char *)&entry->pts[state->idx - 1]);
        if (d < cur) {
            state->t -= entry->pts[state->idx].w;
            if (state->t < 0.0f)
                state->t = 0.0f;
            state->idx -= 1;
            return d;
        }
    }
    return cur;
}

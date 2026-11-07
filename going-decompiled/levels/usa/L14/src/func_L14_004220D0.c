/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x004220D0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The entity base pointer, a word inside L14's larger label D_L14_001B1D00,
 * so address-named (RULING #13291 §2). Upstream marks it `sda`; here it is
 * the ADDRESSING-MODEL DEVICE of RULING #8620: a section(".sdata") attribute
 * on the extern DECLARATION (no data moved, nothing emitted) that only changes
 * how cc1 addresses it. Ablation (upstream file, s136, `long long`->`long`):
 * without it 6 of 85 words differ from the ROM, with it 0. */
extern unsigned char *D_L14_001B251C __attribute__((section(".sdata")));
/* L14's copies of a level-03 and a level-02 helper, at their v2.00 places. */
extern void func_L14_00421EF0(void *);
extern int func_L14_00333388(void *, void *, int, int *);

typedef struct {
    int id;
    unsigned char links[4];
    unsigned char pad08[24];
} PrisonGraphNode;

typedef struct {
    unsigned char pad00[0x80];
    PrisonGraphNode nodes[32];
    int count;
} PrisonGraphState;

/**
 * func_L14_004220D0 - mark or clear the link between two nodes of an entity's
 * graph.
 *
 * @index:  the entity, a 256-byte record of D_L14_001B251C's table.
 * @first:  the first node's key.
 * @second: the second node's key.
 * @clear:  non-zero clears the link's bit 7, zero sets it.
 *
 * An entity whose byte +0x20 is 0 is set up first (func_L14_00421EF0). Both
 * keys are looked up in the entity's state (PrisonGraphState at +0x68) with
 * func_L14_00333388; when both are found and differ, the link byte naming the
 * other node in each node's four links gets bit 7 set (or cleared). Both
 * scans index from node 0's links and read the two ids from their stack
 * slots, as the ROM does.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c,
 * function func_L14_00420B10 (v1.01 address), author commit 4912fc03 (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; edits:
 *   - name func_L14_00420B10 -> func_L14_004220D0 (our split name at
 *     the v2.00 place);
 *   - D_L14_001B259C -> D_L14_001B251C (the v2.00 relocation target;
 *     inside D_L14_001B1D00, so address-named, RULING #13291 §2);
 *   - its `__attribute__((sda))` written
 *     `__attribute__((section(".sdata")))` (RULING #8620
 *     addressing-model device on an extern declaration);
 *   - func_L03_004199F0 -> func_L14_00421EF0 and func_L02_0032EDD8 ->
 *     func_L14_00333388 (L14's copies);
 *   - the audit's `long long`->`long` respelling (RULING #13197) is
 *     needed only to compile the upstream FILE whole on s136; no line
 *     carried into this TU contains `long long` or `long`, so it
 *     applies to nothing here;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_004220D0(int index, void *first, void *second, int clear) {
    unsigned char *self = D_L14_001B251C + index * 256;
    PrisonGraphState *state = *(PrisonGraphState **)(self + 0x68);
    int id0, id1;
    int i;
    unsigned char *links;
    unsigned char *entry;

    if (!self[0x20])
        func_L14_00421EF0(self);
    id0 = id1 = -1;
    if (func_L14_00333388(first, state->nodes, state->count, &id0)) {
        if (func_L14_00333388(second, state->nodes, state->count, &id1)) {
            if (id0 != id1) {
                links = state->nodes[0].links;
                i = 0;
                do {
                    entry = &links[id0 * sizeof(PrisonGraphNode) + i];
                    if ((*entry & 127) == id1) {
                        if (clear)
                            *entry &= 127;
                        else
                            *entry |= 128;
                        break;
                    }
                    ++i;
                } while (i < 4);
                i = 0;
                do {
                    entry = &links[id1 * sizeof(PrisonGraphNode) + i];
                    if ((*entry & 127) == id0) {
                        if (clear)
                            *entry &= 127;
                        else
                            *entry |= 128;
                        break;
                    }
                    ++i;
                } while (i < 4);
            }
        }
    }
}

/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x004337B0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L14's copies of boot helpers, at their v2.00 places. */
extern void func_L14_00323250(void *, int, int);
extern int func_L14_00323360(int, int);
extern int func_L14_00333648(void *);
extern void func_L14_0036AAA0(int);
extern int func_L14_002F6660(void *);
extern void func_L14_002EF2A0(void (*)(void *), void *);
/* The boot's sound-anchor table (resident core): 112-byte records. */
extern char g_listenerPosHistory[];
extern void func_L14_00433A30(void *);
/* Upstream binds its local name to the callee with an asm label; the label
 * names L14's copy. */
extern int prisonStartSound(int, int, void *) __asm__("func_L14_0036ADA0");
typedef struct { char pad[0x74]; unsigned char active; char gap[0x13]; void *owner; } PrisonSoundAnchor;

/**
 * func_L14_004337B0 - the prison sound controller's state machine.
 *
 * @object: the controller; its state block (ints) is at +0x68.
 *
 * State 0 (byte +0x20) raises the five words at state[10..14] by 63, sets
 * state[15] = 10 and clears the anchor slot state[16], then enters state 1.
 * State 1 enters the state func_L14_00323360(2, 3) picks. States 2 and 3,
 * when func_L14_00333648 says so, stop the controller's own live anchor in
 * g_listenerPosHistory (func_L14_0036AAA0) and re-arm the timer state[3];
 * they (re)start sound 0 or 1 when the slot no longer holds a live anchor
 * owned by the controller (func_L14_0036ADA0), and go back to state 1 when the
 * timer runs out (func_L14_002F6660). Every frame ends by queueing
 * func_L14_00433A30 for the controller (func_L14_002EF2A0).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c,
 * function func_L14_004321D0 (v1.01 address), author commit 312b7eea (Louis-Philippe Le Sieur).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED in the audit (code 1l); as carried, VERBATIM but for names; edits:
 *   - name func_L14_004321D0 -> func_L14_004337B0 (our split name at
 *     the v2.00 place);
 *   - D_00188660 -> g_listenerPosHistory (our name at 0x00188660);
 *   - func_00282948 -> func_L14_002F6660, func_002A7750 ->
 *     func_L14_00323250, func_002A7820 -> func_L14_00323360,
 *     func_002B3868 -> func_L14_00333648, func_002E59F8 ->
 *     func_L14_0036AAA0, func_L00_002E6EF8 -> func_L14_002EF2A0,
 *     func_L14_00432450 -> func_L14_00433A30 (L14's copies / v2.00
 *     places);
 *   - prisonStartSound's asm label "func_002E5CF8" ->
 *     "func_L14_0036ADA0" (L14's copy; the local name is upstream's);
 *   - the file's second and third identical declarations of the same
 *     externs not carried;
 *   - the audit's `long long`->`long` respelling (RULING #13197) is
 *     needed only to compile the upstream FILE whole on s136; no line
 *     carried into this TU contains `long long` or `long`, so it
 *     applies to nothing here;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_004337B0(char *object)
{
    int *state = *(int **)(object + 0x68);
    switch ((unsigned char)object[0x20]) {
    case 0:
        state[10] += 63;
        state[11] += 63;
        state[12] += 63;
        state[13] += 63;
        state[14] += 63;
        state[15] = 10;
        state[16] = -1;
        func_L14_00323250(object, 1, -1);
        break;
    case 1:
        func_L14_00323250(object, func_L14_00323360(2, 3), -1);
        break;
    case 2:
        if (func_L14_00333648(object)) {
            if (state[16] != -1 && ((PrisonSoundAnchor *)(g_listenerPosHistory + state[16] * 112))->owner == object && ((PrisonSoundAnchor *)(g_listenerPosHistory + state[16] * 112))->active)
                func_L14_0036AAA0(state[16]);
            state[16] = -1;
            state[3] = func_L14_00323360(state[0], state[1]);
        }
        if (((PrisonSoundAnchor *)(g_listenerPosHistory + state[16] * 112))->owner != object || !((PrisonSoundAnchor *)(g_listenerPosHistory + state[16] * 112))->active)
            state[16] = prisonStartSound(0, 4, object);
        if (func_L14_002F6660(state + 3))
            func_L14_00323250(object, 1, -1);
        break;
    case 3:
        if (func_L14_00333648(object)) {
            if (state[16] != -1 && ((PrisonSoundAnchor *)(g_listenerPosHistory + state[16] * 112))->owner == object && ((PrisonSoundAnchor *)(g_listenerPosHistory + state[16] * 112))->active)
                func_L14_0036AAA0(state[16]);
            state[16] = -1;
            state[3] = func_L14_00323360(state[0], state[1]);
        }
        if (((PrisonSoundAnchor *)(g_listenerPosHistory + state[16] * 112))->owner != object || !((PrisonSoundAnchor *)(g_listenerPosHistory + state[16] * 112))->active)
            state[16] = prisonStartSound(1, 4, object);
        if (func_L14_002F6660(state + 3))
            func_L14_00323250(object, 1, -1);
        break;
    }
    func_L14_002EF2A0(func_L14_00433A30, object);
}

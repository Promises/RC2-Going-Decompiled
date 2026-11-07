extern void func_L02_003D7360(char *);
extern void func_L02_002F2AB0(char *, int, int);
extern void func_L02_003157A0(char *);
extern void func_L02_00316610(char *, char *);

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D7188 - despawn the actor's carried sub-objects. If the chain word
(state+0x300) is set, runs func_L02_003D7360 on it and zeroes the word; then,
walking the five slots at state+0x2A0 backwards, despawns each live one
(func_L02_003157A0) and zeroes the slot; then, if state+0x261 or state+0x221 is
set, hands state+0x260 (resp. the +0x220 block) to func_L02_00316610; finally
clears both 0x40-byte blocks with func_L02_002F2AB0.

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D5C20 (v1.01 address), author commit e80a585a (Flavius).
BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names. Respellings:
func_L02_003D5C20 -> func_L02_003D7188 (our v2.00 place);
func_L02_003D5DF8 -> func_L02_003D7360, func_00282A60 -> func_L02_002F2AB0,
func_0029F338 -> func_L02_003157A0, func_0029F9F0 -> func_L02_00316610
(relocation-derived). Extern decls written out; #include dropped (unused).

*/

void func_L02_003D7188(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *holder = *(char **)(state + 0x300);
    char **slots;
    char *s4;
    int i;

    if (holder != 0) {
        func_L02_003D7360(holder);
        *(int *)(state + 0x300) = 0;
    }
    s4 = state + 0x220;
    slots = (char **)(state + 0x2A0);
    for (i = 4; i >= 0; i--) {
        if (slots[0] != 0) {
            func_L02_003157A0(slots[0]);
            slots[0] = 0;
        }
        slots++;
    }
    if (*(unsigned char *)(state + 0x261) != 0) {
        func_L02_00316610(arg0, state + 0x260);
    }
    if (*(unsigned char *)(state + 0x221) != 0) {
        func_L02_00316610(arg0, s4);
    }
    func_L02_002F2AB0(s4, 0, 0x40);
    func_L02_002F2AB0(s4, 0, 0x40);
}

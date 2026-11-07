extern void func_L02_002EAE30(void (*)(char *), char *);
extern void func_L02_0032C1D0(char *, char *, int, int, float);
extern void func_L02_003D74B8(char *, int, int);
extern void func_L02_003D7858(char *);
extern char *D_18C2BC;

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D5C20 - per-frame maintenance of the Maktar mallet-carrier actor.
If the variant byte (obj+0x95) is 2 or the state byte (obj+0x20) is 0x12, runs
the two-limb reset func_L02_003D74B8 with (2,0) then (3,1). Always runs
func_L02_002EAE30 with func_L02_003D7858 as the callback (per-actor update
dispatch). Advances the halfword counter at obj+0x96 up to 0xFFFF. If the
active-gadget pointer D_18C2BC is set and is this actor or its +0xAA sound id
is 0x9F8, latches state+0x32A to -1; then increments the two halfword timers at
state+0x32A and +0x32E and starts the 2.5f/6/1 effect (func_L02_0032C1D0 on
state+0x340).

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D46B8 (v1.01 address), author commit e80a585a (Flavius).
BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names. Respellings:
func_L02_003D46B8 -> func_L02_003D5C20 (our v2.00 place); callees and the data
name by the v2.00 ROM's own relocations at this place (draft relverify,
task #2558): func_L02_003D5F50 -> func_L02_003D74B8,
func_L00_002E6EF8 -> func_L02_002EAE30, func_L02_003D62F0 -> func_L02_003D7858,
func_002B0990 -> func_L02_0032C1D0. D_0018C2BC -> D_18C2BC (task #2621,
census NOTE #13409): the same address, 0x18C2BC (g_activeGadgetItem+0x1D4,
inside that larger split label),
respelled to the name the boot asm already uses for it, the %hi/%lo operand
at asm/usa/text/208010.s:33524; it stays an address-named extern (RULING
#13291 §2, the name states the address). Extern decls written out; #include
dropped (unused).

*/

void func_L02_003D5C20(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x95) == 2 ||
        *(unsigned char *)(arg0 + 0x20) == 0x12) {
        func_L02_003D74B8(arg0, 2, 0);
        func_L02_003D74B8(arg0, 3, 1);
    }
    func_L02_002EAE30(func_L02_003D7858, arg0);
    if (*(unsigned short *)(arg0 + 0x96) < 0xFFFF) {
        (*(unsigned short *)(arg0 + 0x96))++;
    }
    if (D_18C2BC != 0 &&
        (D_18C2BC == arg0 || *(short *)(D_18C2BC + 0xAA) == 0x9F8)) {
        *(short *)(state + 0x32A) = -1;
    }
    *(unsigned short *)(state + 0x32A) = *(unsigned short *)(state + 0x32A) + 1;
    *(unsigned short *)(state + 0x32E) = *(unsigned short *)(state + 0x32E) + 1;
    func_L02_0032C1D0(arg0, state + 0x340, 6, 1, 2.5f);
}

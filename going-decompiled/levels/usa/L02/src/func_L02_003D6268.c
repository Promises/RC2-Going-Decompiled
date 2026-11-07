extern int func_L02_0031FA68(int);

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D6268 - the mallet-carrier's timer release. Returns 0 while the
halfword timer at state+0x32E is under 0x12C; between 0x12C and 0x1F4, returns
0 while func_L02_0031FA68 says the remaining delta (0x1F4 - state+0x32A) is
still running; otherwise zeroes both timers (+0x32A, +0x32E) and returns 1.

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D4D00 (v1.01 address), author commit 773eafc2
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respellings: func_L02_003D4D00 -> func_L02_003D6268 (our v2.00 place);
func_002A77E0 -> func_L02_0031FA68 (relocation-derived). Extern decl written
out; #include dropped (unused).

*/

int func_L02_003D6268(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    short timer = *(short *)(state + 0x32E);
    int delta;

    if (timer < 0x12C) {
        return 0;
    }
    if (timer < 0x1F5) {
        delta = 0x1F4 - *(short *)(state + 0x32A);
        if (delta > 0) {
            if (func_L02_0031FA68(delta) != 0) {
                return 0;
            }
        }
    }
    *(short *)(state + 0x32A) = 0;
    *(short *)(state + 0x32E) = 0;
    return 1;
}

extern void func_L02_003D78B0(char *);
extern void func_L02_003D7AA0(char *);
extern void func_L02_003D7F88(char *);

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D7858 - run the actor's two-variant update chain: always
func_L02_003D7AA0 (limb set 4/5); then, if the variant byte at +0x95 is 2,
func_L02_003D78B0 (limb set 2/3); then, if the byte at +0x42 is 2,
func_L02_003D7F88.

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D62F0 (v1.01 address), author commit 773eafc2
(llesieur99). ADAPTED per RULING #13291 §7: names, and one prototype added.
Respellings: func_L02_003D62F0 -> func_L02_003D7858 (our v2.00 place);
func_L02_003D6538 -> func_L02_003D7AA0, func_L02_003D6348 -> func_L02_003D78B0,
func_L02_003D6A20 -> func_L02_003D7F88 (relocation-derived). Extern decls
written out; func_L02_003D7F88's `(char *)` prototype is ours: upstream calls
it undeclared, and its definition takes no arguments (byte-neutral, validator
#2594). #include dropped (unused).

*/

void func_L02_003D7858(char *arg0) {
    char *state = arg0;
    unsigned char mode;

    func_L02_003D7AA0(state);
    mode = *(unsigned char *)(state + 0x95);
    if (mode == 2) {
        func_L02_003D78B0(state);
    }
    mode = *(unsigned char *)(state + 0x42);
    if (mode == 2) {
        func_L02_003D7F88(state);
    }
}

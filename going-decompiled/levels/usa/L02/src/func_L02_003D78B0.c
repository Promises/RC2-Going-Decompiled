extern void func_L02_003D78F0(char *, int, int);

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D78B0 - reset limb sets 2 and 3 of the actor:
func_L02_003D78F0 with (2,0) then (3,1).

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D6348 (v1.01 address), author commit 773eafc2
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respellings: func_L02_003D6348 -> func_L02_003D78B0 (our v2.00 place);
func_L02_003D6388 -> func_L02_003D78F0 (relocation-derived). Extern decl
written out; #include dropped (unused).

*/

void func_L02_003D78B0(char *arg0) {
    func_L02_003D78F0(arg0, 2, 0);
    func_L02_003D78F0(arg0, 3, 1);
}

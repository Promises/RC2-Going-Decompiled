extern void func_L02_0031F998(char *, int, int);

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003DC858 - set the actor's animation slot 7 to loop
(func_L02_0031F998 with (7, -1)).

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003DB2F0 (v1.01 address), author commit 773eafc2
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respellings: func_L02_003DB2F0 -> func_L02_003DC858 (our v2.00 place);
func_002A7750 -> func_L02_0031F998 (relocation-derived). Extern decl written
out; #include dropped (unused).

*/

void func_L02_003DC858(char *arg0) {
    func_L02_0031F998(arg0, 7, -1);
}

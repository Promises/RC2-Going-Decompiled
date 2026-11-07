extern void func_L02_003D7188(char *);
extern void func_L02_003174E8(char *);
extern void func_L02_0031F6B8(char *, int, int, int);
extern void func_L02_0031F998(char *, int, int);

/*
Level 02 (Maktar Nebula), USA v2.00, OpenRAC adoption row L02-B (task #2558).

func_L02_003D5E68 - reset the mallet-carrier actor's carried sub-objects. When
the state's slot array at +0x2A0 is non-zero, runs func_L02_003D7188 (release
them); then re-arms the actor (func_L02_0031F6B8 with (0,0,3),
func_L02_003174E8, func_L02_0031F998 with (1,0)), rewrites the flags halfword
at obj+0x34 as (flags & 0xEFFF) | 3, and zeroes obj+0x98 and obj+0x31.

Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
carries no notice of its own), file src/levels/02_maktar_nebula/L02_003D26B0.c,
function func_L02_003D4900 (v1.01 address), author commit 431ac503
(llesieur99). BYTE-IDENTICAL per RULING #13291 §7: verbatim but for names.
Respellings: func_L02_003D4900 -> func_L02_003D5E68 (our v2.00 place);
callees by the v2.00 ROM's own relocations at this place (draft relverify,
task #2558): func_L02_003D5C20 -> func_L02_003D7188,
func_002A05A8 -> func_L02_003174E8, func_002A7470 -> func_L02_0031F6B8,
func_002A7750 -> func_L02_0031F998. Extern decls written out; #include
dropped (unused).

*/

void func_L02_003D5E68(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(int *)(state + 0x2A0) != 0) {
        func_L02_003D7188(arg0);
    }
    func_L02_0031F6B8(arg0, 0, 0, 3);
    func_L02_003174E8(arg0);
    func_L02_0031F998(arg0, 1, 0);
    *(unsigned short *)(arg0 + 0x34) =
        (*(unsigned short *)(arg0 + 0x34) & 0xEFFF) | 3;
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned char *)(arg0 + 0x31) = 0;
}

#!/usr/bin/perl -p
# libgcc_lid_dli.pl — rewrite cc1's `li.d $<GPR>, <decimal>` into
# `dli $<GPR>, 0x<IEEE-754 double bits>` in the assembler input of a libgcc.a
# member (tools/ee/build_libgcc.sh only; task #893).
#
# cc1 2.9-ee-991111 emits li.d for a soft-float double constant held in a GPR;
# SN as.exe expanded it natively, binutils 2.40 refuses it at -march=r5900. This
# is an OUTPUT-side rewrite of the assembler input, the same class as
# move_fixup.sed: the GCC source is never edited (RULING #8206, #893).
# Measured: _fixunsdfdi.o (li.d 2^-32) and _floatdidf.o (li.d 65536.0, 2^32)
# assemble to the ROM's words with this rule, 0 differing words, both regions.
# Scope, deliberately: libgcc members only. It is NOT in move_fixup.sed or
# asm_unit.sh (RULING #8172 clause 4 keeps li.d out of the game-unit pipeline).
# An FPR destination ($f<n>) is the hard-float form, unmeasured: it is left
# alone so the assembler rejects it loudly. Whether the expansion equals SN's
# for a member is decided by the whole-image cmp of landing_gate.sh, not here.
s/^(\s*)li\.d\s+(\$(?:[0-9]+|[a-eg-z]\w*)),\s*(\S+)\s*$/sprintf("%sdli\t%s,0x%016X\n",$1,$2,unpack("Q<",pack("d<",$3)))/e;

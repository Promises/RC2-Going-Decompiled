# Rewrite ee-gcc cc1's `move rd, rs` pseudo to explicit `daddu rd, rs, $zero`.
# GNU `as -mabi=eabi` assembles the `move` pseudo to `or` (0x25), but the EE
# toolchain (and the original game) used `daddu` (0x2d) for register moves
# (25058 daddu-moves vs 2 or-moves in the original disassembly). cc1 emits the
# `move` pseudo, so we must force daddu here for the bytes to match.
#
# Anchored at line start (after indentation) so it only touches real `move`
# instructions, never substrings; a no-op on original/INCLUDE_ASM asm (which
# spimdisasm already renders as explicit `daddu`, never `move`).
s/^([[:space:]]*)move[[:space:]]+(\$[A-Za-z0-9]+),[[:space:]]*(\$[A-Za-z0-9]+)/\1daddu \2, \3, $zero/

# Rewrite cc1's `cvt.w.s $fd, $fs` to its raw encoding. binutils 2.40 refuses
# the mnemonic at -march=r5900 (same reason spimdisasm renders the original
# instruction as a `.word` with a comment), but the EE has it and ee-gcc emits
# it for every float->int cast. Encoding: COP1 fmt=S | fs<<11 | fd<<6 | 0x24.
# GNU as evaluates the shift expression, so the register numbers can be pasted
# in textually.
s/^([[:space:]]*)cvt\.w\.s[[:space:]]+\$f([0-9]+),[[:space:]]*\$f([0-9]+)/\1.word 0x46000024+(\3<<11)+(\2<<6) # cvt.w.s $f\2,$f\3/

# Rewrite cc1's div-by-zero trap `break 7` to `break 0,7`. The SN ee-as
# encoded cc1's `break 7` as 0x000001CD (code 7 in the LOW code field, which
# GNU as spells `break 0,7`); GNU as puts a bare `break 7` in the HIGH field
# (0x0007000D), which never occurs in the original binaries. Measured: every
# compiler-emitted break in BOTH regions' disassembly is 0x000001CD (the only
# other break anywhere is one handwritten `break 1023,1023`, asm-only). Only
# affects div/mod-using functions, none of which could match before this fix.
s/^([[:space:]]*)break[[:space:]]+7[[:space:]]*\r?$/\1break 0,7/

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

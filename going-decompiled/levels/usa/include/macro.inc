# Assembler macros for the split LEVEL asm (going-decompiled/levels/usa/LNN/asm),
# for GNU as. Level-local on purpose: the level build must not read the boot's
# gitignored going-decompiled/build/usa/include, and must not add to
# going-decompiled/include/ (an EU build input, RULING #7207). Same macro names and
# operands as splat 0.41 emits for the boot; none of them emits a byte, they only
# define symbols, so the section bytes are the asm's own. tools/levels/README.md.

.ifndef _LEVEL_MACRO_INC_GUARD
.internal _LEVEL_MACRO_INC_GUARD
.set _LEVEL_MACRO_INC_GUARD, 1

# function start / end
.macro glabel label, visibility=global
    .\visibility "\label"
    .type "\label", @function
    "\label":
        .ent "\label"
.endm

.macro endlabel label
    .size "\label", . - "\label"
    .end "\label"
.endm

# alternative entry point inside a function
.macro alabel label, visibility=global
    .\visibility "\label"
    .type "\label", @function
    "\label":
        .aent "\label"
.endm

# label referenced by an error-handler table
.macro ehlabel label, visibility=global
    .\visibility "\label"
    "\label":
.endm

# label referenced by a jump table
.macro jlabel label, visibility=global
    .\visibility "\label"
    "\label":
.endm

# data symbol start / end
.macro dlabel label, visibility=global
    .\visibility "\label"
    .type "\label", @object
    "\label":
.endm

.macro enddlabel label
    .size "\label", . - "\label"
.endm

# splat's "not yet matched" marker: a zero-byte <label>.NON_MATCHING object symbol
.macro nonmatching label, size=1
    .global "\label\().NON_MATCHING"
    .type "\label\().NON_MATCHING", @object
    .size "\label\().NON_MATCHING", \size
    "\label\().NON_MATCHING":
.endm

.endif

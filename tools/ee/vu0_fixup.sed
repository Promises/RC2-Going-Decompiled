# vu0_fixup.sed — make spimdisasm's VU0 (COP2) macro-mode asm assemble with
# mips-linux-gnu-as (binutils 2.40, -march=r5900).
#
# WHY: spimdisasm emits the VU0 macro special registers Q and ACC as bare
# tokens (e.g. `vaddq.x $vf5, $vf0, Q`, `vdiv Q, $vf0w, $vf5x`,
# `vmulax.xyzw ACC, $vf6, $vf4x`). GNU as wants them written with a `$`
# prefix (`$Q`, `$ACC`) — the operand *layout* spimdisasm uses is otherwise
# exactly what GNU as expects, and the encoding is byte-identical (proven:
# vaddq->6001004B, vdiv->BC03654A, vmulq->9C21C04B, vmulax->BC31E44B, etc).
#
# This filter ONLY rewrites the operand tokens; it is a no-op on every other
# line, so it is safe to run over all asm unconditionally.
#
# IMPORTANT exclusions:
#  * `vsqrt`/`vrsqrt` are NOT touched: GNU as 2.40 encodes `vsqrt` differently
#    from the real R5900 (4A2503BD vs the hardware's 4A0503BD), so spimdisasm
#    already emits those as raw `.word` with the disasm in a trailing comment.
#    Those comments contain a bare `Q` (`# vsqrt Q, $vf7w`) which must survive
#    verbatim — handled by only transforming the part of the line before `#`.
#
# Usage:  sed -f tools/ee/vu0_fixup.sed < in.s > out.s
#         (or piped:  ... | sed -f tools/ee/vu0_fixup.sed | mips-linux-gnu-as ...)

# Operate only on lines that actually carry a `v...` macro op with a bare
# Q/ACC operand, and never disturb a trailing `#` comment: we anchor on the
# operand context (a `$vfNN...` register followed by `, Q`/`, ACC`, or the
# instruction's first operand being `Q,`/`ACC,`).

# 1) trailing source operand:  "... , Q"  /  "... , ACC"  (before any '#')
s/\(,[[:space:]]*\)\(Q\|ACC\)\([[:space:]]*\)$/\1$\2\3/

# 2) trailing source operand followed by a '#' comment: "... , Q  # ..."
s/\(,[[:space:]]*\)\(Q\|ACC\)\([[:space:]]*#\)/\1$\2\3/

# 3) destination operand on vdiv/vsqrt-style ops:  "<mnemonic> Q, ..." /
#    "<mnemonic> ACC, ...".  Anchored to a `*/`-closed address comment so it
#    only ever fires inside the instruction text, never inside a `#` comment
#    (where `Q,` may legitimately appear, e.g. `# vsqrt Q, $vf7w`).
s/\(\*\/[[:space:]]*v[a-z0-9.]\+[[:space:]]\+\)\(Q\|ACC\)\(,\)/\1$\2\3/

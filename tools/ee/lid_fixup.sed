# CALT-overlay ACCEPTANCE fixup: expand cc1's `li.d $<GPR>, 1.0` pseudo.
#
# WHY THIS FILE IS SEPARATE FROM move_fixup.sed:
# move_fixup.sed belongs to the BYTE-MATCHING pipeline (tools/ee/build.sh) and
# build_conly.sh extracts its rule r2 BY LINE NUMBER. Adding a rule there would
# shift those lines and silently change which rule the CALT overlay invokes.
# Ruling #20508 is explicit that it does not touch the byte-matching pipeline,
# so this rule lives in its own file. move_fixup.sed stays byte-identical to
# origin/master.
#
# THE CONSTRUCT: cc1 emits `li.d` for a soft-float double materialised into a
# GPR immediately before `jal fptodp`. binutils 2.40 refuses the mnemonic at
# -march=r5900 ("opcode not supported on this processor: r5900 (mips3)"), so it
# is acceptance-blocking for the GNU-as CALT path exactly as cvt.w.s is.
#
# THE EXPANSION, measured from SN as.exe (ledger fact #20515, decode verified by
# re-assembling the mnemonics with GNU as and comparing bytes, with a sa=13
# negative control proving the comparison can fail):
#     li.d $r, 1.0   ->   ori    $r, $0, 0xffc0     0x3400FFC0 | (r<<16)
#                         dsll32 $r, $r,  14        0x0000003C | (r<<16)
#                                                              | (r<<11)
#                                                              | (14<<6)
# 0xffc0 << 46 == 0x3FF0000000000000 == IEEE-754 1.0. The destination register
# is carried by the mnemonic, so one rule covers every site. No relocation, no
# .lit8/.lit4 pool. This happens to reproduce SN's own bytes, which is a bonus:
# the CALT overlay at 0xC00000 is functional, not byte-matching.
#
# ⚠️ THIS RULE ENCODES THE CONSTANT 1.0 AND NOTHING ELSE, DELIBERATELY.
# The expansion is value-dependent (0xffc0 and the shift 14 are chosen to
# materialise 0x3FF0000000000000 minimally); #20515 measured 1.0 only and states
# no general algorithm. li.d is cc1-synthesised — 1D54C0.c's #else arms contain
# zero double literals yet emit two li.d (#20565) — so the constants that will
# appear CANNOT be enumerated from source in advance.
# ⇒ Anything this rule does not rewrite survives into the output, and
# tools/ee/lid_guard.sh FAILS THE BUILD on it, naming file, line and constant.
# The guard's notion of "handled" is literally "this sed rewrote it", so the two
# cannot drift apart. Do not widen this pattern without measuring the expansion
# for the constant you are widening it to.
#
# Registers: numeric ($20) or a GPR name not beginning with `f`. `$f<n>` is
# deliberately EXCLUDED — li.d into an FPR is the hard-float construct, whose
# expansion is different and unmeasured; it falls through to the guard.
s/^([[:space:]]*)li\.d[[:space:]]+(\$([0-9]+|[a-eg-z][a-z0-9]*)),[[:space:]]*1\.0*([eE]\+?0+)?[[:space:]]*\r?$/\1ori \2, $0, 0xffc0\n\1dsll32 \2, \2, 14/

# unit_flags.sh — per-unit -G / cc1 flags for the EE c-unit compile.
#
# Sets GFLAG (+ CC1EXTRA) for a given src/<...>.c path. The originally nonzero-G
# sub-TUs use uniform %gp_rel small-data, so they MUST compile at -G8 to be
# byte-exact; everything else is -G0. The C-only build (build_conly.sh /
# conly_finish.sh) sources this so its NORMAL objects are byte-exact — at -G0 a
# -G8 unit emits absolute (lui/%lo) addressing instead of the 1-instr %gp_rel
# store, so the unit compiles LONGER, every function after it drifts, and the
# .data/.lit `.word <func>` pointer tables resolve to wrong linked addresses
# (boot reads a struct ptr that's off -> null/garbage deref). Safe now that the
# gp-window globals are correctly placed (the .cod_bss anchor pin 36fcf47 + the
# cod mis-split recoveries 347edf4/06333c5) so %gp_rel no longer overflows.
#
# MUST stay in sync with the inline case in build.sh / objdiff_build.sh / diff.sh.
unit_flags() {
  c="$1"
  GFLAG="-G0"; CC1EXTRA=""
  case "$c" in
    */cod/0321A0.c) GFLAG="-G8";;
    */usa/text/183178.c) GFLAG="-G8";;
    */usa/text/188580.c) GFLAG="-G8";;
    */usa/text/188858.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/1907F0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/191238.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/198FA0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/1A00F0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/1A8180.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/250080.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/16E980.c) GFLAG="-G8";;
    */usa/text/248B50.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/235FE8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing";; # -fsa restores per-store pointer-member reload in GuiElementSetPos/Scale (#75)
    */usa/text/1CA080.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/1D54C0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/1B4218.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/178E88.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/1823B8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/1DFF80.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/1EFFC0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/1FCF48.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/1FFBA0.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */usa/text/24D728.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */eu/text/16E7B8.c) GFLAG="-G8";;
    */eu/text/183088.c) GFLAG="-G8";;
    */eu/text/188470.c) GFLAG="-G8";;
    */eu/text/190808.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */eu/text/198B58.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */eu/text/1A7D10.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */eu/text/1C9F58.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */eu/text/1D5488.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */eu/text/236ED8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse -fno-strict-aliasing";; # USA 235FE8 twin (#75 -fsa parity)
    */eu/text/249FE8.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */eu/text/251520.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */eu/text/188748.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */eu/text/191240.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
    */eu/text/19FC78.c) GFLAG="-G8"; CC1EXTRA="-fno-gcse";;
  esac
}

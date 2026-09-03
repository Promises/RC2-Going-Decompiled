# overlay_syms_select.sh — choose the overlay link's PROVIDE symbol scripts.
# SOURCED (not executed) by overlay_package.sh step 4. Sets $SYMS_LD.
#
# Requires from the caller: BUILD, ASM, SRC, SA  (and CWD = repo root).
#
# WHY THIS IS A SEPARATE FILE: it is the only part of step 4 that can be
# exercised without a VM, a linker or a build. Kept inline it was untestable,
# and an untested branch is how the defect below survived. tools/ee/.t20238/
# arms.sh drives THIS FILE in all three arms.
#
# ── THE DEFECT THIS REPLACES ──────────────────────────────────────────────
# The old test was `[ -s "$BUILD/aas_conly.ld" ]` — exists and is non-empty.
# That is satisfied by an arbitrarily old file and unsatisfied only by a
# MISSING one. So the script's correctness rested on the artefact not
# existing: workers regenerate because `-s` fails, not because anything
# checked freshness. aas_conly.ld is UNTRACKED and $BUILD survives a branch
# switch, so a checkout can co-locate a months-old artefact with today's
# inputs, and the link would silently prefer the old one.
#
# ── WHY "STALE ⇒ FALL BACK" AND NOT "STALE ⇒ REBUILD" ─────────────────────
# Nothing in this repo writes aas_conly.ld — its provenance is unknown and no
# writer exists in any git ref. So it cannot be regenerated in place. The
# regeneration that IS available is step 3, which rebuilds aas_prov.ld and
# named_prov.ld from the current inputs on every run. Dropping a stale
# aas_conly.ld therefore lands on exactly the ABSENT path, which is unchanged.
#
# ── INPUT SET ────────────────────────────────────────────────────────────
# aas_conly.ld stands in for the else-arm, so its inputs are the else-arm's:
#   $ASM, $SRC                      -> aas_prov.ld   (overlay_package.sh:72-77)
#   $SA/symbol_addrs.txt,
#   $SA/cheat_globals.staged.txt    -> named_prov.ld (overlay_package.sh:80-85)
#   tools/ee/eetest/ghidra_named_funcs.ld            (tracked; no generator here)
# A missing input contributes nothing — a file that does not exist cannot make
# the artefact stale.
#
# ── SCOPE / WHAT THIS DOES NOT DO ────────────────────────────────────────
# This is a FRESHNESS verdict only. It does NOT address the separate finding
# of 833a28be (not on origin/master; on d2/backflow-partition) that NEITHER
# side dominates — aas_conly is the last carrier of 7 alias globals the
# generated trio lacks, while the trio supplies 66 aas_conly lacks. Rejecting
# a stale aas_conly therefore COSTS those 7. Whether that trade is right is a
# ruling, not a measurement. See tools/ee/.t20238/REPORT.md.

AAS_CONLY="$BUILD/aas_conly.ld"
SYMS_GENERATED="-T $BUILD/aas_prov.ld -T $BUILD/named_prov.ld -T tools/ee/eetest/ghidra_named_funcs.ld"
AAS_INPUTS="$ASM $SRC $SA/symbol_addrs.txt $SA/cheat_globals.staged.txt tools/ee/eetest/ghidra_named_funcs.ld"

# Age + short hash. NOT a PROVIDE count: a count tells you the file has
# contents, it never tells you they are the RIGHT contents. 23,901 PROVIDEs is
# exactly as reassuring when the file is two months stale as when it is fresh.
overlay_syms_describe() {
  _osd_f=$1
  _osd_m=$(/usr/bin/stat -f %m "$_osd_f" 2>/dev/null || /usr/bin/stat -c %Y "$_osd_f" 2>/dev/null || echo 0)
  _osd_age=$(( ( $(date +%s) - _osd_m ) / 86400 ))
  _osd_h=$(/usr/bin/shasum -a 256 "$_osd_f" 2>/dev/null | cut -c1-12)
  echo "age ${_osd_age}d, sha256 ${_osd_h}"
}

# Print every input strictly newer than $1. Empty output == fresh.
# THE REACHABLE NEGATIVE: this prints filenames. If it can never print, the
# check is decoration — tools/ee/.t20238/arms.sh makes it print.
overlay_syms_newer_inputs() {
  /usr/bin/find $AAS_INPUTS -type f -newer "$1" -print 2>/dev/null
}

if [ ! -s "$AAS_CONLY" ]; then
  # ── ARM: ABSENT ── behaviour and output deliberately BYTE-IDENTICAL to the
  # pre-fix script. This is the only arm exercised in practice today; changing
  # it would be a regression in the one path every worker relies on.
  SYMS_LD="$SYMS_GENERATED"
  echo "   using generated aas_prov + named_prov + ghidra_named_funcs"
else
  _osd_stale=$(overlay_syms_newer_inputs "$AAS_CONLY")
  if [ -n "$_osd_stale" ]; then
    # ── ARM: PRESENT-STALE ── reject, and say WHICH input outran it.
    SYMS_LD="$SYMS_GENERATED"
    echo "   !! aas_conly.ld STALE ($(overlay_syms_describe "$AAS_CONLY")) — REJECTED"
    echo "      $(echo "$_osd_stale" | wc -l | tr -d ' ') input(s) newer than it (showing up to 5):"
    echo "$_osd_stale" | head -5 | sed 's/^/        /'
    echo "   using generated aas_prov + named_prov + ghidra_named_funcs"
  else
    # ── ARM: PRESENT-FRESH ── accept.
    SYMS_LD="-T $AAS_CONLY"
    echo "   using comprehensive aas_conly.ld ($(overlay_syms_describe "$AAS_CONLY")) — FRESH"
  fi
fi

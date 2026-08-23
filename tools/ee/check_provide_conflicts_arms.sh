#!/bin/sh
# check_provide_conflicts_arms.sh — the aliveness proof for check_provide_conflicts.py.
#
# A gate that is silent on a clean input and silent on a dirty one is
# indistinguishable from a gate that never ran.  ARM 1 is what proves this one
# is alive; ARM 3's zero is only meaningful cited against it, from the same gate
# in the same run.  Run this after ANY change to check_provide_conflicts.py.
#
#   sh tools/ee/check_provide_conflicts_arms.sh
#   exit 0 = all four arms behaved as specified.
#
# Inputs come straight out of git, so this needs no checkout of the branch under
# test and cannot be perturbed by the working tree.
set -u
cd "$(dirname "$0")/../.."
GATE="/usr/bin/python3 tools/ee/check_provide_conflicts.py"
BASE_REF="${BASE_REF:-origin/master}"
BRANCH_REF="${BRANCH_REF:-f4ea6aae}"   # d3/conly-alias-32's conly_provides.ld — 37 PROVIDEs
T="$(mktemp -d)"
trap 'rm -rf "$T"' EXIT

git show "$BRANCH_REF:tools/ee/conly_provides.ld"          > "$T/branch_conly.ld"  || exit 2
git show "$BASE_REF:tools/ee/conly_provides.ld"            > "$T/master_conly.ld"  || exit 2
git show "$BASE_REF:tools/ee/eetest/ghidra_named_funcs.ld" > "$T/baseline.ld"      || exit 2
printf 'PROVIDE(iSignalSema = func_00352000);\n' > "$T/arm2_pos.ld"
printf 'PROVIDE(FlushCache  = g_dialogVoiceActive);\n' > "$T/arm4_pos.ld"

fails=0
check() { # label expected_exit must_match must_not_match
  label="$1"; want="$2"; yes="$3"; no="$4"; shift 4
  out="$($GATE "$@" 2>&1)"; got=$?
  echo "########## $label  (exit $got, wanted $want)"
  echo "$out"
  [ "$got" = "$want" ] || { echo "!! $label: WRONG EXIT"; fails=$((fails+1)); }
  if [ -n "$yes" ]; then
    echo "$out" | /usr/bin/grep -q "$yes" || { echo "!! $label: expected to see '$yes'"; fails=$((fails+1)); }
  fi
  if [ -n "$no" ]; then
    echo "$out" | /usr/bin/grep -q "$no" && { echo "!! $label: must NOT mention '$no'"; fails=$((fails+1)); }
  fi
  echo
}

# ARM 1  KNOWN POSITIVE — the bug that motivated this gate. Silence here = broken.
check "ARM 1 KNOWN POSITIVE (branch conly_provides vs baseline)" 1 \
      'SignalSema               NEW func_00352000 (0x00352000)' 'iSignalSema' \
      "$T/branch_conly.ld" "$T/baseline.ld"

# ARM 2  KNOWN NEGATIVE — the 5 agreeing overlaps stay out of DISAGREE (ARM 1's
# DISAGREE block lists exactly one member), and `iSignalSema` is not conflated
# with `SignalSema` (the must-not-match above).  Its positive control, on the
# QUERY FORM rather than on the subject: the same gate DOES report iSignalSema
# when iSignalSema is genuinely the overlapping name.
check "ARM 2 POSITIVE CONTROL (gate can see iSignalSema when it is real)" 1 \
      'iSignalSema              NEW func_00352000' '' \
      "$T/arm2_pos.ld" "$T/baseline.ld"

# ARM 3  CLEAN CONTROL — master's 5 PROVIDEs overlap the baseline nowhere.
check "ARM 3 CLEAN CONTROL (master conly_provides vs baseline)" 0 \
      'RESULT: 0 disagreeing + 0 uncomparable' 'SignalSema' \
      "$T/master_conly.ld" "$T/baseline.ld"

# ARM 4  The UNCOMPARABLE bucket is reachable — an overlap whose value form we
# cannot resolve must be reported, not silently filed as agreement.
check "ARM 4 UNCOMPARABLE reachable" 1 \
      'UNCOMPARABLE overlap (1)' '' \
      "$T/arm4_pos.ld" "$T/baseline.ld"

if [ "$fails" -eq 0 ]; then echo "ALL ARMS OK"; exit 0; fi
echo "ARM FAILURES: $fails"; exit 1

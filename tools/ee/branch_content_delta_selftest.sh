#!/usr/bin/env bash
# branch_content_delta_selftest.sh - drive branch_content_delta.sh against a
# synthetic repo whose verdicts are known by construction.
#
# WHY SYNTHETIC AND NOT "run it on preserve/d1-stack": a control anchored on a
# live branch expires the moment that branch lands or master advances past it,
# and it expires SILENTLY - the assertion just starts describing a different
# world. This builds the world it asserts about, so it cannot go stale.
#
# Every case is a MUTANT of the case above it, differing in exactly one term, so
# a pass localises: BRANCH vs BOTH differ only in whether master also moved.
#
# MUTATION-PROVEN (3 mutants, 3 kills, each localised to the case that owns it):
#   invert the BRANCH/BOTH test        -> 4 feat cases fail
#   restore the early-exit on ??       -> the sweep case fails
#   collapse the two zero-causes       -> only the netempty case fails
# ALWAYS `bash -n` A MUTANT BEFORE BELIEVING IT SURVIVED OR DIED: my first attempt
# at the third one put an apostrophe inside a single-quoted printf, and EVERY case
# failed. That reads like a devastating kill and is really a broken script.
#
# Exit: 0 = all cases pass / 1 = a case failed / 2 = could not look
set -u

TOOL="$(cd "$(dirname "$0")" && pwd)/branch_content_delta.sh"
[ -x "$TOOL" ] || { echo "$TOOL not executable: could not look" >&2; exit 2; }

R=$(mktemp -d) || { echo "mktemp failed: could not look" >&2; exit 2; }
trap 'rm -rf "$R"' EXIT
cd "$R" || exit 2

g() { git -c user.name=selftest -c user.email=selftest@invalid "$@"; }

g init -q -b main .
printf 'base\n' > shared.txt
printf 'base\n' > branch_only.txt
printf 'base\n' > master_only.txt
g add -A; g commit -qm base
BASE_SHA=$(g rev-parse HEAD)

# --- the branch: touches shared.txt and branch_only.txt -----------------------
# TWO branch-only files against ONE contested, deliberately: with 1 and 1 the
# summary line reads "(branch-only 1 · contested 1)" whether or not the direction
# test is inverted, and a mutant that swaps BRANCH for BOTH SURVIVES. Measured -
# that is exactly what the first version of this selftest did. Asymmetric counts
# plus the per-file labels below are what make the mutation observable.
g checkout -qb feat
printf 'branch\n' > shared.txt
printf 'branch\n' > branch_only.txt
printf 'added\n'  > branch_added.txt   # absent on base AND on master -> still branch-only
g add -A
g commit -qm feat

# --- a branch whose own commits cancel ---------------------------------------
g checkout -q -b netempty "$BASE_SHA"
printf 'x\n' > branch_only.txt; g commit -qam churn
printf 'base\n' > branch_only.txt; g commit -qam revert

# --- a branch whose content master will also hold ------------------------------
g checkout -q -b upstreamed "$BASE_SHA"
printf 'agreed\n' > master_only.txt; g commit -qam agreed

# --- master moves: contests shared.txt, adopts upstreamed's content -----------
g checkout -q main
printf 'master\n' > shared.txt
g commit -qam master-moves
printf 'agreed\n' > master_only.txt
g commit -qam adopt

fail=0
check() { # <label> <expected-substring> <branch>
  out=$(BASE=main "$TOOL" "$3" 2>&1)
  case "$out" in
    *"$2"*) printf 'ok   %s\n' "$1" ;;
    *) fail=1; printf 'FAIL %s\n     expected: %s\n     got:      %s\n' "$1" "$2" "$out" ;;
  esac
}

# shared.txt: both moved -> contested. branch_only.txt: only feat moved.
# master_only.txt is NOT in feat's touched set and must not appear at all.
check "feat: 3 differing, 2 branch-only, 1 contested" \
      "3 feat  (branch-only 2 · contested 1)" feat
# The counts alone do not pin the DIRECTION - assert which file got which label.
check "feat: shared.txt is CONTESTED (both sides moved it)" "BOTH   shared.txt" feat
check "feat: branch_only.txt is BRANCH-only (master never moved it)" \
      "BRANCH branch_only.txt" feat
check "feat: a file added only on the branch is BRANCH-only" \
      "BRANCH branch_added.txt" feat
# (a `check <label> "" feat` once stood here. Every string contains the empty
#  string, so it passed unconditionally - a clause that cannot fail is not a
#  test. Deleted rather than repaired; the case below actually asserts it.)
case "$(BASE=main "$TOOL" feat 2>&1)" in
  *master_only.txt*) fail=1; echo "FAIL feat: master_only.txt leaked into the report";;
  *) echo "ok   feat: master_only.txt absent from the report";;
esac

check "netempty: reported as commits-cancel, not as upstream" \
      "net-empty" netempty
check "upstreamed: reported as blobs-already-on-master" \
      "already hold master's blob" upstreamed

# exit bands
BASE=main "$TOOL" upstreamed >/dev/null 2>&1
[ $? -eq 0 ] || { fail=1; echo "FAIL band: a clean branch must exit 0"; }
BASE=main "$TOOL" feat >/dev/null 2>&1
[ $? -eq 1 ] || { fail=1; echo "FAIL band: a carrying branch must exit 1"; }
BASE=main "$TOOL" >/dev/null 2>&1
[ $? -eq 3 ] || { fail=1; echo "FAIL band: no arguments must exit 3"; }

# An unexaminable member must NOT abort the sweep - the defect this tool shipped
# on its first run. `feat` must still be measured, and the run must band 2.
g checkout -q --orphan orphan
g rm -qrf . >/dev/null 2>&1 || true
printf 'orphan\n' > o.txt; g add -A; g commit -qm orphan
sweep=$(BASE=main "$TOOL" orphan feat 2>&1); rc=$?
case "$sweep" in
  *"feat"*) echo "ok   sweep: continues past an unexaminable member";;
  *) fail=1; printf 'FAIL sweep: aborted before feat\n     got: %s\n' "$sweep";;
esac
[ "$rc" -eq 2 ] || { fail=1; echo "FAIL sweep: must band 2 when a member was unexaminable (got $rc)"; }

[ "$fail" -eq 0 ] && { echo "all cases pass"; exit 0; }
exit 1

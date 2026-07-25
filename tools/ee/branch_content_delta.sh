#!/usr/bin/env bash
# branch_content_delta.sh - for each branch, does its own touched-file set still
# DIFFER from master by CONTENT?
#
# `git cherry` is patch-id keyed, so a commit that landed after a rebase with
# conflict resolution gets a NEW patch-id and reports as `+` (unlanded) even
# though its content is upstream. That is a false-POSITIVE direction: cherry is
# an UPPER BOUND on unlanded work, not a measurement of it.
#
# This asks the content question instead: of the files the branch itself
# touched (merge-base..tip), how many hold a different blob on the branch than
# on master? Zero means the branch's content is fully upstream however it got
# there. Nonzero names the files, so the number ships with its members.
#
# Usage: branch_content_delta.sh <branch> [<branch> ...]
# Exit:  0 = every branch fully upstream / 1 = at least one carries content
#        2 = could not look / 3 = usage
set -u

[ $# -ge 1 ] || { echo "usage: $0 <branch> [...]" >&2; exit 3; }

# Overridable ONLY so branch_content_delta_selftest.sh can drive it against a
# synthetic repo with known verdicts. Every real invocation leaves it alone.
BASE="${BASE:-origin/master}"
git rev-parse --verify --quiet "$BASE" >/dev/null || {
  echo "$BASE does not resolve: could not look" >&2; exit 2; }

# An UNEXAMINABLE member must not abort the sweep: a checker that early-exits
# cannot count, and the partial output is indistinguishable from a clean one.
# Unexaminables are collected and reported; the run ends on band 2 only after
# every examinable branch has been measured.
found=0
unexaminable=0
for b in "$@"; do
  mb=$(git merge-base "$BASE" "$b" 2>/dev/null) || {
    echo "?? $b  no merge-base with $BASE: could not look" >&2
    unexaminable=$((unexaminable + 1))
    continue; }

  # files the BRANCH touched, its own side only
  files=$(git diff --name-only "$mb".."$b")
  # A 0 has TWO causes and they are not the same finding. Both mean "landing it
  # changes nothing", but one says the work is upstream and the other says the
  # branch never contributed any. Print the cause, not just the endpoint.
  [ -n "$files" ] || {
    printf '0 %s  (net-empty: tip tree == merge-base tree; its commits cancel)\n' "$b"
    continue; }

  # "differs from master" is NOT "the branch carries work". Both sides can move,
  # and a stale branch whose file master has since advanced differs for the
  # OPPOSITE reason. The merge-base blob decides it:
  #   master == base   master never touched f  -> the branch is the only mover  (BRANCH)
  #   master != base   both moved              -> undecidable from blobs alone  (BOTH)
  # BOTH is where a reset-style landing silently reverts master. It needs eyes.
  differing=$(
    printf '%s\n' "$files" | while IFS= read -r f; do
      [ -n "$f" ] || continue
      x=$(git rev-parse --quiet --verify "$b:$f" 2>/dev/null || echo ABSENT)
      y=$(git rev-parse --quiet --verify "$BASE:$f" 2>/dev/null || echo ABSENT)
      [ "$x" = "$y" ] && continue
      z=$(git rev-parse --quiet --verify "$mb:$f" 2>/dev/null || echo ABSENT)
      if [ "$y" = "$z" ]; then printf 'BRANCH %s\n' "$f"; else printf 'BOTH   %s\n' "$f"; fi
    done
  )

  n=$(printf '%s' "$differing" | /usr/bin/grep -c . )
  if [ "$n" -eq 0 ]; then
    printf '0 %s  (all %s touched file(s) already hold master'\''s blob)\n' \
      "$b" "$(printf '%s\n' "$files" | /usr/bin/grep -c .)"
  else
    printf '%s %s  (branch-only %s · contested %s)\n' "$n" "$b" \
      "$(printf '%s\n' "$differing" | /usr/bin/grep -c '^BRANCH ')" \
      "$(printf '%s\n' "$differing" | /usr/bin/grep -c '^BOTH   ')"
  fi
  if [ "$n" -gt 0 ]; then
    found=1
    printf '%s\n' "$differing" | sed 's/^/    /'
  fi
done

[ "$unexaminable" -eq 0 ] || {
  echo "$unexaminable branch(es) unexaminable; see ?? lines above" >&2; exit 2; }
[ "$found" -eq 0 ] && exit 0
exit 1

#!/usr/bin/env python3
"""Would landing this branch DESTROY work that is already on the base?

WHY (measured, four times in one shift): a branch forked from an old master and landed
as-is silently reverts everything the base gained meanwhile. The near-miss that prompted
this would have deleted all five EU cmp runners - the entire EU dynamic-oracle
capability - plus defcheck.py, guard_blocks.py and float_corpus_sweep.py.

WHAT THE NUMBER MEANS - corrected after gating (watcher-m1-5, forum/3212). An earlier
version of the message said "landing this as-is would remove the files above",
unqualified, and that is FALSE for `git merge`: merging the branch that prompted this
tool into master deletes ZERO files, verified with `git merge-tree --write-tree`. The
files are only lost by a REPLACEMENT-style landing - squash to the branch's tree,
reset --hard, force-push. What the count always means is that the branch was authored
and tested WITHOUT those files, so its results do not describe the current base. That
is a staleness signal, and it was worth keeping once it stopped claiming to be a
deletion prediction.

THE NAIVE CHECK IS WORSE THAN NOTHING. `git diff base..branch` on that same branch
reports 7601 deletions, essentially all of them phantom: the base's own newer commits
seen from a stale fork point. A reviewer who looks at that number learns nothing and
may well conclude the tool is noisy. The signal is 30 files, and it only appears once
you subtract what the branch genuinely removed:

    AT RISK = (files on BASE) - (files on BRANCH TIP) - (files the BRANCH deleted itself)

That third term is the whole trick. Without it you cannot tell "the base moved on"
from "this branch removed something on purpose", and both look like deletions.

SCOPE - what this does NOT answer. This is a FILE-set check: it sees a file that would
vanish, never a file whose CONTENTS would be reverted. A branch that stales a file
in place is invisible here; tools/eu-lockstep/merge_superset_check.py is the
within-file counterpart. Two tools, two questions, neither subsumes the other.

Exit: 0 nothing at risk · 1 at-risk files found · 2 could not look · 3 usage error.
A nonzero exit is not automatically a finding: 2 and 3 mean the question was never asked.
"""
import argparse
import subprocess
import sys


def git(*args, check=False):
    p = subprocess.run(("git",) + args, capture_output=True, text=True)
    if check and p.returncode:
        raise OSError((p.stderr or "").strip() or "git failed")
    return p.stdout


def exists(ref, path, repo=None):
    """Does `path` exist at `ref`?

    Answered by EXIT STATUS. `git rev-parse REF:PATH` echoes its unresolved argument
    to stdout on a missing path, so a caller testing "did I get output?" is satisfied
    by the very string that means failure - measured: it reported 0 missing files
    across 47 branches, when the true count was 17.
    """
    cmd = ["git"] + (["-C", repo] if repo else []) + ["cat-file", "-e", f"{ref}:{path}"]
    return subprocess.run(cmd, capture_output=True).returncode == 0


def at_risk(branch, base, repo=None):
    """(behind, [paths]) - files on `base` that landing `branch` as-is would remove."""
    def g(*a):
        return git(*(("-C", repo) + a if repo else a))
    mb = g("merge-base", branch, base).strip()
    if not mb:
        raise OSError(f"no merge-base between {branch} and {base}")
    behind = int(g("rev-list", "--count", f"{branch}..{base}").strip() or 0)
    base_files = set(g("ls-tree", "-r", "--name-only", base).splitlines())
    tip_files = set(g("ls-tree", "-r", "--name-only", branch).splitlines())
    deliberate = {l.split("\t")[-1]
                  for l in g("diff", "--name-status", mb, branch).splitlines()
                  if l.startswith("D\t")}
    return behind, sorted(base_files - tip_files - deliberate)


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("branch", nargs="?")
    ap.add_argument("--base", default="origin/master")
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args(argv[1:])
    if a.selftest:
        return selftest()
    if not a.branch:
        ap.print_usage(sys.stderr)
        return 3
    try:
        behind, risk = at_risk(a.branch, a.base)
    except OSError as e:
        print(f"COULD-NOT-LOOK: {e}", file=sys.stderr)
        return 2
    print(f"branch {a.branch}  vs base {a.base}")
    print(f"  commits behind base : {behind}")
    print(f"  files AT RISK       : {len(risk)}")
    for p in risk:
        print(f"     {p}")
    if risk:
        print("\nThese files exist on the base, are absent from this branch, and this branch\n"
              "never deleted them - so the branch was authored and TESTED without them.\n"
              "\n"
              "  git merge      -> they SURVIVE. Measured: merging the branch that prompted\n"
              "                    this tool into master deletes 0 files.\n"
              "  REPLACEMENT    -> they are LOST. Squash-to-branch-tree, reset --hard, a\n"
              "                    force-push, or any landing that takes the branch's tree.\n"
              "\n"
              "So this is a STALENESS signal, not a deletion prediction: the branch's results\n"
              "do not describe the current base. Rebase before trusting them.")
    return 1 if risk else 0


def selftest():
    """Real git repos, not mocks - a mocked git would test the mock.

    Three arms, and the third is the one that makes the predicate non-trivial:
    a branch that deletes a file ON PURPOSE must NOT be flagged for it.
    """
    import tempfile, os, shutil
    tmp = tempfile.mkdtemp()
    P, F = 0, 0

    def check(label, got, want):
        nonlocal P, F
        if got == want:
            P += 1
        else:
            F += 1
            print(f"  FAIL {label}: got {got!r} want {want!r}", file=sys.stderr)

    def g(*a):
        return git("-C", tmp, *a)
    try:
        g("init", "-q", "-b", "main")
        g("config", "user.email", "t@t"); g("config", "user.name", "t")
        for name in ("keep.txt", "doomed.txt"):
            open(os.path.join(tmp, name), "w").write("x\n")
        g("add", "-A"); g("commit", "-qm", "base")
        g("branch", "stale")

        # the base moves on, gaining a file the branch will never have
        open(os.path.join(tmp, "new_on_base.txt"), "w").write("y\n")
        g("add", "-A"); g("commit", "-qm", "base gains a file")

        behind, risk = at_risk("stale", "main", repo=tmp)
        check("stale branch is behind", behind, 1)
        check("base's new file is AT RISK", risk, ["new_on_base.txt"])

        # a branch that deletes a file DELIBERATELY must not be flagged for it
        g("checkout", "-q", "stale")
        os.remove(os.path.join(tmp, "doomed.txt"))
        g("add", "-A"); g("commit", "-qm", "branch removes doomed.txt on purpose")
        _, risk2 = at_risk("stale", "main", repo=tmp)
        check("deliberate deletion NOT flagged", risk2, ["new_on_base.txt"])

        # once caught up, nothing is at risk
        g("merge", "-q", "main", "-m", "catch up")
        behind3, risk3 = at_risk("stale", "main", repo=tmp)
        check("caught-up branch is not behind", behind3, 0)
        check("caught-up branch has nothing at risk", risk3, [])

        # existence must be answered by exit status, not by rev-parse's echo
        check("exists() true for a real path", exists("main", "keep.txt", repo=tmp), True)
        check("exists() false for a missing path",
              exists("main", "no_such_file.txt", repo=tmp), False)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    print(f"\nlanding_safety_check selftest: {P + F} checks, {F} failed")
    return 1 if F else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

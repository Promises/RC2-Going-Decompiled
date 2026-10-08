#!/bin/sh
# objdiff_cli.sh — pick the objdiff-cli binary for THIS host. Sourced, not run:
#
#   . tools/ee/objdiff_cli.sh          (CWD must be the repo root)
#
# Sets OBJDIFF to a repo-relative (or override) path. Callers: unit_report.sh,
# diff.sh, diff96.sh, objdiff_demo.sh.
#
#   Darwin/arm64  -> tools/objdiff-cli-macos-arm64
#   Linux/x86_64  -> tools/objdiff-cli-linux-x86_64
#
# Both are the objdiff-cli 3.7.2 release assets and write byte-identical
# `report generate` JSON for the same objects (FACT #9904).
#
# OBJDIFF_CLI overrides detection: a value containing `/` is used as a path, a
# bare name is looked up in tools/ (e.g. OBJDIFF_CLI=objdiff-cli-linux-x86_64).
#
# The binaries are GITIGNORED (.gitignore `tools/objdiff-cli-*`), so a fresh
# worktree has none. A missing or non-executable binary, or an unknown host, is
# a hard exit 2 naming the path — never an empty OBJDIFF, which would run
# nothing and let a caller read "no output" as "no differences".
if [ -n "${OBJDIFF_CLI:-}" ]; then
  case "$OBJDIFF_CLI" in
    */*) OBJDIFF="$OBJDIFF_CLI";;
    *)   OBJDIFF="tools/$OBJDIFF_CLI";;
  esac
else
  case "$(uname -s)/$(uname -m)" in
    Darwin/arm64) OBJDIFF=tools/objdiff-cli-macos-arm64;;
    Linux/x86_64) OBJDIFF=tools/objdiff-cli-linux-x86_64;;
    *) echo "objdiff_cli: FATAL — no objdiff-cli known for host $(uname -s)/$(uname -m); set OBJDIFF_CLI" >&2; exit 2;;
  esac
fi
[ -x "$OBJDIFF" ] || { echo "objdiff_cli: FATAL — $OBJDIFF is missing or not executable (gitignored; fetch the objdiff-cli 3.7.2 release asset, or set OBJDIFF_CLI)" >&2; exit 2; }

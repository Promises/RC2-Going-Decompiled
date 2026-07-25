#!/usr/bin/env bash
# Selftest for symaddrs_lint.py. Every fixture is a KNOWN answer; a lint that
# cannot fail here has not been shown to discriminate.
#
# The fixture that matters most is CONTAMINATION: splat's symbol table is
# module-global, so parsing two files in one interpreter leaks state and yields a
# bogus "Duplicate symbol" failure. That bogus failure is indistinguishable from
# a real defect, so a lint that can be contaminated is worse than no lint - its
# green and its red are both untrustworthy. We assert here that the SAME two
# files pass when isolated and that the contaminated form is NOT what we ship.
set -u
D="$(cd "$(dirname "$0")" && pwd)"
LINT="$D/symaddrs_lint.py"
PY="${SYMLINT_PY:-~/Documents/projects/ps2-gc-re/.venv-decomp/bin/python}"
[ -x "$PY" ] || { echo "no splat python at $PY: could not look" >&2; exit 2; }
"$PY" -c 'import splat' 2>/dev/null || { echo "$PY cannot import splat: could not look" >&2; exit 2; }

T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
fail=0
want() { # want <label> <expected-rc> <files...>
  local label="$1" exp="$2"; shift 2
  "$PY" "$LINT" --python "$PY" "$@" >"$T/out" 2>&1; local rc=$?
  if [ "$rc" != "$exp" ]; then
    echo "FAIL $label: expected rc=$exp got rc=$rc"; sed 's/^/      /' "$T/out"; fail=1
  else
    echo "  ok  $label"
  fi
}

printf 'g_alpha = 0x00100000; // type:func CONFIRMED plain entry\n' > "$T/clean.txt"
want "a clean map parses" 0 "$T/clean.txt"

# splat parses every space-separated comment token containing ':' as attr:val.
printf 'g_beta = 0x00100010; // Note: prose with a bare colon token\n' > "$T/bareattr.txt"
want "'Note:' in a comment is REJECTED (empty attr value)" 1 "$T/bareattr.txt"

# assert line.count(";") == 1 counts semicolons inside the comment too.
printf 'g_gamma = 0x00100020; // one; two semicolons\n' > "$T/semi.txt"
want "a second ';' in a comment is REJECTED" 1 "$T/semi.txt"

# The look-alike NEGATIVE control: a REAL splat attribute also contains a colon
# and MUST NOT be flagged. Without this, "reject anything with a colon" passes
# every other case here and is completely wrong.
printf 'g_delta = 0x00100030; // type:func size:0x40\n' > "$T/realattr.txt"
want "a REAL attr (type:func size:0x40) is ACCEPTED" 0 "$T/realattr.txt"

# A REAL duplicate inside ONE file must be caught. Not hypothetical: decomper-m1
# pinned 12 names of which 4 were ALREADY pinned ~450 lines further down a file
# they had only read around; their static checker passed and splat's parser
# rejected it. That is the case this lint exists for - and it is also why the
# isolation fixture below matters: "Duplicate symbol" can be a REAL clash or a
# CONTAMINATION artifact, and only forking makes the message trustworthy.
printf 'AlphaName = 0x00300000; // type:func first pin\nBetaName = 0x00300000; // type:func same address\n' > "$T/dup.txt"
want "a REAL duplicate symbol in ONE file is REJECTED" 1 "$T/dup.txt"

# Bands: a missing file is could-not-look, never a verdict about content.
want "a missing file is could-not-look (2)" 2 "$T/does_not_exist.txt"

"$PY" "$LINT" >/dev/null 2>&1
[ $? -eq 3 ] && echo "  ok  no args is usage (3)" || { echo "FAIL no args"; fail=1; }

# ISOLATION. Two files that each parse alone must also pass TOGETHER. If the
# lint ever stops forking, splat's global table leaks and this goes red.
printf 'g_eps = 0x00100040; // type:func\n' > "$T/iso_a.txt"
printf 'g_zeta = 0x00100050; // type:func\n' > "$T/iso_b.txt"
want "two files in one INVOCATION still pass (isolation holds)" 0 "$T/iso_a.txt" "$T/iso_b.txt"

# ...and the contaminated form is demonstrably different: parsing both in ONE
# interpreter re-registers the first file's symbols and fails. This is the
# fixture that proves the isolation is load-bearing rather than incidental.
cat > "$T/contaminated.py" <<'PY'
import sys
from pathlib import Path
from splat.util import symbols
for p in sys.argv[1:]:
    symbols.handle_sym_addrs(Path(p), open(p).readlines(), [])
print("NO_CONTAMINATION_OBSERVED")
PY
out=$("$PY" "$T/contaminated.py" "$T/iso_a.txt" "$T/iso_a.txt" 2>&1)
if printf '%s' "$out" | /usr/bin/grep -qi 'duplicate'; then
  echo "  ok  contaminated single-process run DOES fail (isolation is load-bearing)"
elif printf '%s' "$out" | /usr/bin/grep -q 'NO_CONTAMINATION_OBSERVED'; then
  echo "NOTE: no contamination observed in this splat build — the isolation fixture is"
  echo "      INERT here. Isolation is still correct by construction, but this case is"
  echo "      no longer evidence for it. Do not read the suite's green as covering it."
else
  echo "  ok  contaminated single-process run fails ($(printf '%s' "$out" | tail -1 | cut -c1-50))"
fi

# ATTRIBUTION. Two DIFFERENT files with the SAME basename must produce two rows
# that can be told apart. Every region's map is literally `symbol_addrs.txt`, and
# the natural way to compare one across refs is to extract both as
# `usa_symbol_addrs.txt` - so a report keyed on the basename prints one FAIL and
# one ok with no way to say which ref each belonged to. The rc was right and the
# report was unusable; a verdict you cannot attribute is not a verdict.
mkdir -p "$T/refA" "$T/refB"
cp "$T/clean.txt" "$T/refA/symbol_addrs.txt"
cp "$T/semi.txt"  "$T/refB/symbol_addrs.txt"
"$PY" "$LINT" --python "$PY" "$T/refA/symbol_addrs.txt" "$T/refB/symbol_addrs.txt" >"$T/attr" 2>&1
if /usr/bin/grep -q 'refA/symbol_addrs.txt' "$T/attr" && /usr/bin/grep -q 'refB/symbol_addrs.txt' "$T/attr"; then
  echo "  ok  same-basename files are attributed by full path"
else
  echo "FAIL same-basename rows are indistinguishable"; sed 's/^/      /' "$T/attr"; fail=1
fi

[ $fail -eq 0 ] && echo "symaddrs_lint selftest: all checks passed" || echo "symaddrs_lint selftest: FAILURES"
exit $fail

#!/bin/sh
# FAIL THE BUILD on any `li.d` that tools/ee/lid_fixup.sed does not rewrite.
#
#   usage:  sh tools/ee/lid_guard.sh <fixup.sed> <file.s> [file.s ...]
#   exit 0  every li.d in every file is rewritten by the fixup (or there are none)
#   exit 1  at least one is not -- each is reported on stderr as file:line:constant
#
# WHY THIS EXISTS. lid_fixup.sed encodes exactly one constant (1.0), because
# that is the only expansion SN as.exe was measured emitting (#20515). A rule
# that quietly passes any other constant through would be a silent trap: the
# assembler would either reject it (loud, by luck) or -- worse -- a future
# widening of the rule could encode it WRONGLY and nothing would say so. li.d is
# cc1-synthesised with no source-side proxy (#20565), so the set of constants
# that will appear cannot be enumerated in advance. The uncovered case therefore
# has to announce itself.
#
# HOW IT CANNOT DRIFT FROM THE RULE. "Handled" is not re-stated here as a second
# regex; it is defined as "the fixup rewrote this line". Each candidate line is
# fed through the SAME sed file the build applies, one line at a time, and a
# surviving `li.d` is by definition unhandled. There is one source of truth.
# Testing line-by-line (rather than grepping the whole filtered file) is what
# lets the report carry the ORIGINAL source line number: the fixup turns one
# line into two, so line numbers in the filtered stream do not match the .s.
#
# SCOPE: the GNU-as CALT overlay path in build_conly.sh only. The normal
# (byte-matching) path assembles with SN as.exe, which expands li.d natively.
set -u
[ $# -ge 2 ] || { echo "usage: lid_guard.sh <fixup.sed> <file.s> [...]" >&2; exit 2; }
FIX="$1"; shift
[ -f "$FIX" ] || { echo "lid_guard: fixup not found: $FIX" >&2; exit 2; }

T="${TMPDIR:-/tmp}/lid_guard.$$"
rc=0
for S in "$@"; do
  grep -n 'li\.d' "$S" > "$T.hits" 2>/dev/null
  [ -s "$T.hits" ] || continue
  while IFS= read -r hit; do
    n=${hit%%:*}
    txt=${hit#*:}
    printf '%s\n' "$txt" > "$T.one"
    sed -E -f "$FIX" "$T.one" | grep -q 'li\.d' || continue
    const=$(printf '%s\n' "$txt" \
            | sed -E 's/.*li\.d[[:space:]]+[^,]*,[[:space:]]*//; s/[[:space:]]*\r?$//')
    echo "BUILD FAIL: li.d constant not handled by $FIX" >&2
    echo "  $S:$n: constant '$const'  in: $(printf '%s' "$txt" | tr -d '\r')" >&2
    rc=1
  done < "$T.hits"
done
rm -f "$T.hits" "$T.one"
[ "$rc" = 0 ] || echo "lid_guard: the CALT overlay cannot assemble an unmeasured li.d expansion; measure it (see #20515) and widen $FIX, or remove the construct." >&2
exit $rc

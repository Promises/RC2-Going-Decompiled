#!/usr/bin/env bash
# text_size_check.sh — whole-unit layout check a per-function gate cannot make
# (FACT #7982, task #699).
#
#   tools/ee/text_size_check.sh <region> <unit>
#   e.g. tools/ee/text_size_check.sh usa text/191238
#
# Run AFTER tools/ee/objdiff_build.sh <region> <unit>. Reads the sdk29 BASE
# object (build/<region>/obj/<unit>.o) and checks every function symbol's
# offset in .text against ROM TRUTH: its `glabel` vaddr in the unit's
# nonmatchings .s tree; (offset - vaddr) must be the same for every symbol. A promotion that
# drops words splat left after a function's `endlabel` makes every later
# function land low; the unit objdiff report, verify_match_unit.sh and
# per-function compares score functions in isolation and stay green.
#
# ⚠️ Why not base-vs-expected .text size: expected/<unit>.o is assembled from
# EVERY .s, so it carries BOTH a function's post-endlabel words AND any filler
# .s the C tree added to restore them (the INCLUDE_ASM_FRAGMENT idiom) — it is
# longer than the ROM wherever a filler exists. Measured at master f0aa4fd0:
# the size compare fires on 7 of 11 units whose image is byte-exact. It is
# printed below for reference only; it is not the verdict.
#
# Exit 0 = 0 symbols off their ROM offset; 1 = at least one (first 5 printed);
# 2 = could not look (missing object, 0 symbols joined). Only the sdk29 arm is
# checked: it is the arm build.sh links into the image (MATCH_-guarded
# functions keep their INCLUDE_ASM there).
set -u
region=${1:?region}; unit=${2:?unit}
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
base="going-decompiled/build/$region/obj/$unit.o"
exp="going-decompiled/build/$region/expected/$unit.o"
asm="$ROOT/going-decompiled/asm/$region/nonmatchings/$unit"
[ -f "$ROOT/$base" ] || { echo "text_size_check: MISSING $base"; exit 2; }
[ -d "$asm" ] || { echo "text_size_check: MISSING $asm"; exit 2; }
echo "text_size_check: base mtime $(stat -f %Sm "$ROOT/$base" 2>/dev/null || stat -c %y "$ROOT/$base")"
nmout=$("$ROOT/tools/ee/vm.sh" "
  for o in $base $exp; do
    [ -f \$o ] || continue
    mips-linux-gnu-objdump -h \$o | awk -v o=\$o '\$2==\".text\"{print \"SIZE\", o, \$3}'
  done
  mips-linux-gnu-nm $base | awk '\$2 ~ /^[Tt]\$/ && \$3 !~ /NON_MATCHING/ {print \"SYM\", \$3, \$1}'")
printf '%s\n' "$nmout" | python3 -c '
import re, sys, pathlib
asm, base, exp = sys.argv[1:4]
truth = {}
for p in pathlib.Path(asm).glob("*.s"):
    name = None
    for ln in p.read_text().splitlines():
        m = re.match(r"\s*glabel\s+(\w+)", ln)
        if m: name = m.group(1); continue
        m = re.search(r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s+[0-9A-Fa-f]{8}\s*\*/", ln)
        if m and name and name not in truth:
            truth[name] = int(m.group(1), 16)
if not truth:
    print("text_size_check: no glabel in", asm); sys.exit(2)
size, sym = {}, {}
for ln in sys.stdin:
    f = ln.split()
    if f and f[0] == "SIZE": size[f[1]] = f[2]
    elif f and f[0] == "SYM": sym[f[1]] = int(f[2], 16)
# Anchor on the lowest-offset symbol present in both: the unit may start
# before the folder lowest glabel or after it (a dead .s for a word linked
# elsewhere), and the linker places the unit as a whole. A dropped pad is a
# STEP in (offset - vaddr) after the promoted function, not a uniform shift.
both = sorted((off, n) for n, off in sym.items() if n in truth)
joined, moved = len(both), 0
if both:
    anchor = both[0][0] - truth[both[0][1]]
    for off, n in both:
        want = truth[n] + anchor
        if off != want:
            moved += 1
            if moved <= 5:
                print("  off %s base 0x%x rom 0x%x (%+d)" % (n, off, want, off - want))
print("text_size_check: reference only: .text base 0x%s expected 0x%s" % (size.get(base, "?"), size.get(exp, "?")))
print("text_size_check: vs ROM glabel offsets: joined %d, off %d" % (joined, moved))
if joined == 0:
    print("text_size_check: 0 symbols joined -- nothing compared"); sys.exit(2)
sys.exit(1 if moved else 0)
' "$asm" "$base" "$exp"

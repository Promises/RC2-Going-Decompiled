#!/bin/bash
# splache_selftest.sh <region> — prove scripts/configure.py's cache policy
# (task #449, FACT #7247/#7248) with the failing arm observed, in a scratch
# clone so the worktree is never touched.
#
#   1. --use-cache, no cache present  -> every segment row "0 cached" (full)
#   2. --use-cache again              -> at least one row "N cached", N>0
#                                        (the cache CAN hit: the positive control
#                                        without which step 3's zero means nothing)
#   3. one symbol_addrs line appended, --use-cache -> "cache DISCARDED
#      (changed: …symbol_addrs.txt)" and every row "0 cached"
#   4. default invocation (no flag) with the step-3 .splache still present
#                                     -> every row "0 cached" (the default does
#                                        not read the cache)
#   5. fingerprint-only: a top-level `subalign`, `vram` or `bss_size` edit in a
#      yaml copy changes cache_inputs() (no split — the decision is stored == inputs)
#
# Run from the repo root with a populated tree (extracted/ and
# going-decompiled/build/<region>/{include,assets} present). Exit 0 = every
# arm observed; 1 = an arm did not show the expected observable (printed).
# ~3 full splits of one region; budget several minutes.
set -u
R=${1:?region}
W=$(pwd)
PY="$W/.venv-decomp/bin/python"
[ -x "$PY" ] || { echo "no venv python at $PY" >&2; exit 2; }
SC=$(mktemp -d)
trap 'rm -rf "$SC"' EXIT
S="$SC/tree"
git clone -q --shared --no-checkout "$W" "$S"
git -C "$S" checkout -q --detach HEAD
ln -s "$W/extracted" "$S/extracted"
mkdir -p "$S/going-decompiled/build/$R"
cp -R "$W/going-decompiled/build/$R/include" "$W/going-decompiled/build/$R/assets" "$S/going-decompiled/build/$R/"
# the branch's configure.py, not HEAD's (so an uncommitted edit is what is tested).
# CONFIGURE_PY=<file> substitutes another copy — the selftest's own failure arm is
# observed with the pre-#449 script (`git show 8b536f0c:scripts/configure.py`),
# which must FAIL steps 3 and 4.
cp "${CONFIGURE_PY:-$W/scripts/configure.py}" "$S/scripts/configure.py"
SA="$S/going-decompiled/symbol_addrs/$R/symbol_addrs.txt"
CACHE="$S/going-decompiled/build/$R/.splache"

rc=0
split() {  # split <label> <args...>; prints the cached rows; stores log in $SC/<label>.log
  local label=$1; shift
  (cd "$S" && "$PY" scripts/configure.py --region "$R" "$@") > "$SC/$label.log" 2>&1 || { echo "FAIL $label: configure.py exited non-zero"; tail -5 "$SC/$label.log"; rc=1; }
  /usr/bin/grep -E 'split, [0-9]+ cached' "$SC/$label.log" | /usr/bin/sed -E 's/^/    /'
}
rows_cached_nonzero() { /usr/bin/grep -E 'split, [0-9]+ cached' "$1" | /usr/bin/grep -vE ' 0 cached' | wc -l | tr -d ' '; }
rows_total() { /usr/bin/grep -cE 'split, [0-9]+ cached' "$1" || true; }

echo "== 1. --use-cache with no cache present (expect every row 0 cached, cache written)"
split s1 --use-cache
n=$(rows_cached_nonzero "$SC/s1.log"); t=$(rows_total "$SC/s1.log")
if [ "$t" -gt 0 ] && [ "$n" = 0 ] && [ -f "$CACHE" ]; then echo "OK   $t rows, 0 with a cache hit, .splache written"; else echo "FAIL step 1: rows=$t nonzero-cached=$n cache=$([ -f "$CACHE" ] && echo present || echo absent)"; rc=1; fi
"$PY" - "$CACHE" <<'PY' || { echo "FAIL step 1: fingerprint not stored in the cache file"; rc=1; }
import pickle, sys
c = pickle.load(open(sys.argv[1], "rb"))
k = c.get("__configure_inputs__")
assert isinstance(k, dict) and any(x.endswith("symbol_addrs.txt") for x in k), "no __configure_inputs__"
print("OK   __configure_inputs__ present:", sorted(k)[:3], "...")
PY

echo "== 2. --use-cache again, nothing changed (expect cache kept and at least one row N cached, N>0)"
split s2 --use-cache
n=$(rows_cached_nonzero "$SC/s2.log")
if /usr/bin/grep -q 'cache inputs unchanged, cache kept' "$SC/s2.log" && [ "$n" -gt 0 ]; then echo "OK   cache kept, $n rows with a cache hit (positive control: the cache can hit)"; else echo "FAIL step 2: kept=$(/usr/bin/grep -c 'cache kept' "$SC/s2.log" || true) nonzero-cached=$n"; rc=1; fi

echo "== 3. one symbol_addrs line appended, --use-cache (expect cache DISCARDED and every row 0 cached)"
printf 'T449SplacheSeed = 0x00DEAD10; // type:func t449 splache selftest seed\n' >> "$SA"
split s3 --use-cache
n=$(rows_cached_nonzero "$SC/s3.log"); t=$(rows_total "$SC/s3.log")
if /usr/bin/grep -q 'cache DISCARDED (changed: .*symbol_addrs.txt' "$SC/s3.log" && [ "$t" -gt 0 ] && [ "$n" = 0 ]; then echo "OK   $(/usr/bin/grep -o 'cache DISCARDED ([^)]*)' "$SC/s3.log"); $t rows, 0 with a cache hit"; else echo "FAIL step 3: discarded=$(/usr/bin/grep -c 'cache DISCARDED' "$SC/s3.log" || true) rows=$t nonzero-cached=$n"; rc=1; fi

echo "== 4. default invocation with the .splache still present (expect no cache read: every row 0 cached)"
[ -f "$CACHE" ] || { echo "FAIL step 4 precondition: no .splache after step 3"; rc=1; }
split s4
n=$(rows_cached_nonzero "$SC/s4.log"); t=$(rows_total "$SC/s4.log")
if /usr/bin/grep -q 'full split, no cache' "$SC/s4.log" && [ "$t" -gt 0 ] && [ "$n" = 0 ]; then echo "OK   default split without the cache: $t rows, 0 with a cache hit"; else echo "FAIL step 4: rows=$t nonzero-cached=$n"; rc=1; fi

echo "== 5. fingerprint moves on a top-level subalign / vram / bss_size edit (no split)"
"$PY" - "$S" "$R" <<'PY' || rc=1
import sys, re, importlib.util
root, region = sys.argv[1], sys.argv[2]
spec = importlib.util.spec_from_file_location("configure", f"{root}/scripts/configure.py")
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
cfg = m.CONFIGS[region]
orig = cfg.read_text()
base = m.cache_inputs(region)
ok = True
for key, pat, rep in (("subalign", r"subalign: 8\b", "subalign: 16"),
                      ("vram", r"vram: 0x00100080", "vram: 0x00100090"),
                      ("bss_size", r"bss_size: 0x0\b", "bss_size: 0x10")):
    new, n = re.subn(pat, rep, orig, count=1)
    if n == 0:
        print(f"FAIL step 5: pattern for {key} not found in {cfg.name} (control cannot run)"); ok = False; continue
    cfg.write_text(new)
    moved = m.cache_inputs(region) != base
    cfg.write_text(orig)
    print(("OK   " if moved else "FAIL ") + f"{key} edit {'moves' if moved else 'does NOT move'} the fingerprint")
    ok &= moved
# and an edit to something the fingerprint must NOT track (a comment line) keeps it
cfg.write_text(orig + "\n# t449 comment-only edit\n")
same = m.cache_inputs(region) == base
cfg.write_text(orig)
print(("OK   " if same else "FAIL ") + "comment-only yaml edit keeps the fingerprint")
sys.exit(0 if ok and same else 1)
PY

echo "== result: $([ $rc = 0 ] && echo PASS || echo FAIL) (logs in $SC until exit)"
exit $rc

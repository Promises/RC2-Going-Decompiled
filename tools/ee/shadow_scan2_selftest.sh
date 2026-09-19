#!/bin/bash
# shadow_scan2_selftest.sh <region> — prove shadow_scan2.sh can FAIL before
# trusting its zero. Copies src/, symbol_addrs/ and the INCLUDE_ASM'd asm of
# <region> to a scratch root, records the baseline row counts per class, then
# plants one site per class and requires EXACTLY one new row per class, each
# naming the planted address:
#
#   CLASS1  INCLUDE_ASM("<asmdir>", func_00DEAD00);   appended to one .c
#   CLASS2  s32 func_00DEAD04(void) { return 0; }      appended to the same .c
#           (inside the compiled arm), plus a control definition parked under
#           #ifdef TARGET_NATIVE that must NOT produce a CLASS2 row
#   CLASS3  alabel func_00DEAD08                      appended to one included .s
#
# with fake symbol_addrs lines T448SeedC1/C2/C3 = 0x00DEAD00/04/08. Run from
# the repo root. Exit 0 = every control fired and nothing else moved; exit 1 =
# a control did not fire (the scan cannot see that class) or the baseline moved.
set -u
R=${1:?region}
HERE=$(cd "$(dirname "$0")" && pwd)
SC=$(mktemp -d)
trap 'rm -rf "$SC"' EXIT

mkdir -p "$SC/going-decompiled/asm/$R"
cp -R going-decompiled/src "$SC/going-decompiled/src"
cp -R going-decompiled/symbol_addrs "$SC/going-decompiled/symbol_addrs"
cp -R "going-decompiled/asm/$R/nonmatchings" "$SC/going-decompiled/asm/$R/nonmatchings"

counts() { awk '{print $1}' | sort | uniq -c | awk '{print $2"="$1}' | tr '\n' ' '; }
rows_for() { /usr/bin/grep -E "func_00DEAD0[0-9] -> T448Seed" "$1" | /usr/bin/grep -c "^$2 " || true; }

ROOT="$SC" bash "$HERE/shadow_scan2.sh" "$R" > "$SC/before.txt"
echo "baseline: $(counts < "$SC/before.txt")"

# pick a .c and an included .s to plant into (the first of each, deterministic)
C=$(/usr/bin/find "$SC/going-decompiled/src/$R" -name '*.c' | sort | head -1)
S=$(/usr/bin/grep -rhoE 'INCLUDE_ASM\("[^"]+",[[:space:]]*[A-Za-z_][A-Za-z0-9_]*' "$SC/going-decompiled/src/$R" \
     | /usr/bin/sed -E 's/INCLUDE_ASM\("([^"]+)",[[:space:]]*([A-Za-z0-9_]+)/\1\/\2.s/' | sort -u | head -1)
ASMDIR=$(dirname "$S")
cat >> "$C" <<EOF

/* t448 selftest seeds — never commit */
INCLUDE_ASM("$ASMDIR", func_00DEAD00);
s32 func_00DEAD04(void) { return 0; }
#ifdef TARGET_NATIVE
s32 func_00DEAD0C(void) { return 0; }
#endif
EOF
printf 'alabel func_00DEAD08\n' >> "$SC/$S"
cat >> "$SC/going-decompiled/symbol_addrs/$R/symbol_addrs.txt" <<EOF
T448SeedC1 = 0x00DEAD00; // type:func t448 selftest seed
T448SeedC2 = 0x00DEAD04; // type:func t448 selftest seed
T448SeedC3 = 0x00DEAD08; // type:func t448 selftest seed
T448SeedNotCompiled = 0x00DEAD0C; // type:func t448 selftest seed (TARGET_NATIVE arm, must NOT fire)
EOF

ROOT="$SC" bash "$HERE/shadow_scan2.sh" "$R" > "$SC/after.txt"
echo "seeded:   $(counts < "$SC/after.txt")"

rc=0
for cls in CLASS1 CLASS2 CLASS3; do
  n=$(rows_for "$SC/after.txt" "$cls")
  if [ "$n" = "1" ]; then
    echo "OK   $cls control fired: $(/usr/bin/grep -E "^$cls .*T448Seed" "$SC/after.txt")"
  else
    echo "FAIL $cls control: expected 1 seeded row, got $n"; rc=1
  fi
done
n=$(/usr/bin/grep -c 'T448SeedNotCompiled' "$SC/after.txt" || true)
if [ "$n" = "0" ]; then echo "OK   TARGET_NATIVE-arm definition did not fire"; else echo "FAIL not-compiled control fired ($n rows)"; rc=1; fi
# nothing else may move: after minus the seeded rows == before
if diff <(/usr/bin/grep -v 'T448Seed' "$SC/after.txt") "$SC/before.txt" > /dev/null; then
  echo "OK   no other row changed"
else
  echo "FAIL rows other than the seeds changed:"; diff <(/usr/bin/grep -v 'T448Seed' "$SC/after.txt") "$SC/before.txt"; rc=1
fi
exit $rc

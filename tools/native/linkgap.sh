#!/usr/bin/env bash
# linkgap.sh — M0 native link-gap inventory (READ-ONLY).
#
# The TARGET_NATIVE build syntax-compiles (see check.sh) but does not yet LINK a
# runnable binary: it references symbols that nothing in the native object set
# defines. This tool quantifies and categorizes exactly that gap — it is the
# work-list for the HLE backend (see docs/HLE.md).
#
# What it does:
#   1. compiles every TARGET_NATIVE unit ILP32 `-c` (same flags as check.sh),
#   2. computes GAP = symbols referenced by some unit but defined by none,
#   3. buckets each gap symbol into one of:
#        SDK/libc      - provided by a host shim (memset/memcpy/...; runtime/sdk)
#        runtime-global- a data global (D_*/g_*); needs the BSS/data arena
#        tier3-hle     - a hardware function (manifest class 'hle'); needs ps2hw
#        decomp-asm    - a still-INCLUDE_ASM game function (no #else body yet)
#
# It writes NOTHING into the tree and mutates no build. Output goes to stdout;
# pass a path as $1 to also tee the full categorized listing there.
#
# Usage: tools/native/linkgap.sh [report-out.txt]
set -u

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
INC="$ROOT/going-decompiled/include"
SHIM="$ROOT/tools/native/mips_callees.h"
SYMS="$ROOT/going-decompiled/symbol_addrs/usa/symbol_addrs.txt"
MANIFEST="$ROOT/tools/ee/eetest/cmp/manifest.txt"
ASMDIR="$ROOT/going-decompiled/asm/usa"
CC="${CC:-clang}"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

CFLAGS="-DTARGET_NATIVE -m32 -c -I$INC -include $SHIM"
CFLAGS="$CFLAGS -Wno-error=implicit-function-declaration -Wno-int-conversion"

# 1. compile all TARGET_NATIVE units to objects (best-effort; a unit that fails
#    check.sh just contributes no symbols — linkgap is not the compile gate).
units="$(cd "$ROOT" && /usr/bin/grep -rl TARGET_NATIVE going-decompiled/src | sed "s#^#$ROOT/#")"
nunits=0
for f in $units; do
  base="$(basename "$f" .c)"
  if $CC $CFLAGS "$f" -o "$OUT/$base.o" 2>/dev/null; then
    nunits=$((nunits+1))
  fi
done

# 2. defined / undefined symbol sets across the whole object set.
#    Mach-O nm prefixes a leading '_' on C symbols; strip it. Undefined rows
#    have type 'U'; everything else is a definition.
# nm rows: defined = "<addr> <type> <name>" (NF==3); undefined = "U <name>"
# (NF==2, leading whitespace collapsed so $1=="U").
nm "$OUT"/*.o 2>/dev/null | awk 'NF==3 {print $3}' | sed 's/^_//' | sort -u > "$OUT/defined.txt"
nm "$OUT"/*.o 2>/dev/null | awk 'NF==2 && $1=="U" {print $2}' | sed 's/^_//' | sort -u > "$OUT/undef.txt"
# GAP = referenced-undefined that is defined by no native unit.
comm -23 "$OUT/undef.txt" "$OUT/defined.txt" > "$OUT/gap.txt"

# 3. classifier inputs.
#    funcs  = authoritative game-function name set (every asm glabel).
#    hle    = manifest tier-3 hardware functions.
#    libc   = symbol_addrs libc.a entries.
/usr/bin/grep -rh '^glabel ' "$ASMDIR" 2>/dev/null | awk '{print $2}' | sort -u > "$OUT/funcs.txt"
awk '$3=="hle"{print $2}' "$MANIFEST" 2>/dev/null | sort -u > "$OUT/hle.txt"
/usr/bin/grep 'libc.a' "$SYMS" 2>/dev/null | awk '{print $1}' | sort -u > "$OUT/libc.txt"

is_in() { grep -qxF "$1" "$2" 2>/dev/null; }

: > "$OUT/cat_libc.txt"; : > "$OUT/cat_global.txt"
: > "$OUT/cat_hle.txt";  : > "$OUT/cat_asm.txt"

# Classification is convention-driven (this codebase is disciplined about names —
# see CLAUDE.md): functions are VerbNoun CamelCase or func_XXXX; data globals are
# g_*/D_*. The asm glabel set is authoritative for un-renamed funcs but stale for
# functions renamed in symbol_addrs without a re-split, so the CamelCase rule
# catches those. libc/libm/compiler-rt are matched by an explicit name set.
while IFS= read -r s; do
  [ -z "$s" ] && continue
  # 1. libc / libm / compiler soft-float — host-provided, near-free.
  case "$s" in
    memset|memcpy|memcmp|memmove|memchr|strlen|strcpy|strncpy|strcmp|strncmp|\
    strcat|strncat|strchr|strrchr|strstr|sprintf|snprintf|vsprintf|printf|\
    puts|malloc|calloc|realloc|free|qsort|bsearch|rand|srand|abort|exit|\
    sin|cos|tan|asin|acos|atan|atan2|sqrt|fabs|pow|exp|log|log10|floor|ceil|\
    fmod|ldexp|frexp|modf|sinf|cosf|tanf|asinf|acosf|atanf|atan2f|sqrtf|fabsf|\
    powf|expf|logf|floorf|ceilf|fmodf|ldexpf|frexpf|modff|\
    __*|_*chk|*_chk)
      echo "$s" >> "$OUT/cat_libc.txt"; continue ;;
  esac
  if is_in "$s" "$OUT/libc.txt"; then
    echo "$s" >> "$OUT/cat_libc.txt"
  # 2. tier-3 hardware — manifest is authoritative regardless of name shape.
  elif is_in "$s" "$OUT/hle.txt"; then
    echo "$s" >> "$OUT/cat_hle.txt"
  # 3. game functions: func_XXXX (any case), an asm glabel, or VerbNoun CamelCase.
  elif printf '%s' "$s" | /usr/bin/grep -qiE '^(func_|fun_)[0-9a-f]+$' \
       || is_in "$s" "$OUT/funcs.txt" \
       || printf '%s' "$s" | /usr/bin/grep -qE '^[A-Z][A-Za-z0-9]*[a-z][A-Za-z0-9]*$'; then
    echo "$s" >> "$OUT/cat_asm.txt"
  # 4. everything else (g_*/D_*/sdata) is a data global needing the arena.
  else
    echo "$s" >> "$OUT/cat_global.txt"
  fi
done < "$OUT/gap.txt"

c_libc=$(wc -l < "$OUT/cat_libc.txt"   | tr -d ' ')
c_glob=$(wc -l < "$OUT/cat_global.txt" | tr -d ' ')
c_hle=$(wc -l  < "$OUT/cat_hle.txt"    | tr -d ' ')
c_asm=$(wc -l  < "$OUT/cat_asm.txt"    | tr -d ' ')
c_gap=$(wc -l  < "$OUT/gap.txt"        | tr -d ' ')

emit() {
  echo "=== native link-gap inventory (M0) ==="
  echo "units compiled : $nunits"
  echo "link gaps total: $c_gap  (symbols referenced but defined by no native unit)"
  echo
  printf "  %-14s %4s   %s\n" "SDK/libc"       "$c_libc" "-> runtime/sdk shims"
  printf "  %-14s %4s   %s\n" "runtime-global" "$c_glob" "-> BSS/data arena (from symbol_addrs)"
  printf "  %-14s %4s   %s\n" "tier3-hle"      "$c_hle"  "-> ps2hw backend (manifest class hle)"
  printf "  %-14s %4s   %s\n" "decomp-asm"     "$c_asm"  "-> still INCLUDE_ASM (deep decomp)"
  echo
  for grp in libc global hle asm; do
    f="$OUT/cat_$grp.txt"
    [ -s "$f" ] || continue
    echo "--- $grp ---"
    sort "$f" | sed 's/^/  /'
    echo
  done
}

if [ "$#" -ge 1 ]; then
  emit | tee "$1"
else
  emit
fi

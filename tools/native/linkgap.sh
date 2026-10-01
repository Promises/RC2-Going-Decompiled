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
# pass a path to also tee the full categorized listing there.
#
# Scope is USA by default: EU work is queued behind a byte-exact USA (RULING
# #5339), so EU units do not belong in the default inventory. --with-eu adds
# the src/eu units to the object set. The classifier inputs below (asm glabels,
# symbol_addrs) stay USA's either way, so an EU-only name is bucketed by its
# shape, not by EU's own tables.
#
# Usage: tools/native/linkgap.sh [--with-eu] [report-out.txt]
set -u

with_eu=0
report=""
for a in "$@"; do
  case "$a" in
    --with-eu) with_eu=1 ;;
    -*) echo "linkgap: unknown option $a" >&2; exit 2 ;;
    *) report="$a" ;;
  esac
done

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
INC="$ROOT/going-decompiled/include"
SHIM="$ROOT/tools/native/mips_callees.h"
SYMS="$ROOT/going-decompiled/symbol_addrs/usa/symbol_addrs.txt"
MANIFEST="$ROOT/tools/ee/eetest/cmp/manifest.txt"
ASMDIR="$ROOT/going-decompiled/asm/usa"
# Same default as check.sh: the units are ELF-targeted C, and on a Mac plain
# `clang` targets Mach-O, which rejects their ELF `section` attributes and
# underscore-mangles symbols, so the object set measures the HOST. Default to
# an i386 ELF target there, borrowing the SDK's libc headers. An explicit CC
# still wins.
if [ -z "${CC:-}" ] && [ "$(uname -s)" = "Darwin" ]; then
  CC="clang --target=i386-pc-linux-gnu -isystem $(xcrun --show-sdk-path)/usr/include"
fi
CC="${CC:-clang}"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

CFLAGS="-DTARGET_NATIVE -m32 -c -I$INC -include $SHIM"
CFLAGS="$CFLAGS -Wno-error=implicit-function-declaration -Wno-int-conversion"
# A .cpp unit (task #1258) compiles as check.sh compiles it: C++ from a wrapper
# `extern "C" { #include shim; #include unit }`. Handed to $CC directly, clang
# picks C++ by the extension WITHOUT the wrap, so every symbol the unit defines
# or references is C++-mangled (_Z13func_00290EA0v): its definitions satisfy
# nothing and its references land in the gap as data globals (task #1285).
CXXFLAGS="-x c++ -DTARGET_NATIVE -m32 -c -I$INC"

# 1. compile all TARGET_NATIVE units to objects (best-effort; a unit that fails
#    check.sh just contributes no symbols — linkgap is not the compile gate).
#    Name each object by its path under src/, not its basename: both regions
#    carry units of the same name (usa/cod/015180.c and eu/cod/015180.c), and a
#    basename object lets the second compile silently replace the first, so its
#    symbols vanish from every set below while the unit count still includes it.
regions="usa"
[ "$with_eu" = 1 ] && regions="usa eu"
srcdirs=""
for r in $regions; do srcdirs="$srcdirs going-decompiled/src/$r"; done
units="$(cd "$ROOT" && /usr/bin/grep -rl TARGET_NATIVE $srcdirs | sed "s#^#$ROOT/#")"
nunits=0; failed=""
for f in $units; do
  rel="${f#"$ROOT/going-decompiled/src/"}"
  case "$rel" in *.cpp) stem="${rel%.cpp}" ;; *) stem="${rel%.c}" ;; esac
  obj="$OUT/$(printf '%s' "$stem" | sed 's#/#__#g').o"
  if [ -e "$obj" ]; then
    echo "linkgap: object name $obj is not unique ($f)" >&2; exit 2
  fi
  case "$f" in
    *.cpp)
      wrap="${obj%.o}.wrap.cpp"
      printf 'extern "C" {\n#include "%s"\n#include "%s"\n}\n' "$SHIM" "$f" > "$wrap"
      cmd="$CC $CXXFLAGS $wrap" ;;
    *) cmd="$CC $CFLAGS $f" ;;
  esac
  if $cmd -o "$obj" 2>/dev/null; then
    nunits=$((nunits+1))
  else
    failed="$failed $rel"
  fi
done
# Still best-effort, but never silent (task #1285): a unit that contributes no
# symbols is named, so a dropped unit cannot pass for a smaller gap.
[ -z "$failed" ] || echo "linkgap: WARN: unit(s) failed to compile, contributing no symbols:$failed" >&2

# 2. defined / undefined symbol sets across the whole object set.
#    Mach-O prefixes a '_' on every C symbol, so strip exactly one there. ELF
#    adds none, so a leading '_' is part of the real name (__builtin_next_arg,
#    _GLOBAL_OFFSET_TABLE_) and must survive. Decide from the objects' magic,
#    not from $CC: an explicit CC can target either format.
prefix=""
fmt=""
for o in "$OUT"/*.o; do
  [ -e "$o" ] || continue
  case "$(od -An -tx1 -N4 "$o" | tr -d ' \n')" in
    7f454c46)                            f=elf ;;
    cefaedfe|cffaedfe|feedface|feedfacf) f=macho ;;
    *) echo "linkgap: $o is neither ELF nor Mach-O" >&2; exit 2 ;;
  esac
  if [ -n "$fmt" ] && [ "$f" != "$fmt" ]; then
    echo "linkgap: object set mixes $fmt and $f" >&2; exit 2
  fi
  fmt="$f"
done
[ "$fmt" = macho ] && prefix="_"
# Undefined rows have type 'U'; everything else is a definition.
# nm rows: defined = "<addr> <type> <name>" (NF==3); undefined = "U <name>"
# (NF==2, leading whitespace collapsed so $1=="U").
nm "$OUT"/*.o 2>/dev/null | awk 'NF==3 {print $3}' | sed "s/^$prefix//" | sort -u > "$OUT/defined.txt"
nm "$OUT"/*.o 2>/dev/null | awk 'NF==2 && $1=="U" {print $2}' | sed "s/^$prefix//" | sort -u > "$OUT/undef.txt"
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
  echo "regions        : $regions"
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

if [ -n "$report" ]; then
  emit | tee "$report"
else
  emit
fi

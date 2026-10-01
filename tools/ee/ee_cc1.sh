#!/bin/sh
# ee_cc1.sh — the ONE place a src/<region> unit is preprocessed and compiled to
# EE assembly, for a `.c` unit (C front end, cc1) or a `.cpp` unit (C++ front
# end, cc1plus). build.sh and objdiff_build.sh both call it; neither carries its
# own cpp/cc1 lines for the unit under test any more (task #1258).
#
#   sh tools/ee/ee_cc1.sh <arm> <src.c|src.cpp> <out.i> <out.s> "<cpp args>" "<cc1 args>"
#   sh tools/ee/ee_cc1.sh --resolve <unit path without extension>
#
# Runs INSIDE the ee-build container with CWD at the repo root (/work), except
# --resolve, which only stats files and runs anywhere.
#
# <arm>      sdk29    2.9-ee-991111b/r4: cc1.exe / cc1plus.exe under wibo (the
#                     image arm, and objdiff_build.sh's sdk29 base)
#            engine96 2.96-ee-001003-1: cc1 / cc1plus through the bundled loader
#                     ($CC296, default tools/ee/cc-296; objdiff_build.sh's
#                     engine96 arm). The preprocessor is the 2.9 cpp on both
#                     arms, as diff96.sh and objdiff_build.sh always did.
#            s136     SN 2.95.3 BUILD 1.36: cc1.exe / cc1plus.exe under wibo
#                     (tools/ee/s136os_splice.sh's per-function arm, FACT #8810;
#                     the caller adds -fopt-stack). 2.9 cpp, as the splice
#                     always used. Its cc1plus is the sibling of the held cc1
#                     (same ProDG 3.01 tree, cc1 sha-identical; task #1284).
# <cpp args> and <cc1 args> are word-split on purpose (no flag holds a space).
#
# THE C++ ARM (FACT #8809: cc1plus 2.9-ee-991111b/r4 emits the same code as cc1
# for this tree's C, 423/448 functions in 8 units, every difference a `::`
# asm-operand spelling):
#   - cpp runs with what the ee-gcc driver adds for C++ — `-lang-c++
#     -D__GNUG__=2 -D__cplusplus` (measured from `ee-gcc -v -x c++`; the driver
#     KEEPS -D__LANGUAGE_C/_LANGUAGE_C for C++, so the caller's set is unchanged).
#     The driver's -D__EXCEPTIONS is not added: we compile -fno-exceptions.
#   - the preprocessed text is wrapped in ONE `extern "C" { ... }` BY THIS
#     SCRIPT, so the source stays unmangled without carrying the wrapper itself.
#   - cc1plus gets -fno-exceptions -fno-rtti.
#   - the intermediate is written to the SAME <out.i> path a C unit would use.
#     The `.file` directive names the compiler's input file, so a .c and a .cpp
#     build of one unit emit the identical `.file` line; the only symbol that
#     differs is the front end's marker label (__gnu_compiled_c vs
#     __gnu_compiled_cplusplus), which this script checks for: cc1plus that did
#     not run as C++ is a failure, not a quieter C build.
#   - cc1plus reports some C-isms as errors and still writes a complete .s with
#     the offending statement dropped (FACT #6760, PROCEDURE #6258's diagmap.py),
#     so ONLY its exit status says whether the arm compiled: any non-zero exit
#     fails here and the .s is deleted.
#
# THE C ARM is byte-for-byte the cpp/cc1 command lines build.sh and
# objdiff_build.sh ran before this file existed.
#
# A unit is ONE source: <unit>.c and <unit>.cpp side by side is refused (both
# modes), because both would compile to the same object path and which one won
# would depend on enumeration order.
#
# Exit 0 = <out.s> written; 1 = refused or a compile step failed (the step is
# named on stderr, <out.s> removed); 2 = usage.
set -e

fail() { echo "ee_cc1.sh: FAIL: $*" >&2; exit 1; }

# sibling_check SRC — refuse a unit that exists as both .c and .cpp.
sibling_check() {
  case "$1" in
    *.cpp) sib="${1%.cpp}.c" ;;
    *.c)   sib="${1%.c}.cpp" ;;
    *)     fail "not a .c or .cpp unit: $1" ;;
  esac
  [ ! -e "$sib" ] || fail "unit exists as both $1 and $sib — one source per unit"
}

if [ "${1:-}" = "--resolve" ]; then
  [ $# -eq 2 ] || { echo "usage: ee_cc1.sh --resolve <unit path without extension>" >&2; exit 2; }
  if [ -f "$2.cpp" ]; then sibling_check "$2.cpp"; echo "$2.cpp"
  elif [ -f "$2.c" ]; then echo "$2.c"
  else fail "no $2.c or $2.cpp"
  fi
  exit 0
fi

[ $# -eq 6 ] || { echo "usage: ee_cc1.sh <sdk29|engine96|s136> <src.c|src.cpp> <out.i> <out.s> \"<cpp args>\" \"<cc1 args>\"" >&2; exit 2; }
ARM="$1"; SRC="$2"; OUT_I="$3"; OUT_S="$4"; CPPARGS="$5"; CC1ARGS="$6"

WIBO=/usr/local/bin/wibo
G=tools/ee/cc/lib/gcc-lib/ee/2.9-ee-991111
CC296="${CC296:-tools/ee/cc-296}"
B96="$CC296/lib/gcc-lib/ee/2.96-ee-001003-1"
G136=tools/ee/cc/lib/gcc-lib/ee/2.95.3

[ -f "$SRC" ] || fail "no source $SRC"
sibling_check "$SRC"
case "$SRC" in *.cpp) SRCLANG=c++ ;; *) SRCLANG=c ;; esac

case "$ARM/$SRCLANG" in
  sdk29/c)      CC="$WIBO $G/cc1.exe" ;;
  sdk29/c++)    CC="$WIBO $G/cc1plus.exe" ;;
  engine96/c)   CC="$CC296/ld-2.3.6.so --library-path $CC296 $B96/cc1" ;;
  engine96/c++) CC="$CC296/ld-2.3.6.so --library-path $CC296 $B96/cc1plus" ;;
  s136/c)       CC="$WIBO $G136/cc1.exe" ;;
  s136/c++)     CC="$WIBO $G136/cc1plus.exe" ;;
  *) echo "ee_cc1.sh: unknown arm '$ARM' (sdk29|engine96|s136)" >&2; exit 2 ;;
esac
# the compiler binary is the last word of $CC
for w in $CC; do bin="$w"; done
[ -f "$bin" ] || fail "$ARM $SRCLANG compiler missing: $bin (run scripts/fetch_ee_toolchain.sh)"

rm -f "$OUT_I" "$OUT_S"
if [ "$SRCLANG" = c ]; then
  # shellcheck disable=SC2086
  $WIBO $G/cpp.exe $CPPARGS "$SRC" "$OUT_I" || fail "cpp: $SRC"
  # shellcheck disable=SC2086
  $CC -quiet $CC1ARGS "$OUT_I" -o "$OUT_S" || { rm -f "$OUT_S"; fail "cc1 ($ARM): $SRC"; }
else
  raw="$OUT_I.cxx"
  rm -f "$raw"
  # shellcheck disable=SC2086
  $WIBO $G/cpp.exe -lang-c++ -D__GNUG__=2 -D__cplusplus $CPPARGS "$SRC" "$raw" || { rm -f "$raw"; fail "cpp -lang-c++: $SRC"; }
  { printf 'extern "C" {\n'; cat "$raw"; printf '}\n'; } > "$OUT_I"
  rm -f "$raw"
  # shellcheck disable=SC2086
  $CC -quiet $CC1ARGS -fno-exceptions -fno-rtti "$OUT_I" -o "$OUT_S" || { rm -f "$OUT_S"; fail "cc1plus ($ARM): $SRC"; }
  grep -q '^__gnu_compiled_cplusplus:' "$OUT_S" || { rm -f "$OUT_S"; fail "cc1plus ($ARM) output lacks __gnu_compiled_cplusplus — the C++ front end did not compile $SRC"; }
fi
[ -s "$OUT_S" ] || fail "no assembly written for $SRC"

#!/usr/bin/env bash
# check_hle_stubs.sh — does the declared HLE stub set in runtime/sdk/iop_null.c
# match what the bodies above it actually reach WHEN EXECUTED? (task #1556)
#
# WHY THIS EXISTS. runtime/link_native.sh links with
# -Wl,--unresolved-symbols=ignore-all, so it CANNOT see a missing or a
# superfluous HLE stub. Task #1535 dropped each of iop_null.c's three stubs in
# turn and link_native still returned rc 0, 29/29. A "link-minimality" check
# made with link_native cannot fail. This script answers the question by
# EXECUTION, the way #1535's uncommitted driver did:
#
#   - It links the USA TARGET_NATIVE units, the arena, the runtime backends and
#     hle_stub_driver.c into one executable. The link uses gcc -m32
#     --gc-sections -rdynamic, with weak native_stub_hit trap stubs generated for
#     every symbol a probe link leaves undefined. There is NO
#     --allow-multiple-definition: a duplicate definition fails the link.
#   - Each declared stub is linked behind -Wl,--wrap so the driver records which
#     bodies reach it.
#   - The driver runs RULING #9179's eight bodies in forked alarm(3) children
#     (see hle_stub_driver.c for the argument vectors), on a ZERO arena and on
#     the SEEDED arena.
#
# VERDICTS (check_hle_stubs_verdict.py prints them; each names its arena):
#   MISSING      FAIL. A body trapped in a native_stub_hit stub that is not a
#                declared HLE stub and is not a known trap listed in
#                hle_stub_traps.txt. The line names the body and the symbol.
#   KNOWN        A trap listed in hle_stub_traps.txt: measured, with its cause
#                beside it, and not yet covered by a stub. Listing a trap
#                records it. It does not rule that it should stay a trap.
#   STALE        FAIL. An hle_stub_traps.txt row whose arena ran and which
#                did not fire. Lower the baseline (RULING #7317).
#   NEEDED       A declared stub reached by at least one body, on the named
#                arena(s).
#   SUPERFLUOUS  FAIL. A declared stub that no body reached on EITHER arena.
#                It is only printed when BOTH arenas ran, the seeded one from
#                the canonical seed files.
#   UNDETERMINED A declared stub not reached on the zero arena while the
#                canonical seeded arena did not run. The script cannot rule it
#                superfluous: InvalidDCache (func_0011B500) is reached ONLY on
#                the seeded arena (#1535), so a zero-arena-only run would
#                otherwise call it superfluous because its input was missing.
#   SKIPPED      The seeded arena did not run. The line names the missing path.
#   Bodies that CRASH, TIME OUT or trap are listed with their reach so far.
#   Their reach after that point is UNOBSERVED.
# Exit: 0 PASS (both arenas ran, nothing failed) · 1 FAIL · 2 INCOMPLETE (no
# failure, but a verdict above is unreachable, e.g. the seeded arena is absent)
# or the run itself could not be completed. 2 is not a pass.
#
# THE SEEDED ARENA. The canonical seed is tools/ee/eetest/state/globals.bin
# (0x1A7000+0x15000) and input.bin (0x138300+0x400), the windows of
# gen_batch.py's fallback. They are GITIGNORED (copyrighted RAM), so a fresh
# tree has none: the run prints SKIPPED and exits 2, never 0. Seed files whose
# sha1 differs from the canonical pair are still run but labelled
# "seeded-noncanonical", and that arena does not discharge UNDETERMINED.
#
# SEEDS (so every FAIL verdict can be made to fire on purpose):
#   --seed-drop=<stub>   compile iop_null.c with that definition renamed away
#                        (-D<stub>=hle_seed_dropped_<stub>, checked with nm). Dropping func_0011B3D0
#                        (SyncDCache) must FAIL naming func_00132AC8 and
#                        func_00133250.
#   --seed-add=<sym>     link an extra no-op strong definition of <sym> and
#                        count it as declared. A symbol no body reaches gives
#                        SUPERFLUOUS when both arenas ran, and UNDETERMINED
#                        otherwise.
#   --seed-dir=<dir>     read globals.bin / input.bin from <dir>
#                        (default tools/ee/eetest/state).
#   --baseline=<file>    read the known traps from <file> instead of
#                        hle_stub_traps.txt (/dev/null makes every trap MISSING;
#                        an extra row that never fires makes it STALE).
#   --selftest           exercise every verdict of the verdict script on
#                        recorded rows. No docker run.
#
# RELATION TO stub_hits.txt. tools/native/stub_hits.txt (and its successor
# tools/ee/eetest/state/stub_hits.txt) rank the trap stubs that the void(void)
# batch, run_state_batch.sh, reaches across the whole corpus. That population is
# different: every void(void) body, on the seeded front-end arena. This script
# checks one declared set (iop_null.c) against the eight bodies that set was
# derived from, in both directions. Neither file is regenerated here.
#
# Out of scope: the stubs in hw/backend_null.c and sdk/snd_null.c cover the
# whole corpus, not these eight bodies. Not being reached by the eight would
# not make them superfluous, so they are linked but not judged.
#
# Usage: tools/native/check_hle_stubs.sh [--seed-drop=S]... [--seed-add=S]...
#                         [--seed-dir=D] [--baseline=F] [--selftest]
set -eu

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"
IMG="${NATIVE_IMG:-native-build}"
CTX="${DOCKER_CONTEXT:-colima-ee-x86}"
IOP=tools/native/runtime/sdk/iop_null.c
BASELINE=tools/native/hle_stub_traps.txt
VERDICT=tools/native/check_hle_stubs_verdict.py
SEED_DIR=tools/ee/eetest/state
# canonical seed (FACT #9238): name, ROM address, length, sha1 prefix
SEEDS="globals.bin 0x1A7000 0x15000 e5d5ea0f
input.bin 0x138300 0x400 d926603e"

drops=""; adds=""
for a in "$@"; do
  case "$a" in
    --seed-drop=*) drops="$drops ${a#--seed-drop=}" ;;
    --seed-add=*)  adds="$adds ${a#--seed-add=}" ;;
    --seed-dir=*)  SEED_DIR="${a#--seed-dir=}" ;;
    --baseline=*)  BASELINE="${a#--baseline=}" ;;
    --selftest)    exec python3 "$VERDICT" --selftest ;;
    *) echo "check_hle_stubs: unknown argument $a" >&2; exit 2 ;;
  esac
done

# The declared set: every top-level definition in iop_null.c. A zero is not a
# result, so finding none is an error, not an empty pass.
declared=$(sed -nE 's/^[a-z0-9_]+[ *]+([A-Za-z_][A-Za-z0-9_]*)\(([^)]*)\)[ ]*\{.*/\1 \2/p' "$IOP")
[ -n "$declared" ] || { echo "check_hle_stubs: no definitions parsed from $IOP" >&2; exit 2; }
# The --wrap shim forwards no arguments, which is exact only for (void) stubs.
bad=$(printf '%s\n' "$declared" | awk '$2 != "void" {print $1}')
[ -z "$bad" ] || { echo "check_hle_stubs: $IOP defines non-(void) stub(s) the wrapper cannot forward:" $bad >&2; exit 2; }
declared=$(printf '%s\n' "$declared" | awk '{print $1}' | LC_ALL=C sort -u | tr '\n' ' ')
for d in $drops; do
  case " $declared " in *" $d "*) ;; *) echo "check_hle_stubs: --seed-drop=$d is not declared in $IOP (declared: $declared)" >&2; exit 2 ;; esac
done
for d in $adds; do
  case " $declared " in *" $d "*) echo "check_hle_stubs: --seed-add=$d is already declared" >&2; exit 2 ;; esac
done
echo "declared HLE stubs ($IOP): $declared"
[ -z "$drops" ] || echo "SEED: dropped from iop_null.o:$drops"
[ -z "$adds" ]  || echo "SEED: extra no-op stub(s) counted as declared:$adds"

# Scratch inside the tree: docker mounts only $ROOT. Removed on exit.
TMP=$(mktemp -d "$ROOT/tools/native/.hle_tmp.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
T=/work/${TMP#"$ROOT"/}

# sha1 prefix; sha1sum on Linux, shasum on macOS (the M1 has no sha1sum).
sha1_8() { if command -v sha1sum >/dev/null 2>&1; then sha1sum "$1"; else shasum -a 1 "$1"; fi | cut -c1-8; }

# Seeded arena: all files present -> run; sha1 decides canonical or not.
seeded="absent"; seed_args=""; missing=""; bad_sha=""
while read -r f rom len sha; do
  if [ -f "$SEED_DIR/$f" ]; then
    cp "$SEED_DIR/$f" "$TMP/$f"
    got=$(sha1_8 "$TMP/$f")
    seed_args="$seed_args $T/$f $rom $len"
    [ "$got" = "$sha" ] || bad_sha="$bad_sha,$f=$got"
  else
    missing="$missing $SEED_DIR/$f"
  fi
done <<EOF
$SEEDS
EOF
if [ -n "$missing" ]; then seeded="absent"
elif [ -n "$bad_sha" ]; then seeded="noncanonical:${bad_sha#,}"
else seeded="canonical"; fi
echo "seeded arena: $seeded${missing:+ (missing:$missing)}"

hdr=$(sed -nE 's/^# base 0x([0-9A-Fa-f]+)[[:space:]]+span 0x([0-9A-Fa-f]+).*/0x\1 0x\2/p' tools/native/runtime/arena/arena_map.txt)
[ -n "$hdr" ] || { echo "check_hle_stubs: no '# base .. span ..' header in arena_map.txt" >&2; exit 2; }
set -- $hdr; ABASE=$1; ASPAN=$2

# --wrap shims: __wrap_<s> records the reach, then runs the real stub (or, for a
# dropped one, the generated trap stub that now defines <s>).
{
  echo "extern void hle_note_reach(const char *);"
  for s in $declared $adds; do
    echo "long __real_$s(void);"
    echo "long __wrap_$s(void) { hle_note_reach(\"$s\"); return __real_$s(); }"
  done
} > "$TMP/wrap.c"
: > "$TMP/add.c"
for s in $adds; do echo "void $s(void) {}" >> "$TMP/add.c"; done

docker --context "$CTX" run --rm --user "$(id -u):$(id -g)" -v "$ROOT":/work -w /work \
  -e T="$T" -e DROPS="$drops" -e WRAPS="$declared $adds" -e ABASE="$ABASE" -e ASPAN="$ASPAN" \
  -e SEEDED="$seeded" -e SEED_ARGS="$seed_args" "$IMG" sh -c '
  set -e
  O=$T/obj; mkdir -p $O
  CF="-m32 -DTARGET_NATIVE -O0 -I/work/going-decompiled/include -I/work/tools/native -I/work/tools/native/runtime/rt0 -include /work/tools/native/mips_callees.h -ffunction-sections -fdata-sections -Wno-implicit-function-declaration -Wno-int-conversion -Wno-builtin-declaration-mismatch"
  CXF="-x c++ -std=gnu++98 -fno-strict-return -m32 -DTARGET_NATIVE -O0 -I/work/going-decompiled/include -I/work/tools/native -I/work/tools/native/runtime/rt0 -ffunction-sections -fdata-sections"
  # The units, compiled exactly as runtime/link_native.sh compiles them (USA only).
  objs=""; total=0; failed=""
  for f in $(grep -rl TARGET_NATIVE going-decompiled/src/usa | sort); do
    b=$(basename "$f"); b=${b%.*}; total=$((total+1))
    case "$f" in
      *.cpp) printf "extern \"C\" {\n#include \"%s\"\n#include \"%s\"\n}\n" \
               /work/tools/native/mips_callees.h "/work/$f" > $O/$b.wrap.cpp
             set -- clang $CXF $O/$b.wrap.cpp ;;
      *)     set -- gcc $CF "$f" ;;
    esac
    if "$@" -c -o $O/$b.o 2>$O/$b.cerr; then
      # The driver owns main(); a unit defining it (022FA8) must not collide.
      objcopy --weaken-symbol=main $O/$b.o; objs="$objs $O/$b.o"
    else failed="$failed $f"; fi
  done
  echo "units compiled (.c: gcc, .cpp: clang): $((total - $(echo $failed | wc -w))) / $total"
  if [ -n "$failed" ]; then
    echo "FAIL - unit(s) did not compile:$failed"
    for f in $failed; do b=$(basename "$f"); b=${b%.*}; { grep -m3 "error:" $O/$b.cerr || head -3 $O/$b.cerr; } | sed "s/^/   /"; done
    exit 2
  fi
  gcc $CF -c tools/native/runtime/arena/arena_storage.c -o $O/arena.o
  gcc $CF -c tools/native/runtime/rt0/stubs.c           -o $O/stubs.o
  gcc $CF -c tools/native/runtime/rt0/native_stub.c     -o $O/nstub.o
  # Host C, not game C: no game include path and no -include shim, which would
  # pull in <features.h> before the _GNU_SOURCE of the driver (REG_EIP, Dl_info).
  gcc -m32 -O0 -Wall -DARENA_BASE=${ABASE}u -DARENA_SPAN=${ASPAN}u -c tools/native/hle_stub_driver.c -o $O/driver.o
  gcc $CF -c $T/wrap.c -o $O/wrap.o
  gcc $CF -c $T/add.c  -o $O/add.o
  back="$O/arena.o $O/stubs.o $O/nstub.o $O/add.o"
  # --seed-drop: compile iop_null.c with the dropped definition renamed away, so
  # the symbol is undefined, as if the line were deleted. nm then proves it.
  dropdefs=""; for d in $DROPS; do dropdefs="$dropdefs -D$d=hle_seed_dropped_$d"; done
  for s in tools/native/runtime/sdk/*.c tools/native/runtime/hw/*.c; do
    [ -e "$s" ] || continue; sb=$(basename "$s" .c)
    case "$sb" in iop_null) extra="$dropdefs" ;; *) extra="" ;; esac
    gcc $CF $extra -c "$s" -o $O/rt_$sb.o; back="$back $O/rt_$sb.o"
  done
  for d in $DROPS; do
    if nm --defined-only $O/rt_iop_null.o | grep -qw "$d"; then
      echo "FAIL - seed: $d is still defined in iop_null.o"; exit 2
    fi
  done
  echo "iop_null.o defines: $(nm --defined-only $O/rt_iop_null.o | awk "\$2 ~ /^[TtWw]\$/ {print \$3}" | sort | tr "\n" " ")"
  # Probe without --wrap and without --allow-multiple-definition. Its undefined
  # set becomes weak trap stubs, and a duplicate definition fails here, by name.
  if ! gcc -m32 -shared -Wl,--unresolved-symbols=ignore-all -Wl,-T,tools/native/runtime/arena/arena.ld \
       $O/driver.o $objs $back -lm -o $O/probe.so 2>$O/probe.err; then
    echo "FAIL - probe link failed:"; sed "s/^/   /" $O/probe.err
    echo "   ($(grep -c "multiple definition of" $O/probe.err || true) multiple definition(s))"; exit 2
  fi
  echo "extern void native_stub_hit(const char *);" > $T/auto_stubs.c
  nm -u $O/probe.so | sed "s/^ *[Uw] //" | sort -u \
    | grep -v "@GLIBC" | grep -vE "^(_ITM_.*|__gmon_start__|__cxa_finalize|stderr|native_stub_hit|hle_note_reach)$" \
    | sed "s/.*/__attribute__((weak)) void \\0(void){ native_stub_hit(\"\\0\"); }/" >> $T/auto_stubs.c
  echo "auto trap stubs: $(($(wc -l < $T/auto_stubs.c) - 1))"
  gcc $CF -c $T/auto_stubs.c -o $O/auto_stubs.o
  wrapflags=""; for s in $WRAPS; do wrapflags="$wrapflags -Wl,--wrap=$s"; done
  if ! gcc -m32 -rdynamic -Wl,--gc-sections -Wl,-T,tools/native/runtime/arena/arena.ld $wrapflags \
       $O/driver.o $O/wrap.o $objs $back $O/auto_stubs.o -lm -o $O/hle_drv 2>$O/link.err; then
    echo "FAIL - driver link failed:"; sed "s/^/   /" $O/link.err; exit 2
  fi
  $O/hle_drv zero
  if [ "$SEEDED" != absent ]; then
    case "$SEEDED" in canonical) lbl=seeded ;; *) lbl=seeded-noncanonical ;; esac
    $O/hle_drv $lbl $SEED_ARGS
  fi
' | tee "$TMP/run.log"
st=${PIPESTATUS[0]}
[ "$st" -eq 0 ] || { echo "check_hle_stubs: INCOMPLETE - the build/run step exited $st" >&2; exit 2; }

python3 "$VERDICT" --rows "$TMP/run.log" --declared "$declared" --added "$adds" --dropped "$drops" \
  --baseline "$BASELINE" --seeded "$seeded" --seed-paths "$missing"

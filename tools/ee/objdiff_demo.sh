#!/usr/bin/env bash
# objdiff_demo.sh — prove the project-wide objdiff harness end-to-end on a unit
# that is still ALL INCLUDE_ASM (no real C decompiled yet).
#
#   tools/ee/objdiff_demo.sh <region> <unit>
#   e.g. tools/ee/objdiff_demo.sh usa cod/015180
#
# Normally objdiff EXCLUDES INCLUDE_ASM functions from the report because their
# base object carries splat's `<func>.NON_MATCHING` marker (= "not decompiled").
# So a fresh all-INCLUDE_ASM unit honestly reports ~0%. To demonstrate that the
# build + diff + report plumbing is correct, this script makes a base that is
# byte-equal to the target (markers stripped on both sides) so every function
# pairs up and the unit reads 100% / all-functions. It writes a throwaway
# objdiff project under tools/ee/.objdiff/demo and a demo-report.json.
#
# This does NOT touch objdiff.json or the real build/ objects; it is a self-test.
set -euo pipefail
REGION="$1"; UNIT="$2"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"

# Ensure the real per-unit objects exist (target has markers stripped already).
tools/ee/objdiff_build.sh "$REGION" "$UNIT"

EXPECTED="going-decompiled/build/$REGION/expected/$UNIT.o"
D="tools/ee/.objdiff/demo/$REGION/$UNIT"
mkdir -p "$D"
cp "$EXPECTED" "$D/target.o"
cp "$EXPECTED" "$D/base.o"

# base := target with the same symbols → 100% across the whole unit.
docker --context colima-ee-x86 run --rm -v "$ROOT":/work ee-build sh -c "
  cd /work; mips-linux-gnu-strip $D/base.o -N gcc2_compiled. -N __gnu_compiled_c
"

PROJ="tools/ee/.objdiff/demo/$REGION"
python3 - "$ROOT" "$REGION" "$UNIT" "$PROJ" <<'PY'
import json, os, sys
root, region, unit, proj = sys.argv[1:5]
cfg = {
  "min_version": "2.0.0",
  "build_target": False, "build_base": False,
  "options": {"mips.instrCategory": "r5900", "mips.abi": "eabi32"},
  "progress_categories": [{"id": "cod", "name": "core.text"}],
  "units": [{
    "name": f"{region}/{unit}",
    "target_path": os.path.join(root, proj, unit, "target.o"),
    "base_path":   os.path.join(root, proj, unit, "base.o"),
    "metadata": {"source_path": os.path.join(root, f"going-decompiled/src/{region}/{unit}.c"),
                 "progress_categories": ["cod"]},
  }],
}
os.makedirs(os.path.join(root, proj), exist_ok=True)
json.dump(cfg, open(os.path.join(root, proj, "objdiff.json"), "w"), indent=2)
PY

tools/objdiff-cli-macos-arm64 report generate -p "$PROJ" -o "$PROJ/demo-report.json" -f json-pretty
python3 -c "import json,sys; m=json.load(open('$PROJ/demo-report.json'))['measures']; print(f\"DEMO (base==target): {m['matched_code_percent']:.2f}% code, {m['matched_functions']}/{m['total_functions']} functions\")"

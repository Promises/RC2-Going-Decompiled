#!/bin/sh
# unit_report.sh — the UNIT objdiff report for one unit, from the objects
# objdiff_build.sh just produced. This is the second half of the gate: a
# function is MATCHED only when its row here reads 100.00% (fuzzy, so a wrong or
# region-shifted %lo can still hide behind it — verify_match_unit.sh is the raw
# byte check on top).
#
#   tools/ee/unit_report.sh <region> <unit> [label]
#
# TWO ARMS, ONE LISTING. objdiff_build.sh builds the base by up to two arms
# (see its header): obj/<unit>.o (sdk29, cc1 2.9) and, for an engine-region unit
# with MATCH_ guards, obj/<unit>.engine96.o (the 2.96 engine pipeline). Each
# function's row is taken from the arm that OWNS it — the engine arm owns
# exactly the functions listed in tools/ee/.objdiff/<region>/<unit>/
# engine_funcs.txt (the MATCH_<fn> guards), the sdk arm owns every other. The
# `arm` column says which. The headline measures are recomputed over the merged
# rows: matched_functions = rows at 100.00%, matched_code = their sizes summed.
#
# A function still on INCLUDE_ASM carries the `.NON_MATCHING` marker on the base
# side and objdiff scores it 0.00% — such a row is NOT a pass and NOT a failure,
# it is "not decompiled". A function absent from the listing was never scored.
#
# Runs on the host (objdiff-cli, python3); needs no VM.
set -u
REGION="$1"; UNIT="$2"; LABEL="${3:-run}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$ROOT"
EXPECTED="going-decompiled/build/$REGION/expected/$UNIT.o"
OBJ="going-decompiled/build/$REGION/obj/$UNIT.o"
OBJ96="going-decompiled/build/$REGION/obj/$UNIT.engine96.o"
W="tools/ee/.objdiff/$REGION/$UNIT"
ENGINE_FUNCS="$W/engine_funcs.txt"
OBJDIFF=tools/objdiff-cli-macos-arm64
[ -f "$EXPECTED" ] && [ -f "$OBJ" ] || { echo "FATAL: objects missing ($EXPECTED / $OBJ) — run objdiff_build.sh first" >&2; exit 2; }
PROJ="$W/report"; mkdir -p "$PROJ"

# One objdiff project holding the sdk29 arm and, when built, the engine96 arm as
# a second "unit" against the same target.
python3 - "$ROOT" "$REGION" "$UNIT" "$PROJ" "$EXPECTED" "$OBJ" "$OBJ96" <<'PY'
import json, os, sys
root, region, unit, proj, exp, obj, obj96 = sys.argv[1:8]
def u(name, base):
    return {"name": name, "target_path": os.path.join(root, exp), "base_path": os.path.join(root, base),
            "metadata": {"source_path": os.path.join(root, f"going-decompiled/src/{region}/{unit}.c"),
                         "progress_categories": ["text"]}}
units = [u(f"{region}/{unit}", obj)]
if os.path.exists(obj96):
    units.append(u(f"{region}/{unit}@engine96", obj96))
cfg = {"min_version": "2.0.0", "build_target": False, "build_base": False,
       "options": {"mips.instrCategory": "r5900", "mips.abi": "eabi32"},
       "progress_categories": [{"id": "text", "name": ".text"}], "units": units}
json.dump(cfg, open(os.path.join(root, proj, "objdiff.json"), "w"), indent=2)
PY
"$OBJDIFF" report generate -p "$PROJ" -o "$PROJ/report.json" -f json-pretty >/dev/null 2>&1 \
  || { echo "FATAL: objdiff-cli report generate failed in $PROJ" >&2; exit 2; }

python3 - "$PROJ/report.json" "$REGION/$UNIT" "$LABEL" "$EXPECTED" "$OBJ" "$OBJ96" "$ENGINE_FUNCS" <<'PY'
import json, os, sys, time
rep = json.load(open(sys.argv[1])); unit, label, exp, obj, obj96, ef = sys.argv[2:8]
mt = lambda p: time.strftime('%Y-%m-%dT%H:%M:%S', time.localtime(os.path.getmtime(p)))
by_unit = {u["name"]: u for u in rep["units"]}
sdk = by_unit[unit]; eng = by_unit.get(unit + "@engine96")
engine_funcs = set()
if eng is not None and os.path.exists(ef):
    engine_funcs = {l.strip() for l in open(ef) if l.strip()}
def rows(u):
    return {f["name"]: (f.get("fuzzy_match_percent") or 0.0, int(f.get("size", 0))) for f in u.get("functions", [])}
rs, re_ = rows(sdk), rows(eng) if eng is not None else {}
order = [f["name"] for f in sdk.get("functions", [])]
merged = []
for name in order:
    if name in engine_funcs and name in re_:
        merged.append((name, re_[name][0], re_[name][1], "engine96"))
    else:
        merged.append((name, rs[name][0], rs[name][1], "sdk29"))
# an engine-owned function the sdk arm never saw (cannot happen: same source, but
# say so rather than drop it)
for name in engine_funcs:
    if name in re_ and name not in rs:
        merged.append((name, re_[name][0], re_[name][1], "engine96"))
total = len(merged); matched = [m for m in merged if m[1] == 100.0]
total_code = sum(m[2] for m in merged); matched_code = sum(m[2] for m in matched)
pct = (100.0 * matched_code / total_code) if total_code else 0.0
arms = f"sdk29 obj mtime {mt(obj)}" + (f", engine96 obj mtime {mt(obj96)}" if eng is not None else ", engine96 arm not built")
print(f"UNIT REPORT [{label}] {unit}: {pct:.2f}% code, {len(matched)}/{total} functions matched, "
      f"{matched_code}/{total_code} code bytes  (objdiff-cli report generate; expected.o mtime {mt(exp)}; {arms})")
n_eng = sum(1 for m in merged if m[3] == "engine96")
print(f"  scored functions: {total}  (sdk29: {total - n_eng}, engine96: {n_eng})")
for name, p, size, arm in merged:
    print(f"    {name:<40} {p:7.2f}%  size={size:<6} {arm}")
missing = sorted(f for f in engine_funcs if f not in re_)
if missing:
    print(f"  WARNING: engine-owned but not scored on the engine arm: {' '.join(missing)}")
PY

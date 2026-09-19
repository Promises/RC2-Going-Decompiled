#!/bin/sh
# asmfix_default.sh — the ONE place the ASMFIX_SHARED mirror is defaulted.
#
# SOURCED, never executed:
#     REGION=usa; . tools/ee/asmfix_default.sh
# with CWD at the repo root (host $ROOT or container /work). On return:
#     ASMFIX_SHARED  exported — the mirror path asm_unit.sh will use (CONTAINER
#                    path, /work/…, because asm_unit.sh only ever runs there)
#     ASMFIX_STATE   warm | cold | "explicit, not fingerprinted"
#     ASMFIX_PRUNE   a shell snippet that removes superseded mirrors of this
#                    region; run it INSIDE the container (host-side deletion of a
#                    container-written path is the #391 hazard). ":" when the
#                    caller set ASMFIX_SHARED itself.
#     ASMFIX_DIR / ASMFIX_MIRROR   the pieces, for logging.
#
# WHY ONE FILE (#438). asm_unit.sh rebuilds its VU0-fixed copy of the whole
# nonmatchings tree for EVERY object unless ASMFIX_SHARED names a mirror to
# reuse: ~3 min per object over the VM mount (FACT #7054/#7215). #398 added the
# fingerprinted default to objdiff_build.sh only; build.sh — the image gate,
# 26 USA units — kept building cold (~75 min/region, #428) because nothing
# carried the block across and the "keep in sync" comment is not a control.
# Both scripts now source THIS file, and this file checks, every time it is
# sourced, that every registered consumer still sources it and that nothing
# else in tools/ee assigns ASMFIX_SHARED — so re-inlining the block in one
# script, or dropping the source line from the other, aborts the next run of
# EITHER script with the file named, instead of one gate silently going cold.
#
# WHAT THE DEFAULT IS (#398). When ASMFIX_SHARED is unset, name a PER-WORKTREE
# mirror by a fingerprint of everything the mirror is derived from
# (tools/ee/asmfix_stamp.sh: every .s under the region's nonmatchings tree,
# vu0_fixup.sed, macro.inc, asm_unit.sh itself). A source change selects a new
# mirror path — built cold once by asm_unit.sh and marked `.built` — instead of
# reusing a stale one; an unchanged tree reuses the warm mirror. The path is
# under the worktree (mounted as /work), never per-machine: the VM has no
# enforced exclusivity and two tasks' builds do run at once. Superseded mirrors
# of the same region are pruned by $ASMFIX_PRUNE, in the container.
# An explicitly set ASMFIX_SHARED is honoured verbatim and NOT fingerprinted:
# the opt-in keeps its bare-`.built` behaviour (FACT #7183: it serves stale asm
# after a source edit — it is the control, not the default).
# The stamp is a content hash and is the SAME on the host and in the container
# (measured #438: d108cbf0f934cf04 both sides at d64f4be4), so objdiff_build.sh
# (stamps on the host, ~0.2 s) and build.sh (stamps in the container, ~20 s
# over the mount) name — and share — one mirror per worktree per region.
# BOUND: the stamp covers the mirror's INPUTS, not its contents; a mirror edited
# after `.built` is reused as-is. Safe only for SEQUENTIAL runs in one worktree
# (asm_unit.sh's own constraint): two runs straddling an asm-tree edit can
# prune each other's mirror, and two cold runs at once race on `.built`.
#
# Requires: REGION set; CWD = repo root. Aborts (exit 1, message on stderr) on
# consumer drift; that exit propagates to the sourcing script by design.

# --- drift control -----------------------------------------------------------
# (/usr/bin/grep, not grep: a shimmed `grep` on this fleet fails into a silent
# zero — reference_bare_grep_is_dead — and a zero here would read as "no
# drift".) The consumer list is the registry; add a script here when it starts
# sourcing this file.
ASMFIX_CONSUMERS="tools/ee/build.sh tools/ee/objdiff_build.sh"
for _c in $ASMFIX_CONSUMERS; do
  [ -f "$_c" ] || { echo "asmfix_default: registered consumer $_c is missing (CWD must be the repo root: $(pwd))" >&2; exit 1; }
  /usr/bin/grep -q '^[[:space:]]*\. tools/ee/asmfix_default\.sh' "$_c" \
    || { echo "asmfix_default: $_c no longer sources tools/ee/asmfix_default.sh — it will build COLD; restore the source line or update ASMFIX_CONSUMERS" >&2; exit 1; }
done
_stray="$(/usr/bin/grep -l '^[[:space:]]*\(export[[:space:]]\{1,\}\)\{0,1\}ASMFIX_SHARED=' tools/ee/*.sh | /usr/bin/grep -v '/asmfix_default\.sh$' || true)"
[ -z "$_stray" ] || { echo "asmfix_default: ASMFIX_SHARED is assigned outside tools/ee/asmfix_default.sh — the mirror default must live in ONE place: $(echo $_stray)" >&2; exit 1; }
unset _c _stray

# --- the default -------------------------------------------------------------
ASMFIX_DIR="tools/ee/.asmfix"
if [ -z "${ASMFIX_SHARED:-}" ]; then
  ASMFIX_STAMP="$(sh tools/ee/asmfix_stamp.sh "$REGION")"
  ASMFIX_MIRROR="mirror-$REGION-$ASMFIX_STAMP"
  ASMFIX_SHARED="/work/$ASMFIX_DIR/$ASMFIX_MIRROR"
  ASMFIX_PRUNE="mkdir -p $ASMFIX_DIR; find $ASMFIX_DIR -mindepth 1 -maxdepth 1 -name 'mirror-$REGION-*' ! -name $ASMFIX_MIRROR -exec rm -rf {} +"
  if [ -f "$ASMFIX_DIR/$ASMFIX_MIRROR/.built" ]; then ASMFIX_STATE=warm; else ASMFIX_STATE=cold; fi
else
  ASMFIX_MIRROR="$(basename "$ASMFIX_SHARED")"
  ASMFIX_PRUNE=":"
  ASMFIX_STATE="explicit, not fingerprinted"
fi
export ASMFIX_SHARED

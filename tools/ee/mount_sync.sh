#!/bin/sh
# mount_sync.sh — guard a container read against the colima sshfs stale-view
# trap (task #542 MOUNT-SYNC-1; FACT #7449, NOTE #7430, #473/#483/#497/#498).
#
# The worktree reaches the ee-build container through the VM's fuse.sshfs mount
# of /Users/<you>. A file the HOST rewrites is read TRUNCATED by a container
# whenever the VM still holds the file's cached size from an earlier access:
# the read stops at the OLD length, so the container sees the NEW content cut
# short. Measured on this host (tools/ee/.t542/, 480 + 60 + 22 trials, both
# VMs, tools/ee/ and going-decompiled/build/usa/ paths): a rewrite that GREW
# the file read truncated in 224 of 225 cases at Δ = 0..5 s and 18 of 20 at
# Δ = 10 s after the VM's last access (+ ~3 s of docker overhead; the 2 misses
# were first trials the VM had never seen), and in 0 of 40 at Δ = 20 and 30 s
# (sshfs's default 20 s attribute cache, `sshfs -o slave -o allow_other`); a
# rewrite that SHRANK the file (249/249) or kept its length (12/12) read the
# current bytes. The stale view heals on the next open — every stale first
# read was correct on the second, 0.25 s later — so a verified re-read is a
# real remedy, not a hope. objdiff_build.sh's (2a) container-write -> (2b)
# host mtc1_fixup.py (inserts nop lines: the file GROWS) -> (2c) container
# assemble is exactly this shape: a .s cut at its old length loses its tail —
# an `unterminated` assembler error the worker reads as their own edit, or a
# silently short object and a wrong unit %. A byte count cannot see a same-
# length change, so content (md5) is what is compared.
#
# Two subcommands, one file, so host and container agree on the digest:
#
#   sh tools/ee/mount_sync.sh md5 <file>
#       HOST side: print the md5 of <file> (macOS `md5 -q` or `md5sum`).
#
#   sh tools/ee/mount_sync.sh check <file> <host_md5>
#       CONTAINER side, POSIX sh (no python in the image): read <file> until its
#       md5sum equals <host_md5>, re-reading every MOUNT_SYNC_SLEEP s (0.5) up
#       to MOUNT_SYNC_TRIES (20) times. rc 0 when it agrees (a retry that was
#       needed is reported on stderr, never silent); rc 9 and a `MOUNT-SYNC FAIL`
#       line NAMING the file, both digests and both byte counts when it never
#       does. It never proceeds on a mismatch: the caller's `set -e` aborts.
#
# Callers: objdiff_build.sh (target_unit.c + base C before each cpp, base96.s
# before the engine assemble), asm_unit.sh (ASM_UNIT_S_MD5 for a host-written
# unit .s), landing_gate.sh do_build (the four split-regenerated link inputs
# before build.sh). landing_gate.sh --selftest (15) seeds a truncated file and
# requires the FAIL line; a blinded copy of this file must make that arm
# SELFTEST-FAIL.
set -u
usage() { echo "usage: mount_sync.sh md5 <file> | check <file> <host_md5>" >&2; exit 2; }
[ $# -ge 2 ] || usage
CMD="$1"; FILE="$2"

digest() {  # digest <file> — the md5 hex, whichever tool this side has
  if command -v md5sum >/dev/null 2>&1; then md5sum < "$1" | cut -d' ' -f1
  elif command -v md5 >/dev/null 2>&1; then md5 -q "$1"
  else echo "mount_sync: neither md5sum nor md5 on this side" >&2; exit 2; fi
}

case "$CMD" in
  md5)
    [ -f "$FILE" ] || { echo "mount_sync: no such file on the host: $FILE" >&2; exit 2; }
    digest "$FILE" ;;
  check)
    [ $# -ge 3 ] || usage
    WANT="$3"; TRIES="${MOUNT_SYNC_TRIES:-20}"; SLEEP="${MOUNT_SYNC_SLEEP:-0.5}"
    i=1
    while :; do
      if [ -f "$FILE" ]; then got=$(digest "$FILE"); len=$(wc -c < "$FILE" | tr -d ' '); else got=absent; len=0; fi
      if [ "$got" = "$WANT" ]; then
        [ "$i" = 1 ] || echo "mount_sync: $FILE agreed with the host on try $i of $TRIES (the mount served a stale view first)" >&2
        exit 0
      fi
      if [ "$i" -ge "$TRIES" ]; then
        echo "MOUNT-SYNC FAIL: $FILE — container md5 $got ($len B) != host md5 $WANT after $TRIES tries, ${SLEEP}s apart; the VM's sshfs view of the host file is stale or truncated, so NOTHING below was run (FACT #7449 / NOTE #7430)" >&2
        exit 9
      fi
      i=$((i+1)); sleep "$SLEEP"
    done ;;
  *) usage ;;
esac

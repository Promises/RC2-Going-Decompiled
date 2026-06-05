#!/usr/bin/env bash
# Run a command inside the colima `ee-x86` VM (the `ee-build` image), with the
# repo mounted at /work and CWD there. Gives every ad-hoc VM invocation one
# stable prefix (`tools/ee/vm.sh ...`) instead of a fresh `docker ... sh -c`.
#
#   tools/ee/vm.sh 'mips-linux-gnu-objdump -dr going-decompiled/build/usa/...o'
#   tools/ee/vm.sh 'mips-linux-gnu-as -march=r5900 -mabi=eabi -EL -G0 -o a.o a.s'
#
# The argument(s) are joined and run via `sh -c`. (build.sh / diff.sh /
# objdiff_build.sh already wrap their own docker calls — use this for one-offs.)
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
exec docker --context colima-ee-x86 run --rm -v "$ROOT":/work -w /work ee-build sh -c "$*"

#!/bin/sh
# src_alias_provides.sh <region> - PROVIDE lines for the address-named spellings
# (func_<hex>, D_<hex>) that the C sources still use for a symbol symbol_addrs
# has since RENAMED. Sourced output goes into the link's all_addr_syms.ld.
#
# A symbol_addrs rename moves splat's label to the new name (strcmp, find_fde),
# while C bodies keep calling func_00115544 / reading D_<hex>. The address is in
# the old name, so the alias is `PROVIDE(func_00115544 = 0x00115544);`.
#
# A name is aliased ONLY when this region's symbol_addrs.txt (the file splat
# reads, symbol_addrs_path in the yaml) has a live entry `name = 0x<addr>;` at
# exactly that address. A commented-out line, a staged side file
# (cheat_globals.staged.txt) and a longer identifier that merely starts with
# func_<hex> do not count. An address-named token with no symbol there is left
# undefined on purpose: in EU C that is how a USA-address name copied into EU
# code shows up (ldundef_eu.txt). Binding it to the address in its name would
# hide that behind a possibly-USA address. PROVIDE, not plain assignment, so a
# name some object defines is left alone (see overlay_package.sh).
set -eu
REGION="$1"
cd "$(dirname "$0")/../.."
SRC=going-decompiled/src/$REGION
SYMS=going-decompiled/symbol_addrs/$REGION/symbol_addrs.txt
[ -d "$SRC" ] && [ -f "$SYMS" ] || exit 0
# Whole identifiers, then an exact-shape filter. Not `grep -ow`: BSD grep's -o -w
# drops every later match on a line once one candidate fails the word boundary
# (`func_0BADC0DE_hook(); func_00C0FFEE();` yields nothing for func_00C0FFEE).
grep -rhoE '[A-Za-z_][A-Za-z0-9_]*' "$SRC" \
  | awk '/^(D_|func_)[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]+$/' | sort -u \
  | awk -v syms="$SYMS" '
    function pad8(h) { h = toupper(h); while (length(h) < 8) h = "0" h; return h }
    BEGIN {
      while ((getline line < syms) > 0) {
        sub(/\/\/.*/, "", line)                  # drop the comment part
        if (line ~ /^[ \t]*[A-Za-z_][A-Za-z0-9_]*[ \t]*=[ \t]*0x[0-9A-Fa-f]+[ \t]*;/) {
          a = line; sub(/^[^=]*=[ \t]*0x/, "", a); sub(/[^0-9A-Fa-f].*$/, "", a)
          known[pad8(a)] = 1
        }
      }
      close(syms)
    }
    {
      hex = $0; sub(/^(D_|func_)/, "", hex)
      if (pad8(hex) in known) printf "PROVIDE(%s = 0x%s);\n", $0, hex
    }'

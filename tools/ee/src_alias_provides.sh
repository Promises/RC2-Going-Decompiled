#!/bin/sh
# src_alias_provides.sh <region> - PROVIDE lines for the address-named spellings
# (func_<hex>, D_<hex>) that the C sources still use for a symbol symbol_addrs
# has since RENAMED. Sourced output goes into the link's all_addr_syms.ld.
#
# A symbol_addrs rename moves splat's label to the new name (strcmp, find_fde),
# while C bodies keep calling func_00115544 / reading D_<hex>. The address is in
# the old name, so the alias is `PROVIDE(func_00115544 = 0x00115544);`.
#
# A name is aliased ONLY when this region's symbol_addrs defines a symbol at
# exactly that address. An address-named token with no symbol there is left
# undefined on purpose: in EU C that is how a USA-address name copied into EU
# code shows up (ldundef_eu.txt). Binding it to the address in its name would
# hide that behind a possibly-USA address. PROVIDE, not plain assignment, so a
# name some object defines is left alone (see overlay_package.sh).
set -eu
REGION="$1"
cd "$(dirname "$0")/../.."
SRC=going-decompiled/src/$REGION
SYMDIR=going-decompiled/symbol_addrs/$REGION
[ -d "$SRC" ] && [ -d "$SYMDIR" ] || exit 0
grep -rhoE '(D_|func_)[0-9A-Fa-f]{4,}' "$SRC" | sort -u \
  | awk -v symfiles="$(ls "$SYMDIR"/*.txt | tr '\n' ' ')" '
    function pad8(h) { h = toupper(h); while (length(h) < 8) h = "0" h; return h }
    BEGIN {
      n = split(symfiles, f, " ")
      for (i = 1; i <= n; i++) {
        if (f[i] == "") continue
        while ((getline line < f[i]) > 0) {
          if (match(line, /=[ \t]*0x[0-9A-Fa-f]+/)) {
            a = substr(line, RSTART, RLENGTH); sub(/=[ \t]*0x/, "", a)
            known[pad8(a)] = 1
          }
        }
        close(f[i])
      }
    }
    {
      hex = $0; sub(/^(D_|func_)/, "", hex)
      if (pad8(hex) in known) printf "PROVIDE(%s = 0x%s);\n", $0, hex
    }'

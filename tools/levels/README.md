# tools/levels — level-program tooling (USA v2.00)

Level material lives ONLY under `tools/levels/`, `extracted/usa_levels/` and
`going-decompiled/levels/`. None of these is read by the boot or EU build
(`tools/ee/`, `going-decompiled/{src,asm,symbol_addrs,config}/<region>`,
`include/`, `linker_scripts/`), so nothing here can move the boot gate.
The plan is claudiverse NOTE #13098 §5 (row L2 is this extractor).

## extract_levels.py

Our own extractor. It was written from the format in NOTE #13098 §1, not
copied from OpenRAC's GPL `tools/levels.py`. It runs on the host with a plain
`python3 -I`, needs no VM, and takes about 1 s.

    # the disc: symlink, never copy (sha256 eda31b45…ec6e72)
    ln -s "/mnt/qvo/ps2-isos/Ratchet & Clank - Going Commando (USA) (v2.00).iso" source-isos/
    python3 -I tools/levels/extract_levels.py extract \
        --iso "source-isos/Ratchet & Clank - Going Commando (USA) (v2.00).iso"
    python3 -I tools/levels/extract_levels.py repack [--expect programs.tsv --version us2]

`extract` reads `G/LEVELn.WAD` (n = 0..26) and writes one directory per level
under `extracted/usa_levels/`. It is gitignored, untrusted data, and must never
be committed. Directories are named `NN_<name>`, where `NN` is the archive's own
level number: `G/LEVEL21.WAD` holds level 30.

Each directory holds:
- `lit.bin`, `bss.bin`, `data.bin`, `vtbl.bin`, `camvtbl.bin`, `sndvtbl.bin`
  and `text.bin`, the 7 sections as stored;
- `manifest.json`, with each section's dest, size, type and sha1, plus the
  entry point, the level number, the WAD location, the slot table and the
  stored program's sha1;
- `LNN.elf`, a synthesized ELF32 MIPS executable with one PT_LOAD per section.
  `.bss` is NOBITS, and the e_flags are the boot's.

The extractor fails loudly, rather than writing anything, when the format does
not hold. It fails when:
- there are not exactly 7 sections when `entry` changes or the program ends;
- the records leave bytes over;
- a section has an unexpected type;
- `.lit` is not at 0x1A7C80;
- the repacked program differs from the stored one.

There is no size heuristic. NOTE #10140 records one silently truncating
10 of 27 levels.

`repack` rebuilds each program's archive form (the 16-byte
`{dest,size,type,entry}` records, each followed by its bytes) from the `.bin`
files and the manifest only. It then prints the sha1, size and entry. With
`--expect`, it compares them against a `version program sha1 size entry` TSV
and exits 1 on any mismatch. OpenRAC rac2-decomp `config/programs.tsv` at
`b2a35115` is such a file; read it as data with `git show`.

## gen_yaml.py + split_level.py (splitting one level)

Row L3 of NOTE #13098. The split asm is committed under
`going-decompiled/levels/usa/LNN/asm/`, so a gate never re-splits.

    # function starts: OpenRAC's us2 catalogue, read as DATA (addresses only)
    R=b2a35115e0cf580a1fdab8dd1071b157799e590e
    git -C <empty-dir clone of OpenRAC/rac2-decomp> show "${R}:config/us2/levels/functions.tsv" > <scratch>/functions.tsv
    python3 -I tools/levels/gen_yaml.py --level 24 --catalogue <scratch>/functions.tsv
    .venv-decomp/bin/python tools/levels/split_level.py going-decompiled/levels/usa/L24/config/L24.yaml

`gen_yaml.py` needs the level extracted first (`extract_levels.py extract --only 24`).
It writes:
- `extracted/usa_levels/NN_<name>/LNN.split.bin`, the image splat reads. It is
  the 7 stored sections back to back (the archive-form program minus its
  record headers), so it is gitignored with the rest of `extracted/`.
- `going-decompiled/levels/usa/LNN/config/LNN.yaml`. Each section is its own
  segment at its own dest: `.text` as asm, `.bss` as a NOBITS bss subsegment,
  the other five as data, and `$gp` 0x1AEFF0.
- `.../symbol_addrs/core_externs.txt`. These are the boot's symbol_addrs
  entries below 0x1A7C80, the resident core, carrying `type:`/`size:` only.
  Nothing at or above 0x1A7C80 is taken from the boot, because the level
  overwrites that range (RULING #7226).
- `.../symbol_addrs/LNN_functions.txt`. It holds one `func_LNN_XXXXXXXX` per
  catalogue place of this level: the addresses are data, and no upstream name
  is used.

`split_level.py` runs splat unchanged, plus one rule: every auto-generated name
whose address lies inside the program (0x1A7C80 to the end of `.text`) becomes
`func_LNN_XXXXXXXX` / `D_LNN_XXXXXXXX` / `.LLNN_XXXXXXXX`. References below
0x1A7C80 keep the boot's names. Always split a level config through it. A
plain `python -m splat split` gives boot-style names to level addresses.

Splat's own analysis also starts a function at linker debris between catalogued
functions (`addiu $sp,N` / `nop` epilogue remnants). Those starts are kept, and
they get the same prefixed names. Without the catalogue, splat fuses 57 of
those debris runs into the next real function on L24 (NOTE #13153, task #2456).

Splat's undefined-symbol lists go to `going-decompiled/build/usa_levels/LNN/`,
which is gitignored. They are the linker input for row L4.

## level_build.sh + gen_ld.py + level_elf.py (building one level)

Row L4 of NOTE #13098 (task #2457). It builds a level from its committed split
asm and compares it with the disc. Nothing is re-split, and no C is compiled yet.

    python3 -I tools/levels/gen_ld.py going-decompiled/levels/usa/L24/config/L24.yaml
    tools/levels/level_build.sh 24 [--expect <OpenRAC config/programs.tsv, read as data>]

`gen_ld.py` writes the committed `going-decompiled/levels/usa/LNN/linker_scripts/LNN.ld`
from the yaml, `core_externs.txt` and the asm. The script has:
- the 7 sections at their dests in program order, each exactly its object's
  bytes (no ALIGN padding);
- one `PT_LOAD`;
- `_gp` from the yaml;
- `ENTRY()` set to the glabel at the program's entry;
- the boot's core names below 0x1A7C80 as ABSOLUTE values.

Splat's own generated `.ld` now goes to the gitignored build dir
(`splat_LNN.ld`), so a re-split cannot overwrite it. With `--check`,
`gen_ld.py` only reports whether the committed script is current; the build
runs it that way.

`level_build.sh` re-runs itself in the `ee-build` image. It uses the boot
gate's docker context (`EE_DOCKER_CONTEXT`, default `colima-ee-x86`). The steps:
1. GNU `as` assembles each `.s` with the boot's flags. The boot's VU0 operand
   fixup (`tools/ee/vu0_fixup.sed`) is read, never written. The macros come
   from the level-local `going-decompiled/levels/usa/include/macro.inc`.
2. `nm -u` minus the names another level object defines gives the undefined
   set. `level_elf.py undef` accepts only core names from `LNN.ld` or
   ADDRESS-NAMED symbols, which are valued from their name. Anything else fails.
3. GNU `ld` links, then `level_elf.py check` runs. It cuts the 7 sections out
   of the ELF's one `PT_LOAD` and compares each, by name, with
   `extracted/usa_levels/NN_*/<stem>.bin`. It then checks dest, size, type and
   entry, and repacks the archive form from the ELF alone. That sha1 must equal
   the one in the yaml header and, with `--expect`, the TSV row.
4. `cmp(1)` re-checks every blob pair as a second instrument.

The last line is `#### level_build LNN: PASS|FAIL`, followed by the wall time
with the host and docker context. Output goes to
`going-decompiled/build/usa_levels/LNN/` (gitignored): `obj/`, `LNN.elf`,
`.map`, `ld.log`, `undefined_syms.ld` and `blobs/`.

## C functions in a level (row L6, task #2495)

A level function is built from C by listing it in
`going-decompiled/levels/usa/LNN/config/LNN_c.tsv`. The row is
`function<TAB>source<TAB>arm<TAB>gflag[<TAB>as<TAB>cc1flags]`, and the source
sits under the level's own `src/`. Level headers, if any are ever needed, go in a level-local
`include/` dir, never in `going-decompiled/include/` (an EU input). The
function keeps its split name (`func_LNN_<v2.00 address>`); the level's asm
and data refer to it by that name.

    func_L24_0034D598	src/func_L24_0034D598.c	s136	-G8
    func_L24_0037AE10	src/func_L24_0037AE10.c	s136	-G8	asm_unit	-
    python3 -I tools/levels/gen_ld.py going-decompiled/levels/usa/L24/config/L24.yaml   # re-run after editing the table
    bash tools/levels/level_gate.sh 24

The two optional columns (task #2552, RULING #13291 section 1) come together
or not at all. A 4-column row is read as `as -`, which is the route every row
had before them, so an old row builds byte-identically with no edit:
- `as`: the assembler, `as` or `asm_unit` (step 3). Any other value is refused.
- `cc1flags`: extra cc1 flags, comma-separated with no spaces (e.g.
  `-fno-gcse`), or `-` for none. Each must look like an option. Refused: any
  `-G` flag (the gflag column owns it), `-O2`, and for `s136` `-fopt-stack`
  (the arm's own flags, which the build passes already), and a flag given
  twice.
`LNN.ld`'s comment names the route only for a row that is not `as -`, so a
level whose rows are all `as -` gets the same script as before.

The build steps:
1. `gen_ld.py --cut` cuts each listed function's block out of the `.text` asm.
   The block runs from the `.align` before its `nonmatching` line through its
   `endlabel`. The pieces in between go into ONE file,
   `<build>/cut/text/<X>.cut.s`, piece k under `.section .text.cutNNNN`, and
   are assembled once instead of the whole file (task #2516: 1,000 per-piece
   `as` calls would cost minutes). Each piece's `.align` lines are dropped and
   every piece section gets alignment 4: a piece often starts at the padding
   `nop` after a cut block (4 mod 8), where an `.align 3`, measured from the
   section start, would insert 4 bytes. `--cut` checks that every dropped
   `.align` was already satisfied by its next instruction's address.
2. `gen_ld.py` writes `LNN.ld` with `.text` listing piece 0, the C object,
   piece 1, and so on, in address order.
3. Each function is compiled alone through the boot's own wrapper,
   `tools/ee/ee_cc1.sh <arm>` (read-only; `-O2 <gflag>`, `s136` adds
   `-fopt-stack`, then the row's cc1flags), and assembled by the row's `as`:
   - `as`: the boot's `tools/ee/move_fixup.sed` (cc1's `move` → `daddu`), then
     GNU `as` at the row's `-G`.
   - `asm_unit`: the boot's `tools/ee/asm_unit.sh usa <cc1 .s> <obj> <gflag>`
     (read-only), our model of SN's assembler, on cc1's own output. In the boot
     the steps between cc1 and asm_unit.sh are the s136os/sn2952 splices and
     `jtbl_bind.sh`, all keyed to boot units by selector files; a level
     function is compiled alone at its own arm, so none of them applies, and
     asm_unit.sh applies `move_fixup.sed` itself. At -G8 its SN-as rules then
     run (the `la` and `li.s` expansions, the COP1/mfc1 boundary rules, the
     delay-slot rules, the la-slot pin of RULING #9966), then the RULING #8549
     `dli` pass and the RULING #10043 noapp-slot pin, then GNU as.
     What asm_unit.sh reads, and what the level route gives it:
     - `ASMFIX_SHARED` = `<build>/c/asmfix_level`, an EMPTY mirror holding
       only `.built`, so asm_unit.sh does not copy the boot's
       `asm/usa/nonmatchings` tree or the boot build's `macro.inc` (a level
       C unit `.include`s nothing). The build checks the mirror is still
       empty afterwards.
     - `tools/ee/ps2eeas_dli_sites.txt`, the RULING #8549 allowlist: always
       read (asm_unit.sh refuses an empty one), but its rows are keyed by
       region and BOOT function name, so none can match the row's own
       `func_LNN_` level function. A boot-named helper defined in the same
       level TU CAN match a row and be transformed (validator #2574 seeded
       func_0027C020 to show it). The build requires asm_unit.sh's
       `dli: 0 transforms` line for every level object, so any row that
       reaches level code FAILs the build.
     - Nothing else is passed: no unit key, no `cc1_args.tsv`, no
       `s136os_functions.txt`/`sn2952_functions.txt` row, no jtbl table, no
       `ASM_UNIT_*_MD5` (the inputs are written in the container).
     The la-slot and noapp-slot pins are assembler-wide rules (RULINGs #9966,
     #10043), not keyed to boot units; their per-object counts are printed.
4. C objects get their source's own `.text` alignment through
   `objcopy --set-section-alignment` (pieces get 4, step 1). GNU as records 16
   regardless, which placed the first C function 8 bytes late before the fix.
5. `level_elf.py cobj` refuses a C object that has any allocated section other
   than `.text`, a `.text` of the wrong size, or a global besides the function.
   `/DISCARD/` would otherwise drop a `.rodata` silently.
6. After the link, `C-PLACED ok` needs the map to put the object at the
   function's address and size, with its glabel in no assembled asm.

The build then prints `level progress LNN: <C rows> of <entries in
symbol_addrs/LNN_functions.txt> catalogue functions from C`. That is the
level's own ledger line, never added to the boot's INCLUDE_ASM figure.

A level-only C row (`LNN_c.tsv`) is single placement: its source lives in
one level. Boot C placed in many levels is the next section.

## Boot C placed in a level (row L7b, task #2516, RULING #13219)

The boot's own matched C goes into a level program at each catalogue place of
that level, without compiling a boot source again and without reading the disc
for any symbol value (RULING #13219, NOTE #13218 option (c)).

    bash tools/levels/boot_objects.sh                       # once per tree (the gate runs it)
    python3 -I tools/levels/gen_boot_names.py names --level 24   # regenerate the committed map
    python3 -I tools/levels/gen_ld.py going-decompiled/levels/usa/L24/config/L24.yaml
    bash tools/levels/level_gate.sh 24

1. `boot_objects.sh` (host): the boot build's record
   `tools/ee/.gate_landing/usa/built_tree.txt` must name this worktree's tree,
   else it runs the boot build first (`landing_gate.sh usa`, building form, cmp
   0 required). Then, in ee-build, `boot_sections.py` puts every compiled
   function of each boot unit's FINAL asm (`<unit>._u.s`, after the
   s136os/sn2952 splices and jtbl_bind) in its own `.text.<fn>` section, and
   `tools/ee/asm_unit.sh` (read-only) assembles each unit once at the -G the
   gate's `cc1_args.tsv` records; each section's alignment is set from its
   block's own `.align`. Output: `going-decompiled/build/usa_levels/boot_objs/`,
   one object per unit, shared by all 27 levels, reused while `stamp.txt` names
   the same tree and `._u.s` sha1s.
2. Committed per level, written by `gen_boot_names.py`:
   - `config/LNN_boot_places.tsv` (`places`): the boot<->place pairs of the
     OpenRAC catalogue, as data (addresses and sizes only);
   - `config/LNN_boot_c.tsv` (`names`): one row per placed function,
     `func_LNN_<place> <boot name> <unit> <size>`;
   - `symbol_addrs/LNN_boot_names.txt` (`names`): `name = 0x...; // source` for
     every name the placed code relocates against (sources: `core`, `placed`,
     `catalogue`, `split-operand`), plus `func_LNN_<place> = <boot name>;`
     aliases. A level value is the committed split's operand at the same
     offset minus the object's in-place addend; never an instruction word.
   The generator REFUSES by member, never silently: an unsectioned block, a
   size or split-block mismatch, a relocation against a section other than a
   placed `.text.<G>` or a local symbol, an unreadable split operand, and every
   user of a name that needs two values in the program.
3. `level_build.sh`: `names --check` (the committed files are the generator's
   output now), BOOTLINK (the same sections linked at their BOOT addresses with
   the names' boot values must equal the boot build's ELF: `bootcmp`), the
   level link with `LNN_boot_names.txt` as an ld script, then one
   `PLACED ok` row per placed function (map address = place = map value, size
   = catalogue size, alias present, glabel in no assembled asm) and the ledger
   line `ledger LNN: boot-copy placed N + level C M of <functions>`.

## levels_refresh.sh (keeping the boot maps current, task #2525)

The committed `LNN_boot_c.tsv`, `LNN_boot_names.txt` and `LNN.ld` are a
function of the boot C set. A boot landing that changes, or turns into C, a
function a level carries makes them stale, and the boot gate does not run
level gates. Run this on every landing:

    bash tools/levels/levels_refresh.sh          # 0 = nothing changed, 1 = changed (commit them)
    bash tools/levels/levels_refresh.sh --gate   # then level_gate.sh NN for each level; 3 = a gate FAILed

It runs `boot_objects.sh`, then `gen_boot_names.py names` and `gen_ld.py` for
every level that has a `config/LNN_boot_c.tsv`. It prints one
`CHANGED <path>: <why>` line per changed file under
`going-decompiled/levels/usa/LNN/`, plus the `placed +`/`placed -` rows of a
changed `LNN_boot_c.tsv`. Exit 2 means a step failed or no level was found.
Under `--gate`, uncommitted refreshed files fail the gate's `TREE` row, so
commit them first and re-run. Freshness is keyed on the whole worktree, so
any uncommitted edit anywhere makes the first step run the boot build.

## level_gate.sh (the level gate)

Row L5 of NOTE #13098 (task #2458). It gives a PASS or FAIL verdict for one
level program built from its COMMITTED asm. It is separate from the boot gate:
`tools/ee/landing_gate.sh` does not call it, and level results are a separate
ledger line.

    bash tools/levels/level_gate.sh 24 [--expect <OpenRAC config/programs.tsv, read as data>]
    bash tools/levels/level_gate.sh --selftest 24

Read the verdict from the `#### level_gate L24: PASS|FAIL` line, not the job
status. A wall-time line (host, docker context) follows it. The exit code is
0 for PASS, 1 for FAIL and 2 for a missing prerequisite (the level is not
extracted, or there is no config). It needs the level extracted first.

The gate checks these rows, and any FAIL fails the gate:
- `TREE`: the level's committed inputs (`going-decompiled/levels/usa/LNN`,
  `going-decompiled/levels/usa/include`, `tools/levels`) have no uncommitted
  change.
- `INPUT`: the extracted section sha1s equal the manifest's, and the
  manifest's program sha1 equals the `LNN.yaml` header.
- `BUILD`: `level_build.sh` reports PASS, and its log, ELF and blobs were all
  written by this run.
- `C` (only with an `LNN_c.tsv`): one `C-PLACED ok` build row per table row.
- `NAMES` (only with an `LNN_c.tsv`, task #2617): `second_names.py` reads every
  C row's object (fresh from this run). No undefined address-named symbol
  (`D_001395E3`, `D_L09_001B1578`; `level_elf.py`'s ADDR_NAME) may sit at an
  address that already has a DIFFERENT name. Below 0x1A7C80 that means a boot
  label, a boot operand name or a `symbol_addrs.txt` entry. At or above it,
  a label or operand name of the level's own split. The FAIL line names the
  existing name to spell instead. Base+off names (no name at the exact
  address, RULING #13291 §2) pass. So do the per-site rows of
  `tools/levels/second_names_allow.tsv`: `ACCEPTED` second names, and
  `PENDING` members that were already on master when the check landed and are
  awaiting their respell. Never add a `PENDING` row: the respell row deletes it,
  and an allowlist row that matches nothing FAILs as `STALE`. The per-level
  `LNN_boot_names.txt` maps and the boot's view of overlay memory are out of
  scope (census NOTE #13409, classes A2/A3, unruled).
- `FRESH` and `BOOTC` (only with an `LNN_boot_c.tsv`): `boot_objects.sh`
  PASS for this tree; one `PLACED ok` row per table row, 0 `PLACED FAIL`, 0
  `PLACED-DIFFERS`, `bootcmp: N of N`, and the name map up to date. Every
  PLACED row, every REFUSED member and the ledger line are printed.
- `SECTIONS`: 7 of 7 `level_elf` cmp rows read 0 with dest/size/type ok, and
  7 of 7 `cmp(1)` rows read 0.
- `ENTRY`: `e_entry` equals the manifest's entry.
- `SHA1`: the repacked archive-form sha1 equals the yaml header and the
  manifest (and the `--expect` row, if given), and a host sha1 of every built
  blob equals the manifest.
- `ANCHOR` (mandatory, external): the repacked sha1/size/entry equals this
  level's row of the committed `tools/levels/expected_programs.tsv`. Every row
  above is a round trip through `extract_levels.py`'s blobs, which the asm was
  split from, so none of them can see an extractor error. A level with no row
  FAILs as `UNANCHORED`.
- `DISC`: if `source-isos/` resolves, `disc_programs.py read` re-reads the
  program's raw range from the ISO and it must equal the table row. If not, the
  row prints `--` and the committed table is the anchor.
- `CONTROL`: the negative control, as in the boot gate's ROW line. A copy of the
  built `.text` with 4 bytes flipped must `cmp` against the disc as more than 0;
  on a byte-equal build it reads 4.

`--selftest NN` copies the level's inputs into the gitignored
`going-decompiled/build/usa_levels/level_gate_selftest/` and runs the gate
there once per arm (4 arms, 6 for a level with C rows, 9 with boot rows too,
plus arm 10's two builds wherever its fixture shape exists).
It never edits the tracked tree.
- The unseeded copy must PASS.
- Arm 11 NAMES (task #2617) runs right after it. Four fixture TUs, compiled in
  the scratch root by the C rows' route, go through `second_names.py`:
  - `D_001395E3` must FAIL naming `D_1395E3`;
  - `D_1395E3` must PASS as the same name;
  - L09 `func_L09_0040C5B8`'s allowlisted `D_001B1578` must PASS, and the same
    object filed under another function must FAIL naming `D_L09_001B1578`;
  - the base+off `D_0018CD60` must PASS.

  For a level with C rows, the gate's own NAMES row then runs on arm 1's
  objects. It must be ok as committed. It must FAIL, naming each existing name,
  with the level's allowlist rows removed. It must FAIL as STALE with a
  phantom row added.
- The seeded copy, where the entry instruction becomes `.word <value XOR 1>`,
  must FAIL on `.text` only, with the first difference at the entry address.
- With the asm restored and the level's table sha1 changed in its last digit,
  it must FAIL on `ANCHOR` (and `DISC`) and nothing else.
- With the level's table row deleted, it must FAIL as `ANCHOR: UNANCHORED`.
- For a level with C rows, two more arms run on the first row.
  - SEEDED C: the first decimal literal ≥ 1 in the function body is
    incremented. It must FAIL on `.text` only, with the first difference inside
    the function's block.
  - C ARM OFF: the table is removed and `LNN.ld` regenerated, so the function
    is assembled from its asm again. It must PASS, with a built `.text` sha1
    equal to the unseeded C-on run's.
  - The C compile needs `tools/ee/{ee_cc1.sh,move_fixup.sed,cc}`, and the
    `asm_unit` route `tools/ee/{asm_unit.sh,ps2eeas_dli.awk,
    ps2eeas_dli_sites.txt}`; all are copied read-only into the scratch root.
- ASSEMBLER COLUMN (arm 10, task #2552, every level): proves the `as` column
  changes bytes on func_L00_0042B6A8-class code (FACT #13288/#13289: cc1
  prints `la $2,SYM` for a small datum at -G8, GNU as expands it to the 64-bit
  `daddiu`, SN's assembler to `addiu`). It finds the level's copy of that
  function by its 8-instruction shape in the split `.text` asm (in L24,
  `func_L24_0040BA78`), writes a fixture TU of our own for it in the scratch
  copy only (`*obj = &D; F(obj);`, never a real row), and builds the level
  twice with that row added at `s136 -G8`, once with `as` and once with
  `asm_unit`. It passes only if the `asm_unit` build PASSes (the ROM's
  bytes), the `as` build FAILs on `.text` only with its first difference inside
  the fixture's block, and the two fixture objects' `.text` differ. Seeded in
  the same run: the same verdict with the `asm_unit` object in place of the
  `as` one (the builds' outcomes unchanged) must FAIL on the object compare.
  A level with no such block prints `--` for the arm.
- For a level with boot rows, three more arms (the boot objects and the boot
  ELF/map are copied in read-only):
  - SEEDED MAP: `g_mobyClassHeaders` +4 in `LNN_boot_names.txt`. It must FAIL
    on `.text` only, and the `PLACED-DIFFERS` members must be exactly
    `gen_boot_names.py users` of that name (read from the relocations).
  - NO ALIGNMENT: every `.text.<fn>` of the copied objects set back to 16. It
    must FAIL with `MISPLACED` rows.
  - PLACEMENTS OFF: `LNN_boot_c.tsv` removed and `LNN.ld` regenerated. It must
    PASS with a built `.text` sha1 equal to arm 1's.

Scratch verdicts are indented (`  [seeded] #### level_gate L24: FAIL`), so they
cannot be read as real ones. The last line is
`#### level_gate LNN SELFTEST: PASS|FAIL`.

### The anchor table: disc_programs.py + expected_programs.tsv

`disc_programs.py` shares no code with `extract_levels.py`. It does its own
ISO9660 walk and its own read of the WAD header and slot 0, then hashes the
program's RAW byte range as the disc stores it, with no record parsing and no
repack. `expected_programs.tsv` is its output for all 27 WADs, committed. It
was cross-checked against OpenRAC's `config/programs.tsv` (rac2-decomp
`b2a35115`, read as data with `git show`), and 27 of 27 rows are EQUAL. Their
`boot` row is not a level and is listed as informational.

    python3 -I tools/levels/disc_programs.py table --iso "source-isos/Ratchet & Clank - Going Commando (USA) (v2.00).iso" --names extracted/usa_levels > tools/levels/expected_programs.tsv
    python3 -I tools/levels/disc_programs.py crosscheck tools/levels/expected_programs.tsv <scratch>/programs.tsv

Timing (task #2458), measured at e34468af1 on xps13-arch. The docker context
was `colima-ee-x86`, the local daemon. Reproduce with the commands below.
- Gate wall time: 5.21 s, of which `level_build` took 4.78 s.
- Selftest wall time: 21.64 s, for 4 arms.

    bash tools/levels/level_gate.sh 24 | tail -1     # `level_gate: wall N s (host, context, rc)`

A cold re-split of L24 into scratch, the input for L8's commit-vs-generate
decision, took 23.50 s and 25.80 s on xps13-arch. In both runs `diff -r`
against the committed asm was empty. The committed asm is 30,800,317 B in
7 files:

| file | bytes |
|---|---|
| `text` | 26,392,564 |
| `data` | 3,768,116 |
| `lit` | 608,757 |
| `lvl.vtbl` | 26,160 |
| `lvl.camvtbl` | 2,393 |
| `bss` | 2,068 |
| `lvl.sndvtbl` | 259 |

The split reads `extracted/usa_levels/24_*/L24.split.bin`, which `gen_yaml.py`
writes. It is the 7 `.bin` files in program order, back to back (sha1 = the
yaml's `sha1:`).

    R=going-decompiled/build/usa_levels/resplit; rm -rf $R; mkdir -p $R
    sed -e 's|^  base_path: .*|  base_path: ../../../..|' -e "s|going-decompiled/levels/usa/L24/asm|$R/asm|; s|going-decompiled/levels/usa/L24/src|$R/src|; s|going-decompiled/build/usa_levels/L24|$R/build|g" going-decompiled/levels/usa/L24/config/L24.yaml > $R/L24.yaml
    time .venv-decomp/bin/python tools/levels/split_level.py $R/L24.yaml && diff -r going-decompiled/levels/usa/L24/asm $R/asm && echo IDENTICAL

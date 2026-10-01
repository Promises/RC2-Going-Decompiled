#!/usr/bin/env python3
"""disasm_word_blob.py - produce a READABLE listing for a .s that is only `.word`.

WHY THIS EXISTS
---------------
Eight function bodies in the SDK region are emitted by splat as raw `.word`
directives rather than instructions. They are not padding and not data: they are
complete, well-formed functions. `func_00131DE8` (= `snd_Init`) opens
`addiu sp,sp,-64`, spills s0-s4 and ra, and closes `jr ra` + `addiu sp,sp,64`.
(Since task #1255 splat emits snd_Init and snd_BankLoadFromEE_CB decoded under
those names, so their listings are gone; the other bodies are unchanged.)

The bytes are correct and the build is unaffected -- a `.word` assembles to the
same word. What is lost is READABILITY: every instruction-level tool in this tree
(float_bits_check, call_order_check, PORT_SAFETY) parses instructions, so a body
with none is invisible to all of them, and a human cannot read it either.

WHY IT IS NOT A FIX
-------------------
The root cause is in the split and is UNKNOWN. Measured and ruled out:
  * missing `type:func` in symbol_addrs -- 615 correctly-disassembled bodies
    lack it too, so it cannot be the discriminator;
  * unsupported opcodes -- objdump decodes 100% of the words, zero COP2/VU;
  * missing return -- 6 of 7 carry a clean `jr $ra` epilogue;
  * explicit configuration -- the addresses appear in no config or script.

So this emits a SIDECAR listing and touches nothing splat owns. The `.s` stays
byte-identical, because it is what the matching build assembles. Fixing the split
remains the real job; this only stops the bodies being unreadable meanwhile.

The disassembler is the toolchain's own (`mips-linux-gnu-objdump -m mips:5900`),
run in the EE VM -- not a decoder written here. A hand-rolled decoder would be a
second thing to be wrong, and the point is to read what the ROM actually contains.

Usage:
    tools/ee/disasm_word_blob.py <blob.s> [<blob.s> ...]
Writes <name>.listing.txt next to a chosen output dir, and prints a summary.

Exit: 0 = every input listed; 1 = an input was not a word-blob; 2 = cannot run
(objdump/VM unavailable). Never 0 on a partial run.
"""
import os
import re
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
VM = os.path.join(ROOT, "tools/ee/vm.sh")


def words_of(path):
    """Return (words, base_vaddr) for a .word-only .s, or (None, None).

    The address is read from the .s's own comment field (`/* fileoff vaddr word */`),
    which is splat's ground truth -- deriving it from the filename would break on
    every recovered name, and those are exactly the bodies worth reading.
    """
    text = open(path, errors="replace").read()
    rows = re.findall(r'/\* [0-9A-F]+ ([0-9A-F]+) [0-9A-F]+ \*/\s+\.word 0x([0-9A-Fa-f]{8})',
                      text)
    if not rows:
        return (None, None)
    # Every instruction line must be a .word, or this is not a blob and the caller
    # is pointing at something else -- report rather than half-disassemble it.
    instr_lines = [l for l in text.split("\n") if re.match(r'\s*/\* [0-9A-F]', l)]
    if len(rows) != len(instr_lines):
        return (None, None)
    return ([int(w, 16) for w in (r[1] for r in rows)], int(rows[0][0], 16))


def disassemble(words, base):
    """Disassemble via the toolchain in the VM. Returns a list of text lines.

    Written into the repo (mounted at /work in the VM) rather than host /tmp,
    which the VM cannot see -- a mistake worth encoding here so it is not repeated.
    """
    # THE PAYLOAD TRAVELS IN THE COMMAND, NOT THROUGH THE MOUNT. Writing the probe
    # into the repo and letting the VM read it over the 9p mount races: the guest can
    # see a PARTIALLY-written file and objdump then disassembles a prefix. Measured --
    # a 608-byte probe came back as 32 instructions instead of 144, and the same input
    # succeeded moments earlier, so it is a race and not a deterministic bug. This repo
    # has hit the same 9p class before in the asmfix path. base64 through the command
    # line has no shared-filesystem step to be half-done.
    blob = b"".join(struct.pack("<I", w) for w in words) + b"\x00" * 32
    import base64
    b64 = base64.b64encode(blob).decode()
    try:
        # -z (--disassemble-zeroes) is LOAD-BEARING, not cosmetic. Without it objdump
        # ELIDES runs of zero words as `...`, so the listing silently loses rows and
        # every instruction after the first such run is attributed to the wrong
        # address. Measured on func_00131DE8: 144 words in, 120 rows out, misaligned
        # from 0x131EBC onward -- while the prologue AND the epilogue still read
        # perfectly, which is exactly why eyeballing the output did not catch it.
        r = subprocess.run(
            ["bash", VM,
             "echo %s | base64 -d > /tmp/dwb.bin && "
             "mips-linux-gnu-objdump -D -z -b binary -m mips:5900 -EL "
             "--adjust-vma=0x%X /tmp/dwb.bin" % (b64, base)],
            capture_output=True, text=True, timeout=300)
    finally:
        pass
    if r.returncode != 0:
        return None
    out = []
    for line in r.stdout.split("\n"):
        m = re.match(r'\s*([0-9a-f]+):\s+([0-9a-f]{8})\s+(.*)', line)
        if m:
            out.append("    /* %s %s */  %s" % (m.group(1).upper(), m.group(2), m.group(3)))
    # Drop the padding words appended above so the listing matches the body exactly.
    return out[:len(words)]


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    bad = 0
    for path in sys.argv[1:]:
        name = os.path.basename(path)[:-2]
        words, base = words_of(path)
        if words is None:
            print("NOT-A-WORD-BLOB: %s (has decoded instructions, or no .word rows)" % path,
                  file=sys.stderr)
            bad = 1
            continue
        lines = disassemble(words, base)
        if lines is None:
            print("UNEXAMINABLE: objdump could not run (VM down?)", file=sys.stderr)
            sys.exit(2)
        # SELF-VERIFY BEFORE WRITING. The first version of this tool emitted a listing
        # that was misaligned from its 24th instruction onward and still opened with a
        # correct prologue and closed with a correct epilogue -- unreadable as an error
        # by eye. A listing whose addresses or words disagree with the .s is worse than
        # no listing: it is a wrong body that reads like a right one, and it would be
        # decompiled in good faith. So the encoding is checked, not the appearance.
        got = re.findall(r'/\* ([0-9A-F]+) ([0-9a-f]{8}) \*/', "\n".join(lines))
        mismatch = None
        if len(got) != len(words):
            mismatch = "row count %d != word count %d" % (len(got), len(words))
        else:
            for i, (addr, word) in enumerate(got):
                if int(addr, 16) != base + 4 * i or int(word, 16) != words[i]:
                    mismatch = ("row %d: listing %s/%s vs .s %08X/%08X"
                                % (i, addr, word, base + 4 * i, words[i]))
                    break
        if mismatch:
            print("VERIFY FAILED for %s -- %s. No listing written." % (name, mismatch),
                  file=sys.stderr)
            bad = 1
            continue
        dest = path[:-2] + ".listing.txt"
        with open(dest, "w") as fh:
            fh.write("/* READABLE LISTING - NOT BUILD INPUT.\n"
                     " * Generated by tools/ee/disasm_word_blob.py from %s, which splat\n"
                     " * emitted as raw .word. The .s remains the byte-exact build input;\n"
                     " * this file exists only so the body can be read and decompiled.\n"
                     " * %d instructions, base vaddr 0x%08X.\n */\n\n"
                     % (os.path.basename(path), len(words), base))
            fh.write("glabel %s\n" % name)
            fh.write("\n".join(lines))
            fh.write("\nendlabel %s\n" % name)
        print("  %-22s %4d instructions -> %s" % (name, len(words), os.path.relpath(dest, ROOT)))
    return bad


if __name__ == "__main__":
    sys.exit(main())

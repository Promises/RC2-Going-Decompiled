#!/usr/bin/env python3
"""ps2eeas_dli_sites.py: re-derive every row of tools/ee/ps2eeas_dli_sites.txt.

RULING #8549 permits Ps2EeAs.exe's `dli` expansion only at checked-in sites,
each carrying evidence that the expansion equals the ROM there. asm_unit.sh
(via ps2eeas_dli.awk) trusts the words on a row. This script is what stops a
row that is not the ROM's, or not Ps2EeAs's, from being trusted.

    python3 tools/ee/ps2eeas_dli_sites.py            # host only, no VM
    python3 tools/ee/ps2eeas_dli_sites.py --ps2eeas  # also run Ps2EeAs.exe (VM)
    python3 tools/ee/ps2eeas_dli_sites.py [--ps2eeas] <other.txt>   # check a copy

A row FAILS when any of these holds:
  SIM      simulating the words as 64-bit EE ops does not leave exactly <value>
           in <reg>, from two different starting register values. Words that
           read the old value, or write another register, fail here.
  SPLAT    <function>'s splat file (asm/<region>/nonmatchings/**/<function>.s)
           is missing, or does not hold every word address of the site. This
           is what a rename or a re-split shows as.
  ROM      the words differ from extracted/<region>/*.rom at the splat line's
           ROM offset, or from the byte field splat transcribed on that line.
  DUP      the (region, function, operands) key appears on two rows.
  PS2EEAS  (with --ps2eeas) the words Ps2EeAs.exe emits for `dli <operands>`
           differ from the row. Needs the VM and tools/ee/cc/ee/bin/Ps2EeAs.exe.

Prints one line per row and a final `ps2eeas_dli_sites: N rows, F failed`.
Exits 0 only when F is 0. Exit 2 means an input could not be read.
"""
import glob
import os
import re
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SITES = os.path.join(ROOT, 'tools/ee/ps2eeas_dli_sites.txt')
M64 = (1 << 64) - 1
SPLAT_LINE = re.compile(r'/\* ([0-9A-F]+) ([0-9A-F]{8}) ([0-9A-F]{8}) \*/')


def parse(path):
    rows = []
    with open(path) as fh:
        for n, raw in enumerate(fh, 1):
            f = raw.split('#', 1)[0].split()
            if not f:
                continue
            if (len(f) < 6 or f[0] not in ('usa', 'eu')
                    or not re.fullmatch(r'0x[0-9A-Fa-f]+', f[2])
                    or not re.fullmatch(r'\$[0-9]+,0x[0-9a-f]+', f[3])
                    or not all(re.fullmatch(r'[0-9a-f]{8}', w) for w in f[4:])):
                rows.append((n, None, raw.rstrip()))
                continue
            reg, val = f[3].split(',')
            rows.append((n, dict(region=f[0], fn=f[1], addr=int(f[2], 16), ops=f[3],
                                 reg=int(reg[1:]), value=int(val, 16),
                                 words=[int(w, 16) for w in f[4:]]), raw.rstrip()))
    return rows


def simulate(words, reg, start):
    """Run the words on a register file where every GPR holds `start`."""
    r = [start & M64] * 32
    r[0] = 0
    for w in words:
        op, rs, rt = w >> 26, (w >> 21) & 31, (w >> 16) & 31
        rd, sa, fn, imm = (w >> 11) & 31, (w >> 6) & 31, w & 63, w & 0xFFFF
        if op == 13:                                   # ori
            dst, v = rt, r[rs] | imm
        elif op == 9:                                  # addiu (sign-extends)
            v = (r[rs] + (imm - 0x10000 if imm & 0x8000 else imm)) & 0xFFFFFFFF
            dst, v = rt, (v - (1 << 32) if v & 0x80000000 else v) & M64
        elif op == 15 and rs == 0:                     # lui (sign-extends)
            v = imm << 16
            dst, v = rt, (v - (1 << 32) if v & 0x80000000 else v) & M64
        elif op == 0 and rs == 0 and fn == 56:         # dsll
            dst, v = rd, (r[rt] << sa) & M64
        elif op == 0 and rs == 0 and fn == 60:         # dsll32
            dst, v = rd, (r[rt] << (sa + 32)) & M64
        else:
            return None, 'word %08x is not addiu/ori/lui/dsll/dsll32' % w
        if dst != reg:
            return None, 'word %08x writes $%d, not $%d' % (w, dst, reg)
        r[dst] = v
    return r[reg], ''


def splat_words(region, fn, addr, count):
    """[(rom_offset, splat_word)] for count words from addr, or an error."""
    hits = glob.glob(os.path.join(ROOT, 'going-decompiled/asm', region, 'nonmatchings', '**', fn + '.s'),
                     recursive=True)
    if len(hits) != 1:
        return None, '%d splat files named %s.s' % (len(hits), fn)
    have = {}
    with open(hits[0]) as fh:
        for line in fh:
            m = SPLAT_LINE.search(line)
            if m:
                b = bytes.fromhex(m.group(3))
                have[int(m.group(2), 16)] = (int(m.group(1), 16), struct.unpack('<I', b)[0])
    want = [addr + 4 * i for i in range(count)]
    missing = [a for a in want if a not in have]
    if missing:
        return None, '%s holds no word at %s' % (os.path.relpath(hits[0], ROOT),
                                                 ' '.join('0x%08X' % a for a in missing))
    return [have[a] for a in want], ''


def ps2eeas_words(rows):
    """Assemble every row's dli with Ps2EeAs.exe in the VM: {row line: [words]}."""
    tmp = os.path.join(ROOT, 'tools/ee/.ps2eeas_dli')
    os.makedirs(tmp, exist_ok=True)
    with open(os.path.join(tmp, 'sites.s'), 'w') as fh:
        fh.write('\t.text\n\t.set\tnoreorder\n')
        for n, r, _ in rows:
            fh.write('site_%d:\n\tdli\t%s\n' % (n, r['ops']))
    cmd = ('cd tools/ee/.ps2eeas_dli && rm -f sites.o && '
           'wibo /work/tools/ee/cc/ee/bin/Ps2EeAs.exe -o sites.o sites.s && '
           'mips-linux-gnu-objdump -d -z sites.o')
    out = subprocess.run([os.path.join(ROOT, 'tools/ee/vm.sh'), cmd],
                         capture_output=True, text=True)
    if out.returncode != 0:
        sys.exit('ps2eeas_dli_sites: Ps2EeAs run failed rc %d\n%s%s'
                 % (out.returncode, out.stdout, out.stderr))
    got, cur = {}, None
    for line in out.stdout.splitlines():
        m = re.match(r'[0-9a-f]+ <site_(\d+)>:', line)
        if m:
            cur = int(m.group(1)); got[cur] = []
            continue
        m = re.match(r'\s+[0-9a-f]+:\s+([0-9a-f]{8})\s', line)
        if m and cur is not None:
            got[cur].append(int(m.group(1), 16))
    return got


def main():
    args = sys.argv[1:]
    use_ps2eeas = '--ps2eeas' in args
    paths = [a for a in args if a != '--ps2eeas']
    try:
        rows = parse(paths[0] if paths else SITES)
    except OSError as e:
        print('ps2eeas_dli_sites: %s' % e, file=sys.stderr)
        sys.exit(2)
    roms = {}
    good = [(n, r, raw) for n, r, raw in rows if r]
    asm = ps2eeas_words(good) if use_ps2eeas and good else {}
    seen, failed = {}, 0
    for n, r, raw in rows:
        why = []
        if r is None:
            why.append('FORMAT row does not parse')
        else:
            key = (r['region'], r['fn'], r['reg'], r['value'])   # the awk's normops()
            if key in seen:
                why.append('DUP same key as line %d' % seen[key])
            seen.setdefault(key, n)
            for start in (0, M64):
                v, err = simulate(r['words'], r['reg'], start)
                if v is None:
                    why.append('SIM ' + err); break
                if v != r['value']:
                    why.append('SIM leaves 0x%x in $%d, not 0x%x' % (v, r['reg'], r['value'])); break
            sw, err = splat_words(r['region'], r['fn'], r['addr'], len(r['words']))
            if sw is None:
                why.append('SPLAT ' + err)
            else:
                if r['region'] not in roms:
                    hits = glob.glob(os.path.join(ROOT, 'extracted', r['region'], '*.rom'))
                    if len(hits) != 1:
                        print('ps2eeas_dli_sites: need exactly one extracted/%s/*.rom, found %d'
                              % (r['region'], len(hits)), file=sys.stderr)
                        sys.exit(2)
                    roms[r['region']] = open(hits[0], 'rb').read()
                rom = roms[r['region']]
                romw = [struct.unpack_from('<I', rom, off)[0] for off, _ in sw]
                if romw != r['words']:
                    why.append('ROM words %s' % ' '.join('%08x' % w for w in romw))
                if [w for _, w in sw] != romw:
                    why.append('ROM splat transcription %s differs from the ROM'
                               % ' '.join('%08x' % w for _, w in sw))
            if use_ps2eeas and asm.get(n) != r['words']:
                why.append('PS2EEAS emits %s' % ' '.join('%08x' % w for w in asm.get(n, [])))
        failed += bool(why)
        print('%s line %d: %s%s' % ('FAIL' if why else 'ok  ', n, raw.split('#')[0].strip(),
                                     ''.join('\n      ' + w for w in why)))
    print('ps2eeas_dli_sites: %d rows, %d failed%s'
          % (len(rows), failed, ' (Ps2EeAs checked)' if use_ps2eeas else ' (Ps2EeAs NOT run)'))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()

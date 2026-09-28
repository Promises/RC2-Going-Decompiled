# zeropad — splat extension segtype (yaml `extensions_path: tools/splat_ext`).
#
# A gap the original linker left as ZERO words inside a segment whose
# ld_fill_value is something else. It emits, at its place in the output section:
#
#     FILL(0x00000000);
#     . += <size>;
#     FILL(<the top-level segment's ld_fill_value>);
#
# so the gap is zero and every later gap gets the segment's own fill again
# (GNU ld applies a FILL statement from that point of the section onwards).
#
# Why it exists: when the SN linker took one archive member whose .text ended
# 4 mod 8 bytes past an 8-aligned start, the tail pad was the member's own
# assembler alignment (zero), not the inter-object 0xCDCDCDCD fill. A library
# member linked alone from verbatim source ends without that tail, and under
# .cod's SUBALIGN(8) nothing can be placed at a 4 mod 8 address, so the gap can
# only be filled by the linker. Link side only: no source and no object changes.
# First use: 0x123024 after libgcc.a:_fpcmp_parts_df.o (task #918).
#
# yaml:   - [0x022FA4, zeropad]            # size = up to the next subsegment

from pathlib import Path
from typing import List

from splat.segtypes.common.segment import CommonSegment
from splat.segtypes.linker_entry import LinkerEntry, LinkerWriter
from splat.segtypes.segment import Segment


def _segment_fill(segment: Segment) -> int:
    # splat gives every subsegment the global default ld_fill_value; only the
    # top-level segment's value is the one its FILL() line was written from.
    seg = segment
    while seg.parent is not None:
        seg = seg.parent
    return seg.ld_fill_value if seg.ld_fill_value is not None else 0


class LinkerEntryZeroPad(LinkerEntry):
    def __init__(self, segment: Segment):
        super().__init__(segment, [], Path(), "zeropad", "zeropad", False)
        self.object_path = None

    def emit_entry(self, linker_writer: LinkerWriter):
        linker_writer._writeln("FILL(0x00000000);")
        linker_writer._writeln(f". += 0x{self.segment.size:X};")
        linker_writer._writeln(f"FILL(0x{_segment_fill(self.segment):08X});")


class PS2SegZeropad(CommonSegment):
    def get_linker_section_order(self) -> str:
        return ""

    def get_linker_section_linksection(self) -> str:
        return ""

    def get_linker_entries(self) -> List[LinkerEntry]:
        return [LinkerEntryZeroPad(self)]

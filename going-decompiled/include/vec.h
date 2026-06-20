#ifndef VEC_H
#define VEC_H

#include "common.h"

/* Going Commando core 4-float vector (Vec4).
 *
 * The pervasive PS2-EE math primitive: a 16-byte, 16-byte-ALIGNED block of four
 * IEEE-754 floats laid out x(+0x0) y(+0x4) z(+0x8) w(+0xC). The alignment is not
 * cosmetic - the engine's vec-math helpers move the whole vector with a single
 * 128-bit EE qword (`lqc2`/`sqc2`, i.e. `lq`/`sq`), so a Vec4 MUST sit on a
 * 16-byte boundary or those ops fault. The `__attribute__((aligned(16)))` below
 * enforces that for any Vec4 the tester/native build instantiates.
 *
 * Recovered from the VU0-macro vec helpers in the USA v2.00 (SCUS_972.68) math
 * cluster around 0x283600-0x2839xx (region-agnostic; the EU v1.00 twins are
 * byte-identical modulo the usual .text offset):
 *
 *   - Every helper begins `lqc2 vf,0x0(arg)` - the vector is read as ONE qword
 *     from offset 0, so x/y/z/w map directly to VU lanes .x/.y/.z/.w == +0/+4/+8/+C.
 *   - The xyz family (add/sub/scale/dot/cross/length, e.g. 0x283670 dot,
 *     0x2836A0 cross, 0x2836E0 length, 0x283740 xy-length) operates on `.xyz`
 *     only - the w lane is carried through the qword load/store but NOT used in
 *     the arithmetic. For these helpers w is effectively padding (preserved on
 *     copy-style ops, ignored on math).
 *   - The full-width family DOES use w: the `vmul.xyzw` scale-with-w sibling
 *     ScaleVec4IncludingW @0x283710 multiplies all four lanes. So w is a real
 *     4th component for those - it just isn't part of 3D position/direction math.
 *     (0x283640/0x283648 is a different op - the SetVec4UnitZ unit-vector store.)
 *
 * Conclusion: Vec4 is a homogeneous-style xyzw vector. In 3D use (position,
 * direction, color rgba, plane) w carries meaning per call site; in the xyz
 * vector-math helpers w is pass-through. CONFIRMED layout + alignment; the
 * per-field SEMANTICS of w are call-site-dependent (documented at each site).
 *
 * This matches the bsphere/position vec4s already modeled in moby.h (+0x00,
 * +0x10) and the camera pos/fade/matrix-row vec4s in camera.h.
 */
typedef struct Vec4 {
    f32 x;  /* +0x0  VU lane .x */
    f32 y;  /* +0x4  VU lane .y */
    f32 z;  /* +0x8  VU lane .z */
    f32 w;  /* +0xC  VU lane .w - 4th component (homogeneous/alpha/used by xyzw
               ops); pass-through padding for the xyz-only vec math */
} __attribute__((aligned(16))) Vec4;

/* Vec4 has no pointer fields, so its size/alignment are ABI-independent: these
 * hold under both ILP32 and LP64 native compiles. Guard on C11 so the ee-gcc 2.9
 * (C89) matching toolchain - which lacks _Static_assert/_Alignof and would emit
 * recoverable parse-error noise when a matched TU includes this header - skips
 * them. Byte output is unaffected either way; this just keeps the EE build log clean. */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(Vec4) == 0x10, "Vec4 must be 16 bytes for lqc2/sqc2");
_Static_assert(__builtin_offsetof(Vec4, x) == 0x0, "Vec4.x");
_Static_assert(__builtin_offsetof(Vec4, y) == 0x4, "Vec4.y");
_Static_assert(__builtin_offsetof(Vec4, z) == 0x8, "Vec4.z");
_Static_assert(__builtin_offsetof(Vec4, w) == 0xC, "Vec4.w");
_Static_assert(_Alignof(Vec4) == 0x10, "Vec4 must be 16-byte aligned");
#endif

#endif /* VEC_H */

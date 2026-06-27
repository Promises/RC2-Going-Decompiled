/* conly_provides.ld — C-only-boot overlay-link PROVIDE aliases.
 *
 * Maps the compiler-runtime symbol names that the -DTARGET_NATIVE calt objects
 * reference to their in-ROM addresses, so GNU ld resolves them WITHOUT an
 * external SN-format libgcc/libm (which GNU ld rejects: ".symtab local symbol
 * at index N (>= sh_info)").  These helpers are ALREADY compiled into the ROM
 * by SN ProDG ee-gcc, exactly like the matched memset/memcpy SDK funcs.
 *
 * LINK-ONLY aliases — NOT symbol_addrs entries.  Putting them in symbol_addrs
 * would rename the matched func_<addr> and break its by-name objdiff match.
 * PROVIDE = define only if otherwise undefined (never clobbers a real def).
 * Both addresses below are region-identical: cod/015180 is region-co-located
 * (EU twin == USA addr, delta +0x0, byte-identical) — so one file serves USA
 * and EU.  Churn-free for the matching build (link-input only).
 *
 * ── CRITICAL DEPENDENCY (see resolution note in the progress ledger) ──
 * The portable #else arms of these five helpers are DEGENERATE / self-
 * referential:  func_0011FC68's #else body is literally `return a / b;`, which
 * ee-gcc lowers on EE to `jal __divdi3` == func_0011FC68 itself.  (Same for
 * __moddi3 `a%b`, __udivdi3 `u/v`, __umoddi3 `u%v`.)  They only "work" on a
 * TARGET_NATIVE x86 host that has hardware 64-bit divide.  Therefore the calt
 * overlay MUST NOT replace func_0011FC68 / func_00121450 / func_00121B20 /
 * func_001220F0 with their #else bodies — those four functions must stay as
 * the original asm (they ARE the libgcc runtime).  PROVIDE below then resolves
 * every *other* unit's `jal __divdi3` to that real asm.  If the overlay does
 * place the degenerate calt bodies at these addresses, the PROVIDE points the
 * call straight back into an infinite self-recursion.
 *
 * (Float math — sqrtf/cosf/sinf/asinf/atan2f — is NOT listed here: every one of
 * those libcalls originates solely in text/183558 (the VU0 vector-math leaves),
 * which must likewise stay asm for an EE boot — its #else arms are scalar
 * stand-ins for VU0 hardware, strictly worse than the real microcode that runs
 * fine on EE/PCSX2.  Excluding 183558 from the overlay removes all five float
 * symbols entirely, so no alias is needed.)
 */

/* libgcc 64-bit integer runtime (cod/015180 — region-co-located, delta +0x0) */
PROVIDE(__divdi3  = 0x0011FC68);   /* signed   64-bit divide  a / b */
PROVIDE(__moddi3  = 0x00121450);   /* signed   64-bit modulo  a % b */
PROVIDE(__udivdi3 = 0x00121B20);   /* unsigned 64-bit divide  u / v */
PROVIDE(__umoddi3 = 0x001220F0);   /* unsigned 64-bit modulo  u % v */

/* soft-float widen, float -> double (__extendsfdf2); referenced by text/1D54C0 */
PROVIDE(fptodp    = 0x001234F0);

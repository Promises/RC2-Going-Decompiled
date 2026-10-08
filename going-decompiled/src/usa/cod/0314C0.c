/*
 * cod/0314C0 (0x131540..0x13221F): the libsn/SDK code after libm.a's w_sqrt.o
 * (sqrt, 0x131430..0x13153B, linked from newlib's own source,
 * going-decompiled/libm/, RULING #9783), up to the snd sub-TU cod/0321A0. It
 * holds _start (the ELF entry, 0x131AE8) and main. The CD word at 0x13153C is
 * the link's fill after w_sqrt.o, not part of this unit. Split out of
 * cod/022FA8 when s_isnan.o and w_sqrt.o became `lib` subsegments
 * (task #1856); the functions and their C are unchanged.
 */
#include "common.h"

extern void *Kprintf(const char *format, ...);


extern s32 func_0011E0D8(const char *path, s32 mode);
extern s32 func_0011E4E0(s32 fd, void *buf, s32 size);
extern s32 func_0011E360(s32 fd);
extern s8 D_00138158[];    /* ROM version string block */
extern char D_0013BF50[];  /* path opened read-only */
extern char D_0013BF60[];  /* open-failure message */
extern char D_0013BF78[];  /* read-failure message */

/**
 * Return the ROM version block D_00138158, reading it on first use: when its
 * first byte is still 0, open D_0013BF50 (mode 1) with func_0011E0D8, read
 * 0xE bytes into the block with func_0011E4E0 and close it (func_0011E360).
 * A -1 from the open or the read is only reported through Kprintf; the read
 * and the close still go ahead.
 */
s8 *func_00131540(void) {
    if (D_00138158[0] == 0) {
        s32 fd = func_0011E0D8(D_0013BF50, 1);
        if (fd == -1) {
            Kprintf(D_0013BF60);
        }
        if (func_0011E4E0(fd, D_00138158, 0xE) == -1) {
            Kprintf(D_0013BF78);
        }
        func_0011E360(fd);
    }
    return D_00138158;
}


/**
 * Lazily initialise the global block D_00138158 (calling func_00131540() the
 * first time, detected by its leading byte being 0), then return 1 if byte 4 of
 * the block equals 0x54 ('T'), else 0 — a region/territory check.
 */
s32 func_001315E0(void) {
    if (D_00138158[0] == 0) {
        func_00131540();
    }
    return D_00138158[4] == 0x54;
}

/* func_00131620: 8 bytes of inter-function padding (a dead `sdr $3,0($7); nop`)
 * between func_001315E0 and the real func_00131628 — a splat mis-split, pinned
 * to size 0x8 in symbol_addrs so func_00131628 gets a clean .s. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0314C0", func_00131620);

extern u8 D_00138152;
extern s16 D_00138150;
extern u8 D_00138156;
extern void func_0011ACD0(s32 *out);
extern s32 func_0011AF30(void *buf, s32 size, s32 offset);

/**
 * Return a 2-bit status code. For the 'T' (0x54) territory variant
 * (func_001315E0() true) this is the cached byte D_00138152; otherwise sample
 * the pad/controller status word (func_0011ACD0) and return bits 1..2 of it.
 */
s32 func_00131628(void) {
    u32 status;
    if (func_001315E0()) {
        return D_00138152;
    }
    func_0011ACD0((s32 *)&status);
    return (status >> 1) & 3;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0314C0", func_00131668);

/**
 * Return the console's timezone offset in minutes. For the 'T' territory
 * variant (func_001315E0() true) this is the cached s16 D_00138150. Otherwise
 * read the OSD config word with func_0011ACD0 (EE syscall 0x4B,
 * GetOsdConfigParam): bits 21..31 are the signed timezone offset, but when the
 * 3-bit config version field (bits 13..15) is 0 the field is not valid and 540
 * (+9h, JST) is returned instead.
 *
 * The result is formed in one variable on both paths: returning from inside the
 * 'T' branch makes cc1 put the %hi of D_00138150 in $3, while the ROM (and this
 * spelling) loads it through the return register $2.
 */
s32 func_00131670(void) {
    u32 config;
    s32 offset;
    if (func_001315E0()) {
        offset = D_00138150;
    } else {
        func_0011ACD0((s32 *)&config);
        offset = (s32)config >> 21;
        if (((config >> 13) & 7) == 0) {
            offset = 540;
        }
    }
    return offset;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0314C0", func_001316C0);

/**
 * Return the console's daylight-saving flag (0/1). For the 'T' territory
 * variant (func_001315E0() true) this is the cached byte D_00138156. Otherwise
 * read the OSD config word (func_0011ACD0, GetOsdConfigParam); if its 3-bit
 * version field (bits 13..15) is 0 there is no extended config and the flag is
 * 0, else fetch byte 1 of the extended config with func_0011AF30 (EE syscall
 * 0x6F, GetOsdConfigParam2(buf, 1, 1)) and return its bit 4. The caller below
 * scales it by 60 and adds it to func_00131670's offset.
 *
 * As in func_00131670, the result is formed in one variable on every path; the
 * early-return spelling moves the %hi of D_00138156 from $2 to $3.
 */
s32 func_001316C8(void) {
    u32 config[2];
    s32 daylight;
    if (func_001315E0()) {
        daylight = D_00138156;
    } else {
        func_0011ACD0((s32 *)&config[0]);
        if (((config[0] >> 13) & 7) == 0) {
            daylight = 0;
        } else {
            func_0011AF30(&config[1], 1, 1);
            daylight = (((u8 *)&config[1])[0] >> 4) & 1;
        }
    }
    return daylight;
}

/**
 * Binary byte (0..99) → packed BCD (RTC/BCD clock family, inverse of the
 * neighbouring BCD→binary func_00131760): BCD(n) = n + 6*(n/10), e.g.
 * 59 → 0x59. The u8 param/return produce the callee-side andi masks; the
 * divide-by-10 emits the divu + beql/break zero-guard (harness break-0,7
 * fixup in move_fixup.sed) and the 3-op `mult` this cc1 produces natively.
 */
u8 func_00131730(u8 binary) {
    return binary / 10 * 6 + binary;
}

/**
 * Packed BCD byte → binary (RTC/BCD clock family, inverse of the neighbouring
 * binary→BCD func_00131730): n - 6*(n>>4), e.g. 0x59 → 59.
 *
 *   packed  a packed-BCD byte (high nibble tens, low nibble units)
 *   ->      the binary value, truncated to a byte
 *
 * Written to mirror the ROM rather than as the more obvious
 * `(n >> 4) * 10 + (n & 0xF)`: the original computes the CORRECTION term
 * (each BCD nibble wastes 6 of its 16 codes), which is why the .s holds a
 * single `mult ... 6` and a `subu` and never masks the low nibble.
 *
 * The two forms are EXACTLY equivalent, not merely equivalent on valid BCD:
 *   n = 16h + l  ->  n - 6h = 16h + l - 6h = 10h + l
 * Verified over all 256 byte values, 0 disagreements -- including malformed
 * BCD (a nibble > 9), where the low nibble simply carries into the result.
 *
 * The u8 param/return produce the callee-side `andi 0xFF` masks -- the entry
 * `andi $2,$4,0xFF` and the one in the jr delay slot.
 *
 * Non-obvious: the correction term is a GNU C local register variable bound
 * to LO. The ROM multiplies with the 2-operand `mult $3,$4` (rd = $0, product
 * left in LO) followed by `mflo $3`; a plain `(n >> 4) * 6` makes this cc1
 * pick the 3-operand R5900 `mult $3,$3,$4` alternative of its mulsi3 pattern
 * instead. Binding the product to LO selects the pattern's LO-output
 * alternative, and the read back out of LO is the `mflo`. The host build has
 * no LO register, so it gets an ordinary local.
 */
u8 func_00131760(u8 packed) {
#ifndef TARGET_NATIVE
    register u32 correction asm("lo");
#else
    u32 correction;
#endif

    correction = (packed >> 4) * 6;
    return packed - correction;
}

extern u8 func_00131760(u8 packed);

/**
 * Convert the packed-BCD time fields of the record at arg0 to binary in place:
 * apply func_00131760 to the bytes at offsets 7,6,5,3,2,1 (skipping offset 4),
 * each replaced by its decoded value.
 */
void func_00131780(u8 *arg0) {
    arg0[7] = func_00131760(arg0[7]);
    arg0[6] = func_00131760(arg0[6]);
    arg0[5] = func_00131760(arg0[5]);
    arg0[3] = func_00131760(arg0[3]);
    arg0[2] = func_00131760(arg0[2]);
    arg0[1] = func_00131760(arg0[1]);
}

extern u8 func_00131730(u8 binary);

/**
 * Convert the binary time fields of the record at arg0 to packed BCD in place
 * (the inverse of func_00131780): apply func_00131730 to the bytes at offsets
 * 7,6,5,3,2,1 (skipping offset 4), each replaced by its encoded value.
 */
void func_001317E8(u8 *arg0) {
    arg0[7] = func_00131730(arg0[7]);
    arg0[6] = func_00131730(arg0[6]);
    arg0[5] = func_00131730(arg0[5]);
    arg0[3] = func_00131730(arg0[3]);
    arg0[2] = func_00131730(arg0[2]);
    arg0[1] = func_00131730(arg0[1]);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0314C0", func_00131850);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0314C0", func_00131908);

extern void func_00131850(u8 *arg0);

/**
 * Advance the counter byte at arg0+0x3: increment it, and when it wraps to
 * 0x18 reset it to 0 and run func_00131850(arg0) to roll over to the next unit.
 */
void func_001319B0(u8 *arg0) {
    s32 next = arg0[3] + 1;
    arg0[3] = next;
    if ((next & 0xFF) == 0x18) {
        arg0[3] = 0;
        func_00131850(arg0);
    }
}

extern void func_00131908(u8 *arg0);

/**
 * Tick down the cooldown byte at arg0+0x3: if non-zero, just decrement it;
 * otherwise reload it to 0x17 and run func_00131908(arg0) to advance state.
 */
void func_001319E0(u8 *arg0) {
    u8 timer = arg0[3];
    if (timer != 0) {
        arg0[3] = timer - 1;
    } else {
        arg0[3] = 0x17;
        func_00131908(arg0);
    }
}

/**
 * Add `minutes` to the packed-BCD clock record `clock` (the layout
 * func_00131780 / func_001317E8 convert): decode it to binary, add to the
 * minutes byte (+2), carry whole hours into the hour (func_001319B0 forward,
 * func_001319E0 back) 60 minutes at a time, store the minutes and re-encode.
 * The forward carry runs only while the total exceeds 60, so a total of
 * exactly 60 is stored as-is.
 */
void func_00131A08(u8 *clock, s32 minutes) {
    s32 m;
    func_00131780(clock);
    m = clock[2] + minutes;
    if (m >= 0) {
        while (m > 60) {
            m -= 60;
            func_001319B0(clock);
        }
    } else {
        while (m < 0) {
            m += 60;
            func_001319E0(clock);
        }
    }
    clock[2] = m;
    func_001317E8(clock);
}

extern s32 func_00131670(void);
extern s32 func_001316C8(void);

/**
 * func_00131A98(arg0): combine the two clock/territory getters and dispatch.
 * value = func_00131670() + (func_001316C8() * 0x3C - 0x21C); then tail-call
 * func_00131A08(arg0, value). Both getters are parameterless (they read their
 * own globals), so arg0 only survives across the calls (in $16) for the final
 * dispatch. Void sibling call (lever #7).
 *
 * Matched 2026-06-30: keys were (1) the two getters take NO args (declaring
 * them void stops ee-gcc reloading a0 before each call), (2) grouping the
 * constant fold as a temp `r2*0x3C - 0x21C` so -0x21C lands on the product not
 * on r1, (3) the void tail call compiles to `j func_00131A08`.
 */
void func_00131A98(void *arg0) {
    s32 r1 = func_00131670();
    s32 t = func_001316C8() * 0x3C - 0x21C;
    func_00131A08(arg0, r1 + t);
}

/* Inter-function padding at 0x131AE0..0x131AE7 — the two zero words retail
 * places between func_00131A98 and _start. They exist ONLY in the trailing
 * context of asm/usa/nonmatchings/cod/015180/func_00131A98.s, after
 * `endlabel func_00131A98` (whose stub header declares size 0x48, ending the
 * body at 0x131AE0). That file is not included at all once this unit supplies a
 * C body for the function, so without this directive nothing emits them and
 * everything from _start to the next 16-byte-aligned boundary lands 8 bytes low
 * — putting the retail ELF entry 0x00131AE8 two instructions inside _start.
 *
 * Guarded because the C-alt arm is placed in the 0xC00000 overlay, which is not
 * address-pinned to retail: there the padding buys nothing and only shifts
 * relocations.
 *
 * MEASURED here (t15866, USA, ref fd032de7 + this change, flat .rom from
 * tools/ee/build.sh's LMA link vs extracted/usa/SCUS_972.68.rom):
 *   without it — _start 0x00131AE0 (retail e_entry 0x00131AE8), snd_Pump
 *     0x00132020 (retail 0x00132028), 1353 differing bytes in the .cod band;
 *   with it    — _start 0x00131AE8, snd_Pump 0x00132028, 0 differing bytes in
 *     the .cod band.
 * Prior art: af962ded (USA, never gated) and 4a16558b (EU twin). */
#ifndef TARGET_NATIVE
__asm__(".word 0\n\t.word 0");
#endif

/* _start (0x131AE8): the EE ELF entry point — hand-written crt0, NOT compilable
 * from C and not given a portable #else (it IS the machine bootstrap: a C-only
 * boot still enters through this exact assembly before any C can run). It clears
 * all 32 GPRs (padduw), all 32 FPRs (mtc1), HI/LO/HI1/LO1/SA and the FCR, zeroes
 * the .bss span D_0013C080..D_001A7470 with 128-bit `sq` stores, sets up $gp
 * (D_001AEFF0) and $sp via the two SetMemoryMode/stack syscalls (0x3C/0x3D),
 * then calls _InitSys, func_0011AEA0(0), enables interrupts (ei) and calls
 * main(argc,argv) before tail-jumping to exit. Left as INCLUDE_ASM by nature. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0314C0", _start);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0314C0", _exitThunk);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0314C0", func_00131CB0);


/* A 16-byte section header in a loaded overlay/WAD segment. The section's
 * payload immediately follows the header inline (at +0x10). */
typedef struct SectionHeader {
    void *dest;   /* 0x0 destination VA the payload is relocated to */
    s32   size;   /* 0x4 payload size in bytes */
    s32   pad8;   /* 0x8 (unused) */
    s32   key;    /* 0xC group id shared by a contiguous run of sections */
} SectionHeader;  /* 0x10 */

extern u8 *g_pLoadedSegment;

/* InstallLoadedOverlay's EE-arm match devices (task #1123). None of them emits an
 * instruction cc1 would not; natively each is empty or a plain spelling.
 *
 *   g_pLoadedSegmentAbs
 *     ADDRESSING-MODEL DEVICE (RULING #8620). The ROM reads the segment pointer
 *     with the GAS `lw $3,g_pLoadedSegment` macro expansion, `lui $3,%hi(sym);
 *     lw $3,%lo(sym)($3)` (the destination register reused for the %hi). cc1 at
 *     this unit's -G0 instead splits HIGH/LO_SUM and colours the %hi temp $4. The
 *     `.sdata` section attribute on this EXTERN DECLARATION (no definition, no
 *     data moved) makes cc1 treat the symbol as small and print the one-line
 *     macro; the #8036 assembler alias `.extern g_pLoadedSegmentAbs,16` makes
 *     GAS expand it absolutely rather than %gp_rel. The alias is an equate of
 *     g_pLoadedSegment, so the relocation names the real symbol and no alias
 *     symbol reaches the object or the image. g_pLoadedSegment has no other user
 *     in this unit. Without the attribute (or the alias): 96.18% (diff.sh fuzzy,
 *     task #1123), the split `lui $4` head.
 *   EE_REG(r)
 *     REGISTER-PIN DEVICE (RULING #8598) on a live local; see the two pins in
 *     the body for the residual each closes.
 *   R5900_SHORT_LOOP_PAD1(v, next)
 *     the R5900 short-loop pad (RULING #8435 SCHEDULING DEVICE): one `nop`
 *     before a copy loop's closing branch, tied to the pointer the branch tests
 *     (`v`) and the one reorg moves into its delay slot (`next`), as in
 *     text/188858.c and text/1DFF80.c. */
#ifndef TARGET_NATIVE
__asm__(".extern g_pLoadedSegmentAbs, 16\n\tg_pLoadedSegmentAbs = g_pLoadedSegment");
extern u8 *g_pLoadedSegmentAbs __attribute__((section(".sdata")));
#define EE_REG(r) __asm__(r)
#define R5900_SHORT_LOOP_PAD1(v, next) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+r"(v) : "r"(next))
#else
#define g_pLoadedSegmentAbs g_pLoadedSegment
#define EE_REG(r)
#define R5900_SHORT_LOOP_PAD1(v, next) ((void)0)
#endif

/* One 64-bit copy unit of the aligned payload path (ld/sd). */
typedef union CopyDword {
    u64 d;
    s32 w;
} CopyDword;

/* InstallLoadedOverlay (0x131CB8): relocate/install the freshly loaded overlay
 * segment pointed to by g_pLoadedSegment. The segment's first word is the byte
 * offset to the first section header; from there it walks consecutive 16-byte
 * section headers, copying each section's payload (which immediately follows its
 * header) to the header's dest VA — 64 bits at a time when size, src and dest
 * are all 8-byte aligned, else 32 bits at a time. It installs the run of
 * sections that share the first section's group id (key) and returns that id,
 * stopping at the first section whose id differs.
 *
 * Returns: the group id of the installed run (the caller, main, adopts it as the
 * next stage routine's address).
 *
 * Non-obvious behaviour: a zero-size section is skipped without copying, and the
 * walk has no terminator of its own — it relies on the segment ending with a
 * header whose key differs from the first one.
 *
 * MATCHED, byte-exact (task #1123; body from NOTE #8618, t1110). Load-bearing
 * spellings (each measured by removal in NOTE #8618):
 *   - the cursor as `*(s32 *)seg + (u32)seg` gives the ROM's `addu t0,v0,v1`
 *     operand order; `key == hdr->key` gives its `beql t1,v0`;
 *   - `(low = raw & 7, size = raw, low)` makes the andi read the load temp
 *     before the copy (otherwise regmove rewrites the andi onto the copy);
 *   - the empty `"+r"` fences (RULING #8483) keep each store before its
 *     pointer increment. */
s32 InstallLoadedOverlay(void) {
    u8 *seg = g_pLoadedSegmentAbs;
    s32 key = 0;
    u8 *cur = (u8 *)(*(s32 *)seg + (u32)seg);
    SectionHeader *hdr;
    u8 *dst;
    /* REGISTER-PIN DEVICE (#8598): the payload read pointer in $4, the ROM's
     * `daddu a0,t0,zero`. Unpinned, cc1 colours it differently: 97.00% (diff.sh
     * fuzzy, task #1123). */
    register u8 *src EE_REG("$4");

    while (hdr = (SectionHeader *)cur, cur = (u8 *)(hdr + 1), dst = (u8 *)hdr->dest, src = cur,
           key == 0 ? (key = hdr->key, 1) : key == hdr->key) {
        /* REGISTER-PIN DEVICE (#8598): the loaded hdr->size in $2, the ROM's
         * `lw v0,4(a3)` feeding both `daddu v1,v0,zero` and `andi v0,v0,7`.
         * Unpinned, regmove loads it straight into the copy's register:
         * 79.55% (diff.sh fuzzy, task #1123). */
        register s32 raw EE_REG("$2") = hdr->size;
        s32 size;
        s32 low;

        if ((low = raw & 7, size = raw, low) == 0 && ((u32)cur & 7) == 0 && ((u32)dst & 7) == 0) {
            /* size, src and dest all 8-byte aligned: copy 64 bits at a time */
            CopyDword *d = (CopyDword *)dst;
            CopyDword *s = (CopyDword *)cur;
            CopyDword *end = (CopyDword *)(dst + size);
            while (d != end) {
                d->d = s->d;
                __asm__("" : "+r"(d));
                d++;
                R5900_SHORT_LOOP_PAD1(d, s);
                R5900_SHORT_LOOP_PAD1(d, s);
                s++;
            }
        } else {
            /* otherwise copy 32 bits at a time */
            u8 *end = dst + size;
            while (dst != end) {
                *(u32 *)dst = *(u32 *)src;
                __asm__("" : "+r"(dst));
                dst += 4;
                R5900_SHORT_LOOP_PAD1(dst, src);
                R5900_SHORT_LOOP_PAD1(dst, src);
                src += 4;
            }
        }
        cur += hdr->size;
    }
    return key;
}

extern void LoadLevelAndInitHealth(void);
extern s32 InstallLoadedOverlay(void);

/* main (0x131D98): the game's top-level loop. Runs the one-shot init
 * func_0011FC48 once, then loops forever: call the current stage routine
 * (initially the level loader LoadLevelAndInitHealth), install the overlay
 * segment it loaded (InstallLoadedOverlay) and adopt that call's returned id as
 * the next stage routine to run, then pump the frame twice via func_0011AEA0
 * (modes 0 and 2). Never returns.
 *
 * NEAR-MISS, kept as INCLUDE_ASM for the matching build (#ifndef TARGET_NATIVE).
 * The body is otherwise byte-exact (the asm-label trick below suppresses the
 * `jal __main` ctor hook ee-gcc injects into any function literally named
 * `main`), but the original fills the first func_0011AEA0(0) call's delay slot
 * with the InstallLoadedOverlay-return capture (`move s0,v0`) and emits the
 * `a0=0` arg setup standalone, whereas ee-gcc unconditionally fills that delay
 * slot with the closest arg setup (`a0=0`) and emits the capture standalone.
 * That is a delay-slot filler tie-break — `move a0,0` is always RTL-emitted last
 * (it is part of the following call), so no C statement ordering can place the
 * capture after it. 98.8% byte-match; the portable #else below is the C entry. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0314C0", main);
#else
/* The C entry routine compiles to the symbol `main` via an asm label rather than
 * being literally named `main` so cc1 does not inject the `jal __main` ctor hook
 * (the original is built freestanding and has no such call). */
void GameMain(void) __asm__("main");
void GameMain(void) {
    void (*stage)(void);

    func_0011FC48();
    stage = LoadLevelAndInitHealth;
    for (;;) {
        stage();
        stage = (void (*)(void))InstallLoadedOverlay();
        func_0011AEA0(0);
        func_0011AEA0(2);
    }
}
#endif

// recovered splat-dropped code (epilogue-stump mis-split), byte-exact. A raw .word dump
// while it was func_00131DE8; decoded instructions since the rename (task #1255).
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0314C0", snd_Init);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/0314C0", snd_Pump);

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/0314C0", func_00132210);

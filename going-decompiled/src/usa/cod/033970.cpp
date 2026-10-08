#include "common.h"

/*
 * cod/033970 — core.text tail (0x1339F0..0x133B7F), the functions after the
 * carved 989snd sub-TU cod/0321A0. Built at the default -O2 -G0 like
 * cod/015180.
 */

extern u8 g_discToc[];
extern s32 CdReadSync(s32 arg0, s32 arg1, void *arg2);

/* Callees in cod/0321A0, declared with their definitions' types. */
extern s32 CdStartRead(s32 lbn, s32 sectors, s32 dest, void *rmode);
extern s32 func_00133230(void);
extern s32 snd_Pump(void);
extern s32 snd_CheckLoadInProgress(s32 noWait);
extern void func_001339F0(u8 *dst, u8 *src, s32 count);

/* The disc TOC that LoadDiscToc reads to g_discToc (0x14B540). Only the
 * per-level part is laid out here; the four regions are the symbols
 * g_levelTocDirectory (0x150538), g_levelTocHeader (0x1507D8),
 * g_levelAssetToc (0x150838) and g_levelDialogToc (0x151850). The sizes are
 * the byte counts LoadLevelToc copies, and each region ends where the next
 * begins. */
typedef struct {
    s32 lbn;
    s32 size;
} TocExtent;

typedef struct {
    u8 _pad0[0x4FF8];
    TocExtent levelTocDirectory[28][3]; /* per level: header, asset, dialog */
    u8 levelTocHeader[0x60];
    u8 levelAssetToc[0x1018];
    u8 levelDialogToc[0x137C];
} DiscToc;

/**
 * func_001339F0 - copy `count` bytes from src to dst, one byte at a time by
 * index (dst[i] = src[i]). A non-positive count copies nothing. LoadLevelToc
 * runs it over each TOC block it loads.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, selected in
 * tools/ee/s136os_functions.txt). That compiler fills the loop's `bnez` delay
 * slot with the `sb` and aligns the loop head with one nop, as the ROM does;
 * cc1 2.9 put the store before the branch (~88%).
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_001339F0)
S136OS_SLOT(func_001339F0);
#else
void func_001339F0(u8 *dst, u8 *src, s32 count) {
    s32 i;

    for (i = 0; i < count; i++) {
        dst[i] = src[i];
    }
}
#endif

/**
 * CdReadSync - read `sectors` sectors from disc LBA `lbn` into buf and wait
 * for the transfer to finish.
 *
 * Builds a 4-byte sceCdRMode on the stack (trycount 0x20, spindle control 1,
 * data pattern 0) and starts the read through CdStartRead, then runs the
 * 989snd tick (func_00133230), snd_Pump, and snd_CheckLoadInProgress(0),
 * which blocks until the load is no longer in progress. Returns
 * snd_CheckLoadInProgress's result.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack). It saves $ra
 * before the mode-byte stores and puts the fourth `sb` in the jal delay slot,
 * as the ROM does; cc1 2.9 grouped the stores (~54%).
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_CdReadSync)
S136OS_SLOT(CdReadSync);
#else
s32 CdReadSync(s32 lbn, s32 sectors, void *buf) {
    u8 mode[4];

    mode[0] = 0x20; /* trycount */
    mode[1] = 1;    /* spindlctrl */
    mode[2] = 0;    /* datapattern */
    mode[3] = 0;
    CdStartRead(lbn, sectors, (s32)buf, mode);
    func_00133230();
    snd_Pump();
    return snd_CheckLoadInProgress(0);
}
#endif

/**
 * LoadDiscToc - synchronously read the global disc TOC: 0xB sectors from LBA
 * 0x3E9 into g_discToc via CdReadSync, forwarding its return value. (The
 * value-return is what keeps ee-gcc from sibling-call-optimising this into
 * `j CdReadSync` - the original keeps the frame and uses jal+jr.)
 */
s32 LoadDiscToc(void) {
    return CdReadSync(0x3E9, 0xB, g_discToc);
}

/**
 * LoadLevelToc - load the TOC blocks of `level` into the disc TOC.
 *
 * Reads each of the level's three blocks (1, 3 and 3 sectors, from the LBAs
 * in levelTocDirectory[level]) into `scratch` with CdReadSync, and copies the
 * part it keeps into levelTocHeader, levelAssetToc and levelDialogToc with
 * func_001339F0. RunLevelTransition passes g_proceduralAnimFrames as the
 * scratch buffer.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack), which packs the
 * s0-s2/ra saves 8 bytes apart as the ROM does. The struct spelling matters:
 * with byte offsets from g_discToc, cc1 folds g_discToc+0x4FF8 into the base
 * register, where the ROM keeps g_discToc itself and puts 0x4FF8 in the `lw`.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_LoadLevelToc)
S136OS_SLOT(LoadLevelToc);
#else
void LoadLevelToc(s32 level, u8 *scratch) {
    DiscToc *toc = (DiscToc *)g_discToc;

    CdReadSync(toc->levelTocDirectory[level][0].lbn, 1, scratch);
    func_001339F0(toc->levelTocHeader, scratch, sizeof(toc->levelTocHeader));
    CdReadSync(toc->levelTocDirectory[level][1].lbn, 3, scratch);
    func_001339F0(toc->levelAssetToc, scratch, sizeof(toc->levelAssetToc));
    CdReadSync(toc->levelTocDirectory[level][2].lbn, 3, scratch);
    func_001339F0(toc->levelDialogToc, scratch, sizeof(toc->levelDialogToc));
}
#endif

/* Retail inter-function padding: the ROM has 13 zero words after LoadLevelToc,
 * 0x133B4C..0x133B7F, up to the cod/033B00 data at 0x133B80. They are layout
 * data, not code (RULING #8467; precedent cod/0314C0.c before _start). Without
 * them the data after this unit links 0x30 low, and every reference to it
 * moves: landing_gate usa --strict measured cmp 13918 differing bytes at
 * 5a32d8ff6 (task #1790). */
#ifndef TARGET_NATIVE
__asm__(".word 0\n\t.word 0\n\t.word 0\n\t.word 0\n\t.word 0\n\t.word 0\n\t"
        ".word 0\n\t.word 0\n\t.word 0\n\t.word 0\n\t.word 0\n\t.word 0\n\t"
        ".word 0");
#endif

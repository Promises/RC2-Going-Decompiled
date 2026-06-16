#ifndef TEXT_TABLE_H
#define TEXT_TABLE_H

#include "common.h"

/* Localized text / string-table subsystem (USA v2.00, SCUS_972.68).
 *
 * The game keeps its UI / dialog strings in a "text table": a flat array of
 * fixed-size entries, one per textId, each pointing at the already-localized
 * string for the CURRENT language. UI code never embeds strings - it calls
 * GetLocalizedString(textId) (0x2899F8) which delegates to FindTextTableEntry
 * (0x289988) to linear-search the active table by id and returns the entry's
 * char* (~100 menu/HUD/vendor/map callers; the broad EU string anchor).
 *
 * There are TWO producers of the active table:
 *   1. The level loader (LoadLevelAndInitHealth 0x26EDE8): the level WAD carries
 *      a 6-slot per-language text-table TOC (parsed at 0x26F0F0..0x26F184); the
 *      loader picks the g_bCurrentLanguage (0x1A7BBC) slot and stores it into
 *      g_pActiveTextTable (0x1B17C0) + g_nActiveTextTableCount (0x254E24). This
 *      is the primary, level-resident table.
 *   2. StreamTextTable (0x2D8F70): an on-demand swap that streams a different
 *      language text table off disc into g_pTextTableLoadBuf (0x1F28D8) - LBN
 *      g_nTextTableLbnOffset (0x14CBF0) + g_nGlobalWadBaseLbn - relocates each
 *      entry's string pointer, PUSHES the previous active table into the caller
 *      command (cmd+0x54 base / cmd+0x38 count), and installs the new one.
 *      RestorePrevTextTable (0x2D8F00) POPS it back. Used for dialog scenes.
 *
 * Entry layout is CONFIRMED from four independent sites:
 *   +0 str         GetLocalizedString returns *(base + idx*0x10); BeginSubtitle-
 *                  Display reads base[idx*4]; StreamTextTable relocates word[0].
 *   +4 textId      FindTextTableEntry compares base[idx*4 + 1] == textId.
 *   +8 voiceClip   UpdateSubtitleStateMachine (0x289BB8) reads base[idx*4 + 2]
 *                  as a voice/clip index: indexes the voice table at 0x14DF60
 *                  and is compared to g_dialogVoiceId(0x1A6414) - 6000 to sync
 *                  subtitle display to dialog audio. 0xFFFFFFFF = no voice.
 *   +0xC reserved  Stride is 0x10 (the relocate loop and all indexers advance by
 *                  4 words); no read/write of +0xC observed. Purpose UNCONFIRMED
 *                  (likely flags/padding).
 *
 * EU build (SCES_516.07) ships EN/FR/DE/ES/IT; string CONTENT and per-language
 * table data differ, but this layout/mechanism is shared. Pointer-bearing field
 * (str) is 4 bytes under the game's ILP32 ABI - the offset asserts below are
 * guarded for that and inert under a host-64 lint build. */

typedef struct TextTableEntry {
    char *str;       /* +0x0  localized string for the current language          */
    int   textId;    /* +0x4  lookup key (matched by FindTextTableEntry)          */
    int   voiceClip; /* +0x8  dialog voice/clip index, -1 if none (subtitle sync) */
    int   reserved;  /* +0xC  unobserved (flags/padding) - UNCONFIRMED            */
} TextTableEntry;

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(TextTableEntry) == 0x10, "TextTableEntry must be 0x10 under ILP32");
_Static_assert(__builtin_offsetof(TextTableEntry, str)       == 0x0, "str");
_Static_assert(__builtin_offsetof(TextTableEntry, textId)    == 0x4, "textId");
_Static_assert(__builtin_offsetof(TextTableEntry, voiceClip) == 0x8, "voiceClip");
_Static_assert(__builtin_offsetof(TextTableEntry, reserved)  == 0xC, "reserved");
#endif

#endif /* TEXT_TABLE_H */

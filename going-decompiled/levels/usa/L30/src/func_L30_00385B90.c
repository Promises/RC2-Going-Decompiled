/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00385B90, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm the SDK 2.9 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

/* The boot's file-load / dialog-voice manager block (core, symbol_addrs). */
extern char g_fileLoadVoiceState[];

/**
 * func_L30_00385B90 - start the voice line of the actor's current volume.
 * @a0: the actor; its state block is at +0x68 (the volume index chosen by
 *      func_L30_00385978 at +0x144, per-volume voice words at +0x70).
 *
 * Only when the voice manager is idle (g_fileLoadVoiceState +0x68 is 0 and
 * +0x24 is -1) and a volume is chosen: +0x24 becomes the volume's word
 * & 0x3FFF plus 0xBB8 and +0x28 is cleared.
 * Return: 1 when a line was started, else 0.
 *
 * Arm: every arm tried (sdk29 and s136, at -G0 and -G8) gives 0 differing
 * words here; sdk29 -G0 is NOTE #13281's record (its tie-break order), not a
 * measured distinction.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0032AAF0.c, function
 * func_L30_00384980 (v1.01 address), author commit 431ac503 (llesieur99;
 * `git blame -M -C` gives all 18 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_00384980 -> func_L30_00385B90 (our split name);
 *   - D_001A63A8 -> g_fileLoadVoiceState (the boot's symbol_addrs name at
 *     exactly 0x001A63A8);
 *   - the file's `#include` lines and other declarations dropped.
 */
int func_L30_00385B90(char *a0) {
    char *st = *(char **)(a0 + 0x68);
    char *g = (char *)g_fileLoadVoiceState;
    int idx;

    if (*(int *)(g + 0x68) == 0 && *(int *)(g + 0x24) == -1) {
        char *q;

        idx = *(int *)(st + 0x144);
        if (idx != -1) {
            q = st + idx * 4;
            *(int *)(g + 0x24) = (*(int *)(q + 0x70) & 0x3FFF) + 0xBB8;
            *(int *)(g + 0x28) = 0;
            return 1;
        }
    }
    return 0;
}

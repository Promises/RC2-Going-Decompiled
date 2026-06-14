#include "common.h"

/* ===== EU build (SCES_516.07) - region twin of the USA text/16E980 unit
 * (src/usa/text/16E980.c): splash/attract boot helpers, the camera system
 * core and the screen-space sprite-FX queue. Built at -O2 -G8 (per-unit
 * GFLAG in tools/ee/objdiff_build.sh), the same as the USA sibling.
 *
 * The C bodies are REGION-AGNOSTIC - objdiff masks the gp/reloc deltas - so the
 * matched bodies are spliced verbatim from the USA unit; only the referenced
 * global symbol NAMES are retargeted to the EU addresses:
 *   USA g_cameraSlots    0x1B56D0  -> EU 0x1B5750 (spimdisasm names this
 *                                     `g_nVendorBuyQuantity + 0x3508`)
 *   USA g_cameraModeVtbl 0x26E900  -> EU D_0026E680
 * The 15 compiled-out hook stubs (return 0 / no-op) carry no relocs and match
 * for free. Functions on the USA-side instruction-divergence walls (qword
 * block-copy form, branch-likely layout, SN-as nop padding, 8-byte-packed
 * saves, etc.) stay INCLUDE_ASM here as well; see the USA unit for the
 * per-function wall notes. ===== */

/* USA g_cameraModeVtbl (0x14-byte stride, runtime-filled by level overlays).
 * In the EU asm tree this is the unnamed symbol D_0026E680. */
typedef struct CameraModeVtblEntry {
    /* 0x00 */ s32 id;
    /* 0x04 */ s32 (*takeover)();
    /* 0x08 */ s32 (*enter)();
    /* 0x0C */ s32 (*update)();
    /* 0x10 */ s32 (*poll)();
} CameraModeVtblEntry;
extern CameraModeVtblEntry D_0026E680[];

/* USA g_cameraSlots (48 records of 0xA0 bytes). The EU symbol is unnamed; the
 * asm reaches it as `g_nVendorBuyQuantity + 0x3508`, so the body indexes that
 * named base to reproduce the same relocation. */
typedef struct Camera {
    /* 0x00 */ u8 pad0[0x86];
    /* 0x86 */ s16 type;          /* slot search key */
    /* 0x88 */ u8 pad88[4];
    /* 0x8C */ s16 modeId;        /* index into the camera-mode vtable */
    /* 0x8E */ u8 pad8E[0x12];
} Camera;
extern u8 g_nVendorBuyQuantity[];

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026E838);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026E928);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026EA00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026EAC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", LoadLevelAndInitHealth);

/* func_0026F580: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F580(void) {
    return 0;
}

void func_0026F588(void) {
}

void func_0026F590(void) {
}

void func_0026F598(void) {
}

/* func_0026F5A0: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F5A0(void) {
    return 0;
}

/* func_0026F5A8: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F5A8(void) {
    return 0;
}

/* func_0026F5B0: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F5B0(void) {
    return 0;
}

/* func_0026F5B8: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F5B8(void) {
    return 0;
}

void func_0026F5C0(void) {
}

void func_0026F5C8(void) {
}

void func_0026F5D0(void) {
}

void func_0026F5D8(void) {
}

void func_0026F5E0(void) {
}

void func_0026F5E8(void) {
}

void func_0026F5F0(void) {
}

void func_0026F5F8(void) {
}

void func_0026F600(void) {
}

void func_0026F608(void) {
}

/* func_0026F610: return-0 stub but the EU split folds 8 bytes of trailing
 * inter-function 0x0 padding into the body (jr/daddu + 2 nops, size 0x10). A
 * `return 0` compiles to only the 8-byte body, so this cannot match here; the
 * USA twin lands its stub without the trailing pad. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026F610);

void func_0026F620(void) {
}

/* func_0026F628: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F628(void) {
    return 0;
}

/* func_0026F630: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F630(void) {
    return 0;
}

/* func_0026F638: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F638(void) {
    return 0;
}

/* func_0026F640: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F640(void) {
    return 0;
}

/* func_0026F648: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F648(void) {
    return 0;
}

/* func_0026F650: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F650(void) {
    return 0;
}

/* func_0026F658: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F658(void) {
    return 0;
}

/* func_0026F660: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F660(void) {
    return 0;
}

/* func_0026F668: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F668(void) {
    return 0;
}

/* func_0026F670: stub-table entry - compiled-out hook, always returns 0. */
s32 func_0026F670(void) {
    return 0;
}

void func_0026F678(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026F680);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FAE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FAE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FCB8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FD28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FD60);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_0026FE88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270018);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270020);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270080);

/* func_002700F0 (USA func_00270290): find the first camera slot whose type
 * matches `type` (48-slot linear scan); returns NULL when no slot matches.
 * g_cameraSlots = g_nVendorBuyQuantity + 0x3508 in the EU asm. */
Camera *func_002700F0(s32 type) {
    Camera *cam = (Camera *)(g_nVendorBuyQuantity + 0x3508);

    do {
        if (cam->type == type) {
            return cam;
        }
        cam++;
    } while ((s32)cam < (s32)((Camera *)(g_nVendorBuyQuantity + 0x3508) + 48));
    return NULL;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270128);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270138);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270220);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270340);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270360);

/* func_002703C0 (USA CallCameraEnterHandler): invoke the camera-mode vtbl
 * `enter` handler (slot +0x8) for the camera's mode id, if installed. */
void func_002703C0(Camera *cam) {
    s32 (*handler)() = D_0026E680[cam->modeId].enter;

    if (handler != NULL) {
        handler(cam);
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270408);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002706A8);

/* func_00270870 (USA CallCameraPollHandler): invoke the camera-mode vtbl
 * `poll` handler (slot +0x10) for the camera's mode id, if installed. */
void func_00270870(Camera *cam) {
    s32 (*handler)() = D_0026E680[cam->modeId].poll;

    if (handler != NULL) {
        handler(cam);
    }
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002708B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002709C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270BC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270CA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270D18);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270D50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00270FC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00271178);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00271758);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002717F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00271A30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00271D28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00271E78);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272038);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272068);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272120);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272208);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002723E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272468);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272840);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00272B50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002731B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002732B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273438);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273440);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_002735D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273818);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273A10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273BB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/16E7B8", func_00273D38);

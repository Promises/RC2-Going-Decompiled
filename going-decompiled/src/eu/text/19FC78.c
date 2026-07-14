#include "common.h"

/*
 * EU text/19FC78 - REGION-AXIS twin of USA text/1A00F0 (moby render/anim/grid +
 * ammo-drop + bestiary TU). Ported verbatim from src/usa/text/1A00F0.c across
 * the REGION axis (2026-06-15); the matched body is region-agnostic - only the
 * unit-local callee name is retargeted to its EU address. Built -O2 -G8
 * -fno-gcse (mirrors the USA unit).
 */

extern void FillMemory32(void *dst, u32 pattern, s32 len);

/* Opaque full-size moby view; the bodies do their own (u8*)moby offset
 * arithmetic. Byte-neutral (a struct typedef emits no code; the matching arms
 * are INCLUDE_ASM regardless). */
typedef struct Moby { u8 _bytes[0x100]; } Moby;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(Moby) == 0x100, "Moby must be 0x100 under ILP32");
#endif

/*
 * Releases a moby slot: marks its state byte (+0x20) 0xFD for a static slot
 * (below the dynamic-spawn region) or 0xFE for a dynamic slot, schedules its
 * release time (+0xA0) two frames out, then removes it from the spatial grid by
 * re-bucketing with the 0x80807F7F "off-grid" sentinel range.
 * EU twin of USA FreeMoby; region-agnostic body (globals + UpdateMobyGridCells
 * resolve to the EU addresses via symbol_addrs).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_0029FCF8);
#else
extern void *g_mobySpawnStart;
extern s32 g_gameTime;
extern void UpdateMobyGridCells(void *moby, u32 sentinelRange);
void FreeMoby(Moby *moby) {
    u8 state;
    if ((u32)moby < (u32)g_mobySpawnStart) {
        state = 0xFD;
    } else {
        state = 0xFE;
    }
    *(u8 *)((u8 *)moby + 0x20) = state;
    *(s32 *)((u8 *)moby + 0xA0) = g_gameTime + 2;
    UpdateMobyGridCells(moby, 0x80807F7F);
}
#endif

/*
 * ResolveMobyAnimFramePtrs(moby): resolve the moby's primary + secondary anim
 * frame-data pointers from its animation set table. The table lives at
 * `animBase(moby+0x24) + 0x48` and is indexed by a 1-byte frame id; each entry
 * points at an "anim set" record. From the PRIMARY frame (moby+0x42, sub-index
 * moby+0x40) it caches the frame-data pointer (set[+idx*4+0x1C]) at moby+0x58 and
 * two set bytes at moby+0x6E (set[0x12]) and moby+0x6C (set[0x11]). When the
 * primary frame id is the 0xFF sentinel ("procedural"), it instead points moby+0x58
 * straight into the procedural frame pool (g_proceduralAnimFrames + idx*0x800),
 * stamps moby+0x6C=0xFF and moby+0x6E=0. The SECONDARY frame (moby+0x43, sub-index
 * moby+0x41) always caches its frame-data pointer at moby+0x5C.
 * EU twin of USA ResolveMobyAnimFramePtrs; region-agnostic body.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_0029FD50);
#else
extern u8 g_proceduralAnimFrames[];
void ResolveMobyAnimFramePtrs(Moby *moby) {
    u8 *m = (u8 *)moby;
    void **table = (void **)(*(u8 **)(m + 0x24) + 0x48);
    u8 frame = m[0x42];

    if (frame != 0xFF) {
        u8 *set = (u8 *)table[frame];
        *(void **)(m + 0x58) = *(void **)(set + m[0x40] * 4 + 0x1C);
        m[0x6E] = set[0x12];
        m[0x6C] = set[0x11];
    } else {
        m[0x6C] = frame;          /* 0xFF sentinel */
        m[0x6E] = 0;
        *(void **)(m + 0x58) = g_proceduralAnimFrames + (m[0x40] << 11);
    }

    {
        u8 *set2 = (u8 *)table[m[0x43]];
        *(void **)(m + 0x5C) = *(void **)(set2 + m[0x41] * 4 + 0x1C);
    }
}
#endif

/* UpdateMobyAnimLoopSound: maintain the moby's looping sequence sound. The active
 * emitter slot is held in +0x6D (0xFF = none) and the current sequence id in +0x6C.
 * If a slot is active, it is stopped (and the slot released) when our slot was
 * stolen (the emitter's owner at +0x88 no longer points at this moby), the playing
 * sequence changed (emitter +0x7E != +0x6C), the moby's stop flag +0x34 bit 0x40 is
 * set, or +0x7C bit 0x8000 is set. If no slot is active and the sequence is present
 * (+0x6C != 0xFF) and neither stop flag is set, it starts the loop via
 * PlayMobySound(seq, 4, moby) and records the returned slot in +0x6D. Emitter slots
 * live at g_listenerPosHistory + slot*0x70. EU twin of USA UpdateMobyAnimLoopSound. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_0029FE10);
#else
extern u8 g_listenerPosHistory[];
extern s32 PlayMobySound(s32 seq, s32 mode, Moby *moby);
extern void StopSoundEmitter(s32 slot);
void UpdateMobyAnimLoopSound(Moby *moby) {
    u8 *m = (u8 *)moby;
    s32 slot = m[0x6D];

    if (slot != 0xFF) {
        u8 *emitter = g_listenerPosHistory + slot * 0x70;
        if (*(void **)(emitter + 0x88) != (void *)moby) {
            /* our slot was stolen by another moby */
            m[0x6D] = 0xFF;
            return;
        }
        if (*(s16 *)(emitter + 0x7E) != m[0x6C] ||
            (*(u16 *)(m + 0x34) & 0x40) ||
            (*(u16 *)(m + 0x7C) & 0x8000)) {
            StopSoundEmitter(slot);
            m[0x6D] = 0xFF;
        }
        return;
    }

    /* no active emitter: start the loop unless the sequence is absent or blocked */
    if (m[0x6C] == 0xFF ||
        (*(u16 *)(m + 0x34) & 0x40) ||
        (*(u16 *)(m + 0x7C) & 0x8000)) {
        return;
    }
    m[0x6D] = PlayMobySound(m[0x6C], 4, moby);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_0029FEE8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_0029FFE8);

/* func_002A0060 (EU twin of USA func_002A04D8) — pose and average two collision-mesh
 * keyframe vectors for a moby into dst. Locates the class-header entry (header at
 * obj+0x24, base index at header+0x2C, plus arg1) and takes hi = max of the two frame
 * counts at entry+0x6 / entry+0xE; when hi >= 0 it skins the collision mesh into the
 * SPR cache. Loads the two packed keyframe vectors (func_002839F0), optionally samples
 * the per-frame scratchpad vectors at 0x70000000 + count*64 (func_00283980), scales
 * each by (obj+0x2C)/1024, applies the moby's 3x3 rotation (func_00283958 over obj+0xC0)
 * and translation (Vec4AddVu0 obj+0x10), averages the two into dst (add then *0.5), and
 * stores the func_00283708 scalar into dst.w. The unnamed VU0 helper callees are the EU
 * (-0xF0) twins of USA func_00283A48/A70/AE0/7F8. Matching build keeps the asm;
 * faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, f32 s, void *src);
extern void func_00283958(void *out, void *v, void *m);
extern void func_00283980(void *out, void *v, void *m);
extern void func_002839F0(void *dst, u64 packed);
extern f32  func_00283708(void *a, void *b);
extern void SkinMobyCollisionMesh(void *entry, s32 count, u32 flags);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0060);
#else
void func_002A0060(void *obj, s32 arg1, void *dst) {
    u8 *o = (u8 *)obj;
    u8 *hdr = *(u8 **)(o + 0x24);
    u8 *entry = hdr + *(u8 *)(hdr + 0x2C) * 16 + arg1 * 16 + 0x40;
    s16 a = *(s16 *)(entry + 0x6);
    s16 b = *(s16 *)(entry + 0xE);
    s16 hi = (a < b) ? b : a;
    f32 vecA[4];
    f32 vecB[4];

    if (hi >= 0) {
        SkinMobyCollisionMesh(entry, hi + 1, 0x80000000);
    }
    func_002839F0(vecA, *(u64 *)(entry + 0));
    func_002839F0(vecB, *(u64 *)(entry + 8));
    vecB[3] = 1.0f;
    vecA[3] = 1.0f;
    if (hi >= 0) {
        func_00283980(vecA, vecA, (void *)(0x70000000 + (*(s16 *)(entry + 0x6) << 6)));
        func_00283980(vecB, vecB, (void *)(0x70000000 + (*(s16 *)(entry + 0xE) << 6)));
    }
    Vec4ScaleVu0(vecA, *(f32 *)(o + 0x2C) * (1.0f / 1024.0f), vecA);
    Vec4ScaleVu0(vecB, *(f32 *)(o + 0x2C) * (1.0f / 1024.0f), vecB);
    func_00283958(vecA, vecA, o + 0xC0);
    func_00283958(vecB, vecB, o + 0xC0);
    Vec4AddVu0(vecA, vecA, o + 0x10);
    Vec4AddVu0(vecB, vecB, o + 0x10);
    Vec4AddVu0(dst, vecA, vecB);
    Vec4ScaleVu0(dst, 0.5f, dst);
    *(f32 *)((u8 *)dst + 0xC) = func_00283708(dst, vecA);
}
#endif

/* func_002A0200 (EU twin of USA func_002A0678) — transform a moby's indexed
 * sub-vector set into world space. The moby's class header (obj+0x24) holds a count
 * at +0x2E; the entry table sits at header + count*16. Byte arg3 offsets into that
 * table to a sentinel: if the following byte is 0xFF (empty) it returns 0, else that
 * byte indexes the source vec4 (entry + idx*16). The source is scaled by
 * (obj+0x2C)*(1/1024) via ScaleVec4IncludingW, then transformed by the moby's matrix
 * rows (obj+0xC0 / obj+0x10 via func_00283958 / Vec4AddVu0) into dst. When outArray is
 * non-null and the entry's count field (+0x16) is positive, each extra vec4 is loaded
 * (func_002839F0), scaled (Vec4ScaleVu0) and w-terminated (1.0) into outArray. Returns
 * the entry count. Unnamed VU0 callees are the EU (-0xF0) twins of USA func_00283A48/AE0.
 * Matching build keeps the asm; faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
/* Vec4AddVu0/Vec4ScaleVu0/func_00283958/func_002839F0 declared with func_002A0060 above. */
extern void ScaleVec4IncludingW(void *dst, f32 s, void *src);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0200);
#else
s32 func_002A0200(void *obj, void *dst, void *outArray, s32 arg3) {
    u8 *hdr = *(u8 **)((u8 *)obj + 0x24);
    s16 n = *(s16 *)(hdr + 0x2E);
    u8 *entry;
    u8 *sub;
    u8 *p;
    f32 scale;
    s16 count;
    u32 idx;

    if (n == 0) {
        return 0;
    }
    entry = hdr + n * 16;
    p = entry + arg3;
    if (*(u8 *)(p + 1) == 0xFF) {
        return 0;
    }
    idx = *(u8 *)(p + 1);
    scale = *(f32 *)((u8 *)obj + 0x2C) * (1.0f / 1024.0f);
    sub = entry + idx * 16;
    ScaleVec4IncludingW(dst, scale, sub);
    sub += 0x10;
    func_00283958(dst, dst, (u8 *)obj + 0xC0);
    Vec4AddVu0(dst, dst, (u8 *)obj + 0x10);

    count = *(s16 *)(sub + 6);
    if (outArray != 0 && count > 0) {
        u8 *loopDst = (u8 *)outArray;
        s16 i = count;
        do {
            func_002839F0(loopDst, *(u64 *)sub);
            sub += 8;
            Vec4ScaleVu0(loopDst,
                         *(f32 *)((u8 *)obj + 0x2C) * (1.0f / 1024.0f), loopDst);
            *(f32 *)(loopDst + 0xC) = 1.0f;
            i--;
            loopDst += 0x10;
        } while (i != 0);
    }
    return count;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0320);

/*
 * func_002A03B0 (EU twin of USA func_002A0828): unlink `node` from `list`'s
 * singly-linked free/active chain (head at list+0x54, nodes linked through +0x8) and
 * then wipe the removed node with FillMemory32(node, 0, 0x40). A null `node` is a
 * no-op. Walks from the head to find `node`'s predecessor, splices it out
 * (prev->next = node->next or head = node->next when it was first), then zeroes the
 * node's 0x40-byte record. Matching build keeps the asm; faithful TARGET_NATIVE arm.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A03B0);
#else
struct A0828Node { u8 _pad[8]; struct A0828Node *next; };
struct A0828List { u8 _pad[0x54]; struct A0828Node *head; };
void func_002A03B0(struct A0828List *list, struct A0828Node *node) {
    if (node != 0) {
        if (list->head == node) {
            list->head = node->next;
        } else {
            struct A0828Node *prev = list->head;
            if (prev != 0) {
                struct A0828Node *cur = prev->next;
                if (cur != 0) {
                    while (cur != node) {
                        prev = cur;
                        cur = prev->next;
                        if (cur == 0) {
                            break;
                        }
                    }
                    if (cur == node) {
                        prev->next = node->next;
                    }
                }
            }
        }
        FillMemory32(node, 0, 0x40);
    }
}
#endif

/*
 * AcquireProceduralAnimSlot (EU twin func_002A0448 of USA func_002A08C0):
 * first-fit/reuse a procedural-anim-frame slot for a moby. Scans
 * g_proceduralAnimSlotOwners[0..0xF]; reuses the slot already owned by `moby`, or
 * claims the first free (==0) slot, storing `moby` as the owner and resetting that
 * slot's frame counter (g_proceduralAnimSlotTimer); returns the slot index, or -1
 * if all 16 slots are taken by other mobys. Matching build keeps the asm; faithful
 * TARGET_NATIVE arm.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0448);
#else
extern u32 g_proceduralAnimSlotOwners[];
extern u32 g_proceduralAnimSlotTimer[];
s32 func_002A0448(u32 moby) {
    s32 slot;
    for (slot = 0; slot < 0x10; slot++) {
        u32 cur = g_proceduralAnimSlotOwners[slot];
        if (cur == 0 || cur == moby) {
            g_proceduralAnimSlotOwners[slot] = moby;
            g_proceduralAnimSlotTimer[slot] = 0;
            return slot;
        }
    }
    return -1;
}
#endif

/* Clears the two 16-word moby spawn-credit sub-tables at g_mobySpawnCredit
 * +0x40 and +0x80 (e.g. on level reset). EU twin of USA func_002A0918. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A04A0);
#else
extern u32 g_mobySpawnCredit[];
void func_002A04A0(void) {
    u32 *a = &g_mobySpawnCredit[0x10];
    u32 *b = &g_mobySpawnCredit[0x20];
    s32 i = 0xF;
    do {
        *a++ = 0;
        *b++ = 0;
        i--;
    } while (i >= 0);
}
#endif

/* func_002A04E0 (EU twin of USA func_002A0958): service the 16 procedural-animation
 * slots. For each slot with an owner moby: release the slot (clear the owner) if the
 * owner is being torn down (+0x20 bit 0x80) or is no longer idle (+0x42 sequence !=
 * 0xFF). While idle, hold for +0x31 frames or until the per-slot timer reaches 0x14;
 * once elapsed, if the owner's rest animation descriptor (via +0x24 anim set, indexed
 * by +0x43) matches the idle sequence, kick the blend back toward rest (+0x44 = 1 -
 * +0x4C) and step the animation (UpdateMobyAnimation). Slots with no owner reset. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A04E0);
#else
extern u32 g_proceduralAnimSlotOwners[];
extern u32 g_proceduralAnimSlotTimer[];
extern void UpdateMobyAnimation(Moby *moby);
void func_002A04E0(void) {
    s32 i;
    for (i = 0; i < 0x10; i++) {
        u8 *owner = (u8 *)g_proceduralAnimSlotOwners[i];
        u8 *animDesc;

        if (owner == NULL) {
            g_proceduralAnimSlotTimer[i] = 0;
            continue;
        }
        if ((owner[0x20] & 0x80) || owner[0x42] != 0xFF) {
            g_proceduralAnimSlotOwners[i] = 0;
            continue;
        }
        if (owner[0x31] != 0 || g_proceduralAnimSlotTimer[i] < 0x14) {
            g_proceduralAnimSlotTimer[i]++;
            continue;
        }
        animDesc = *(u8 **)(*(u8 **)(owner + 0x24) + owner[0x43] * 4 + 0x48);
        if (animDesc[0x13] == owner[0x42]) {
            *(f32 *)(owner + 0x44) = 1.0f - *(f32 *)(owner + 0x4C);
            UpdateMobyAnimation((Moby *)owner);
        }
    }
}
#endif

/* func_002A05E0 (EU twin of USA func_002A0A58) — pose one moby keyframe vector into
 * the caller's buffer arg2. func_002A4908 (EU twin of USA func_002A4D60) fills arg2
 * from the moby's keyframe data (input word = arg1 at a scratch slot). The posed
 * vector at arg2+0x30 is scaled by (obj+0x2C)/1024 (Vec4ScaleVu0), then the moby's
 * rotation is built (func_00283F18 from obj+0xC0 into a scratch matrix) and applied
 * (MatrixMultiplyVu0 arg2 = matrix * arg2), and the translation (obj+0x10) is added
 * (Vec4AddVu0). func_00283F18 is the EU (-0xF0) twin of USA func_00284008; func_002A4908
 * the (-0x458) twin of USA func_002A4D60. Callee func_002A4908 UNCONFIRMED (named by
 * shape). Faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern void func_002A4908(void *obj, s32 flag, void *inParams, void *outBuf);
extern void func_00283F18(void *dst, void *src);
extern void MatrixMultiplyVu0(void *dst, void *a, void *b);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A05E0);
#else
void func_002A05E0(void *obj, s32 arg1, void *arg2) {
    u8 *o = (u8 *)obj;
    u8 *a2 = (u8 *)arg2;
    u8 buf[0x50];
    f32 scale = *(f32 *)(o + 0x2C) * (1.0f / 1024.0f);

    *(s32 *)(buf + 0x40) = arg1;
    func_002A4908(obj, 1, buf + 0x40, arg2);
    Vec4ScaleVu0(a2 + 0x30, scale, a2 + 0x30);
    func_00283F18(buf, o + 0xC0);
    MatrixMultiplyVu0(arg2, buf, arg2);
    Vec4AddVu0(a2 + 0x30, a2 + 0x30, o + 0x10);
}
#endif

/* func_002A0680 (EU twin of USA func_002A0AF8) — pose one moby keyframe vector into a
 * local scratch buffer and transform it into dst. func_002A4908 (EU twin of USA
 * func_002A4D60) fills the scratch (input word = arg1); the posed vector at scratch+0x30
 * is scaled by (obj+0x2C)/1024 into dst (Vec4ScaleVu0), then the moby's rotation
 * (func_00283958 over obj+0xC0) and translation (Vec4AddVu0 obj+0x10) are applied.
 * func_00283958 is the EU (-0xF0) twin of USA func_00283A48. Callee func_002A4908
 * UNCONFIRMED (named by shape). Faithful TARGET_NATIVE coverage arm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0680);
#else
void func_002A0680(void *obj, s32 arg1, void *dst) {
    u8 *o = (u8 *)obj;
    u8 buf[0x80];
    f32 scale = *(f32 *)(o + 0x2C) * (1.0f / 1024.0f);

    *(s32 *)(buf + 0x40) = arg1;
    func_002A4908(obj, 1, buf + 0x40, buf);
    Vec4ScaleVu0(dst, scale, buf + 0x30);
    func_00283958(dst, dst, o + 0xC0);
    Vec4AddVu0(dst, dst, o + 0x10);
}
#endif

/* func_002A0708 (EU twin of USA func_002A0B80) — pose and transform a run of `count`
 * moby keyframe vectors in place. Forwards its args to func_002A47B0 (EU twin of USA
 * func_002A4C08, handwritten VU0 helper that fills the dst run from the moby's keyframe
 * data), then for each of the count vectors at dst[i] (stride 0x10) scales by
 * (obj+0x2C)/1024 (Vec4ScaleVu0), applies the moby rotation (func_00283958 over
 * obj+0xC0) and adds the translation (Vec4AddVu0 obj+0x10). func_00283958 is the EU
 * (-0xF0) twin of USA func_00283A48; func_002A47B0 the (-0x458) twin of USA func_002A4C08.
 * Callee func_002A47B0 UNCONFIRMED (named by shape). Faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern void func_002A47B0(void *obj, s32 count, void *arg2, void *dst);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0708);
#else
void func_002A0708(void *obj, s32 count, void *arg2, void *dst) {
    u8 *o = (u8 *)obj;
    u8 *p = (u8 *)dst;
    f32 scale = *(f32 *)(o + 0x2C) * (1.0f / 1024.0f);
    s32 i;

    func_002A47B0(obj, count, arg2, dst);
    for (i = count; i > 0; i--) {
        Vec4ScaleVu0(p, scale, p);
        func_00283958(p, p, o + 0xC0);
        Vec4AddVu0(p, p, o + 0x10);
        p += 0x10;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A07A8);

/* CloseMobyDmaSegment (EU twin func_002A07B0 of USA CloseMobyDmaSegment) — close the
 * moby texture-upload DMA segment. Reserves a DMATAG qword at g_frameDmaCursor and
 * back-patches the segment's open tag (g_mobySegmentOpenTag) into a CNT tag chaining to
 * it, uploads the moby textures (UploadMobyTextures over g_vramAllocCursor) + appends
 * the default TEX0 flush, then lays two more CNT DMATAG qwords closing the chain.
 * DMATAGs are 4 words (0x10 bytes); 0x20000000 = CNT. g_frameDmaCursor is re-read after
 * the upload/flush calls (they append through it). Faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern u32  *g_frameDmaCursor;         /* per-frame DMA write pointer */
extern u32  *g_mobySegmentOpenTag;     /* moby draw-segment open tag  */
extern void *g_vramAllocCursor;        /* VRAM bump cursor            */
extern void  UploadMobyTextures(void *vramCursor);
extern void  AppendTexFlushDefaultTex0(void);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A07B0);
#else
void CloseMobyDmaSegment(void) {
    u32 *start = g_frameDmaCursor;

    g_frameDmaCursor += 4;
    g_mobySegmentOpenTag[0] = 0x20000000;
    g_mobySegmentOpenTag[1] = (u32)g_frameDmaCursor;
    g_mobySegmentOpenTag[2] = 0;
    g_mobySegmentOpenTag[3] = 0;

    UploadMobyTextures(g_vramAllocCursor);
    AppendTexFlushDefaultTex0();

    g_frameDmaCursor[0] = 0x20000000;
    g_frameDmaCursor[1] = (u32)(g_mobySegmentOpenTag + 4);
    g_frameDmaCursor[2] = 0;
    g_frameDmaCursor[3] = 0;
    g_frameDmaCursor += 4;

    start[0] = 0x20000000;
    start[3] = 0;
    start[1] = (u32)g_frameDmaCursor;
    start[2] = 0;
}
#endif

/* PatchMobyPacketTex0 (EU twin func_002A08A0 of USA PatchMobyPacketTex0) — stamp
 * texture-VRAM coordinates into every loaded moby class's GIF packets. Walks the
 * present-class-slot list at &g_mobyClassDataSizes[0xF0] (s32 slot indices, terminated
 * by a negative entry). For each slot's class header it follows the texture-binding
 * node chain at header+0x20 (nodes stride 0x10; node+0xC packs the GIF-packet-entry
 * pointer in bits 0..30 and a "has next node" flag in bit 31). Each node holds an inline
 * tex-index byte list (from node+0, terminated by 0xFF); per index it looks up
 * g_mobyTexVramTable[idx] (two s16 VRAM fields) and OR's each non-zero field into the
 * low 14 bits of the packet's +0x30 / +0x40 TEX0 words (packet stride 0x40). Leaf,
 * region-agnostic body. Faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern u32   g_mobyClassDataSizes[];   /* &[0xF0] = present-slot list */
extern void *g_mobyClassHeaders[];     /* class header ptr per slot   */
extern s16   g_mobyTexVramTable[];     /* 2 s16 VRAM fields per tex    */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A08A0);
#else
void PatchMobyPacketTex0(void) {
    s32 *classSlot = (s32 *)&g_mobyClassDataSizes[0xF0];

    if (*classSlot < 0) {
        return;
    }
    for (;;) {
        u8 *header = (u8 *)g_mobyClassHeaders[*classSlot];
        u8 *node = *(u8 **)(header + 0x20);

        for (;;) {
            s32 nodeLink = *(s32 *)(node + 0xC);

            if (*node != 0xFF) {
                u8 *packet = (u8 *)(nodeLink & 0x7FFFFFFF);
                u8 *cursor = node;

                do {
                    s16 *vram = &g_mobyTexVramTable[*cursor * 2];

                    if (vram[0] != 0) {
                        *(u32 *)(packet + 0x30) =
                            (*(u32 *)(packet + 0x30) & 0xFFFFC000) | vram[0];
                    }
                    if (vram[1] != 0) {
                        *(u32 *)(packet + 0x40) =
                            (*(u32 *)(packet + 0x40) & 0xFFFFC000) | vram[1];
                    }
                    cursor++;
                    packet += 0x40;
                } while (*cursor != 0xFF);
            }
            node += 0x10;
            if (nodeLink < 0) {
                break;
            }
        }
        classSlot++;
        if (*classSlot < 0) {
            break;
        }
    }
}
#endif

/* func_002A0978 (EU twin of USA func_002A0DF0) — recompute the moby glow segment's 2D
 * light direction from the hero. func_002A0EA8 (USA func_002A1320) fills a 2-float
 * vector from g_pHeroMoby; func_00283B08 turns it into an angle, and the sin/cos-style
 * pair func_00283A40 / func_00283A58 is scaled by 0.14 into the glow parameter block at
 * g_deferredSegment2Tag+0x10 (cos) / +0x14 (sin), with a fixed -0.99 at +0x18. The three
 * 0x283xxx callees are the EU (-0xF0) twins of USA func_00283BF8/B30/B48. Callee roles
 * UNCONFIRMED (named by shape). Faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern s32   g_deferredSegment2Tag;
extern void *g_pHeroMoby;              /* hero (Ratchet) moby */
extern void  func_002A0EA8(void *moby, f32 *outVec);  /* USA func_002A1320; defined later */
extern f32   func_00283B08(f32 a, f32 b);
extern f32   func_00283A40(f32 x);
extern f32   func_00283A58(f32 x);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0978);
#else
void func_002A0978(void) {
    f32 vec[2];
    f32 angle;
    f32 *glow = (f32 *)((u8 *)&g_deferredSegment2Tag + 0x10);

    func_002A0EA8(g_pHeroMoby, vec);
    angle = func_00283B08(vec[0], vec[1]);
    glow[0] = func_00283A40(angle) * 0.14f;
    glow[2] = -0.99f;
    glow[1] = func_00283A58(angle) * 0.14f;
}
#endif

/* CloseMobyGlowSegment (EU twin func_002A0A08) — close the deferred moby-glow draw
 * segment. If no glows were queued this frame (g_mobyGlowCount == 0) it writes just an
 * END DMATAG (0x10000000) into the segment tag at *g_deferredSegment2Tag. Otherwise it
 * reserves a CNT qword, builds the glow records (BuildMobyGlowRecords) and emits their
 * packets (EmitMobyGlowPackets over g_mobyGlowWorkBuf), then closes the chain with two
 * more CNT DMATAGs. g_frameDmaCursor is re-read after the calls. Faithful TARGET_NATIVE
 * coverage arm. */
#ifdef TARGET_NATIVE
extern s32  g_mobyGlowCount;           /* glow records queued     */
extern u8   g_mobyGlowWorkBuf[];       /* glow record work buffer */
extern void BuildMobyGlowRecords(void);
extern void EmitMobyGlowPackets(void *workBuf);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0A08);
#else
void CloseMobyGlowSegment(void) {
    u32 *tag;

    if (g_mobyGlowCount == 0) {
        tag = (u32 *)g_deferredSegment2Tag;
        tag[0] = 0x10000000;
        tag[1] = 0;
        tag[2] = 0;
        tag[3] = 0;
    } else {
        u32 *start = g_frameDmaCursor;

        g_frameDmaCursor += 4;
        tag = (u32 *)g_deferredSegment2Tag;
        tag[0] = 0x20000000;
        tag[1] = (u32)g_frameDmaCursor;
        tag[2] = 0;
        tag[3] = 0;

        BuildMobyGlowRecords();
        EmitMobyGlowPackets(g_mobyGlowWorkBuf);

        g_frameDmaCursor[0] = 0x20000000;
        g_frameDmaCursor[1] = (u32)((u32 *)g_deferredSegment2Tag + 4);
        g_frameDmaCursor[2] = 0;
        g_frameDmaCursor[3] = 0;
        g_frameDmaCursor += 4;

        start[0] = 0x20000000;
        start[3] = 0;
        start[1] = (u32)g_frameDmaCursor;
        start[2] = 0;
    }
}
#endif

/* RunSprRenderPipeline (EU twin func_002A0B40) — kick the sprite/moby render pass.
 * Flushes any pending RPC (func_0011AEA0(0)), stages the 0x800-byte DMA/GIF template
 * (D_238E80) into the render scratchpad at 0x70003800 via CopyQwords, then runs the
 * frame's render task list (RunRenderTaskList over g_renderTaskList /
 * g_renderTaskWorkBuf). CopyQwords resolves to EU 0x283410. Faithful TARGET_NATIVE arm. */
#ifdef TARGET_NATIVE
extern void  func_0011AEA0(s32 arg);
extern void  CopyQwords(void *dst, void *src, s32 len);
extern void  RunRenderTaskList(void *taskList, void *workBuf);
extern u8    D_238E80[];              /* 0x800-byte SPR render template */
extern void *g_renderTaskList;
extern void *g_renderTaskWorkBuf;
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0B40);
#else
void RunSprRenderPipeline(void) {
    func_0011AEA0(0);
    CopyQwords((void *)0x70003800, D_238E80, 0x800);
    RunRenderTaskList(g_renderTaskList, g_renderTaskWorkBuf);
}
#endif

/* Clears 0x3C0 bytes of the moby scratchpad block at 0x70003A00 to 0x40000000.
 * The empty-asm guard suppresses cc1's sibling-call (tail-jump) so the original
 * jal + frame is reproduced. USA twin: func_002A1000. */
void func_002A0B88(void) {
    FillMemory32((void *)0x70003A00, 0x40000000, 0x3C0);
    __asm__ __volatile__("");
}

/* func_002A0BB0 (EU twin of USA func_002A1028) — save the procedural-anim bounds
 * scratch (0x3C0 bytes at 0x70003A00) back to g_proceduralAnimBounds[0x280] via
 * CopyQwords (EU 0x283410). Faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern u8   g_proceduralAnimBounds[];
extern void CopyQwords(void *dst, void *src, s32 len);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0BB0);
#else
void func_002A0BB0(void) {
    CopyQwords(&g_proceduralAnimBounds[0x280], (void *)0x70003A00, 0x3C0);
}
#endif

/* func_002A0BE0 (EU twin of USA func_002A1058) — load the procedural-anim bounds
 * (g_proceduralAnimBounds[0x280]) into the scratch block at 0x70003A00 via CopyQwords
 * (same prologue-scheduling wall as func_002A0BB0). Faithful TARGET_NATIVE coverage arm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0BE0);
#else
void func_002A0BE0(void) {
    CopyQwords((void *)0x70003A00, &g_proceduralAnimBounds[0x280], 0x3C0);
}
#endif

/* BeginMobyDrawSegment (EU twin func_002A0C10) — open the per-frame moby draw segment.
 * Appends the VIF code-ref tag (D_10FFC0 / D_10FFB0), selects VU1 program 6, kicks the
 * VIF0 chain (D_100080) and appends the segment's GS reg packet (reg 0x47 = SCISSOR,
 * value 0x5360B). Then opens the DMA segment: remembers the current g_frameDmaCursor as
 * the open tag, resets the VRAM bump cursor to g_vramDynamicBase, reserves a qword,
 * points the frame-DMA scratch at g_renderTaskWorkBuf-0x10000, seeds the VU-chain cursor
 * from g_renderTaskList, and clears g_deferredSegment2Tag. Faithful TARGET_NATIVE arm. */
#ifdef TARGET_NATIVE
extern u16   D_10FFB0;                  /* VIF code-ref tag qword count */
extern u8    D_10FFC0[];                /* VIF code-ref tag template    */
extern u8    D_100080[];                /* VIF0 kick chain             */
extern s32   g_activeVu1Program;        /* uploaded VU1 microcode id   */
extern void *g_vramDynamicBase;         /* VRAM dynamic region base    */
extern void *g_mobyVuChainCursor;       /* moby VU/DMA chain cursor    */
extern void  AppendVifCodeRefTag(void *code, u32 count);
extern void  KickVif0Chain(void *chain);
extern void  AppendGsRegPacket(s32 reg, u32 data);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0C10);
#else
void BeginMobyDrawSegment(void) {
    AppendVifCodeRefTag(D_10FFC0, D_10FFB0);
    g_activeVu1Program = 6;
    KickVif0Chain(D_100080);
    AppendGsRegPacket(0x47, 0x5360B);

    g_mobySegmentOpenTag = g_frameDmaCursor;
    g_vramAllocCursor = g_vramDynamicBase;
    g_frameDmaCursor = (u32 *)((u8 *)g_frameDmaCursor + 0x10);
    ((u32 *)&g_frameDmaCursor)[1] = (u32)((u8 *)g_renderTaskWorkBuf - 0x10000);
    g_mobyVuChainCursor = g_renderTaskList;
    g_deferredSegment2Tag = 0;
}
#endif

/* func_002A0CC0 (EU twin of USA func_002A1138) — build the moby VU1 render chain for
 * one moby table. Appends the segment's GS SCISSOR reg packet (0x47 / 0x5360B), flushes
 * the pending RPC (func_0011AEA0(0)), swaps in the procedural-anim bounds scratch
 * (func_002A0BE0), extends the VU chain (BuildMobyVuChain over g_mobyVuChainCursor),
 * saves the scratch back (func_002A0BB0) and rewinds the cursor by one qword. Faithful
 * TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern void *BuildMobyVuChain(void *tableBase, void *cursor, s32 count, s32 flag);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0CC0);
#else
void func_002A0CC0(void *tableBase, s32 count) {
    AppendGsRegPacket(0x47, 0x5360B);
    func_0011AEA0(0);
    func_002A0BE0();
    g_mobyVuChainCursor = BuildMobyVuChain(tableBase, g_mobyVuChainCursor, count, 0);
    func_002A0BB0();
    g_mobyVuChainCursor = (void *)((u8 *)g_mobyVuChainCursor - 0x10);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0D40);

/* RenderMobys (EU twin func_002A0D80) — per-frame moby render driver. Opens the draw
 * segment (BeginMobyDrawSegment), clears the anim-bounds scratch (func_002A0B88, EU twin
 * of USA func_002A1000), builds the moby VU1 chain over the whole table
 * (BuildMobyVuChain(g_mobyTableBase, cursor, -1, 1)); if the chain overran the frame-DMA
 * budget (the limit at g_frameDmaCursor[+0x4] fell below the write cursor) it logs the
 * "mobys dropped" overflow string, then finishes the chain (FinishMobyRenderChain, EU
 * twin func_002A0D40). Faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern void *g_mobyTableBase;        /* moby entity array base (stride 0x100) */
extern char  D_1A9E48[];             /* "N mobys dropped" overflow log string */
extern s32   DebugPrintStub(void *msg);
extern void  FinishMobyRenderChain(void);   /* EU twin func_002A0D40 (still INCLUDE_ASM) */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0D80);
#else
void RenderMobys(void) {
    BeginMobyDrawSegment();
    func_002A0B88();
    g_mobyVuChainCursor = BuildMobyVuChain(g_mobyTableBase, g_mobyVuChainCursor, -1, 1);
    if (*(s32 *)((u8 *)&g_frameDmaCursor + 0x4) < (s32)g_frameDmaCursor) {
        DebugPrintStub(D_1A9E48);   /* VU chain budget exceeded — mobys dropped */
    }
    FinishMobyRenderChain();
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0DF0);

/* func_002A0E28 (EU twin of USA func_002A12A0) — packs a 4-byte tuple (hi,b1,b2,b3)
 * into the 64-bit field at +0x38 of a moby: the high 32 bits hold hi, the low 32 bits
 * pack b1 | b2<<8 | b3<<16. Faithful TARGET_NATIVE coverage arm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0E28);
#else
void func_002A0E28(s64 *moby, s64 hi, s64 b1, s64 b2, s64 b3) {
    moby[7] = (((hi << 32) | b1) | (b2 << 8)) | (b3 << 16);
}
#endif

/* func_002A0E48 (EU twin of USA func_002A12C0) — writes the high 32 bits of the moby's
 * 64-bit field at +0x38 as a packed 3-byte tuple (b0 in bits 32..39, b1 in 40..47, b2 in
 * 48..55), preserving the existing low 32 bits. Inverse of func_002A0E78. Faithful
 * TARGET_NATIVE coverage arm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0E48);
#else
void func_002A0E48(u64 *moby, u64 b0, u64 b1, u64 b2) {
    u64 lo = (u32)moby[7];
    moby[7] = lo | (b0 << 32) | (b1 << 40) | (b2 << 48);
}
#endif

/* func_002A0E78 (EU twin of USA func_002A12F0) — unpacks 3 bytes out of the high half of
 * the +0x38 field of a moby into three separate s32 out-params (inverse of the high-half
 * writer func_002A0E48). Faithful TARGET_NATIVE coverage arm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0E78);
#else
void func_002A0E78(u64 *moby, u32 *out0, u32 *out1, u32 *out2) {
    u64 packed = moby[7];
    *out0 = (packed >> 32) & 0xFF;
    *out1 = (packed >> 40) & 0xFF;
    *out2 = (packed >> 48) & 0xFF;
}
#endif

/* func_002A0EA8 (EU twin of USA func_002A1320)(obj, out): resolve the object's packed
 * directional-light field at obj+0x38 (bytes idx0/idx1/blend, written by
 * func_002A0E28/func_002A0E48) into an interpolated light vector in out. Looks up the
 * 0x40-stride g_dirLightMatrices, using the vec4 row at +0x10 of each entry: when blend
 * == 0 it copies matrix idx0's row, otherwise it lerps matrix idx0 -> idx1 by
 * t = (blend*16)/4096 on x/y/z only (the w lane stays matrix idx0's, since the VU0 vmadd
 * is .xyz). Matches the batch-3 forward decl `void func_002A0EA8(void*, f32*)`. Faithful
 * TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern u8 g_dirLightMatrices[];   /* 0x40-stride dir-light matrices */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0EA8);
#else
void func_002A0EA8(void *obj, f32 *out) {
    u64 v = *(u64 *)((u8 *)obj + 0x38);
    u32 idx0  = (u32)v & 0xFF;
    u32 idx1  = ((u32)v >> 8) & 0xFF;
    u32 blend = ((u32)v >> 16) & 0xFF;
    f32 *m0 = (f32 *)(g_dirLightMatrices + idx0 * 0x40 + 0x10);

    if (blend == 0) {
        out[0] = m0[0];
        out[1] = m0[1];
        out[2] = m0[2];
        out[3] = m0[3];
    } else {
        f32 *m1 = (f32 *)(g_dirLightMatrices + idx1 * 0x40 + 0x10);
        f32 t  = (blend << 4) * (1.0f / 4096.0f);
        f32 it = 1.0f - t;
        out[0] = m0[0] * it + m1[0] * t;
        out[1] = m0[1] * it + m1[1] * t;
        out[2] = m0[2] * it + m1[2] * t;
        out[3] = m0[3];   /* w lane unchanged from m0 (vmadd is .xyz only) */
    }
}
#endif

/* func_002A0F18 (EU twin of USA func_002A1390)(a, b): signed gap between two bounding
 * spheres, scaled by 1/1024 — the centre distance |a.xyz - b.xyz| minus the sum of radii
 * (a.w + b.w), times 1/1024. Negative when the spheres overlap. Faithful TARGET_NATIVE
 * coverage arm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0F18);
#else
f32 func_002A0F18(f32 *a, f32 *b) {
    f32 dx = a[0] - b[0];
    f32 dy = a[1] - b[1];
    f32 dz = a[2] - b[2];
    f32 dist = __builtin_sqrtf(dx * dx + dy * dy + dz * dz);
    return (dist - (a[3] + b[3])) * (1.0f / 1024.0f);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A0F68);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1280);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1408);

/* ReleaseMobyGridBlockBits (EU twin func_002A1568)(start, count): clear `count`
 * allocation bits starting at bit index `start` in g_mobyGridBlockBitmap (byte start>>3,
 * bit start&7). The original asserts on double-free — clearing a bit that was already 0
 * executes an unconditional `teq` trap; the #else models that error path as an early stop
 * (unreachable in correct use). Faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern u8 g_mobyGridBlockBitmap[];   /* bit-per-block allocation bitmap */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1568);
#else
void ReleaseMobyGridBlockBits(s32 start, s32 count) {
    for (;;) {
        u8 *byte = g_mobyGridBlockBitmap + (start >> 3);
        u8 old = *byte;
        u8 cleared = (u8)(old & ~(1 << (start & 7)));
        start++;
        count--;
        if (cleared == old) {
            /* bit already clear — original does an unconditional teq trap
             * (double-free assert); modelled here as a stop. */
            return;
        }
        *byte = cleared;
        if (count <= 0) {
            return;
        }
    }
}
#endif

/* AllocMobyGridBlockBits (EU twin func_002A15C8)(width): allocate a free run of `width`
 * consecutive bits in g_mobyGridBlockBitmap and return its global bit index. Scans 32-bit
 * words; within a non-full word it slides an aligned width-bit mask (stepping by width)
 * until the masked bits are all clear — the mask shifting fully out of the low 32 bits
 * yields position 0x20 (no fit), advancing to the next word. Sets the run and returns
 * word*0x20 + position. `width` is a power of two dividing 32. Faithful TARGET_NATIVE arm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A15C8);
#else
s32 AllocMobyGridBlockBits(s32 width) {
    u32 *p = (u32 *)g_mobyGridBlockBitmap;
    u32 mask0 = (1u << width) - 1;
    s32 base = -0x20;

    for (;;) {
        u32 word = *p++;
        base += 0x20;
        if (word == 0xFFFFFFFF) {
            continue;   /* full word */
        }
        {
            u64 mask = mask0;
            s32 pos = 0;
            while (word & (u32)mask) {
                mask <<= width;
                pos += width;
            }
            if (pos == 0x20) {
                continue;   /* no run fits in this word */
            }
            p[-1] = word | (u32)mask;
            return base + pos;
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1648);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1AC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1B10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1D58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A1E70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A2118);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A2E30);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A3B08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A47B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A4908);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A4A70);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A54D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A5780);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A5860);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A7038);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A7174);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", PickLowAmmoWeaponForDrop);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A74D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/19FC78", func_002A7650);

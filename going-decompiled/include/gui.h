#ifndef GUI_H
#define GUI_H

#include "common.h"

/* Going Commando in-game GUI / HUD widget tree (gui unit, .text 0x336xxx-0x34Fxxx).
 *
 * The GUI is a classic retained-mode widget tree built on two recurring object
 * kinds, both recovered from the USA v2.00 (SCUS_972.68) GUI cluster. The layout
 * is region-agnostic - the EU v1.00 twins share it byte-for-byte modulo the
 * usual small .text offset.
 *
 *   GuiElement  - a LEAF drawable node (sprite / text / glyph / list-row). Has a
 *                 vtable at +0x30, a small set of pooled vec4 records, and a few
 *                 typed extension fields. Base size 0x4C; the text variant
 *                 extends it to 0x58.
 *   GuiWidget   - a CONTAINER / screen object that OWNS elements. It does NOT use
 *                 a uniform struct across all screens: every Gui*ScreenInit ctor
 *                 lays out its own children INLINE at fixed byte offsets (the
 *                 elements are embedded sub-objects, not pointers). What IS shared
 *                 and stable is the GuiMenuList widget (the vertical text-menu
 *                 list driven by GuiMenuList* accessors) - that is the GuiWidget
 *                 modeled in full below. The generic screen header prefix and the
 *                 0x88-stride child-widget activation array are documented as
 *                 separate notes; they are screen-specific, not one struct.
 *
 * RELATIONSHIP: GuiWidget (container/screen) is the parent; it embeds/owns one or
 * more GuiElement leaves and draws them by direct call to their *ElementDraw
 * function (the per-screen draw method calls GuiSpriteElementDraw/GuiTextElement-
 * Draw on each child element directly - NOT through the element vtable in the
 * common case; the vtable +0xC draw slot is the generic fallback path).
 *
 * ---- THE GuiElement VTABLE (+0x30) ----------------------------------------
 * Installed by GuiElementInstallBaseVtable (base table g_GuiElementVtable
 * @0x1ADA58) and then overwritten by the typed ctors with a variant table:
 *   base   @0x1ADA58  draw slot -> 0x121B18 (tail-jump no-op stub)
 *   TypeB  @0x1ADA38  draw slot -> GuiSpriteElementDraw (0x337128)   [sprite]
 *   TypeC  @0x1AD9F8  draw slot -> GuiTextElementDraw   (0x3378A0)   [text]
 *   ListRow@0x1AD9D8  draw slot -> 0x337350 (DrawFlatRect bar)       [list row]
 * VTABLE SLOT LAYOUT (offsets into the vtable, CONFIRMED from the 4 tables):
 *   +0x0  reserved/0      +0x4  reserved/0      +0x8  reserved/0
 *   +0xC  DRAW virtual    fn(GuiElement*)                            CONFIRMED
 *   +0x10 reserved/0
 *   +0x14 DTOR / cleanup  fn(GuiElement*, flags) (frees pooled vecs) CONFIRMED
 *   +0x18 reserved/0  (base table is followed by the RTTI-ish string
 *         "gui/memoryManagement.cpp" - the real table is only ~0x1C bytes)
 *   ListRow/TypeC also populate +0x2C, +0x30 with extra row/text virtuals.
 *
 * Confidence: offsets/sizes are CONFIRMED (driven from the ctors' inline-child
 * strides and the exact store offsets in the accessors/draw fns). Field MEANING
 * is CONFIRMED where a named accessor + the asm agree, PROBABLE where the offset
 * is seen but the role is inferred, UNCONFIRMED where only the offset is known.
 * Gaps are explicit padding - genuinely un-analyzed bytes, NOT asserted layout.
 */

/* ======================================================================== *
 *  GuiElement - leaf drawable node. Base 0x4C, text variant 0x58.          *
 * ======================================================================== *
 *
 * EVIDENCE BASE (every offset below traced in Ghidra/asm):
 *   GuiElementBaseInit       0x336DD0  base ctor: allocs the 5 pooled vec4s
 *                                       at [0]+0x00,[1]+0x04,[2]+0x08,[3]+0x0C,
 *                                       [4]+0x10; sets +0x28 name, +0x2C pool
 *   GuiElementInit           0x336FE8  allocs +0x34,+0x38 vecs, scale=(1,1,1)
 *   GuiElementInstallBaseVtable 0x336DB8  writes +0x30 vtable
 *   GuiElementGetScaleVec    0x336C20  reads +0x04
 *   GuiElementGetColor       0x336C28  reads +0x0C
 *   GuiElementSetPos         0x336C40  writes *(+0x00)[0..3]
 *   GuiElementSetScale       0x336D90  writes *(+0x04)[0..3]
 *   GuiElementSetVisible     0x336C70  writes *(+0x10)[0] = 1.0/0.0
 *   GuiElementIsVisible      0x336C90  reads  *(+0x10)[0] > 0
 *   GuiElementShareScaleVec  0x336CB8  rebinds +0x04, sets +0x18 share flag
 *   GuiElementSetAlpha       0x3371C8  writes *(+0x38)[0]   (text/sprite alpha)
 *   GuiElementSetGlyph/Text  0x337198/0x337860  writes +0x40
 *   GuiElementSetTextFlag    0x337858  writes +0x44
 *   GuiSpriteSetTexture/Get  0x337728/0x3375C0  +0x34 (sprite uv vec)
 *   GuiSpriteElementInit     0x337518  sprite: +0x34 uv vec, +0x38 color, +0x40 tex
 *   GuiTextElementInit       0x3377B0  text:   +0x34..+0x54 (see below)
 *   GuiTextElementDraw/Measure 0x3378A0/0x337868  read text fields
 *   GuiListElementInit       0x337210  list:   +0x3C cap, +0x40 count, +0x44 sentinel
 *   FUN_00336F00 (dtor slot) reads +0x2C pool, +0x14..+0x20 share flags
 *
 * NOTE on the +0x34/+0x38/+0x40 OVERLAP: these bytes are a UNION reused per
 * element subtype. For a BASE element (GuiElementInit) +0x34 and +0x38 are
 * POINTERS to pooled vec4s (texture-uv vec, alpha vec). For a TEXT element they
 * are a direct string ptr (+0x34) and a small-int (+0x38). For a LIST element
 * +0x34 is a backing ptr and +0x3C/+0x40/+0x44 are capacity/count/sentinel. The
 * struct below models the BASE/SPRITE form and documents the text/list overlays
 * in comments; bind the right view per subtype.
 */
typedef struct GuiElement {                 /* === base 0x4C, text variant 0x58 === */
    f32 *posVec;        /* +0x00 ptr to pooled vec4 = position (x,y,z,w). GuiElementSetPos /
                           GuiSpriteElementDraw read [0],[1].                       CONFIRMED */
    f32 *scaleVec;      /* +0x04 ptr to pooled vec4 = scale/size (default (1,1,1)). GetScaleVec /
                           SetScale / GuiShareScaleVec (may be re-pointed to a shared vec). CONFIRMED */
    f32 *vec2;          /* +0x08 ptr to pooled vec4, 3rd transform vec (border/extent in some
                           draws; alloc'd as element[2] by GuiElementBaseInit).      PROBABLE */
    f32 *colorVec;      /* +0x0C ptr to pooled vec4 = packed RGBA color (GuiElementGetColor;
                           screens write 0x80F0F0F0 etc through it).                 CONFIRMED */
    f32 *visVec;        /* +0x10 ptr to pooled vec4 = visibility/alpha gate; [0]=1.0 visible /
                           0.0 hidden (GuiElementSetVisible / GuiElementIsVisible).  CONFIRMED */
    u32  shareFlag14;   /* +0x14 ownership/share latch for vec2 (dtor frees only if 0)  PROBABLE */
    u32  shareFlag18;   /* +0x18 ownership/share latch for scaleVec, set by ShareScaleVec CONFIRMED */
    u32  shareFlag1C;   /* +0x1C ownership/share latch (vec slot)                     PROBABLE */
    u32  shareFlag20;   /* +0x20 ownership/share latch (vec slot)                     PROBABLE */
    u8   pad24[4];      /* +0x24 un-analyzed                                        UNCONFIRMED */
    const char *name;   /* +0x28 element name string (param to GuiElementBaseInit).   CONFIRMED */
    void *pool;         /* +0x2C GUI pool/arena this element allocs its vecs from.    CONFIRMED */
    void **vtable;      /* +0x30 vtable ptr (see header). +0xC=draw, +0x14=dtor.      CONFIRMED */
    /* ---- subtype-specific extension fields (UNION region +0x34.. ) ---------- */
    void *ext34;        /* +0x34 SPRITE: ptr to pooled uv vec4 (GuiSpriteSetTexture writes [0],[1]).
                           TEXT: direct font/string ptr (GuiTextElementInit = &DAT_00263B10).
                           LIST: backing ptr (GuiListElementInit param_3).           CONFIRMED */
    void *ext38;        /* +0x38 SPRITE: ptr to alpha vec4 (GuiElementSetAlpha writes [0]).
                           TEXT: small int (init 1).                                 CONFIRMED */
    u32  ext3C;         /* +0x3C LIST: row capacity (GuiListElementInit). list-row draw reads as
                           clamp width. Other subtypes: unused/0.                    PROBABLE */
    u32  textOrCount40; /* +0x40 TEXT/GLYPH: text/glyph handle (GuiElementSetText/SetGlyph).
                           LIST: item count (GuiListSetItemCount, init 100).         CONFIRMED */
    u32  flag44;        /* +0x44 TEXT: horiz align/flag (GuiElementSetTextFlag; 0=L,1=C,2=R read by
                           GuiTextElementDraw). LIST: 0x80000000 sentinel.           CONFIRMED */
    u32  field48;       /* +0x48 TEXT: auto-fit enable flag (GuiTextElementDraw elem[0x12]); cleared
                           by GuiElementInit.                                        PROBABLE */
    /* ---- TEXT-variant tail (present only when sizeof==0x58) ------------------ */
    u32  textField4C;   /* +0x4C TEXT: glyph/atlas param (GuiTextElementInit=0x200; Draw elem[0x13]
                           passed to ComputeTextAutoFitScale).                       PROBABLE */
    f32  textField50;   /* +0x50 TEXT: target/base auto-fit scale (init 0.7=0x3F333333; Draw
                           elem[0x14]).                                              PROBABLE */
    u32  textField54;   /* +0x54 TEXT: inline-color-code enable flag (Draw elem[0x15]: nonzero ->
                           EnableInlineColorCodes).                                  PROBABLE */
} GuiElement;                               /* base sizeof 0x4C; text variant 0x58 */

/* ======================================================================== *
 *  GuiWidget (GuiMenuList variant) - the vertical text-menu list container *
 * ======================================================================== *
 *
 * This is the cleanest, fully-mapped GuiWidget: a container that owns a row
 * array of menu entries and reuses ONE embedded GuiElement (at its base, fields
 * +0x00..) as the per-row text-render scratch. Driven by the GuiMenuList*
 * accessor family and rendered by GuiMenuListDraw.
 *
 * EVIDENCE BASE (all CONFIRMED, traced in Ghidra/asm):
 *   GuiMenuListDraw                 0x348E70  reads the full struct (per-row render)
 *   GuiMenuListSetRows              0x348DA0  +0x68 rowArray, counts -> +0xC0
 *   GuiMenuListHandleInput          0x348CB8  +0x60 sel, +0x6C enabled, +0xC0 count
 *   GuiMenuListSetOrigin            0x348E28  *(+0x5C) origin vec
 *   GuiMenuListSetRowSpacing        0x348E10  +0xB8 base, +0xBC step
 *   GuiMenuListGetRowCount          0x348E68  +0xC0
 *   GuiMenuListGetSelectedRow       0x348E60  +0x60
 *   GuiMenuListSetSelectedRow       0x348E50  +0x60
 *   GuiMenuListSetUnderlineRow      0x348E58  +0xCC
 *   GuiMenuListSetSelectedUnderlineFlag 0x348E20 +0xC8
 *
 * NOTE: offsets +0x00..+0x54 are the embedded GuiElement scratch (the list draw
 * calls GuiElementSetText/SetScale/GetColor on `self` itself). The widget-
 * specific fields begin at +0x5C.
 */
typedef struct GuiWidget {                  /* === GuiMenuList container; >= 0xD0 === */
    GuiElement scratchElem; /* +0x00 embedded GuiElement reused as the per-row text-render scratch
                               (GuiMenuListDraw calls GuiElementSetText/SetScale/GetColor on
                               `self`).                                              CONFIRMED */
    u8   pad58[4];      /* +0x58 align/un-analyzed                                  UNCONFIRMED */
    f32 *originVec;     /* +0x5C ptr to vec4: [0]=origin X, [1]=origin Y (row layout). CONFIRMED */
    s32  selectedRow;   /* +0x60 highlighted/selected row index (uses +0xAC color).  CONFIRMED */
    u8   pad64[4];      /* +0x64 un-analyzed                                        UNCONFIRMED */
    f32 *rowArray;      /* +0x68 ptr to row-entry array, stride 0x14 (5 floats): entry[0]=textElem
                           handle, entry[1]=stringId/textPtr. Counted by SetRows.    CONFIRMED */
    s32  rowEnabled[16];/* +0x6C int[] per-row enabled flag; 0 -> disabled color +0xB4 and skipped
                           in input nav. Indexed [i] up to rowCount (cap 16).        CONFIRMED.
                           Spans +0x6C..+0xAB (0x40 bytes) - header resumes at +0xAC below.        */
    u32  selectedColor; /* +0xAC RGBA for the selected row (also highlight-rect fill). CONFIRMED */
    u32  normalColor;   /* +0xB0 RGBA enabled, non-selected rows.                     CONFIRMED */
    u32  disabledColor; /* +0xB4 RGBA disabled rows (rowEnabled==0).                  CONFIRMED */
    s32  rowYBase;      /* +0xB8 first row Y (GuiMenuListSetRowSpacing).              CONFIRMED */
    s32  rowYStep;      /* +0xBC per-row Y stride.                                    CONFIRMED */
    s32  rowCount;      /* +0xC0 computed by SetRows; loop bound in Draw/Input.       CONFIRMED */
    s32  useRawTextFlag;/* +0xC4 0 -> GetLocalizedString(id); else raw ptr/id.        CONFIRMED */
    s32  drawSelectedUnderlineFlag; /* +0xC8                                          CONFIRMED */
    s32  underlineRowIndex; /* +0xCC row that gets the first highlight rect.          CONFIRMED */
} GuiWidget;                                /* GuiMenuList; sizeof 0xD0 */

/* Canonical absolute-offset view of the GuiMenuList header (use these when the
 * struct's rowEnabled[] span makes field access ambiguous). All CONFIRMED. */
#define GUIMENULIST_ORIGIN_VEC      0x5C  /* f32* origin (x,y)            */
#define GUIMENULIST_SELECTED_ROW    0x60  /* s32 selected row index       */
#define GUIMENULIST_ROW_ARRAY       0x68  /* f32* row entries, stride 0x14*/
#define GUIMENULIST_ROW_ENABLED     0x6C  /* s32[] per-row enabled flag   */
#define GUIMENULIST_SELECTED_COLOR  0xAC  /* u32 RGBA selected row        */
#define GUIMENULIST_NORMAL_COLOR    0xB0  /* u32 RGBA enabled row         */
#define GUIMENULIST_DISABLED_COLOR  0xB4  /* u32 RGBA disabled row        */
#define GUIMENULIST_ROW_Y_BASE      0xB8  /* s32 first row Y              */
#define GUIMENULIST_ROW_Y_STEP      0xBC  /* s32 per-row Y stride         */
#define GUIMENULIST_ROW_COUNT       0xC0  /* s32 row count                */
#define GUIMENULIST_USE_RAW_TEXT    0xC4  /* s32 raw-vs-localized flag    */
#define GUIMENULIST_DRAW_SEL_ULINE  0xC8  /* s32 draw selected underline  */
#define GUIMENULIST_UNDERLINE_ROW   0xCC  /* s32 static underline row     */
#define GUIMENULIST_ROW_STRIDE      0x14  /* row-entry stride (5 floats)  */

/* ---- Screen/container header prefix (screen-SPECIFIC, NOT one struct) -----
 * The Gui*ScreenInit ctors (e.g. GuiScreenWithPlanetNameInit @0x3493E0,
 * GuiIconScreenInit @0x342130) each embed their children inline. A typical
 * screen header seen across several ctors:
 *   +0x00..  the screen's own base GuiElement (title element)
 *   +0x0C    u32  dirty/show flag (GuiScreenSetEventAndReveal sets 1)        PROBABLE
 *   inline child GuiElements at fixed strides (0x4C base / 0x58 text), e.g.
 *     +0x4C, +0x98, +0xE4, +0x130, +0x17C, +0x1C8 (stride 0x4C)             CONFIRMED
 *   per-screen ptr/vec fields near the tail (e.g. +0x228/+0x22C/+0x230 in
 *     GuiIconScreenInit; +0x4A4/+0x4A8/+0x4AC/+0x4B0 in the planet screen)   PROBABLE
 *   GuiScreenSetEventAndReveal screen fields:
 *     +0x3F4 u32 eventId, +0x400 u32 pendingFlag, +0x0C dirty,
 *     +0x1D4/+0x25C/+0x2E4/+0x36C child-widget records (0x88 stride),
 *     each activated via the child-activate helper FUN_0034A370:
 *       child+0x18 alpha, child+0x1C targetState, child+0x28 initFlag.       CONFIRMED
 * Because the screen body is per-ctor, no single C struct captures all of them;
 * bind each screen by its ctor's offsets. The two RECURRING reusable types are
 * GuiElement and the GuiMenuList GuiWidget above.
 *
 * The GuiManager SINGLETON (g_pGuiManager @0x1A8D04, sizeof 0x3FB20) is NOT a
 * GuiWidget - it is the root that hosts callback tables at +0x3F9D4.. and the
 * sub-managers at +0x36F28 / +0x3CEA0 and the font atlas at +0x8710.
 */

/* === Compile-time layout verification (ILP32 targets only) ================
 * Active only under a 4-byte-pointer compile (PS2 EE + native -m32). ee-gcc 2.9
 * predates __SIZEOF_POINTER__ so this is inert in the matching build; the host-64
 * linter is skipped too. */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(__builtin_offsetof(GuiElement, scaleVec)    == 0x04, "scaleVec");
_Static_assert(__builtin_offsetof(GuiElement, colorVec)    == 0x0C, "colorVec");
_Static_assert(__builtin_offsetof(GuiElement, visVec)      == 0x10, "visVec");
_Static_assert(__builtin_offsetof(GuiElement, name)        == 0x28, "name");
_Static_assert(__builtin_offsetof(GuiElement, pool)        == 0x2C, "pool");
_Static_assert(__builtin_offsetof(GuiElement, vtable)      == 0x30, "vtable");
_Static_assert(__builtin_offsetof(GuiElement, ext34)       == 0x34, "ext34");
_Static_assert(__builtin_offsetof(GuiElement, ext38)       == 0x38, "ext38");
_Static_assert(__builtin_offsetof(GuiElement, textOrCount40) == 0x40, "textOrCount40");
_Static_assert(__builtin_offsetof(GuiElement, flag44)      == 0x44, "flag44");
_Static_assert(__builtin_offsetof(GuiElement, textField4C) == 0x4C, "textField4C");
_Static_assert(__builtin_offsetof(GuiElement, textField54) == 0x54, "textField54");
_Static_assert(sizeof(GuiElement) == 0x58, "GuiElement text variant == 0x58");

_Static_assert(__builtin_offsetof(GuiWidget, originVec)    == 0x5C, "originVec");
_Static_assert(__builtin_offsetof(GuiWidget, selectedRow)  == 0x60, "selectedRow");
_Static_assert(__builtin_offsetof(GuiWidget, rowArray)     == 0x68, "rowArray");
_Static_assert(__builtin_offsetof(GuiWidget, rowEnabled)   == 0x6C, "rowEnabled");
#endif

/* Shared 2D UI draw primitives (recovered signatures; definitions live in
 * src/usa/text/188858.c / 178E88.c). Include this header in a menu-draw TU to
 * write faithful #else bodies without guessing arg boundaries (#67 unblock).
 * Byte-neutral for the matching build (those arms stay INCLUDE_ASM). */
void func_002904B0(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 mode);        /* box/line quad */
void func_0028FFF0(s32 iconIndex, s32 x0, s32 y0, s32 x1, s32 y1,
                   s32 u0, s32 v0, s32 u1, s32 v1, s32 alpha);                  /* icon/sprite */
s32  func_0028EDF0(s32 name, s32 level);                                       /* localized name+level */
void func_0027FBA8(s32 a, s32 b, s32 c, s32 d, s32 e);                         /* text/glyph draw */
void func_00280090(s32 a, s32 b, s32 c, s32 d, s32 e);                         /* right-justified fixed-font string */

#endif /* GUI_H */

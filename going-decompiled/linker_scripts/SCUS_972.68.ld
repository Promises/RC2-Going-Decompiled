SECTIONS
{
    __romPos = 0;
    _gp = 0x1AEFF0;
    cod_ROM_START = __romPos;
    cod_VRAM = ADDR(.cod);
    .cod 0x100080 : AT(cod_ROM_START)
    {
        FILL(0x00000000);
        cod_TEXT_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/cod/000000.o(.text*);
        going-decompiled/build/usa/going-decompiled/src/usa/cod/015180.o(.text*);
        . = ALIGN(., 16);
        cod_TEXT_END = .;
        cod_TEXT_SIZE = ABSOLUTE(cod_TEXT_END - cod_TEXT_START);
        cod_DATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/cod/000000.o(.data*);
        going-decompiled/build/usa/going-decompiled/src/usa/cod/015180.o(.data*);
        going-decompiled/build/usa/going-decompiled/asm/usa/data/cod/033B00.data.o(.data*);
        . = ALIGN(., 16);
        cod_DATA_END = .;
        cod_DATA_SIZE = ABSOLUTE(cod_DATA_END - cod_DATA_START);
        cod_RODATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/cod/03A180.rodata.o(.rodata*);
        . = ALIGN(., 16);
        cod_RODATA_END = .;
        cod_RODATA_SIZE = ABSOLUTE(cod_RODATA_END - cod_RODATA_START);
    }
    cod_bss_VRAM = ADDR(.cod_bss);
    .cod_bss (NOLOAD) :
    {
        FILL(0x00000000);
        cod_BSS_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/cod/000000.o(.bss COMMON .scommon);
        going-decompiled/build/usa/going-decompiled/src/usa/cod/015180.o(.bss COMMON .scommon);
        going-decompiled/build/usa/going-decompiled/asm/usa/data/cod/0013C080.bss.o(.bss COMMON .scommon);
        . = ALIGN(., 16);
        cod_BSS_END = .;
        cod_BSS_SIZE = ABSOLUTE(cod_BSS_END - cod_BSS_START);
    }
    __romPos += SIZEOF(.cod);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    cod_ROM_END = __romPos;
    cod_VRAM_END = .;

    core_lit_ROM_START = __romPos;
    core_lit_VRAM = ADDR(.core_lit);
    .core_lit 0x1A7480 : AT(core_lit_ROM_START)
    {
        FILL(0x00000000);
        core_lit_DATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/core.lit/0A7400.data.o(.data*);
        . = ALIGN(., 16);
        core_lit_DATA_END = .;
        core_lit_DATA_SIZE = ABSOLUTE(core_lit_DATA_END - core_lit_DATA_START);
    }
    __romPos += SIZEOF(.core_lit);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    core_lit_ROM_END = __romPos;
    core_lit_VRAM_END = .;

    lit_ROM_START = __romPos;
    lit_VRAM = ADDR(.lit);
    .lit 0x1A7C80 : AT(lit_ROM_START)
    {
        FILL(0x00000000);
        lit_DATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/lit/0A7C00.data.o(.data*);
        . = ALIGN(., 16);
        lit_DATA_END = .;
        lit_DATA_SIZE = ABSOLUTE(lit_DATA_END - lit_DATA_START);
    }
    __romPos += SIZEOF(.lit);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    lit_ROM_END = __romPos;
    lit_VRAM_END = .;

    bss_ROM_START = __romPos;
    bss_bss_VRAM = ADDR(.bss_bss);
    .bss_bss 0x1B1300 (NOLOAD) :
    {
        FILL(0x00000000);
        bss_BSS_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/bss/001B1300.bss.o(.bss COMMON .scommon);
        . = ALIGN(., 16);
        bss_BSS_END = .;
        bss_BSS_SIZE = ABSOLUTE(bss_BSS_END - bss_BSS_START);
    }
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    bss_ROM_END = __romPos;
    bss_VRAM_END = .;

    data_ROM_START = __romPos;
    data_VRAM = ADDR(.data);
    .data 0x238C00 : AT(data_ROM_START)
    {
        FILL(0x00000000);
        data_DATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/data/138B80.data.o(.data*);
        . = ALIGN(., 16);
        data_DATA_END = .;
        data_DATA_SIZE = ABSOLUTE(data_DATA_END - data_DATA_START);
    }
    __romPos += SIZEOF(.data);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    data_ROM_END = __romPos;
    data_VRAM_END = .;

    lvl_vtbl_ROM_START = __romPos;
    lvl_vtbl_VRAM = ADDR(.lvl_vtbl);
    .lvl_vtbl 0x26E880 : AT(lvl_vtbl_ROM_START)
    {
        FILL(0x00000000);
        lvl_vtbl_RODATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/lvl.vtbl/16E800.rodata.o(.rodata*);
        . = ALIGN(., 16);
        lvl_vtbl_RODATA_END = .;
        lvl_vtbl_RODATA_SIZE = ABSOLUTE(lvl_vtbl_RODATA_END - lvl_vtbl_RODATA_START);
    }
    __romPos += SIZEOF(.lvl_vtbl);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    lvl_vtbl_ROM_END = __romPos;
    lvl_vtbl_VRAM_END = .;

    lvl_camvtbl_ROM_START = __romPos;
    lvl_camvtbl_VRAM = ADDR(.lvl_camvtbl);
    .lvl_camvtbl 0x26E900 : AT(lvl_camvtbl_ROM_START)
    {
        FILL(0x00000000);
        lvl_camvtbl_RODATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/lvl.camvtbl/16E880.rodata.o(.rodata*);
        . = ALIGN(., 16);
        lvl_camvtbl_RODATA_END = .;
        lvl_camvtbl_RODATA_SIZE = ABSOLUTE(lvl_camvtbl_RODATA_END - lvl_camvtbl_RODATA_START);
    }
    __romPos += SIZEOF(.lvl_camvtbl);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    lvl_camvtbl_ROM_END = __romPos;
    lvl_camvtbl_VRAM_END = .;

    lvl_sndvtbl_ROM_START = __romPos;
    lvl_sndvtbl_VRAM = ADDR(.lvl_sndvtbl);
    .lvl_sndvtbl 0x26E980 : AT(lvl_sndvtbl_ROM_START)
    {
        FILL(0x00000000);
        lvl_sndvtbl_RODATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/lvl.sndvtbl/16E900.rodata.o(.rodata*);
        . = ALIGN(., 16);
        lvl_sndvtbl_RODATA_END = .;
        lvl_sndvtbl_RODATA_SIZE = ABSOLUTE(lvl_sndvtbl_RODATA_END - lvl_sndvtbl_RODATA_START);
    }
    __romPos += SIZEOF(.lvl_sndvtbl);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    lvl_sndvtbl_ROM_END = __romPos;
    lvl_sndvtbl_VRAM_END = .;

    text_ROM_START = __romPos;
    text_VRAM = ADDR(.text);
    .text 0x26EA00 : AT(text_ROM_START)
    {
        FILL(0x00000000);
        text_TEXT_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/text/16E980.o(.text*);
        . = ALIGN(., 16);
        text_TEXT_END = .;
        text_TEXT_SIZE = ABSOLUTE(text_TEXT_END - text_TEXT_START);
        text_DATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/text/16E980.o(.data*);
        . = ALIGN(., 16);
        text_DATA_END = .;
        text_DATA_SIZE = ABSOLUTE(text_DATA_END - text_DATA_START);
    }
    text_bss_VRAM = ADDR(.text_bss);
    .text_bss (NOLOAD) :
    {
        FILL(0x00000000);
        text_BSS_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/text/16E980.o(.bss COMMON .scommon);
        . = ALIGN(., 16);
        text_BSS_END = .;
        text_BSS_SIZE = ABSOLUTE(text_BSS_END - text_BSS_START);
    }
    __romPos += SIZEOF(.text);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    text_ROM_END = __romPos;
    text_VRAM_END = .;

    legal_data_ROM_START = __romPos;
    legal_data_VRAM = ADDR(.legal_data);
    .legal_data 0x1800000 : AT(legal_data_ROM_START)
    {
        FILL(0x00000000);
        legal_data_DATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/legal.data/253000.data.o(.data*);
        . = ALIGN(., 16);
        legal_data_DATA_END = .;
        legal_data_DATA_SIZE = ABSOLUTE(legal_data_DATA_END - legal_data_DATA_START);
    }
    __romPos += SIZEOF(.legal_data);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    legal_data_ROM_END = __romPos;
    legal_data_VRAM_END = .;

    mc1_data_ROM_START = __romPos;
    mc1_data_VRAM = ADDR(.mc1_data);
    .mc1_data 0x1805D80 : AT(mc1_data_ROM_START)
    {
        FILL(0x00000000);
        mc1_data_DATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/mc1.data/258D80.data.o(.data*);
        . = ALIGN(., 16);
        mc1_data_DATA_END = .;
        mc1_data_DATA_SIZE = ABSOLUTE(mc1_data_DATA_END - mc1_data_DATA_START);
    }
    __romPos += SIZEOF(.mc1_data);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    mc1_data_ROM_END = __romPos;
    mc1_data_VRAM_END = .;

    mc1_data_2_ROM_START = __romPos;
    mc1_data_2_VRAM = ADDR(.mc1_data_2);
    .mc1_data_2 0x180D480 : AT(mc1_data_2_ROM_START)
    {
        FILL(0x00000000);
        mc1_data_2_DATA_START = .;
        going-decompiled/build/usa/going-decompiled/asm/usa/data/mc1.data_2/260480.data.o(.data*);
        . = ALIGN(., 16);
        mc1_data_2_DATA_END = .;
        mc1_data_2_DATA_SIZE = ABSOLUTE(mc1_data_2_DATA_END - mc1_data_2_DATA_START);
    }
    __romPos += SIZEOF(.mc1_data_2);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    mc1_data_2_ROM_END = __romPos;
    mc1_data_2_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}

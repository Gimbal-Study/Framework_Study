MEMORY
{
    ROM  (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

REGION_ALIAS("REGION_TEXT",   ROM);
REGION_ALIAS("REGION_RODATA", ROM);
REGION_ALIAS("REGION_DATA",   RAM);
REGION_ALIAS("REGION_BSS",    RAM);

SECTIONS
{
    /* =====================================================
     * Code / Vector Table
     * ===================================================== */
    .text :
    {
        __RO_BASE__ = .;

        /*
         * Startup + Vector Table을 가장 먼저 배치
         */
        KEEP(Board/NUCLEO_F411RE/crt0.o(.text))

        /*
         * 나머지 프로그램 코드
         */
        *(.text)
        *(.text.*)

        . = ALIGN(4);
    } > REGION_TEXT


    /* =====================================================
     * ARM Exception Tables
     * ===================================================== */
    .ARM.extab :
    {
        *(.ARM.extab*)
        *(.gnu.linkonce.armextab.*)

        . = ALIGN(4);
    } > REGION_RODATA


    .ARM.exidx :
    {
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)

        . = ALIGN(4);
    } > REGION_RODATA


    /* =====================================================
     * Read Only Data
     * ===================================================== */
    .rodata :
    {
        *(.rodata)
        *(.rodata.*)

        . = ALIGN(8);

        __RO_LIMIT__ = .;
    } > REGION_RODATA


    /* =====================================================
     * Initialized Data
     *
     * VMA : RAM
     * LMA : FLASH (__RO_LIMIT__)
     * ===================================================== */
    .data : AT(__RO_LIMIT__)
    {
        __RW_BASE__ = .;

        *(.data)
        *(.data.*)

        . = ALIGN(4);

        __RW_LIMIT__ = .;
    } > REGION_DATA


    __RW_SIZE__      = SIZEOF(.data);
    __RW_LOAD_ADDR__ = LOADADDR(.data);


    /* =====================================================
     * Zero Initialized Data
     * ===================================================== */
    .bss :
    {
        __ZI_BASE__ = .;

        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(8);

        __ZI_LIMIT__ = .;

        /*
         * newlib / _sbrk()에서 사용하는 Heap 시작 위치
         */
        end  = .;
        _end = .;

    } > REGION_BSS


    __ZI_SIZE__ = SIZEOF(.bss);
}
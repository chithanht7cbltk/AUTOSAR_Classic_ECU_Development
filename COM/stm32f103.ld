/* stm32f103.ld */

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

ENTRY(Reset_Handler)

/* Stack pointer khởi tạo = đỉnh RAM (20KB) */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  .isr_vector :
  {
    KEEP(*(.isr_vector))
  } > FLASH

  .text :
  {
    *(.text*)
    *(.init)
    *(.fini)
    *(.rodata*)
    *(.ARM.extab*)
    _etext = .;
  } > FLASH

  .ARM.exidx :
  {
    *(.ARM.exidx*)
  } > FLASH

  .data : AT(_etext)
  {
    _data = .;
    *(.data*)
    _edata = .;
  } > RAM

  .bss :
  {
    _bss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM

  /DISCARD/ :
  {
    *(.note*)
    *(.comment*)
  }
}

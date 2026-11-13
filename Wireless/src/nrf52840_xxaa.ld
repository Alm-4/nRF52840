MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 1M
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 256K
}

SECTIONS
{
  .text :
  {
    KEEP(*(.isr_vector))
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
    __text_end = .;
  } > FLASH

  .data : 
  {
    __data_start = .;
    *(.data*)
    . = ALIGN(4);
    __data_end = .;
  } > RAM AT > FLASH

  __data_load = LOADADDR(.data);

  .bss :
  {
    . = ALIGN(4);
    __bss_start = .;
    __bss_start__ = __bss_start;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end = .;
    __bss_end__ = __bss_end;
  } > RAM

  .heap (NOLOAD):
  {
    __heap_start = .;
    *(.heap*)
    . = ALIGN(8);
    __heap_end = .;
  } > RAM

  .stack (NOLOAD):
  {
    . = ALIGN(8);
    __stack_bottom = .;
    . = . + 0x2000; /* 8K of stack memory */
    __stack_top = .;
  } > RAM

  __ram_end = ORIGIN(RAM) + LENGTH(RAM);
  
  end = .;
  
  ASSERT(__stack_top <= __ram_end, "RAM overflow")
}

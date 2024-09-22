OUTPUT_ARCH( "riscv" )

ENTRY( _entry )

SECTIONS
{
  . = 0x80000000;  /* This is bootloader-start */

  .text : {
    *(.text .text.*)
    . = ALIGN(4096);
    PROVIDE(ecode = .);
  }

  .rodata : {
    *(.rodata .rodata.*)
    . = ALIGN(4096);
    PROVIDE(erodata = .);
  }

  .data : {
    *(.data .data.*)
    . = ALIGN(4096);
    PROVIDE(edata = .);
  }

  .bss : {
    *(.bss .bss.*)
    . = ALIGN(4096);
    PROVIDE(ebss = .);
  }

  PROVIDE(end = .);
}
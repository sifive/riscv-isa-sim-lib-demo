SECTIONS {
  . = 0x80000000;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  .srodata : { *(.srodata) }
}

.text          # declaration of ’text’ section (which means ’program’)
.globl _main    # entry point to the ELF linker or loader.
_main:
  movl $01, %eax
  ret
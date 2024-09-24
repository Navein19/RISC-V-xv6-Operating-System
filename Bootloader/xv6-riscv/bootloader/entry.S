# _entry should be the first function invoked when
# the QEMU CPU starts executing. It's location should
# according to the memory layout provided in param.h

.section .text
.global _entry
_entry: 
    # Load the address of bl_stack_end to sp
    la sp, bl_stack_end
    call start

spin:
    j spin
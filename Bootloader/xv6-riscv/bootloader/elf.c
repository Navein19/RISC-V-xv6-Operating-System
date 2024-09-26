#include "types.h"
#include "param.h"
#include "layout.h"
#include "riscv.h"
#include "defs.h"
#include "buf.h"
#include "elf.h"

#include <stdbool.h>

struct elfhdr* kernel_elfhdr;
struct proghdr* kernel_phdr;

uint64 find_kernel_load_addr(enum kernel ktype) {
    /* CSE 536: Get kernel load address from headers */
    // Point an ELF struct (elfhdr) to RAMDISK
    /* CSE 536: Get kernel load address from headers */
    
    uint64 kernel_addr = 0x0;
    
    if (ktype == NORMAL)
    	kernel_addr = RAMDISK;
    if (ktype == RECOVERY)
    	kernel_addr = RECOVERYDISK;
    
    kernel_elfhdr =  (struct elfhdr *)kernel_addr;
    
     
    kernel_phdr = (struct proghdr *) (kernel_addr + kernel_elfhdr->phoff + kernel_elfhdr->phentsize);
    return kernel_phdr->vaddr;
}

uint64 find_kernel_size(enum kernel ktype) {
    /* CSE 536: Get kernel binary size from headers */
    return 0;
}

uint64 find_kernel_entry_addr(enum kernel ktype) {
    /* CSE 536: Get kernel entry point from headers */
    return 0;
}
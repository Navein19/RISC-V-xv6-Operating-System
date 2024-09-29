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
    uint64 kernel_addr = 0x0;
    
    
    if (ktype == NORMAL)
    	kernel_addr = RAMDISK;
    if (ktype == RECOVERY)
    	kernel_addr = RECOVERYDISK;
    
    kernel_elfhdr =  (struct elfhdr *)kernel_addr;
    // Verify ELF magic number
    if(kernel_elfhdr->magic != ELF_MAGIC) {
        // Handle error: not a valid ELF file
        return 0;
    }
    
    
    // Calculate end of section header table
    
    uint64 sect_table_end = kernel_addr + kernel_elfhdr->shoff + (kernel_elfhdr->shnum * kernel_elfhdr->shentsize);
    
    // Calculate size by subtracting the start address from the max end address
    return sect_table_end - kernel_addr;
}

uint64 find_kernel_entry_addr(enum kernel ktype) {
    
    uint64 kernel_addr = 0x0;
    
    
    if (ktype == NORMAL)
    	kernel_addr = RAMDISK;
    if (ktype == RECOVERY)
    	kernel_addr = RECOVERYDISK;
    
    	

    kernel_elfhdr = (struct elfhdr *)kernel_addr;
    return (*kernel_elfhdr).entry;
}

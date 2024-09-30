/* These files have been taken from the open-source xv6 Operating System codebase (MIT License).  */

#include "types.h"
#include "param.h"
#include "layout.h"
#include "riscv.h"
#include "defs.h"
#include "buf.h"
#include "measurements.h"
#include <stdbool.h>

void main();
void timerinit();

/* entry.S needs one stack per CPU */
__attribute__ ((aligned (16))) char bl_stack[STSIZE * NCPU];
char *bl_stack_end = bl_stack + STSIZE * NCPU;

/* Context (SHA-256) for secure boot */
SHA256_CTX sha256_ctx;

/* Structure to collects system information */
struct sys_info {
  /* Bootloader binary addresses */
  uint64 bl_start;
  uint64 bl_end;
  /* Accessible DRAM addresses (excluding bootloader) */
  uint64 dr_start;
  uint64 dr_end;
  /* Kernel SHA-256 hashes */
  BYTE expected_kernel_measurement[32];
  BYTE observed_kernel_measurement[32];
};
struct sys_info* sys_info_ptr;

extern void _entry(void);
void panic(char *s)
{
  for(;;)
    ;
}

/*  Boot into the RECOVERY kernel instead of NORMAL kernel
 * when hash verification fails. */
void setup_recovery_kernel(void) {
  /*  Load the RECVOERY kernel binary. */

  uint64 kernel_load_addr       = find_kernel_load_addr(RECOVERY);
  uint64 kernel_binary_size     = find_kernel_size(RECOVERY);     
  uint64 kernel_entry           = find_kernel_entry_addr(RECOVERY);

  

  

  /* Copying Kernel */

  uint64 no_of_blocks_to_copy = kernel_binary_size/BSIZE;
  no_of_blocks_to_copy = no_of_blocks_to_copy - 4; //skipping first 4 blocks corresponding to ELF header

  

  struct buf b;

  for(uint64 i=0; i<no_of_blocks_to_copy; i++) {


  b.blockno = i+4;
  b.valid = 0;
  //b.data = (char*)kernel_load_addr + (i * BSIZE);
  kernel_copy(RECOVERY, &b);
  memmove((char*)kernel_load_addr + (i * BSIZE), b.data, BSIZE);

  }

  

  uint64 remaining_bytes = kernel_binary_size % BSIZE;


  if(remaining_bytes > 0 ) {
   b.blockno = no_of_blocks_to_copy + 4;
   b.valid = 0;
   //b.data = (char*)kernel_load_addr + (no_of_blocks_to_copy * BSIZE);
   kernel_copy(RECOVERY, &b);
   memmove((char*)kernel_load_addr + (no_of_blocks_to_copy * BSIZE), b.data, remaining_bytes);

  }

  /*  Write the correct kernel entry point */
  w_mepc((uint64) kernel_entry);
}

/*  Function verifies if NORMAL kernel is expected or tampered. */
bool is_secure_boot(void) {
  bool verification = true;
  
  BYTE hash_obs[32];

  /* Read the binary and update the observed measurement 
   * (simplified template provided below) */
  sha256_init(&sha256_ctx);
  sha256_update(&sha256_ctx, (const unsigned char*) RAMDISK, find_kernel_size(NORMAL));
  sha256_final(&sha256_ctx, hash_obs);

  memmove(sys_info_ptr->observed_kernel_measurement, hash_obs, 32);
  
  /* Three more tasks required below: 
   *  1. Compare observed measurement with expected hash
   *  2. Setup the recovery kernel if comparison fails
   *  3. Copy expected kernel hash to the system information table */
  
  for (int i = 0; i < 32; ++i) {
    if (sys_info_ptr->observed_kernel_measurement[i] != trusted_kernel_hash[i]) {
      setup_recovery_kernel();
      memmove(sys_info_ptr->expected_kernel_measurement, trusted_kernel_hash, 32);
      verification = false;
    }
  }
  return verification;
}

// entry.S jumps here in machine mode on stack0.
void start()
{
  //Define the system information table's location. */
  sys_info_ptr = (struct sys_info*) 0x80080000;

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  w_tp(id);

  // set M Previous Privilege mode to Supervisor, for mret.
  unsigned long x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  w_mstatus(x);

  // disable paging
  w_satp(0);

  /*  Unless kernelpmp[1-2] booted, allow all memory 
   * regions to be accessed in S-mode. */ 
  #if !defined(KERNELPMP1) || !defined(KERNELPMP2)
    w_pmpaddr0(0x3fffffffffffffull);
    w_pmpcfg0(0xf);
  #endif

  /*  With kernelpmp1, isolate upper 10MBs using TOR */ 
  #if defined(KERNELPMP1)
    w_pmpaddr0(0x21d40000ull);
    w_pmpcfg0(0xf);
   #endif

  /*  With kernelpmp2, isolate 118-120 MB and 122-126 MB using NAPOT */ 
  #if defined(KERNELPMP2)
    w_pmpaddr0(0x21d80000ull);  // 0x80000000 + 118MB

    // 2. NAPOT for 120-122MB
    w_pmpaddr1(0x21E3FFFFull);  // (0x87600000 | 0x1FFFFF) >> 2

    //NAPOT for 126-128MB // (0x87e00000 | 01xFFFFF) >> 2
    w_pmpaddr2(0x21FBFFFFull);

    // Configure pmpcfg0 for all 3 entries (remains the same)
    w_pmpcfg0(0x1F1F0Full);

  #endif

  /*  Verify if the kernel is untampered for secure boot */
  if (!is_secure_boot()) {
    /* Skip loading since we should have booted into a recovery kernel 
     * in the function is_secure_boot() */
    goto out;
  }
  
  //Load the NORMAL kernel binary (assuming secure boot passed). */
  uint64 kernel_load_addr       = find_kernel_load_addr(NORMAL);
  // uint64 kernel_binary_size     = find_kernel_size(NORMAL);     
  uint64 kernel_entry           = find_kernel_entry_addr(NORMAL);

  /* Copying Kernel */

  uint64 no_of_blocks_to_copy = kernel_binary_size/BSIZE;
  no_of_blocks_to_copy = no_of_blocks_to_copy - 4; //skipping first 4 blocks corresponding to ELF header

  struct buf b;

  for(uint64 i=0; i<no_of_blocks_to_copy; i++) {

  b.blockno = i+4;
  b.valid = 0;

  //b.data = (char*)kernel_load_addr + (i * BSIZE);
  kernel_copy(NORMAL, &b);
  memmove((char*)kernel_load_addr + (i * BSIZE), b.data, BSIZE);
  }

  
  uint64 remaining_bytes = kernel_binary_size % BSIZE;

  
  if(remaining_bytes > 0 ) {
   b.blockno = no_of_blocks_to_copy + 4;
   b.valid = 0;
   //b.data = (char*)kernel_load_addr + (no_of_blocks_to_copy * BSIZE);
   kernel_copy(NORMAL, &b);
   memmove((char*)kernel_load_addr + (no_of_blocks_to_copy * BSIZE), b.data, remaining_bytes);
  }
  
  /*  Write the correct kernel entry point */
  w_mepc((uint64) kernel_entry);
 
 out:
  /*  Provide system information to the kernel. */
  /*  Send the observed hash value to the kernel (using sys_info_ptr) */
  sys_info_ptr->bl_start = 0x80000000;
  sys_info_ptr->bl_end = end;
  sys_info_ptr->dr_start = KERNBASE;
  sys_info_ptr->dr_end = PHYSTOP;
  // delegate all interrupts and exceptions to supervisor mode.
  w_medeleg(0xffff);
  w_mideleg(0xffff);
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

  // return address fix
  uint64 addr = (uint64) panic;
  asm volatile("mv ra, %0" : : "r" (addr));

  // switch to supervisor mode and jump to main().
  asm volatile("mret");
}

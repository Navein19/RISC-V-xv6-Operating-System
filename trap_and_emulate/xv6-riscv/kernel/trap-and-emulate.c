#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "csr_constants.h"

#include <stdbool.h>

#define M_MODE 2
#define S_MODE 1
#define U_MODE 0

// Struct to keep VM registers (Sample; feel free to change.)
struct vm_reg
{
    int code;
    int mode; // using this variable for tracking mode
              //  0 -> Machine Mode, 1-> Supervisor Mode, 2-> User Mode
    uint64 val;
};

struct vm_virtual_state
{
    // User trap setup
    struct vm_reg ustatus;
    struct vm_reg uie;
    struct vm_reg utvec;

    // User trap handling
    struct vm_reg uscratch;
    struct vm_reg uepc;
    struct vm_reg ucause;
    struct vm_reg utval;
    struct vm_reg uip;

    // Supervisor trap setup
    struct vm_reg sstatus;
    struct vm_reg sedeleg;
    struct vm_reg sideleg;
    struct vm_reg sie;
    struct vm_reg stvec;
    struct vm_reg scounteren;

    // Supervisor trap handling
    struct vm_reg sscratch;
    struct vm_reg sepc;
    struct vm_reg scause;
    struct vm_reg stval;
    struct vm_reg sip;

    // Supervisor page table register
    struct vm_reg satp;

    // Machine information registers
    struct vm_reg mvendorid;
    struct vm_reg marchid;
    struct vm_reg mimpid;
    struct vm_reg mhartid;

    // Machine trap setup registers
    struct vm_reg mstatus;
    struct vm_reg misa;
    struct vm_reg medeleg;
    struct vm_reg mideleg;
    struct vm_reg mie;
    struct vm_reg mtvec;
    struct vm_reg mcounteren;

    // Machine trap handling registers
    struct vm_reg mscratch;
    struct vm_reg mepc;
    struct vm_reg mcause;
    struct vm_reg mtval;
    struct vm_reg mip;

    // Machine physical memory protection registers
    struct vm_reg pmpcfg[16];
    struct vm_reg pmpaddr[64];

    // Privilege mode and page table setup
    uint64 priviledge_mode;    // 0: U-mode, 1: S-mode, 2: M-mode
    bool pmp_setup;            // Is PMP configured?
    pagetable_t pmp_pagetable; // PMP page table
    pagetable_t og_pagetable;  // Original page table
};

struct vm_virtual_state vm_state;

struct vm_reg* get_csr_reg(uint32 csr_address, struct vm_virtual_state *vm_state)
{
    switch (csr_address)
    {
    // User trap setup
    case CSR_USTATUS:
        return &vm_state->ustatus;
    case CSR_UIE:
        return &vm_state->uie;
    case CSR_UTVEC:
        return &vm_state->utvec;

    // User trap handling
    case CSR_USCRATCH:
        return &vm_state->uscratch;
    case CSR_UEPC:
        return &vm_state->uepc;
    case CSR_UCAUSE:
        return &vm_state->ucause;
    case CSR_UTVAL:
        return &vm_state->utval;
    case CSR_UIP:
        return &vm_state->uip;

    // Supervisor trap setup
    case CSR_SSTATUS:
        return &vm_state->sstatus;
    case CSR_SEDELEG:
        return &vm_state->sedeleg;
    case CSR_SIDELEG:
        return &vm_state->sideleg;
    case CSR_SIE:
        return &vm_state->sie;
    case CSR_STVEC:
        return &vm_state->stvec;
    case CSR_SCOUNTEREN:
        return &vm_state->scounteren;

    // Supervisor trap handling
    case CSR_SSCRATCH:
        return &vm_state->sscratch;
    case CSR_SEPC:
        return &vm_state->sepc;
    case CSR_SCAUSE:
        return &vm_state->scause;
    case CSR_STVAL:
        return &vm_state->stval;
    case CSR_SIP:
        return &vm_state->sip;

    // Supervisor page table register
    case CSR_SATP:
        return &vm_state->satp;

    // Machine information registers
    case CSR_MVENDORID:
        return &vm_state->mvendorid;
    case CSR_MARCHID:
        return &vm_state->marchid;
    case CSR_MIMPID:
        return &vm_state->mimpid;
    case CSR_MHARTID:
        return &vm_state->mhartid;

    // Machine trap setup registers
    case CSR_MSTATUS:
        return &vm_state->mstatus;
    case CSR_MISA:
        return &vm_state->misa;
    case CSR_MEDELEG:
        return &vm_state->medeleg;
    case CSR_MIDELEG:
        return &vm_state->mideleg;
    case CSR_MIE:
        return &vm_state->mie;
    case CSR_MTVEC:
        return &vm_state->mtvec;
    case CSR_MCOUNTEREN:
        return &vm_state->mcounteren;

    // Machine trap handling registers
    case CSR_MSCRATCH:
        return &vm_state->mscratch;
    case CSR_MEPC:
        return &vm_state->mepc;
    case CSR_MCAUSE:
        return &vm_state->mcause;
    case CSR_MTVAL:
        return &vm_state->mtval;
    case CSR_MIP:
        return &vm_state->mip;

    // Machine physical memory protection registers
    case CSR_PMPCFG_BASE ...(CSR_PMPCFG_BASE + 15): // pmpcfg0 to pmpcfg15
        vm_state->pmp_setup = true;                 // enbaling PMP is any of cgf registers is being used in the instruction
        return &vm_state->pmpcfg[csr_address - CSR_PMPCFG_BASE];
    case CSR_PMPADDR_BASE ...(CSR_PMPADDR_BASE + 63): // pmpaddr0 to pmpaddr63
        vm_state->pmp_setup = true;
        return &vm_state->pmpaddr[csr_address - CSR_PMPADDR_BASE];

    // Default case
    default:
    
        panic("Trap and Emulate : Unknown CSR address!!");
    }
}

void init_reg(int MODE, uint32 code, uint64 val) {
    struct vm_reg tempReg;
    tempReg.code = code; 
    tempReg.mode = MODE;
    tempReg.val = val; 

    struct vm_reg* regPtr = get_csr_reg(code, &vm_state);
    if (regPtr != NULL) {
        *regPtr = tempReg;
    }
}

void trap_and_emulate_init(void)
{
    /* Create and initialize all state for the VM */
    // initializing user registers
    init_reg(U_MODE, CSR_USTATUS, 0);
    init_reg(U_MODE, CSR_UIE, 0);
    init_reg(U_MODE, CSR_UTVEC, 0);
    init_reg(U_MODE, CSR_USTATUS, 0);
    init_reg(U_MODE, CSR_USCRATCH, 0);
    init_reg(U_MODE, CSR_UEPC, 0);
    init_reg(U_MODE, CSR_UCAUSE, 0);
    init_reg(U_MODE, CSR_UTVAL, 0);
    init_reg(U_MODE, CSR_UIP, 0);

    // initaliazing supervisor registers
    init_reg(S_MODE, CSR_SSTATUS, 0);
    init_reg(S_MODE, CSR_SEDELEG, 0);
    init_reg(S_MODE, CSR_SIDELEG, 0);
    init_reg(S_MODE, CSR_SIE, 0);
    init_reg(S_MODE, CSR_STVEC, 0);
    init_reg(S_MODE, CSR_SCOUNTEREN, 0);
    init_reg(S_MODE, CSR_SSCRATCH, 0);
    init_reg(S_MODE, CSR_SEPC, 0);
    init_reg(S_MODE, CSR_SCAUSE, 0);
    init_reg(S_MODE, CSR_STVAL, 0);
    init_reg(S_MODE, CSR_SIP, 0);
    init_reg(S_MODE, CSR_SATP, 0);

    // initailizing machine registers
    init_reg(M_MODE, CSR_MVENDORID, 0);
    init_reg(M_MODE, CSR_MARCHID, 0);
    init_reg(M_MODE, CSR_MIMPID, 0);
    init_reg(M_MODE, CSR_MHARTID, 0);
    init_reg(M_MODE, CSR_MSTATUS, 0);
    init_reg(M_MODE, CSR_MISA, 0);
    init_reg(M_MODE, CSR_MEDELEG, 0);
    init_reg(M_MODE, CSR_MIDELEG, 0);
    init_reg(M_MODE, CSR_MIE, 0);
    init_reg(M_MODE, CSR_MTVEC, 0);
    init_reg(M_MODE, CSR_MCOUNTEREN, 0);
    init_reg(M_MODE, CSR_MSCRATCH, 0);
    init_reg(M_MODE, CSR_MEPC, 0);
    init_reg(M_MODE, CSR_MCAUSE, 0);
    init_reg(M_MODE, CSR_MTVAL, 0);
    init_reg(M_MODE, CSR_MIP, 0);

    // Machine physical memory protection
    for (int i = 0; i < 16; i++)
    {
        vm_state.pmpcfg[i].code = CSR_PMPCFG_BASE + i;
        vm_state.pmpcfg[i].mode = M_MODE;
        vm_state.pmpcfg[i].val = 0x0;
    }

    for (int i = 0; i < 64; i++)
    {
        vm_state.pmpaddr[i].code = CSR_PMPADDR_BASE + i;
        vm_state.pmpaddr[i].mode = M_MODE;
        vm_state.pmpaddr[i].val = 0x0;
    }

    vm_state.mvendorid.val = 0x637365353336; // Set mvendorid to "cse536" in HEX
    vm_state.priviledge_mode = M_MODE;       // VM should boot at M-Mode

    vm_state.pmp_setup = false;
}

void emulate_sret(struct proc *p)
{
    // Step 1: Retrieve the previous program counter (sepc)
    uint64 sepc = vm_state.sepc.val;

    // Step 2: Decode the SPP (Supervisor Previous Privilege) field from sstatus
    uint64 sstatus = vm_state.sstatus.val;
    uint64 spp = (sstatus >> 8) & 0x1; // SPP is bit 8 in sstatus

    // Step 3: Restore the privilege mode based on SPP
    if (spp == 0)
    {
        vm_state.priviledge_mode = 0; // Return to User mode
    }
    else if (spp == 1)
    {
        vm_state.priviledge_mode = 1; // Return to Supervisor mode
    }
    else
    {
        setkilled(p);
        panic("Invalid SPP value during SRET emulation");
    }

    p->trapframe->epc = sepc;
}

void emulate_mret(struct proc *p)
{
    // Step 1: Retrieve the previous program counter (mepc)
    uint64 mepc = vm_state.mepc.val;

    // Step 2: Decode the MPP (Machine Previous Privilege) field from mstatus
    uint64 mstatus = vm_state.mstatus.val;
    uint64 mpp = (mstatus >> 11) & 0x3; // MPP is bits [12:11]

    // Step 3: Restore the privilege mode based on MPP
    if (mpp == 0)
    {
        vm_state.priviledge_mode = U_MODE; // Return to User mode
    }
    else if (mpp == 1)
    {
        vm_state.priviledge_mode = S_MODE; // Return to Supervisor mode
    }
    else if (mpp == 3)
    {
        vm_state.priviledge_mode = M_MODE; // Return to Machine mode
    }
    else
    {
        setkilled(p);
        panic("Invalid MPP value during MRET emulation");
    }

    // Step 4: Clear the MPP field in mstatus
    mstatus &= ~(3UL << 11); // Clear bits [12:11] (MPP)

    // Step 5: Restore the MIE (Machine Interrupt Enable) bit
    mstatus |= (mstatus & (1UL << 3)); // Preserve MIE bit (bit 3)

    // Update the mstatus register in the VM state
    vm_state.mstatus.val = mstatus;

    // Step 6: Set the program counter to the saved exception program counter (mepc)
    p->trapframe->epc = mepc;

    // handle PMP protection here
}

void emulate_ecall(int current_mode, struct proc *p)
{

    printf("(EC at %p)\n", p->trapframe->epc);
    // 1. Set the trap cause
    if (current_mode == U_MODE)
    {                            // U-mode
        vm_state.scause.val = 8; // Environment call from U-mode
    }
    else if (current_mode == S_MODE)
    {                            // S-mode
        vm_state.scause.val = 9; // Environment call from S-mode
    }
    else
    {
        panic("ECALL from unsupported privilege mode! Maybe from M Mode :(");
    }

    // 2. Save the program counter to sepc
    vm_state.sepc.val = p->trapframe->epc;

    // 3. Determine the next privilege mode and set the trap handler
    if (current_mode == U_MODE)
    {
        // U-mode to S-mode transition
        vm_state.priviledge_mode = 1;           // Transition to Supervisor mode
        p->trapframe->epc = vm_state.stvec.val; // Set PC to Supervisor trap handler
    }
    else if (current_mode == S_MODE)
    {
        // S-mode to M-mode transition (if configured)
        vm_state.priviledge_mode = 0;           // Transition to Machine mode
        p->trapframe->epc = vm_state.mtvec.val; // Set PC to Machine trap handler
    }

    // 4. Setting up page table
    /*
    if (current_mode == U_MODE) {
        if(vm_state.pmp_setup == true)
            p->pagetable = vm_state.pmp_pagetable;
    }
    if (current_mode == S_MODE) {
        if(vm_state.pmp_setup == true)
            p->pagetable = vm_state.og_pagetable;
    }
    */
}

void emulate_csrr(struct proc *p, uint32 rd, uint32 rs1, uint32 uimm)
{
    if (rs1 != 0x0)
    {
        setkilled(p);
        trap_and_emulate_init();
        panic("Invalid CSRW instruction, destination, rs1 register is not empty");
    }

    struct vm_reg* src = get_csr_reg(uimm, &vm_state);
    uint64 *dest = &(p->trapframe->ra) + rd - 1;

    if ((uimm == CSR_MVENDORID)) // CSRR instruction can be used to read CSR_MVENDORID in all privilege modes
    {
        *dest = src->val;
        p->trapframe->epc += 4;
        return;
    }

    if (vm_state.priviledge_mode >= src->mode)
    {
        *dest = src->val;
        p->trapframe->epc += 4;
    }
    else
    {
        setkilled(p);
        trap_and_emulate_init();
        panic("Invalid instruction CSRW, trying to execute higher privelaged instruction ...");
    }
}

void emulate_csrw(struct proc *p, uint32 rd, uint32 rs1, uint32 uimm)
{

    if (rd != 0x0)
    {
        //printf("here1");
        setkilled(p);
        trap_and_emulate_init();
        panic("Invalid CSRW instruction, destination, rd register is not empty");
    }

    struct vm_reg* dest = get_csr_reg(uimm, &vm_state);
    uint64 *src = &(p->trapframe->ra) + rs1 - 1;

    if (vm_state.priviledge_mode >= dest->mode)
    {
        //printf("here2");
        if ((uimm == CSR_MVENDORID) && (*src == 0x0))
        {
            // cannot overrite empty value into vendorID hardware register
            //printf("here3");
            setkilled(p);
            trap_and_emulate_init();
        }
        //printf("here4");
        dest->val = *src;
        p->trapframe->epc += 4;
    }
    else
    {
        //printf("here5");
        setkilled(p);
        trap_and_emulate_init();
        panic("Invalid instruction CSRW, trying to execute higher privelaged instruction ...");
    }
}

// In your ECALL, add the following for prints
// struct proc* p = myproc();
// printf("(EC at %p)\n", p->trapframe->epc);

void trap_and_emulate(void)
{
    /* Comes here when a VM tries to execute a supervisor instruction. */

    struct proc *p = myproc();

    //printf("current mode : %d", vm_state.priviledge_mode);
    /* Retrieve all required values from the instruction */
    uint64 addr = r_sepc();
    uint32 instruction = 0; // in RISCV-xv6 all the instructions are 32 bit

    // Fetch the instruction from virtual memory
    if (copyin(p->pagetable, (char *)&instruction, addr, sizeof(uint32)) < 0)
    {
        panic("Failed to fetch instruction from virtual memory");
    }

    // Decode and handle the instruction (rest of the code unchanged)
    uint32 op = instruction & 0x7F;            // Bits [6:0] (opcode)
    uint32 rd = (instruction >> 7) & 0x1F;     // Bits [11:7] (destination register)
    uint32 funct3 = (instruction >> 12) & 0x7; // Bits [14:12] (funct3)
    uint32 rs1 = (instruction >> 15) & 0x1F;   // Bits [19:15] (source register)
    uint32 uimm = (instruction >> 20) & 0xFFF; // Bits [31:20] (CSR address)

    // printf("(Decoded Instruction) addr: %p, opcode: 0x%x, rd: x%d, funct3: 0x%x, rs1: x%d, csr: 0x%x\n",addr, op, rd, funct3, rs1, uimm);

    int current_mode = vm_state.priviledge_mode;

    switch (op)
    {
    case 0x73:
        // only case we have to handle as for all instructions the op code is same
        switch (funct3)
        {
        case 0x0: // for ECALL, SRET & MRET
            if (rd == 0x0 && rs1 == 0x0)
            {
                if (uimm == 0x0)
                {
                    // ECALL
                    if (current_mode != U_MODE && current_mode != S_MODE)
                    {
                        return; // ECALL is not supported from M mode.
                    }
                    // ECALL
                    emulate_ecall(current_mode, p);
                    //printf("(PI at %p) op = %x, rd = %x, funct3 = %x, rs1 = %x, uimm = %x\n", addr, op, rd, funct3, rs1, uimm);
                }

                else if (uimm == 0x102 && current_mode == S_MODE)
                {
                    // SRET
                    /* Print the statement */
                    printf("(PI at %p) op = %x, rd = %x, funct3 = %x, rs1 = %x, uimm = %x\n", addr, op, rd, funct3, rs1, uimm);
                    emulate_sret(p);
                }
                else if (uimm == 0x302 && current_mode == M_MODE)
                {
                    // MRET
                    printf("(PI at %p) op = %x, rd = %x, funct3 = %x, rs1 = %x, uimm = %x\n", addr, op, rd, funct3, rs1, uimm);
                    emulate_mret(p);
                }
                else
                {
                    printf("(PI at %p) op = %x, rd = %x, funct3 = %x, rs1 = %x, uimm = %x\n", addr, op, rd, funct3, rs1, uimm);
                    printf("Instruction is not correct.\n");
                    setkilled(p);
                    trap_and_emulate_init();
                }
            }
            else
            {
                printf("(PI at %p) op = %x, rd = %x, funct3 = %x, rs1 = %x, uimm = %x\n", addr, op, rd, funct3, rs1, uimm);
               printf("Instruction is not correct\n");
                setkilled(p);
                trap_and_emulate_init();
            }

            break;
        case 0x1: // for CSRW
            printf("(PI at %p) op = %x, rd = %x, funct3 = %x, rs1 = %x, uimm = %x\n", addr, op, rd, funct3, rs1, uimm);
            emulate_csrw(p, rd, rs1, uimm);
            break;
        case 0x2: // for CSR
            printf("(PI at %p) op = %x, rd = %x, funct3 = %x, rs1 = %x, uimm = %x\n", addr, op, rd, funct3, rs1, uimm);
            emulate_csrr(p, rd, rs1, uimm);
            break;
        default:
            setkilled(p);
            break;
        }
        break;

    default:
        setkilled(p);
        panic("Unsupported opcode in trap_and_emulate()");
    }
}

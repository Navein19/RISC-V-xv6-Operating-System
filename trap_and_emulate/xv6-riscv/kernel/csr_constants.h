// User trap setup registers
#define CSR_USTATUS      0x000
#define CSR_UIE          0x004
#define CSR_UTVEC        0x005

// User trap handling registers
#define CSR_USCRATCH     0x040
#define CSR_UEPC         0x041
#define CSR_UCAUSE       0x042
#define CSR_UTVAL        0x043
#define CSR_UIP          0x044

// Supervisor trap setup registers
#define CSR_SSTATUS      0x100
#define CSR_SEDELEG      0x102
#define CSR_SIDELEG      0x103
#define CSR_SIE          0x104
#define CSR_STVEC        0x105
#define CSR_SCOUNTEREN   0x106

// Supervisor trap handling registers
#define CSR_SSCRATCH     0x140
#define CSR_SEPC         0x141
#define CSR_SCAUSE       0x142
#define CSR_STVAL        0x143
#define CSR_SIP          0x144

// Supervisor page table register
#define CSR_SATP         0x180

// Machine information registers
#define CSR_MVENDORID    0xF11
#define CSR_MARCHID      0xF12
#define CSR_MIMPID       0xF13
#define CSR_MHARTID      0xF14

// Machine trap setup registers
#define CSR_MSTATUS      0x300
#define CSR_MISA         0x301
#define CSR_MEDELEG      0x302
#define CSR_MIDELEG      0x303
#define CSR_MIE          0x304
#define CSR_MTVEC        0x305
#define CSR_MCOUNTEREN   0x306

// Machine trap handling registers
#define CSR_MSCRATCH     0x340
#define CSR_MEPC         0x341
#define CSR_MCAUSE       0x342
#define CSR_MTVAL        0x343
#define CSR_MIP          0x344

// Machine physical memory protection registers
#define CSR_PMPCFG_BASE  0x3A0 // Starting address for pmpcfg
#define CSR_PMPADDR_BASE 0x3B0 // Starting address for pmpaddr
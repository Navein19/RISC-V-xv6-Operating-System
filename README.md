# xv6 RISC-V Operating System Development

This repository contains my work on **xv6**, a Unix-like operating system developed for educational purposes, with a focus on the **RISC-V architecture**. The project begins with the implementation and understanding of the bootloader and will expand to cover other core components of the operating system, such as process management, file systems, and memory management.

## Repository Structure

- **main**: The main branch contains stable versions of the code after significant features have been developed and tested.
- **bootloader**: The branch dedicated to the **bootloader**, which is responsible for loading the OS into memory and starting execution.
- Other branches will be created for each significant feature or milestone in the development process (e.g., process scheduling, system calls, memory management, etc.).

## Getting Started

### Prerequisites

To run or modify the xv6 RISC-V OS, you will need:

- A RISC-V toolchain for compiling and running the OS
- A QEMU virtual machine setup for emulating the RISC-V architecture

### Setup Instructions

To clone and set up the repository locally:

```bash
git clone https://github.com/<your-username>/xv6-riscv.git
cd xv6-riscv

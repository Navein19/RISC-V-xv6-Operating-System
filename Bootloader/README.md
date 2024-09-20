# xv6 RISC-V OS Development

This repository is part of an ongoing project to develop and understand the internals of the xv6 Operating System, a Unix-like OS designed for the RISC-V architecture. xv6, originally developed by MIT, serves as an educational tool for OS concepts, but this project is aimed at further exploring and enhancing various aspects of OS development, starting with the bootloader.

## Bootloader Module

The first phase of this project focuses on the **bootloader** of xv6. The bootloader is a critical component responsible for loading the kernel into memory and handing control over to it during system startup. This module provides a deep dive into how the boot process works, covering the steps required to boot xv6 on the RISC-V architecture using QEMU.

## A. Installing Pre-requisites

Before we dive into the bootloader, we need to set up the necessary tools to run xv6 on your machine.

### 1. Using Package Managers (Recommended)

#### Linux/WSL:
1. Update your system:
   ```bash
   sudo apt-get update
   ```
2. Install the required packages:
   ```bash
   sudo apt-get install git build-essential gdb-multiarch qemu-system-misc gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
   ```

#### macOS:
1. Install Xcode Command Line Tools:
   ```bash
   xcode-select --install
   ```
2. Install Homebrew (if not already installed):
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```
3. Configure Homebrew:
   ```bash
   (echo; echo 'eval "$(/opt/homebrew/bin/brew shellenv)"') >> ~/.zprofile
   eval "$(/opt/homebrew/bin/brew shellenv)"
   ```
4. Tap the RISC-V tools:
   ```bash
   brew tap riscv/riscv
   ```
5. Install the RISC-V toolchain and QEMU:
   ```bash
   brew install riscv-tools
   brew install qemu
   brew link qemu
   brew link --overwrite qemu
   ```
6. Add the toolchain to your path:
   ```bash
   echo 'export PATH="/opt/homebrew/Cellar/riscv-gnu-toolchain/main/bin:$PATH"' >> ~/.zshrc
   source ~/.zshrc
   ```

### 2. Building from Source (Alternative)

If you prefer to build the toolchain and QEMU from source, you can follow these steps:

#### Linux/WSL:
1. Navigate to the `install/linux-wsl` directory.
2. Install QEMU:
   ```bash
   ./linux-qemu.sh
   ```
3. Install the RISC-V toolchain:
   ```bash
   ./linux-toolchain.sh
   ```
4. Add the binaries to your path:
   ```bash
   source .add-linux-paths
   ```

#### macOS:
1. Navigate to the `install/mac` directory.
2. Install QEMU:
   ```bash
   ./mac-qemu.sh
   ```
3. Install the RISC-V toolchain:
   ```bash
   ./mac-toolchain.sh
   ```
4. Add the binaries to your path:
   ```bash
   source .add-mac-paths
   ```

## B. Running the xv6 OS

Once the environment is set up, clone the xv6 repository and boot the OS using QEMU:

1. Clone the xv6 source code:
   ```bash
   git clone https://github.com/mit-pdos/xv6-riscv.git
   ```
2. Navigate to the cloned directory:
   ```bash
   cd xv6-riscv
   ```
3. Run xv6 using QEMU:
   ```bash
   make qemu
   ```

## C. Common Issues & Solutions

1. **glib-2.48 and gthread-2.0 Required for QEMU Compilation**: If you encounter the following error during `linux-qemu.sh` execution:
   ```
   ERROR: glib-2.48 gthread-2.0 is required to compile QEMU
   ```
   Fix it by installing the required dependency:
   ```bash
   sudo apt install libglib2.0-dev
   ```

2. **pixman-1 Dependency Error**: During `linux-qemu.sh`, you might encounter:
   ```
   ../meson.build:328:2: ERROR: Dependency "pixman-1" not found
   ```
   Resolve it by installing the `libpixman` development package:
   ```bash
   sudo apt install libpixman-1-dev
   ```

## Acknowledgements

This project is built on the open-source work by the xv6 team at MIT, and is extended with enhancements for deeper exploration of RISC-V architecture and OS design. Special thanks to Rahul, Divyam, Ayushman, and Mukul for their guidance and contributions to the repository setup and toolchain support.
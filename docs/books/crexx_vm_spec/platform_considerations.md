# Platform Considerations

## Instructions

All VM instructions work in exactly the same way on all supported platforms - for numerical algorithms, within the limitations of the available hardware.

## Bytecode and layout of binary modules

The binary file layout containing the VM bytecode instructions is identical over all supported platforms. This means that the `.rxbin` files can be transported to other platforms, for example from Linux on arm64 to Windows on X86_64 and to Linux on arm64, z390x or RISC-V, and work unchanged.

## Native executables

This portability does not hold for native executables. These are `.rxbin` files which are preprocessed by the `rxcpack` command into `.c` source which are then compiled and linked by platform specific compiler toolchains. These are usable on only one instruction set - operating system combination.

The same goes for `.rxplugin` files; these are also native executables.

## 64-bit limitation

cRexx is not intended for, or is expected to work on 32-bit architectures. Certain IBM legacy architectures, like s/370, might work when explicitly indicated.

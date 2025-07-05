# Platform Considerations

## Instructions

All VM instructions work in exactly the same way on all supported platforms. 

## Bytecode and layout of binary modules

The binary file layout containing the VM bytecode instructions is identical over all supported platforms. This means that the `.rxbin` files can be transported to other platforms, for example from Linux on arm64 to Windows on X86_64 and to Linux on RISC-V and work unchanged.

## Native executables

This portability does not hold for native executables. These are `.rxbin` files which are preprocessed by the `rxcpack` command into `.c` source which are then compiled and linked by platform specific compiler toolchains. These are usable on only one instruction set - operating system combination.

The same goes for `.rxplugin` files; these are also native executables.

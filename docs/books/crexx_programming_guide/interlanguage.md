# Interlanguage calls

A cRexx program is able to call programs written in the Rexx
language, but also programs native to the platform, using a number of
calling conventions:

## Current Mechanisms

Address
: the `address` statement can use the shell and
    I/O indirection to start native executables and provide input, and
    retrieve the output.

Plugins
 : the plugin system that is available to the cRexx environment.

Crexxsaa
: the host integration facade for C applications that want to embed cRexx,
    register ADDRESS environments, and run `.rxbin` or source files through
    the cRexx toolchain and runtime.

<!-- ## Future Mechanisms -->

<!-- RexxSaa -->
<!-- : the traditional RexxSAA calling convention can be -->
<!--       used for direct interfaces to executables that are designed to -->
<!--       function as a Rexx library. In its most simple form, these can -->
<!--       return Rexx strings to the calling program. -->

<!-- Generic Call Interface -->
<!-- : the Generic Call Interface is a direct way to call functions in a dll or shared object file. -->

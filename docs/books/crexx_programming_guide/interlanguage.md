# Interlanguage calls
A \crexx{} program is able to call programs written in the \rexx{}
language, but also programs native to the platform, using a number of
calling conventions:

## Current Mechanisms

Address
: the \code{address} statement can use the shell and
    I/O indirection to start native executables and provide input, and
    retrieve the output.

Plugins
 : the plugin system that is available to the \crexx{} environment.

<!-- ## Future Mechanisms -->

<!-- RexxSaa -->
<!-- : the traditional RexxSAA calling convention can be -->
<!--       used for direct interfaces to executables that are designed to -->
<!--       function as a \rexx{} library. In its most simple form, these can -->
<!--       return \rexx{} strings to the calling program. -->

<!-- Generic Call Interface -->
<!-- : the Generic Call Interface is a direct way to call functions in a dll or shared object file. -->


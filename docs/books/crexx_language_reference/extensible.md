## Extensibility

A small language is easy to learn and easy to use. When the language is too small, and there is no large, well organised runtime library, a lot of function needs to be provided by the user. Classic \rexx{} provides extensibility through addressing environments and function packages. These environments need to be addressed each in their own way, the function packages need to be registered and checked; the searching and linkage conmventions differ per platform and operating system.

\crexx{} adds an easily extensible module system which integrates extended runtime functionality which behaves in the same manner over library extensions written in Rexx or native functions written in C and in other languages.

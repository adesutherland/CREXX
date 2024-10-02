# Modules {#modules}

Each source file (.rexx) is compiled and assembled into a module file (\*.rxbin). These module files can be combined into a single module library through simple file concatenation. This module library can be regarded as a module itself.

Each module will contain a number of procedures, class members, and global variables that (if exposed) can be called by other modules.

*CURRENT STATUS: Classes are not implemented*

The name of the module is the same as the name of the file, but this doesn't have any particular significance for crexx itself. The namespace is the key attribute.

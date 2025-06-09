# Special Instruction set considerations for the Rexx language

The ```rxvm``` virtual machine instruction set aims to be a generic virtual machine implementation, but in a limited number of instructrions special consideration to the Rexx language has been given. These cases are highlighted in this chapter.



say

: ```say``` is a nod to users of the Rexx language and is intended as an easy and familiar way to display data on the console. It takes either a ```STRING``` or a ```REG``` argument; in the ```REG``` version, it will look at the type of the register and convert it to string. The ```SAYX``` form performs the same function without adding a line ending.

strupper

: ```strupper``` supports the ```upper``` built-in function 


strlower

: ```strlower``` supports the ```lower``` built-in function 

strlength

: ```strlength``` supports the ```length``` built-in function 


## The register conversion instructions

As the VM is register based, and a register can contain numbers or pointers to strings, these data conversion instructions can ready the contents of a register for usage in other instructions, working on different types. These can be put to good use, for example, to convert the register content to a string before displaying it with `SAY`.

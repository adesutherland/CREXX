# Virtual Machine Instruction Set

This chapter contains detailed description of the machine instructions, grouped by functional characteristics. For each instruction, its variants, opcodes and usage examples are listed. It is intended to be used for reference; therefore it is recommended to read [Assembler Programming Guide](#assembler-programming-guide) (starting on page \pageref{assembler-programming-guide}) in its entirety before first usage.

## Characteristics of `rxvm` instructions

Most \crexx{} Virtual Machine instructions share the following
characteristics:

- Logical data flow is from right to left
  
- There is no separate CC (Condition Code) or status register; a
    register specified in the instruction is used for these results;
    for example all comparisons leave their result in a register, which
    subsequently can be used for a conditional branch
    
- The assembler does not have any data definition directives; data is entered directly into registers,
  which then are used by the instructions. In `.rxbin` modules this data is represented in the Constant Pool. 

## Instruction Argument Types

|Type | Description |
|------|------------------------------------|
|REG| a register|
|STRING| a sequence of Unicode characters|
|FUNC| a global or local function|
|ID| a label|
|INT| an integer type (a whole number)|
|FLOAT| a floating point number|



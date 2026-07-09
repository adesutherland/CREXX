# Floating Point

Floating-point arithmetic, comparison, formatting, conversion, and sign operations.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction placeholders in this section: 24.

## Section Checklist

- Purpose overview: TODO
- Operand conventions: TODO
- Signal/error behavior: TODO
- Examples: TODO
- Cross-links to related instructions: TODO

## `fadd`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x012c` | `{REG,REG,REG}` | Float Add (op1=op2+op3) |
| `0x012d` | `{REG,REG,FLOAT}` | Float Add (op1=op2+op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fcopy`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x000a` | `{REG,REG}` | Copy Float op2 to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fdiv`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0133` | `{REG,REG,REG}` | Float Divide (op1=op2/op3) |
| `0x0134` | `{REG,REG,FLOAT}` | Float Divide (op1=op2/op3) |
| `0x0135` | `{REG,FLOAT,REG}` | Float Divide (op1=op2/op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `feq`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x013c` | `{REG,REG,REG}` | Float Equals op1=(op2==op3) |
| `0x013d` | `{REG,REG,FLOAT}` | Float Equals op1=(op2==op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fextr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0152` | `{REG,REG,REG}` | op3 float extracted to op1 string coefficient and op2 int decimal exponent |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fformat`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0106` | `{REG,REG,REG}` | DEPRECATED use fextr. Set string value from float value using a format string |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fgt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0140` | `{REG,REG,REG}` | Float Greater than op1=(op2>op3) |
| `0x0141` | `{REG,REG,FLOAT}` | Float Greater than op1=(op2>op3) |
| `0x0142` | `{REG,FLOAT,REG}` | Float Greater than op1=(op2>op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fgtbr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x014c` | `{ID,REG,REG}` | Float Greater than if (op2>op3) goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fgte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0143` | `{REG,REG,REG}` | Float Greater than equals op1=(op2>=op3) |
| `0x0144` | `{REG,REG,FLOAT}` | Float Greater than equals op1=(op2>=op3) |
| `0x0145` | `{REG,FLOAT,REG}` | Float Greater than equals op1=(op2>=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fidiv`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0136` | `{REG,REG,REG}` | Integer Divide with Floats (op1=op2/op3) |
| `0x0137` | `{REG,REG,FLOAT}` | Integer Divide with Floats (op1=op2/op3) |
| `0x0138` | `{REG,FLOAT,REG}` | Integer Divide with Floats (op1=op2/op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `flt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0146` | `{REG,REG,REG}` | Float Less than op1=(op2<op3) |
| `0x0147` | `{REG,REG,FLOAT}` | Float Less than op1=(op2<op3) |
| `0x0148` | `{REG,FLOAT,REG}` | Float Less than op1=(op2<op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fltbr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x014d` | `{ID,REG,REG}` | Float Less than if (op2<op3) goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `flte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0149` | `{REG,REG,REG}` | Float Less than equals op1=(op2<=op3) |
| `0x014a` | `{REG,REG,FLOAT}` | Float Less than equals op1=(op2<=op3) |
| `0x014b` | `{REG,FLOAT,REG}` | Float Less than equals op1=(op2<=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fmod`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0139` | `{REG,REG,REG}` | Float Modulo (op1=op2%op3) |
| `0x013a` | `{REG,REG,FLOAT}` | Float Modulo (op1=op2%op3) |
| `0x013b` | `{REG,FLOAT,REG}` | Float Modulo (op1=op2&op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fmult`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0131` | `{REG,REG,REG}` | Float Multiply (op1=op2*op3) |
| `0x0132` | `{REG,REG,FLOAT}` | Float Multiply (op1=op2*op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fndblnk`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00a5` | `{REG,REG,REG}` | op1 = find next blank in op2[op3] and behind |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fndnblnk`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00a6` | `{REG,REG,REG}` | op1 = find next next non blank in op2[op3] and behind |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fne`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x013e` | `{REG,REG,REG}` | Float Not equals op1=(op2!=op3) |
| `0x013f` | `{REG,REG,FLOAT}` | Float Not equals op1=(op2!=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fpow`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x014e` | `{REG,REG,REG}` | op1=op2**op3 |
| `0x014f` | `{REG,REG,FLOAT}` | op1=op2**op3 |
| `0x0150` | `{REG,FLOAT,REG}` | op1=op2**op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fsex`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0151` | `{REG}` | float op1 = -op1 (sign change) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fsub`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x012e` | `{REG,REG,REG}` | Float Subtract (op1=op2-op3) |
| `0x012f` | `{REG,REG,FLOAT}` | Float Subtract (op1=op2-op3) |
| `0x0130` | `{REG,FLOAT,REG}` | Float Subtract (op1=op2-op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ftob`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00e9` | `{REG}` | Set register boolean (int 1 or 0) value from its float value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ftoi`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00e8` | `{REG}` | Set register int value from its float value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ftos`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00e6` | `{REG}` | Set register string value from its float value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

# Data Movement And Conversion

Register copying, loading, type conversion, and basic register lifecycle instructions.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction placeholders in this section: 7.

## Section Checklist

- Purpose overview: TODO
- Operand conventions: TODO
- Signal/error behavior: TODO
- Examples: TODO
- Cross-links to related instructions: TODO

## `acopy`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x000d` | `{REG,REG}` | Copy status Attributes op2 to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `copy`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0008` | `{REG,REG}` | Copy op2 to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `erase`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01fc` | `{REG}` | erases register contents |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `load`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0001` | `{REG,INT}` | Load op1 with op2 |
| `0x0002` | `{REG,FLOAT}` | Load op1 with op2 |
| `0x0003` | `{REG,STRING}` | Load op1 with op2 |
| `0x0004` | `{REG,REG}` | Load op1 with op2 |
| `0x0005` | `{REG,DECIMAL}` | Load op1 with op2 |
| `0x0006` | `{INT,INT}` | Load op1 with op2 (non symbolic registers) |
| `0x0007` | `{INT,REG}` | Load op1 with op2 (non symbolic registers) |
| `0x00b7` | `{REG,BINARY}` | Load binary literal op2 into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `loadsettp`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0206` | `{REG,INT,INT}` | load register and sets externally writable status flags load op1=op2 (op1.flags = op3) |
| `0x0207` | `{REG,FLOAT,INT}` | load register and sets externally writable status flags load op1=op2 (op1.flags = op3) |
| `0x0208` | `{REG,STRING,INT}` | load register and sets externally writable status flags load op1=op2 (op1.flags = op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `move`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01fd` | `{REG,REG}` | Move op2 to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `null`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x000e` | `{REG}` | Null (clear) op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

# Program Control And Calls

Branches, counted loops, calls, returns, exits, and no-op control instructions.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction placeholders in this section: 20.

## Section Checklist

- Purpose overview: TODO
- Operand conventions: TODO
- Signal/error behavior: TODO
- Examples: TODO
- Cross-links to related instructions: TODO

## `bcf`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x015e` | `{ID,REG}` | if op2=0 goto op1(if false) else dec op2 |
| `0x015f` | `{ID,REG,REG}` | if op2=0 goto op1(if false) else dec op2 and inc op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `bct`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0159` | `{ID,REG}` | dec op2; if op2>0; goto op1(if true) |
| `0x015a` | `{ID,REG,REG}` | dec op2; inc op3, if op2>0; goto op1(if true) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `bctnm`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x015b` | `{ID,REG}` | dec op2; if op2>=0; goto op1(if true) |
| `0x015c` | `{ID,REG,REG}` | dec op2; inc op3, if op2>=0; goto op1(if true) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `bctp`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x015d` | `{ID,REG}` | inc op2; goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `beq`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0028` | `{ID,REG,REG}` | if op2==op3 then goto op1 |
| `0x0029` | `{ID,REG,INT}` | if op2==op3 then goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `bge`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0162` | `{ID,REG,REG}` | if op2>=op3 then goto op1 |
| `0x0163` | `{ID,REG,INT}` | if op2>=op3 then goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `bgt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0160` | `{ID,REG,REG}` | if op2>op3 then goto op1 |
| `0x0161` | `{ID,REG,INT}` | if op2>op3 then goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ble`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0166` | `{ID,REG,REG}` | if op2<=op3 then goto op1 |
| `0x0167` | `{ID,REG,INT}` | if op2<=op3 then goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `blt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0164` | `{ID,REG,REG}` | if op2<op3 then goto op1 |
| `0x0165` | `{ID,REG,INT}` | if op2<op3 then goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `bne`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x002a` | `{ID,REG,REG}` | if op2!=op3 then goto op1 |
| `0x002b` | `{ID,REG,INT}` | if op2!=op3 then goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `br`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0024` | `{ID}` | Branch to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `brf`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0026` | `{ID,REG}` | Branch to op1 if op2 false |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `brt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0025` | `{ID,REG}` | Branch to op1 if op2 true |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `brtf`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0027` | `{ID,ID,REG}` | Branch to op1 if op3 true, otherwise branch to op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `brtpandt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x020c` | `{ID,REG,INT}` | if op2 readable status flags & op3 true then goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `brtpt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x020b` | `{ID,REG}` | if op2 has externally writable status flags then goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `call`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x002c` | `{FUNC}` | Call procedure (op1()) |
| `0x002d` | `{REG,FUNC}` | Call procedure (op1=op2()) |
| `0x002e` | `{REG,FUNC,REG}` | Call procedure (op1=op2(op3...) ) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `cnop`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x021c` | `no operand` | no operation |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `exit`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0035` | `no operand` | Exit |
| `0x0036` | `{REG}` | Exit op1 |
| `0x0037` | `{INT}` | Exit op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ret`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0030` | `no operand` | Return VOID |
| `0x0031` | `{REG}` | Return op1 |
| `0x0032` | `{INT}` | Return op1 |
| `0x0033` | `{FLOAT}` | Return op1 |
| `0x0034` | `{STRING}` | Return op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

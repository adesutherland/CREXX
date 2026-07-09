# Strings And Characters

String comparison, concatenation, slicing, character access, trimming, translation, and UTF-facing helpers.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction placeholders in this section: 34.

## Section Checklist

- Purpose overview: TODO
- Operand conventions: TODO
- Signal/error behavior: TODO
- Examples: TODO
- Cross-links to related instructions: TODO

## `append`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x008f` | `{REG,REG}` | String Append (op1=op1||op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `appendchar`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x008c` | `{REG,REG}` | Append Concat Char op2 (as int) on op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `concat`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0086` | `{REG,REG,REG}` | String Concat (op1=op2||op3) |
| `0x0087` | `{REG,REG,STRING}` | String Concat (op1=op2||op3) |
| `0x0088` | `{REG,STRING,REG}` | String Concat (op1=op2||op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `concchar`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x008d` | `{REG,REG,REG}` | Concat Char op1 from op2 position op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `getstrpos`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x009b` | `{REG,REG}` | Get String (op2) charpos into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `hexchar`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0098` | `{REG,REG,REG}` | op1 (as hex) = op2[op3] |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `padstr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00a3` | `{REG,REG,REG}` | set op1=op2[repeated op3 times] |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `poschar`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0099` | `{REG,REG,REG}` | op1 = position of op3 in op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sappend`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x008e` | `{REG,REG}` | String Append with space (op1=op1||op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sconcat`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0089` | `{REG,REG,REG}` | String Concat with space (op1=op2||op3) |
| `0x008a` | `{REG,REG,STRING}` | String Concat with space (op1=op2||op3) |
| `0x008b` | `{REG,STRING,REG}` | String Concat with space (op1=op2||op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `scopy`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x000b` | `{REG,REG}` | Copy String op2 to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `seq`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0074` | `{REG,REG,REG}` | String Equals op1=(op2==op3) |
| `0x0075` | `{REG,REG,STRING}` | String Equals op1=(op2==op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setstrpos`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x009a` | `{REG,REG}` | Set String (op1) charpos set to op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sgt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x007a` | `{REG,REG,REG}` | String Greater than op1=(op2>op3) |
| `0x007b` | `{REG,REG,STRING}` | String Greater than op1=(op2>op3) |
| `0x007c` | `{REG,STRING,REG}` | String Greater than op1=(op2>op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sgte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x007d` | `{REG,REG,REG}` | String Greater than equals op1=(op2>=op3) |
| `0x007e` | `{REG,REG,STRING}` | String Greater than equals op1=(op2>=op3) |
| `0x007f` | `{REG,STRING,REG}` | String Greater than equals op1=(op2>=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `slt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0080` | `{REG,REG,REG}` | String Less than op1=(op2<op3) |
| `0x0081` | `{REG,REG,STRING}` | String Less than op1=(op2<op3) |
| `0x0082` | `{REG,STRING,REG}` | String Less than op1=(op2<op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `slte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0083` | `{REG,REG,REG}` | String Less than equals op1=(op2<=op3) |
| `0x0084` | `{REG,REG,STRING}` | String Less than equals op1=(op2<=op3) |
| `0x0085` | `{REG,STRING,REG}` | String Less than equals op1=(op2<=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sne`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0078` | `{REG,REG,REG}` | String Not equals op1=(op2!=op3) |
| `0x0079` | `{REG,REG,STRING}` | String Not equals op1=(op2!=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `stob`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00eb` | `{REG}` | Set register boolean (int 1 or 0) value from its string value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `stof`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00ec` | `{REG}` | Set register float value from its string value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `stoi`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00ed` | `{REG}` | Set register int value from its string value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `strchar`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0096` | `{REG,REG,REG}` | op1 (as int) = op2[op3] |
| `0x0097` | `{REG,REG}` | op1 (as int) = op2[charpos] |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `strlen`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0095` | `{REG,REG}` | String Length op1 = length(op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `strlower`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x009d` | `{REG,REG}` | Set string to lower case value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `strpos`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00a4` | `{REG,REG,REG}` | op1 is position of op2 in op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `strupper`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x009e` | `{REG,REG}` | Set string to upper case value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `substcut`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00a2` | `{REG,REG}` | set op1=substr(op1,,op2) cuts off op1 after position op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `substr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x009c` | `{REG,REG,REG}` | op1 = op2[charpos]...op2[charpos+op3-1] |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `substring`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00a1` | `{REG,REG,REG}` | set op1=substr(op2,op3) remaining string |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `swap`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01fe` | `{REG,REG}` | Swap op1 and op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `transchar`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x009f` | `{REG,REG,REG}` | replace op1 if it is in op3-list by char in op2-list |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `triml`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0090` | `{REG,REG}` | Trim String (op1) from Left by (op2) Chars |
| `0x0092` | `{REG,REG,REG}` | Trim String (op2) from Left by (op3) Chars into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `trimr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0091` | `{REG,REG}` | Trim String (op1) from Right by (op2) Chars |
| `0x0093` | `{REG,REG,REG}` | Trim String (op2) from Right by (op3) Chars into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `trunc`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0094` | `{REG,REG,REG}` | Trunc String (op2) to (op3) Chars into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

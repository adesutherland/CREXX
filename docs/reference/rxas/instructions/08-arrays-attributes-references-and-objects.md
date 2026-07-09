# Arrays, Attributes, References, And Objects

Attribute arrays, references, redirection, object type metadata, and initialization checks.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction placeholders in this section: 32.

## Section Checklist

- Purpose overview: TODO
- Operand conventions: TODO
- Signal/error behavior: TODO
- Examples: TODO
- Cross-links to related instructions: TODO

## `arr2redir`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01d6` | `{REG,REG}` | Redirect op1 <- array op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `assertinitialized`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0055` | `{REG}` | Assert op1 is initialized, else signal OBJECT_NOT_INITIALIZED |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `delattrs`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x022b` | `{REG,REG,REG}` | Delete op3 attributes from op1 starting at zero-based index op2 |
| `0x022c` | `{REG,REG,INT}` | Delete op3 attributes from op1 starting at zero-based index op2 |
| `0x022d` | `{REG,INT,REG}` | Delete op3 attributes from op1 starting at zero-based index op2 |
| `0x022e` | `{REG,INT,INT}` | Delete op3 attributes from op1 starting at zero-based index op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `delattrs1`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x022f` | `{REG,REG,REG}` | Delete op3 attributes from op1 starting at one-based index op2 |
| `0x0230` | `{REG,REG,INT}` | Delete op3 attributes from op1 starting at one-based index op2 |
| `0x0231` | `{REG,INT,REG}` | Delete op3 attributes from op1 starting at one-based index op2 |
| `0x0232` | `{REG,INT,INT}` | Delete op3 attributes from op1 starting at one-based index op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `deref`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00c3` | `{REG,REG}` | Copy referenced storage op2 into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `getabufs`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00dc` | `{REG,REG}` | get attribute buffer size op1 = op2.max_attributes |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `getattrs`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00d6` | `{REG,REG}` | get number attributes op1 = op2.num_attributes |
| `0x00d7` | `{REG,REG,INT}` | get number attributes op1 = op2.num_attributes + op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `insattrs`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0223` | `{REG,REG,REG}` | Insert op3 attributes into op1 before zero-based index op2 |
| `0x0224` | `{REG,REG,INT}` | Insert op3 attributes into op1 before zero-based index op2 |
| `0x0225` | `{REG,INT,REG}` | Insert op3 attributes into op1 before zero-based index op2 |
| `0x0226` | `{REG,INT,INT}` | Insert op3 attributes into op1 before zero-based index op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `insattrs1`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0227` | `{REG,REG,REG}` | Insert op3 attributes into op1 before one-based index op2 |
| `0x0228` | `{REG,REG,INT}` | Insert op3 attributes into op1 before one-based index op2 |
| `0x0229` | `{REG,INT,REG}` | Insert op3 attributes into op1 before one-based index op2 |
| `0x022a` | `{REG,INT,INT}` | Insert op3 attributes into op1 before one-based index op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `isinitialized`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0056` | `{REG,REG}` | Set op1 boolean if op2 is initialized |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `link`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00d4` | `{REG,REG}` | Link op2 to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `linkarg`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01ff` | `{REG,INT}` | Link args[op2] to op1 |
| `0x0200` | `{REG,REG,INT}` | Link args[op2+op3] to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `linkattr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00c8` | `{REG,REG,REG}` | Link attribute op3 of op2 to op1 |
| `0x00c9` | `{REG,REG,INT}` | Link attribute op3 of op2 to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `linkattr1`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00ca` | `{REG,REG,REG}` | Link attribute op3 (1 base) of op2 to op1 |
| `0x00cb` | `{REG,REG,INT}` | Link attribute op3 (1 base) of op2 to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `linkref`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00c4` | `{REG,REG}` | Link op1 to referenced storage op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `linktoattr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00cc` | `{REG,REG,REG}` | Link op3 to attribute op1 of op2 |
| `0x00cd` | `{INT,REG,REG}` | Link op3 to attribute op1 of op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `linktoattr1`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00ce` | `{REG,REG,REG}` | Link op3 to attribute op1 (1 base) of op2 |
| `0x00cf` | `{INT,REG,REG}` | Link op3 to attribute op1 (1 base) of op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `minattrs`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00dd` | `{REG,REG}` | ensure min number attributes op1.num_attributes >= op2 |
| `0x00de` | `{REG,INT}` | ensure min number attributes op1.num_attributes >= op2 |
| `0x00df` | `{REG,REG,INT}` | ensure min number attributes op1.num_attributes >= op2 + op3 |
| `0x00e0` | `{REG,INT,INT}` | ensure min number attributes op1.num_attributes >= op2 + op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `mkref`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00c2` | `{REG,REG}` | Create reference value op1 to storage op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `nullredir`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01d7` | `{REG}` | Redirect op1 = to/from null |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `redir2arr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01d4` | `{REG,REG}` | Redirect op1 -> array op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `redir2str`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01d3` | `{REG,REG}` | Redirect op1 -> string op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `refvalid`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00c6` | `{REG,REG}` | Set op1 to 1 if reference op2 is valid, else 0 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setattrs`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00d8` | `{REG,REG}` | set number attributes op1.num_attributes = op2 |
| `0x00d9` | `{REG,INT}` | set number attributes op1.num_attributes = op2 |
| `0x00da` | `{REG,REG,INT}` | set number attributes op1.num_attributes = op2 + op3 |
| `0x00db` | `{REG,INT,INT}` | set number attributes op1.num_attributes = op2 + op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setobjtype`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0038` | `{REG,STRING}` | Set runtime concrete object type metadata on op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setobjuninit`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0054` | `{REG,STRING}` | Set runtime object type metadata on op1 and mark it uninitialized |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setref`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00c5` | `{REG,REG}` | Copy op2 into referenced storage op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `str2redir`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01d5` | `{REG,REG}` | Redirect op1 <- string op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `unlink`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00d5` | `{REG}` | Unlink op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `unlinkattr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00d0` | `{REG,REG}` | Unlink attribute op1 of op2 |
| `0x00d1` | `{INT,REG}` | Unlink attribute op1 of op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `unlinkattr1`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00d2` | `{REG,REG}` | Unlink attribute op1 (1 base) of op2 |
| `0x00d3` | `{INT,REG}` | Unlink attribute op1 (1 base) of op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `unref`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00c7` | `{REG}` | Clear reference value op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

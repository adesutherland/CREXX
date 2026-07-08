# Metadata And Introspection

Program/module metadata loading, procedure metadata, and runtime type/introspection helpers.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction placeholders in this section: 23.

## Section Checklist

- Purpose overview: TODO
- Operand conventions: TODO
- Signal/error behavior: TODO
- Examples: TODO
- Cross-links to related instructions: TODO

## `asserttype`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x003d` | `{REG,STRING}` | Assert op1 matches the requested type name op2, else signal CONVERSION_ERROR |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `getandtp`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x020a` | `{REG,REG,INT}` | get readable register status flags with mask (op1(int) = op2.flags & op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `gettp`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0204` | `{REG,REG}` | gets the readable register status flags (op1 = op2.flags) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `istype`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x003c` | `{REG,REG,STRING}` | Set op1 boolean if op2 matches the requested type name op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metadecodeinst`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0212` | `{REG,REG}` | Decode opcode (op1 decoded op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metalinkpreg`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0218` | `{REG,REG}` | Link parent-frame-register[op2] to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloadcalleraddr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0219` | `{REG}` | Load caller address object to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloaddata`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0217` | `{REG,REG,REG}` | Load Metadata (op1 = (metadata)op2[op3]) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloadedeprocs`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0210` | `{REG,REG}` | Loaded Exposed Procedures (op1 = array procedures in module op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloadedmodules`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x020e` | `{REG}` | Loaded Modules (op1 = array loaded modules) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloadedprocs`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x020f` | `{REG,REG}` | Loaded Procedures (op1 = array procedures in module op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloadfoperand`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0214` | `{REG,REG,REG}` | Load Float Operand (op1 = (float)op2[op3]) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloadinst`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0211` | `{REG,REG,REG}` | Load Instruction Code (op1 = (inst)op2[op3]) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloadioperand`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0213` | `{REG,REG,REG}` | Load Integer/Index Operand (op1 = (int)op2[op3]) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloadmodule`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x020d` | `{REG,REG}` | Load Module (op1 = module num of last loaded module in rxbin op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloadpoperand`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0216` | `{REG,REG,REG}` | Load Procedure Operand (op1 = (proc)op2[op3]) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `metaloadsoperand`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0215` | `{REG,REG,REG}` | Load String Operand (op1 = (string)op2[op3]) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setortp`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0209` | `{REG,INT}` | or externally writable register status flags (op1.flags = op1.flags | op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `settp`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0205` | `{REG,INT}` | sets externally writable register status flags, preserving VM-private flags |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `settpmask`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0203` | `{REG,REG,INT}` | masked replace of source-writable status flags (op1.flags=(op1.flags&~op3)|(op2&op3)) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `srcfprocsel`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x003a` | `{REG,STRING,REG}` | Resolve an interface factory procedure from descriptor op2 and argument list op3 into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `srcmethodsel`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0039` | `{REG,REG,STRING}` | Resolve a concrete method procedure from object op2 and descriptor op3 into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `typeof`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x003b` | `{REG,REG}` | Resolve the canonical source type name of op2 into string register op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

# Signals, Breakpoints, And Runtime

Signal handler control, breakpoints, runtime version/hash, and lifecycle helpers.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction placeholders in this section: 19.

## Section Checklist

- Purpose overview: TODO
- Operand conventions: TODO
- Signal/error behavior: TODO
- Examples: TODO
- Cross-links to related instructions: TODO

## `bpoff`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01ef` | `no operand` | Disable Breakpoints |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `bpon`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01ed` | `{FUNC}` | Enable Breakpoints with op1 handler |
| `0x01ee` | `no operand` | Enable Breakpoints with existing handler |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `endlife`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0202` | `{REG}` | End storage reference lifetime for op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `rxhash`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0201` | `{REG,REG,REG}` | returns hash value, etc, op1=hash(op2,len(op3)) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `rxvers`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01cd` | `{REG}` | get version, op1=version details |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sigbr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01f3` | `{ID,STRING}` | Set Signal op2 Handle to Branch to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sigbrv`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0222` | `{ID,REG,STRING}` | Set Signal op3 Handle to Branch to op1 and bind signal object to op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sigcall`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01f4` | `{FUNC,STRING}` | Set Signal op2 Handle to Call op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sigcalla`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x021d` | `{FUNC,STRING}` | Set Signal op2 Handle to action-aware Call op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sigcallbr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01f5` | `{ID,FUNC,STRING}` | Set Signal op3 Handle to Call op2 returning to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sighalt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01f1` | `{STRING}` | Set Signal op1 Handle to Halt |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sigignore`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01f0` | `{STRING}` | Set Signal op1 Handle to Ignore |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `signal`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01e6` | `{STRING}` | Signal type op1 |
| `0x01e9` | `{STRING,STRING}` | Signal type op1 (message op2) |
| `0x01ea` | `{STRING,REG}` | Signal type op1 (payload op2) |
| `0x021e` | `{REG}` | Signal type from op1 string register |
| `0x021f` | `{REG,REG}` | Signal type from op1 string register with payload op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `signalf`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01e8` | `{STRING,REG}` | Signal type op1 if op2 false |
| `0x01ec` | `{STRING,STRING,REG}` | Signal type op1 (message op2) if op3 false |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `signalt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01e7` | `{STRING,REG}` | Signal type op1 if op2 true |
| `0x01eb` | `{STRING,STRING,REG}` | Signal type op1 (message op2) if op3 true |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sigpop`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0221` | `{STRING}` | Pop and restore current Signal op1 handler from the current frame handler stack |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sigpush`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0220` | `{STRING}` | Push current Signal op1 handler on the current frame handler stack |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sigret`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01f6` | `{STRING}` | Set Signal op1 Handle to Return |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sigshalt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01f2` | `{STRING}` | Set Signal op1 Handle to Silent Halt |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

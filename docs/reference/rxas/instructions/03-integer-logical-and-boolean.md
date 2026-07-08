# Integer, Logical, And Boolean

Fixed-width integer arithmetic, integer comparisons, bitwise logic, boolean conversion, and integer range helpers.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction placeholders in this section: 44.

## Section Checklist

- Purpose overview: TODO
- Operand conventions: TODO
- Signal/error behavior: TODO
- Examples: TODO
- Cross-links to related instructions: TODO

## `and`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00b4` | `{REG,REG,REG}` | Logical (int) and op1=(op2 && op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `btof`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00e3` | `{REG}` | Set register float value from its boolean value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `btoi`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00e1` | `{REG}` | Set register integer value from its boolean value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `btos`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00e4` | `{REG}` | Set register string value from its boolean value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `iadd`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x000f` | `{REG,REG,REG}` | Integer Add (op1=op2+op3) |
| `0x0010` | `{REG,REG,INT}` | Integer Add (op1=op2+op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `iand`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00a8` | `{REG,REG,REG}` | bit wise and of 2 integers (op1=op2&op3) |
| `0x00a9` | `{REG,REG,INT}` | bit wise and of 2 integers (op1=op2&op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ichkrng`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01f7` | `{REG,INT,INT}` | if op1<op2 | op1>op3 signal OUT_OF_RANGE |
| `0x01f8` | `{REG,INT,REG}` | if op1<op2 | op1>op3 signal OUT_OF_RANGE |
| `0x01f9` | `{REG,REG,REG}` | if op1<op2 | op1>op3 signal OUT_OF_RANGE |
| `0x01fa` | `{INT,INT,REG}` | if op1<op2 | op1>op3 signal OUT_OF_RANGE |
| `0x01fb` | `{INT,REG,REG}` | if op1<op2 | op1>op3 signal OUT_OF_RANGE |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `icopy`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0009` | `{REG,REG}` | Copy Integer op2 to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `idiv`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0016` | `{REG,REG,REG}` | Integer Divide (op1=op2/op3) |
| `0x0017` | `{REG,REG,INT}` | Integer Divide (op1=op2/op3) |
| `0x0018` | `{REG,INT,REG}` | Integer Divide (op1=op2/op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ieq`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0064` | `{REG,REG,REG}` | Int Equals op1=(op2==op3) |
| `0x0065` | `{REG,REG,INT}` | Int Equals op1=(op2==op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `igt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0068` | `{REG,REG,REG}` | Int Greater than op1=(op2>op3) |
| `0x0069` | `{REG,REG,INT}` | Int Greater than op1=(op2>op3) |
| `0x006a` | `{REG,INT,REG}` | Int Greater than op1=(op2>op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `igtbr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0157` | `{ID,REG,REG}` | Int Greater than if (op2>op3) goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `igte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x006b` | `{REG,REG,REG}` | Int Greater than equals op1=(op2>=op3) |
| `0x006c` | `{REG,REG,INT}` | Int Greater than equals op1=(op2>=op3) |
| `0x006d` | `{REG,INT,REG}` | Int Greater than equals op1=(op2>=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ilt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x006e` | `{REG,REG,REG}` | Int Less than op1=(op2<op3) |
| `0x006f` | `{REG,REG,INT}` | Int Less than op1=(op2<op3) |
| `0x0070` | `{REG,INT,REG}` | Int Less than op1=(op2<op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `iltbr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0158` | `{ID,REG,REG}` | Int Less than if (op2<op3) goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ilte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0071` | `{REG,REG,REG}` | Int Less than equals op1=(op2<=op3) |
| `0x0072` | `{REG,REG,INT}` | Int Less than equals op1=(op2<=op3) |
| `0x0073` | `{REG,INT,REG}` | Int Less than equals op1=(op2<=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `imod`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0019` | `{REG,REG,REG}` | Integer Modulo (op1=op2%op3) |
| `0x001a` | `{REG,REG,INT}` | Integer Modulo (op1=op2%op3) |
| `0x001b` | `{REG,INT,REG}` | Integer Modulo (op1=op2&op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `imult`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0014` | `{REG,REG,REG}` | Integer Multiply (op1=op2*op3) |
| `0x0015` | `{REG,REG,INT}` | Integer Multiply (op1=op2*op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `inc`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x001c` | `{REG}` | Increment Int (op1++) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `inc0`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x001e` | `no operand` | Increment R0++ Int |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `inc1`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0020` | `no operand` | Increment R1++ Int |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `inc2`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0022` | `no operand` | Increment R2++ Int |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ine`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0066` | `{REG,REG,REG}` | Int Not equals op1=(op2!=op3) |
| `0x0067` | `{REG,REG,INT}` | Int Not equals op1=(op2!=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `inot`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00b2` | `{REG,REG}` | inverts all bits of an integer (op1=~op2) |
| `0x00b3` | `{REG,INT}` | inverts all bits of an integer (op1=~op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ior`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00aa` | `{REG,REG,REG}` | bit wise or of 2 integers (op1=op2|op3) |
| `0x00ab` | `{REG,REG,INT}` | bit wise or of 2 integers (op1=op2|op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ipow`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0153` | `{REG,REG,REG}` | op1=op2**op3 |
| `0x0154` | `{REG,REG,INT}` | op1=op2**op3 |
| `0x0155` | `{REG,INT,REG}` | op1=op2**op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `irand`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01d0` | `{REG,REG}` | random number random, op1=irand(op2) |
| `0x01d1` | `{REG,INT}` | random number random, op1=irand(op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `isex`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0156` | `{REG}` | dec op1 = -op1 (sign change) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ishl`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00ae` | `{REG,REG,REG}` | bit wise shift logical left of integer (op1=op2<<op3) |
| `0x00af` | `{REG,REG,INT}` | bit wise shift logical left of integer (op1=op2<<op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ishr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00b0` | `{REG,REG,REG}` | bit wise shift logical right of integer (op1=op2>>op3) |
| `0x00b1` | `{REG,REG,INT}` | bit wise shift logical right of integer (op1=op2>>op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `isub`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0011` | `{REG,REG,REG}` | Integer Subtract (op1=op2-op3) |
| `0x0012` | `{REG,REG,INT}` | Integer Subtract (op1=op2-op3) |
| `0x0013` | `{REG,INT,REG}` | Integer Subtract (op1=op2-op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `itob`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00ea` | `{REG}` | Set register boolean (int 1 or 0) value from its integer value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `itof`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00e7` | `{REG}` | Set register float value from its int value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `itos`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00e5` | `{REG}` | Set register string value from its int value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ixor`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00ac` | `{REG,REG,REG}` | bit wise exclusive OR of 2 integers (op1=op2^op3) |
| `0x00ad` | `{REG,REG,INT}` | bit wise exclusive OR of 2 integers (op1=op2^op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `not`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00b6` | `{REG,REG}` | Logical (int) not op1=!op2 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `or`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00b5` | `{REG,REG,REG}` | Logical (int) or op1=(op2 || op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `req`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0233` | `{REG,REG,REG}` | Loose REXX Equals op1=(op2=op3) |
| `0x0234` | `{REG,REG,STRING}` | Loose REXX Equals op1=(op2=op3) |
| `0x0235` | `{REG,STRING,REG}` | Loose REXX Equals op1=(op2=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `rgt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0239` | `{REG,REG,REG}` | Loose REXX Greater than op1=(op2>op3) |
| `0x023a` | `{REG,REG,STRING}` | Loose REXX Greater than op1=(op2>op3) |
| `0x023b` | `{REG,STRING,REG}` | Loose REXX Greater than op1=(op2>op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `rgte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x023c` | `{REG,REG,REG}` | Loose REXX Greater than equals op1=(op2>=op3) |
| `0x023d` | `{REG,REG,STRING}` | Loose REXX Greater than equals op1=(op2>=op3) |
| `0x023e` | `{REG,STRING,REG}` | Loose REXX Greater than equals op1=(op2>=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `rlt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x023f` | `{REG,REG,REG}` | Loose REXX Less than op1=(op2<op3) |
| `0x0240` | `{REG,REG,STRING}` | Loose REXX Less than op1=(op2<op3) |
| `0x0241` | `{REG,STRING,REG}` | Loose REXX Less than op1=(op2<op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `rlte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0242` | `{REG,REG,REG}` | Loose REXX Less than equals op1=(op2<=op3) |
| `0x0243` | `{REG,REG,STRING}` | Loose REXX Less than equals op1=(op2<=op3) |
| `0x0244` | `{REG,STRING,REG}` | Loose REXX Less than equals op1=(op2<=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `rne`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0236` | `{REG,REG,REG}` | Loose REXX Not equals op1=(op2<>op3) |
| `0x0237` | `{REG,REG,STRING}` | Loose REXX Not equals op1=(op2<>op3) |
| `0x0238` | `{REG,STRING,REG}` | Loose REXX Not equals op1=(op2<>op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `rseq`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0076` | `{REG,REG,REG}` | non strict String Equals op1=(op2=op3) |
| `0x0077` | `{REG,REG,STRING}` | non strict String Equals op1=(op2=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

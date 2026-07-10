# Program Control And Calls

Branches, counted loops, calls, returns, exits, and no-op control instructions.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction entries in this section: 24.

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

## `jumpb`

Status: documented.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0278` | `{REG,BINARY}` | Jump through table op2 using binary register op1 |

Human reference content:

- Purpose: dispatches through an assembled jump table using the whole logical
  binary value in `op1` as the lookup key.
- Operand notes: `op2` is a procedure-local `.jtable` name. The table cases
  must be binary literals decorated with `.jcase`.
- Result and side effects: on match, the VM branches to the case target; on
  miss, execution continues at the next instruction. The source register and
  table constant are not modified.
- Signals/errors: malformed packed table data raises `RXBIN_CORRUPTION`.
- Example:

```rxas
main() .locals=3
    .jtable tokens linear
    br after_cases
tok_ab: .jcase tokens 0x6162
    load r1,1
    ret
after_cases:
    load r0,0x6162
    jumpb r0,tokens
    load r1,0
    ret
```

- Related instructions: `jumps`, `jumpbs`, `jumpi`, `.jtable`, `.jcase`.

## `jumpbs`

Status: documented.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0279` | `{REG,REG,BINARY}` | Jump through fixed-width table op3 using binary register op1 at byte offset op2 |

Human reference content:

- Purpose: dispatches through an assembled jump table using a zero-copy fixed
  width slice of a binary register.
- Operand notes: `op1` is the binary source, `op2.int_value` is the byte
  offset, and `op3` is a procedure-local `.jtable` name. The table must have
  fixed-length, non-empty binary literal keys; the packed table key length is
  the slice length.
- Result and side effects: on match, the VM branches to the case target; on
  miss, execution continues at the next instruction. No slice register is
  allocated.
- Signals/errors: negative offsets, variable-width tables, zero-width tables,
  or slices outside the source logical binary length raise `OUT_OF_RANGE`.
  Malformed packed table data raises `RXBIN_CORRUPTION`.
- Example:

```rxas
main() .locals=4
    .jtable pairs linear
    br after_cases
tok_bc: .jcase pairs 0x6263
    load r2,1
    ret
after_cases:
    load r0,0x616263
    load r1,1
    jumpbs r0,r1,pairs
    load r2,0
    ret
```

- Related instructions: `jumpb`, `jumps`, `jumpi`, `bcopy`, `bslice`.

## `jumpi`

Status: documented.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x027a` | `{REG,BINARY}` | Jump through table op2 using integer register op1 |

Human reference content:

- Purpose: dispatches through an assembled jump table using `op1.int_value` as
  the lookup key.
- Operand notes: integer keys are canonicalized to eight little-endian bytes.
  `op2` is a procedure-local `.jtable` name, and cases must be integer
  literals decorated with `.jcase`.
- Result and side effects: on match, the VM branches to the case target; on
  miss, execution continues at the next instruction.
- Signals/errors: malformed packed table data raises `RXBIN_CORRUPTION`.
- Example:

```rxas
main() .locals=3
    .jtable choices auto
    br after_cases
case_two: .jcase choices 2
    load r1,20
    ret
after_cases:
    load r0,2
    jumpi r0,choices
    load r1,0
    ret
```

- Related instructions: `jumps`, `jumpb`, `jumpbs`.

## `jumps`

Status: documented.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0277` | `{REG,BINARY}` | Jump through table op2 using string register op1 |

Human reference content:

- Purpose: dispatches through an assembled jump table using the UTF-8 bytes of
  a string register as the lookup key.
- Operand notes: `op2` is a procedure-local `.jtable` name. Table cases must be
  string literals decorated with `.jcase`; no NUL terminator is included in the
  key.
- Result and side effects: on match, the VM branches to the case target; on
  miss, execution continues at the next instruction.
- Signals/errors: malformed packed table data raises `RXBIN_CORRUPTION`.
- Example:

```rxas
main() .locals=3
    .jtable keywords linear
    br after_cases
keyword_if: .jcase keywords "if"
    load r1,1
    ret
after_cases:
    load r0,"if"
    jumps r0,keywords
    load r1,0
    ret
```

- Related instructions: `jumpb`, `jumpbs`, `jumpi`, `.jtable`, `.jcase`.

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

# Decimal And Numeric Settings

Decimal arithmetic plus decimal/numeric context inspection and update instructions.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction placeholders in this section: 46.

## Section Checklist

- Purpose overview: TODO
- Operand conventions: TODO
- Signal/error behavior: TODO
- Examples: TODO
- Cross-links to related instructions: TODO

## `btod`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00e2` | `{REG}` | Set register decimal value from its boolean value |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dadd`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0168` | `{REG,REG,REG}` | Decimal Add (op1=op2-op3) |
| `0x0169` | `{REG,REG,DECIMAL}` | Decimal Add (op1=op2-op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dcall`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x002f` | `{REG,REG,REG}` | Dynamic call procedure (op1=op2(op3...) ) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dcopy`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x000c` | `{REG,REG}` | Copy Decimal op2 to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ddiv`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x016f` | `{REG,REG,REG}` | Decimal Divide (op1=op2/op3) |
| `0x0170` | `{REG,REG,DECIMAL}` | Decimal Divide (op1=op2/op3) |
| `0x0171` | `{REG,DECIMAL,REG}` | Decimal Divide (op1=op2/op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dec`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x001d` | `{REG}` | Decrement Int (op1--) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dec0`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x001f` | `no operand` | Decrement R0-- Int |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dec1`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0021` | `no operand` | Decrement R1-- Int |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dec2`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0023` | `no operand` | Decrement R2-- Int |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `decplnm`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0190` | `{REG,REG,REG}` | Get Decimal Plugin Name op1=name op2=description op3=version |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `deq`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0178` | `{REG,REG,REG}` | Decimal Equals op1=(op2==op3) |
| `0x0179` | `{REG,REG,DECIMAL}` | Decimal Equals op1=(op2==op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `deqbr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x018a` | `{ID,REG,REG}` | Decimal Equal if (op2=op3) goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dextr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x018f` | `{REG,REG,REG}` | op3 decimal extracted to op1 string coefficient and op2 int decimal exponent |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dgt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x017c` | `{REG,REG,REG}` | Decimal Greater than op1=(op2>op3) |
| `0x017d` | `{REG,REG,DECIMAL}` | Decimal Greater than op1=(op2>op3) |
| `0x017e` | `{REG,DECIMAL,REG}` | Decimal Greater than op1=(op2>op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dgtbr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0188` | `{ID,REG,REG}` | Decimal Greater than if (op2>op3) goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dgte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x017f` | `{REG,REG,REG}` | Decimal Greater than equals op1=(op2>=op3) |
| `0x0180` | `{REG,REG,DECIMAL}` | Decimal Greater than equals op1=(op2>=op3) |
| `0x0181` | `{REG,DECIMAL,REG}` | Decimal Greater than equals op1=(op2>=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `didiv`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0172` | `{REG,REG,REG}` | Integer Divide with Decimals (op1=op2/op3) |
| `0x0173` | `{REG,REG,DECIMAL}` | Integer Divide with Decimals (op1=op2/op3) |
| `0x0174` | `{REG,DECIMAL,REG}` | Integer Divide with Decimals (op1=op2/op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dlt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0182` | `{REG,REG,REG}` | Decimal Less than op1=(op2<op3) |
| `0x0183` | `{REG,REG,DECIMAL}` | Decimal Less than op1=(op2<op3) |
| `0x0184` | `{REG,DECIMAL,REG}` | Decimal Less than op1=(op2<op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dltbr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0189` | `{ID,REG,REG}` | Decimal Less than if (op2<op3) goto op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dlte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0185` | `{REG,REG,REG}` | Decimal Less than equals op1=(op2<=op3) |
| `0x0186` | `{REG,REG,DECIMAL}` | Decimal Less than equals op1=(op2<=op3) |
| `0x0187` | `{REG,DECIMAL,REG}` | Decimal Less than equals op1=(op2<=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dmod`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0175` | `{REG,REG,REG}` | Decimal Modulo (op1=op2%op3) |
| `0x0176` | `{REG,REG,DECIMAL}` | Decimal Modulo (op1=op2%op3) |
| `0x0177` | `{REG,DECIMAL,REG}` | Decimal Modulo (op1=op2&op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dmult`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x016d` | `{REG,REG,REG}` | Decimal Multiply (op1=op2*op3) |
| `0x016e` | `{REG,REG,DECIMAL}` | Decimal Multiply (op1=op2*op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dne`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x017a` | `{REG,REG,REG}` | Decimal Not equals op1=(op2!=op3) |
| `0x017b` | `{REG,REG,DECIMAL}` | Decimal Not equals op1=(op2!=op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dpow`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x018b` | `{REG,REG,REG}` | op1=op2**op3 |
| `0x018c` | `{REG,REG,DECIMAL}` | op1=op2**op3 |
| `0x018d` | `{REG,DECIMAL,REG}` | op1=op2**op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dropchar`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00a0` | `{REG,REG,REG}` | set op1 from op2 after dropping all chars from op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dsex`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x018e` | `{REG}` | Decimal op1 = -op1 (sign change) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dsub`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x016a` | `{REG,REG,REG}` | Decimal Subtract (op1=op2-op3) |
| `0x016b` | `{REG,REG,DECIMAL}` | Decimal Subtract (op1=op2-op3) |
| `0x016c` | `{REG,DECIMAL,REG}` | Decimal Subtract (op1=op2-op3) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dtob`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00f1` | `{REG}` | Convert Decimal Number to Boolean op1=dec2s(op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dtof`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00f4` | `{REG}` | Convert Decimal Number to Float op1=f2dec(op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dtoi`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00f0` | `{REG}` | Convert Decimal Number to Integer op1=dec2s(op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `dtos`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00ef` | `{REG}` | Convert Decimal Number to Decimal String op1=dec2s(op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ftod`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00f3` | `{REG}` | Convert Float to Decimal Number op1=f2dec(op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `getnumcas`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0100` | `{REG}` | Get Numeric Case=op1 (1=lower,2=upper) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `getnumdgts`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00f7` | `{REG}` | Get Numeric Digits op1=digits |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `getnumfrm`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00fd` | `{REG}` | Get Numeric Form=op1 (1=sci,2=eng) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `getnumfuz`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00fa` | `{REG}` | Get Numeric Fuzz op1=digits |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `getnumstd`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0103` | `{REG}` | Get Numeric Standard=op1 (1=common,2=classic) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `itod`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00f2` | `{REG}` | Convert Integer to Decimal Number op1=s2dec(op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `numeng`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0105` | `{INT,INT,INT}` | Setup Engineering Numeric digits=op1, case=op2, std=op3, fuzz=0, form=eng |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `numsci`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0104` | `{INT,INT,INT}` | Setup Scientific Numeric digits=op1, case=op2, std=op3, fuzz=0, form=sci |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setnumcas`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00fe` | `{REG}` | Set Numeric Case=op1 (1=lower,2=upper) |
| `0x00ff` | `{INT}` | Set Numeric Case=op1 (1=lower,2=upper) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setnumdgts`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00f5` | `{REG}` | Set Numeric Digits digits=op1 (>4) |
| `0x00f6` | `{INT}` | Set Numeric Digits digits=op1 (>4) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setnumfrm`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00fb` | `{REG}` | Set Numeric Form=op1 (1=sci,2=eng) |
| `0x00fc` | `{INT}` | Set Numeric Form=op1 (1=sci,2=eng) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setnumfuz`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00f8` | `{REG}` | Set Numeric Fuzz digits=op1 (>=0) |
| `0x00f9` | `{INT}` | Set Numeric Fuzz digits=op1 (>=0) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `setnumstd`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0101` | `{REG}` | Set Numeric Standard=op1 (1=common,2=classic) |
| `0x0102` | `{INT}` | Set Numeric Standard=op1 (1=common,2=classic) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `stod`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x00ee` | `{REG}` | Convert Decimal String to Decimal Number op1=s2dec(op2) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

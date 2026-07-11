# Decimal And Numeric Settings

These instructions use the current frame's decimal plugin and numeric context
for decimal arithmetic, comparison, conversion, formatting, and settings. A
decimal literal uses RXAS decimal syntax such as `1d` or `1e3`. Unless an entry
states otherwise, plugin conditions are propagated as their corresponding VM
signal after the operation.

## `btod`

Convert an in-place Boolean integer to decimal.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e2` | `btod rValue` | Set the decimal payload to zero or one. |

### Operands And Semantics

Zero in the integer payload converts to decimal zero; any nonzero value converts
to decimal one under the current context. Only the decimal payload changes.

### Signals

Propagates decimal-plugin allocation or conversion signals.

### Example

<!-- rxas-example name="decimal-btod" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    btod r0
    ret
```

### Related

`itod`, `ftod`, `dtob`.

## `dadd`

Add two decimal values under the current numeric context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0168` | `dadd rResult,rLeft,rRight` | Add register decimals. |
| `0x0169` | `dadd rResult,rLeft,decimal` | Add a decimal literal. |

### Operands And Semantics

The destination decimal payload receives the context-rounded sum. Register
sources, scalar payloads, attributes, and cursors remain unchanged; destination
aliasing with a source is supported by the plugin.

### Signals

Propagates decimal-plugin conditions, including invalid operands and context
overflow/underflow.

### Example

<!-- rxas-example name="decimal-dadd" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1.5d
    dadd r0,r1,2.5d
    ret
```

### Related

`dsub`, `dmult`, `setnumdgts`.

## `dcall`

Call a procedure through an opaque runtime procedure pointer.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x002f` | `dcall rResult,rProcedure,rCount` | Invoke pointer with a contiguous argument block. |

### Operands And Semantics

`rProcedure.int` must hold a `proc_runtime` pointer, commonly produced by
`srcmethodsel` or `srcfprocsel`. `rCount.int` is the argument count and the
arguments occupy consecutive registers immediately after `rCount`. Native calls
copy their result into `rResult`; bytecode calls return into it after a new
frame whose arguments link to the caller registers. The pointer/count/arguments
are otherwise unchanged.

### Signals

Raises `FUNCTION_NOT_FOUND` for an unresolved procedure, `FAILURE` if a bytecode
frame cannot be allocated, and propagates native procedure signals. The opaque
pointer and argument-block bounds are not validated.

### Example

<!-- rxas-example name="decimal-dcall" test="assemble" -->
```rxas
.globals=0

main() .locals=3
    load r2,0
    dcall r0,r1,r2
    ret
```

### Related

`call`, `srcmethodsel`, `srcfprocsel`.

## `dcopy`

Copy only a decimal payload between registers.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x000c` | `dcopy rDestination,rSource` | Duplicate the source decimal storage. |

### Operands And Semantics

The destination decimal buffer is allocated or enlarged as necessary and
receives the exact encoded decimal. Other payloads, cursors, attributes, type
metadata, and flags remain intact. Self-copy is a no-op; the source is unchanged.

### Signals

Raises `INVALID_ARGUMENTS` when the source has no decimal payload. Allocation
failure is not translated into a VM signal.

### Example

<!-- rxas-example name="decimal-dcopy" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1.25d
    dcopy r0,r1
    ret
```

### Related

`copy`, `dtos`, `load`.

## `ddiv`

Divide decimal values under the current numeric context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x016f` | `ddiv rResult,rDividend,rDivisor` | Register decimals. |
| `0x0170` | `ddiv rResult,rDividend,divisor` | Literal divisor. |
| `0x0171` | `ddiv rResult,dividend,rDivisor` | Literal dividend. |

### Operands And Semantics

The destination decimal receives the context-rounded quotient. Register sources
and non-decimal destination state remain unchanged.

### Signals

Propagates decimal-plugin conditions, notably division by zero, invalid decimal
data, and context overflow/underflow.

### Example

<!-- rxas-example name="decimal-ddiv" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,9d
    ddiv r0,r1,2d
    ret
```

### Related

`didiv`, `dmod`, `dmult`.

## `dec`

Decrement a selected integer payload in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x001d` | `dec rValue` | Subtract one from `rValue.int`. |

### Operands And Semantics

Despite its chapter placement and name, this is fixed-width integer decrement,
not decimal arithmetic. Only the integer payload changes.

### Signals

Raises `OVERFLOW_UNDERFLOW` at minimum `rxinteger` without wrapping.

### Example

<!-- rxas-example name="decimal-dec" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,2
    dec r0
    ret
```

### Related

`dec0`, `dec1`, `dec2`, `inc`.

## `dec0`

Decrement `r0`'s integer payload using a compact operand-free opcode.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x001f` | `dec0` | Subtract one from `r0.int`. |

### Operands And Semantics

Only the integer payload of local register zero changes; the frame must provide
that register. This is not a decimal operation.

### Signals

Raises `OVERFLOW_UNDERFLOW` at minimum `rxinteger`.

### Example

<!-- rxas-example name="decimal-dec0" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    dec0
    ret
```

### Related

`dec`, `dec1`, `dec2`.

## `dec1`

Decrement `r1`'s integer payload using a compact operand-free opcode.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0021` | `dec1` | Subtract one from `r1.int`. |

### Operands And Semantics

Only the integer payload of local register one changes; the frame must provide
that register. This is not a decimal operation.

### Signals

Raises `OVERFLOW_UNDERFLOW` at minimum `rxinteger`.

### Example

<!-- rxas-example name="decimal-dec1" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1
    dec1
    ret
```

### Related

`dec`, `dec0`, `dec2`.

## `dec2`

Decrement `r2`'s integer payload using a compact operand-free opcode.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0023` | `dec2` | Subtract one from `r2.int`. |

### Operands And Semantics

Only the integer payload of local register two changes; the frame must provide
that register. This is not a decimal operation.

### Signals

Raises `OVERFLOW_UNDERFLOW` at minimum `rxinteger`.

### Example

<!-- rxas-example name="decimal-dec2" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r2,1
    dec2
    ret
```

### Related

`dec`, `dec0`, `dec1`.

## `decplnm`

Read the active decimal plugin's identity strings.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0190` | `decplnm rName,rDescription,rVersion` | Store three plugin strings. |

### Operands And Semantics

The three destination string payloads are replaced respectively with the
plugin name, human description, and version. Their string cursors reset; other
value state remains intact. Destinations should be distinct because writes are
performed in operand order.

### Signals

There is no translated VM signal; allocation failure is fatal.

### Example

<!-- rxas-example name="decimal-decplnm" test="run" -->
```rxas
.globals=0

main() .locals=3
    decplnm r0,r1,r2
    ret
```

### Related

`getnumdgts`, `getnumform`, `getnumstd`.

## `deq`

Compare decimal values for numeric equality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0178` | `deq rResult,rLeft,rRight` | Compare register decimals. |
| `0x0179` | `deq rResult,rLeft,decimal` | Compare with a decimal literal. |

### Operands And Semantics

The destination is set to integer Boolean `1` when decimal numeric values are
equal, otherwise `0`. Exponent/encoding differences do not defeat numeric
equality. Sources and cursors are unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-deq" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1d
    deq r0,r1,1.0d
    ret
```

### Related

`dne`, `deqbr`, `dgt`.

## `deqbr`

Branch when two register decimals are numerically equal.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x018a` | `deqbr label,rLeft,rRight` | Branch on decimal equality. |

### Operands And Semantics

True transfers to the procedure-local label; false falls through. Sources and
all cursors remain unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-deqbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,1d
    load r1,1.0d
    deqbr equal,r0,r1
    ret
equal:
    ret
```

### Related

`deq`, `dgtbr`, `dltbr`.

## `dextr`

Extract a decimal into a coefficient string and integer exponent.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x018f` | `dextr rCoefficient,rExponent,rDecimal` | Split the decimal representation. |

### Operands And Semantics

The plugin normalizes and rounds to the current digits setting, trims trailing
coefficient zeros, writes the coefficient string (or `nan`, `inf`, `-inf`) to
the first destination, and writes the base-ten exponent integer to the second.
The source decimal is unchanged. The coefficient string cursor resets.

### Signals

The instruction does not perform the normal post-call plugin-signal check;
allocation failure is not translated into a VM signal.

### Example

<!-- rxas-example name="decimal-dextr" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r2,12.5d
    dextr r0,r1,r2
    ret
```

### Related

`dtos`, `dformat`, `getnumdgts`.

## `dgt`

Compare whether one decimal value is greater than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x017c` | `dgt rResult,rLeft,rRight` | Register decimals. |
| `0x017d` | `dgt rResult,rLeft,decimal` | Register and literal. |
| `0x017e` | `dgt rResult,decimal,rRight` | Literal and register. |

### Operands And Semantics

The destination becomes integer Boolean `1` iff left is numerically greater,
otherwise `0`. Sources and cursors are unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dgt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2d
    dgt r0,3d,r1
    ret
```

### Related

`dgte`, `dlt`, `dgtbr`.

## `dgtbr`

Branch when one register decimal is greater than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0188` | `dgtbr label,rLeft,rRight` | Branch on decimal greater-than. |

### Operands And Semantics

True transfers to the procedure-local label; false falls through. Sources and
cursors are unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dgtbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,2d
    load r1,1d
    dgtbr greater,r0,r1
    ret
greater:
    ret
```

### Related

`dgt`, `dltbr`, `deqbr`.

## `dgte`

Compare whether one decimal value is greater than or equal to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x017f` | `dgte rResult,rLeft,rRight` | Register decimals. |
| `0x0180` | `dgte rResult,rLeft,decimal` | Register and literal. |
| `0x0181` | `dgte rResult,decimal,rRight` | Literal and register. |

### Operands And Semantics

The destination becomes canonical integer Boolean greater-or-equal. Numeric
decimal comparison ignores encoding/exponent differences; sources are unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dgte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2d
    dgte r0,r1,2.0d
    ret
```

### Related

`dgt`, `dlte`, `deq`.

## `didiv`

Divide decimal values and truncate the quotient to an integral decimal.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0172` | `didiv rResult,rDividend,rDivisor` | Register decimals. |
| `0x0173` | `didiv rResult,rDividend,divisor` | Literal divisor. |
| `0x0174` | `didiv rResult,dividend,rDivisor` | Literal dividend. |

### Operands And Semantics

The plugin first performs context decimal division, then truncates the result
toward zero to a decimal with no fractional part. Register sources are unchanged.

### Signals

Propagates plugin division, invalid-operand, overflow, and underflow conditions.

### Example

<!-- rxas-example name="decimal-didiv" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7d
    didiv r0,r1,2d
    ret
```

### Related

`ddiv`, `dmod`, `dtoi`.

## `dlt`

Compare whether one decimal value is less than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0182` | `dlt rResult,rLeft,rRight` | Register decimals. |
| `0x0183` | `dlt rResult,rLeft,decimal` | Register and literal. |
| `0x0184` | `dlt rResult,decimal,rRight` | Literal and register. |

### Operands And Semantics

The destination becomes integer Boolean `1` iff left is numerically less,
otherwise `0`. Sources and cursors are unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dlt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,3d
    dlt r0,2d,r1
    ret
```

### Related

`dlte`, `dgt`, `dltbr`.

## `dltbr`

Branch when one register decimal is less than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0189` | `dltbr label,rLeft,rRight` | Branch on decimal less-than. |

### Operands And Semantics

True transfers to the procedure-local label; false falls through. Both decimal
sources and their cursors remain unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dltbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,1d
    load r1,2d
    dltbr less,r0,r1
    ret
less:
    ret
```

### Related

`dlt`, `dgtbr`, `deqbr`.

## `dlte`

Compare whether one decimal value is less than or equal to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0185` | `dlte rResult,rLeft,rRight` | Register decimals. |
| `0x0186` | `dlte rResult,rLeft,decimal` | Register and literal. |
| `0x0187` | `dlte rResult,decimal,rRight` | Literal and register. |

### Operands And Semantics

The destination becomes canonical integer Boolean less-or-equal. Decimal
sources, cursors, and other destination state remain unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dlte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2d
    dlte r0,r1,2d
    ret
```

### Related

`dlt`, `dgte`, `deq`.

## `dmod`

Compute the decimal remainder paired with truncating decimal division.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0175` | `dmod rResult,rDividend,rDivisor` | Register decimals. |
| `0x0176` | `dmod rResult,rDividend,divisor` | Literal divisor. |
| `0x0177` | `dmod rResult,dividend,rDivisor` | Literal dividend. |

### Operands And Semantics

The VM computes `dividend - truncate(dividend/divisor) * divisor` using plugin
decimal operations and the current context. Register sources remain unchanged;
aliasing the destination with either source is supported.

### Signals

Propagates plugin conditions from division, truncation, multiplication, or
subtraction, including division by zero.

### Example

<!-- rxas-example name="decimal-dmod" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7d
    dmod r0,r1,3d
    ret
```

### Related

`didiv`, `ddiv`, `dmult`.

## `dmult`

Multiply decimal values under the current numeric context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x016d` | `dmult rResult,rLeft,rRight` | Multiply register decimals. |
| `0x016e` | `dmult rResult,rLeft,decimal` | Multiply by a literal. |

### Operands And Semantics

The destination decimal receives the context-rounded product. Sources and
non-decimal value state remain unchanged; destination aliasing is supported.

### Signals

Propagates decimal-plugin invalid, overflow, and underflow conditions.

### Example

<!-- rxas-example name="decimal-dmult" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2.5d
    dmult r0,r1,4d
    ret
```

### Related

`dadd`, `ddiv`, `dpow`.

## `dne`

Compare decimal values for numeric inequality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x017a` | `dne rResult,rLeft,rRight` | Compare register decimals. |
| `0x017b` | `dne rResult,rLeft,decimal` | Compare with a literal. |

### Operands And Semantics

The destination becomes integer Boolean `1` when numeric decimal values differ
and `0` when equal. Sources, decimal encodings, and cursors remain unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dne" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1d
    dne r0,r1,2d
    ret
```

### Related

`deq`, `dgt`, `dlt`.

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

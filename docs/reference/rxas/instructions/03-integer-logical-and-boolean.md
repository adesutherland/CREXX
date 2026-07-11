# Integer, Logical, And Boolean

These instructions provide checked fixed-width integer arithmetic, comparisons,
logical and bitwise operations, Boolean/scalar conversion, and inclusive range
checks. Unless stated otherwise, a destination receives only the indicated
scalar payload; unrelated payloads, attributes, type metadata, flags, and
source cursors remain unchanged.

## `and`

Compute logical conjunction of two integer truth values.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00b4` | `and rResult,rLeft,rRight` | Store `1` iff both operands are nonzero. |

### Operands And Semantics

Both source integer payloads are read. The destination integer becomes the
canonical Boolean `0` or `1`; sources and all cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-and" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,1
    load r2,2
    and r0,r1,r2
    ret
```

### Related

`or`, `not`, `iand`.

## `btof`

Convert an in-place Boolean integer to floating point.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e3` | `btof rValue` | Set the float payload to `0.0` or `1.0`. |

### Operands And Semantics

Zero in the register's integer payload produces `0.0`; any nonzero value
produces `1.0`. Only the float payload changes; the integer payload, cursor,
attributes, type metadata, and flags remain intact.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-btof" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    btof r0
    ret
```

### Related

`btoi`, `btos`, `itof`.

## `btoi`

Canonicalize an in-place Boolean integer.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e1` | `btoi rValue` | Replace any nonzero integer with `1`. |

### Operands And Semantics

An integer zero remains zero; every nonzero integer becomes one. No other
payload, cursor, attribute, type field, or flag is changed.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-btoi" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,9
    btoi r0
    ret
```

### Related

`btof`, `btos`, `itob`.

## `btos`

Convert an in-place Boolean integer to its one-character string form.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e4` | `btos rValue` | Set the string payload to `"0"` or `"1"`. |

### Operands And Semantics

Zero in the integer payload selects `"0"`; nonzero selects `"1"`. The string
payload is replaced and its cursor reset. The integer payload and non-string
state remain unchanged.

### Signals

There is no translated VM signal; string allocation failure is fatal.

### Example

<!-- rxas-example name="integer-btos" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    btos r0
    ret
```

### Related

`btof`, `btoi`, `itos`.

## `iadd`

Add two signed integers with overflow checking.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x000f` | `iadd rResult,rLeft,rRight` | Add two register integers. |
| `0x0010` | `iadd rResult,rLeft,integer` | Add a register integer and literal. |

### Operands And Semantics

The destination integer receives the mathematical sum when representable.
Sources are unchanged; only the destination integer payload is written.

### Signals

Raises `OVERFLOW_UNDERFLOW` when the signed result is outside `rxinteger`.
On that path the destination is not replaced with a wrapped result.

### Example

<!-- rxas-example name="integer-iadd" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,40
    iadd r0,r1,2
    ret
```

### Related

`isub`, `imult`, `inc`.

## `iand`

Compute bitwise AND of two fixed-width integers.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00a8` | `iand rResult,rLeft,rRight` | AND two register integers. |
| `0x00a9` | `iand rResult,rLeft,integer` | AND a register integer and literal. |

### Operands And Semantics

The operation covers every bit of the signed `rxinteger` representation and
writes only the destination integer payload. Sources and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-iand" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,13
    iand r0,r1,6
    ret
```

### Related

`ior`, `ixor`, `inot`, `and`.

## `ichkrng`

Require an integer to lie within an inclusive lower/upper bound.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01f7` | `ichkrng rValue,minimum,maximum` | Register value, literal bounds. |
| `0x01f8` | `ichkrng rValue,minimum,rMaximum` | Register value, mixed bounds. |
| `0x01f9` | `ichkrng rValue,rMinimum,rMaximum` | All values in registers. |
| `0x01fa` | `ichkrng value,minimum,rMaximum` | Literal value/minimum, register maximum. |
| `0x01fb` | `ichkrng value,rMinimum,rMaximum` | Literal value, register bounds. |

### Operands And Semantics

Every operand is read as a signed integer. Equality with either bound succeeds.
The instruction is a check only: it does not modify registers or cursors, and
does not diagnose reversed bounds separately.

### Signals

Raises `OUT_OF_RANGE` when `value < minimum` or `value > maximum`.

### Example

<!-- rxas-example name="integer-ichkrng" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,5
    ichkrng r0,1,10
    ret
```

### Related

`igt`, `ilt`, `asserttype`.

## `icopy`

Copy only an integer payload between registers.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0009` | `icopy rDestination,rSource` | Assign `rDestination.int = rSource.int`. |

### Operands And Semantics

Only the destination integer payload changes. Its string, float, decimal and
binary payloads, attributes, cursor, type metadata, and flags remain intact;
the source is unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-icopy" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,42
    icopy r0,r1
    ret
```

### Related

`copy`, `fcopy`, `scopy`, `bcopy`.

## `idiv`

Divide signed integers with truncation toward zero.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0016` | `idiv rResult,rDividend,rDivisor` | Divide two register integers. |
| `0x0017` | `idiv rResult,rDividend,divisor` | Use a literal divisor. |
| `0x0018` | `idiv rResult,dividend,rDivisor` | Use a literal dividend. |

### Operands And Semantics

The destination integer receives the C signed-integer quotient, truncated
toward zero. Sources are unchanged and no remainder is retained.

### Signals

Raises `DIVISION_BY_ZERO` for a zero divisor. Raises
`OVERFLOW_UNDERFLOW` for minimum-`rxinteger` divided by `-1`; no wrapped result
is written on either path.

### Example

<!-- rxas-example name="integer-idiv" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7
    idiv r0,r1,2
    ret
```

### Related

`imod`, `imult`.

## `ieq`

Compare signed integers for equality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0064` | `ieq rResult,rLeft,rRight` | Compare two register integers. |
| `0x0065` | `ieq rResult,rLeft,integer` | Compare with a literal. |

### Operands And Semantics

The destination integer becomes canonical Boolean `1` for equality, otherwise
`0`. Sources and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-ieq" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,4
    ieq r0,r1,4
    ret
```

### Related

`ine`, `rseq`, `req`.

## `igt`

Compare whether one signed integer is greater than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0068` | `igt rResult,rLeft,rRight` | Test register > register. |
| `0x0069` | `igt rResult,rLeft,integer` | Test register > literal. |
| `0x006a` | `igt rResult,integer,rRight` | Test literal > register. |

### Operands And Semantics

The destination integer becomes `1` when the ordered comparison is true and
`0` otherwise. Only that payload changes; sources are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-igt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,3
    igt r0,5,r1
    ret
```

### Related

`igte`, `ilt`, `igtbr`.

## `igtbr`

Branch when one register integer is greater than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0157` | `igtbr label,rLeft,rRight` | Branch if `rLeft.int > rRight.int`. |

### Operands And Semantics

The label is resolved within the current procedure. A true signed comparison
transfers control; false falls through. Neither source is mutated.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-igtbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,2
    load r1,1
    igtbr greater,r0,r1
    ret
greater:
    ret
```

### Related

`iltbr`, `igt`, `brt`.

## `igte`

Compare whether one signed integer is greater than or equal to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x006b` | `igte rResult,rLeft,rRight` | Test register >= register. |
| `0x006c` | `igte rResult,rLeft,integer` | Test register >= literal. |
| `0x006d` | `igte rResult,integer,rRight` | Test literal >= register. |

### Operands And Semantics

The destination integer becomes canonical Boolean `0` or `1`. Only that
payload changes; both sources remain unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-igte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,5
    igte r0,r1,5
    ret
```

### Related

`igt`, `ilte`, `ichkrng`.

## `ilt`

Compare whether one signed integer is less than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x006e` | `ilt rResult,rLeft,rRight` | Test register < register. |
| `0x006f` | `ilt rResult,rLeft,integer` | Test register < literal. |
| `0x0070` | `ilt rResult,integer,rRight` | Test literal < register. |

### Operands And Semantics

The destination integer becomes `1` when true and `0` otherwise. Only that
payload changes; sources remain unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-ilt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,5
    ilt r0,r1,6
    ret
```

### Related

`ilte`, `igt`, `iltbr`.

## `iltbr`

Branch when one register integer is less than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0158` | `iltbr label,rLeft,rRight` | Branch if `rLeft.int < rRight.int`. |

### Operands And Semantics

A true signed comparison transfers to the procedure-local label; false falls
through. Neither register nor cursor is changed.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-iltbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,1
    load r1,2
    iltbr less,r0,r1
    ret
less:
    ret
```

### Related

`igtbr`, `ilt`, `brt`.

## `ilte`

Compare whether one signed integer is less than or equal to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0071` | `ilte rResult,rLeft,rRight` | Test register <= register. |
| `0x0072` | `ilte rResult,rLeft,integer` | Test register <= literal. |
| `0x0073` | `ilte rResult,integer,rRight` | Test literal <= register. |

### Operands And Semantics

The destination integer becomes canonical Boolean `0` or `1`. Sources and
cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-ilte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,5
    ilte r0,5,r1
    ret
```

### Related

`ilt`, `igte`, `ichkrng`.

## `imod`

Compute the signed remainder paired with truncating integer division.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0019` | `imod rResult,rDividend,rDivisor` | Register remainder register. |
| `0x001a` | `imod rResult,rDividend,divisor` | Literal divisor. |
| `0x001b` | `imod rResult,dividend,rDivisor` | Literal dividend. |

### Operands And Semantics

The result follows C signed `%`: it is consistent with division truncated
toward zero and has the dividend's sign when nonzero. Only the destination
integer changes; sources remain unchanged.

### Signals

Raises `DIVISION_BY_ZERO` for zero divisor and `OVERFLOW_UNDERFLOW` for
minimum-`rxinteger` modulo `-1`; no result is written on failure.

### Example

<!-- rxas-example name="integer-imod" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7
    imod r0,r1,3
    ret
```

### Related

`idiv`, `imult`.

## `imult`

Multiply two signed integers with overflow checking.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0014` | `imult rResult,rLeft,rRight` | Multiply register integers. |
| `0x0015` | `imult rResult,rLeft,integer` | Multiply by a literal. |

### Operands And Semantics

The destination integer receives the product when representable. Sources and
cursors remain unchanged.

### Signals

Raises `OVERFLOW_UNDERFLOW` rather than storing a wrapped product.

### Example

<!-- rxas-example name="integer-imult" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,6
    imult r0,r1,7
    ret
```

### Related

`iadd`, `idiv`, `ipow`.

## `inc`

Increment a selected register integer in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x001c` | `inc rValue` | Add one to `rValue.int`. |

### Operands And Semantics

Only the register's integer payload changes. Its other payloads, cursor,
attributes, type metadata, and flags remain intact.

### Signals

Raises `OVERFLOW_UNDERFLOW` at maximum `rxinteger` without wrapping.

### Example

<!-- rxas-example name="integer-inc" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0
    inc r0
    ret
```

### Related

`inc0`, `inc1`, `inc2`, `iadd`.

## `inc0`

Increment local register `r0` without encoding an operand.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x001e` | `inc0` | Add one to `r0.int`. |

### Operands And Semantics

Only `r0`'s integer payload changes. The compact form requires the current
frame to provide register zero.

### Signals

Raises `OVERFLOW_UNDERFLOW` at maximum `rxinteger`.

### Example

<!-- rxas-example name="integer-inc0" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0
    inc0
    ret
```

### Related

`inc`, `inc1`, `inc2`.

## `inc1`

Increment local register `r1` without encoding an operand.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0020` | `inc1` | Add one to `r1.int`. |

### Operands And Semantics

Only `r1`'s integer payload changes. The frame must provide register one.

### Signals

Raises `OVERFLOW_UNDERFLOW` at maximum `rxinteger`.

### Example

<!-- rxas-example name="integer-inc1" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0
    inc1
    ret
```

### Related

`inc`, `inc0`, `inc2`.

## `inc2`

Increment local register `r2` without encoding an operand.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0022` | `inc2` | Add one to `r2.int`. |

### Operands And Semantics

Only `r2`'s integer payload changes. The frame must provide register two.

### Signals

Raises `OVERFLOW_UNDERFLOW` at maximum `rxinteger`.

### Example

<!-- rxas-example name="integer-inc2" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r2,0
    inc2
    ret
```

### Related

`inc`, `inc0`, `inc1`.

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

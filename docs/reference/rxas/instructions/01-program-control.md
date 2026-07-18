# Program Control And Calls

This chapter covers direct and conditional branches, counted-loop primitives,
procedure calls and returns, VM termination, no-op control points, and packed
jump-table dispatch.

Branch labels and procedure names are resolved by the assembler. Ordinary
conditional branches read integer payloads without changing them; counted forms
state their mutations explicitly. Packed jump-table instructions fall through
on a miss and reserve signals for malformed runtime table data or strict slice
bounds failures.

## `bcf`

Branch on a zero counter; otherwise advance one counted-loop step.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x015e` | `bcf label,rCounter` | Branch if zero; otherwise decrement `rCounter`. |
| `0x015f` | `bcf label,rCounter,rIndex` | On fall-through, decrement the counter and increment the index. |

### Operands And Semantics

The zero test occurs before mutation. If `rCounter` is zero, control branches
and neither register changes. Otherwise its integer payload is decremented;
the three-operand form also increments `rIndex`. This is useful as a loop-entry
or exhaustion test.

### Signals

No overflow/underflow or control-flow signal is raised; integer mutation uses
the VM integer representation.

### Example

<!-- rxas-example name="control-bcf" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0
    bcf empty,r0
    ret
empty:
    ret
```

### Related

`bct`, `bctnm`, `brf`.

## `bct`

Decrement a counter and branch while the new value remains positive.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0159` | `bct label,rCounter` | Decrement, then branch if greater than zero. |
| `0x015a` | `bct label,rCounter,rIndex` | Also increment `rIndex` before testing the counter. |

### Operands And Semantics

The counter always decrements, whether or not the branch is taken. In the
three-register form the index always increments. The branch uses the
post-decrement counter value.

### Signals

No integer overflow/underflow signal is raised.

### Example

<!-- rxas-example name="control-bct" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    bct again,r0
    ret
again:
    ret
```

### Related

`bcf`, `bctnm`, `bctp`.

## `bctnm`

Decrement a counter and branch while the new value is nonnegative.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x015b` | `bctnm label,rCounter` | Decrement, then branch if at least zero. |
| `0x015c` | `bctnm label,rCounter,rIndex` | Also increment `rIndex` before the test. |

### Operands And Semantics

Both mutations are unconditional; the branch tests the counter afterward. This
differs from `bct` by including the iteration whose resulting counter is zero.

### Signals

No integer overflow/underflow signal is raised.

### Example

<!-- rxas-example name="control-bctnm" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0
    bctnm again,r0
    ret
again:
    ret
```

### Related

`bct`, `bcf`, `bctp`.

## `bctp`

Increment an integer register and branch unconditionally.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x015d` | `bctp label,rIndex` | Increment `rIndex`, then jump to `label`. |

### Operands And Semantics

The integer payload is incremented exactly once before control transfers. The
instruction is a compact loop-back or progress-step primitive and never falls
through.

### Signals

No integer overflow signal is raised.

### Example

<!-- rxas-example name="control-bctp" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0
    bctp done,r0
done:
    ret
```

### Related

`bct`, `bctnm`, `br`.

## `beq`

Branch when two VM integer values are equal.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0028` | `beq label,rLeft,rRight` | Compare two integer register payloads. |
| `0x0029` | `beq label,rLeft,value` | Compare with an integer literal. |

### Operands And Semantics

The operands are not modified. Equality transfers to the procedure-local
label; inequality falls through to the next instruction.

### Signals

The comparison and branch do not signal.

### Example

<!-- rxas-example name="control-beq" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,2
    beq equal,r0,2
    ret
equal:
    ret
```

### Related

`bne`, `ieq`, `brt`.

## `bge`

Branch when a VM integer value is greater than or equal to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0162` | `bge label,rLeft,rRight` | Compare two integer register payloads. |
| `0x0163` | `bge label,rLeft,value` | Compare with an integer literal. |

### Operands And Semantics

The signed integer comparison does not change either operand. A true relation
transfers to the label; false falls through.

### Signals

The comparison and branch do not signal.

### Example

<!-- rxas-example name="control-bge" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,2
    bge matched,r0,2
    ret
matched:
    ret
```

### Related

`bgt`, `ble`, `igte`.

## `bgt`

Branch when a VM integer value is strictly greater than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0160` | `bgt label,rLeft,rRight` | Compare two integer register payloads. |
| `0x0161` | `bgt label,rLeft,value` | Compare with an integer literal. |

### Operands And Semantics

The signed comparison is side-effect free. A true relation branches; equality
or a smaller left value falls through.

### Signals

The comparison and branch do not signal.

### Example

<!-- rxas-example name="control-bgt" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,3
    bgt matched,r0,2
    ret
matched:
    ret
```

### Related

`bge`, `blt`, `igt`.

## `ble`

Branch when a VM integer value is less than or equal to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0166` | `ble label,rLeft,rRight` | Compare two integer register payloads. |
| `0x0167` | `ble label,rLeft,value` | Compare with an integer literal. |

### Operands And Semantics

The signed comparison leaves both operands unchanged. A true relation branches;
a greater left operand falls through.

### Signals

The comparison and branch do not signal.

### Example

<!-- rxas-example name="control-ble" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,2
    ble matched,r0,2
    ret
matched:
    ret
```

### Related

`blt`, `bge`, `ilte`.

## `blt`

Branch when a VM integer value is strictly less than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0164` | `blt label,rLeft,rRight` | Compare two integer register payloads. |
| `0x0165` | `blt label,rLeft,value` | Compare with an integer literal. |

### Operands And Semantics

The signed comparison is side-effect free. A true relation transfers control;
equality or a greater left value falls through.

### Signals

The comparison and branch do not signal.

### Example

<!-- rxas-example name="control-blt" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    blt matched,r0,2
    ret
matched:
    ret
```

### Related

`ble`, `bgt`, `ilt`.

## `bne`

Branch when two VM integer values are unequal.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x002a` | `bne label,rLeft,rRight` | Compare two integer register payloads. |
| `0x002b` | `bne label,rLeft,value` | Compare with an integer literal. |

### Operands And Semantics

The operands are unchanged. Inequality transfers to the target label;
equality falls through.

### Signals

The comparison and branch do not signal.

### Example

<!-- rxas-example name="control-bne" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    bne different,r0,2
    ret
different:
    ret
```

### Related

`beq`, `ine`, `brf`.

## `br`

Transfer control unconditionally to a procedure-local label.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0024` | `br label` | Continue execution at `label`. |

### Operands And Semantics

The assembler resolves and backpatches the label. No register, payload, flag,
or cursor is read or modified, and there is no fall-through path.

### Signals

A valid assembled branch does not signal. Undefined labels are assembler errors.

### Example

<!-- rxas-example name="control-br" test="run" -->
```rxas
.globals=0

main() .locals=0
    br done
done:
    ret
```

### Related

`brt`, `brf`, `brtf`.

## `brf`

Branch when an integer register payload is zero.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0026` | `brf label,rCondition` | Branch for integer false; otherwise fall through. |

### Operands And Semantics

Zero is false and every nonzero integer is true. The register is not normalized
or otherwise modified.

### Signals

The test and branch do not signal.

### Example

<!-- rxas-example name="control-brf" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0
    brf false,r0
    ret
false:
    ret
```

### Related

`brt`, `brtf`, `bcf`.

## `brt`

Branch when an integer register payload is nonzero.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0025` | `brt label,rCondition` | Branch for integer true; otherwise fall through. |

### Operands And Semantics

Every nonzero integer is true. The source is inspected without normalization or
mutation.

### Signals

The test and branch do not signal.

### Example

<!-- rxas-example name="control-brt" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    brt true,r0
    ret
true:
    ret
```

### Related

`brf`, `brtf`, `br`.

## `brtf`

Choose between two labels from one integer Boolean condition.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0027` | `brtf trueLabel,falseLabel,rCondition` | Branch true for nonzero, false for zero. |

### Operands And Semantics

The instruction always transfers control and never falls through. It reads
only the integer payload and does not mutate the condition register.

### Signals

The test and branch do not signal.

### Example

<!-- rxas-example name="control-brtf" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    brtf true,false,r0
true:
    ret
false:
    ret
```

### Related

`brt`, `brf`, `br`.

## `brtpandt`

Branch when any requested readable status-flag bit is set.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x020c` | `brtpandt label,rValue,mask` | Test `(readable_flags & mask) != 0`. |

### Operands And Semantics

The integer literal becomes a 32-bit mask intersected with
`RXFLAG_READABLE_MASK`; the reserved sign bit cannot match. This is an any-bit
test. The value and all flags remain unchanged.

### Signals

The flag test and branch do not signal.

### Example

<!-- rxas-example name="control-brtpandt" test="run" -->
```rxas
.globals=0

main() .locals=1
    setortp r0,512
    brtpandt matched,r0,512
    ret
matched:
    ret
```

### Related

`brtpt`, `getandtp`, `setortp`.

## `brtpt`

Branch when a value has any public writable status-flag bit set.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x020b` | `brtpt label,rValue` | Test compiler, library, and user flag bands. |

### Operands And Semantics

The test uses `RXFLAG_PUBLIC_TEST_MASK`, excluding VM-private and reserved bits.
It does not alter the value or flags.

### Signals

The flag test and branch do not signal.

### Example

<!-- rxas-example name="control-brtpt" test="run" -->
```rxas
.globals=0

main() .locals=1
    setortp r0,512
    brtpt matched,r0
    ret
matched:
    ret
```

### Related

`brtpandt`, `gettp`, `setortp`.

## `call`

Invoke a named RXAS or native procedure, optionally collecting a return value
and passing a contiguous argument block.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x002c` | `call procedure()` | Call without a return destination or arguments. |
| `0x002d` | `call rResult,procedure()` | Clear `rResult`, call, and collect a return value. |
| `0x002e` | `call rResult,procedure(),rArgCount` | Call with a contiguous argument block. |

### Operands And Semantics

For the argument form, `rArgCount` holds the integer count and the actual
arguments occupy consecutive register numbers immediately after it. RXAS
procedures receive live argument storage bindings; native procedures receive
the corresponding values through the plugin ABI. The no-argument result form
clears `rResult` before dispatch. A return resumes at the next instruction.

### Signals

Raises `FUNCTION_NOT_FOUND` for an unresolved runtime procedure and `FAILURE`
when an RXAS call frame cannot be allocated. Native or callee signals propagate
normally. The opcode does not bounds-check a hand-written argument block.

### Example

<!-- rxas-example name="control-call" test="run" -->
```rxas
.globals=0

main() .locals=1
    call r0,worker()
    ret

worker() .locals=0
    ret 7
```

### Related

`ret`, `dcall`, `linkarg`.

## `cnop`

Execute an explicit no-operation instruction, primarily for tests, patch
points, or deliberately retained instruction positions.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0057` | `cnop rA,rB,rC,rD,rE,rF,rG,rH,rI` | Consume nine register inputs and advance without changing VM state. |
| `0x021c` | `cnop` | Advance to the next instruction without changing VM state. |

### Operands And Semantics

The zero-operand form has no inputs. The nine-operand form records explicit
register reads for optimiser liveness but intentionally leaves the register
values unchanged. Cursors, flags, frames, and control metadata remain unchanged
except for normal program-counter advance.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="control-cnop" test="run" -->
```rxas
.globals=0

main() .locals=9
    load r0,0
    load r1,1
    load r2,2
    load r3,3
    load r4,4
    load r5,5
    load r6,6
    load r7,7
    load r8,8
    cnop r0,r1,r2,r3,r4,r5,r6,r7,r8
    ret
```

### Related

`br`.

## `exit`

Terminate the entire VM invocation immediately with an integer process result.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0035` | `exit` | Terminate with result code `0`. |
| `0x0036` | `exit rCode` | Use the register's integer payload. |
| `0x0037` | `exit code` | Use an integer literal. |

### Operands And Semantics

Unlike `ret`, `exit` does not return to a caller or unwind through RXAS return
sites. The selected integer becomes the interpreter's result code; later
instructions are not executed.

### Signals

Normal exit raises no VM signal.

### Example

<!-- rxas-example name="control-exit" test="run" -->
```rxas
.globals=0

main() .locals=0
    exit
```

### Related

`ret`.

## `jumpb`

Dispatch through a packed table using a whole logical binary value as the key.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0278` | `jumpb rKey,table` | Branch to the matching binary `.jcase`. |

### Operands And Semantics

`rKey` supplies its complete binary payload and `table` is a procedure-local
`.jtable` whose cases are binary literals. A match branches to the case label;
a miss falls through. The source, binary cursor, and table are unchanged.

### Signals

Malformed packed data or an invalid target raises `RXBIN_CORRUPTION`.

### Example

<!-- rxas-example name="control-jumpb" test="run" -->
```rxas
.globals=0

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

### Related

`jumps`, `jumpbs`, `jumpi`, `.jtable`, `.jcase`.

## `jumpbs`

Dispatch through a fixed-width binary table using a zero-copy source slice.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0279` | `jumpbs rBinary,rOffset,table` | Match the table-width slice at a byte offset. |

### Operands And Semantics

`rOffset` supplies a zero-based byte offset. All table keys must be nonempty
binary literals of one fixed width, which determines the slice length. A match
branches and a miss falls through. Neither register nor the binary cursor is
modified, and no temporary slice is allocated.

### Signals

Negative offsets, variable/zero-width tables, or a slice outside the logical
binary length raise `OUT_OF_RANGE`. Malformed packed data or targets raise
`RXBIN_CORRUPTION`.

### Example

<!-- rxas-example name="control-jumpbs" test="run" -->
```rxas
.globals=0

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

### Related

`jumpb`, `jumps`, `jumpi`, `bcopy`, `bslice`.

## `jumpi`

Dispatch through a packed table using a VM integer key.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x027a` | `jumpi rKey,table` | Branch to the matching integer `.jcase`. |

### Operands And Semantics

The integer payload is canonicalized as eight little-endian bytes, matching
integer-literal cases in the procedure-local table. A match branches and a miss
falls through. The key register is unchanged.

### Signals

Malformed packed data or an invalid target raises `RXBIN_CORRUPTION`.

### Example

<!-- rxas-example name="control-jumpi" test="run" -->
```rxas
.globals=0

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

### Related

`jumps`, `jumpb`, `jumpbs`.

## `jumps`

Dispatch through a packed table using a string's exact UTF-8 bytes.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0277` | `jumps rKey,table` | Branch to an exact string `.jcase`. |

### Operands And Semantics

The logical string bytes, excluding any NUL terminator, are matched against
string-literal cases. A match branches and a miss falls through. The string and
its cursor are unchanged; this form performs exact byte matching without
numeric or trailing-blank canonicalization.

### Signals

Malformed packed data or an invalid target raises `RXBIN_CORRUPTION`.

### Example

<!-- rxas-example name="control-jumps" test="run" -->
```rxas
.globals=0

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

### Related

`jumpb`, `jumpbs`, `jumpi`, `jumpr`, `jumpn`, `.jtable`, `.jcase`.

## `jumpr`

Dispatch using Rexx blank-padded nonnumeric string equality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x027b` | `jumpr rKey,table` | Match after ignoring trailing ASCII spaces. |

### Operands And Semantics

The assembler trims trailing ASCII spaces from case keys and the VM ignores
them in the runtime string. Leading spaces remain significant. A match branches
and a miss falls through without copying or changing the source or cursor. A
table cannot mix exact, padded, and numeric string modes.

### Signals

Malformed packed data or a bad target raises `RXBIN_CORRUPTION`; canonical
duplicate cases are assembler errors.

### Example

<!-- rxas-example name="control-jumpr" test="run" -->
```rxas
.globals=0

main() .locals=2
    .jtable words auto
    br after_cases
word_if: .jcase words "if   "
    ret
after_cases:
    load r0,"if"
    jumpr r0,words
    ret
```

### Related

`jumps`, `jumpn`, `.jtable`, `.jcase`, `req`.

## `jumpn`

Parse a string once and dispatch equivalent numeric spellings through a packed
table.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x027c` | `jumpn rKey,table` | Match a canonical numeric string key. |

### Operands And Semantics

Assembler and VM use the shared string-to-double parser and canonical
little-endian IEEE-754 key bytes; `-0` and `0` are one key. A numeric match
branches, while nonnumeric input falls through. The source and cursor are
unchanged. NaN input retains loose-comparison first-case alias behavior.

### Signals

Malformed packed data or targets raise `RXBIN_CORRUPTION`. Nonnumeric/NaN case
keys, canonical duplicates, and mixed table modes are assembler errors.

### Example

<!-- rxas-example name="control-jumpn" test="run" -->
```rxas
.globals=0

main() .locals=2
    .jtable numbers auto
    br after_cases
case_one: .jcase numbers "1"
    ret
after_cases:
    load r0,"01.000"
    jumpn r0,numbers
    ret
```

### Related

`jumpr`, `jumps`, `.jtable`, `.jcase`, `req`.

## `ret`

Return from the current procedure, optionally supplying a typed result.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0030` | `ret` | Return without a value. |
| `0x0031` | `ret rValue` | Return a complete register value. |
| `0x0032` | `ret integer` | Return an integer literal payload. |
| `0x0033` | `ret float` | Return a float literal payload. |
| `0x0034` | `ret "text"` | Return a string constant payload. |

### Operands And Semantics

The register form full-copies nonlocal/linked storage but may move an ordinary
callee-local value into the caller's return register before destroying the
frame. Literal forms write only their respective payload. A void return leaves
the caller's result as prepared by `call`. Returning from `main()` terminates
the interpreter; an integer result supplies its process result code.

### Signals

Return itself does not signal. It also participates in signal-handler action
returns when the current frame is an interrupt frame.

### Example

<!-- rxas-example name="control-ret" test="run" -->
```rxas
.globals=0

main() .locals=1
    call r0,worker()
    ret

worker() .locals=0
    ret "done"
```

### Related

`call`, `exit`.

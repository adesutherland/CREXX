# REXXSCRIPT Command

## Overview

`REXXSCRIPT` executes a RexxScript snippet from within a CREXX program.

It provides a simple integration layer between CREXX and the RexxScript execution engine, allowing selected variables to be shared, script output to be captured, and execution results to be returned to the caller.

The command is intended for configurable business rules, decision logic, data transformations, and other situations where small scripts need to be evaluated dynamically.

---

## Syntax

```rexx
REXXSCRIPT script-expression
           [EXPOSE name [, name] ...]
           [OUTPUT output-variable]
           [RESULT result-variable]
```

The script expression is mandatory.

All other clauses are optional.

## Execution Status

After execution, the variable `script_status` contains the completion status returned by the RexxScript engine.

If execution succeeds, the value is:

```text
OK
```

If execution fails, `script_status` contains a diagnostic message describing the reason for the failure.

Example:

```rexx
REXXSCRIPT rule EXPOSE customer, score

if script_status <> "OK" then
  say "RexxScript failed:" script_status
```

Typical failure messages might look like:

```text
undefined function LEFTX
```

or:

```text
syntax_error: expected END
```

or:

```text
division by zero
```

The exact wording of diagnostic messages is not part of the RexxScript API and may change between releases as the evaluator evolves.

Applications should therefore use:

```rexx
if script_status = "OK" then ...
```

to detect successful execution and treat any other value as an execution failure.

Diagnostic messages are intended primarily for debugging, testing, and user feedback.

---

## Clauses

### Script Expression

The first argument specifies the RexxScript source to execute.

The expression may be a string literal:

```rexx
REXXSCRIPT "say 'Hello World'"
```

or a variable:

```rexx
rule = "score = score + 1"
REXXSCRIPT rule
```

---

### EXPOSE

The `EXPOSE` clause specifies variables shared between CREXX and RexxScript.

Values are copied into the RexxScript environment before execution and copied back to CREXX when execution completes.

Example:

```rexx
a = "1"
message = ""

REXXSCRIPT "a = a + 4; message = 'done ' || a" ,
           EXPOSE a, message

say a
say message
```

Result:

```text
5
done 5
```

Only variables explicitly listed in `EXPOSE` are shared.

Variables created inside RexxScript remain local unless exposed.

---

### OUTPUT

The `OUTPUT` clause captures all RexxScript `SAY` output.

The target variable receives a `.string[]` array.

Example:

```rexx
out = .string[]

REXXSCRIPT "say 'line 1'; say 'line 2'" ,
           OUTPUT out
```

Result:

```text
out[0] = 2
out[1] = line 1
out[2] = line 2
```

---

### RESULT

The `RESULT` clause receives the raw result array returned by the RexxScript engine.

Example:

```rexx
raw = .string[]

REXXSCRIPT "x = 1" RESULT raw

say raw[1]
```

Typical result:

```text
OK
```

The result array is primarily useful for diagnostics, testing, and future extensions.

---

## Complete Example

```rexx
options levelb
import rxfnsb

main: procedure

  out = .string[]
  raw = .string[]

  customer = "Mr Smith"
  score = "8"
  decision = "PENDING"

  decision_rule = ,
    "say 'Script sees customer ' || customer;" ,
    "score = score + 5;" ,
    "if score >= 10 then" ,
    "  decision = 'APPROVE'" ,
    "else" ,
    "  decision = 'REVIEW';" ,
    "say 'Script score now ' || score"

  REXXSCRIPT decision_rule ,
             EXPOSE customer, score, decision ,
             OUTPUT out ,
             RESULT raw

  say "Captured RexxScript output:"
  do i = 1 to out[0]
    say "  " || out[i]
  end

  say
  say "Host after:"
  say "  customer =" customer
  say "  score    =" score
  say "  decision =" decision
  say "  status   =" raw[1]

  return
```

Output:

```text
Captured RexxScript output:
  Script sees customer Mr Smith
  Script score now 13

Host after:
  customer = Mr Smith
  score    = 13
  decision = APPROVE
  status   = OK
```

---

## Data Flow

```text
CREXX Variables
      │
      ▼
    EXPOSE
      │
      ▼
 RexxScript
 Execution
      │
      ├────────► OUTPUT
      │            (captured SAY lines)
      │
      ▼
 Updated Variables
      │
      ▼
    CREXX

 Execution Result
      │
      ▼
    RESULT
```

---

## Execution Model

The RexxScript engine executes the supplied script within an isolated script environment.

Only variables listed in the `EXPOSE` clause are shared with the calling CREXX program.

This explicit sharing model avoids unintended side effects and clearly defines the communication boundary between CREXX and RexxScript.

Conceptually:

```text
EXPOSE
    Shared variables

OUTPUT
    Captured SAY output

RESULT
    Execution status and diagnostic information
```

---
## Type Conversion Limits With EXPOSE

RexxScript stores its internal variables as strings. This follows the traditional Rexx model.

Even when a value is stored as a string, it may still participate in arithmetic operations if its contents are numeric.

For example:

```rexx
b = "11"

REXXSCRIPT "b = b + 4" EXPOSE b
```

After execution:

```text
b = "15"
```

Although `b` is stored internally as a string, RexxScript performs numeric conversion automatically when evaluating arithmetic expressions. The result is then stored back as a string.

Likewise:

```rexx
a = 1
b = "11"
message = ""

REXXSCRIPT "say 'script starting'; say a; a = a + 4; b = 'Alpha'; message = 'done ' || a" ,
           EXPOSE a, b, message ,
           OUTPUT out ,
           RESULT raw
```

Here `b` is a string variable. It initially contains a numeric value and could therefore be used in calculations. However, RexxScript may also freely replace it with another string value such as `"Alpha"`.

However, the following example is different:

```rexx
b = 11

REXXSCRIPT "b = 'Alpha'" ,
           EXPOSE b
```

Here `b` is a strictly typed numeric CREXX variable. RexxScript can receive its value, but assigning non-numeric text such as `"Alpha"` back to `b` is not valid.

When execution completes, values from the RexxScript environment are copied back to the exposed CREXX variables. During this copy-back step, normal CREXX type conversion rules apply. If a RexxScript value cannot be converted to the target CREXX type, a conversion error will occur.

In summary:

```text
CREXX string variable
    may receive any RexxScript string value
    may participate in arithmetic if its contents are numeric

CREXX numeric variable
    may only receive values convertible to that numeric type
```

Recommended practice:

```rexx
b = "11"     /* string-compatible exposed value */
```

instead of:

```rexx
b = 11       /* strict numeric value */
```

when RexxScript is allowed to replace the value with arbitrary text.

This limitation applies only when values are copied back through the `EXPOSE` mechanism. Internally, RexxScript continues to use string-based variables and performs numeric conversions only when required by arithmetic or comparison operations.

---

## Intrinsic Functions

RexxScript provides a small set of built-in intrinsic functions.

Function names are matched case-insensitively.

### String Functions

```text
LENGTH
SUBSTR

LEFT
RIGHT

STRIP
POS

UPPER
LOWER

WORDS
WORD
``` 
Arithmetic Functions
```text
ABS
``` 
Examples:
```rexx
LEFT(name,10)
RIGHT(code,3)
STRIP(customer)
POS("-", date)
WORD(fullname,2)
ABS(balance)
```

Unknown function names result in a runtime error.

### Intrinsic Compatibility

The current intrinsic functions are intended to provide practical utility rather than complete Rexx compatibility.

Some intrinsics currently implement only their most commonly used functionality.

For example:

* `SUBSTR()` supports the standard two- and three-argument forms but does not currently implement padding semantics.
* `LEFT()` and `RIGHT()` support width-based truncation and padding.
* Additional optional arguments supported by Classic Rexx implementations may not yet be available.

Examples:

```rexx
SUBSTR(text,5)
SUBSTR(text,5,3)

LEFT(name,10)
RIGHT(code,3)
```

The intrinsic function set will evolve incrementally as practical requirements emerge.

Applications should therefore rely only on the documented behavior of the currently implemented intrinsics.

---

## Function Support

RexxScript currently supports only the documented intrinsic functions.

General Rexx function dispatch, external function calls, dynamic function resolution, and user-defined functions are not currently supported.

This restriction keeps the execution environment small, predictable, and easy to embed within CREXX applications.

## Typical Use Cases

### Decision Rules

```rexx
rule = ,
  "if score >= 10 then" ,
  "  decision = 'APPROVE'" ,
  "else" ,
  "  decision = 'REVIEW'"

REXXSCRIPT rule EXPOSE score, decision
```

### Data Transformation

```rexx
rule = ,
  "customer = strip(upper(customer))"

REXXSCRIPT rule EXPOSE customer
```

### Dynamic Formulas

```rexx
formula = ,
  "total = price * quantity"

REXXSCRIPT formula EXPOSE price, quantity, total
```

### Validation

```rexx
rule = ,
  "if length(customer) = 0 then" ,
  "  status = 'INVALID'"

REXXSCRIPT rule EXPOSE customer, status
```

---

## Relationship to EVALUATE()

Internally, `REXXSCRIPT` acts as a convenience integration layer around the RexxScript execution engine.

The underlying evaluator remains independent of the host integration mechanism.

This separation allows the same RexxScript engine to be reused in different contexts, including:

- Runtime execution
- Rule processing
- Future preprocessor integration
- Future macro facilities

---

## Error Handling

### Missing Script

```rexx
REXXSCRIPT
```

Error:

```text
REXXSCRIPT requires a script expression
```

---

### Invalid EXPOSE Variable

```rexx
REXXSCRIPT "x = 1" EXPOSE 123
```

Error:

```text
REXXSCRIPT EXPOSE only accepts variable names
```

---

### Repeated OUTPUT Clause

```rexx
REXXSCRIPT "x = 1" OUTPUT out1 OUTPUT out2
```

Error:

```text
REXXSCRIPT OUTPUT clause repeated
```

---

### Repeated RESULT Clause

```rexx
REXXSCRIPT "x = 1" RESULT r1 RESULT r2
```

Error:

```text
REXXSCRIPT RESULT clause repeated
```

---

### Unknown Clause

```rexx
REXXSCRIPT "x = 1" INTO out
```

Error:

```text
REXXSCRIPT expected EXPOSE, OUTPUT, or RESULT clause
```

---

## Notes

- The script expression is mandatory.
- `EXPOSE`, `OUTPUT`, and `RESULT` are optional.
- Variable names in `EXPOSE` must be valid identifiers.
- `OUTPUT` receives captured `SAY` lines.
- `RESULT` receives the raw evaluator result.
- Only explicitly exposed variables are shared.
- RexxScript stores variables internally as strings.
- Numeric conversion occurs automatically when required by arithmetic or comparison operations.
- The command executes the supported RexxScript subset rather than a full Rexx `INTERPRET`.
- Only the documented intrinsic functions are currently available.
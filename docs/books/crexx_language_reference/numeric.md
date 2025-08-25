# NUMERIC Instruction

The `NUMERIC` instruction in CREXX Level B controls the precision, rounding, and comparison behaviour for all numeric operations within a procedure. To enable performance optimisations, its usage is slightly restricted compared to classic REXX.

## Syntax:

The `NUMERIC` instruction must appear as the **very first instruction** within a procedure, immediately following the procedure's label. It may specify `DIGITS`, `FUZZ`, or `FORM` with a **constant numeric or string value**, or it may explicitly request to inherit these settings from the calling procedure.

```rexx
procedure_name: procedure = <type> expose <expose_list>
  numeric digits {<constant_value> | inherited}
  numeric fuzz {<constant_value> | inherited}
  numeric form {scientific | engineering | inherited}
  numeric inherited
  arg <arg_list>
  -- ... rest of the procedure's code ...
```

*   **`<constant_value>`**: Must be a literal number (e.g., `18`, `0`) or a literal string for `FORM` (e.g., `'SCIENTIFIC'`). Expressions are not permitted.
*   **`SCIENTIFIC` | `ENGINEERING`**: Keywords defining the exponential notation format.
*   **`INHERITED`**: A special keyword indicating that the specific numeric setting should be taken from the calling procedure's context.

## Effect:

1.  **Setting Numeric Behaviour**:
    *   **`NUMERIC DIGITS <value>`**: Sets the number of significant digits used for all arithmetic calculations and arithmetic built-in functions within this procedure. (Default if not specified or inherited: **18**).
    *   **`NUMERIC FUZZ <value>`**: Sets the number of digits ignored during numeric comparisons. (Default if not specified or inherited: **0**).
    *   **`NUMERIC FORM {SCIENTIFIC | ENGINEERING}`**: Sets the preferred exponential notation for results. (Default if not specified or inherited: `SCIENTIFIC`).
2.  **Uniqueness and Immutability**: Each numeric setting (`DIGITS`, `FUZZ`, `FORM`) can be specified **only once** as a constant within a procedure. Once set, it cannot be changed dynamically within that procedure.
3.  **Inheritance**:
    *   If `NUMERIC DIGITS INHERITED` (or `FUZZ INHERITED`, `FORM INHERITED`) is used, the procedure will adopt the respective numeric setting from the procedure that called it. This value is determined at runtime.
    *   If `NUMERIC INHERITED` is specified, the procedure will inherit all numeric settings from the caller.
    *   If no `NUMERIC` instruction is present, or if `INHERITED` is not specified for a particular setting, the default values are applied: **DIGITS 18**, **FUZZ 0**, and **FORM SCIENTIFIC**.

## Motivation:

The primary motivation for these restrictions is **performance optimisation**. REXX's arbitrary-precision decimal arithmetic is computationally intensive. By requiring `NUMERIC` settings to be constant and declared at the beginning of a procedure, 
the CREXX compiler's constant folding optimiser can **Pre-calculate and embed numeric context values**. The values for precision, fuzz, and form 
can be determined at compile-time and directly incorporated into the bytecode instructions.

When `INHERITED` is used, the numeric settings are not known until runtime (as they depend on the caller). In such cases, the optimiser cannot perform these compile-time constant folding optimisations for decimal arithmetic, 
leading to potentially slower execution for affected numeric operations within that specific procedure. This design represents a deliberate trade-off, prioritising maximum interpretative performance by default, while allowing 
flexibility for explicit dynamic behaviour when required.

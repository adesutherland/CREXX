# SELECT Statement Equivalence Reference

This document serves as an internal reference to validate how `SELECT` statements (both Classic and C-Style) are rewritten into `IF / THEN / ELSE` AST structures during the compilation process. 

## 1. Classic Style SELECT

### Input Rexx Syntax
```rexx
select
  when x = 1 then say "One"
  when y = 2 then say "Two"
  otherwise say "Other"
end
```

### Equivalent AST / Rexx Rewriting
The above `select` structure is mapped to the following nested `IF / THEN / ELSE` equivalence before backend code generation:
```rexx
if x = 1 then
  say "One"
else if y = 2 then
  say "Two"
else /* otherwise */
  say "Other"
```

*Note: If no `otherwise` is provided, the final `else` block should conceptually trigger a runtime or compile-time `#SELECT_NO_OTHERWISE` error, or default to a safe `NOP` depending on final cREXX level B specifications.*

## 2. C-Style SELECT (SWITCH)

### Input Rexx Syntax
```rexx
select my_complex_expression()
  when 1 then say "One"
  when 2, 3 then say "Two or Three"
  otherwise say "Other"
end
```

### Equivalent AST / Rexx Rewriting
To guarantee `my_complex_expression()` is **only evaluated once**, the compiler introduces a temporary scoped variable (e.g., `select_tmp`). Subsequent `when` blocks implicitly compare their expressions to this temporary variable. By initializing `select_tmp = .unknown`, we force shadowing.

```rexx
select_tmp = .unknown
select_tmp = my_complex_expression()

if select_tmp = 1 then
  say "One"
else if select_tmp = 2 | select_tmp = 3 then
  say "Two or Three"
else /* otherwise */
  say "Other"
```

This ensures optimal performance by preventing repeated evaluation of the initial selector, while smoothly piggybacking off the existing, heavily tested `IF / THEN / ELSE` code generation paths.

# Operators

## Expression Operators

Level B incorporates the Classic REXX operators, which function similarly to their counterparts in 
other programming languages. However, as a language with a strong typing system, CREXX Level B 
introduces rules for type promotion that differ significantly from Classic REXX's string-based model.

> **Note: Influence of `OPTIONS` and `NUMERIC`**
>
> The precise behaviour of operators is governed by two key instructions:
>
>   * **`OPTIONS`**: A file-wide instruction that sets **parser** rules. It determines operator precedence, 
>     associativity, and which symbols (e.g., `%`, `//`) are used for certain operations.
>   * **`NUMERIC`**: A procedure-scoped instruction that controls the **semantic** behaviour of arithmetic, 
>     such as precision, rounding, and the `FUZZ` setting for comparisons.

## Arithmetic operators

| Operator   | Description                                             |
|:-----------|:--------------------------------------------------------|
| `+`        | Add                                                     |
| `-`        | Subtract                                                |
| `*`        | Multiply                                                |
| `/`        | Divide (normal or integer, see note)                    |
| `%`        | Integer Division or Remainder (see note)                |
| `//`       | Remainder (see note)                                    |
| `**`       | Raise a number to a whole-number power (exponentiation) |
| Prefix `-` | Negate the next term                                    |
| Prefix `+` | Take the next term as-is                                |

Table: Arithmetic Operators {#tbl:id}

### Influence of `OPTIONS` on Arithmetic

The `OPTIONS {CLASSICNUMERIC|COMMONNUMERIC}` setting determines which symbols are used for certain operations for the entire source file.

* Under **`OPTIONS CLASSICNUMERIC`**:
    * **`%`** performs **integer division**.
    * **`//`** performs the **remainder** operation.
* Under **`OPTIONS COMMONNUMERIC`**:
    * **`%`** performs the **remainder** operation.
    * **`/`** performs **integer division** if both operands are integers; otherwise, it performs normal division.
    * **`//`** is not strickly a valid operator, however, to support legacy code, it is treated as an alias for `%`.

## Comparison operators

### Strict Operators renamed to String Comparison Operators
CREXX Level B refers to what other REXX dialects call strict comparison (e.g., ==, >>) as string comparison.

This terminology is used intentionally to make the operator's semantics explicit within a typed language. 
The core principle of a string comparison is that it unconditionally promotes both of its operands to the 
`.string` data type before performing a direct, character-for-character comparison. This ensures that the operation
is always a test of exact textual identity, with no possibility of numeric interpretation or padding, 
which clarifies its behaviour and purpose.

### Comparison Operator Categories
CREXX provides a comprehensive set of comparison operators that fall into four categories based on two distinctions: **loose vs. string** and **case-sensitive vs. case-insensitive**.

* **Loose** operators (like `=`) perform padding to make strings of unequal length comparable and can be numeric-aware.
* **String** operators (like `==`) require strings to be identical in length and always operate on the string representation of terms.
* **Case-Sensitive** operators distinguish between uppercase and lowercase letters.
* **Case-Insensitive** operators (`~~`, `~~~`) treat uppercase and lowercase letters as equivalent.

The complete set of equality operators is summarised below:

| Operator  | Type       | Case-Sensitive?   | Padding?   | Description                                        |
|:----------|:-----------|:------------------|:-----------|:---------------------------------------------------|
| `=`       | Loose      | Yes               | **Yes**    | Standard REXX equality (numeric or padded string). |
| `==`      | **String** | Yes               | No         | **String** character-for-character equality.       |
| `~~`      | **Loose**  | **No**            | **Yes**    | **Case-insensitive, padded string equality.**      |
| `~~~`     | **String** | **No**            | No         | **String, case-insensitive equality.**             |

Table: Comparison Operator Categories {#tbl:id}

### Standard Comparison (Case-Sensitive, Loose)

These operators perform a numeric comparison if either operand is a numeric type (causing a runtime error if conversion fails).
Otherwise, they perform a character comparison where the shorter string is padded with blanks.

* The numeric comparison is affected by the **`NUMERIC FUZZ`** setting.

> **CREXX Level B Unicode String Comparison**
>
> To ensure accuracy with Unicode, CREXX Level B enhances the standard string comparison. Before the padding and 
> comparison steps, both strings are first brought into a consistent representation by applying Unicode Normalization 
> Form C (NFC). 

| Operator(s)   | Description                        |
|:--------------|:-----------------------------------|
| `=`           | Equal (numerically or when padded) |
| `¬=`, `/=`    | Not equal (inverse of `=`)         |
| `>`           | Greater than                       |
| `<`           | Less than                          |
| `< >`         | Not equal (same as `¬=`)           |
| `>=`          | Greater than or equal              |
| `¬<`          | Not less than                      |
| `<=`          | Less than or equal                 |
| `¬>`          | Not greater than                   |

Table: Standard Comparison Operators {#tbl:id}

### String Comparison (Case-Sensitive)

These operators perform what is known in other REXX dialects as a **strict** comparison. 
CREXX refers to them as **string comparisons** to emphasize their semantics: they *always* operate on the string 
representation of the terms.

Before the comparison, both operands are **promoted to the `.string` type**. This forces a character-by-character comparison 
with no padding.

* **No Padding**: If the strings have different lengths, they are not equal.
* **No Numeric Interpretation**: `1.0 == 1` may be **false** because the operands are first converted to the 
* strings `'1.0'` and `'1'`, which are not identical. Numeric types are converted to their string representation before comparison,
* using the current `NUMERIC FORM`, `NUMERIC CASE`, and `NUMERIC DIGITS` settings.

| Operator(s)   | Description                          |
|:--------------|:-------------------------------------|
| `==`          | Strictly equal (identical)           |
| `¬==`, `/==`  | Strictly not equal (inverse of `==`) |
| `>>`          | Strictly greater than                |
| `\>>`         | Strictly not greater than            |
| `<<`          | Strictly less than                   |
| `\<<`         | Strictly not less than               |
| `>>=`         | Strictly greater than or equal       |
| `<<=`         | Strictly less than or equal          |

Table: String Comparison Operators {#tbl:id}

### Approximate Comparison (Case-Insensitive, Loose) `~~`

The `~~` operator is the case-insensitive equivalent of the standard `=` operator. It performs a **loose** comparison.

1.  Both strings undergo a full Unicode case-fold.
2.  The shorter of the two resulting strings is padded on the right with blanks to match the length of the longer string.
3.  The strings are then compared.

<!-- end list -->

* `'Hello' ~~ 'hello '` -\> **true** (After case-folding, `'hello'` is padded to match.)
* The negated forms are `¬~~` and `/~~`.

### String Approximate Comparison (Case-Insensitive) `~~~`

The `~~~` operator is the case-insensitive version of the string comparison operator `==`. Like `==`, it 
first **promotes both operands to their string representation**.

1.  Both strings undergo a full Unicode case-fold.
2.  The strings are then compared directly with **no padding**. They must be identical in content and length after case-folding.

<!-- end list -->

* `'Hello' ~~~ 'hello'` -\> **true**
* `'Hello' ~~~ 'hello '` -\> **false** (Lengths are different.)
* The negated forms are `¬~~~` and `/~~~`.

## String Concatenation

| Operator        | Description                                                                 |
|:----------------|:----------------------------------------------------------------------------|
| `\|\|`          | Concatenate terms (with no blank between them)                              |
| space (`     `) | Concatenate with a single space added between terms                         |
| abuttal         | Concatenate without a space (i.e., no space between a literal and variable) |

Table: String Concatenation Operators {#tbl:id}

## Logical operators

| Operator   | Description                                                     |
|:-----------|:----------------------------------------------------------------|
| `&`        | AND (returns `1` if both terms are true)                        |
| `\|`       | Inclusive OR (returns `1` if either term is true)               |
| `&&`       | Exclusive OR (returns `1` if either term is true, but not both) |
| Prefix `¬` | Logical NOT (negates; `1` becomes `0` and vice versa)           |

Table: Logical Operators {#tbl:id}

## Term Operators

| Operator    | Description            |
|:------------|:-----------------------|
| `()`        | Parenthetical grouping |
| `[]` or `.` | Stem / Array index     |

Table: Term Operators {#tbl:id}

## Operator Precedence

The order of priority of the operators (from highest to lowest). Note that the `OPTIONS` instruction affects the precedence and associativity of certain operators.

| Priority  | Operators                      | Description                         | Notes                                                                                                                                                             |
|:----------|:-------------------------------|:------------------------------------|:------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 1         | `()` `[]` `.`                  | Term operators (grouping, indexing) |                                                                                                                                                                   |
| 2         | Prefix `+` `-` `¬`             | Unary operators                     | **`OPTIONS CLASSICNUMERIC`**: Prefix `-` has higher priority than `**`.                                                                                           |
| 3         | `**`                           | Exponentiation                      | **`CLASSICNUMERIC`**: Left-associative and has lower priority than Prefix `-`.<br>**`COMMONNUMERIC`**: Right-associative and has higher priority than Prefix `-`. |
| 4         | `*` `/` `%` `//`               | Multiply and divide                 | Which operators are valid depends on `OPTIONS`.                                                                                                                   |
| 5         | `+` `-`                        | Add and subtract                    |                                                                                                                                                                   |
| 6         | `\|\|` (space, abuttal)        | Concatenation                       |                                                                                                                                                                   |
| 7         | `=` `>` `<` `==` `~~` `~~~`... | All comparison operators            |                                                                                                                                                                   |
| 8         | `&`                            | Logical AND                         |                                                                                                                                                                   |
| 9         | `\|` `&&`                      | Logical OR and exclusive OR         |                                                                                                                                                                   |

Table: Operator Priorities {#tbl:id}

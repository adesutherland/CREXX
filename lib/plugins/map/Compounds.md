# CREXX Stem Processing – Conceptual Overview

## 0. Arrays vs. Stems in CREXX (Important)

In classic REXX, dotted variables such as `a.x` are commonly used as **arrays**. CREXX fully preserves this behavior.

For the moment, **basic one-dot notation is treated as an array**, not as a hierarchical stem.

```rexx
a.x = 42      /* array element */
```

To avoid ambiguity and improve clarity, CREXX also supports the explicit array syntax:

```rexx
a[x] = 42
```

This form is fully supported and is the **preferred way** to address arrays going forward.

### Hierarchical Stems (Precompiler Feature)

CREXX, via its **precompiler**, introduces hierarchical stems as an *add-on* to familiar REXX notation.

```rexx
Customer.IBM.name
```

Key points:

- Multi-dot notation is eligible for stem processing
- One-dot notation defaults to arrays
- Stem expansion happens in the **precompiler**, not in the runtime language
- Optional precompiler switches can relax or tighten stem detection rules

This distinction allows CREXX to remain compatible with classic REXX while enabling structured, hierarchical keys where needed.

---



## 0. Fundamental Distinction: Arrays vs. Stems in CREXX

Before discussing stems, it is important to understand a **deliberate and fundamental distinction** in CREXX.

### 0.1 One-Dot Notation Is an Array, Not a Stem

In **CREXX**, a construct of the form:

```rexx
a.x
```

is **not treated as a stem**.

It is interpreted as a **CREXX array access**, equivalent to:

```rexx
a[x]
```

This means:

- `a` is an array
- `x` is an index (literal or variable)
- The construct represents indexed storage, **not** a hierarchical key

This rule applies **always** when exactly **one dot** is present.

### 0.2 Why This Rule Exists

Classic REXX historically uses `a.x` for stem arrays.  
CREXX, however, introduces **true hierarchical stems** via the precompiler.

Allowing both meanings for `a.x` would create ambiguity:

- Is `a.x` an array lookup?
- Or a structured key with root `a` and tail `x`?

CREXX resolves this by rule:

> **One dot → array**  
> **Two or more dots → stem**

This keeps code predictable and prevents accidental reinterpretation.

### 0.3 Preferred Array Syntax in CREXX

Although `a.x` is accepted for compatibility, the **preferred and recommended array syntax** in CREXX is:

```rexx
a[x]
```

Advantages:

- Explicit intent
- No ambiguity with stems
- Compatible with one-dot stem mode
- Clear migration path for classic REXX code

### 0.4 One-Dot Stems (Optional Mode)

CREXX supports an optional precompiler mode:

```rexx
##cflags dotisstem <other optional flags> 
```

When enabled:

- `a.x` is treated as a stem
- Classic `a.x` arrays are no longer available
- Arrays must be written as `a[x]`

Because this is a **breaking semantic change**, it is **opt-in only**.

### 0.5 Summary

| Syntax | Meaning in CREXX |
|------|------------------|
| `a[x]` | Array access (preferred) |
| `a.x` | Array access (compatibility) |
| `a.b.c` | Stem |
| `a.(expr).c` | Stem with computed tail |
| `DOT_IS_STEM` | Treat `a.x` as stem |

---

# CREXX Stem Processing – Conceptual Overview

This document explains **how stems work in CREXX from a user’s point of view**. It deliberately avoids internal implementation details, algorithms, or compiler mechanics. The goal is to make stem usage predictable, readable, and safe.

---

## 1. What Is a Stem?

A **stem** is a structured variable name composed of a **root** and one or more **tails**, separated by dots.

Example:
```
Customer.IBM.name
```

- `Customer` → root
- `IBM`, `name` → tails

Conceptually, a stem behaves like a **hierarchical key** rather than a traditional array.

> Important distinction: in CREXX, **stems are not the same as classic REXX arrays**. This distinction becomes crucial when enabling single-dot stems.

---

## 2. Why Stems Exist

Stems allow you to:

- Model structured data without defining records or classes
- Build dynamic keys at runtime
- Store and retrieve related values efficiently

Typical use cases:
- Business objects (Customer, Order, Log, Summary)
- Tables keyed by multiple attributes
- Dynamic joins between datasets

Internally, CREXX stems are **not native variables**. They are translated by the **CREXX Pre-Compiler** into calls to a high-performance key/value store based on the **Robin Hood hash algorithm**.

From the programmer’s perspective, stems *behave* like language features — but they are an intentional abstraction layered on top of CREXX.

---

## 3. Static vs. Dynamic Tails

### Static tail
A literal name:
```
Customer.IBM.city
```

### Dynamic tail
If a variable exists, its **value** is used:
```
customer = 'IBM'
Customer.customer.city
```
This resolves to:
```
Customer.IBM.city
```

If the variable does *not* exist, the literal name is used.

---

## 4. Computed Tails

A tail can be computed using an expression wrapped in **`.(` and `)`**.

Example:
```
Order.(year||month).total
```

Rules:
- Computed tails must always start with `.(`
- The expression inside is evaluated first
- The result becomes the tail value

---

## 5. Nested Computed Tails (Supported, but Disciplined)

CREXX supports **limited nesting** of computed tails:
```
a.(b.(c.d)).e.f
```

This is evaluated **inside-out**:
1. Resolve `c.d`
2. Use result to resolve `b.(...)`
3. Use that result in `a.(...).e.f`

Nested stems are resolved only when they are clearly unambiguous.

> Deep or highly complex nesting is intentionally discouraged.

---

## 6. Functions Inside Computed Tails

Functions may appear inside computed tails:
```
stem.(substr(other.key,1,3)).value
```

Behavior:
- Function arguments are evaluated first
- If arguments contain stems, those stems are resolved first
- The function result becomes the tail

This allows safe composition of logic without breaking stem semantics.

---

## 7. One-Dot vs. Multi-Dot Stems

### Multi-dot stems (default)
Traditionally, a valid stem requires **at least two tails**:
```
Customer.IBM.name
```

This rule avoids ambiguity with numbers, file names, and classic REXX idioms.

### One-dot stems (optional)
CREXX can optionally treat **single-dot names as stems**:
```
Order.123
```

This behavior is enabled explicitly:
```
##rxpp option DOT_IS_STEM
```

When enabled:
- `a.b` is treated as a stem
- This allows simpler data models
- It also introduces a conflict with classic REXX array syntax

For this reason, the option is **opt-in**.

---

## 8. CREXX Arrays vs. Stems (Why the Distinction Matters)

Classic REXX uses **single-dot notation** for arrays:
```
a.1 = 'foo'
a.2 = 'bar'
```

If CREXX allowed one-dot stems by default, this would be ambiguous:
```
a.x
```

To keep semantics clean, CREXX separates the two concepts:

- **Stems** → dotted hierarchical keys
- **Arrays** → index-based containers

When `DOT_IS_STEM` is enabled, **array access must use bracket syntax**:
```
a[1] = 'foo'
a[x] = 'bar'
```

This makes intent explicit and avoids subtle bugs.

---

## 8. What Is *Not* a Stem

The following are **not** treated as stems:

- Floating point numbers:
  ```
  3.14159
  ```
- Quoted text:
  ```
  "a.b.c"
  ```

CREXX is conservative by design to avoid false positives.

---

## 9. Error Handling Philosophy

If a stem cannot be resolved safely, CREXX:

- Leaves the original source unchanged
- Emits a clear, localized error reason internally
- Never guesses or silently rewrites ambiguous constructs

This prevents “action at a distance” bugs.

---

## 10. Design Philosophy (Important)

The stem system follows these principles:

1. **First make it work** – simple stems are trivial
2. **Then make it right** – computed tails are explicit
3. **Avoid cleverness** – readability beats magic
4. **No creeping elegance** – features must earn their place

Stems are a **tool**, not a language inside the language.

---

## 11. Practical Advice

Use stems when:
- You need structured keys
- Relationships are dynamic
- Data shape matters more than type

Avoid stems when:
- Simple variables suffice
- Logic becomes unreadable
- You need deep expression trees

---

## 9. Migration Guide (Classic REXX → CREXX)

### Classic REXX (arrays via dots)
```
a.1 = 'foo'
a.2 = 'bar'
say a.1
```

### CREXX with multi-dot stems only (default)
Works unchanged — arrays still use dot syntax.

### CREXX with `DOT_IS_STEM`
Array syntax must change:

**Before**
```
a.1 = 'foo'
say a.1
```

**After**
```
a[1] = 'foo'
say a[1]
```

### Stem usage (new capability)
```
Order.1001.total = 99.95
say Order.1001.total
```

---

## 10. Summary

Stems in CREXX are:
- A **precompiler feature**, not a core language primitive
- Backed by a fast Robin Hood hash implementation
- Designed to scale from simple keys to structured data

Key takeaways:
- Multi-dot stems are safe by default
- One-dot stems are powerful but explicit
- Arrays and stems intentionally diverge in syntax

> Stems add structure without adding ceremony — as long as you keep them readable and disciplined.

---

## Appendix A – Precompiler Examples (Before / After)

This appendix shows **exact transformations** performed by the CREXX precompiler.

---

### A.1 Two Stems in One Statement

**Before:**

```rexx
if List.(213+4711).(harry||rose)\='' then
   say "OK is "List.(213+4711).(harry||Mary)
```

**After:**

```rexx
_expr_1 = harry||rose
_expr_2 = 213+4711
_tmp_rxpp_1 = getstem("List."_expr_2"."_expr_1)

_expr_1 = harry||Mary
_expr_2 = 213+4711
_tmp_rxpp_2 = getstem("List."_expr_2"."_expr_1)

if _tmp_rxpp_1\='' then
   say "OK is "_tmp_rxpp_2
```

---

### A.2 Nested Computed Tail

**Before:**

```rexx
say a.(b.(c.d)).e.f
```

**After:**

```rexx
_tmp_rxpp_3 = getstem("c."d)
_expr_1 = _tmp_rxpp_3
_tmp_rxpp_4 = getstem("b."_expr_1)

_expr_2 = _tmp_rxpp_4
_tmp_rxpp_5 = getstem("a."_expr_2"."e"."f)

say _tmp_rxpp_5
```

---

### A.3 Function Inside Computed Tail

**Before:**

```rexx
say stem.(substr(another.tail1.xyz,7,9)).tail.xend
```

**After:**

```rexx
_tmp_rxpp_6 = getstem("another."tail1"."xyz)
_expr_1 = substr(_tmp_rxpp_6,7,9)
_tmp_rxpp_7 = getstem("stem."_expr_1"."tail"."xend)

say _tmp_rxpp_7
```

**Before:**
```
customer  = 'IBM'
orderid   = '123'
customer2 = 'MS'
orderid2  = '9001'

Customer.customer.name  = 'International Business Machines'
Customer.customer.city  = 'Armonk'
Customer.customer2.name = 'Microsoft'
Customer.customer2.city = 'Redmond'
```


**After:**

```rexx
customer = 'IBM'
orderid = '123'
customer2 = 'MS'
orderid2 = '9001'
/* ++++++++++++++++ Customer.customer.name = 'International Business Machines' */
src=putstem("Customer."customer"."name, 'International Business Machines')
/* ++++++++++++++++ Customer.customer.city = 'Armonk' */
src=putstem("Customer."customer"."city, 'Armonk')
/* ++++++++++++++++ Customer.customer2.name = 'Microsoft' */
src=putstem("Customer."customer2"."name, 'Microsoft')
/* ++++++++++++++++ Customer.customer2.city = 'Redmond' */
src=putstem("Customer."customer2"."city, 'Redmond')

---

## Example: Creating Orders with Dynamic Segments

This example shows how dynamic stem tails are expanded by the CREXX precompiler.

### Before (Source Code)

```rexx
say "----- 1. Create 2 orders using dynamic segments"

Order.customer.orderid.date   = '2025-12-20'
Order.customer.orderid.amount = '149.95'
Order.customer.orderid.status = 'OPEN'

Order.customer2.orderid2.date   = '2025-12-21'
Order.customer2.orderid2.amount = '399.00'
Order.customer2.orderid2.status = 'CLOSED'
```
### After

```rexx
say "----- 1. Create 2 orders using dynamic segments"

/* ++++++++++++++++ Order.customer.orderid.date   = '2025-12-20' */
src=putstem("Order."customer"."orderid"."date, '2025-12-20')
/* ++++++++++++++++ Order.customer.orderid.amount = '149.95' */
src=putstem("Order."customer"."orderid"."amount, '149.95')
/* ++++++++++++++++ Order.customer.orderid.status = 'OPEN' */
src=putstem("Order."customer"."orderid"."status, 'OPEN')
/* ++++++++++++++++ Order.customer2.orderid2.date   = '2025-12-21' */
src=putstem("Order."customer2"."orderid2"."date, '2025-12-21')
/* ++++++++++++++++ Order.customer2.orderid2.amount = '399.00' */
src=putstem("Order."customer2"."orderid2"."amount, '399.00')
/* ++++++++++++++++ Order.customer2.orderid2.status = 'CLOSED' */
src=putstem("Order."customer2"."orderid2"."status, 'CLOSED')
```

## Example: Reading Orders and Joining to Customer Data

This example demonstrates how multiple stems on a single statement line are expanded, and how dynamic joins are resolved by the CREXX precompiler.

---

### Before (Source Code)

```rexx
say "----- 2. Read back the two orders, show join to Customer.<cust>.name/city"

customer = 'IBM'
orderid  = '123'

say '2.1' 'Order ' orderid' for 'Customer.customer.name' ( 'Customer.customer.city' ) \
    date='Order.customer.orderid.date' amount='Order.customer.orderid.amount' \
    status=' Order.customer.orderid.status
```

### After
```rexx
say "----- 2. Read back the two orders, show join to Customer.<cust>.name/city"

customer = 'IBM'
orderid  = '123'

/* ++++++++++++++++ say '2.1' 'Order ' orderid' for 'Customer.customer.name' ( 'Customer.customer.city' )
date='Order.customer.orderid.date' amount='Order.customer.orderid.amount'
status=' Order.customer.orderid.status */
_tmp_rxpp_1 =getstem("Customer."customer"."name)
_tmp_rxpp_2 =getstem("Customer."customer"."city)
_tmp_rxpp_3 =getstem("Order."customer"."orderid"."date)
_tmp_rxpp_4 =getstem("Order."customer"."orderid"."amount)
_tmp_rxpp_5 =getstem("Order."customer"."orderid"."status)

say '2.1' 'Order ' orderid' for '_tmp_rxpp_1' ( '_tmp_rxpp_2' ) \
date='_tmp_rxpp_3' amount='_tmp_rxpp_4' status=' _tmp_rxpp_5
```


## Final Note

After preprocessing:

- No stem syntax remains
- All evaluation is explicit
- Runtime behavior is predictable and fast

**If the generated code is readable, the stem is valid.**

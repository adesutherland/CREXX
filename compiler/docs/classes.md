# cREXX Level B: Class & Interop Reference

**Version:** 1.2
**Status:** Phase 1 Implemented
**Authority:** CREXX Architect

---

## 1. Class Definition & Lifecycle

### 1.1 Class Declaration
A class is defined by a label followed by the `class` keyword. It may optionally declare a generic type using `of`.

**Syntax:**
```rexx
/* Standard Class */
token: class

/* Generic Class (e.g., a List of Integers) */
intlist: class of .int

```

### 1.2 The Factory (Constructor)

Level B uses a single **Default Factory** defined by the wildcard `*`.

* **Definition:** `*: factory` block inside the class.
* **Invocation:** Calling the class name as a function: `.classname(args)`.
* **Return:** Must explicitly `return this`.

**Example:**

```rexx
*: factory
    arg initial_value = .int
    /* ... setup ... */
    return this

```

---

## 2. Attribute Mapping (The `WITH` Clause)

Attributes define the internal state of the class. They can be **Implicit** (Compiler-managed, auto-allocated) or **Explicit** (mapped to specific VM/Object storage). If a `WITH register` mapping is omitted, the compiler will automatically allocate an unused register (e.g. `r1`, `r2`) for the attribute.

**Syntax:**
`variable_name = type [WITH source_location]`

### 2.1 Implicit Assignment (Standard Objects)

If `WITH` is omitted, the Compiler automatically allocates an unused register for the attribute. Automatic register allocation uses a two-pass mechanism to avoid conflicts with any explicitly mapped registers in the same class.

```rexx
/* Compiler allocates an available register (e.g., reg 1) */
counter = .int
/* Compiler allocates next available register (e.g., reg 2) */
name    = .string

```

### 2.2 Explicit Physical Mapping (VM/Plugin Interop)

Used to map REXX variables to fixed C-Struct offsets or specific VM registers. This is essential for Compiler Plugins and can be freely mixed with Implicit Assignment.

**Syntax:** `WITH register[.index][.field]`

* **`register`**: The object root (self).
* **`.index`**: The numeric slot index (1..N).
* **`.field`**: The specific independent storage slot (`int`, `float`, `string`, `binary`, `status`, `count`).

**Examples:**

```rexx
/* Map 'id' to the Integer slot of Register 1 */
id      = .int    with register.1.int

/* Map 'raw' to the underlying Object of Register 2 */
payload = .object with register.2

/* Map 'is_ready' to Bit 0 of the Object's Status field */
ready   = .boolean with register.status.0

/* Map 'size' to the attribute count of the root object */
size    = .int     with register.count

```

### 2.3 Dynamic String Mapping (Dynamic Objects)

Used for Objects/Stems where storage is accessed via a String Key rather than a numeric index.

**Syntax:** `WITH register["key"]`

```rexx
/* Map 'userName' to the slot named "user" in the dynamic object */
user    = .string with register["user"]

/* Map 'config' to the slot named "cfg" */
cfg     = .int    with register["config"]

```

---

## 3. Methods & Syntactic Sugar (Magic Methods)

Methods are the only public interface. Level B establishes specific method names that the Compiler maps to operators ("Syntactic Sugar").

### 3.1 Standard Methods

Defined using the `method` keyword.

```rexx
/* Definition */
print: method = .string
    return "Value: " || counter

/* Usage */
obj.print()

```

### 3.2 Operator Overloading & Resolution Strategy

The Compiler applies specific resolution rules to map syntax to methods.

**1. Bracket Access (`[]`)**
* `obj[expr]` always resolves to `obj.get(expr)`.
* `obj[expr] = val` always resolves to `obj.set(expr, val)`.

**2. Dot Access (`.`)**
When the compiler encounters `obj.member`, it resolves in this order:
1.  **Method Match:** If `member` is a defined method with no arguments, generate `call member`. Note: RHS only.
2.  **Private Attribute:** (Inside Class Only) If `member` is a `reserve` field, generate attribute access.
3.  **Fallback (Sugar):** Syntactic sugar attempts to rewrite property access. 
    * `obj.member` rewrites to `obj.getMember()`. 
    * `obj.member = val` rewrites to `obj.setMember(val)`. 
    The compiler automatically upper-cases the first letter of the property name.
4.  **Error:** If none of the above matches, it raises an `#INVALID_PUBLIC_ATTRIBUTE` error for property access or `#METHOD_NOT_FOUND` for method calls.

**3. Reserved Method Names**
To enable this sugar, the following method signatures are reserved / expected:

| Method | Signature | Triggered By |
| :--- | :--- | :--- |
| `get` | `arg key` | `obj[key]` |
| `set` | `arg key, value` | `obj[key]=v` |
| `getProp` | `(none)` | `obj.prop` |
| `setProp` | `arg value` | `obj.prop=v` |
| `tostring`| `(none)` | `(string)obj`, String concatenation (implicit conversion to `TP_STRING`, throws `#BAD_CONVERSION` if missing) |
| `(int)obj` | `toint: method` | Integer Math Context |
| `(boolean)obj` | `toboolean: method` | Logic Context (IF/WHEN) |

**Example: Implementing a Stem**

```rexx
stem: class
    /* ... implicit or explicit storage ... */

    /* Handles: x = myStem["foo"] AND x = myStem.foo */
    get: method = .string
        arg key = .string
        /* ... impl ... */

    /* Handles: myStem["foo"] = bar */
    set: method 
        arg key = .string, val = .string
        /* ... impl ... */

```

---

## 4. Implementation Phasing

### Phase 1: Physical Interop (COMPLETED)

**Goal:** Enable Compiler Plugins (Tokens, AST Nodes).
**Status:** Integrated via the Exit Bridge and `exit_dispatch_walker`.
**Scope:**

1. `CLASS`, `FACTORY`, `METHOD` keywords.
2. `WITH register[.index][.field]` (Explicit Numeric/Field mapping).
3. Basic Method Dispatch (`obj.method()`).
4. Implicit Register Allocation (Compiler creates the map for unassigned attributes).
5. **Usage**: Successfully used to map `token` objects for Rexx plugins and allows automatic VM registry mappings for objects.

### Phase 2: Dynamic & Sugar (Follow-up)

**Goal:** Enable Stems and Logic Classes.
**Scope:**

0. Review syntax and sub phased based on Phase 1 experience.
1. `WITH register["string"]` (Dynamic mapping).
2. Syntactic Sugar (`get`/`set`/`tostring`).
3. `OF` (Generics/Type hints)

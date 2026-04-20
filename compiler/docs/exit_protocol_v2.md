## cREXX Exit Protocol V2

Status: active contract

This document is the normative contract for compiler exits.

Use this document for:

- the required method shape
- the allowed data in each phase
- the meaning of statuses and protocol objects
- the distinction between default imports and per-occurrence imports

Use [exit_bridge_guide.md](exit_bridge_guide.md) for compiler-specific bridge
behavior such as discovery, marshalling, fixpoint integration, and debugging.

### 1. Terminology

- `compiler exit`: the preferred term for a Rexx class that participates in
  the compiler exit bridge
- `bridge plugin`: historical term for the same mechanism
- `certified exit`: a compiler exit that owns a reserved Rexx keyword or the
  implicit-command fallback path, subject to compiler allowlisting
- `exit bundle`: an `.rxbin` module containing one or more exit classes in the
  `rxcpexits` namespace
- `occurrence`: one AST node being handled by one exit instance

### 2. Core Rule

All symbol-affecting data must be declared no later than `pre_process()`.

That includes:

- bindings
- keyword claims
- imports
- helper procedures

`process()` is intentionally non-structural. It may lower, reject, defer, or
diagnose, but it must not change compiler shape.

### 3. Mandatory Methods

Every exit class must implement:

```rexx
describe: method = .exitdescriptor
pre_process: method = .exitplan
process: method = .exitresult
```

There is no compatibility path for:

- string-returning `pre_process()`
- mutable getter-based result state
- compiler hardcoding of exit-owned imports

### 4. Lifecycle

1. The compiler loads the exit bundle and instantiates each exit class.
2. `describe()` is called once per class during discovery.
3. `pre_process()` is called for each owned occurrence during the planning
   phase, before symbol harvesting.
4. The compiler applies descriptor imports and plan data.
5. Symbol harvesting, typing, and fixed-point iteration proceed.
6. `process()` is called later during lowering/dispatch for the same
   occurrence.

### 5. Phase Contracts

#### 5.1 `describe()`

`describe()` is class-level and static.

It declares:

- protocol version
- primary keyword
- additional keywords
- default imports
- exit flags

It must not depend on:

- per-occurrence token content
- current symbol types
- current scope state

Use `describe()` for imports that are always required whenever the exit is
present in a file.

Example:

```rexx
describe: method = .exitdescriptor
    desc = .exitdescriptor("parse")
    call desc.add_flag("certified")
    call desc.add_flag("reserved_keyword")
    call desc.add_import("rxfnsb", "descriptor", "")
    return desc
```

In this example, `rxfnsb` is a default import. It is unconditional for the
exit class.

#### 5.2 `pre_process()`

`pre_process()` is the only structural phase.

It may declare:

- bindings to hoist
- contextual keyword claims
- per-occurrence imports
- helper procedures
- diagnostics
- notes

It must return data as early as possible. If the exit can know something from
lexical structure alone, it must emit that information here instead of waiting
for `process()`.

`pre_process()` must not emit replacement code.

Example:

```rexx
pre_process: method = .exitplan
    arg tokens = .token[]
    plan = .exitplan("READY")

    if tokens.0 >= 3 & upper(tokens[2].get_text()) = "FAST" then do
        call plan.add_import("rxjson", "plan", "conditional")
        call plan.add_keyword(2, tokens[2].get_text(), "mode", "myexit")
    end

    if tokens.0 >= 4 & tokens[4].get_type() = "identifier" then do
        call plan.add_binding("var", tokens[4].get_text(), "", ".string", 0, "myexit_target", "")
    end

    return plan
```

In this example, the import is the same `rxcp.importplan` shape as a default
import, but its meaning is different:

- descriptor import: unconditional default import for the exit class
- plan import: conditional import for one occurrence

The compiler merges both sources and deduplicates them per file.

#### 5.3 `process()`

`process()` is non-structural.

It may return:

- `REJECT`
- `PENDING`
- `ACCEPT`
- `REPLACE`
- `ERROR`

It may also carry:

- replacement lines
- diagnostics
- notes

It must not introduce:

- bindings
- imports
- helper procedures

Example:

```rexx
process: method = .exitresult
    arg tokens = .token[]

    result = .exitresult("EMPTY")

    if tokens.0 < 2 then do
        call result.set_status("REJECT")
        return result
    end

    if tokens[2].get_value_type() = ".unknown" then do
        call result.set_status("PENDING")
        return result
    end

    call result.set_status("REPLACE")
    call result.add_replacement_line("say 'value=' || {2}")
    return result
```

#### 5.4 Status Semantics

`exitplan.status`:

- `READY`: planning data for this iteration is complete
- `PENDING`: planning cannot yet finish; request another compiler iteration
- `ERROR`: planning failed; emit diagnostics and stop treating the occurrence
  as healthy

`exitresult.status`:

- `REJECT`: the exit declines ownership for this occurrence
- `ACCEPT`: the exit handled the occurrence without replacement
- `PENDING`: lowering cannot yet finish; request another compiler iteration
- `REPLACE`: replace the occurrence with the supplied replacement lines
- `ERROR`: lowering failed; emit diagnostics

Guidance:

- use `PENDING` only when later typing or symbol convergence can legitimately
  unblock the occurrence
- do not use `process()` to "finish planning later"
- use `ERROR` when the occurrence is invalid regardless of future iterations

#### 5.5 Token Semantics

The compiler marshals exit input as semantic token categories rather than raw
parser internals.

Certified-exit payload nodes are normalized into categories such as:

- `identifier`
- `string_literal`
- `int_literal`
- `float_literal`
- `decimal_literal`
- `operator`
- `comma`
- `bracket`
- `other`

Exits must:

- recognize contextual keywords by token text
- not assume that internal parser marker types like `EXIT_TOKEN` are preserved
  in `.token.get_type()`

This is important for exits such as `PARSE`, where words like `WITH`, `TRIM`,
and `INTO` are recognized by text, while targets and literals must still keep
their semantic token kinds.

### 6. Import Model

Both default imports and plan imports use `rxcp.importplan`.

The difference is declaration timing and ownership:

| Source | Object field | Meaning |
| :--- | :--- | :--- |
| `describe()` | `exitdescriptor.default_imports` | unconditional imports for the exit class |
| `pre_process()` | `exitplan.imports` | conditional imports for one occurrence |

Compiler behavior:

- both import sources are inserted as normal file-level `IMPORT` nodes
- imports are deduplicated per file
- imports must be declared by `describe()` or `pre_process()`, never by
  `process()`

### 7. Protocol Objects

The shared protocol classes live in `compiler/exits/token.rexx`.

#### 7.1 `rxcp.token`

Fields:

- `type`
- `subtype`
- `text`
- `line`
- `column`
- `length`
- `file`
- `node_type`
- `value_type`
- `type_string`
- `value_dims`
- `join_before`

`join_before` is normalized concat metadata emitted by the compiler when an
expression is flattened for the exit bridge:

- `""`: no inherited concat boundary
- `"concat"`: concatenate without inserting a blank
- `"sconcat"`: concatenate with one blank

This lets exits reconstruct command or expression text without preserving the
full concat subtree shape.

#### 7.2 `rxcp.exitdescriptor`

Fields:

- `protocol_version`
- `primary_keyword`
- `additional_keywords`
- `default_imports`
- `flags`

Use it to describe static class-level ownership.

#### 7.3 `rxcp.exitplan`

Fields:

- `status`
- `bindings`
- `keywords`
- `imports`
- `helpers`
- `diagnostics`
- `notes`

Use it to declare all structural data for the current occurrence.

#### 7.4 `rxcp.exitresult`

Fields:

- `status`
- `replacement_lines`
- `diagnostics`
- `notes`

Use it to lower or reject an occurrence after planning is already complete.

#### 7.5 `rxcp.bindingplan`

Fields:

- `kind`
- `internal_name`
- `external_alias`
- `value_type`
- `dimensions`
- `provenance`
- `flags`

Typical uses:

- hoisted local variable
- typed stem binding
- exit-owned synthetic temporary

#### 7.6 `rxcp.keywordclaim`

Fields:

- `token_index`
- `keyword_text`
- `keyword_role`
- `provenance`

Use it when the exit needs the compiler to treat a particular token as an
exit-owned contextual keyword.

#### 7.7 `rxcp.importplan`

Fields:

- `namespace_name`
- `provenance`
- `flags`

This object is shared by both default imports and plan imports.

#### 7.8 `rxcp.helperplan`

Fields:

- `helper_id`
- `scope`
- `symbol_name`
- `lines`
- `flags`

Wave-1 supported scope:

- `file_tail`

Helper procedures must be declared in `pre_process()`, not in `process()`.

#### 7.9 `rxcp.exitdiagnostic`

Fields:

- `severity`
- `token_index`
- `code`
- `message`

Severities currently expected by the compiler:

- `error`
- `warning`
- `note`

### 8. Examples

#### 8.1 Minimal Replacing Exit

```rexx
namespace rxcpexits expose helloexit
import rxcp

helloexit: class
    *: factory
        return

    describe: method = .exitdescriptor
        return .exitdescriptor("hello")

    pre_process: method = .exitplan
        return .exitplan("READY")

    process: method = .exitresult
        result = .exitresult("REPLACE")
        call result.add_replacement_line("say 'hello from exit'")
        return result
```

#### 8.2 Planning Exit With Bindings And Keyword Claims

Pattern:

- claim contextual keywords in `pre_process()`
- hoist bindings in `pre_process()`
- lower in `process()`

This is the shape used by exits such as `PARSE` and `EXECIO`.

#### 8.3 Helper Exit

Pattern:

- declare a helper procedure in `pre_process()`
- declare any needed bindings in `pre_process()`
- call the helper from multiline replacement text in `process()`

This is the shape exercised by the bundled `dummy` exit.

### 9. Certified Exit Ownership

The compiler retains ownership policy for certified exits:

- reserved keywords
- implicit-command fallback ownership

The exit descriptor supplies declared flags, but the compiler validates those
flags against the certified allowlist.

### 10. Testing Requirements

Every bundled exit should be covered by:

- direct protocol tests for `describe()` / `pre_process()` / `process()`
- runtime lowering tests
- any special bridge capability it relies on, such as helpers or structured
  diagnostics

Bridge features not exercised by production exits must still be covered by
dedicated harness exits such as `dummy`.

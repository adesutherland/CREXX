# cREXX Exit Bridge Guide

This document describes the compiler-specific side of the exit system.

Use this document for:

- how exits are discovered and loaded
- when `describe()`, `pre_process()`, and `process()` run in the compiler
- how tokens are marshalled for exits
- how imports, helpers, diagnostics, and replacement fragments are applied
- how to test and debug the bridge

Use [exit_protocol_v2.md](exit_protocol_v2.md) for the normative protocol
contract.

## 1. Terminology

- `compiler exit`: the preferred name for the mechanism
- `bridge plugin`: historical name for the same mechanism
- `certified exit`: compiler-owned reserved-keyword exit, such as `ADDRESS`
  or `PARSE`
- `exit bridge`: the compiler code that discovers exits, marshals token
  streams, applies plan/result data, and grafts rewritten fragments back into
  the AST

## 2. Discovery And Loading

The compiler always attempts to load:

1. the certified `rxcexits.rxbin` bundle
2. an optional extra bundle named by `RXCP_EXIT_MODULE`

Classes are discovered in the `rxcpexits` namespace. For each class:

1. the compiler instantiates the class
2. it calls `describe()`
3. it validates the protocol version
4. it registers the primary keyword, additional keywords, flags, and default
   imports

Certified exits are still compiler-owned at the policy level. The descriptor
declares flags, but the compiler verifies them against the certified allowlist.

Relevant implementation:

- `compiler/rxcp_exit.c`
- `compiler/rxcp_exit.h`

## 3. Validation Pipeline Integration

The bridge participates in two separate validation moments.

### 3.1 Planning Phase

`pre_process()` runs before symbol harvesting.

The compiler applies:

- descriptor default imports
- plan imports
- plan bindings
- plan keyword claims
- plan helper procedures
- plan diagnostics and notes

This is why structural information must be declared in `pre_process()`.

### 3.2 Lowering Phase

`process()` runs later in the normal exit-dispatch pass.

The compiler then:

- reads result status
- reads diagnostics and notes
- joins `replacement_lines`
- interpolates and parses replacement fragments when status is `REPLACE`

### 3.3 Fixed-Point Behavior

Both plan-side and result-side `PENDING` statuses request another compiler
iteration.

That is valid only when future symbol/type convergence can unblock the exit.
`PENDING` is not a substitute for late planning.

Relevant implementation:

- `compiler/rxcp_val_plugin.c`
- `compiler/rxcp_val_orch.c`

## 4. Token Marshalling

The bridge marshals exit input as `.token[]`.

Each marshalled token carries:

- raw lexer token type
- source text
- source location
- AST node type
- current value type
- current value dimensions

For certified exits, payload tokens are normalized into semantic categories
before they reach the Rexx exit:

- `identifier`
- `string_literal`
- `int_literal`
- `float_literal`
- `decimal_literal`
- `operator`
- `comma`
- `bracket`
- `other`

This avoids leaking compiler-internal node markers such as `EXIT_TOKEN` into
exit code.

Practical consequence:

- exits should match contextual keywords by token text
- exits should use token kind only for semantic categories such as identifier,
  literal, operator, bracket, and comma

Relevant implementation:

- `compiler/rxcp_exit.c`
- `compiler/exits/token.rexx`

## 5. Imports

The bridge treats both import sources as normal file-level imports:

- descriptor default imports from `describe()`
- per-occurrence imports from `pre_process()`

They are merged and deduplicated per file before later validation stages.

That means they are structurally the same operation in the compiler, but they
come from different declaration sites with different intent:

- descriptor import: unconditional exit default
- plan import: conditional occurrence-specific import

## 6. Helpers

Helper procedures are declared in `pre_process()` through `helperplan`.

Current bridge behavior:

- supported scope is `file_tail`
- helper source is parsed as a fragment
- helper procedures are appended to the containing file AST
- helpers are deduplicated by file plus `helper_id`
- conflicting helper bodies with the same `helper_id` are a compiler error

This is compiler-owned behavior. Exits declare helper intent, but they do not
manually splice helper AST nodes into the main file.

## 7. Replacement Fragments

When a result returns `REPLACE`, the bridge:

1. joins `replacement_lines` with newline separators
2. interpolates token placeholders
3. reparses the fragment
4. runs the early structural/source fixup passes on the fragment
5. grafts the fragment back into the main AST

The bridge intentionally preserves source-valid token text where possible so
quoted literals stay quoted in generated Rexx code.

Important scope note:

- replacement fragments are structurally normalized before grafting, but their
  final lexical scopes are completed later by `structure_symbols_walker` and
  `build_symbols_walker`
- structured replacements are therefore supported, including nested `DO` /
  `IF` / nested `INSTRUCTIONS`
- debug validation for post-dispatch fragments runs after that scope rebuild,
  so `-d2` / `-d3` checks the stabilized tree rather than the transient
  pre-scope form

## 8. Diagnostics And Notes

Both `exitplan` and `exitresult` may emit:

- errors
- warnings
- notes

Compiler behavior:

- errors enter the normal compiler diagnostic stream and fail the occurrence
- warnings enter the normal warning stream
- notes are currently logged for debug-oriented tracing when debug mode is high

## 9. Testing Strategy

There are three useful test layers.

### 9.1 Direct Protocol Tests

Run the Rexx exit object directly and assert:

- `describe()` output
- `pre_process()` bindings/keywords/imports/helpers
- `process()` result status and replacement text

This is the fastest way to verify protocol behavior.

### 9.2 Compiler Runtime Tests

Compile and run real Rexx source through `rxc`, `rxas`, and `rxvm`.

Use this layer for:

- actual lowering behavior
- interaction with typing and symbol convergence
- disabled certified-exit diagnostics
- inline statement positions such as `then`, `else`, `when`, `otherwise`

### 9.3 Bridge Feature Harness Tests

Keep dedicated coverage for bridge capabilities that are not necessarily used
by every production exit.

Current examples include:

- helper insertion
- structured diagnostics
- direct bridge cache/highlighting behavior

## 10. Debugging Guidance

When debugging the bridge:

1. confirm the bundle is being loaded from the expected search path
2. confirm the exit class is discovered in `rxcpexits`
3. check `describe()` output before looking at lowering
4. verify token kinds and token text together
5. check whether the failure is in planning or in lowering
6. distinguish stale binaries from real bridge failures by rebuilding the
   consuming executable, not just `rxc`
7. if a debug-only failure mentions missing local scope on an exit-generated
   nested block, inspect the symbol-structure rebuild path before assuming the
   fragment shape itself is invalid

Useful places to inspect:

- `compiler/rxcp_exit.c`
- `compiler/rxcp_val_plugin.c`
- `compiler/rxcp_val_orch.c`
- `compiler/tests/src/test_highlight_cache.c`

## 11. Relationship To The Protocol Spec

The split between the two docs is intentional:

- [exit_protocol_v2.md](exit_protocol_v2.md) says what an exit must implement
- this guide says how the cREXX compiler bridge consumes that protocol

If a rule is about allowed protocol shape, it belongs in the protocol spec.
If it is about discovery, AST application, iteration, or debugging, it belongs
here.

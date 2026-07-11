# RXC Dispatch Optimization

This document is the durable compiler contract for lowering suitable Rexx
`SELECT` and equality ladders to RXAS packed jump tables. The RXAS syntax and
packed RXBIN format are documented separately in
`docs/reference/rxas/program-syntax.md` and `RXBIN_JUMP_TABLES.md`.

## Source Semantics

Dispatch lowering is an optimization only. It preserves these source rules:

- C-style `select expression` evaluates its selector once and compares each
  `when` value using `=` semantics.
- Classic `select` evaluates independent Boolean `when` expressions.
- Selection is first-match-wins.
- A table miss continues at the original next comparison or fallback.
- An expression between two constant runs is never moved or skipped.

## Compiler Pipeline

The early control-flow rewrite gives C-style SELECT a private typed selector
temporary and converts it to a canonical equality ladder. It marks the generated
block with SELECT provenance. General optimized IF/classic-SELECT recognition
uses the same candidate collector after normal validation and optimization.

Eligible runs become `OPT_DISPATCH` AST nodes containing typed constant cases,
their original bodies, and an optional fallback. The optimizer describes the
dispatch; `compiler/rxcp_emit_flow.c` owns `.jtable`, `jump*`, `.jcase`, miss,
and join-label emission. Runs are rewritten from the tail so each table keeps
the untouched residual ladder as its miss path.

Explicit eligible C-style SELECT is considered in optimized and no-opt builds.
General IF and classic SELECT recognition is optimized-build-only.

## Eligibility

A run must contain consecutive constant cases of one canonical dispatch kind
over the same resolved scalar selector. The compiler currently emits:

| Kind | RXAS instruction | Comparison contract |
| --- | --- | --- |
| Integer | `jumpi` | signed 64-bit integer |
| Exact string | `jumps` | exact normalized UTF-8 bytes |
| Padded string | `jumpr` | loose nonnumeric comparison with trailing ASCII spaces removed |
| Numeric string | `jumpn` | shared Rexx string-to-double canonicalization |
| Exact binary | `jumpb` | whole logical binary bytes |

The lowering fails closed for dynamic or duplicate keys, mixed kinds inside a
run, references, exposed/global/class/indexed/property selectors, calls,
getters, mutation, aliases, or any shape whose repeated read and evaluation
order cannot be proved. Safe runs on either side of an unsupported condition
may still be lowered independently.

Numeric string keys use the same parser as ordinary loose comparison. Signed
zero is folded, NaN case literals are rejected, canonical duplicates are
rejected, and the assembler adds the internal NaN alias required by existing
first-match behavior. Padded string keys must all be provably nonnumeric.

## Profitability Policy

The Release 1 minimum consecutive case counts are:

| Kind | Minimum cases |
| --- | ---: |
| Integer | 8 |
| Exact string | 3 |
| Padded string | 2 |
| Numeric string | 2 |
| Exact binary | 3 |

These thresholds choose between a source ladder and a table. The assembler's
independent `auto` policy chooses the packed `linear`, `openhash`, or `acph`
representation. Measurements and rationale are recorded in
`docs/planning/beta-3/reports/jump-table-04-profitability.md` and
`jump-table-05-policy-and-docs.md` in the same directory.

## Regression Invariants

Integration exposed and fixed four compiler defects. Their regression tests
must not be weakened merely to keep jump-table tests green:

- No-opt binary equality incorrectly emitted branch `beq` as an expression.
  Whole binary equality and inequality now emit `bineq`/`binne`, covered by
  `select_dispatch_strings` for lowered and residual paths.
- Repeated call arguments such as `nested(i, i)` could destructively swap the
  first occurrence before staging the second. Non-primary repeated values are
  now copied to final call slots before swaps; `repro_duplicate_call_argument`
  covers integer/string values and caller preservation.
- Mixed-dispatch rewriting could detach case bodies before capturing the final
  fallback child. The fallback is captured before AST mutation and
  `select_dispatch_mixed` covers dynamic miss paths.
- An eager comparison after inlining could capture a symbol-backed operand as
  stale `.unknown`, producing a nonexistent generic `eq`. Temporary-symbol
  remapping now uses the resolved symbol shape; optimized
  `select_dispatch_mixed` covers this path.

## Future RXAS Optimization

Recognizing arbitrary emitted or hand-written RXAS branch ladders is deferred.
It requires a control-flow graph, reaching definitions, liveness, and a complete
instruction-database audit of register reads, writes, mutation, and flow edges.
The roadmap tracks that work; the current keyhole window must not guess across
arbitrary control flow.

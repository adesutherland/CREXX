# CREXX Decimal Benchmark (CDB-1) specification

Status: **draft 0.1; arithmetic core implemented and correctness-qualified;
no performance result published**

Date: 2026-08-05

## 1. Purpose and naming

CDB-1 is a cREXX-owned, publishable fixed-work benchmark for comparing decimal
providers through the CREXX plugin and VM boundaries. It is informed by public
decimal-arithmetic literature and application shapes, but its source, operands,
checksums and reporting contract are independently authored for CREXX.

CDB-1 is not an IEEE, ANSI or General Decimal Arithmetic standard benchmark.
The standards define required arithmetic semantics. The official `decTest`
files are correctness vectors, not performance workloads. Cowlishaw's Telco is
a useful billing-application reference, but its author explicitly warns that
its narrow operation mix is not suitable for benchmarking decimal
implementations generally. A Telco-derived or billing-style workload may
therefore appear only as one separately named application cell, never as the
whole benchmark or an implied standards-body score.

All CREXX-authored CDB-1 files are distributed under the repository MIT
licence. No third-party source or bulk input data may be copied into CDB-1
without a recorded compatible licence. Public material may inform an original
operation shape; provenance and the independence boundary must remain in the
published result.

## 2. Implemented core

The maintained arithmetic core is:

- `tests/performance/decimal/decimal_gate1_common18.crexx` for Common,
  18 digits;
- `tests/performance/decimal/decimal_gate1_classic9.crexx` for Classic,
  9 digits; and
- `tests/performance/decimal/decimal_gate1_adapter.c` for the direct CREXX
  decimal-plugin ABI boundary.

The Level B programs accept `MODE ITERATIONS OPAQUE_SEED`. Published CDB-1
core results use seed `1`; it is supplied at runtime so optimized code cannot
constant-fold provider arithmetic or comparisons. `all` executes every kernel
only for checksum qualification. A measured cell selects exactly one mode.

| ID | Mode | Work per iteration | Boundary exercised |
| --- | --- | ---: | --- |
| A1 | `arithmetic` | four decimal operations | add, subtract, multiply and exact divide over pre-parsed operands |
| C1 | `conversion` | two conversions | string-to-decimal and decimal-to-string across rotating textual shapes |
| Q1 | `compare` | three comparisons | equality plus early- and late-difference ordering |
| X1 | `context` | one procedure/context event | call/return context synchronization, add and observed result |
| L1 | `ledger` | three decimal actions | parse, signed balance update and formatting |
| F1 | `compound` | 60 decimal actions | 12 rounds of multiply, divide, add, subtract and format |

The adapter payload separately exposes arithmetic, conversion, comparison,
copy/clear and context-sync modes. Adapter and VM results must not be combined
into one score because they answer different attribution questions.

## 3. Correctness contract

A result is reportable only when:

1. the exact provider, context, VM and optimizer image are named;
2. the reported final checksum matches the accepted row for that same context;
3. all three optimizer images retain runtime decimal operations for the opaque
   operands;
4. default/static `mc_decimal` and explicitly loaded dynamic `mc_decimal`
   agree exactly before packaging cost is compared;
5. `db_decimal` Common-18 differences remain visible and are excluded from
   correctness-qualified ranking; and
6. a Classic-9 `db_decimal` cell is admitted only when its output and branch
   checksum exactly matches the D0 baseline.

`decTest`, the cREXX numeric-context tests and provider-specific signal/text
tests remain correctness gates outside the timed region. Passing CDB-1
checksums does not substitute for the full semantic corpus.

## 4. Measurement contract

The harness, not the workload, owns timing and statistics. Workloads perform no
internal timing. Like-for-like rows use one common calibrated iteration count;
calibration must make the fastest row last at least one second without changing
the work. Formal absolute results use two warmups and ten recorded serial
observations with rotated row order. Candidate decisions additionally follow
the paired/interleaved rules in the DECIMAL-01 engineering plan.

Three optimizer modes execute identical work. Compiler-on/RXAS-on is the
primary product score. The same optimized compiler RXAS assembled with RXAS
`-n` isolates assembler effects. Compiler-off/RXAS-off is a broader diagnostic.
The modes are not pooled or averaged. A maintained build-time integrity test
requires runtime parse/format, add, subtract, multiply, divide, equality and
ordering instructions in disassemblies of every final RXBIN image before it is
eligible for timing.

Published results must provide:

- source commit and uncommitted-state declaration;
- operating system, architecture, CPU, compiler and build configuration;
- provider identity/version and static or dynamic selection;
- context, VM, bytecode optimization mode, benchmark mode, seed and iteration
  count;
- exact operation count and checksum;
- every raw observation plus median and spread;
- process/lifecycle and steady-state boundaries kept separate;
- RSS and artifact sizes separately from throughput; and
- invalidated cells and correctness mismatches, not only winning rows.

There is no cross-machine aggregate. Ratios are calculated only against the
same-host, same-context D0 baseline. Canonical RexxCPS remains a separately
reported whole-product guard and does not enter a CDB-1 arithmetic score.

## 5. Planned publishable application extension

Before the first public CDB-1 result, add an independently authored billing
application cell with generated or CREXX-owned input data. Its specification
will publish the transaction schema, rates, rounding points, expected small
worked result, large-input generator, operation profile and checksum. It may
use Telco's public design lessons—explicit decimal conversion, per-item
rounding, taxation and final accumulation—but it must not copy Telco source or
restricted bulk data, and it will be labelled as an original CDB-1 workload.

The application extension must include at least one halfway rounding case and
must report parsing, arithmetic and output costs both together and, where
possible, as separately attributable cells. It remains one portfolio member;
the operation-balanced A1/C1/Q1/X1/L1/F1 cells prevent its particular billing
mix from deciding the provider verdict alone.

## 6. Versioning

Any change to operands, context, work count, checksum, timing boundary or
sampling rules increments the CDB-1 draft revision. Once the first result is
published, such a change creates a new benchmark version; old and new results
must not be pooled. Compiler or harness defect fixes that do not change work
still record the exact source commit and a written compatibility disposition.

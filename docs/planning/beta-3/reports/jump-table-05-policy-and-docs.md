# Jump Table Report 5: Profitability Policy And Documentation

Date: 2026-07-11

## Scope

Apply the profiling findings to compiler policy, preserve positive and negative
code-generation coverage at the new boundaries, remeasure real workloads, and
move every current/future contract out of the working note.

## Policy

The former single three-case threshold is replaced by:

| Dispatch kind | Minimum cases |
| --- | ---: |
| Integer | 8 |
| Exact string | 3 |
| Padded nonnumeric string | 2 |
| Numeric string | 2 |
| Exact binary | 3 |

Integer eight is the first measured uniform-traffic point that beats the branch
ladder on this VM. Padded and numeric strings use two because repeated loose
comparisons are already more expensive than one canonicalization and lookup.
Exact string and binary remain conservatively at three pending equally focused
measurements.

The policy is selected from the canonical dispatch kind after all semantic
eligibility checks. It does not weaken alias, mutation, duplicate, type, or
evaluation-order gates.

## Coverage Changes

- Explicit C-style integer SELECT now has an eight-case optimized/no-opt
  boundary fixture.
- General optimized integer IF/strict-IF fixtures contain eight cases.
- A mixed ladder contains one eight-case constant run followed by dynamic and
  undersized residual runs.
- Existing smaller integer SELECT/IF fixtures prove short ladders remain
  ordinary comparisons.
- Two-case padded and numeric C-style SELECT fixtures prove the lower text
  thresholds in optimized and no-opt builds.
- Exact string and binary three-case fixtures remain covered.

All 20 focused compiler RXAS/runtime tests pass after intentional golden
regeneration.

## Tuned Workload Check

Release medians compared with dispatch completely disabled:

| `tinyexpr` operation | Tuned us | Disabled us | Result |
| --- | ---: | ---: | --- |
| Lex | 44,134 | 45,610 | 3.2% faster |
| Evaluate | 114,417 | 114,149 | flat |
| Token names | 22,530 | 24,087 | 6.5% faster |

The tuned policy gives up a small amount of the blanket three-case lexing gain
in exchange for avoiding the measured four- and six-case integer regressions.

`rxjson` remains within measurement noise because it has no material eligible
ladder. Its next performance work remains algorithmic.

## Permanent Documentation

Current behavior now lives in:

- RXAS syntax and instruction reference for `.jtable`, `.jcase`, and all
  `jump*` forms;
- the programming guide for `rxdas` reconstruction;
- `docs/ai-context/RXBIN_JUMP_TABLES.md` for the packed binary contract,
  ACPH provenance, canonicalization, ownership, and corruption behavior;
- `docs/ai-context/RXC_DISPATCH_OPTIMIZATION.md` for compiler lowering,
  semantic gates, mixed-run handling, and fixed integration regressions;
- the language performance chapter for compiler eligibility and thresholds;
- the Release 1 plan for landed beta 3 WIP status.

Future work now lives in `docs/ROADMAP.md`: RXAS branch-ladder recognition
requires a real control-flow graph, reaching definitions, liveness, and an
instruction-database audit of register reads, writes, mutation, and flow edges.

## Retirement

The final complete build and 1,562-test Debug CTest run passed. The working note
no longer owned a unique current or planned contract after the compiler-side
architecture and integration regressions moved to the durable AI context. It
was therefore removed at the end of this review.

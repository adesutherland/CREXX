# Performance scorecard: `<release/change and date>`

Status: `<release claim | observation | diagnostic | inference | upper bound>`

## Scope

- Source commit/tag:
- Branch and dirty scope:
- Platform/host:
- Build type/options:
- Canonical product mode:
- Runtime substrate and numeric mode:
- Portfolio membership/version:
- Evidence bundle:
- Interpretation boundary:

## Executive result

State the result without combining throughput, lifecycle, memory and artifact
size. Name any regression guard hit or unresolved noisy cell.

## Correctness and comparability

| Workload | CREXX | ooRexx | NetRexx numeric mode/substrate | Regina/control | Aggregate disposition |
| --- | --- | --- | --- | --- | --- |
| `<name>` | `<pass/label>` | `<pass/label>` | `<pass/label>` | `<label or out of scope>` | `<included/excluded and reason>` |

## Canonical throughput

All elapsed-time values identify the lifecycle and units. Native rates stay
separate from process elapsed time.

| Workload | Metric | Work | `rxvm` median (n) | `rxbvm` median (n) | ooRexx median (n) | NetRexx median (n) | Regina/control | Uncertainty |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | --- |
| `<name>` | `<elapsed/rate>` | `<argument>` | | | | | | `<IQR/MAD/range>` |

## Common aggregate

Membership: Sieve, Permute, Bounce, Richards and Base64. `N=5` unless a named
formal revision changes the approved membership.

| Comparison | Geometric mean | Exact membership | Exclusions |
| --- | ---: | --- | --- |
| `rxvm / ooRexx` | | | |
| `rxvm / NetRexx` | | | |
| `rxbvm / ooRexx` | | | |
| `rxbvm / NetRexx` | | | |

Ratios are oriented so larger is better. Missing or non-comparable cells are
never imputed.

## Lifecycle and startup

| Runtime/VM | Phase | Median | n | IQR/MAD/range | Guard status |
| --- | --- | ---: | ---: | --- | --- |
| | compile/translate/assemble/load-first-result | | | | |

## Peak RSS

| Workload | Runtime/VM | Median peak RSS | n | Range | Guard status |
| --- | --- | ---: | ---: | --- | --- |
| | | | | | |

## Artifact size

| Workload/runtime | Source | Generated source | RXAS | RXBIN/class | Linked image/package | Hash reference |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| | | | | | | |

## Diagnostics and controls

List opaque-input, no-TRACE, profiling, stripped-metadata, translated-image,
binary-typed NetRexx, alternate JVM mode, Java/native-C or research controls.
Label each `diagnostic`, `control` or `upper bound`; do not mix it into the
canonical tables or aggregate.

## Regression guards

| Dimension/cell | Observed change | Approved guard | Result/action |
| --- | ---: | ---: | --- |
| | | | |

## Provenance and reproducibility

- Machine/power/thermal/load record:
- Runtime/compiler/JVM versions and flags:
- Source/generated/internal-form/executable hashes:
- Exact command/schedule record:
- Raw samples and correctness/output tables:
- Recursive checksum verification:

## Interpretation and open decisions

Separate facts from inferences. Name material outliers, trade-offs, owners and
next actions. State explicitly whether the evidence qualifies as a release
claim and the host/platform scope of that claim.

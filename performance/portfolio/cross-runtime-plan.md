# Cross-runtime benchmark coverage plan

Status: active comparative scope; 12-item Tier A portfolio approved 2026-07-15

The programme target is stronger than “rerun RexxCPS”: every approved Tier A
workload should be ported, qualified and run on **CREXX, ooRexx and NetRexx**.
**Regina is deliberately limited to RexxCPS.** Java and native C remain
selected controls, not mandatory full-portfolio implementations. A missing or
semantically misleading port is recorded as `not comparable`, not silently
omitted and not included in a common aggregate.

## When comparative runs happen

Comparative work is a pipeline, not one late milestone:

1. **NR-01 inventories and fills coverage.** Select the portfolio, add missing
   workload classes, and identify or implement the runtime-specific ports.
2. **NR-02 ports, qualifies and pilot-runs each workload.** Record source
   provenance, timed-kernel equivalence, observable results,
   optimizer-resistance variants and disclosed substitutions. RexxCPS is first
   across CREXX, ooRexx, NetRexx and Regina. Every other selected Tier A
   workload then follows across CREXX, ooRexx and NetRexx only.
3. **NR-10 records formal baselines.** As soon as a workload clears NR-02, run
   its same-host implementation matrix and retain raw results and runtime
   forensics. NR-10 therefore overlaps later NR-01/NR-02 expansion.
4. **NR-11 defines aggregation.** Only the common qualified CREXX/ooRexx/
   NetRexx subset contributes to the cross-runtime portfolio aggregate.
   Regina's RexxCPS result and other controls remain visible separately.

## NR-02 exit criterion

NR-02 is complete only when:

- RexxCPS canonical and optimizer-resistance variants have provenance,
  correctness, equivalence notes and retained pilot results for CREXX, ooRexx,
  NetRexx and Regina;
- every other selected Tier A workload has a CREXX, ooRexx and NetRexx source
  or an explicit `not comparable` decision, plus correctness, equivalence and
  optimizer-resistance evidence appropriate to that workload;
- each qualified CREXX/ooRexx/NetRexx workload has at least one retained pilot
  cross-runtime run before it enters the formal NR-10 baseline lane; and
- all substitutions, warmup/runtime behavior and observable-result checks are
  recorded. Regina has no NR-02 obligation beyond RexxCPS.

## Coverage completion rule

Do not choose a benchmark count merely because it is round. Close named
coverage gaps first. Adrian approved a deliberately bounded **12-item Tier A
portfolio** on 2026-07-15: the five seed workloads, six additional steady-state
workloads, and one separately reported lifecycle lane. The portfolio applies to
the common CREXX/ooRexx/NetRexx comparison, not to Regina.

| Coverage category | Current seed | Planned completion work |
| --- | --- | --- |
| Classic mixed semantics | RexxCPS 2.2c for cREXX | Canonical/ported RexxCPS plus optimizer-resistance family across runtimes |
| Integer arrays and indexing | AWFY Sieve | Add another array/iteration workload if the expanded set does not provide one |
| Calls, recursion and return handling | AWFY Permute; Towers | Add direct calls/recursion and larger application-call-graph coverage |
| Floating point, branches and bit operations | AWFY Mandelbrot | Retain; add only if cross-runtime comparability exposes a gap |
| Objects and allocation | AWFY Towers | Add Bounce/Storage and record Classic-runtime adaptation limits |
| Collections and lookup | none dedicated | Add List; JSON adds structured collection/lookup pressure |
| Parser and text processing | none dedicated | Add a deterministic RAP-shaped JSON parser capability workload |
| Binary processing | none | Add an RFC 4648 Base64 in-memory round trip with checksum observation |
| Larger application algorithms | none | Add Richards; retain DeltaBlue/Havlak as reserves |
| Startup, load and first result | process timing only | Add explicit compile/translate, cREXX assemble and load-to-first-result lifecycle cases, reported outside the steady-state aggregate |
| Integration/application behavior | none | Richards supplies the bounded application-style call graph |
| PARSE, stems, TRACE and ADDRESS | present inside RexxCPS mix | Keep focused probes in Tier B unless profiling demonstrates a coverage gap |

The approved additions are Bounce, Storage, List, Richards, JSON, an RFC 4648
Base64 round trip, and compile/load/first-result lifecycle measurement. Queens,
DeltaBlue, Havlak, filesystem-I/O workloads and focused Classic-semantics probes
remain Tier B/reserve work. They are not silently included in the Tier A
aggregate.

## Initial implementation matrix

`Inventory/port required` means no result should be published until the source,
algorithm and correctness contract have been identified. This table is updated
as part of NR-01 and NR-02.

| Workload | CREXX | ooRexx | Regina | NetRexx | Java/control |
| --- | --- | --- | --- | --- | --- |
| RexxCPS | qualified 2.2c disclosed adaptation | qualified canonical 2.2 plus A/B diagnostics | qualified canonical 2.2 plus A/B diagnostics | qualified disclosed 2.2n adaptation; unchanged 2.1n retained/excluded | mechanical/dynamic C ceilings remain optional; Java only if a defensible port is added |
| AWFY Sieve | qualified equivalent port | qualified equivalent Classic/stem port | out of scope: RexxCPS only | qualified equivalent port | inventory/provenance review required |
| AWFY Permute | qualified equivalent port | qualified equivalent procedural port | out of scope: RexxCPS only | qualified equivalent object port | inventory/provenance review required |
| AWFY Mandelbrot | qualified equivalent port | `not comparable`: decimal numerics fail common checksums at sizes 500/750 | out of scope: RexxCPS only | disclosed arithmetic-XOR adaptation; aggregate review open | inventory/provenance review required |
| AWFY Towers | qualified equivalent object port | correct procedural diagnostic but `not comparable` for object/allocation scoring | out of scope: RexxCPS only | qualified equivalent object port | inventory/provenance review required |
| AWFY Bounce | qualified equivalent reference/object port | qualified equivalent object port | out of scope: RexxCPS only | qualified equivalent object port | upstream Java reference available |
| AWFY Storage | correct disclosed node-wrapper diagnostic; `not comparable` for allocation score | qualified equivalent object/array port | out of scope: RexxCPS only | qualified equivalent object/array port | upstream Java reference available |
| AWFY List | qualified disclosed weak-reference arena; aggregate review open | qualified equivalent object port | out of scope: RexxCPS only | qualified equivalent object port | upstream Java reference available |
| AWFY Richards | qualified common state-machine adaptation | qualified common state-machine adaptation | out of scope: RexxCPS only | qualified common state-machine adaptation | upstream queue/hold result retained |
| JSON capability probe | correct path/count diagnostic; `not comparable` to DOM cells | correct supplied-DOM diagnostic | out of scope: RexxCPS only | correct Java-collections-DOM diagnostic | common correctness only; no common timing score |
| RFC 4648 Base64 | qualified preallocated `.binary` port | qualified byte-string port | out of scope: RexxCPS only | qualified Java `byte[]` port | exact round-trip/length/checksum contract |
| Compile/load/first-result | qualified compile/assemble/load-first-result capture | qualified translate/load-first-result capture | out of scope: RexxCPS only | qualified compile/JVM-load-first-result capture | reported separately; no load-only CLI boundary |

The port exercise's confirmed capability gaps and accuracy checks are retained
in `performance/capability-gaps.md`. In particular, references and binary-safe
storage are present cREXX capabilities; the open questions are nested
heterogeneous containers, a parse-once JSON representation, reusable Base64
library coverage and a more precise loader measurement boundary.

## Comparability labels

Every result cell uses one of these labels:

- `canonical`: the recognized upstream workload for that implementation;
- `equivalent port`: the same algorithm, timed work and observable result with
  language-required spelling changes;
- `disclosed adaptation`: material substitutions exist and are documented;
- `control`: an intentionally different ceiling or mechanism comparison;
- `not comparable`: the semantic or algorithmic gap cannot support the common
  score; or
- `out of scope`: deliberately excluded from that implementation's programme
  lane, such as non-RexxCPS Regina workloads; or
- `pending`: source, port, provenance or qualification work is incomplete.

Classic runtimes do not need to imitate cREXX classes merely to fill a cell.
Use an algorithmically equivalent procedural representation when it preserves
the intended measured work; otherwise mark the workload not comparable and
exclude it from the common aggregate.

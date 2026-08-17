# Performance portfolio manifest v2

Manifest version: 2

Status: **source contract frozen; correctness qualification and first formal
Mac baseline complete**

Approved for PERF3 closeout: 2026-08-17

Version 2 starts a new evidence series. It does not rewrite the retained seed
manifest, earlier scorecards, canonical AWFY sources, or RexxCPS 2.2d. It adds
explicit faster-source controls and reserve benchmarks so future compiler work
can be judged against a broader, named surface without changing historical
cells in place.

## Runtime identities

| Executable | Identity | Aggregate rule |
| --- | --- | --- |
| `rxvm` | compiler-selected cREXX product | authoritative cREXX product cell; exactly one per workload |
| `rxtvm` | direct threaded concrete-engine control | optional control; never substitutes for `rxvm` |
| `rxbvm` | portable concrete-engine control | optional control; omit when it is executable-alias equivalent to the selected `rxvm` |
| `rexx` | qualified ooRexx runtime | reference cell where the source is comparable |
| `java` plus NetRexx classes | qualified NetRexx/default-HotSpot substrate | reference or labelled JVM control according to the source contract |
| Regina | canonical RexxCPS-only control | never a portfolio aggregate member |
| CPython / Java source ports | language controls | visible separately; never Rexx aggregate members |

The convenience `run_benchmarks.crexx` output is labelled `process-smoke`.
Formal evidence uses `run_cross_runtime_matrix.crexx`, which emits only the
product/concrete aggregate rows actually present. It does not invent absent
engine cells.

## Workload contract

| ID | Source | Class and provenance | v2 comparability | v2 common aggregate |
| --- | --- | --- | --- | --- |
| `sieve` | `awfy_sieve.crexx` | AWFY/SOM integer arrays and iteration | retained equivalent port | yes |
| `permute` | `awfy_permute.crexx` | AWFY/SOM recursion, calls and mutation | retained equivalent port | yes |
| `mandelbrot` | `awfy_mandelbrot.crexx` | canonical AWFY/Benchmarks Game source | canonical cREXX cell; cross-runtime exclusions retained | no |
| `mandelbrot-hoisted` | `awfy_mandelbrot_hoisted_control.crexx` | v2 source-hoisted invariant-conversion diagnostic | cREXX control only | no |
| `towers` | `awfy_towers.crexx` | AWFY/SOM objects, allocation and recursion | retained qualified separate lane | no |
| `bounce` | `awfy_bounce.crexx` | AWFY/SOM object/reference dispatch | retained equivalent port | yes |
| `storage` | `awfy_storage.crexx` | AWFY/SOM allocation with disclosed cREXX wrapper | retained diagnostic | no |
| `list` | `awfy_list.crexx` | AWFY/SOM list with disclosed weak-reference arena | retained diagnostic | no |
| `richards` | `awfy_richards.crexx` | AWFY/SOM larger state-machine call graph | retained common adaptation | yes |
| `json-legacy` | `json_parser.crexx` | historical RAP-shaped capability probe | retained historical diagnostic | no |
| `json-parse` | `json_parse.crexx` | v2 construct-and-validate fresh indexed document | cREXX parse control | no |
| `json-query` | `json_query.crexx` | v2 parse-once indexed traversal | cREXX query control | no |
| `base64-v1` | `base64_roundtrip.crexx` | historical RFC 4648 round trip | retained historical cell | no in v2 |
| `base64-v2` | `base64_roundtrip_v2.crexx` plus v2 ooRexx/NetRexx ports | RFC 4648 with each runtime's fastest disclosed byte surface and arithmetic digit decode | new qualified-series candidate | yes, replacing only the v2 Base64 member |
| `rexxcps` | `rexxcps_levelb.crexx` | RexxCPS 2.2d for cREXX | retained disclosed adaptation and native CPS | no; mandatory separate representative row |
| `queens` | `awfy_queens.crexx` | AWFY/SOM commit `74306fec151070fd07157cefeacf19e7e0bcdc89` | Tier B equivalent cREXX port | no |
| `nbody` | `awfy_nbody.crexx` | AWFY/Benchmarks Game via the same pinned AWFY commit; disclosed `rxmath` square root | Tier B native-math control | no |
| `lifecycle` | existing cross-runtime lifecycle probes | compile/translate, assemble and load-to-first-result | retained separate dimension | never |

The v2 common aggregate is exactly Sieve, Permute, Bounce, Richards and
Base64-v2 across the qualified cREXX product, ooRexx and decimal-semantics
NetRexx cells. Its membership count remains five, but its Base64 source has a
new identity, so it must never be compared as though it were the v1 aggregate.

## Correctness and measurement contract

- Build every new cREXX source in optimized and unoptimized modes.
- Require its deterministic `PASS:` marker and RC 0 under `rxvm`, `rxtvm` and
  `rxbvm` where the same runtime substrate applies.
- NBody must disclose and load `rxmath`; it is not a pure bytecode/runtime
  comparison.
- Validate Base64-v2 length 1,368, byte equality and checksum 130,560 in all
  three language implementations before any timing.
- Keep JSON construction (`json-parse`) separate from parse-once traversal
  (`json-query`). Neither is a full AWFY Json port.
- Keep the canonical and hoisted Mandelbrot sources separate in every artifact
  name and table.
- Formal timing remains ordinary profiling-off Release, two warmups and ten
  serial recorded processes per cell with rotating runtime order.
- Retain raw samples, product/tool/source hashes, build settings, host state,
  lifecycle, RSS and artifact results under a new dated v2 evidence directory.

Full AWFY Json, DeltaBlue, CD and Havlak are deliberately post-PERF3. They need
their own provenance, equivalence and qualification gates before promotion.

The first formal Apple ARM64 baseline is retained in
[`2026-08-17-perf3-closeout-current-mac-v2`](../evidence/2026-08-17-perf3-closeout-current-mac-v2/).
It starts the v2 evidence series; it does not numerically extend the v1
aggregate.

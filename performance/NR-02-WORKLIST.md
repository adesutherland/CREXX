# NR-02 resumable cross-runtime worklist

Status: in progress

Started: 2026-07-15

This is the temporary control plane for NR-02. Keep it until every exit
criterion in `portfolio/cross-runtime-plan.md` is met. Update one workload /
runtime cell at a time. A pilot result is evidence for qualification only;
formal repeated baselines and runtime forensics transfer to NR-10.

## State vocabulary

- `pending`: the named check or artifact has not been completed;
- `pass`: the check completed and retained evidence identifies the command;
- `review`: evidence exists but equivalence or disposition needs review;
- `blocked`: an external runtime, source, or decision is unavailable;
- `n/a`: the check does not apply to this cell.

Disposition uses the programme labels: `canonical`, `equivalent port`,
`disclosed adaptation`, `control`, `not comparable`, `out of scope`, or
`pending`.

## Environment and installation gates

| Gate | State | Retained fact / next action |
| --- | --- | --- |
| Branch and dirty scope captured | pass | `develop` at `44dd4dbf3da624e2d8e79eccf696f78f023d436e`; intentional performance-programme dirty tree |
| Java/JDK inventory | pass | Temurin OpenJDK/JDK 26.0.1; exact output belongs in the dated evidence bundle |
| Regina inventory | pass | Regina 3.9.7, Homebrew `regina-rexx`; both `rexx` and `regina` resolve to Regina |
| ooRexx inventory and install | pass | Homebrew had no ooRexx formula/cask and its `rexx` is Regina; official stable 5.1.0 r12973 portable universal macOS build installed user-locally under `/Users/adrian/.local/opt/oorexx/5.1.0-12973` |
| NetRexx official release check | pass | official `netrexx.org` current release is 5.10-GA, released 2026-03-20 |
| NetRexx user-local installation | pass | installed under `/Users/adrian/.local/opt/netrexx/5.10-GA`; compile/run proof, generated Java, class and `javap` retained in the dated evidence bundle |

## Workload / runtime cells

Each row must record source provenance and license, correctness, timed-kernel
equivalence, optimizer resistance, a retained pilot, and final disposition.

| Workload | Runtime | Source provenance / license | Correctness | Equivalence | Optimizer resistance | Pilot | Disposition / next gate |
| --- | --- | --- | --- | --- | --- | --- | --- |
| RexxCPS | CREXX | `tests/benchmarks/rexxcps_levelb.crexx`, RexxCPS 2.2c Level B port of ooRexx 2.2; CPL 1.0 plus retained upstream redistribution terms | pass: canonical and A/B | report audit plus retained NR-02 timed-kernel ledger | pass: opaque A/B and observed `1|69|1.22694`; opt/no-opt tests; partial trace retained | pass: one full-default canonical/A/B each | disclosed adaptation; available-runtime cell qualified, bounded dynamic-count follow-up open |
| RexxCPS | ooRexx | official ooRexx `samples/rexxcps.rex` 2.2; CPL 1.0 plus retained redistribution terms | pass: canonical and A/B | canonical 2.2 plus separately named diagnostic | pass: opaque A/B observed `1|69|1.22694`; count-one `TRACE I`; translated images retained and executed | pass: 1 warmup + 3 canonical; 1 + 2 per A/B | canonical; ooRexx RexxCPS lane qualified and complete |
| RexxCPS | Regina | byte-exact official ooRexx 2.2 source run on Regina; same source terms | pass: canonical and A/B | canonical Classic source on alternate runtime | pass: opaque A/B, observed `1|69|1.22694`, completed count-one source trace | pass: 1 warmup + 3 canonical; 1 + 2 per A/B | canonical; Regina NR-02 lane qualified and complete |
| RexxCPS | NetRexx | byte-exact bundled 5.10-GA 2.1n plus separate `rexxcps_2_2n.nrx`; distribution `LICENSE` plus RexxCPS provenance | pass: 2.1n, 2.2n and A/B | diff proves 2.2n timed kernel is bundled 2.1n; self-calibration/reporting disclosed | pass: opaque A/B, observed `1|69|1.22694`, generated Java/class/`javap` retained | pass: 1 warmup + 3 2.2n; 1 + 2 per A/B | disclosed adaptation; available-runtime cell qualified; 2.1n short-timer result excluded |
| Sieve | CREXX | `tests/benchmarks/awfy_sieve.crexx`; AWFY/SOM MIT | pass: 669 primes | same 5,000-slot sieve, bounds and marking loop | pass: runtime repetitions; 1/2 perturbations; observed result | pass: 50 repetitions, 1 warmup + 3 recorded | equivalent port; available-runtime cell qualified |
| Sieve | ooRexx | `tests/benchmarks/cross-runtime/classic/awfy_sieve.rex`; procedural port of AWFY/SOM; MIT | pass: 669 primes at 1/2 and pilot size | same sieve bounds/marking; stems are the Classic array representation | pass: runtime repetitions, observed result and executable translated image | pass: 50 repetitions, 1 warmup + 3 recorded | equivalent port; seed cell qualified |
| Sieve | NetRexx | `tests/benchmarks/cross-runtime/netrexx/awfy_sieve.nrx`; port of versioned AWFY/SOM algorithm; MIT | pass: 669 primes | same 5,000-slot sieve, bounds and marking loop; primitive Java `int[]` | pass: runtime repetitions; 1/2 perturbations; generated Java/result path inspected | pass: 50 repetitions, 1 warmup + 3 recorded | equivalent port; available-runtime cell qualified |
| Permute | CREXX | `tests/benchmarks/awfy_permute.crexx`; AWFY/SOM MIT | pass: 8,660 calls | same recursion, object construction, six-slot mutation and paired swaps | pass: runtime repetitions; 1/2 perturbations; observed count | pass: 50 repetitions, 1 warmup + 3 recorded | equivalent port; available-runtime cell qualified |
| Permute | ooRexx | `tests/benchmarks/cross-runtime/classic/awfy_permute.rex`; procedural port of AWFY/SOM; MIT | pass: 8,660 calls at 1/2 and pilot size | same recursion, six-slot stem mutation and paired swaps; procedural Classic representation disclosed | pass: runtime repetitions, observed count and executable translated image | pass: 50 repetitions, 1 warmup + 3 recorded | equivalent port; seed cell qualified |
| Permute | NetRexx | `tests/benchmarks/cross-runtime/netrexx/awfy_permute.nrx`; port of versioned AWFY/SOM algorithm; MIT | pass: 8,660 calls | same recursion, object construction, six-slot mutation and paired swaps | pass: runtime repetitions; 1/2 perturbations; generated Java/classes/result path inspected | pass: 50 repetitions, 1 warmup + 3 recorded | equivalent port; available-runtime cell qualified |
| Mandelbrot | CREXX | `tests/benchmarks/awfy_mandelbrot.crexx`; AWFY/Benchmarks Game Revised BSD | pass: 1/500/750 checksums 128/191/50 | versioned AWFY loop, floating update, byte packing and XOR | pass: runtime size; three perturbations; observed checksum | pass: size 500, 1 warmup + 3 recorded | equivalent port; available-runtime cell qualified |
| Mandelbrot | ooRexx | `tests/benchmarks/cross-runtime/classic/awfy_mandelbrot.rex`; procedural port of AWFY; Revised BSD | fail at common sizes: size 500 gives 255 not 191; size 750 gives 128 not 50; size 1 passes | same fractal loop, but ooRexx decimal arithmetic changes boundary results; arithmetic XOR/padding also add timed work | pass as a negative check: runtime sizes, observed failures and executable translated image retained | no score: serial size-500/750 negative runs retained | not comparable; excluded from the common aggregate rather than accepting runtime-specific answers |
| Mandelbrot | NetRexx | `tests/benchmarks/cross-runtime/netrexx/awfy_mandelbrot.nrx`; port of versioned AWFY algorithm; Revised BSD | pass: 1/500/750 checksums 128/191/50 | same fractal loop; NetRexx lacks integer `^`, so exact XOR uses an eight-step arithmetic helper and partial-byte shift uses a loop, adding timed work | pass: runtime size; three perturbations; generated Java/class/result path inspected | pass: size 500, 1 warmup + 3 recorded | disclosed adaptation; common-aggregate equivalence review required |
| Towers | CREXX | `tests/benchmarks/awfy_towers.crexx`; AWFY/SOM MIT | pass: 8,191 moves | same disk allocation, pile operations and 13-disk recursion | pass: runtime repetitions; 1/2 perturbations; observed moves | pass: 10 repetitions, 1 warmup + 3 recorded | equivalent port; available-runtime cell qualified |
| Towers | ooRexx | `tests/benchmarks/cross-runtime/classic/awfy_towers.rex`; procedural AWFY/SOM adaptation; MIT | pass: 8,191 moves at 1/2 and pilot size | recursion/link mutations preserved; numeric node ids/stems do not preserve object dispatch or allocator cost | pass for the diagnostic adaptation: runtime repetitions, observed moves and executable translated image | pass: 10 repetitions, 1 warmup + 3 recorded | not comparable for the common object/allocation score; retained as a disclosed procedural diagnostic unless replaced |
| Towers | NetRexx | `tests/benchmarks/cross-runtime/netrexx/awfy_towers.nrx`; object port of AWFY/SOM; MIT | pass: 8,191 moves | same disk objects/allocation, pile operations and 13-disk recursion; primitive arrays hold three pile roots/presence flags | pass: runtime repetitions; 1/2 perturbations; generated Java/classes/result path inspected | pass: 10 repetitions, 1 warmup + 3 recorded | equivalent port; available-runtime cell qualified |
| Non-RexxCPS seed workloads | Regina | programme scope decision | n/a | n/a | n/a | n/a | out of scope |

## Exact expanded Tier A working proposal

The proposed common CREXX / ooRexx / NetRexx Tier A working set is 16
workloads. Approval remains an NR-11 decision; NR-01/NR-02 may inventory and
prototype the uncontested candidates without treating this proposal as final.

| # | Workload | Primary coverage role |
| ---: | --- | --- |
| 1 | RexxCPS | Classic mixed semantics; PARSE, stems, TRACE and ADDRESS mix |
| 2 | Sieve | integer arrays, indexing and nested iteration |
| 3 | Permute | recursion, calls, returns and array mutation |
| 4 | Mandelbrot | floating point, branches and bit operations |
| 5 | Towers | objects, allocation and recursion |
| 6 | Queens | backtracking, direct recursion and arrays |
| 7 | Bounce | object dispatch and allocation |
| 8 | Storage | sustained allocation and tree construction |
| 9 | List | dedicated list construction and traversal |
| 10 | Richards | larger scheduler/application call graph |
| 11 | DeltaBlue | constraints plus collection/lookup pressure |
| 12 | JSON | parser and text processing |
| 13 | deterministic binary codec | binary byte semantics and checksum observation |
| 14 | compile/load/first-result | explicit startup lifecycle separate from steady state |
| 15 | focused Classic semantics matrix | PARSE, stems, TRACE and ADDRESS outside the RexxCPS mix |
| 16 | deterministic file-processing application | integration, I/O, text/collection work and end-to-end result |

Havlak remains the first reserve if DeltaBlue does not provide credible
map/set/graph coverage across all three required runtimes. The set closes the
named coverage cells without selecting both large graph workloads before their
port/equivalence cost is known.

## Resumption order

1. Keep the completed RexxCPS four-runtime slice and current five-workload seed
   evidence immutable; transfer only qualified cells to NR-10. Mandelbrot/
   ooRexx and Towers/ooRexx remain explicit `not comparable` cells.
2. Resolve the NR-11 approval/disposition of the exact 16-workload proposal.
   Until then, prototype only uncontested additions without calling the Tier A
   portfolio final.
3. Continue one workload/runtime cell at a time, starting with Queens, Bounce,
   Storage and List to close recursion, object/allocation and collection gaps.
4. For each addition, run correctness and perturbation checks before timing,
   retain generated/translated forms, and move it to NR-10 only after every
   applicable CREXX/ooRexx/NetRexx column passes or records an honest
   `not comparable` decision.

Verified validation command for the current seed slice:

```sh
ctest --test-dir cmake-build-release -L benchmark --output-on-failure --parallel 10
```

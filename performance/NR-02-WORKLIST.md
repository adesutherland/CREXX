# NR-02 resumable cross-runtime worklist

Status: complete for the approved portfolio; formal baselines transfer to NR-10

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
| Branch and dirty scope captured | pass | expansion built on `develop` commit `3aee597382c90152b31a4a772e70ceac174688dc`; intentional performance-programme dirty tree retained in the dated bundle |
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
| RexxCPS | CREXX | `tests/benchmarks/rexxcps_levelb.crexx`, RexxCPS 2.2d Level B port of ooRexx 2.2; CPL 1.0 plus retained upstream redistribution terms | pass: canonical and A/B | report audit plus retained NR-02 timed-kernel ledger | pass: opaque A/B and observed `1|69|1.22694`; opt/no-opt tests; partial trace retained | pass: one full-default canonical/A/B each | 2.2d accepted under NUMERIC-01 with explicit digits 9 and honest native types; prior 2.2c evidence remains historical |
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
| Bounce | CREXX | `tests/benchmarks/awfy_bounce.crexx`; AWFY/SOM MIT | pass: 1,331 bounces at 1/2 | object/typed-array port; PRNG and ball slots use explicit references | pass: opt/no-opt, runtime repetition and observed result | pass: 100 repetitions, 1 warmup + 3 recorded | equivalent reference/object port; qualified |
| Bounce | ooRexx | `tests/benchmarks/cross-runtime/oorexx/awfy_bounce.rex`; AWFY/SOM MIT | pass: 1,331 bounces at 1/2 | object-native port with same PRNG, 100 balls and 50 steps | pass: runtime repetition and translated image | pass: 100 repetitions, 1 warmup + 3 recorded | equivalent object port; qualified |
| Bounce | NetRexx | `tests/benchmarks/cross-runtime/netrexx/awfy_bounce.nrx`; AWFY/SOM MIT | pass: 1,331 bounces at 1/2 | object-native port with primitive ball array | pass: runtime repetition; generated Java/classes retained | pass: 100 repetitions, 1 warmup + 3 recorded | equivalent object port; qualified |
| Storage | CREXX | `tests/benchmarks/awfy_storage.crexx`; AWFY/SOM MIT | pass: 5,461 logical nodes at 1/2 | arrays are not object values and nested reference containers are unavailable, so each logical array is a `StorageNode` plus `.object[]` | pass: opt/no-opt, runtime repetition and observed count | pass: 10 repetitions, 1 warmup + 3 recorded | correct disclosed adaptation; `not comparable` for common allocation score |
| Storage | ooRexx | `tests/benchmarks/cross-runtime/oorexx/awfy_storage.rex`; AWFY/SOM MIT | pass: 5,461 allocations at 1/2 | object/array tree preserves upstream allocation shape | pass: runtime repetition and translated image | pass: 10 repetitions, 1 warmup + 3 recorded | equivalent port; qualified |
| Storage | NetRexx | `tests/benchmarks/cross-runtime/netrexx/awfy_storage.nrx`; AWFY/SOM MIT | pass: 5,461 allocations at 1/2 | Java object arrays preserve upstream allocation shape | pass: runtime repetition; generated Java/classes retained | pass: 10 repetitions, 1 warmup + 3 recorded | equivalent port; qualified |
| List | CREXX | `tests/benchmarks/awfy_list.crexx`; AWFY/SOM MIT | pass: length 10 at 1/2 | links are explicit weak references; a typed-array arena owns targets | pass: opt/no-opt, runtime repetition and observed length | pass: 100 repetitions, 1 warmup + 3 recorded | disclosed reference/ownership adaptation; aggregate review open |
| List | ooRexx | `tests/benchmarks/cross-runtime/oorexx/awfy_list.rex`; AWFY/SOM MIT | pass: length 10 at 1/2 | object links and recursive tail algorithm preserved | pass: runtime repetition and translated image | pass: 100 repetitions, 1 warmup + 3 recorded | equivalent object port; qualified |
| List | NetRexx | `tests/benchmarks/cross-runtime/netrexx/awfy_list.nrx`; AWFY/SOM MIT | pass: length 10 at 1/2 | object links and recursive tail algorithm preserved | pass: runtime repetition; generated Java/classes retained | pass: 100 repetitions, 1 warmup + 3 recorded | equivalent object port; qualified |
| Richards | CREXX | `tests/benchmarks/awfy_richards.crexx`; AWFY-derived MIT | pass: 23,246 queued / 9,297 held at 1/2 | common task-kind state machine preserves queues, packets, task state and result | pass: opt/no-opt, runtime repetition and both counters | pass: 1 repetition, 1 warmup + 3 recorded | common disclosed state-machine adaptation; qualified |
| Richards | ooRexx | `tests/benchmarks/cross-runtime/oorexx/awfy_richards.rex`; AWFY-derived MIT | pass: 23,246 / 9,297 at 1/2 | same state-machine representation as the other ports | pass: runtime repetition and translated image | pass: 1 repetition, 1 warmup + 3 recorded | common disclosed state-machine adaptation; qualified |
| Richards | NetRexx | `tests/benchmarks/cross-runtime/netrexx/awfy_richards.nrx`; AWFY-derived MIT | pass: 23,246 / 9,297 at 1/2 | same state-machine representation; arithmetic integer XOR helper | pass: runtime repetition; generated Java/classes retained | pass: 1 repetition, 1 warmup + 3 recorded | common disclosed state-machine adaptation; qualified |
| JSON | CREXX | `tests/benchmarks/json_parser.crexx`; deterministic RAP-shaped fixture | pass: 8 operations at 1/2 | `rxjson` reparses a string/path count; no DOM is built | pass: opt/no-opt, runtime repetition and observed count | pass: 5,000 repetitions, 1 warmup + 3 recorded | native-surface diagnostic; `not comparable` to DOM cells |
| JSON | ooRexx | `tests/benchmarks/cross-runtime/oorexx/json_parser.rex`; same fixture | pass: root directory and 8 operations at 1/2 | supplied `json.cls` builds the ooRexx object model | pass: runtime repetition and translated image | pass: 5,000 repetitions, 1 warmup + 3 recorded | native-surface diagnostic; no common JSON timing score |
| JSON | NetRexx | `tests/benchmarks/cross-runtime/netrexx/json_parser.nrx`; same fixture | pass: 8 operations at 1/2 | parser builds Java `LinkedHashMap`/`ArrayList` DOM | pass: runtime repetition; generated Java/classes retained | pass: 5,000 repetitions, 1 warmup + 3 recorded | native-surface diagnostic; no common JSON timing score |
| Base64 | CREXX | `tests/benchmarks/base64_roundtrip.crexx`; deterministic RFC 4648 | pass: 1,368 encoded bytes / 130,560 checksum at 1/2 | same arithmetic codec; pre-sized `.binary` output/input and direct byte access | pass: opt/no-opt, runtime repetition and byte equality | pass: 500 repetitions, 1 warmup + 3 recorded | equivalent binary port; qualified |
| Base64 | ooRexx | `tests/benchmarks/cross-runtime/oorexx/base64_roundtrip.rex`; deterministic RFC 4648 | pass: length/checksum/equality at 1/2 | same arithmetic codec using byte strings | pass: runtime repetition and translated image | pass: 500 repetitions, 1 warmup + 3 recorded | equivalent byte-string port; qualified |
| Base64 | NetRexx | `tests/benchmarks/cross-runtime/netrexx/base64_roundtrip.nrx`; deterministic RFC 4648 | pass: length/checksum/equality at 1/2 | same arithmetic codec using Java `byte[]` | pass: runtime repetition; generated Java/classes retained | pass: 500 repetitions, 1 warmup + 3 recorded | equivalent byte-array port; qualified |
| Lifecycle | CREXX | `lifecycle/lifecycle_probe.crexx` plus `performance/tools/run_lifecycle.crexx`; MIT | pass: Fibonacci result 6,765 | compile, assemble and combined load-to-first-result are separate rows | pass: every phase/correctness check | pass: 3 per phase | separate lifecycle diagnostic; qualified |
| Lifecycle | ooRexx | `lifecycle/lifecycle_probe.rex`; MIT | pass: result 6,765 | translate and combined load-to-first-result | pass: every phase/correctness check | pass: 3 per phase | separate lifecycle diagnostic; qualified |
| Lifecycle | NetRexx | `lifecycle/lifecycle_probe.nrx`; MIT | pass: result 6,765 | compile and combined JVM-load-to-first-result | pass: every phase/correctness check | pass: 3 per phase | separate lifecycle diagnostic; qualified |
| Non-RexxCPS seed workloads | Regina | programme scope decision | n/a | n/a | n/a | n/a | out of scope |

## Approved Tier A portfolio

Adrian approved the bounded common CREXX / ooRexx / NetRexx portfolio on
2026-07-15. It contains 11 steady-state workloads plus one separately reported
lifecycle lane. Regina remains RexxCPS-only.

| # | Workload | Primary coverage role |
| ---: | --- | --- |
| 1 | RexxCPS | Classic mixed semantics; PARSE, stems, TRACE and ADDRESS mix |
| 2 | Sieve | integer arrays, indexing and nested iteration |
| 3 | Permute | recursion, calls, returns and array mutation |
| 4 | Mandelbrot | floating point, branches and bit operations |
| 5 | Towers | objects, allocation and recursion |
| 6 | Bounce | object dispatch and short-lived allocation |
| 7 | Storage | sustained allocation and tree construction |
| 8 | List | linked-list construction, traversal and reference mutation |
| 9 | Richards | larger scheduler/application call graph and state transitions |
| 10 | JSON | parser, text, arrays, maps and lookup processing |
| 11 | RFC 4648 Base64 round trip | binary/byte semantics, shifts, masks, buffers and checksum observation |
| 12 | compile/load/first-result | explicit startup lifecycle, kept outside the steady-state aggregate |

Queens, DeltaBlue, Havlak, filesystem-I/O work and focused Classic-semantics
probes remain Tier B/reserve candidates. They are added only if the approved
set exposes a concrete coverage gap.

## Resumption order

1. Keep the completed RexxCPS, seed and expansion qualification evidence
   immutable; transfer only qualified/comparable cells to NR-10.
2. Preserve explicit exclusions: Mandelbrot/ooRexx, Towers/ooRexx,
   Storage/cREXX and all JSON native-surface timings are not common scores.
   List/cREXX remains a disclosed weak-reference ownership adaptation for NR-11
   aggregate review.
3. Keep the lifecycle lane separate from the steady-state portfolio aggregate;
   it measures compile/translate, cREXX assemble and combined
   load-to-first-result rather than kernel work.
4. Use `performance/capability-gaps.md` for follow-on closure decisions. Do not
   convert a benchmark workaround into a language change without a focused
   correctness contract and measured performance case.

Verified validation command for the portfolio benchmark slice:

```sh
ctest --test-dir cmake-build-release -L benchmark --output-on-failure --parallel 10
```

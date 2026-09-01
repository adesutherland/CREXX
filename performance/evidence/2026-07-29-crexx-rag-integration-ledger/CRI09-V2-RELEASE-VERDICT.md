# CRI-09 V2 ordinary Release verdict

Status: **mixed and rejected by the unchanged compatibility guard; mandatory
decision stop resolved 2026-07-30**

Date: 2026-07-30

Approval record: Adrian approved A2 and B1, with A2 implementation and its
mandatory ordinary Release benchmark to complete before B1 is documented and
closed. V2 remains the frozen comparator. CRI-09 is unfrozen only for A2's
private lean hybrid hot-core changes; the public Option B surface, ownership,
diagnostics, ABI, formats and existing performance guard remain unchanged.

## A2 correctness freeze

A2 production is frozen at `rxjson.crexx` SHA-256
`c2ff6f246ecf8f13837cd82a1f6c5e35cc6f855fa18889eef6a48ee0128a9318`.
The decisive benchmark remains byte-identical at SHA-256
`f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b`.
Focused correctness passes 11/11, zero failed or skipped, across legacy,
document and noisy contracts, optimized/non-optimized compilation, and both
VMs. Raw log `/tmp/cri09-a2-focused.log` has SHA-256
`2c2cd916eb8b03655abe84f5c605d9d49706f4c4710078ccececb990174a5b00`.
`git diff --check` passed. Production and benchmark sources are frozen for the
A2 ordinary profiling-off Release comparison; B1 and broad closeout remain
deferred until that verdict is accepted.

## Frozen source and correctness

Adrian approved the V2 shared-parser countermeasure and asked for a
hand-written table-driven tokenizer. V2 implements that as a streaming lexer:
a 256-byte input-class table and a 10-state number DFA produce transient
kind/start/after spans directly into one recursive JSON parser. It does not
materialize a token list. The parser has full-index, legacy-query and
recoverable-boundary result modes.

- production `lib/rxfnsb/rexx/rxjson.crexx` SHA-256:
  `eac3c5cfaa0a05ec32f8300c12b93fb8afe67c9c72b19653cef4ecceceb419d6`;
- unchanged decisive benchmark SHA-256:
  `f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b`;
- focused correctness: 11/11 passed, zero failed or skipped, covering legacy
  selectors, indexed-document traversal, noisy recovery, numeric DFA
  acceptance/rejection states, duplicate paths, optimized/non-optimized
  compilation and both VMs;
- focused log: `/tmp/cri09-v2-focused-final.log`, SHA-256
  `246b684b1bc06ac054f44fd1dfe2e533a54b38ac7dc165d389bba50eb89f070d`;
- `git diff --check` passed at freeze and again at this stop.

No production or benchmark source changed after the freeze. Broad Debug CTest,
sanitizer, install/package and documentation closeout have not run.

## Ordinary Release product

The clean dedicated product is
`/tmp/crexx-cri09-release-v2.tdjuAF/build`, configured with Ninja, `Release`,
`CREXX_VM_PROFILING=OFF`, `ENABLE_PARSER_MODE=OFF` and `BUILD_TESTING=ON`.
AppleClang is `21.0.0.21000101`; cREXX identifies as
`crexx-1.0.0-beta.3+local.gd78c6fcfa81e.dirty` at source HEAD
`d78c6fcfa81ef03fdbea65ff9cc39ed99e8716bf`.

| Artifact | SHA-256 |
| --- | --- |
| `library.rxbin` | `5cabeaa0fbe6d3955015e2ac549c657b19d95d6632d201769e1a0f4b1f170ec2` |
| `rxvm` | `b1bd31897ac26a4378f52f9498999a7b0b1d32c91443ce908c36ecadf888cb9e` |
| `rxbvm` | `92bcf6d11bb9c393b7116ac04565f641ea708e7ed9578b4dde31936c77d5538a` |
| optimized linked benchmark | `8fda0296db3d9aaca3db9e743076607bffd9c79751d6fb94b0ce22707f1910fe` |
| non-optimized linked benchmark | `13226c5370c1c71860020874a45f3532522e7e01a3827f375d04c9b41a01d8a1` |
| optimized RXAS | `84d5d370841b22003ebfb89760dd536808fc38677602782edb43a46c10760b93` |
| non-optimized RXAS | `a1edfed556e2e36def9c92b32755afe6ecfa9e63fa54417e8bf2647ee00b44fc` |

All three products use byte-identical VM executables. The comparison therefore
measures different Level-B JSON images, not different VM builds.

## Exact comparison method and raw evidence

The maintained benchmark retains 60 rows, 30 operations, a 4,394-byte payload
and exact result checks. After three warmups per variant and VM, the pre-edit,
V1 and V2 optimized products ran 12 balanced triplets. Each variant occupied
each order slot four times. A separate V2 optimized/non-optimized comparison
used three warmups and 12 alternating pairs per VM. No failed execution or
outlier was removed.

Commands used the ordinary shape:

```text
<product>/build/bin/{rxvm|rxbvm} <product>/rxjson_parser_compare_*.rxbin
```

Retained files under `/tmp/crexx-cri09-release-v2.tdjuAF/`:

| Evidence | SHA-256 |
| --- | --- |
| `triplet-raw.log` | `49ec79e2dc43e320f022e4efb4836dc4e03d0734f8b4cb3bc16aab510727b3d1` |
| `triplet-medians.txt` | `09bf1eaa1eec643b883e89a92572396ed31fa0d5c0489fd8858ca576b5b5c45a` |
| `v2-opt-noopt-raw.log` | `d2dd32a9c853ee7a73590eaf4fc5eaa52dab8441a834a31d60d162b9ae50520f` |
| `v2-opt-noopt-medians.txt` | `3f50f26e2da0c9a5663765a5ad46a04d5f9c42dcb842d39a106d995cf9a11f3b` |
| `v2-verdict-calculations.txt` | `b1d1edd340a85f17694c8044d0a1e94b263a75afc0d46b8897221082a01a425a` |
| `v2-optimizer-calculations.txt` | `6ed0f64b2027345f5b3afc9fd2178105747761048cd5dec658ff8af20676ad22` |
| `static-instruction-summary.txt` | `5f2d71409b11bcd2f896da4b3fe93e3353eec8bf304eabc028911d0037529507` |

## Verdict

The compatibility rule remains at most 25% slower than the paired pre-edit
median. Positive changes below are slower.

| VM | Operation | Pre-edit | V1 | V2 | V2 vs pre-edit | Guard |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| `rxvm` | valid | 3,933.0 us | 6,168.5 us | 5,285.5 us | +34.39% | fail |
| `rxvm` | deep get | 4,147.0 us | 6,051.5 us | 4,950.5 us | +19.38% | pass |
| `rxvm` | tail get | 4,018.5 us | 6,214.5 us | 5,119.0 us | +27.39% | fail |
| `rxvm` | count | 7,375.5 us | 5,842.5 us | 4,944.5 us | -32.96% | pass/improves |
| `rxvm` | members | 3,846.5 us | 5,701.0 us | 4,904.0 us | +27.49% | fail |
| `rxbvm` | valid | 4,768.0 us | 7,327.5 us | 6,686.0 us | +40.23% | fail |
| `rxbvm` | deep get | 5,165.5 us | 7,406.5 us | 6,556.0 us | +26.92% | fail |
| `rxbvm` | tail get | 5,053.5 us | 7,689.0 us | 6,618.0 us | +30.96% | fail |
| `rxbvm` | count | 9,138.0 us | 7,102.5 us | 6,488.0 us | -29.00% | pass/improves |
| `rxbvm` | members | 5,004.5 us | 7,138.0 us | 6,480.5 us | +29.49% | fail |

The result-specific sink is effective: every V2 legacy cell improves over V1
by 8.65--18.19%. It does not recover enough parser overhead: seven of ten
predeclared compatibility cells still fail.

The primary parse-once criterion passes. Construction plus 30 indexed path
gets is 982.0 us (`rxvm`) and 1,079.5 us (`rxbvm`), 24.44%/21.36% of the
matched legacy repeated-parse time: a 75.56%/78.64% reduction. Construction
plus 30 resolved-node gets reduces time by 93.49%/93.61%.

No material optimizer-induced inversion is present. Across both VMs and all
nine measured cells, optimized versus non-optimized differences range from
-2.39% to +2.94% and have no consistent adverse shape.

The noisy scanner improves from 514,298.5 to 413.0 us on `rxvm` and from
519,177.5 to 523.0 us on `rxbvm`: 99.92%/99.90% reductions and
1,245x/993x speedups. Invalid `{noise` openers now fail against the original
byte buffer without candidate copies or ephemeral indexes.

There is one contract variance to decide explicitly. The successful scanner
candidate is validated once in boundary mode and then parsed again by the
public `.jsondocument` constructor to build its index. It creates only one
owned slice and one index, and its measured total remains below 0.6 ms, but it
is two grammar passes over the successful candidate rather than the approved
one-pass internal aspiration. B1 below is the explicit option that accepts
this measured construction: one allocation-free boundary-validation pass plus
one indexing pass for the successful slice.

The V2 verdict is therefore **mixed and rejected**: correctness, parse-once
retained access, scanner wall time and optimizer shape pass; the one-shot guard
and exact one-pass scanner-internal target do not.

## Cause

The V1 allocation cause is removed: legacy V2 calls no longer construct the
20,875-byte/494-node index for the 4,394-byte payload. The residual is parser
execution overhead, not copying, VM identity or optimizer inversion.

1. V2 carries full-index, query, path, names and diagnostics state through the
   same recursive calls. `_json_parse_value` has 20 arguments/107 locals,
   `_json_parse_object` 100 locals and `_json_parse_array` 71 locals. V1's
   corresponding local counts are 74/82/51.
2. Mode and result state are held in a dynamic `.int[]`; hot node/container
   paths repeatedly index it and test sink mode. V2 static container bodies
   expand from V1's 227/159 instructions to 444/284. Static counts include
   compiler inlining and are diagnostic rather than dynamic profiles, but the
   matched V1/V2 shape and wall clock agree.
3. Full-index construction, which does not benefit from query routing, is
   21.09% (`rxvm`) and 23.33% (`rxbvm`) slower in V2 than V1. This isolates the
   shared hot-core/table-routing cost from the old ephemeral-index problem.
4. The byte-class dispatch is useful at token boundaries and makes scanner
   recovery cheap, but the full number DFA adds two indirect table reads per
   input byte after the source read. On this register VM that is not assumed
   cheaper than a tight specialized digit loop. The current evidence does not
   attribute the 21--23% construction increase between DFA lookup and sink
   state without a bounded mechanism comparison.
5. Query key matching decodes candidate keys and creates temporary binaries.
   The pre-edit parser retained binary path segments and compared unescaped
   keys directly in source, decoding only escaped keys. Restoring that
   zero-copy fast path does not change JSON or path semantics.

## Decision A — legacy/parser countermeasure

### A1. Accept frozen V2 and override the guard

This ships the cleanest current implementation and excellent retained/scanner
performance, but accepts seven legacy regressions of 27--40%. No ABI or public
surface changes. **Not recommended.**

### A2. Lean hybrid streaming core (recommended)

Keep the hand-written table-driven tokenizer and one recursive grammar, but
make table dispatch occur at token boundaries and use specialized tight loops
inside strings/numbers where indirect DFA lookup loses. Replace dynamic sink
state indexing with scalar hot fields, restore binary path segments and
zero-copy unescaped-key comparison, and avoid constructing unused path/sink
containers for validation. Preserve every public Option B method, error,
ownership rule and scanner contract.

This is the smallest countermeasure consistent with the evidence. It targets
the measured 21--23% construction overhead and the known key allocations while
retaining one grammar. Compatibility and ABI are unchanged; maintenance adds
two deliberately small lexer fast loops backed by the same byte classes and
tests. The existing acceptance rule and another mandatory Release stop remain.

### A3. Iterative table-driven pushdown parser

Replace recursion and high-arity frames with one explicit compact stack and
sink context. This can remove more frame/state overhead and is a plausible
long-term engine, but it is a larger architecture rewrite with more nesting,
error-position and stack-growth risk. It should be considered only if A2's
bounded PoC cannot reach the existing guard. **Not recommended as the next
production edit.**

### A4. Revert Option B

Restore the pre-edit parser and remove the approved public document surface.
Legacy performance returns, but parse-once and production fuzzy extraction are
lost. **Not recommended.**

Smallest exact decision A:

> Approve or reject A2: retain one recursive JSON grammar and the hand-written
> table-driven streaming tokenizer, but authorize a lean hybrid hot path with
> token-boundary table dispatch, specialized string/number inner loops, scalar
> sink state, binary path segments and zero-copy unescaped-key matching. Keep
> the public Option B surface and the existing Release guard unchanged.

## Decision B — successful scanner construction

### B1. Accept boundary validation plus one index construction (recommended)

Accept the current scanner construction: one allocation-free
boundary-validation pass plus one indexing pass for the successful slice. The
first pass validates the candidate in the original buffer; the second owns the
exact slice and builds its sole index. Invalid openers allocate no documents.
Total optimized median is 0.413/0.523 ms. This keeps the public surface exact
and avoids hidden transfer state or a new factory. B1 requires documentation
and closure evidence, not another implementation phase.

### B2. Add an internal seed/adoption factory

Let boundary/index mode build the owned source and index once, then transfer
them through a non-exported seed type into `.jsondocument`. This removes the
second grammar pass but adds a named factory/adoption ownership path to class
metadata and needs proof that the non-exported seed is inaccessible to external
consumers. It is more surface and lifetime complexity for roughly 0.25--0.30 ms
on this workload. **Not recommended.**

Smallest exact decision B:

> Approve or reject B1: accept one allocation-free boundary-validation pass
> plus one indexing pass for the successful slice, with no document/index
> construction for invalid openers, because the measured total is below
> 0.6 ms and it preserves the exact public surface.

## Paste-ready continuation prompt

```text
Resume /Users/adrian/CLionProjects/CREXX at the CRI-09 V2 mandatory Release
stop. Read performance/CREXX-RAG-INTEGRATION-WORKLIST.md and
performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI09-V2-RELEASE-VERDICT.md.

Decision A: I approve CRI-09 A2, the lean hybrid streaming core. Keep one
recursive grammar and the hand-written table-driven tokenizer, using tables at
token boundaries and tight specialized string/number inner loops. Use scalar
hot sink state, binary path segments and zero-copy unescaped-key comparison.
Do not change the public Option B API, diagnostics, ownership, ABI or formats.
Retain the existing performance guard and stop again at the next ordinary
Release verdict.

Decision B: I approve CRI-09 B1. Accept one allocation-free
boundary-validation pass plus one indexing pass for the successful scanner
slice; invalid
openers must construct no document or index. Document the deliberate two-pass
successful-candidate behavior and retain the measured sub-0.6 ms guard.

Do not run broad closeout, advance CRI-10, or touch crexx-rag before the next
mandatory verdict is accepted.
```

# CRI-09 Option B first ordinary Release verdict

Status: **mixed and rejected by the predeclared guard; Option 1 countermeasure approved 2026-07-30**

Date: 2026-07-30

Approval record: Adrian approved recommended Option 1 and requested the V2
performance result. The public Option B API remains fixed. CRI-09 is unfrozen
only for the private shared-parser/result-sink countermeasure and must stop
again at its next ordinary Release verdict.

## V2 correctness freeze

The approved countermeasure now uses a hand-written streaming lexer with a
256-byte input-class table and a 10-state number DFA. Tokens are consumed as
kind/start/after spans by the recursive parser and are never materialized as a
token list. The same parser drives the full-index, allocation-free legacy-query
and fail-fast boundary modes.

- V2 `rxjson.crexx` SHA-256:
  `eac3c5cfaa0a05ec32f8300c12b93fb8afe67c9c72b19653cef4ecceceb419d6`.
- Decisive benchmark remains unchanged at SHA-256
  `f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b`.
- Expanded document contract SHA-256:
  `aac116c42b6e19cc9d92268abd160c66044a4c10ee76968bee21ad251dadb21b`.
- Focused correctness: 11/11 passed, zero failed or skipped, including legacy
  selectors, indexed traversal, noisy recovery, optimized/non-optimized
  compilation and both VMs. The expanded lexer checks every accepting number
  state, incomplete transitions, leading-zero/trailing-token failures and
  first-duplicate path semantics.
- Raw focused log: `/tmp/cri09-v2-focused-final.log`, SHA-256
  `246b684b1bc06ac054f44fd1dfe2e533a54b38ac7dc165d389bba50eb89f070d`.
- `git diff --check` passed at freeze.

Production and benchmark sources are frozen at these hashes for the V2
ordinary profiling-off Release comparison. Broad Debug, sanitizer,
install/package and documentation closeout remain prohibited until Adrian
accepts that verdict.

## Frozen candidate and correctness prerequisite

- Production `rxjson.crexx` SHA-256:
  `28d851dd4dc72b175b2381039de3d283d2203ed6fd6f79187fc8930579155165`.
- Decisive benchmark SHA-256:
  `f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b`.
- Focused correctness: 11/11 passed, zero failed or skipped. This covers the
  legacy API, the complete indexed-document contract, optimized and
  non-optimized compilation, and `rxvm`/`rxbvm`.
- Focused log SHA-256:
  `76ac73fb81fa60eee8247dac8841792dc19a9ca677252ee460d83bf59b451709`.
- No broad Debug CTest, sanitizer, install/package, or documentation closeout
  ran before this verdict.

The dedicated candidate is
`/tmp/crexx-cri09-release-candidate.EY7AYo/build`, configured with Ninja,
`Release`, `CREXX_VM_PROFILING=OFF`, `ENABLE_PARSER_MODE=OFF`, and
`BUILD_TESTING=ON`. The comparison uses the separately retained pre-edit
product in `/tmp/crexx-cri09-release-baseline.rxGv7t`.

Candidate product hashes:

| Artifact | SHA-256 |
| --- | --- |
| optimized linked benchmark | `318114d86a07e249088dd30cb34d2dbfb9e12faedc8a56ab8cb8ed2d9fd61c02` |
| non-optimized linked benchmark | `1568d9e66ba30faaca7fbe5ed0cea651ae66abf9f1073bcf94bc38c7e8d835d4` |
| `library.rxbin` | `dc70dcb3de1c65bbf491be361003c847b98c81b930786dd19a801e7e59cdc1d3` |
| `rxvm` | `b1bd31897ac26a4378f52f9498999a7b0b1d32c91443ce908c36ecadf888cb9e` |
| `rxbvm` | `92bcf6d11bb9c393b7116ac04565f641ea708e7ed9578b4dde31936c77d5538a` |
| optimized RXAS | `9cb31e0e79e3d177e4087722c88ad0c8404aaafde3a15fe4d94fe1bbb1df7390` |
| non-optimized RXAS | `a1edfed556e2e36def9c92b32755afe6ecfa9e63fa54417e8bf2647ee00b44fc` |

The VM hashes are byte-identical to the baseline product. The measured change
is in the Level-B library/benchmark image, not a different VM build.

## Exact Release method

The frozen baseline and candidate each use the maintained defaults: 60 rows,
30 operations, a 4,394-byte JSON payload, and exact result checks. After three
unrecorded warmups, each VM ran 12 alternating-order baseline/candidate pairs.
Candidate optimized/non-optimized inversion checks used three warmups and 12
recorded serial samples per VM.

Primary raw files:

- paired optimized samples SHA-256
  `f74a8fad26a6ad8fe76b0fe6965ab397a8e61cc2b41111c2e83826d90253c6d4`;
- paired medians SHA-256
  `ef2e46877f9140bf3d3490317d96057933beea11c977ddbce5ad0cd2e329464e`;
- non-optimized samples SHA-256
  `874c67a42776cd3eff4357cd926ebf90cd0fe1c899d53c519a015911e8136a46`;
- non-optimized medians SHA-256
  `24bc19d8bdbdc5987059ddb5cd6b3314333c8e8b3294ae5cf9fbf42e66532b73`.

An earlier `/tmp/.../paired-raw.log` is excluded: zsh deliberately does not
word-split a scalar order list, so it ran only candidate images with ambiguous
labels. `paired-raw-correct.log` is the retained valid comparison.

## Verdict against the predeclared rule

Existing compatibility selectors were required to remain within 25% of the
pre-edit median. Positive changes below are slower.

| VM | Operation | Baseline median | Candidate median | Change | Guard |
| --- | --- | ---: | ---: | ---: | --- |
| `rxvm` | valid | 5,253.0 us | 7,334.0 us | +39.62% | fail |
| `rxvm` | deep get | 5,397.0 us | 7,425.5 us | +37.59% | fail |
| `rxvm` | tail get | 5,329.5 us | 7,634.0 us | +43.24% | fail |
| `rxvm` | count | 9,851.5 us | 6,839.0 us | -30.58% | pass/improves |
| `rxvm` | members | 5,046.0 us | 6,911.0 us | +36.96% | fail |
| `rxbvm` | valid | 6,265.5 us | 9,530.5 us | +52.11% | fail |
| `rxbvm` | deep get | 6,712.0 us | 9,589.0 us | +42.86% | fail |
| `rxbvm` | tail get | 6,660.0 us | 10,215.5 us | +53.39% | fail |
| `rxbvm` | count | 12,233.5 us | 9,402.5 us | -23.14% | pass/improves |
| `rxbvm` | members | 6,570.0 us | 9,596.0 us | +46.06% | fail |

The parse-once criterion passes decisively. One document construction plus 30
indexed path gets is 1,111.5 us on `rxvm` and 1,354.0 us on `rxbvm`, only
20.86% and 20.33% of the matched legacy repeated-parse medians. That is a
79.14%/79.67% reduction. Resolving the node once and performing 30
`node_get` calls reduces the corresponding totals by 95.10%/94.55%.

There is no material optimizer-induced inversion. Optimized document path
access is 13.20% faster than non-optimized on `rxvm` and 0.15% faster on
`rxbvm`; optimized scanner time is 6.05% and 0.76% faster. The full operation
set has the same shape; a sub-percent cell is treated as timing noise, not an
inversion.

The adversarial 4,779-character scanner workload returns the correct retained
document, but optimized medians are 639,531 us (`rxvm`) and 688,135.5 us
(`rxbvm`). This is not acceptable as the production fuzzy-container path even
though it eliminates the old 4,161-endpoint substring search.

Therefore the first Release verdict is **mixed and rejected**: indexed use
meets the primary parse-once target with large margin and correctness is green,
but eight of ten compatibility guard cells fail and noisy recovery retains a
large avoidable restart cost.

## Cause and copy/memory evidence

The cause is architectural and reproduced, not an optimizer anomaly.

1. Every legacy selector now constructs a complete ephemeral document. The old
   selector parser validated/traversed without retaining every node or decoded
   key. On the exact 4,394-byte payload the candidate retains 494 nodes and a
   20,875-byte private index: 19,760 node bytes plus 1,115 decoded-key bytes.
   That is 4.75 index bytes per source byte before object/binary overhead. The
   extra allocation, writes, key copies, and class calls explain the one-shot
   regressions. `count` improves because the retained child count replaces the
   old second container traversal.
2. `jsonscancontainer` restarts its structural scan at every unmatched opener.
   The retained adversarial shape has 64 invalid `{noise` openers before the
   valid document. The candidate copies/scans about 296,075 candidate
   characters and constructs 64 immediately invalid documents before the one
   valid document. That is bounded far below the old 1,110,760-character,
   4,161-parse workaround, but it is not the approved production intent.
3. Optimized and non-optimized results move in the same direction on both VMs;
   the VM executables are byte-identical across products. Compiler optimization
   is not the cause.

The exact index probe and output remain under the candidate directory. Its
result SHA-256 is
`c74c43a9f0a613804f28b670327fef12d4adf602b1d73f72324e432d31313da4`.

## Countermeasure alternatives

### Option 1 — one parser core with result-specific sinks (recommended)

Keep the approved public Option B surface and its ownership/error contracts,
but relax the packet's internal requirement that every compatibility selector
construct a full ephemeral document. Refactor one strict grammar/parser core
to drive three private sinks:

- a complete index sink for public `.jsondocument` construction;
- a non-owning query/validation sink for legacy one-shot selectors; and
- a recoverable consumed-boundary/index sink for `jsonscancontainer`, failing
  early at invalid openers without rescanning their full balanced suffix.

This retains one JSON grammar, scanner, Unicode validation path, and error
model; it does not restore two independent parsers. It should remove the
20,875-byte ephemeral index from one-shot calls and the 296,075-character
scanner restart. Public source compatibility, ownership, ABI, serialized
formats, and RXAS/RXBIN remain unchanged. Maintenance cost is a private parser
event/sink abstraction and a larger focused state-machine matrix.

### Option 2 — retain the frozen internal contract and accept the regression

Accept the eight one-shot guard failures as the cost of always constructing an
ephemeral document, then optimize only scanner recovery within that contract.
This is the smallest architecture change, but it overrides the predeclared
guard and makes existing `jsonvalid`/`jsonget`/`jsonmembers` workloads
37--53% slower. It is not recommended.

### Option 3 — compact/tune the full index before changing routing

Keep every compatibility call on a full ephemeral document, but store
unescaped key spans in the source, decode only escaped keys, compact private
nodes, and reduce allocation/class overhead. The private layout can change
without ABI impact. This may reduce memory and some time, but it must recover
roughly 9--19% of candidate time merely to reach the 25% guard and does not by
itself eliminate scanner restarts. It is a reasonable secondary optimization,
not the recommended first countermeasure.

### Option 4 — revert Option B and retain the original API

Restore the pre-edit parser and close CRI-09 without the indexed public surface.
This preserves legacy performance but knowingly retains the reproduced
copy/reparse problem and does not implement Adrian's approved production JSON
surface.

Duplicating the old parser beside the new indexer is rejected: it would be the
same two-parser maintenance problem Option B explicitly avoided.

## Smallest exact decision

Approve or reject only this internal countermeasure:

> Keep the approved public CRI-09 Option B API exactly unchanged, but authorize
> one shared strict parser core with private full-index, legacy-query, and
> recoverable-boundary sinks instead of requiring legacy selectors to allocate
> a complete ephemeral document. Unfreeze CRI-09, implement that countermeasure,
> rerun focused correctness, and repeat the mandatory first Release verdict.

## Paste-ready continuation prompt

```text
Resume /Users/adrian/CLionProjects/CREXX at the CRI-09 first Release stop using
performance/CREXX-RAG-INTEGRATION-WORKLIST.md and
performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI09-FIRST-RELEASE-VERDICT.md.

I approve CRI-09 countermeasure Option 1. Keep the approved public Option B API,
ownership, diagnostics, and compatibility behavior unchanged. Relax only the
internal ephemeral-document routing: refactor one strict parser/Unicode/error
core with private full-index, legacy query/validation, and recoverable
container-boundary sinks. Do not create a second grammar/parser, syntax, ABI,
serialized format, schema/repair policy, or packed numeric contract. Preserve
the frozen V1 and pre-edit products as comparators. Run focused opt/no-opt and
dual-VM correctness, freeze V2, repeat the smallest ordinary Release verdict,
report it, and stop again before broad validation. Do not touch crexx-rag.
```

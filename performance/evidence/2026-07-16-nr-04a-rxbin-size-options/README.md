# NR-04A RXBIN 007 physical-layout option PoCs

Status: **historical scratch comparison complete; complete-section compression
selected and implemented**. The disposable harness described below was removed
after selection. Production evidence is in
`../2026-07-16-nr-04a-rxbin-007-compression/`.

This bundle compares three physical-layout options without changing the
production RXBIN reader/writer or VM:

1. restore the former 006 4-KiB-window LZSS codec independently to canonical,
   graph, canonical-plus-graph, or all 007 sections;
2. replace the fixed-width graph facts/indexes with a compact varint seed, with
   either all callables or only callables reachable from dispatch/provider
   facts; and
3. replace the serialized general graph with a minimal link-resolved runtime
   seed that materializes the selected descriptor/dispatch/factory view
   directly.

The harness is `rxbin_size_bench`, an `EXCLUDE_FROM_ALL` scratch target under
`binutils/tools/`. It reads real RXAS/RXLINK 007 containers through production
`rxbin`, round-trip checks every compressed section, decodes both seed shapes,
and compares their type, member, parameter, relationship, declaration,
assignability, dispatch, factory/provider and procedure-reference semantics
against the production graph.

## Provenance

- branch/commit: `develop` at `db94bc9caebf3676131643b88514aaa22d6fc1db`
- worktree: dirty with the already-recorded NR-04A production candidate and
  these disposable PoCs
- host: Darwin 25.5.0 arm64, Apple M5
- toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2
- build: existing ordinary profiling-off `cmake-build-release`
- measurement: seven serial samples for decode/unpack and each 1,000,000-call
  hot micro-loop; medians are retained in `size-bench.txt`

Fresh fixtures were generated through the actual tools:

```text
cmake-build-release/bin/rxas -o /tmp/crexx-nr04a-size-poc/runtime-interface-standalone.rxbin cmake-build-release/tests/performance/runtime_interface_lookup_compare_opt.rxas
cmake-build-release/bin/rxlink -o /tmp/crexx-nr04a-size-poc/runtime-interface-linked.rxbin /tmp/crexx-nr04a-size-poc/runtime-interface-standalone.rxbin cmake-build-release/bin/library.rxbin
cmake-build-release/bin/rxas -o /tmp/crexx-nr04a-size-poc/rexxcps-standalone.rxbin cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxas
cmake-build-release/bin/rxlink -o /tmp/crexx-nr04a-size-poc/rexxcps-linked.rxbin /tmp/crexx-nr04a-size-poc/rexxcps-standalone.rxbin cmake-build-release/bin/library.rxbin
cmake-build-release/bin/rxbin_size_bench /tmp/crexx-nr04a-size-poc/runtime-interface-standalone.rxbin /tmp/crexx-nr04a-size-poc/runtime-interface-linked.rxbin /tmp/crexx-nr04a-size-poc/rexxcps-standalone.rxbin /tmp/crexx-nr04a-size-poc/rexxcps-linked.rxbin
```

| fixture | bytes | SHA-256 |
| --- | ---: | --- |
| interface standalone | 88,516 | `fe4a5574ddeaedcf1a03d0f0dde1c30ce9f2e0da1036f1b3b5b26c13d46d4c06` |
| interface linked retained | 126,972 | `07b3393513db4efc808765eab09d8de2d58719b29530033fc2090dd532e28e7a` |
| RexxCPS standalone | 276,252 | `63b84fbe627655dc40d818ad2d538cad52fca58871dc8fade254eef64c1a5e8f` |
| RexxCPS linked retained | 869,908 | `85309d6a12592ffe4fcb776d075cf53246db56672578251523ee3a0cef62e4b8` |

The fresh images are 80-88 bytes larger than the earlier retained evidence
images and carry the current dirty-build provenance. Comparisons to 006 use the
fresh candidate sizes but the already-retained 006 controls; no 006 rebuild was
required.

## File-size result

The combined rows below use 006 LZSS on the four non-graph sections. The graph
column then selects the current fixed-width sections, compact dynamic seed, or
minimal resolved seed.

| linked retained fixture | current 007 | LZSS all current sections | + compact full | + compact dynamic | + resolved runtime | compact full + resolved | retained 006 control |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| interface | 126,972 | 37,458 | 31,880 | 31,624 | **29,384** | 33,340 | 28,360 |
| RexxCPS | 869,908 | 273,858 | 259,144 | 254,832 | **247,848** | 262,762 | 259,518 |

For the interface fixture, the full/dynamic compact candidates are 12.41% and
11.51% above 006 and the resolved candidate is 3.61% above. For RexxCPS they
are 0.14%, 1.81% and 4.50% below 006. Compression alone is decisive but cannot
remove the interface graph penalty.

`compact_full + resolved` is the measured upper bound for retaining the entire
general seed while giving the runtime its direct view. It is 17.56% above the
interface 006 control and 1.25% above RexxCPS 006 because the two unrefined
seeds duplicate runtime facts. A purpose-built tooling/policy residual should
be smaller, but was not invented or measured before the format decision.

Standalone results establish the seconds-scale RXAS iteration path:

| standalone fixture | current 007 | LZSS + compact full | LZSS + compact dynamic | LZSS + resolved runtime | compact full + resolved |
| --- | ---: | ---: | ---: | ---: | ---: |
| interface | 88,516 | 19,456 | 19,384 | **17,680** | 20,565 |
| RexxCPS | 276,252 | 80,960 | 79,680 | **79,632** | 81,153 |

The compact full form is the relevant standalone RXAS candidate because it
retains all callable symbols/descriptors needed by a later link. Compact
dynamic and resolved-only standalone rows are size/runtime-shape controls, not
proof that such an RXAS output remains linkable.

## Seed and cold-materialization result

| linked fixture | current graph facts + indexes | compact dynamic seed (packed) | compact decode + unpack | resolved seed (packed) | resolved decode + unpack |
| --- | ---: | ---: | ---: | ---: | ---: |
| interface | 26,027 | 13,627 (3,698) | 267 us | **1,872 (1,460)** | **5 us** |
| RexxCPS | 79,196 | 33,837 (10,607) | 3,349 us | **5,086 (3,618)** | **28 us** |

The compact decoder deliberately rebuilds a production `RxGraph` through the
public builder, so its cold cost includes string search and general index
construction. The resolved decoder directly materializes dense hot tables.
Its measured retained arrays, including direct relationship/declaration walks
and type/member name indexes, are 32,606 bytes for interface and 85,611 bytes
for RexxCPS, versus 51,123 and 150,820 bytes for the production general graph.

The resolved seed uses constant-pool references when an exact string is
already present and a small inline fallback otherwise. Interface uses 84 pool
references plus 18 inline strings; RexxCPS uses 69 pool references plus 131
inline strings. It retains:

- canonical type name/kind/flags;
- member name, return type, parameter type/flags and member flags;
- direct class/interface/type relationships and declarations;
- only dispatch/provider-reachable procedure references;
- precomputed assignability closure and sparse dispatch entries; and
- ordered factory/provider buckets.

It deliberately omits general callable symbol/descriptor indexes and
relationship origin/ordinal fields from the runtime seed. Provider and edge
order is retained. Whether those omitted policy/debug facts remain in a
separate compact link-time/tooling section is an architectural choice, not a
PoC conclusion.

## Hot lookup result

On the two interface fixtures, the decoded compact seed measured about 0.90 ns
for positive type support, 0.90 ns for descriptor dispatch, and 0.90 ns for
factory-bucket lookup. The resolved view measured 0.90 ns for all three. The
linked RexxCPS resolved view measured 0.92 ns support, 0.81 ns dispatch and
0.54 ns factory lookup.

The resolved view also materializes the fast walk/search scope that the runtime
and language runtimes need: direct relationship-bucket lookup measured
0.86-0.90 ns versus 2.82-3.64 ns for the general graph, and indexed positive
type-name lookup measured 15-26 ns on the linked/standalone semantic fixtures
versus 51-73 ns for the production graph. Member-name hash buckets and direct
declaration ranges are materialized and semantically checked as well.

These invariant micro-loops use the same form as the existing isolated graph
harness and prove control-cost table shape. They are not integrated opcode or
end-to-end benchmark results; the sub-loop variation reflects compiler/code
layout and should not be interpreted as a meaningful winner below one
nanosecond. Both seed candidates preserve the selected hot representation.

## Decision boundary

No production format or runtime code was changed by these PoCs. The evidence
supports a production design using canonical section compression plus either:

- the compact full seed, if one general re-linkable graph must serve RXAS,
  RXLINK, tooling and runtime and its cold reconstruction cost is acceptable;
- the resolved seed as the terminal RXLINK runtime image, with RXAS using the
  compact full seed, if minimum runtime load cost and file size are primary; or
- a resolved runtime seed plus a purpose-built compact tooling/policy residual
  if linked images must retain facts omitted by the runtime seed. The measured
  full-seed-plus-resolved result is a duplication-heavy upper bound for this
  split, not the optimized residual design.

Adrian subsequently selected the closest existing representation: retain the
complete general six-section graph in equivalent RXAS/RXLINK images and add
transparent compression to every section. Production sizes match the
all-current-sections PoC rows exactly. The alternate seed and size harness
sources were then removed; their retained evidence remains the record for any
future optional graph-seed refinement.

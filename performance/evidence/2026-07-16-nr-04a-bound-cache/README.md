# NR-04A bound callable, factory and site-cache gate

Status: positive focused Release gate; stopped with serialized RXBIN size still
open

This bundle measures the directed continuation of the C3-style/R1 repair. The
portable RXBIN 007 graph remains process-independent, while the VM now resolves
every graph callable to `proc_runtime *` once per semantic-generation rebuild.
It also materializes bound factory/provider rows, uses a two-way method
instruction-site cache, and caches the factory bucket or direct target at each
factory-selection site.

## Provenance and measurement contract

- Source base: `db94bc9caebf3676131643b88514aaa22d6fc1db` on `develop`, with
  the uncommitted NR-04A implementation, benchmark-correctness fixes and
  evidence/documentation changes in the working tree.
- Exact pre-007 comparator: `7a599906b353abbbea5ed9601adf3acd681f2615`
  (`Complete NR-05 call-path census`). No 006 timing was rerun.
- Host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs.
- Build: Ninja, `/usr/bin/cc`, `CMAKE_BUILD_TYPE=Release`,
  `CMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF`.
- Final ordinary Release hashes:
  - `rxvm`: `5db77b21a887c952292ab35d51322b5b59cdb07b3859ecc1e2821dd1411e7bf2`
  - `rxbvm`: `6ab3a08cb79efade6c8ed4d19f7d9951faf18f25fa40ec00409d8af6bb1937e6`
  - `rxgraph_bench`: `4110640ef777cacd1362f12ce1630217af744ba3e1710c375676660860f5d9a0`
- Final image hashes:
  - interface retained: `42c70330b91644b3635e998db346c230d063330b66da13d2a8d269320f398321`
  - interface stripped: `f69a9cf3b9b18022ca97524602f22b3a6e78ba2d4d50da2bf64b3c9d635678ca`
  - RexxCPS retained: `d95cf5381b352aeac24b2857efc76efc2e1517fa26e81a1e3cbde5755a908ce0`
- Interface cells used two warmups and seven recorded runs, serially. The
  authoritative final cells have `-clean-` in their directory names. One
  capture overlapped a Debug fixture rebuild and was discarded rather than
  retained as evidence.
- Instrumented profiles are attribution only. Ordinary profiling-off Release
  process and benchmark-native times are the performance verdict.

The exact commands, arguments, timestamps and raw stdout/stderr are retained
under each cell. The core interface command was:

```text
cmake-build-release/bin/{rxvm,rxbvm} images/runtime-interface-{retained,stripped}.rxbin -a 1000000 100000
```

## Focused Release result

Lower is better. The factory region includes selection, factory invocation,
object construction and the subsequent method; it is not an isolated lookup.

| VM / image | Process median | Method median | Factory-region median |
| --- | ---: | ---: | ---: |
| `rxvm`, retained | 48.965 ms | 37,044 us | 7,905 us |
| `rxvm`, stripped | 50.106 ms | 38,003 us | 8,059 us |
| `rxbvm`, retained | 48.560 ms | 36,727 us | 7,895 us |
| `rxbvm`, stripped | 48.004 ms | 36,438 us | 7,917 us |

The retained exact-006 `rxvm` comparator was 204.432 ms process, 45,591 us
method and 152,985 us factory-region. The stripped comparator was 170.090 ms,
44,488 us and 121,760 us. Therefore the final `rxvm` result is:

| Image | Process change | Method change | Factory-region change |
| --- | ---: | ---: | ---: |
| retained | -76.05% | -18.75% | -94.83% |
| stripped | -70.54% | -14.58% | -93.38% |

There is no retained 006 `rxbvm` comparator, so only its current absolute
result is reported.

The final profiling-off canonical RexxCPS smoke used the rebuilt image with no
arguments and reported 1,132,602 CPS with the required effective `100 x 100`
contract. It is retained in `rexxcps-final-smoke.txt`. The earlier serial
three-sample candidate median was 1,127,421 CPS; both are above the retained
857,561-CPS comparator and show no unrelated-hot-path regression.

## Isolated structural result and the factory defect

`isolated-rxgraph.txt` is a seven-sample, 1,000,000-iteration run of the
production `rxbin` library over the rebuilt retained interface image. Relevant
medians are 0.44 ns for descriptor support, 0.66 ns for descriptor dispatch,
1.06 ns for a factory bucket, and 1.52 ns for its first provider. The harness
also reports `sizeof(value)=248`, `sizeof(RxGraphTypeRef)=56`, and 51,123 bytes
in 26 graph allocations for this image. `nm` finds no out-of-line
type-reference, bound-target, cache or per-query procedure-binding helper in
the final VMs.

The first version of this harness chose a known-valid factory directly from
the graph. It proved that bucket/provider access was cheap, but did not inspect
the factory IDs embedded in executable instructions. The enhanced harness now
audits every graph-bearing operand. Running it over the earlier defective image
found:

```text
type_sites=26,member_sites=2,factory_sites=2,providerless_factory_sites=2,invalid_sites=0
```

The rebuilt image reports the same real sites with
`providerless_factory_sites=0`. The defect was a duplicate factory member:
the interface declaration used the canonical return type
`.runtime_interface_lookup_compare..lookup24`, while the call instruction used
the valid source-short spelling `.lookup24`. The builder treated those as
different factory signatures, and the linker encoded the instruction with an
orphan providerless bucket. Each VM selection then missed the graph path and
ran the legacy descriptor resolver. The graph builder now compares those type
spellings semantically and operand resolution prefers the provider-backed
factory. The graph shrank from 61 types / 43 members / 25 factories to
60 / 42 / 24, with the same 24 providers.

This explains why factory selection looked disproportionately slow: it was not
paying the approximately 1-ns bucket lookup measured by the original harness;
it was taking the compatibility fallback on every execution. The old 006
profile recorded `SRCFPROCSEL` at 448 ns per instruction. The final profile is
14 ns, the same order as `SRCMETHODSEL` (14 ns) and `SETOBJTYPE` (13 ns).

## 006-to-current runtime audit

The targeted source and generated-code audit found these runtime-relevant
differences from 006:

- RXBIN decoding moved from the native-layout 006 header implementation to the
  compiled portable 007 `rxbin` library. This affects load/startup, not an
  executed selector after binding.
- The semantic graph, numeric graph operands and process-local graph views are
  new. Only object stamping/type tests, `SRCMETHODSEL`, `SRCFPROCSEL`, `TYPEOF`
  and their diagnostic paths consume them during normal execution.
- `runtime_graph_procedure()` did not exist in 006. The 006 interface
  registries stored resolved `proc_runtime *` pointers; the rejected 007 path
  introduced a module/procedure scan on every query. The current implementation
  retains only a cold `runtime_graph_procedure_unbound()` pass while rebuilding
  dense callable bindings.
- The rejected 007 value layout was 272 bytes. The current one-pointer type
  descriptor layout is 248 bytes versus 256 bytes in 006, so generic value
  copies and register storage no longer carry a graph penalty.
- The coherent single-snapshot `MTIME` change is a benchmark correctness fix,
  not graph work.
- Canonical metadata ordering/comments and tool validation changes are not VM
  hot paths. The legacy interface registries remain available as a cold
  cross-graph/native compatibility path and still add some rebuild work, but
  they are not entered by the sealed-image measured selectors.

No other ordinary instruction-handler change explains a remaining regression.
The final unrelated RexxCPS smoke and the focused interface result agree with
that audit.

## Correctness and unresolved size gate

- Release graph unit coverage passed, including the source-short/canonical
  factory-signature regression.
- The focused Debug graph/interface/default/factory/match/no-provider/
  tie-break/covariant/late-load/RXVML sweep passed 82/82.
- Both ordinary Release VMs pass the retained and stripped workload images.
- The complete Debug CTest suite was green before this binding/remap slice; it
  was deliberately not rerun before the mandatory early Release verdict.

Runtime performance now passes the focused gate. Serialized size does not:

| Image | 006 | Current 007 | Ratio |
| --- | ---: | ---: | ---: |
| interface retained | 28,360 B | 126,884 B | 4.474x |
| interface stripped | 22,762 B | 105,572 B | 4.638x |

Removing the duplicate factory saved only 140 bytes. The dominant cause remains
the uncompressed canonical constants/metadata that 006 compressed, with the
approximately 26-KiB semantic graph as a separate material cost. Compression
and graph-scope/seed compaction are therefore the next design decision; the
positive runtime result does not close that size defect.

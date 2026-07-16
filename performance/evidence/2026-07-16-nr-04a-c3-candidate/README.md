# NR-04A C3-style runtime-view candidate

Status: isolated hot-path target met; first integrated Release gate retained;
stopped before VM procedure-binding changes

This is the first measured repair candidate after the rejected RXBIN 007
milestone-1 implementation. It keeps the frozen six-section 007 seed and the
existing RXAS/RXLINK outputs, but `rxbin` materializes process-local dense
assignability, dispatch, factory and provider views when a graph is built or
loaded. Object values now carry one immutable `RxGraphTypeRef *` rather than a
name, name length, graph pointer and graph ID.

The retained linked images are byte-for-byte the same images used by the first
007 gate. Therefore these results isolate the new library/runtime layout and VM
value change; no graph facts, graph indexes or benchmark program were rebuilt.
Portable callable-to-`proc_runtime *` binding is deliberately unchanged, so
the interface result exposes that remaining VM cost.

## Provenance

- Source base: `db94bc9caebf3676131643b88514aaa22d6fc1db` on `develop`, plus
  the uncommitted C3-style candidate and the already documented measurement
  fixes/evidence work.
- Host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs.
- Build: ordinary Release, `CREXX_VM_PROFILING=OFF`.
- Binaries:
  - `rxvm`: `e687089a13c998c21b02df1343876a7cb41e1af5c2fe968fcf13d794d7a05f32`
  - `rxbvm`: `5b146a01b132631dd6145e63e71f6a782366d083ed20a2844cb15e17d19eaa25`
  - `crexx`: `def591ca9d40dddf71b9673e5af8fc77d937e53f88a02b3f3ee2225b48a172fb`
  - `rxgraph_bench`: `0464686e34f8ce2726f15f66d055ae517dbc5a44942cdc3402d7cbe386cdbfdc`
- Interface cells used two warmups and seven serial recorded runs. Canonical
  RexxCPS used one warmup and three serial recorded runs. Every stdout, stderr,
  process elapsed time, effective argument contract and command is retained by
  `run_cross_runtime.crexx`.
- The isolated harness contains seven internal samples per metric and links
  production `rxbin` without a VM.

## Isolated result

All numbers are median nanoseconds per operation. The value-descriptor rows are
the path consumed by object values; ID rows are the lower-level graph API.

| Image | ID support + / - | Descriptor support + / - | ID dispatch | Descriptor dispatch | Factory bucket | First provider |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| interface | 0.696 / 0.696 | 1.067 / 0.940 | 0.708 | 0.928 | 0.948 | 1.514 |
| RexxCPS | 0.693 / 0.692 | 0.952 / 0.924 | 0.705 | 0.924 | 1.164 | 1.496 |

The rejected implementation measured 32-38 ns for positive transitive support
and 49-73 ns for negative support. The production C3-style view is now at the
scratch bitset/direct-load control cost. `sizeof(value)` is 248 bytes, down from
the rejected 272-byte form and below the pre-graph 256-byte form.

Materialization increases process-local graph storage while leaving the RXBIN
unchanged:

| Image | Rejected retained graph | C3-style retained graph | Serialized graph |
| --- | ---: | ---: | ---: |
| interface | 27,023 B | 50,703 B | 26,167 B |
| RexxCPS | 81,576 B | 150,048 B | 79,324 B |

The increase is primarily two dense `uint32_t[type_count][member_count]`
dispatch/factory views. It is explicit candidate runtime memory, not new file
size or value-register overhead.

## Integrated Release result

Lower is better for process/method/factory time. Higher is better for RexxCPS.

| `rxvm` cell | Immediate pre-007 | First 007 | C3-style candidate | Candidate vs pre-007 |
| --- | ---: | ---: | ---: | ---: |
| interface retained process | 204.432 ms | 371.401 ms | 215.361 ms | +5.3% |
| interface retained method | 45,591 us | 89,251 us | 50,117 us | +9.9% |
| interface retained factory | 152,985 us | 286,454 us | 161,751 us | +5.7% |
| interface stripped process | 170.090 ms | 300.816 ms | 182.025 ms | +7.0% |
| interface stripped method | 44,488 us | 80,300 us | 50,354 us | +13.2% |
| interface stripped factory | 121,760 us | 208,161 us | 127,353 us | +4.6% |
| canonical RexxCPS | 857,561 CPS | 737,623 CPS | 1,129,206 CPS | +31.7% |

The interface candidate recovers 39.5-42.0% of whole-process time relative to
the first 007 build, but is not yet at the exact pre-007 interface baseline.
The remaining method/factory delta is consistent with the deliberately
unchanged `runtime_graph_procedure()` scan that maps a portable callable to a
VM procedure for each selection.

The 007-only `rxbvm` candidate medians are 215.170 ms retained, 181.650 ms
stripped and 1,109,859 canonical RexxCPS. These improve 37.1%, 50.7% and 55.3%
respectively over the first 007 `rxbvm` gate; no 006 `rxbvm` comparator exists.

## Correctness gate

- Release `rxgraph_unit` passed, including process-local views rebuilt after
  deserialization.
- Debug builds of `rxgraph`, both VM cores and both RXVML libraries succeeded.
- The focused Debug graph/interface/late-load/RXVML sweep passed 81/81.
- Both ordinary Release VMs passed the retained interface image before timing.
- Every retained timed sample passed its workload correctness marker.

## Stop decision

The C3-style value and graph-query shape passes the isolated gate and removes
the generic value penalty. The integrated interface gate is now a small but
real regression while canonical RexxCPS is materially above the retained
baseline. No procedure binding, site cache, image compression or graph-scope
change is included here. The next representation choice is how to bind portable
callable IDs to VM procedure pointers once without putting VM pointers into the
portable `rxbin` codec.

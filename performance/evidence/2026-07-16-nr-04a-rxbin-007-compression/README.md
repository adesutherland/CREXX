# NR-04A RXBIN 007 section-compression gate

Status: **selected compression slice and final NR-04A closeout pass size,
correctness, re-link, isolated lookup, focused Release performance, cross-image
provider, and broad Debug gates**.

This slice keeps the complete six-section RXBIN 007 representation and semantic
graph unchanged. The shared `rxbin` writer applies the former production LZSS
algorithm independently to each completed section and retains the compressed
form only when it is smaller. The 007 reader validates the directory flag and
stored/expanded bounds, expands the section, and then follows the same checked
materialization path. RXAS and RXLINK therefore continue to emit complete,
executable, re-linkable `cReXx007` images.

## Provenance and contract

- Source base: `db94bc9caebf3676131643b88514aaa22d6fc1db` on `develop`, with
  the uncommitted NR-04A implementation, benchmark-correctness fixes,
  compression slice, and evidence/documentation changes in the worktree.
- Exact pre-007 comparator: `7a599906b353abbbea5ed9601adf3acd681f2615`.
  No 006 timing or image was rebuilt.
- Host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs.
- Build: Ninja, `/usr/bin/cc`, `CMAKE_BUILD_TYPE=Release`,
  `CMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF`.
- Interface cells: two warmups and seven recorded serial process runs.
- RexxCPS cells: one warmup and three recorded serial process runs, with no
  argument override and the canonical `100 x 100` provenance contract.
- Raw commands, argv, timestamps, stdout/stderr, and sample streams are under
  `cells/`. All retained samples passed their correctness markers.

Release binary hashes:

| binary | SHA-256 |
| --- | --- |
| `rxas` | `07422e0997cd59d33d75416561cf4089751348eae62a32e822067889a4461352` |
| `rxlink` | `bce9dca81eebde348de51b0c3933d79509cdc42f86a03b64afb70b14c42aa771` |
| `rxvm` | `30d3f3f735ec221110def55b8c601d4e65ad44612954cd0d6c2c6b65820d356e` |
| `rxbvm` | `6c50e20ea88c2a6932f7d0935240545d60b022f6c08efc4f3016a9061d1a238d` |

## Size and lifecycle result

| image | Uncompressed 007 | Compressed 007 | Reduction | 006 control | Compressed vs 006 |
| --- | ---: | ---: | ---: | ---: | ---: |
| interface standalone RXAS | 88,516 B | 23,945 B | 72.95% | n/a | n/a |
| interface linked retained | 126,972 B | 37,458 B | 70.50% | 28,360 B | +32.08% |
| interface linked stripped | 105,572 B | 31,322 B | 70.33% | 22,762 B | +37.61% |
| RexxCPS standalone RXAS | 276,252 B | 82,363 B | 70.19% | n/a | n/a |
| RexxCPS linked retained | 869,908 B | 273,858 B | 68.52% | 259,518 B | +5.53% |

The retained interface RXAS output was linked with the library, then the
resulting linked image was linked again as the sole input. Both linked outputs
are 37,458 bytes with identical SHA-256
`4b207249221d0eff4374a6ddccd48cdeec4fa18b6a77e87e2f6d7e2ffbe4c901`.
Both `rxvm` and `rxbvm` execute the original linked and re-linked images and
pass the workload marker. This proves that compression did not introduce an
object-only or terminal-image representation.

Compression removes the gross 4.5x RXBIN 007 size expansion. The remaining
interface delta to 006 is the material semantic graph and fixed 007 container
representation, not duplicated uncompressed canonical data. RexxCPS is only
5.53% above its retained 006 control.

## Focused Release performance result

Lower is better for process/method/factory time. The comparison is the
immediately preceding bound/cache gate over the same workload and host.

| VM / image | Process median | Change | Method median | Change | Factory median | Change |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm`, retained | 49.184 ms | +0.45% | 37,160 us | +0.31% | 7,959 us | +0.68% |
| `rxvm`, stripped | 48.873 ms | -2.46% | 36,826 us | -3.10% | 7,993 us | -0.82% |
| `rxbvm`, retained | 48.477 ms | -0.17% | 36,450 us | -0.75% | 7,916 us | +0.27% |
| `rxbvm`, stripped | 48.260 ms | +0.53% | 36,405 us | -0.09% | 7,984 us | +0.85% |

All movements are run noise rather than an executed-hot-path regression.
Canonical RexxCPS medians are:

| VM | Compressed 007 | Prior bound/cache gate | Change |
| --- | ---: | ---: | ---: |
| `rxvm` | 1,120,626 CPS | 1,127,421 CPS | -0.60% |
| `rxbvm` | 1,135,200 CPS | 1,106,982 CPS | +2.55% |

## Isolated load and lookup result

`isolated-rxgraph.txt` links the production `rxbin` library and loads the
compressed retained interface image seven times. Median complete image load
and graph materialization is 106 us versus 79 us in the preceding uncompressed
gate, a measured cold cost of about 27 us. The process benchmark is about
49 ms, so this cost is consistent with the unchanged process medians.

The materialized hot view is unchanged: descriptor support is 1.11 ns,
descriptor dispatch 0.90 ns, factory-bucket access 0.93 ns, and first-provider
access 1.51 ns. The operand audit reports 26 type, two member, and two factory
sites with zero invalid or providerless sites. `sizeof(value)` remains 248
bytes.

## Correctness and cleanup

- Focused Debug graph, compact-format, RXDAS round-trip, linked-format,
  interface, and both-VM tests pass 8/8.
- Format coverage accepts mixed raw/compressed sections and rejects unknown
  flags, malformed compressed data, compressed sections without size
  reduction, reserved fields, undersized images, graph corruption, and the
  existing wrong-kind metadata reference.
- The disposable size and alternate-seed PoC sources/target were removed after
  the production result matched their size prediction exactly.
- The isolated `rxgraph_bench` target remains because it is the focused
  production-library performance regression harness, not an alternate format
  implementation.
- Final closeout exposed a separate binding defect: a factory site could use
  only providers in its local graph and select a catch-all before a
  higher-scoring provider in another loaded image. Graph factory buckets now
  aggregate signature-compatible entries from the validated process-wide
  registry after linking. The minimal CMS reproducer, all six ADDRESS
  integration fixtures, and the LLM fixture under `rxbvm` pass.
- The final Debug rebuild succeeds and broad CTest passes 1,846/1,846.

The next optional size decision is compacting the graph seed. It is no longer
needed to correct the gross size defect and should be considered separately
against the remaining 32-38% small-interface size delta.

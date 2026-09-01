# CRI-13 R1 RXAS and VM instruction trace

Status: **diagnosis complete; frozen R1 remains rejected at the mandatory
performance stop**

Date: 2026-07-30

Governed IDs: `CAP-01-J02`, follow-on `PERF2-07-B02`

## Outcome first

`<at..u8>` is not intrinsically slower than the string-helper path. The
compiler lowers each access to one strict `bgetu8`, and the R1 classifier is
fully inlined into `jsondocument.node_f32_array`; there is no per-element
procedure call.

The dominant R1 regression is hidden whole-payload copying at the inlined
by-value `.binary` boundary. For each of 3,072 elements, RXAS copies the entire
retained JSON source into the classifier formal and copies it again while
inlining `binlength(data)`. With a 58,479-byte benchmark source, that is 6,144
full-source copies or 359,294,976 bytes per projection. This is the guarded
non-local-actual case left deliberately outside the accepted CRI-02 compiler
proof, not a failure of that direct-local fix.

Removing those copies in scratch makes the byte scanner substantially faster
than first B: the best bounded diagnostic falls from frozen R1's optimized
5.4295/5.863 ms to 1.311/1.755 ms on `rxvm`/`rxbvm`, versus first B's retained
2.918/3.4755 ms. The scanner still misses the predeclared 2x prototype ceiling
because one projection executes about 1.28 million interpreted RXAS
instructions, including 61,441 individually bounds-checked `bgetu8` reads.

The current RXAS flow optimizer is already the right foundation for a
systemic residual-copy proof. It has whole-procedure CFG, liveness,
available/may-reach copy facts, per-view register effects, async-target,
barrier, metadata and TRACE handling. It deliberately rejects every
non-identity generic `COPY_REG_REG` before applying those proofs with
`full-value-ownership-unproved`. A live debug assembly of the exact frozen R1
source records five such rejections in `node_f32_array`. The missing work is a
bounded full-value proof, not a new assembler optimizer.

## Exact lowering and handler route

The compiler's typed-binary storage map selects `bgetu8` for `.u8` reads in
`compiler/rxcp_util.c`. Frozen optimized RXAS SHA-256 is
`defc72ebf485646b80dd0ae32c0dc894e94a01328cff85b0988874ac33930546`.
The R1 call is inlined as:

```rxas
linkattr1 r39,a1,8
copy r31,r39
...
copy r40,r31
blen r41,r40
...
bgetu8 r44,r31,r38
```

The first `copy` preserves the by-value classifier formal. The second preserves
the by-value `binlength` formal. The loop's `<at..u8>` is the single `bgetu8`.

## Existing RXAS flow boundary

`assembler/rxas_flow.c` can propagate typed integer, float and string copies
only after proving all uses redirectable and the equality fact available on
every reaching path. It rejects unclassified effects, unknown successors,
read/write uses, live values at effect barriers, metadata observations and
TRACE-adjacent rewrites. Generic whole-value copies are currently stopped
earlier:

```text
NR27 reject procedure=§rxjson.jsondocument.node_f32_array \
  candidate=196:COPY_REG_REG node=196:COPY_REG_REG \
  reason=full-value-ownership-unproved expected-views=0x7f actual-views=0x7f
NR27 reject procedure=§rxjson.jsondocument.node_f32_array \
  candidate=290:COPY_REG_REG node=290:COPY_REG_REG \
  reason=full-value-ownership-unproved expected-views=0x7f actual-views=0x7f
NR27 reject procedure=§rxjson.jsondocument.node_f32_array \
  candidate=305:COPY_REG_REG node=305:COPY_REG_REG \
  reason=full-value-ownership-unproved expected-views=0x7f actual-views=0x7f
```

The hot pair has two different proof obligations:

- `copy r40,r31; blen r41,r40` is a register-local residual copy. Only the
  binary view of `r40` is subsequently observed. A bounded RXAS extension can
  project a generic copy onto its actually live views, prove the source view
  unchanged on all reaching paths, redirect the reads and remove the copy.
  It must also retarget the compatible `data` TRACE/register metadata from
  `r40` to `r31`; the current pass detects that observation and rejects rather
  than rewriting it. The existing per-view dataflow supplies nearly all of the
  value proof, while trace/source fidelity remains an explicit obligation.
- `linkattr1 r39,a1,8; copy r31,r39; unlink r39` is not safely handled by
  replacing `r31` uses with `r39`: `unlink` deliberately restores `r39`'s base
  mapping. Eliminating this copy requires either one compiler-proved invariant
  snapshot outside the loop, or an exact RXAS transformation that relocates
  the alias lifetime while preserving all normal and exceptional cleanup,
  TRACE and source-address observations. It is a separate, harder proof.

This distinction prevents an easy local-copy success from being treated as
proof that attribute-value isolation can be removed. It also gives RXAS a
useful role when `rxc` cannot recognize every post-inline library context.

`interpreter/rxvmintp.c` implements `BGETU8_REG_REG_REG` by calling
`rxvm_binary_range` and then `rxvm_binary_read_le`. Every read therefore
rechecks negative offset, host-size representability, logical length and field
width even though the classifier has already proved
`0 <= start < after <= binlength(data)`. `bcheckrange` is a standalone strict
check; current `bgetu8` has no proof-carrying or unchecked companion.

## Isolated dynamic proof

A dedicated Release profiling build used `CREXX_VM_PROFILING=ON`; the ordinary
Release verdict product remained unchanged. A scratch Level B program builds
one 3,072-number document and selects the projection with one argument. Counts
are the exact run-minus-control difference and are identical on both VMs.

| Variant | Dynamic RXAS instructions | `bgetu8` | generic `copy` | important mechanism |
| --- | ---: | ---: | ---: | --- |
| frozen R1 | 1,311,810 | 61,441 | 9,219 | two full-source copies per element |
| reconstructed first B | 1,311,810 | not dominant | 3 | 3,072 float-range helper calls and string instructions |
| exposed source, length once | 1,403,976 | 61,441 | 5 | no repeated payload copy; 73,734 link/unlink pairs |
| one local snapshot, length once | 1,281,096 | 61,441 | 4 | one source snapshot; no projection-loop payload copy |

The minimal frozen-R1 profile directly records 6,150 binary `copy` operations
and 359,301,526 copied bytes versus three operations and 245,926 bytes in the
control. The projection delta contains exactly 6,144 copies of its 58,440-byte
minimal source, or 359,055,360 bytes; the production benchmark's 58,479-byte
source gives the 359,294,976-byte figure above.

The current `bgetu8` timing profile averages 13--15 ns per handler in these
diagnostic builds. Even deleting the whole measured handler cost would not be
enough to bring the `rxbvm` 1.755 ms scratch result below its roughly 0.533 ms
2x prototype ceiling; an unchecked load would retain dispatch, load and result
placement anyway. Bounds checks are real redundant work, but they are not the
dominant cause of frozen R1 and cannot alone close the remaining gate.

## Scratch Release replay

These are seven serial diagnostic samples per VM, not a replacement formal
verdict. Every child execution passed exact benchmark checksums.

| Variant | `rxvm` samples, us | median | `rxbvm` samples, us | median |
| --- | --- | ---: | --- | ---: |
| reconstructed first B | 2702, 2803, 2661, 2910, 2789, 2720, 2803 | 2789 | 3369, 3320, 3273, 3344, 3274, 3276, 3410 | 3320 |
| exposed source, length once | 1429, 1537, 1426, 1428, 1342, 1372, 1406 | 1426 | 1961, 2011, 1972, 2112, 1972, 2028, 2008 | 2008 |
| local snapshot, length once | 1264, 1460, 1311, 1262, 1528, 1419, 1271 | 1311 | 1914, 1755, 1825, 1813, 1617, 1713, 1698 | 1755 |

The first-B reconstruction agrees proportionally with the retained formal
2.918/3.4755 ms medians, validating module replacement and the comparison.

## Consequences and countermeasures

### Immediate CRI-13 production decision

Do not retain frozen R1. A source-local snapshot plus one length check removes
the accidental copy explosion and proves that the scanner design itself is
sound, but it still fails the approved prototype ceiling. The recommended R2
therefore remains parse-time classification: compute nonzero and binary64-safe
facts while `_json_scan_number` is already consuming the token, and store only
private flags in the existing unused u32 node field at offset 36. Projection
then performs no validation rescan. This changes no public JSON API, node size,
ABI or serialized format.

### Systemic copy issue: `PERF2-07-B02`

Queue a byte-weighted payload-copy census across representative Level B
library/class workloads. Attribute each `COPY_REG_REG` and typed payload-copy
byte count to actual/formal shape and source kind: direct local, class
attribute, global/exposed, reference, repeated actual or uncertain escape.
Classify each as required value isolation or a proof opportunity before any
compiler or assembler change. CRI-02's V1 proof remains valid and unchanged;
B02 considers the deliberately excluded non-local cases rather than weakening
value semantics globally.

Candidate proof mechanisms must include retaining the current rule, explicit
source exposure, one caller snapshot, compiler-recognized immutable/read-only
attributes, RXAS post-inline full-copy projection, and runtime
sharing/copy-on-write only as a separately gated architecture option. The
first bounded RXAS PoC should target the register-local `r40 <- r31` copy and
prove exact removed operations/bytes. A second panel should compare an `rxc`
invariant snapshot with an RXAS treatment of the complete
`linkattr/copy/unlink` lifetime; it must include exceptional cleanup and both
VMs. This is not active implementation work while CRI-13 is at its mandatory
stop.

### RXAS range-proof issue

Record, but do not select yet, the mismatch between `bcheckrange` and strict
per-access `bget*`. Viable future panels include retaining strict reads; adding
a distinct compiler-only/proof-checked fast read; or using the now-available
higher operand count for a generic bounded bulk/table-driven operation that
checks once. A new opcode or changed RXBIN semantics is an explicit
architecture/serialized-format decision and is not authorized by CRI-13.

## Raw evidence and commands

Evidence root: `/tmp/crexx-cri13-r1-rxas-trace.8SKYvs/`.

The frozen R1 source SHA-256 is
`1791e30d2400fba8098b6b774cdce1dfb9f042f1066fa99083cba96209a9dacb`.
Its freshly compiled live optimized RXAS SHA-256 is
`96be6ace27eb87983f4a9c4bfa043321356370ec2294deb956af5e0c7611cb06`.
The Debug `rxas` used only for the retained flow diagnostic is SHA-256
`7e6d2355dced39ee9f5ee172e61659bb2eb8540a2365f979042aaec67ced28ab`.
The 440,498-line debug stream is retained as
`rxjson-r1-live-rxas-flow-debug.log`; focused `NR27` reads, rather than the
verbose stream, are quoted above.

The profiling VMs are SHA-256
`1277d08a96aa8d6a16ae51991f43bd6af6944e98facee44b5d11277dc7f90fff`
(`rxvm`) and
`5cbe5a2321277164ab2c3fc1abbf44c9a751de7c53b61367a5ba446f139c3763`
(`rxbvm`). The complete raw manifest SHA-256 is
`37f49634ccbf32440e721e6dac5d79e5424c1976b74bf03157a8a0b8c13c7818`.

Representative commands:

```sh
cmake -S . -B /tmp/crexx-cri13-r1-rxas-trace.8SKYvs/profile-build \
  -G Ninja -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=ON \
  -DBUILD_TESTING=OFF
cmake --build /tmp/crexx-cri13-r1-rxas-trace.8SKYvs/profile-build \
  --target rxvm rxbvm --parallel 10

/tmp/crexx-cri13-r1-rxas-trace.8SKYvs/profile-build/bin/rxvm \
  --profile=counts \
  --profile-output /tmp/crexx-cri13-r1-rxas-trace.8SKYvs/isolate-rxvm-run-counts.csv \
  /tmp/crexx-cri13-r1-rxas-trace.8SKYvs/r1_projection_profile.rxbin \
  /tmp/crexx-cri09-release-a2.N4ELYs/build/bin/library.rxbin -a run
```

All alternative sources, RXAS/RXBIN, compile logs, count/timing profiles,
stdout and scratch Release replays are covered by `manifest.sha256`. No
production source was changed by the trace, and the read-only `crexx-rag`
checkout was not accessed or modified.

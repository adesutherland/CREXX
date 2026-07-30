# CRI-13 parse-once JSON and packed numeric closeout

Disposition: **fixed**

Date: 2026-07-30

Governed ID: `CAP-01-J02`

## Accepted production result

Adrian accepted raw Option B as the Release-1 surface and accepted the measured
disposition not to promote Option C wrappers now. The production
`.jsondocument` therefore exposes explicit `node_f32_array` and
`node_i64_array` projections to owning, headerless canonical-little-endian
`.binary`. The caller chooses type and expected count; JSON itself infers no
width, dimension, byte order, normalization or vector meaning.

Frozen production `lib/rxfnsb/rexx/rxjson.crexx` SHA-256:
`7916d23df7cc488adfee54d4b25e504fa3afd47a8746fb7d15a6f401bde83d77`.

The accepted R2 result uses parse-time private numeric classification and one
scoped conversion-signal translation. Optimized f32 projection is 295/326 us
on `rxvm`/`rxbvm`, 1.14x/1.22x the prototype. Production parse/project/scan is
9.60%/11.03% of the retained current-composition baseline. Exact statuses,
cleared outputs, bytes, signed-i64 extrema, overflow and nonzero-underflow are
covered in opt/no-opt and both VMs.

## Option C disposition

The retained benchmark-local comparison proved that typed wrapper construction
and ownership are cheap, but repeated class members remain 4.56x--5.10x raw B
for reads and 2.41x--3.67x for writes. Adrian accepted the recommendation to
retain B alone. The probe classes were removed after their source, RXAS, RXBIN,
commands, samples and verdict were retained in
[`CRI13-C-CLASS-RELEASE-VERDICT.md`](CRI13-C-CLASS-RELEASE-VERDICT.md).

The maintained benchmark returned exactly to SHA-256
`ffe920aed41293f767356a12db398b5a9a10272b50f2a7d1ed2815e15f784932`.
Its restored Release RXBIN hashes are the frozen R2 values:

| Mode | SHA-256 |
| --- | --- |
| non-optimized | `69422355fc99688f7bf60a5c06df2b3a8c95cc4180d03d84155f821b2709a56e` |
| optimized | `f37fb0beffdd4eb6b6779d0771017ca2dfa7e6d221569b9ba998eb8c5ce9ee59` |

Restore validation passes 5/5 across optimized/non-optimized `rxvm` and
`rxbvm`. Raw logs:

| Evidence | SHA-256 |
| --- | --- |
| `/tmp/cri13-final-benchmark-restore-debug-build.log` | `960ccb4c4f9a8978bf9f420503a87128c130d150451e126f241e828d56151e1f` |
| `/tmp/cri13-final-benchmark-restore-debug-ctest.log` | `145e81618d6b6fadbf494f1d9f68e629bd09e92b8cd340ed3d1375da5b09fea7` |
| `/tmp/cri13-final-benchmark-restore-release-build.log` | `960ccb4c4f9a8978bf9f420503a87128c130d150451e126f241e828d56151e1f` |

## Complete validation and compatibility

- accepted first Release evidence: focused Debug/Release 5/5, broader JSON
  Debug 17/17, formal numeric 40/40 and unchanged parser 24/24;
- proportional closeout: complete Debug 1,963/1,963 and affected ASan 17/17;
- restored maintained benchmark: 5/5 and exact frozen Release image hashes;
- documentation states the public byte, count, status, range and ownership
  contracts; and
- `git diff --check` passes.

The change is additive Level B library API only. No language syntax, native
ABI, public RXAS/RXBIN encoding, serialized format or general STOF/integer
conversion contract changed. The private JSON index remains ephemeral and is
not an ABI or persistence format.

Separate, non-blocking performance observations remain queued:

- `PERF2-07-B02`: residual full-value copy proof;
- `PERF2-07-C01`: bounded generic numeric-conversion review; and
- `PERF2-03-F06`: generic statically resolved concrete/final method-access
  ceiling exposed by the rejected C wrapper.

CRI-13 is fixed; none of those later items reopens it.

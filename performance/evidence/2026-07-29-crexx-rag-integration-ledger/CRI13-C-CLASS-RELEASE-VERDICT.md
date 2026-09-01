# CRI-13 Option C class comparison ordinary Release verdict

Status: **accepted disposition — B retained; public C deferred**

Date: 2026-07-30

Governed ID: `CAP-01-J02-C01`

## Outcome first

The benchmark-local typed wrappers are functionally correct and cheap to
construct, but their per-element members are materially slower than direct
typed binary-memory access. Optimized f32 `read()` is 5.01x raw Option B on
`rxvm` and 4.71x on `rxbvm`; optimized `write()` is 3.67x and 3.19x. The i64
results agree. Every hot-access cell exceeds the predeclared 2x non-promotion
boundary.

For a single parse, projection, construction and one scan, C is only
3.33%--5.24% slower than B because JSON parse/projection dominates one scan.
That total must not conceal the repeated-access result: the class method is the
intended ergonomic access surface and is roughly five times the raw read cost.

Recommendation: retain accepted Option B as the sole production JSON packed
surface now. Do not promote the probe classes as a performance-facing API.
Revisit C only after a separately governed generic concrete/final method
inlining or equivalent safe class-access countermeasure demonstrates that
typed members approach direct binary-memory cost. Do not add operation-specific
comparison members to hide the general access result.

## Frozen comparison

The maintained Level B benchmark is
`tests/performance/rxjson_numeric_materialization_compare.crexx`, SHA-256
`2d6eeb80b599ee5c1fb366b61729dc70cc23a56f045fe283b439e894bbb5a016`.
It retains the complete accepted JSON/B workload and adds two benchmark-local
classes only:

- `PackedF32Probe` owns by-value headerless binary32 bytes;
- `PackedI64Probe` owns by-value headerless signed-i64 bytes;
- `read(index)` returns one typed scalar by value;
- `write(index, value)` accepts the scalar by value and mutates only owned
  bytes;
- `payload()` returns a by-value bridge to raw B; and
- no public class name, JSON method, envelope, metadata or serialization is
  added.

The focused Debug matrix passes 5/5 across optimized/non-optimized `rxvm` and
`rxbvm`. It proves count and byte length, exact values/checksums, class-write
isolation from the caller's B value, returned-payload isolation, and unchanged
JSON/B results.

Focused evidence:

| Artifact | SHA-256 |
| --- | --- |
| `/tmp/cri13-c-focused-build.log` | `960ccb4c4f9a8978bf9f420503a87128c130d150451e126f241e828d56151e1f` |
| `/tmp/cri13-c-focused-ctest.log` | `41d19bdbbcded992f7c021c39864c41b7cc0d2104e307d5a20ae8a1257e937f3` |
| optimized RXAS | `240e45aef85090fbe4333057c249e056ee1e898aa508122e8c3e6f525a4a41ca` |
| non-optimized RXAS | `19447348bc65f5aefa55a3c363c37c4a5693ed7d4141585e373f4c5289dc9973` |
| optimized RXBIN | `78582e9b604dea298f449c72cf33dc314dcc83fe5b33685f1871a279ad5f0ca4` |
| non-optimized RXBIN | `8d1d396e06e4a25767387eb5854756cac2cce16f21b83ae7cb460262d933b036` |

## Ordinary Release protocol

The ordinary profiling-off Release product remains
`/tmp/crexx-cri09-release-a2.N4ELYs/build`, with accepted R2
`rxjson.crexx` SHA-256
`7916d23df7cc488adfee54d4b25e504fa3afd47a8746fb7d15a6f401bde83d77`.
Only the benchmark images changed for this comparison.

Evidence root: `/private/tmp/crexx-cri13-c-release.g25EIS/`.

The Apple M5 host was on AC with low-power mode disabled before and after.
Every opt/no-opt x VM cell used two warmups and ten serial recorded samples,
rotating the starting cell each round. All 8/8 warmups and 40/40 recorded runs
passed; no outlier was removed.

| Evidence | SHA-256 |
| --- | --- |
| `numeric-warmups.log` | `bf3c12c96427f83e34f7eef0a9c455ce34dc5132e45b3c6fe780701877f4c6bf` |
| `numeric-raw.log` | `ab9b6b422cf9264b31e6aa3009d5cfa1246c495afa5ef7ef3c479e189b5a584b` |
| `numeric-samples.tsv` | `d5280559138b33966f34b656262bec9b852160e6abfd4759802e1f2ee6ab4d99` |
| `numeric-medians.tsv` | `c60f3f3e5aac6c5b548b14d7bf48406472f73b0822e6d46fba888f4580495dce` |
| `verdict-calculations.txt` | `4df7ffe0c4401ee93eab728088ba43eac03d06dfb822c447b37eae1a1dddbbd0` |
| `manifest.txt` | `0e74ba50d5cebe22cc5b325a5334bf278c29fb4ee1b5b79ae95277d4461fd28b` |

## f32 medians

Times are microseconds. Reads traverse 3,072 values ten times; writes update
3,072 values once. The total is parse plus accepted B projection plus one raw
or class scan.

| Cell | B read | C read | C/B | B write | C write | C/B | B total | C total | C/B |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm -n` | 193.5 | 942.5 | 4.87x | 19 | 64 | 3.37x | 1,761 | 1,854 | 1.053x |
| `rxvm opt` | 186.5 | 934.5 | 5.01x | 18 | 66 | 3.67x | 1,747 | 1,838.5 | 1.052x |
| `rxbvm -n` | 233 | 1,088 | 4.67x | 23 | 74 | 3.22x | 2,171 | 2,277.5 | 1.049x |
| `rxbvm opt` | 229 | 1,078.5 | 4.71x | 23.5 | 75 | 3.19x | 2,174.5 | 2,247 | 1.033x |

## i64 medians

| Cell | B read | C read | C/B | B write | C write | C/B |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm -n` | 208 | 947.5 | 4.56x | 20 | 64 | 3.20x |
| `rxvm opt` | 183 | 934 | 5.10x | 19 | 64 | 3.37x |
| `rxbvm -n` | 235.5 | 1,082 | 4.59x | 29.5 | 76 | 2.58x |
| `rxbvm opt` | 225.5 | 1,071.5 | 4.75x | 31.5 | 76 | 2.41x |

Wrapper construction medians are 2--3 us for f32 and 3--4 us for i64;
by-value payload extraction is 1--3 us. Peak RSS is 2.11%--2.32% above the R2
diagnostic, not material. Optimized versus non-optimized C f32 reads change
-0.85%/-0.87% on `rxvm`/`rxbvm`; writes change +3.12%/+1.35%; totals change
-0.84%/-1.34%. There is no optimizer inversion above 10%.

## RXAS cause

The optimized raw B loop contains the typed binary-memory operation directly.
The optimized C caller instead retains `call2 ...packedf32probe.read()` or
`call3 ...write()` on every element. The compiler resolves the exact local
method target, but does not inline it.

Each f32 `read()` invocation then executes initialization/numeric-context
setup, `linkattr1` for the payload, index arithmetic, `bgetf32`, `unlink`, and
`ret`. `write()` has the corresponding `bsetf32` path. The scalar itself is
passed/returned by register value; there is no full binary copy per element.
Factory ownership contains one `bcopy`, while caller-isolation checks prove
that factory and `payload()` boundaries have correct by-value semantics.

Therefore this result is a general class-call/attribute-access cost, not a JSON
parser issue, typed binary-memory issue, bound-check issue, case conversion or
repeat of CRI-02's payload-copy regression.

## Verdict against predeclared rules

| Rule | Verdict |
| --- | --- |
| Exact count/bytes/read/write/caller-isolation checks | pass |
| Focused opt/no-opt, dual-VM correctness | pass, 5/5 |
| 40/40 Release samples and checksums | pass |
| Optimized C/B hot read and write <=1.25x | fail |
| C/B hot read and write <=2x non-promotion boundary | fail in every read cell and every write cell |
| No optimizer inversion above 10% | pass |
| No material RSS increase | pass, <=2.32% |
| Public API/ABI/RXAS/RXBIN/serialized format unchanged | pass |

## Decision-stop preservation audit

`git diff --check` passes. CREXX remains on `develop` at
`d78c6fcfa81ef03fdbea65ff9cc39ed99e8716bf`; no change is staged. The accepted
production `rxjson.crexx` hash remains
`7916d23df7cc488adfee54d4b25e504fa3afd47a8746fb7d15a6f401bde83d77`.

The read-only `crexx-rag` checkout remains on `main` at
`97cd87e91344d6ac1773a054bd38df23eb128ed2`. Its tracked-diff, empty index-diff
and no-branch porcelain-v2 status SHA-256 values are respectively
`97443028cfa8e86624fc5fda9ea5fb33d02f4d7413e5a8a848d92ec365288ef9`,
`30cea35503c6dc073f3007218b9458f2bc0c28b2c7661327b9144036d5a7c61d`,
and `02a6e4216ff47be3ce7252be2ccb39157bc45fda316c254f7198648109df14f1`.
They exactly match the preceding item-boundary audit; the sister checkout's
pre-existing 72-line status and all files were preserved.

## Alternatives and recommendation

### A — retain B only for Release 1 (recommended)

Keep `node_f32_array` and `node_i64_array` returning explicit raw headerless
`.binary`. Callers use established typed binary-memory operations. This keeps
the fastest and simplest surface and creates no additional public class
contract.

### B — publish both B and the measured C surface now

C would provide runtime type identity and convenient scalar members. Its
single-pass end-to-end cost is modest, and callers could escape through
`payload()` to B. However the obvious repeated `read`/`write` use is 2.4x--5.1x
slower, creates duplicate public routes and requires decisions about invalid
instances, equality, copying and future widths. Not recommended while the hot
surface has a known generic implementation ceiling.

### C — improve generic concrete/final method access, then remeasure C

Use a separate governed compiler/performance item to test safe inlining or an
equivalent direct class-attribute access transformation for statically resolved
methods. It must preserve initialization, receiver identity, attribute
ownership, signals, source mapping, debug behavior and writable isolation.
Only then rerun this exact B/C comparison. This is the recommended prerequisite
if typed wrapper convenience remains desired.

### D — add bulk or in-place comparison members

Operation-specific methods can amortize calls but do not solve generic typed
read/write ergonomics and would prematurely choose numeric/vector operations.
Rejected for CRI-13.

## Smallest exact decision

Decision: **accepted by Adrian on 2026-07-30**.

Adrian should decide only whether to accept the measured disposition:

> Keep Option B as the sole Release-1 production JSON packed surface and defer
> public Option C wrapper classes until a separate generic class-method
> inlining/access countermeasure brings typed read/write closer to raw B.

Selecting publication of both surfaces despite the measured gap is a separate
public-API trade-off and should be stated explicitly.

## Paste-ready continuation prompt

```text
Resume /Users/adrian/CLionProjects/CREXX at the CRI-13 Option C comparison
decision stop using performance/CREXX-RAG-INTEGRATION-WORKLIST.md and
performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-C-CLASS-RELEASE-VERDICT.md.

I accept the recommended CRI-13 C disposition: retain the accepted raw
node_f32_array/node_i64_array Option B methods as the sole Release-1 production
packed JSON surface. Do not promote the benchmark-local C probe classes now.
Record the generic concrete/final class-method inlining/access opportunity as a
separate governed performance item with this evidence, but do not implement it
inside CRI-13. Remove the benchmark-local C probe after retaining its evidence,
restore the maintained benchmark to the accepted B surface, complete
proportional CRI-13 closeout, and then begin CRI-14. Keep PERF2-07-C01 queued;
do not change STOF, public RXAS/RXBIN, ABI or serialized formats, and do not
touch crexx-rag.
```

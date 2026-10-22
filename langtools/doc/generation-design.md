# Level L generation: lexical construction reuse

**APPROVED by Adrian; implemented and locally qualified.** The [generic Level L plan](../../research/level-l/PLAN.md#current-action-lexer-reuse)
owns LRUSE acceptance and sequence. This contract belongs to `langtools`, not
to a source-language compiler or the runtime SDK.

## Intended outcome and ownership

Avoid rebuilding an unchanged lexer when only parser/AST productions change.
Reuse immutable main and outside-comment-shield DFA construction only. Keep
SDK/runtime algorithms, C/B semantics and generated-language behavior unchanged.
Generated languages remain self-contained, with no cache/provider dependency.
The existing `llregular`, `llmunch` and `lllanguage` public APIs stay unchanged.

`llgenerate` remains a pure service with owned results and no file I/O. Every
request freshly checks source/encoding, frontend admission, parser construction,
comment policy, stages, diagnostics and output. A hit cannot bypass those
checks, current limits or publication rules. Cache state is not grammar policy.

## Service contract

Append optional `?lexical=.binary` after `bytes` in the existing request factory;
all preceding arguments and defaults retain their meanings. Add these queries:

| Call / input | Required behavior |
|---|---|
| `lexical` omitted | Existing cold behavior, with no capsule key/encoding construction overhead. |
| Present empty binary | Cold build; make its current reusable capsule available on success. |
| Present nonempty binary | Try checked reuse; any candidate miss falls back to cold construction. |
| `lexical() -> .binary` | Owned candidate only after successful opted-in `rxas`/`crexx` generation; otherwise empty. |
| `reused() -> .int` | 1 only after successful generation used the admitted complete DFA set; disabled/cold/graph/error requests return 0. |

Queries join the existing once-only request evaluation; repeated queries neither
rerun generation nor change results. A tentative hit followed by any current
admission/output error exposes no output or capsule and reports `reused()=0`.
Graph output does not construct/use capsules. Unstamped manual builds stay cold,
returning empty `lexical()` and zero `reused()` even when opted in.

The existing request `bytes` cap also bounds key construction and capsule
encoding/decoding. Stop canonical key construction at the cap and disable
reuse/capsule production for that request, preserving ordinary cold admission
and output; do not materialize an unbounded key and check size afterward. Reject
an oversize candidate as a miss before decoding. If successful cold generation
cannot encode its capsule within that cap, omit the candidate; generated output
still obeys its ordinary cap. No new high-water or resource-limit API is added.

## Private capsule and identity

Use a private versioned, data-only capsule: no executable code, new installed
provider or public format-stability promise. It contains the immutable main DFA
and every required shield DFA as **all-or-miss**, plus the construction counts
needed to reproduce existing generated headers. Validate image dimensions and
NFA/DFA counts against the exact matching limits. Reconstruct current policy,
source associations and emitted metadata from the new document.

The key covers:

- The private automated generator-source stamp and image/encoding/Unicode identities.
- Canonical ordered reachable lexical graph topology, operations, ordered
  members/operands and literal bytes, actual emitted/skip IDs and rule priorities.
- Ordered shield roots and associations, covering every outside-comment pattern.
- Exact effective `nfa.states` and `dfa.states` construction limits.

Deterministically renumber reachable graph nodes from ordered roots/edges;
exclude incidental raw graph IDs, source lines/labels, parser/AST productions,
source filename and unused/non-emitted token declarations. A declaration change
that changes an actual emitted ID still changes the key. Preserve ordering and
topology; this is not an algebraic equivalence or language-minimization pass.
Do not change `llcore.signature` or add a generic fingerprint API.

Changing lexical construction limits requires cold checks, even if a previous
image happens to fit. Lower limits must never inherit admission from a previous
higher-limit build. Other current policies/limits are freshly applied normally.

## Admission, failures and trust boundary

Missing, stale, malformed or oversize candidate data gives cold construction
with `reused()=0`. Decoder validation checks bounded lengths, overflow, counts,
duplicates and entire input consumption, then admits every image through the
current immutable loader. Partial main/shield reuse is not allowed.

A candidate failure must not leak cache diagnostics into current specification
admission. Source, budget, construction and output failures still fail normally
and expose no artifacts. This does not promise recovery from unrelated host
failures or that every valid-looking bit flip is detected by the service.

The candidate is a trusted host-owned compilation artifact, not authentication
or proof against maliciously rewritten semantics. The service checks structure
and key. Exact SHA256 integrity of persisted bytes belongs to the persistence
host: direct callers reuse owned service outputs or validate stored bytes
externally. Do not introduce a new `rxhash` bootstrap/provider dependency.
The CMake wrapper checks the capsule checksum before supplying a candidate.

## CLI transport

Use `--lexer-in PATH` and `--lexer-out PATH`, at most once each, for `rxas`
and `crexx` only; reject either with `--graph`. Paths must be mutually distinct
and must not designate the source path. Reject exact `stdin`, `stdout` and
`stderr` transport names before I/O. Lexical spelling normalization and a
conservative input/source-byte equality guard are not full filesystem
canonicalization. Create `--lexer-out` scratch exclusively with existing `wbx`
I/O, refusing every existing destination, including symlink/hardlink aliases;
never truncate an input. This is transport, not a second cache
policy or a public capsule format.

Read/write bounded binary data through existing B I/O primitives, without
`rxfs`, `rxhash` or another provider import. The wrapper handles missing/stale/
damaged persistent caches; explicit CLI transport I/O errors fail. Successful
generated stdout remains unchanged. Finish and successfully close any requested
scratch capsule output before writing generated stdout, so a capsule I/O failure
emits no generated artifact. The lexical output path is caller-owned scratch:
direct CLI writes have no atomic publication guarantee; the wrapper owns publication of real products.

## CMake persistence and atomic publication

Use boolean `REUSE_LEXER=ON` for `generate.cmake`. Derive one private cache
location from `OUTPUT`; do not accept a freely shared cache path. Initially opt
in only the research C grammar. Global, user and bootstrap caches stay disabled
by default. The persistent ignored-build artifact is owned and cleaned by its
generation target; it is not a log, receipt or retained diagnostic bundle.

For `REUSE_LEXER=ON`, lock the canonical output producer and its derived cache before reading,
generating or publishing. Use unique owned staging, never another invocation's
`.tmp`. A missing cache is not an output prerequisite and must not force an
otherwise up-to-date generation command to run. Maintain one output producer.
Reuse-disabled generation still uses unique staging and atomic rename, without
shared cache state or a persistent cache lock.

The private manifest records SHA256 of the actual `rxlang` executable, or the
ordered VM plus linked-image pair, the wrapper identity and capsule checksum.
The `.rxl` source is not the persistence key: parser-only edits must be able to
reuse matching lexical construction after fresh full admission.
Reuse assumes unchanged generator/stamp artifacts. A normal post-commit version
relink can change seed-tool hashes and the actual executable fingerprint, causing
an intentional miss even when lexical semantics did not change; reuse across
such artifact changes is not promised.

Run generation and validation entirely before publication. Failed generation
changes neither prior output nor cache. Replace staged cache data and manifest
atomically per file, with the manifest last as commit marker; an interrupted or
inconsistent pair is a miss on the next invocation. Do not rewrite an equal
capsule. Publish the generated output atomically as the real product. A cache
may remain independently valid if final output publication is interrupted;
there is no claim of a multi-file filesystem transaction across all products.

## Automated stamp and bootstrap closure

Use one private automated content stamp shared by L0/L1/L2. Its explicit,
checked source-closure manifest covers stable logical paths plus content hashes
of maintained generator/frontend/SDK/property data and build transforms. Include
the existing pre-L0 `library.rxbin`, `rxfnsl.rxbin` and native seed tool identities/
flags needed to bind the direct service's construction implementation. These
are already-built prerequisites and introduce no reverse bootstrap edge.
Build dependencies cover the whole declared closure, including new modules;
a manual version bump alone is insufficient.

Exclude downstream/stage-generated outputs and grammar/linked images, the stamp
itself, timestamps and absolute paths. Inject the same private generated literal
through supported CMake, standalone and bootstrap source preparation. Unprepared
manual source disables reuse rather than sharing a placeholder identity. The
wrapper's actual executable or VM/image fingerprint additionally detects native,
backend or other build changes not represented by a source stamp alone.
No dependency points back to research, and no new native provider or bootstrap
cycle is introduced. Check the source manifest/dependency coverage and narrow
L0/L1/L2 fixed point; do not silently rely on stale prepared source.

## Qualification boundary

Start with independent non-C/G fixtures: identity hits/misses; fresh parser
errors; disabled/cold/warm identical `rxas` and `crexx` output; priority, Unicode,
spans and comments; lower NFA/DFA limits; malformed/oversize capsules and partial
manifests; failure atomicity, no-op/missing-cache behavior, concurrency and paths
with spaces. Preserve generic provenance and current source-line diagnostics.
Use narrow stamp-closure/fixed-point checks before the real consumer.

Then perform one actual C cold fill and one parser-only warm generation with
unchanged lexer semantics/output equivalence for that parser input; retain an
unchanged G regression. Reuse current products and valid evidence. Independently
inspect actual RXAS where emission changes. Measure only necessary nested-test
additions in normal Debug and complete the required product build; no broad
matrix, per-slice ASan or formal benchmark is commissioned. Keep lasting results
in the generic plan, delete temporary diagnostics and preserve unrelated work.
Implementation follows the approved contract and assigned ownership. After
qualification, root clears scoped commits under Adrian's existing local-commit
authority; push/publication remains a separate instruction.


<a id="unicode-construction-investigation"></a>

## Unicode construction investigation — proposed next increment

The generic plan's `L-UNICODE-REUSE-01` investigation is commissioned on
`hotfix2`; product implementation and new cache/alphabet contracts await
coordinator review. This is an ordinary research development increment.
Broader compiler/platform/sanitizer QA and publication are not part of it.

The current separation is already reusable: `llproperties` supplies pinned
packed Unicode intervals, `llcharset` owns interval algebra and scalar-to-UTF-8
lowering, `llsource` validates each actual input, and `llregular` constructs the
combined lexer. Runtime input validation is not rebuilding Unicode data. The
combined accepting machine still depends on the author's regex composition,
ordered rules, token IDs and longest-match policy.

Scratch phase probes use full XID start/continue sets plus underscore for a
Unicode identifier/space lexer, an independent ASCII name/space lexer and a
small LR list grammar. On this representative Unicode graph, property loading,
set union and UTF-8 lowering are small; determinization dominates. The current
subset construction sorts each successor to canonical binary once, then scans
all previously discovered subsets for exact equality. It also scans the NFA
for every byte while deriving byte classes. A bounded native sample during
DFA work did not observe the known VM storage-owner tree search; it does not
prove that owner cost is absent from larger owners, frontend work or other
phases. The full C grammar/parser and complete-generator lifecycle are not
profiled by these probes. This run does not separately time byte-class
partitioning, epsilon closure/sorting, subset lookup or final table filling;
the exact-key index is a source-inspection hypothesis whose actual benefit is
unmeasured. Phase localization does not prove avoidable work, establish UTF-8
as the cause, or show that current construction is far from optimal. An index
adds hashing, storage and maintenance; its likely benefit must justify that
complexity before implementation. Keeping the current implementation is valid.
This is decision evidence, not a formal benchmark or performance acceptance gate. Numeric timings belong in conversation, not this
contract.

### Possible bounded index slice — benefit not established

If further proportionate evidence justifies a change, one small candidate is a
private bounded exact binary-key index in the existing shared `llregular`
construction owner for DFA subset identity. All user lexers would then use the
same construction algorithm; there is no cross-lexer machine-cache promise.
Current matching, runtime, canonical successor construction, closure ordering
and byte alphabet remain the baseline. No product change is commissioned by
this proposal.

The candidate contract for coordinator review is:

1. Build each canonical successor binary once. Compute its index hash once;
   retain exact bytes as the identity. Hash/length agreement is only a candidate:
   equality always compares the complete canonical bytes. Deliberate collisions
   must neither merge distinct subsets nor lose duplicates.
2. Lookup returns the existing discovery ID. On a miss, enforce the current
   `dfa.states` and representation checks before publishing the next ID, append
   that state exactly where the current algorithm does, then install the index
   entry. Do not enumerate hash buckets to choose state order or acceptance.
   NFA order, first-priority accepting rule, edges and generated state IDs remain
   deterministic. No change to longest matching, byte spans or UTF-8 policy.
3. Grow private index storage from actual admitted states, with overflow-safe
   capacity/hash arithmetic, bounded probing and room for a terminating empty
   bucket. Do not allocate in proportion to an arbitrarily large caller limit.
   Index entries cannot outlive their builder; construction refusal exposes no
   partial DFA. Existing NFA/DFA limits remain authoritative, including their
   failure boundary; hash collisions are not a new state admission limit.
4. Keep the first change local to `llregular` plus focused fixtures and owning
   guidance. Do not simultaneously alter LR interning, Unicode data, the native
   compiler, VM lifetime ownership, emitted image format or LRUSE identity.
   A future second construction consumer may justify a shared SDK index owner;
   that API decision is separate from this private implementation.

Before product edits, review hashing/collision behavior and exact refusal
order; inspect representative RXAS at the first implementation compile.
Focused qualification should cover an independent
small lexer and a Unicode lexer, priority/equal-length overlap, longest match,
spans, nullable rejection, duplicate/colliding subset keys, lower NFA/DFA
limits and cleanup after failure. Compare exact construction images or stable
structural facts where those are independently justified. No speed claim
follows from asymptotic reasoning alone.

### Remaining reuse and alphabet gates

- **Immutable property/set/lowered fragments:** sharing canonical packed input
  or a lowering template is possible across lexers, but merely caching today's
  property loads/lowered graph leaves the expensive combined determinization.
  Compact prefix-factored byte-set fragments could reduce its input size; they
  need a separate reviewed representation, provenance/occurrence mapping and
  unchanged cold/reused logical-limit charging. Do not confuse graph DAG
  sharing with NFA reuse: the current recursive fragment builder expands each
  occurrence. Reusable accepting-machine pieces cannot carry author-specific
  token/priority decisions into another lexer.
- **Scalar/class alphabet:** potentially avoids UTF-8 byte expansion, but needs
  a new or versioned NFA/DFA image/alphabet, shared strict decoding/classification
  boundary, custom-set partitioning and exact original byte spans. Keep this
  larger runtime/loader/generator choice at its own design gate; no duplicate
  decoder/property algorithm and no C-specific policy belongs in generic L.
- **Relink-tolerant persistence:** source/version intent alone cannot certify
  bytes produced by changed native tools. A future semantic-capability manifest
  must cover Unicode inputs, canonical set/encoding/algorithm/image versions,
  relevant options and the actual producing implementation while excluding only
  demonstrably irrelevant relink data. Define trusted producers, host integrity
  checks and stale/malformed refusal before weakening executable fingerprints.
  The current conservative LRUSE stamp, manifest and cold-miss behavior stay
  unchanged in this investigation and in the proposed private-index slice.

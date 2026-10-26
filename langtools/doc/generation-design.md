# Level L generation: frozen and prepared definitions, lexical construction reuse

The [generic Level L plan](../../research/level-l/PLAN.md) owns the acceptance
and sequence for these separate mechanisms. Trusted frozen generated definitions
are the selected LFP implementation increment; its qualification remains tracked
there. The checked scanner/nonregional LR1 prepared-definition profile is locally qualified under LDP; broader Level L
acceptance remains open. Lexical construction reuse is locally qualified under
LRUSE. This contract belongs to generic language tooling, not to one research
source-language compiler.

## Trusted frozen definitions (LFP1)

Generated scanner-only and nonregional DFA/LR1 modules embed read-ready LFP1 data
and call `lllanguage..compiled(image=.binary)`. Both source and RXAS output use the
same build-time constructor and encoder. The RXAS call reserves its complete
physical formal window; source uses a procedure-local named binary constant.
Auxiliary property/flow/services and other parser backends keep their existing
generation route.

`definition.frozenimage(bytes=.int=67108864)` performs normal checked admission
at encode time, then writes all fixed rows, offsets, signatures, CFG nullable/FIRST
facts, LR directories and AST field lists. Existing immutable loaded main/shield
DFA and LR owners are required. `compiled()` consumes only these trusted,
prevalidated artifacts: a short magic/version/profile/extent/fixed-directory
check precedes native section copies and scalar binding. It does not perform
runtime UTF-8/schema/CFG/DFA/LR validation, hydrate per-row arrays or construct
indexes. Arbitrary external bytes are outside this entry's contract; use the
checked [LDP1 loader](#prepared-definitions-ldp1) for them.

LFP1 uses the same portable little-endian `u32`/`i64` conventions and twelve
section IDs listed below, with magic `LFP1` in the 20-byte header. Scanner images
omit IDs 2–5. Grammar includes its exact canonical signature; CFG includes
precomputed facts. Text and blob pools use section-relative `u32 offset,length`
spans, with no pointers or executable state. Main metadata sections have a
32-byte magic/version/count/auxiliary/rows/auxiliary-rows/index/pool header:

| Section | Read-ready layout |
|---|---|
| Comments (`FCMT`) | 72-byte rows and owned shield blobs. Auxiliary header preserves the original checked-export ready flag; compiled rules are always frozen. |
| Stages (`FSTG`) | Initial state/limit then 72-byte rows, preserving first-match order and word normalization. |
| Projection (`FPRJ`) | 24-byte name/kind/arity rows. |
| AST schema (`FAST`) | 24-byte mapping rows, 40-byte field rows and per-mapping `u32` field-ID lists. IDs retain global declaration order; mappings sharing a kind remain distinct. |

Construction owns each component's individual subsection, not another full
outer image per child. Accessors borrow anchored fields while reading. No weak
reference into encoder/constructor locals survives. Collection-returning and
explicit export APIs may still make their ordinary owned copies; B values are
not assumed to be copy-on-write. No per-input AST arena packing is introduced.

Frozen builder attempts signal ERROR with `LL_FROZEN_DEFINITION:` detail and,
if handled, return the existing failure result without poisoning readable data.
Header refusal signals INVALID_ARGUMENTS with `LL_LANGUAGE_IMAGE:` detail and
returns a typed absent definition if handled. Trust does not add a new malformed
input guarantee beyond that header. Explicit `image()` reads the frozen getters
and reconstructs the checked LDP1 form when requested, preserving original raw
labels, revision, token holes, metadata order and comment-export readiness.

Use frozen definitions for repeated parsing, immutable result arenas for tree
consumption, and separate mutable builders/drafts, annotations and IR for
construction or analysis. Rewrites publish a new tree. Immutability and packed
storage are distinct; selecting one does not imply the other.

## Prepared definitions (LDP1)

The checked SDK data route is `definition.image()` followed by
`lllanguage..prepared()`. It remains available for external data and explicit
exports independently of generated modules' trusted entry. The loader does not
replay public graph/schema/stage setters; it validates local candidates and
hydrates owned storage before publishing. A direct RXAS call reserves all three
physical formal slots, including both omitted optionals, and explicitly sets
their presence flags using the native B ABI.

Version 1 supports scanner-only and nonregional DFA/LR1 definitions. Main and
custom shielding DFAs, and LR tables when present, must already be immutable
owners from the existing `loaded()` APIs. Encoding a mutable table owner signals
ERROR with `LL_LANGUAGE_IMAGE` detail; it does not change that owner's mutation
behavior. Other parser backends keep their existing generation route. There is
no implicit conversion of an unsupported profile or fallback after a malformed
prepared image.

The public operations are:

- `definition.image(bytes=.int=67108864) -> .binary`: an owned image of the
  supported definition; existing poisoned metadata cannot serialize success.
- `lllanguage..prepared(image=.binary, ?limits=.lllimits..policy,
  bytes=.int=67108864) -> .lllanguage..definition`: validate local candidates,
  then publish one complete owner. This is a namespace procedure, not a factory.

Failure signals INVALID_ARGUMENTS with an `LL_LANGUAGE_IMAGE:` message prefix;
explicit resource exhaustion signals OUT_OF_RANGE, and an unsupported definition
profile uses ERROR. Original typed owner failures retain their payload and
origin. If a caller handles or ignores failure, `prepared()` returns a typed
absent definition. It does not return a partial object or replace another owner.

### Portable envelope and sections

`LDP1` is a versioned data representation, not a native object-memory dump. All
integers use portable little-endian encoding: unsigned 32-bit lengths, counts,
IDs and flags; signed 64-bit grammar operands, revisions and scalar settings.
Binary values have a `u32` byte length; text has the same framing and must be
valid UTF-8, including length-bearing embedded NUL. Literal and character-set
bytes remain binary. No process address, weak reference, executable state,
request, native binding or global cache is serialized.

The fixed 20-byte header is magic `LDP1`, version 1, total byte extent, profile
(0 scanner, 1 LR1) and section count. Each 12-byte directory row is section ID,
absolute byte offset and byte length. Rows are strictly ordered by ID, contiguous
and exhaustive: no duplicate IDs, gaps, overlap or trailing bytes. Scanner
images omit sections 2–5; every other section is present even when its metadata
collection is empty.

| ID | Owned section |
|---|---|
| 1 | Backend, encoding/Unicode, lexical/EOF/recovery settings and field presence. |
| 2 | Grammar profile, exact raw labels, lines, operands, literal/set bytes and revision. |
| 3 | Canonical CFG productions and RHS layout, bound to grammar/root identity. |
| 4 | Existing immutable sparse LR image. |
| 5 | Exact per-state LR action/goto row directory. |
| 6 | Existing immutable main DFA image. |
| 7 | Comment rules, freeze state, source coordinates and custom shielding DFAs. |
| 8 | Ordered token stages, declaration order and original mutation limits. |
| 9 | Token names, empty holes and original extent. |
| 10 | Projection schema. |
| 11 | AST mapping and ordered field schema. |
| 12 | Authored diagnostics and context masks. |

Small metadata sections carry their own magic/version and bulk-hydrate checked
arrays. Grammar/CFG rows retain packed storage; existing mutation boundaries
materialize only the affected owned value. Raw empty labels stay distinct from
`label()`'s display fallback, and a revision is not inferred from clause count.
A normal LR definition retains no nominal public `grammar()` object and keeps
zero `root()`; its tables own the CFG/source grammar. Scanner definitions acquire no
fabricated parser, operators or regional owner.

### Validation, ownership and limits

Lengths, minimum physical row sizes, integer arithmetic and the complete
section directory are checked before allocation or indexed reads. One existing
CFG expansion authority checks supplied productions/RHS/maxima against the
source graph. The existing DFA/LR validators remain authoritative; a supplied
LR directory must match every real sparse group, including empty rows. No
supplied offset or signature substitutes for these checks. Validation establishes
representation consistency, not artifact authenticity or independent proof that
a producer chose the intended language.

One operation-local checked budget is shared by all codecs. It reserves wire,
owned-copy and logical decoded storage, plus bounded cold metadata-comparison
work, before the corresponding allocation/traversal. It is not an RSS or elapsed
time guarantee. Omitted policies use artifact dimensions rather than a parser
request's default source-byte or construction-state allowances. Explicit policies
retain connected grammar/CFG, DFA/LR and stage constraints. Recognition work and
source acquisition remain governed by their existing request/host limits.
Generation uses the SDK's default 64 MiB logical image budget independently of
the generation request's `bytes` cap, which still belongs to emitted output text.
The hexadecimal-size preflight and existing output writer enforce that text cap;
increasing it does not automatically increase the prepared profile's storage cap.

Every returned component owns its bytes/arrays. No budget, reader or
constructor-local weak reference survives loading. Existing value getters,
comment freeze state, case/order/duplicate rules and mutation errors retain their
meaning. Borrowed readers still require an anchored live owner; loading does not
create a new cache, ownership service or VM mechanism. The existing SDK and
bootstrap source/stamp closure include the codec without a reverse compiler edge.

## Lexical construction reuse

**Implemented and locally qualified.** The
[LRUSE plan](../../research/level-l/PLAN.md#current-action-lexer-reuse) owns its
remaining wider acceptance boundary. A lexical reuse capsule is advisory build
input; LFP1 is the trusted generated runtime artifact, while LDP1 is its checked
external/export counterpart. Their failure and
fallback rules are deliberately different.

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

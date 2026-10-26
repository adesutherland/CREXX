# Level L generation: prepared definitions and lexical construction reuse

The [generic Level L plan](../../research/level-l/PLAN.md) owns the acceptance
and sequence for these two separate mechanisms. The selected scanner/nonregional
LR1 prepared-definition profile is locally qualified under LDP; broader Level L
acceptance remains open. Lexical construction reuse is locally qualified under
LRUSE. This contract belongs to generic language tooling, not to one research
source-language compiler.

## Prepared definitions (LDP1)

A supported generated `language()` embeds one complete data image and calls
`lllanguage..prepared()`. RXAS uses a binary constant operand; generated Level B
source uses a procedure-local named binary constant. Both formats use the same
build-time definition constructor, encoder and runtime hydrator. They do not
replay graph/schema/stage setters on every `language()` call. Properties, flow
facades and their existing exports remain separate generated services.
The RXAS call reserves all three physical formal slots, including both omitted
optionals, and explicitly sets their presence flags using the native B ABI.

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
input; an LDP1 definition is the generated runtime artifact. Their failure and
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

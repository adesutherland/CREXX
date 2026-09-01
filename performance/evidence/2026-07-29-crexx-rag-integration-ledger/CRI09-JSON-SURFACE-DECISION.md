# CRI-09 final JSON surface decision

Status: Option B approved exactly as specified on 2026-07-30; implemented and closed by [`CRI09-CLOSEOUT.md`](CRI09-CLOSEOUT.md)

Date: 2026-07-30

Approval record: Adrian approved the packet's Option B without amendment and
authorized implementation through the mandatory first ordinary Release
performance verdict. Packed numeric serialization, schema syntax,
repair/coercion policy, public C ABI, and RXAS/RXBIN changes remain outside the
approval.

## Exact decision boundary

CREXX has a strict, string/path-oriented `rxjson` module. It can validate a
complete JSON value and distinguish object, array, string, number, boolean,
null, missing, and invalid paths. It cannot recover a typed payload from noisy
text without a consumer either duplicating JSON lexical rules or repeatedly
copying and reparsing substrings. It also reparses the complete JSON input on
every `jsonvalid`, `jsontype`, `jsonget`, `jsoncount`, or `jsonmembers` call.

Adrian has directed that CRI-09 should consider and implement the final
production JSON surface rather than land a narrow helper that CRI-13 would
replace. This changes the dependency order: the core parse-once portion of
CRI-13 must be selected now, while packed numeric serialization remains a
separate CRI-13 decision.

The decision in this packet is only the core JSON document, traversal, typed
access, and noisy-container extraction surface. It does not choose a schema
language, repair/coercion policy, annotation/reflection model, public C ABI,
RXAS/RXBIN change, or packed numeric wire format.

## Current minimized workload

Maintained source:

```text
lib/rxfnsb/tests_functional/ts_rxjson_noisy_contract.crexx
```

Build and four-cell execution:

```sh
cmake --build /tmp/crexx-cri07-clean.bZW30U/build \
  --target ts_rxjson_noisy_contract_noopt_artifact \
           ts_rxjson_noisy_contract_opt_artifact --parallel 10

ctest --test-dir /tmp/crexx-cri07-clean.bZW30U/build \
  -R '^ts_rxjson_noisy_contract(_rxbvm)?_(noopt|opt)$' \
  --output-on-failure
```

The test covers a complete object, leading/trailing Unicode prose, fenced JSON,
missing and extra fields, explicit null, Unicode field values, a non-integer
number, truncation, malformed structure, two valid but semantically ambiguous
objects, and application-local structured rejection text. Optimized and
non-optimized images pass on both `rxvm` and `rxbvm`.

The public-API-only workaround tries every plausible object/array substring.
For the retained 488-character adversarial input, every cell reports exactly:

```text
RXJSON_NOISY_WORKAROUND input_characters=488 attempts=4161 parsed_characters=1110760
PASS: rxjson noisy contract
```

This is 4,161 complete parser invocations and 1,110,760 characters copied into
candidates to recover one 40-character object. Raw logs and hashes:

```text
135db21161a2a04040edc579af0d37c0fb2f3f39a1d5212b854ed21c97563f72  cri09-baseline-probe-build-final.log
d2ff92017f9b552ee9a4f2867608d3a6696929f026999d899aa6effd08b0cf48  cri09-baseline-probe-crossvm-final.log
33e156603614b9a657f144732da730939b3febb2d4ce175006f39df340b4a5c4  cri09-baseline-probe-raw-final.log
```

The files are retained under `/tmp/crexx-integration-ledger.mZ5jyp/`.

## Root cause and existing capability

`rxjson` has one strict recursive parser, but its public entry points require a
complete standalone string. The parser does not expose a consumed boundary,
structural index, node identity, or retained document. Field validation is
possible today by composing:

- `jsontype()` for required/missing/null/type distinctions;
- `jsonget()` for decoded strings and scalar text;
- `jsonmembers()` for extra-field policy; and
- `datatype()` plus explicit range checks for application numeric types.

That composition is semantically adequate and is retained in the generic test.
It is not an efficient extraction or repeated-traversal surface because each
call reparses the source and noisy extraction has no parser cursor.

The read-only `crexx-rag` incubation proves that a Level-B immutable indexed
class is feasible. Its 4,394-byte payload samples built a 20,875-byte private
index in 215--374 microseconds, while thirty indexed deep gets took 167--224
microseconds versus 13,378--14,737 microseconds for thirty legacy deep gets in
the retained final samples. Those historical numbers are evidence of
feasibility, not a current CREXX baseline or an acceptance verdict; CRI-13 must
reproduce them afresh after a surface is approved.

A discarded narrow `jsonspan` prototype also exposed a correctness hazard: the
parser records UTF-8 byte offsets while public `pos` and `substr` use character
positions. The first four-cell run returned a boundary three characters too
large for `Gràdh 中`. The prototype has been removed from the production
surface; its failing log remains
`/tmp/crexx-integration-ledger.mZ5jyp/cri09-jsonspan-crossvm-focused.log`.

## Alternatives

### Option A — retain the current functions and application-local adapters

Keep the current public module unchanged. Document the strict-complete-input
contract and retain the generic workaround/validator as an example.

- Correctness: strict JSON remains intact; every extraction and typed-contract
  policy is visible in application code.
- Compatibility/ABI: no change.
- Time: the retained 488-character case performs 4,161 full parses over
  1,110,760 copied candidate characters; every field check reparses again.
- Memory: no retained document, but repeated transient candidate strings and
  parser arrays.
- Maintenance: every consumer must reproduce framing, ambiguity, diagnostics,
  and numeric range policy.
- Platform: portable Level B on both VMs.

This is safe but does not meet Adrian's direction to establish the production
surface and would force CRI-13 to revisit the same boundary.

### Option B — one `rxjson` module with an immutable indexed document

Preserve every existing functional selector and add one coherent Level-B class
plus a noisy-container factory procedure. Replace the two-parser prototype
shape with one strict parser/index implementation; compatibility functions use
an ephemeral document, while repeated consumers retain one document.

Proposed public surface:

```text
.jsondocument(json)

status:
  valid() error_code() error_position() error()
  source() source_bytes() index_bytes() node_count()

path compatibility:
  type(path) get(path) count(path) members(path, names[])

node traversal:
  root() find(path) member(parent, name) element(parent, index)
  node_type(node) node_get(node) node_count(node)
  children(node, nodes[]) node_name(node)

typed access, returning 0 on success and a negative status on invalid node,
wrong JSON type, non-integral number, or numeric range failure:
  node_string(node, value) node_int(node, value)
  node_float(node, value) node_boolean(node, value)

jsonscancontainer(text, from, document, start, after)
```

`jsonscancontainer` finds the first complete valid object or array at or after a
one-based character position, returns an already indexed document owning only
that exact JSON slice, and returns one-based character positions in the
original text. It does not treat Markdown fences specially, select a semantic
contract, repair JSON, coerce fields, or scan scalar JSON embedded in prose.
The caller validates the returned document and can continue from `after` when
an earlier valid container is incidental.

Node IDs are positive, document-local, ephemeral integers: `0` means missing
and negative method status means invalid usage. They cannot be serialized,
compared across documents, or used after the owning document is gone. The
document owns immutable copies of its source slice and private index. The index
layout is explicitly not public or serialized and may evolve without a format
change.

Parse diagnostics have stable codes plus a one-based character position and a
human-readable message. Typed getters never repair or clamp; application code
chooses defaults, enum policy, optionality, and diagnostics. Arbitrary object
keys use `member(parent, name)`, so the legacy dot/bracket path limitation does
not constrain the final traversal surface.

- Correctness: strict JSON, explicit missing/null/type/range distinctions,
  first-class arbitrary-key traversal, no partial document after failure.
- Compatibility: all current functions and path behavior remain source
  compatible; duplicate-key first-match behavior remains documented for the
  compatibility path. The additions create no C ABI or RXAS/RXBIN change.
- Time: one structural parse and index build, then indexed traversal. Noisy
  extraction performs a framing scan plus one strict parse for the returned
  document, with no candidate substring parse loop.
- Memory: immutable source slice plus a private index proportional to nodes and
  decoded key bytes. Consumers opt into that retained memory by constructing a
  document; one-shot functions release their ephemeral document on return.
- Maintenance: one parser and one unquote implementation. No provider or
  application vocabulary.
- Platform: pure Level B, optimized/non-optimized, both VMs. The first
  production implementation is governed performance work and must stop at the
  mandatory first ordinary Release verdict.

### Option C — promote the incubation unchanged as a second module

Copy the read-only `json_document` module, including path-only access and its
`F32V`/`I64V` packed classes, while retaining the original `rxjson` parser.

- Correctness: the experiment covers strict parsing, Unicode, missing/null,
  repeated traversal, and explicit numeric projections.
- Compatibility: additive, but creates two public parsers and two modules.
- Time/memory: indexed reuse is promising, but one-shot and repeated APIs have
  independent implementations and duplicate maintenance.
- Serialized format: it silently adopts a new 16-byte packed vector envelope,
  little-endian representation, flags, and codec version.
- Maintenance: legacy path limitations remain; parser and Unicode fixes must be
  made twice.

This is the fastest code import but violates the instruction not to select a
serialized format silently and would make the incubation rather than the
product contract authoritative.

### Option D — native/opaque JSON handle

Add a C/C++ parser and expose an opaque runtime handle to Level B.

- Potentially lowers parse cost, but selects a C ABI, allocation/lifetime,
  plugin/package, error, thread, and cross-platform contract.
- Adds native ownership and sanitizer obligations where the Level-B experiment
  already proves functional feasibility.
- Does not improve source ergonomics enough to justify the architecture cost
  before a pure Level-B product surface is measured.

## Recommendation

Approve **Option B** as the final core JSON surface.

It preserves the established functional API, makes parse-once traversal a
first-class opt-in rather than a competing module, solves arbitrary-key access
without inventing path escaping, provides strict typed extraction with explicit
status, and gives noisy text a deterministic container boundary without
embedding model/provider or repair policy. Its private index leaves room for
CRI-13 numeric materialization without committing to a wire format now.

Reject Option A because it knowingly preserves the reproduced copy/reparse
explosion. Reject Option C because it duplicates the parser and silently
selects packed serialization. Defer Option D unless the governed Level-B
Release evidence shows a material ceiling that cannot be addressed portably.

## Deliberately separate later decisions

The following must not be inferred from approval of Option B:

1. CRI-13 will separately compare ordinary typed arrays with explicit packed
   `.binary` projections. Any public envelope, byte order, width, flags, or
   inference rule requires its own exact approval.
2. CRI-14 will separately decide whether a declarative schema/contract surface
   is warranted. CRI-09 typed access supports application-local validation but
   does not create schema syntax, annotations, reflection, or a wire format.
3. Tolerant repair remains outside strict JSON. Missing optional fields,
   defaults, clamping, enums, and repair diagnostics are caller policies unless
   a later generic validation API is explicitly approved.

## Smallest exact decision

Adrian should decide only this:

> Approve CRI-09 JSON surface Option B: preserve the current `rxjson`
> functions, add the immutable indexed `.jsondocument`, document-local node and
> strict typed-access methods, and `jsonscancontainer`; keep packed numeric
> serialization, schema syntax, and repair/coercion policy out of this approval.

## Paste-ready continuation prompt

```text
Resume /Users/adrian/CLionProjects/CREXX at CRI-09 using
performance/CREXX-RAG-INTEGRATION-WORKLIST.md and
performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI09-JSON-SURFACE-DECISION.md.

I approve CRI-09 JSON surface Option B exactly as specified. Preserve every
existing rxjson selector and behavior. Implement the one-parser immutable
.jsondocument, document-local node traversal, strict typed getters, and
jsonscancontainer in generic Level B with no provider/RAG vocabulary, repair
policy, schema syntax, packed numeric serialization, public C ABI, or RXAS/RXBIN
change. Keep CRI-09 as the sole active ledger item. Run the complete correctness
matrix, then follow performance/AGENTS.md: after the first production
performance edit and minimum focused correctness, freeze the implementation,
run the smallest decisive ordinary Release comparison, report that first
verdict, and stop for my direction before broad validation. Do not touch the
read-only crexx-rag checkout.
```

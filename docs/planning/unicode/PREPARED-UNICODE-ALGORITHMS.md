# Prepared Unicode Algorithms: Mathematical Review And Design

Status: mathematical design complete for the Unicode 17.0.0 research baseline;
the experimental NFD/NFKD prepared path and fused NFC/NFKC canonical composer
are implemented and conformant on the Unicode branch. No production
integration, Release performance verdict, C-form Quick_Check fast-path result,
or Level G API decision is claimed by this document.

Branch: `unicode`

Date: 2026-08-26

## Purpose

This document first answered a narrow question before implementation began:

> Can Unicode code points be classified and prepared ahead of time so that a
> generated UTF-8 processor performs fewer runtime operations, while retaining
> an exactly equivalent canonical algorithm for every difficult case?

The answer depends on the operation.

| Operation | Mathematical verdict | Limit of the prepared path |
|---|---|---|
| NFD | Complete prepared partition is justified | An arbitrarily long trailing nonstarter run still needs general canonical ordering |
| NFKD | Same proof as NFD; complete prepared partition is justified | Compatibility expansions are larger, but the boundary problem is unchanged |
| NFC | Prepared fast regions and a fused exact fallback are justified | Independent per-code-point normalization is impossible; the dense slow path is already close to optimal |
| NFKC | Same composition proof as NFC, with compatibility expansion | More code points require expansion, so unchanged fast regions are narrower |
| Simple/full case folding | Complete per-code-point prepared transducer is justified | The gain is constant-factor because a good direct mapping is already linear |
| Extended grapheme boundaries | Exact prepared DFA is justified | A competent property DFA is already asymptotically optimal |
| Default word boundaries | Exact prepared DFA decisions plus delayed output are feasible | Some decisions have unbounded input latency; linguistic word finding needs separate tailoring |

The strongest theoretical opportunities are therefore NFD/NFKD, ordinary case
folding, and replacing interpreted segmentation rules with a generated DFA.
For NFC/NFKC, the strong opportunity is an optimal unchanged-input path and
local prepared handling around exceptional regions. Their general ordered-stream
composition fallback should remain recognizably canonical.

This is an operation-count conclusion, not a speed claim. Code size, cache
pressure, CREXX call and instruction costs, output ownership, and portable
versus host-native table layouts must be measured later.

## Scope And Non-Claims

This review covers:

- the four Unicode normalization forms from Unicode 17.0.0;
- default and Turkic, simple and full case folding;
- canonical and identifier-oriented folding interactions where normalization
  changes the contract;
- extended grapheme cluster boundaries; and
- default UAX #29 word boundaries.

It deliberately does not:

- implement any of the algorithms;
- time the current or proposed algorithms;
- change Level L syntax, Level G APIs, compiler instructions, or VM behavior;
- select portable `<at..type>` versus host-native `<packed..int>` data;
- define locale-sensitive case conversion or collation;
- claim dictionary-quality word segmentation; or
- make normalization implicit in `.string` assignment, comparison,
  concatenation, or I/O.

The pinned Unicode repertoire counts below are static audits of the retained
Unicode 17.0.0 files. They characterize possible generated data; they do not
predict real input frequency.

## Formal Framework

### Sequences And Encoding

Let `S` be the 1,112,064 Unicode scalar values: code points from U+0000 through
U+10FFFF excluding the surrogate range. Let `S*` be finite scalar sequences.

Let:

- `E : S* -> B*` be shortest-form UTF-8 encoding;
- `D : E(S*) -> S*` be strict UTF-8 decoding;
- `b` be the number of input bytes;
- `n` be the number of source scalars;
- `m` be the number of scalars after full decomposition;
- `o` be the number of output bytes;
- `q` be the longest open nonstarter sequence; and
- `g` be the number of reported segmentation boundaries.

For valid UTF-8, `D(E(x)) = x` and each scalar sequence has one byte encoding.
That uniqueness is what permits an identity action to retain or copy the
original bytes without rebuilding a scalar integer and re-encoding it.

Unicode algorithms operate on scalar sequences. An API accepting arbitrary
`.binary` bytes must separately specify whether malformed UTF-8 is rejected or
converted using a precise replacement policy. Silently passing malformed bytes
through is not Unicode normalization, folding, or segmentation. The existing
Level B `.string` contract already provides valid UTF-8 at this boundary.

### What “Prepared” Means

For this review, preparation is performed from a pinned UCD version and may
produce:

- a total scalar classification;
- full recursive decomposition or fold mappings;
- pre-encoded UTF-8 mapping results;
- leading, trailing, and internal canonical-combining-class metadata;
- normalization Quick_Check and stable-boundary metadata;
- composition operand/action identifiers;
- segmentation property tuples and DFA transitions; and
- disjoint scalar ranges lowered to disjoint UTF-8 byte languages.

Preparation must not approximate a Unicode property. It moves invariant work
from runtime to deterministic generation time.

### Operation Model

The later implementation comparison should count at least:

- bytes inspected and copied;
- scalar integers reconstructed and encoded;
- mapping and property lookups;
- recursive mapping invocations;
- canonical-combining-class comparisons and moves;
- composition-pair lookups;
- segmentation-rule or DFA transitions;
- whole-result passes and intermediate materializations; and
- allocation, growth, and ownership operations.

Every transform has an input lower bound of `Omega(b)`. A materialized transform
also has an output lower bound of `Omega(o)`; a boundary iterator has an output
lower bound of `Omega(g)`. A prepared algorithm cannot avoid reading the input.
It succeeds when it combines validation, decoding, classification, and action
selection; removes generic lookups and recursion; bulk-copies identity regions;
or avoids intermediate representations.

### UTF-8 Refinement Theorem

Suppose scalar classes `K1 ... Kr` form a disjoint, complete partition of `S`.
For each class define the byte language:

```text
Li = { E(c) | c is in Ki }
```

The `Li` are disjoint and cover exactly the well-formed UTF-8 encodings of one
scalar. For a sequence transform `A : S* -> S*`, if a generated byte scanner
recognizes each `Li` and invokes the same prepared semantic action as the scalar
algorithm, then induction over the source sequence gives:

```text
T(E(c1 ... cn)) = E(A(c1 ... cn))
```

where `A` is the proved scalar transform and `T` is the byte transducer. The
byte scanner is therefore a refinement of the scalar normalization or folding
algorithm.

Segmentation has a separate output type. The boundary after the first `k`
scalars maps to the byte offset:

```text
sum of |E(ci)| for i = 1 ... k
```

The original bytes remain unchanged. Induction over the shared segmentation
state proves the same break decision at every corresponding scalar/byte
boundary.

This is the shape documented by re2c: regular expressions are stated in code
points, while UTF-8 lowering produces an automaton over one-byte code units. It
can consume UTF-8 byte by byte without materializing a scalar for every
identity character.

For chunked input, the DFA retains its partial-prefix state and, when the source
buffer is transient, up to three already consumed prefix bytes: one lead byte
plus at most two continuations. A chunk boundary is never automatically a
Unicode algorithm boundary.

## Current CREXX NFD Execution Shape

The current proof in
[`unicode_nfd.crexx`](../../../experiments/unicode/level-l-bootstrap/unicode_nfd.crexx)
is a correct scalar-sequence baseline rather than a direct byte transducer. Its
`normalize` method:

1. gets the code-point length;
2. obtains each source scalar with sequential `strchar`;
3. recursively searches and expands canonical mappings;
4. retains the entire decomposed result as 32-bit values;
5. performs a whole-result stable insertion-ordering pass; and
6. calls `appendchar` for each result scalar.

Sequential `strchar` is not accidentally quadratic. The VM’s private string
cache advances the byte position for the next code-point index and the UTF-8
decoder then returns that scalar. The input progression is linear. It is still
different from the former re2c implementation: each scalar crosses an RXAS
instruction/cache interface, is reconstructed as an integer, performs a
mapping lookup, enters the 32-bit work buffer, and is later encoded again.

The current proof also accepts CCC value 255 while compiling its table because
its validation rejects only values above 255. Unicode 17 D104 defines CCC as
0..254. The retained input contains no 255, so this does not invalidate prior
conformance evidence. The prepared generator should reject 255 and may reserve
it as an internal sentinel.

## NFD And NFKD

### Canonical Definition

For `F` in `{D, KD}`, define:

- `dF(c)` as the complete recursive canonical decomposition for D, or complete
  recursive compatibility decomposition for KD, including algorithmic Hangul;
- extend `dF` homomorphically to strings by concatenating each result;
- `ccc(c)` as `Canonical_Combining_Class(c)`; and
- `Ord(x)` as canonical ordering: stable sorting within each maximal run of
  positive-CCC scalars.

Then:

```text
NFD(x)  = Ord(dD(x))
NFKD(x) = Ord(dKD(x))
```

The prepared single-scalar expansion is:

```text
pF(c) = Ord(dF(c))
```

The generator should calculate full closure once and assert that every `pF(c)`
is scalar-valid and canonically ordered.

### Boundary Lemma

Let `u` and `v` already be canonically ordered. Let `tail(u)` be the trailing
run of positive-CCC scalars after the last starter in `u`, and let `head(v)` be
the leading positive-CCC run before the first starter in `v`.

Then `Ord(uv)` can differ from `uv` only inside:

```text
tail(u) + head(v)
```

and the correct replacement is the stable CCC merge of those two already
ordered runs.

Proof:

1. A starter has CCC zero and cannot be either member of a D108 reorderable
   pair, so it is an ordering barrier.
2. Any reorderable pair crossing the join must lie after the final starter of
   `u` and before the first starter of `v`.
3. Both participating runs are already stably ordered internally.
4. Their stable merge is the unique sequence without a descending positive
   CCC pair while preserving source order for equal CCC values.
5. All other portions of `u` and `v` remain unchanged.

Applying the lemma inductively to `pF(c1) ... pF(cn)` proves that a prepared
streaming algorithm produces exactly the canonical D-form result for an input
of arbitrary length.

This lemma captures cross-source-character canonical-order interaction.
Per-scalar normalization followed by plain concatenation is not sufficient;
only the open boundary runs may interact, but they must interact.

### Exhaustive Prepared Classes

Classification must use `pF(c)`, not the source scalar’s CCC and not merely its
immediate UCD mapping.

| Prepared class | Full prepared expansion | Exact action |
|---|---|---|
| `COPY_STARTER` | `[c]`, CCC zero | Flush pending marks and copy the original UTF-8 span |
| `IDENTITY_MARK(k)` | `[c]`, CCC `k > 0` | Add the original bytes to the pending CCC run |
| `MAPPED_CLOSED` | Begins and ends with a starter | Flush pending marks and copy the complete prepared expansion |
| `MAPPED_OPEN` | Begins with a starter and ends in marks | Flush, copy through the last starter, retain the trailing marks |
| `MAPPED_MARKS` | Contains no starter | Stable-merge the complete prepared mark sequence |
| `MAPPED_COMPLEX` | Has leading marks followed by one or more starters | Merge the leading marks, close at the first starter, then process the prepared remainder |
| `HANGUL` | Algorithmic `L,V[,T]`, all CCC zero | Flush pending marks and emit the arithmetic expansion directly |

This is exhaustive: every nonempty finite CCC sequence either has no zero, or
has a first and last zero. `MAPPED_COMPLEX` happens to be empty in Unicode 17,
but remains necessary to make the algorithm version-independent and the proof
complete.

A static audit of all Unicode 17 scalar values gives:

| Prepared class | NFD scalars | NFKD scalars |
|---|---:|---:|
| Identity CCC-zero copy | 1,097,847 | 1,094,014 |
| Identity nonstarter | 964 | 964 |
| Mapped closed | 1,085 | 4,822 |
| Mapped open | 989 | 1,083 |
| Mapped marks only | 7 | 9 |
| Algorithmic Hangul | 11,172 | 11,172 |
| Mapped complex | 0 | 0 |
| **Total scalar values** | **1,112,064** | **1,112,064** |

All ASCII scalars are identity CCC-zero copies in both forms. The identity
CCC-zero sets compress to 429 NFD ranges and 590 NFKD ranges before lowering
those ranges to UTF-8 byte graphs.

### Why Source CCC Is Insufficient

U+0F73 TIBETAN VOWEL SIGN II has source CCC zero but canonically decomposes to
U+0F71 followed by U+0F72, both nonstarters. Treating U+0F73 as a flush boundary
because its source CCC is zero would be wrong.

Likewise:

```text
U+1E0B U+0323
 -> U+0064 U+0307(230) U+0323(220)
 -> U+0064 U+0323 U+0307
```

The second source scalar reorders into the first source scalar’s decomposition.
Prepared classification remains valid because it retains precisely the open
boundary described by the lemma.

### Pre-Expansion Cost

The Unicode 17 mapping closure is small enough that runtime recursion has no
operation-count necessity:

| Form | Source mapping records | Direct values | Full prepared values | Prepared UTF-8 bytes |
|---|---:|---:|---:|---:|
| NFD | 2,081 | 3,127 | 3,450 | 8,041 |
| NFKD | 5,914 | 8,740 | 9,193 | 18,593 |

Full closure adds 323 scalar entries for NFD and 453 for NFKD. These sizes do
not select a storage layout, but they establish that recursive expansion can be
moved entirely to generation time without a large logical mapping pool.

### Canonical-Order Engine

Canonical ordering remains the general fallback, but it need not be a
whole-result insertion pass.

A prepared hybrid can maintain only the nonstarters after the last emitted
starter:

1. While the incoming CCC is not below the pending final CCC, append it. An
   already ordered prepared mark sequence may be appended as one descriptor.
2. At the first inversion in a starter segment, redistribute the pending marks
   into stable FIFO buckets keyed by CCC. Distribute every scalar of the
   inversion-causing prepared run into its own CCC bucket as well; the run must
   not be queued as one item when it contains different CCC values.
3. Add all later marks to the corresponding bucket.
4. At the next starter or true EOF, emit occupied buckets in ascending CCC
   order, preserving FIFO order inside a CCC.

Unicode 17 uses 55 distinct positive CCC values. A generated dense rank and
occupied-bucket bit set avoid scanning 254 empty values. The exact
representation is an implementation decision.

For `k` marks and `I` inversions in a segment, insertion ordering costs
`Theta(k + I)` and has quadratic worst case. Stable buckets cost `Theta(k + C)`
where `C` is the fixed number of supported CCC values. The common monotone path
does not need to instantiate buckets at all.

Pending storage remains inherently unbounded. UAX #15’s 10,000-mark example
shows why a later lower-CCC mark can prevent serialization of the entire open
suffix. A bounded-memory profile must either report a resource limit or expose
the separately named Stream-Safe Text Process, which inserts CGJ and is not
ordinary normalization.

### D-Form Quick Check

NFD_QC and NFKD_QC have only Yes and No, never Maybe. A complete check must
also reject a descending positive CCC sequence between starters.

That check can be fused into the byte scanner. If the Level G storage contract
allows an immutable input `.string` to be returned or shared unchanged, an
already-normalized input needs no output writes. If the API promises a distinct
result buffer, the bytes must still be copied. That ownership choice belongs to
the later API and implementation review, not to this proof.

The generator should consume the normative derived Quick_Check properties
rather than independently inventing them.

### D-Form Operation Verdict

The prepared path removes runtime recursive decomposition, most mapping
searches, the complete UTF-32 work array, and the whole-result ordering pass.
Identity and pre-encoded mapped paths also avoid the generic per-scalar
`appendchar`/UTF-8 re-encode loop; arithmetic Hangul still needs specialized
byte generation unless its short expansions are stored pre-encoded. The path
can bulk-copy identity byte regions and limit ordering work to an open boundary
suffix.

For a fresh contiguous or streamed materialization it does not change the
`Omega(b + o)` lower bound; an unchanged shared result needs `Omega(b)`
inspection but no output-byte writes. In-place mutation or structural sharing
can change the write bound and is a later storage-contract question. The
algorithm also cannot bound an arbitrary nonstarter run. Nevertheless, it
removes whole semantic stages from the current proof and changes canonical
ordering from quadratic worst case to linear in the open segment. NFD and
NFKD therefore pass the mathematical prepared-path test strongly.

Once generated mappings have been exhaustively validated, the production hot
path need not retain runtime recursive decomposition as a fallback. The exact
fallback is the general stable CCC accumulator. A corrupt or wrong-version
prepared image should fail closed.

### Experimental D-Form Implementation Result

The Unicode branch now implements the proved D-form shape in
[`unicode_data.crexx`](../../../experiments/unicode/level-l-bootstrap/unicode_data.crexx)
and
[`unicode_d.crexx`](../../../experiments/unicode/level-l-bootstrap/unicode_d.crexx).
This is a correctness and operation-shape proof, not a production library.

The generator parses pinned Unicode 17.0.0 `UnicodeData.txt`, constructs
canonical and compatibility typed ASTs, calculates recursive closure, orders
each complete expansion, rejects CCC 255, and emits one deterministic portable
image. Its retained manifest is:

| Measure | NFD | NFKD |
|---|---:|---:|
| Mapping records | 2,081 | 5,914 |
| Prepared components | 3,450 | 9,193 |
| Pre-encoded UTF-8 bytes | 8,041 | 18,593 |
| Closed/open/marks/complex mappings | 1,085/989/7/0 | 4,822/1,083/9/0 |

The combined image is 1,659,772 bytes with SHA-256
`224dc0355cdf2ea4bbe51640bde15c2687d6a719196b8332f3455a2ccda5500e`.
It uses portable byte-addressed fields, a dense CCC table, 55-value CCC rank
table, per-form mapping bitsets and pages, fixed records, and pre-encoded
components.

The runtime scans UTF-8 bytes directly and validates arbitrary binary ingress.
It reconstructs non-ASCII scalars for classification and mapping lookup; it is
therefore not yet the fully lowered byte-language DFA described by the UTF-8
refinement theorem. ASCII and adjacent identity starters use coalesced raw-span
copies. The engine retains only the open nonstarter suffix, uses the common
monotone append path, and switches to a stable counting-bucket pass only after
an inversion. A long adversarial fixture proves that fallback is exercised.

The first hand-written string path used an experimental raw string-byte RXAS
surface. Its rough Release result was materially slower than the scalar NFD
baseline, so that instruction experiment was rolled back. The retained oracle
now uses the existing explicit string/binary conversions. The prepared NFD
successor instead lowers these classifications to a generated re2c/Level L
scanner; a future no-copy input view remains a separate decision that requires
evidence from the generated algorithm.

That generated successor now passes the same Unicode 17 NFD relations and
unlisted-scalar identity audit in optimized and noopt images on both VMs. Its
first rough Release screen rejects the current code shape for performance:
the re2c loop/`SELECT` method contains 3,440 states, 73 optimized `JUMPI`
dispatches, and 6,455 locals. Recycled VM activation still relinks every local,
making 20,034 short calls 84.010x/101.059x slower than the scalar baseline.
One 101,506-byte call is 8.974x/8.818x slower and is essentially tied with the
hand-prepared scanner. This distinguishes a Level B backend/register-shape
failure from the prepared partition's mathematical validity. A successor must
bound live state independently of DFA size and reduce the shared ordered-output
engine before another performance claim.

Both optimized and non-optimized images pass identically on both VM families:
20,034 `NormalizationTest.txt` rows, 200,340 NFD/NFKD relations, 1,094,978
unlisted-scalar identity checks per form, focused boundary/Hangul/Quick_Check
cases, and strict malformed UTF-8 rejection. This establishes conformance for
the contiguous-buffer D-form PoC. Arbitrary chunk splitting remains open.

## NFC And NFKC

### Canonical Definition

Let `Comp` be the Unicode Canonical Composition Algorithm, using only Primary
Composites—therefore excluding every `Full_Composition_Exclusion`—with D115
blocking and algorithmic Hangul composition. Then:

```text
NFC(x)  = Comp(NFD(x))
NFKC(x) = Comp(NFKD(x))
```

NFC and NFKC share the same ordering and composition engine. Only their
decomposition mapping differs. Compatibility composites are never produced.

The two passes in the definition are logical dependencies. They do not require
materializing the whole decomposed sequence and then scanning a second whole
buffer.

### Equivalent Fused Slow Path

The D-form engine can feed canonically ordered closed segments directly into a
composer. The composer retains:

- the last surviving starter `L` and its output position;
- the CCC of the last surviving intervening scalar, or zero when none; and
- the output tail that can still change.

`L` may be absent. Leading defective nonstarters are ordered and appended
without attempting composition until a starter appears.

For each scalar `C` emitted by the ordering stage:

1. Apply the D117 blocked-pair test against the last starter.
2. If `C` is unblocked and `(L,C)` has a Primary Composite, replace `L` with
   that composite and do not add `C`.
3. A deleted `C` does not update blocking state. The resulting composite
   remains the active starter and may compose again.
4. Otherwise append `C`, update the blocking CCC, and make it the active
   starter when its CCC is zero.

Proof is by induction over the canonically ordered stream. Before each scalar,
the retained composer state is exactly the state D117 would have after the
same prefix. The transition performs the same blocked-pair decision and
replacement. Therefore the invariant is preserved and EOF yields exactly
`Comp(Ord(dF(x)))`.

This removes a physical intermediate string and a second traversal of a whole
materialized result buffer. The composer still consumes all ordered scalars. It
does not remove either logical phase: the composer may consume a mark only
after canonical ordering has determined its position, and canonical ordering
may retain an arbitrarily long open suffix.

### Safe Prepared Composition Metadata

Composition preparation needs distinctions at both source and decomposed-stream
levels:

| Metadata/class | Safe use |
|---|---|
| Target Quick_Check=Yes | Participate in a quick-check span; it is not automatically safe to write in isolation |
| Quick_Check=Yes and CCC zero | Flush normalized output before this scalar; retain the scalar because it may compose with following input |
| Stable code point under all UAX #15 conditions | A hard normalization fence and direct-copy boundary |
| Pre-expanded decomposition descriptor | Remove recursion and repeated property access; feed every component through ordering/composition state |
| Nonstarter that is never a right composition operand | Skip the pair lookup only; it still reorders and blocks |
| Active starter with no outgoing composition | Skip pair lookups for that starter; its marks still require ordering |
| Hangul L/V/T/LV class | Perform arithmetic composition rather than a stored-pair lookup |

A stable NFC/NFKC code point is stronger than “not found in a composition
pair”. It must have CCC zero, be unchanged in isolation, never compose with a
previous or following character, and never change when another character is
added. The official Quick_Check values must be consumed from the pinned derived
data. Stable code points are not a separate supplied UCD property: they must be
derived from all five UAX #15 conditions or replaced with an explicitly
conservative sufficient subset.

### Required Counterexamples

Unicode 16 introduced composite characters that expose the precise failure in
independent per-source-character treatment. Unicode 17 contains:

```text
NFC(U+16D68) = U+16D68
NFD(U+16D68) = U+16D67 U+16D67

NFC(U+16D63 U+16D68) = U+16D6A
```

U+16D68 is NFC by itself, yet its decomposition begins with a component that
composes with the preceding source scalar. This is the newer UAX #15 section
9.2 context and exactly the cross-source-character normalization interaction
that invalidated the old NFC idea.

Other permanent proof cases are:

- **Starter-to-starter composition:** Hangul `U+1100 U+1161 U+11A8` composes
  first to an LV and then to U+AC01. Each Jamo is CCC zero; encountering a
  starter is not, by itself, permission to flush it.
- **Non-composing marks still reorder:** U+0315 has CCC 232 and occurs as
  neither operand in the Unicode 17 non-Hangul Primary Composite map, but it
  still participates in canonical ordering. The complete result is
  `NFC(U+007A U+0315 U+0301) = U+017A U+0315`.
- **Non-composing marks still block:** U+0305 COMBINING OVERLINE is not a
  composition operand, but its CCC 230 blocks the following equal-CCC U+0301
  from composing with `z`:
  `NFC(U+007A U+0305 U+0301) = U+007A U+0305 U+0301`.
- **Source CCC zero or pair-inertness is insufficient:** U+2126 OHM SIGN is a
  CCC-zero source character but normalizes canonically to U+03A9.
- **Composition is not inversion of every canonical mapping:** U+0958 has
  canonical decomposition `U+0915 U+093C` but is a Full Composition Exclusion,
  so NFC must not reconstruct it.
- **Chunking:** splitting `U+1100 | U+1161 U+11A8`, splitting before a
  lower-CCC mark, or splitting within a multibyte scalar must give the same
  result as one-shot input.

### Quick Check And Local Normalization

Unicode 17 has these non-default Quick_Check cardinalities:

| Property | No | Maybe |
|---|---:|---:|
| NFC_QC | 1,120 | 132 |
| NFKC_QC | 4,965 | 132 |

A byte/scalar pass that sees only Quick_Check=Yes values and no canonical-order
violation proves the entire input already normalized. That `Theta(b)` check is
asymptotically optimal and, with input sharing, may avoid all output writes.

For mixed input, a `spanQuickCheckYes` design may preserve stable regions and
normalize only the region around No or Maybe values. It must retain the latest
relevant starter and use the UAX #15 stable-boundary rules; it cannot simply
copy each Quick_Check=Yes scalar as soon as it appears. The Unicode 16/17
context examples are specifically protected by the derived Quick_Check data.

### Composition Data

Unicode 17 has 961 non-Hangul Primary Composite pairs in the retained
canonical mapping data. Hangul’s 11,172 syllable results are handled
arithmetically. The pinned derived properties also identify 1,120 Full
Composition Exclusion scalars.

Generated descriptors can therefore avoid a generic pair search when the
active starter has no outgoing pair or the candidate cannot be a right
operand. The actual pair-table layout remains a later portable/packed decision.

### C-Form Operation Verdict

On an arbitrary difficult changing region:

- prepared full decomposition costs `Theta(n + m)` descriptor/output work;
- stable CCC buckets can order in linear time for the fixed CCC universe;
- the ordered result has a worst-case `Omega(m)` inspection/output-information
  requirement, met by the canonical composer in `Theta(m)`; and
- a conforming streaming implementation must retain access to `Omega(q)`
  pending information in the worst case, either in an explicit buffer or by
  retaining the corresponding source storage.

The prepared design saves recursion, generic property and pair searches,
intermediate decomposed-string allocation, and a physical second whole-buffer
pass. It cannot asymptotically improve the complete difficult-region algorithm.
Once decomposition and ordering are prepared and the composer is a linear
state machine, that fallback is as fast as the canonical dependency permits in
the operation-count model.

NFC therefore passes with a deliberately asymmetric design:

- an optimal Quick_Check/stable-span path for unchanged or mostly unchanged
  text;
- prepared expansion and pair metadata for local changing regions; and
- the fused ordered-stream canonical composer for everything else.

NFKC reuses that proof exactly. Its repertoire-level Quick_Check=No set is
larger—4,965 scalars versus 1,120—so its unconditional Quick_Check=Yes scalar
set is narrower; actual encounter frequency is workload-dependent. Its fast
path is theoretically valid, and it should follow, not precede, a proved NFC
composer.

### Experimental C-Form Implementation Result

The Unicode branch now implements the fused fallback in the same prepared
engine as NFD/NFKD. `unicode_normprops.crexx` parses and audits all 1,120
normative `Full_Composition_Exclusion` scalars. Generation selects exactly 961
non-Hangul Primary Composite pairs, sorts them under 391 starters, and emits a
portable starter directory plus second/composite pair arrays. The largest
starter bucket has 19 pairs. Hangul L+V and LV+T composition remains
algorithmic.

The ordering stage emits scalars directly into composer state containing the
active starter, last surviving intervening CCC, and mutable ordered tail. A
successful composition replaces the held starter without updating the blocking
CCC; a failed CCC-zero candidate flushes the held segment and becomes the next
starter. Leading nonstarters are emitted without inventing a starter. This is
the state transition proved above and does not allocate an intermediate NFD or
NFKD result.

The portable version-2 four-form image is 1,722,756 bytes with SHA-256
`431f6893c28e6e02f50237d6c48be7d4e7973412fc7e54ad9498a03283b966f9`.
Optimized and compiler/assembler-noopt images pass identically on `rxtvm` and
`rxbvm`: 20,034 `NormalizationTest.txt` rows, 400,680 four-form relations, and
1,094,978 unlisted-scalar identity checks per form. Focused cases retain the
Unicode 16/17 cross-source counterexample, exclusions, D117 blocking, chained
composition, Hangul, malformed UTF-8, and binary-result parity.

This closes the fused-composer correctness PoC. It does not yet implement the
allocation-avoiding C-form Quick_Check/stable-region path described above;
`is_normalized` currently normalizes and compares for NFC/NFKC. That remaining
work is a performance optimization, not part of the equivalence proof.

## Case Folding

### Ordinary Folding Is A Homomorphism

For a fixed fold mode `M`, Unicode data defines one mapping:

```text
fM : S -> S*
```

and extends it per scalar:

```text
FoldM(c1 ... cn) = fM(c1) ... fM(cn)
```

Therefore:

```text
FoldM(xy) = FoldM(x) FoldM(y)
```

This is the strongest possible prepared-path property. No cross-source
ordering, lookahead, or composition state exists in ordinary folding.

The useful modes are:

- full default: CaseFolding statuses C + F;
- simple default: statuses C + S;
- full Turkic: full default with T overriding U+0049 and U+0130; and
- simple Turkic: simple default with the same T overrides.

Default Case Folding means full, non-Turkic folding. Turkic behavior must be an
explicit tailoring, not an ambient locale assumption. Unicode also warns that
the T mappings do not maintain canonical equivalence without additional
processing.

Unicode 17 `CaseFolding.txt` contains 1,481 C mappings, 104 F mappings, 31 S
mappings, and two T mappings. A full ordinary mapping expands to at most three
scalars in this version.

### Prepared Fold Actions

A compact prepared quotient is:

1. ASCII identity;
2. ASCII `A` through `Z`, with `I` split for Turkic mode;
3. non-ASCII identity;
4. one-to-one mapping with equal UTF-8 width;
5. one-to-one mapping with changed UTF-8 width;
6. full expansion to two or three scalars;
7. the two Turkic overrides; and
8. malformed/incomplete UTF-8 at a binary input boundary.

Identity actions retain the original one-to-four input bytes. Mapping actions
copy pre-encoded UTF-8. A direct UTF-8 transducer satisfies the scalar fold by
induction because each scalar action is exactly `fM(c)` and concatenation is
the fold definition.

A “simple” mapping means one output scalar, not equal byte width. It is not in
general an in-place byte-preserving transform.

### Fold Operation Verdict

Both a good scalar table implementation and a byte transducer are
`Theta(b + o)`, the lower bound. Preparation can still remove scalar integer
construction, mapping lookup, output encoding, and per-character dispatch;
identity ASCII and non-ASCII spans can be copied in blocks.

The improvement is therefore provably in semantic operation count but only by
a constant factor. A very large cross-product DFA might lose that advantage to
code and instruction-cache pressure. The safer logical design is a shared
UTF-8 byte classifier returning a small terminal action ID, plus a dedicated
ASCII block path and compact action table.

### Folding With Normalization

Ordinary folding does not preserve normalization. Unicode defines canonical
caseless matching using:

```text
NFD(Fold(NFD(X)))
```

The initial NFD has a specific finite relevance around U+0345 and characters
whose canonical decompositions contain it, but that optimization belongs to a
separately named caseless-matching profile.

`NFKC_Casefold` is also not ordinary folding with a larger table. Its derived
mapping incorporates default full folding, compatibility mapping, and deletion
of `Default_Ignorable_Code_Point` characters. The property derivation iterates
folding and compatibility normalization to its fixed result; it has no Turkic
variant. Its per-scalar mapping stage can be prepared, but Unicode R5 defines
the complete operation as:

```text
NFC(concatenate NFKC_CF(c) for each source scalar c)
```

Adjacent mapped results may reorder or compose. Those actions must feed the
proved NFC state machine rather than being emitted as independently closed
results. Identifier caseless matching is a further D147 contract comparing
`toNFKC_Casefold(NFD(X))`, not merely R5 applied to the raw identifier. This
work should wait until the NFC proof has an accepted implementation.

## Extended Grapheme Cluster Boundaries

### Finite-State Model

The complete prepared scalar class preserves this property tuple:

```text
(Grapheme_Cluster_Break, Extended_Pictographic,
 Indic_Conjunct_Break)
```

Grapheme_Cluster_Break alone is insufficient: GB11 needs
Extended_Pictographic and GB9c needs Indic_Conjunct_Break. GB12/13 also require
Regional_Indicator parity.

A sequential extended-grapheme machine needs finite state for:

- the preceding GCB class, including CR/LF/control and Hangul state;
- parity of the maximal Regional_Indicator suffix;
- an emoji suffix state with values `none`, `EP Extend*`, and
  `EP Extend* ZWJ`; and
- an Indic suffix state with values `none`, `Consonant InCB-Extend*` with no
  linker yet, and a linked GB9c suffix.

Useful invariants after each processed prefix are:

- `ri_odd` exactly when its maximal RI suffix has odd length;
- emoji state `ep_open` exactly for `EP Extend*`, and `ep_zwj` exactly for
  `EP Extend* ZWJ`; and
- Indic state `consonant_open` exactly for a consonant followed only by InCB
  Extend values, and `indic_linked` exactly for
  `Consonant (Extend|Linker)* Linker (Extend|Linker)*` under InCB.

GB11, GB12/13, and GB9c then become direct state-and-current-class tests. The
other ordered rules use the previous GCB class and current prepared tuple.

### Equivalence Proof Shape

Induction over the scalar sequence proves:

1. the three suffix invariants hold;
2. the transition’s boundary bit equals the first applicable UAX #29 rule; and
3. the transition establishes the invariants for the extended prefix.

Quotienting scalars only when their complete property tuple causes identical
transitions preserves that proof. Applying the segmentation form of the UTF-8
refinement theorem then proves a direct byte scanner. Internal boundaries occur
at scalar lead-byte offsets; for nonempty input, the final boundary occurs at
end-of-input. GB1/GB2 specify no boundaries for empty text.

UAX #29 deliberately defines these rules so they can become a deterministic
finite-state machine. It also defines extended grapheme boundaries to operate
directly on non-NFD text and yield canonically equivalent boundaries. A
normalization prepass is neither needed nor desirable.

### Grapheme Operation Verdict

The DFA costs `Theta(b + g)` and is asymptotically optimal. It eliminates
literal rule interpretation and any backward scanning of RI, Extend, ZWJ, or
Indic-linker sequences. Against an implementation that is already a good DFA,
the only remaining theoretical reductions are fusing UTF-8 validation/property
classification and adding an ASCII path. Boundary output itself cannot be
optimized away.

The generated dependency/checksum manifest must include
`DerivedCoreProperties.txt` for Indic_Conjunct_Break as well as
`GraphemeBreakProperty.txt`, `GraphemeBreakTest.txt`, and Unicode emoji
`emoji-data.txt`. `DerivedCoreProperties.txt` is already retained; the current
UCD subset does not contain the other three files.

## Default Word Boundaries

### Feasibility

Exact default UAX #29 word-boundary decisions are finite-state and feasible as
a prepared UTF-8 scanner. UAX #29 explicitly notes that word rules are harder
to compile than grapheme rules but can still become fast deterministic state
machines. A streaming emitter couples that DFA to delayed-output state and a
pending offset register.

The prepared scalar class preserves:

```text
(Word_Break, Extended_Pictographic)
```

The sequential state needs at least:

- the preceding raw class for CRLF and `ZWJ x Extended_Pictographic`;
- the last two effective non-ignored classes;
- RI parity on the effective stream;
- start/newline treatment for ignored Extend, Format, and ZWJ; and
- one pending bridge boundary for letter/quote/punctuation/number rules that
  require the next significant class.

The WB4 ignore transformation must follow UAX #29’s formal rewrite. Simply
deleting ignored characters first is wrong around start-of-text and newline
contexts, and immediate raw ZWJ state remains relevant to WB3c.

### Regularity And Latency

Place a conceptual marker at each candidate boundary. Each ordered WB rule
describes a regular language of marked strings. Rule priority is regular
language difference from earlier rules; regular languages are closed under
union, intersection, complement, and difference. RI oddness adds one parity
bit. The resulting boundary language is therefore regular and has a finite
DFA.

Some punctuation boundaries cannot be decided until the next significant
class after an arbitrarily long run of Extend/Format/ZWJ. The DFA still needs
only constant semantic state and no backtracking, while its online emitter
retains one pending offset. Boundary-reporting latency is unbounded in input
bytes. An iterator returning slices may need to retain its underlying buffer
until the pending decision resolves.

Random access is a separate surface. Forward iteration is direct; reverse or
arbitrary-position queries require reverse rules and a proved safe-start search.

### Word Operation Verdict And Scope

The compiled default DFA is `Theta(b + g)` and is near the theoretical lower
bound. It is a worthwhile generated result if the alternative is interpreted
rules or rescanning, but not a source of asymptotic improvement over an
existing competent DFA.

This proves Unicode default boundaries, not universal linguistic words. Thai,
Lao, Khmer, Myanmar, Chinese, Japanese, and other contexts may require
dictionary or locale tailoring. The Level G boundary should therefore be:

- one exact, versioned default-word-boundary DFA; and
- separately named optional tailoring hooks or profiles.

The branch must pin `WordBreakProperty.txt` and `WordBreakTest.txt` before this
implementation begins.

## Historical Wiki Algorithm Review

The recovered final wiki proposal at commit `aa801f3` contains both the right
central idea and unsafe NFC shortcuts.

| Historical idea | Mathematical disposition |
|---|---|
| Generate UTF-8 rules and hard-code pinned Unicode properties | Retain; this is the prepared byte-classifier model |
| Precompute recursive decomposition | Retain for both D and C forms |
| D-form `starter`, `single`, `starter_then_non_starter`, `complex` paths | Retain in corrected form, classified from the full prepared decomposition |
| Queue nonstarters by CCC | Retain; stable ordered append plus bucket fallback is stronger |
| Flush NFC state whenever a starter arrives | Reject; starter-to-starter and chained Hangul composition are normative |
| Define an inert starter only by absence from pair tables | Reject; identity, both-sided interaction, and context stability are also required |
| Emit an inert nonstarter directly | Reject; it can still reorder and block another composition |
| Treat starter-to-starter source blocks independently | Reject until the boundary pair and any chained composition are resolved |
| Normalize each source character independently | Reject; normalization is not homomorphic over concatenation |
| Keep a composition buffer and hard-coded pair actions | Retain with the D117 state invariant, exclusions, blocking, and Hangul arithmetic |

The D-form concept survived because canonical reordering has the boundary lemma.
The former NFD success is mathematically plausible for precisely that reason.
The former NFC concept failed where its “inert” definitions did not account for
cross-source-character normalization interactions. UAX #15’s modern
stable-code-point conditions and Quick_Check definitions should replace those
informal terms.

## Shared Prepared UTF-8 Architecture

The proof does not require one enormous generated automaton. A practical
logical architecture is:

1. a dedicated ASCII loop for the operation;
2. a generated UTF-8 byte classifier or compact byte trie for non-ASCII ranges;
3. a terminal class/action ID;
4. a small optimized `select` or action table;
5. pre-encoded mapping pools; and
6. operation-specific normalization, folding, or segmentation state.

Keeping the byte classifier separate from semantic state avoids multiplying
every Unicode byte path by every normalization or segmentation state unless
automaton minimization proves that product worthwhile. It still fuses byte
validation and property/action classification and need not construct an
integer scalar on identity paths.

The generated logical descriptor is independent of physical storage. The same
class IDs, mappings, and transitions can later be represented with portable
fixed-width fields, host-native packed integers, or a load/convert-once image.
That choice changes constant instruction and memory costs, not the proof.

For streamed operation, persistent state is the union of:

- incomplete UTF-8 DFA state;
- the open normalization nonstarter run and, for C forms, composition state;
- fold mode, which otherwise has no cross-scalar state; or
- the small segmentation DFA state and any pending boundary offset.

EOF, not input chunk exhaustion, triggers final normalization flush or final
boundary resolution.

## Implementation Order And Current Position

Work follows this dependency and proof order:

1. **Prepared NFD generator records — complete.** Generate the complete scalar partition,
   full mappings, CCC boundary metadata, UTF-8 encodings, and an audit manifest.
2. **Prepared NFD engine — contiguous-buffer proof complete.** Implement
   raw-span copy, ordered append, stable bucket fallback, and Quick_Check
   without composition. Chunked streaming state remains open.
3. **NFKD — complete.** Reuse the proved D engine with compatibility mappings;
   Release measurement remains later work.
4. **Canonical composer — next algorithmic step.** First implement a scalar composer consuming an
   already decomposed, canonically ordered stream; generate Primary Composite
   pairs minus exclusions and add arithmetic Hangul.
5. **NFC.** Pipeline the NFD engine into the composer before adding Quick_Check
   and stable-region shortcuts.
6. **NFKC.** Replace only the expansion source and reuse the same ordering and
   composer.
7. **Ordinary case folding.** Generate the four explicit modes and direct UTF-8
   actions; defer normalized caseless profiles until the NFC engine is proved.
8. **Extended grapheme boundaries.** Pin the missing inputs, build a scalar
   reference DFA, then lower its classifier to UTF-8.
9. **Default word boundaries.** Pin the missing inputs and compile the ordered
   WB rules, retaining pending-boundary and ignore-rule semantics.

This order deliberately completes the four normalization forms before using
NFC as a component of NFKC_Casefold or other matching profiles.

## Proof And Later Validation Obligations

The implementation session should retain proof-oriented generator assertions
as well as conformance tests:

1. Pin Unicode 17.0.0 inputs, URLs, checksums, and licenses.
2. Prove every scalar classifier is a disjoint, total partition of all
   1,112,064 scalar values.
3. Prove recursive mapping closure is acyclic, scalar-valid, and exactly equal
   to the pinned data; reject CCC 255.
4. Verify every prepared D expansion is internally canonically ordered and its
   leading/trailing metadata is exact.
5. Verify generated Quick_Check values against
   `DerivedNormalizationProps.txt`, not only independently derived values.
6. Verify the composition map uses only Primary Composites, honors Full
   Composition Exclusions, and implements Hangul arithmetic.
7. Verify every fold action against the selected CaseFolding status precedence
   for every scalar.
8. Prove a generated scalar segmentation DFA equivalent to the ordered UAX #29
   rules; product/symmetric-difference emptiness is an exhaustive method.
9. Verify each UTF-8 class language is exactly the encoding of its scalar set
   and no action accepts ill-formed or surrogate encodings.
10. Test every possible chunk split, including every byte split inside
    multibyte scalars and semantic splits inside mark, Hangul, RI, emoji-ZWJ,
    Indic-linker, ignored-character, and word-bridge sequences.
11. Run `NormalizationTest.txt`, `GraphemeBreakTest.txt`, and
    `WordBreakTest.txt` as independent conformance evidence.
12. Keep adversarial descending/equal CCC sequences and very long nonstarter
    runs even though ordinary corpora rarely contain them.

Conformance corpora are necessary evidence but do not replace the general
proof. The boundary lemma, composer induction, fold homomorphism, DFA
equivalence, and exhaustive generated-record checks are what extend the result
beyond the finite test rows.

## References

- [The Unicode Standard 17.0.0, Chapter 3: Conformance](https://www.unicode.org/versions/Unicode17.0.0/core-spec/chapter-3/)
- [UAX #15 revision 57: Unicode Normalization Forms](https://www.unicode.org/reports/tr15/tr15-57.html)
- [UAX #29 revision 47: Unicode Text Segmentation](https://www.unicode.org/reports/tr29/tr29-47.html)
- [UAX #44 revision 36: Unicode Character Database](https://www.unicode.org/reports/tr44/tr44-36.html)
- [Unicode 17.0.0 UnicodeData.txt](https://www.unicode.org/Public/17.0.0/ucd/UnicodeData.txt)
- [Unicode 17.0.0 DerivedNormalizationProps.txt](https://www.unicode.org/Public/17.0.0/ucd/DerivedNormalizationProps.txt)
- [Unicode 17.0.0 CaseFolding.txt](https://www.unicode.org/Public/17.0.0/ucd/CaseFolding.txt)
- [Unicode 17.0.0 NormalizationTest.txt](https://www.unicode.org/Public/17.0.0/ucd/NormalizationTest.txt)
- [re2c C/C++ manual: encoding support](https://re2c.org/manual/manual_c.html#encoding-support)

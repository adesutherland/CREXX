# Unicode Algorithm Appendix

This appendix describes the mathematical and execution shape of cREXX's
prepared Unicode 17.0.0 algorithms. It covers the four-form normalization
engine, full default case mapping, ordinary case folding, default extended
grapheme clusters, and typed byte/text codecs. All are public through
`rxunicode`; their private executors, binary methods, table generators, and
numeric selectors are not.

The user-facing contract is in [Unicode text
services](../crexx_library_reference/unicode.md). This appendix explains how
the implementation reaches that contract and why several plausible shortcuts
are rejected.

## Common model

Let `S` be the Unicode scalar values: U+0000 through U+10FFFF, excluding the
surrogate range. `S*` is the set of finite scalar sequences. Let:

- `E : S* -> B*` be shortest-form UTF-8 encoding;
- `D : E(S*) -> S*` be strict UTF-8 decoding;
- `n` be the number of input scalars;
- `b` be the number of input bytes; and
- `o` be the number of output bytes.

For valid UTF-8, `D(E(x)) = x`. A cREXX `.string` already establishes that
validity boundary. The Unicode algorithms therefore consume scalars from a
`.string`; they do not accept arbitrary `.binary` and silently reinterpret it.

Every materialized transform must inspect the input and produce the output, so
its lower bound is `Omega(b + o)`. A boundary scanner must inspect the input
and report its boundaries, giving `Omega(b + g)` for `g` reported boundaries.
Preparation cannot remove those bounds. It removes invariant work from the
runtime path.

## Prepared data

The build reads pinned Unicode Character Database and legacy encoding mapping
files and converts them into immutable, versioned binary images. Preparation
may calculate:

- a total scalar-to-action classification;
- recursive decomposition closure;
- complete case-fold mappings;
- canonical combining classes and normalization Quick_Check values;
- canonical composition pairs and exclusions;
- grapheme-break, extended-pictographic, and Indic-conjunct properties; and
- compact descriptors and mapping spans used by the runtime; and
- dense decode maps and page-bounded reverse encoding indexes.

The generator verifies the Unicode version, retained input checksums, record
counts, scalar validity, table layout, and algorithm-specific invariants. A
generated runtime checks its image header before use.

Each generated image is one module-scoped compiler constant. `rxc` shares the
source payload and emits one named binary constant; `rxas` and `rxlink` retain
one constant-pool item. Runtime code reads that constant directly. It is not
copied into every normalizer, folder, view, or call.

## Codepoint execution path

The product executors use the RXVM string path:

1. obtain the input's codepoint count;
2. read each scalar in order with the VM codepoint operation;
3. use the scalar to select a prepared descriptor or property byte;
4. update the operation's small semantic state; and
5. append result scalars or record a codepoint boundary.

In generated RXAS this is built around `STRLEN`, `STRCHAR`, direct prepared
binary reads such as `BGETU8` or `BGETU32`, and `APPENDCHAR` for transformed
output. Sequential `STRCHAR` uses the VM's advancing string cache, so the scan
is linear rather than repeatedly decoding from byte zero.

This path deliberately avoids three intermediate representations:

- no `.string`-to-`.binary` input copy;
- no separate re2c UTF-8 decoder inside the Rexx algorithm; and
- no complete UTF-32 copy of the input.

The VM remains the authority for decoding its `.string` representation. The
prepared table classifies the resulting scalar; it does not duplicate the
runtime's text representation rules.

## Normalization

Unicode normalization first decomposes characters and canonically orders
combining marks. Composed forms then apply canonical composition.

### Decomposed forms

For `F` equal to `D` or `KD`, let:

- `dF(c)` be the complete recursive canonical (`D`) or compatibility (`KD`)
  decomposition of scalar `c`, including algorithmic Hangul decomposition;
- `ccc(c)` be the canonical combining class of `c`; and
- `Ord(x)` stably order each maximal run of positive-CCC scalars by increasing
  CCC, without moving a mark across a CCC-zero starter.

Then:

```text
NFD(x)  = Ord(dD(x))
NFKD(x) = Ord(dKD(x))
```

The build prepares the fully recursive, individually ordered result
`pF(c) = Ord(dF(c))` for every scalar that changes. Runtime recursion is
unnecessary.

Per-scalar preparation does not make normalization independently composable.
If `u` and `v` are already canonically ordered, their concatenation can still
need reordering where the trailing nonstarters of `u` meet the leading
nonstarters of `v`. No other part can interact across the join because a
CCC-zero starter is an ordering barrier. The runtime therefore retains the
open trailing mark run and stably merges the next prepared leading run into
it.

In the usual monotone case, marks are appended. At the first CCC inversion,
the engine uses stable buckets for the finite set of positive combining
classes. Equal-CCC marks retain source order. At the next starter or end of
input, occupied buckets are emitted in ascending CCC order. The work is linear
in the number of decomposed scalars for the fixed Unicode CCC universe.

Hangul syllables are decomposed arithmetically into `L`, `V`, and optional `T`
Jamo. This avoids storing the regular Hangul mapping range.

Compatibility decomposition may remove formatting or semantic distinctions.
NFKD and NFKC are therefore explicit transformations, never an assignment or
comparison side effect.

### Composed forms

NFC and NFKC use the same decomposition and ordering stream, then perform
canonical composition:

```text
NFC(x)  = Compose(NFD(x))
NFKC(x) = Compose(NFKD(x))
```

The implementation fuses composition with the ordered stream rather than
allocating a complete NFD or NFKD intermediate. It retains:

- the current starter;
- the last surviving intervening combining class; and
- the ordered marks not consumed by composition.

For each candidate, the composer applies the Unicode blocking rule. A prepared
primary-composition pair may replace the held starter only when no intervening
mark blocks it. Full Composition Exclusions are never reconstructed. A
successful composition does not incorrectly change the blocking class. A
failed CCC-zero candidate closes the current segment and becomes the next
starter. Leading nonstarters remain leading nonstarters.

Hangul `L + V` and `LV + T` composition is arithmetic and can chain. This is
why “flush whenever a starter arrives” is incorrect: two CCC-zero Jamo may
compose.

Composition cannot normalize each source scalar independently. A scalar can
be NFC by itself yet participate in a composition when it follows another
source scalar. The canonical ordered-stream state is the authority for these
cross-source interactions.

### Predicates and certificates

`isNFD` and `isNFKD` can check prepared decomposition identity and canonical
order without building an output value.

`isNFC` and `isNFKC` use the official Quick_Check properties:

1. `No` proves the string is not in the requested form.
2. An ordered scan containing only `Yes` proves that it is in the form.
3. `Maybe` transfers to the exact allocation-free composition check.

The exact check is required because `Maybe` depends on context. A predicate
must not normalize into a second string merely to compare the result.

The runtime can attach four positive whole-string certificates to a VM
value/register status word:
known NFD, NFC, NFKD, and NFKC. Absence means unknown, not false. Exact whole
string copies preserve them; any content mutation clears them; empty and known
ASCII strings may acquire all four. Because one string may satisfy several
forms, the facts are independent bits.

Certificates are optimization evidence only. The normalization algorithm
remains correct when no certificate is present. They live in the protected
language-owned register-flag band rather than the VM-internal or class-private
bands. This lets the VM perform required invalidation without exposing the
bits as application state.

A transforming call may return immediately on a matching certificate. Without
one, it performs one transformation pass. It does not first run an unconditional
whole-input predicate: that would make a changed large input pay for two scans.

## Full default case mapping

Unicode default case conversion defines full uppercase and lowercase mappings
from one scalar to a sequence of up to three scalars. Apart from contextual
rules, extending such a map to a string is concatenation, as for case folding.
Full mapping can therefore expand and cannot promise source/result positional
identity.

The prepared descriptor for scalar `c` records:

- its full default upper and lower mapping spans, when non-identity;
- whether `c` has the Unicode `Cased` property;
- whether `c` has the `Case_Ignorable` property; and
- the alternate lowercase span used by the `Final_Sigma` context.

Uppercase performs one scalar-local mapping pass. Lowercase also carries a
Boolean `preceded_cased` state. For a final-sigma candidate at position `i`, it
selects the contextual mapping exactly when a cased scalar precedes `i` after
skipping case-ignorable scalars and no cased scalar follows `i` after the same
skipping. A bounded forward scan is needed only for the candidate; the normal
path remains one descriptor read per source scalar.

The result is Unicode default, locale-neutral casing. Conditional Turkic,
Lithuanian, and other locale-sensitive rules are not silently applied. The
operation neither normalizes nor folds. Its ordinary cost is
`Theta(n + output scalars)`; final-sigma lookahead depends on the ignorable
suffix around each candidate, while the Unicode source property constraints
keep the semantic rule exact.

## Ordinary case folding

For a fixed mode `M`, Unicode case-fold data defines a scalar mapping:

```text
fM : S -> S*
```

Folding extends the mapping by concatenation:

```text
FoldM(c1 ... cn) = fM(c1) ... fM(cn)
FoldM(xy) = FoldM(x) FoldM(y)
```

Unlike normalization, ordinary folding is a homomorphism: one source scalar's
result cannot reorder or compose with another source scalar's result. The
runtime therefore needs no cross-scalar semantic state beyond the selected
mode.

The four modes are:

- full default: Unicode `C + F` records;
- simple default: `C + S` records;
- full Turkic: full default with `T` overrides; and
- simple Turkic: simple default with `T` overrides.

A dense scalar descriptor selects identity, one-scalar mapping, or a prepared
mapping span. Identity emits the input scalar. Mapping spans contain at most
three scalars in Unicode 17.0.0. Full folding may expand; simple folding maps
each input to no more than one result scalar.

Ordinary folding performs no normalization. Canonical caseless matching and
`NFKC_Casefold` are different algorithms because mapped results must feed a
normalization state machine. They must not be advertised as consequences of
`toCasefold`.

The runtime cost is `Theta(n + output scalars)`, meeting the transform's linear
lower bound. Preparation removes generic record searching and runtime mapping
construction; it does not change the asymptotic bound.

## Default extended grapheme clusters

A grapheme boundary scanner implements Unicode Standard Annex #29 revision 47
profile `UAX29-C1-1`. The prepared property for a scalar contains the tuple:

```text
(Grapheme_Cluster_Break, Extended_Pictographic,
 Indic_Conjunct_Break)
```

All three parts matter. Extended pictographic data is needed for emoji ZWJ
sequences, while Indic conjunct properties are needed for conjunct-linking
rules.

The scanner applies GB3 through GB999 in Unicode rule order. Its finite state
retains:

- the previous grapheme-break class;
- regional-indicator suffix parity;
- whether an extended-pictographic/Extend/ZWJ suffix is open; and
- whether an Indic consonant/linker suffix is open.

Those values are sufficient to decide the boundary before the next scalar.
The state is reset or advanced exactly as each selected Unicode rule requires.
No backward rescan of a long regional-indicator, Extend, ZWJ, or linker suffix
is needed.

Grapheme segmentation operates directly on the supplied text. It needs no NFD
prepass, and the public operation neither normalizes nor folds. Boundaries are
recorded as codepoint offsets because `.string` indexing is codepoint based.

The public execution shapes share the same scanner:

- direct count and bounded substring scan once and retain no complete boundary
  vector;
- one-off exact search and reverse derive the boundaries needed for that call;
  and
- `.graphemes(text)` records one packed boundary index for repeated random
  access, searching, reversal, and iteration.

The indexed snapshot intentionally owns its boundary vector. A per-call index
for direct count and bounded substring was rejected because it adds allocation
and was slower in the Release comparison. A hidden mutable or shared grapheme
cache was also rejected: its invalidation and ownership cost would be paid by
all string writers for an explicitly requested Level G service.

The scan is `Theta(n + g)`, where `g` is the number of boundaries, and is
asymptotically optimal. The retained index is `Theta(g)` space. Direct count
uses constant semantic state.

## Byte/text codecs

For each supported encoding `K`, let `EncK : S* -> B*` be its partial encoder
and `DecK : B* -> S*` its partial strict decoder. A strict round trip satisfies:

```text
DecK(EncK(x)) = x
```

for every scalar sequence `x` representable in `K`. The reverse byte round trip
is guaranteed only where the encoding mapping is one-to-one. The four retained
table-backed single-byte mappings are total and choose one reverse byte per
mapped scalar; their tests additionally establish all-256-byte round trips.

UTF-8 strict decode validates shortest form, scalar range, surrogate exclusion,
and continuation structure. Because a successful result has exactly the VM's
`.string` representation, the implementation then uses the runtime conversion
path rather than decoding valid bytes into a second scalar buffer. UTF-16 and
UTF-32 operate on explicit little- or big-endian code units, reject unpaired
surrogates and non-scalars, and append decoded scalars directly. No ambiguous
endian name is accepted.

US-ASCII and ISO-8859-1 are direct range mappings. Each table-backed
single-byte encoding prepares:

- `decodeK[0..255]`, a dense `u32` scalar table;
- `pageK[0..256]`, a `u16` boundary index into reverse records; and
- sorted `(scalar, byte)` reverse records, each eight bytes.

Decoding is one direct table read per byte. To encode scalar `c`, its high byte
selects a bounded record interval and a binary search compares only that page's
records. Non-BMP scalars fail immediately for these mappings. The complete
legacy image is one 14,536-byte constant and is never copied per call.

Strict failure is authoritative. Replacement decode partitions malformed byte
input into the encoding's maximal ill-formed subparts and appends caller text
once per part. Replacement encode appends the caller's exact target byte
sequence once per unmappable scalar. The replacement is first validated as a
non-empty complete value in `K`; this prevents a convenience policy from
injecting malformed target bytes.

A BOM is data at this layer. Encoders add none and decoders do not consume one
as protocol metadata. A higher-level format that owns a BOM convention must
implement that convention explicitly around the codec.

Whole-value codecs necessarily use `Theta(input bytes + output bytes)` time and
materialize the returned value. They keep no state across calls. Incremental
chunk remainder, finalization, cancellation, and stream positioning therefore
remain outside this baseline rather than being approximated by a hidden global
codec.

## Rejected execution shapes

The following are not interchangeable implementation details; each changes
cost, ownership, or correctness in a material way.

| Rejected shape | Reason |
| --- | --- |
| Decode the input again with a generated re2c UTF-8 scanner | `.string` is already VM-owned valid UTF-8. The generated byte-DFA route duplicated that authority and its generated method/frame shape measured far slower than sequential VM codepoint access. |
| Convert the whole input to `.binary` or UTF-32 | Adds a large input copy and an intermediate representation before useful Unicode work begins. |
| Store the prepared image in each algorithm object | Copies or retains duplicate multi-megabyte state; one compiler/linker constant is sufficient. |
| Normalize each source scalar independently | Cross-source canonical ordering and composition make that algorithm incorrect. |
| Flush composition at every CCC-zero starter | Starter-to-starter Hangul and other contextual composition remain possible. |
| Precheck every transforming call | Changed large inputs pay for an avoidable additional full scan; a trusted certificate is a cheaper proof. |
| Make ordinary equality normalize or fold | Changes the Level B language contract, hides potentially expensive work, and erases compatibility distinctions. |
| Make direct count and bounded substring build an index | Adds allocation when a streaming answer is sufficient. |
| Treat a BOM as implicit codec metadata | Makes byte/text conversion depend on an unstated protocol and prevents exact U+FEFF round trips. |
| Make replacement a string policy such as `"?"` for every target | The encoded bytes differ by encoding, notably ASCII/Windows-1252 `3F` versus IBM1047 `6F`; typed target bytes are unambiguous. |
| Attach an encoding to classic file positioning now | Variable-width character seeking, partial sequences, line endings, and task ownership need a separately designed incremental stream contract. |

## Performance evidence behind the selected shape

The retained measurements select execution architecture; they are not
cross-platform promises for application throughput.

### Generated UTF-8 DFA versus VM codepoints

The first generated NFD byte-DFA route expanded to 3,440 states, 6,455 method
locals, and 172,334 optimized RXAS lines. On the retained profiling-off Release
screen it was 84.0x to 101.1x slower than the original NFD path across 20,034
short calls and 8.8x to 9.0x slower on the 101,506-byte buffer. The separate
hand-prepared scanner was close to the generated scanner on the one-call
buffer, while the generated short-call path was another 9.7x to 10.5x slower
because every recycled activation relinked the very large declared-local
frame. This showed that an enormous generated method was not a viable product
shape; it did not show that Unicode classification itself was unsound. See the
[generated NFD rough
screen](../../../experiments/unicode/nfd-performance/evidence/2026-08-26-generated-nfd-rough.txt).

The recovered prepared-symbol executor instead used the existing VM `.string`
decoder, one dense descriptor lookup per scalar, bounded dispatch code, and
direct output. It returned to approximate parity for call-heavy rows and used
0.46x to 0.77x the canonical time on the retained 101 KB buffer samples. On
812 KB and 6.50 MB buffers its median ratios were 0.45x to 0.55x. Host load made
the exact percentages directional, but it could not explain the disappearance
of the earlier order-of-magnitude loss. See the [prepared-symbol NFD recovery
evidence](../../../experiments/unicode/nfd-performance/evidence/2026-08-27-prepared-symbol-nfd-recovery.txt).

The design conclusion is narrower than “re2c is slow.” re2c remains useful for
generators that can produce bounded runtime state. Here, `.string` was already
decoded by the VM and the generated byte-DFA multiplied code and frame costs
without removing the normalization engine's semantic work. The selected
executor lets RXVM decode each codepoint and keeps the prepared descriptor and
normalization state small.

### Common normalization family and predicates

After all four forms adopted the same codepoint/table architecture, the shared
and canonical normalization paths ranged from 0.940x to 1.103x in the final
directional rerun; the 1.103x cell was isolated to one noisy VM/row result.
The old compatibility and composition slowdown disappeared with removal of
the binary, recursive, and per-instance/shared-wrapper routes. This is why the
slowdown was classified as algorithm and routing shape rather than a cost of
RXVM UTF-8 decoding.

Quick_Check substantially improved mixed true/false NFC and NFKC predicate
rows, roughly fourfold in that run. A known-normalized large buffer containing
`Maybe`, however, paid for a complete preliminary scan and then an exact
whole-input fallback, making those cells roughly 8% to 13% slower than the
old exact-only predicate. The selected rule is therefore:

- use Quick_Check for predicates, where `No` and all-`Yes` can decide early;
- fall back exactly on `Maybe`;
- do not put the same precheck in front of every transforming call; and
- consider a stable-region/local-`Maybe` algorithm only as separately proved
  follow-on work.

The full figures, host envelope, semantic limits of the Apple control, and raw
artifact locations are in the [common normalization directional
evidence](../../../experiments/unicode/nfd-performance/evidence/2026-08-28-common-normalization-directional.txt).

The later informal certificate screen found no significant cold-path
regression. A positive matching certificate made repeated normalization on the
unchanged large buffer 26.1x to 133.8x faster and repeated predicates 15.0x to
166.1x faster. Those numbers validate the certificate mechanism's cost shape,
not a formal clean-host performance guarantee. See the [normalization
certificate
screen](../../../experiments/unicode/nfd-performance/evidence/2026-08-28-normalization-certificate-informal.txt).

### Grapheme streaming versus per-call indexing

For direct count and bounded substring on a 49,152-byte, 7,168-grapheme input,
building a complete boundary index inside every call was 1.802% to 2.953%
slower across the two VMs and two operations. The accepted split therefore
keeps direct count and bounded substring streaming while the explicit
`.graphemes` view retains an index for repeated work. The [grapheme first
Release verdict](../../../performance/evidence/2026-08-29-unicode-grapheme-first-release-verdict/README.md)
contains the balanced samples and scope limits.

## Qualification boundary

Normalization qualification covers all four forms against Unicode 17.0.0
`NormalizationTest.txt`, required cross-form relations, unlisted-scalar
identity, Hangul, combining-class ordering, exclusions, Quick_Check context,
optimized and no-opt compilation, and both VM families.

Default case mapping checks every prepared record, identity for every unlisted
scalar, expansion, the full default final-sigma context, non-BMP values, and
both case directions. Case folding checks every listed record in all four modes
and identity for every unlisted scalar. Both harnesses also parse the pinned raw
UCD files independently of the table compilers, so a build-parser defect cannot
serve as its own oracle. Grapheme qualification checks every retained Unicode
17 `GraphemeBreakTest.txt` record plus focused regional-indicator, emoji,
combining-mark, substring, search, iterator, and signal cases.

Codec qualification pins exact bytes at every Unicode encoding boundary,
round-trips all 1,112,064 scalar values through all five UTF forms, covers the
malformed lead/continuation, length, surrogate, and range classes, and checks
maximal-subpart replacement. ASCII and Latin-1 ranges are exhaustive. The four
legacy source files are independently parsed and all 1,024 mappings are checked
for decode, encode, and validity, in addition to whole-table byte round trips.

RXAS shape checks supplement semantic conformance. They require direct
codepoint iteration, one named prepared constant, direct table reads, and the
appropriate output operation; they reject input binary conversion, separate
UTF-8 decoders, table attributes, and repeated constant expansion. These shape
checks protect the performance architecture without replacing functional
tests.

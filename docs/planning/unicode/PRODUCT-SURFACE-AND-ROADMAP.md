# Level G Unicode Product Surface And Roadmap

Status: normalization design and evidence checkpoint captured;
`UNICODE-CERT-01` implemented with an informal Release screen green; compiler
constant lowering and public Level G extraction remain pending.

Branch: `unicode`

Date: 2026-08-28

## Purpose

This note preserves the product direction selected from the TUTOR compatibility
review and the prepared-table Unicode proof. It separates the intended public
Level G surface from the experimental generator, table compiler, benchmark and
bootstrap classes so that later cache work cannot accidentally redefine the
Unicode API.

The first production slice is explicit Unicode normalization. It is not an
implicit change to `.string`, comparison, assignment, identifiers, source
parsing or I/O.

## Normalization Documentation Checkpoint

This checkpoint freezes the approved normalization boundary before compiler
constant lowering and later Unicode families begin. It is a design and
evidence checkpoint, not a claim that `rxunicode` is installed, released or
available from a production library.

The following decisions are closed for the normalization slice:

- normalization is an explicit Level G `.string` service and never an implicit
  `.string` conversion or equality rule;
- the four TUTOR-compatible convenience names are `toNFD`, `toNFC`, `toNFKD`
  and `toNFKC`, with corresponding `is*` predicates;
- the reusable normalizer has one fixed form and serial, task-local scratch
  ownership; exact factory spelling remains a source-compilation gate;
- `.binary`, lexer, mapping, generator and mutable-table surfaces are not
  public normalization APIs;
- all four algorithms use RXVM codepoint operations and one immutable prepared
  Unicode 17.0.0 image; and
- the four language-owned certificates are positive whole-string facts whose
  absence means unknown, whose exact-copy/mutation rules are fixed below.

The frozen implementation passes the complete four-form Unicode 17.0.0
normalization and predicate harness under both VMs, optimized and no-opt. Its
optimized RXAS shape uses `STRCHAR`, direct prepared-table reads and
`APPENDCHAR`, with no input `.string`/`.binary` conversion and no per-normalizer
table image.

The first deliberately informal certificate screen used the profiling-off
Release product VM on battery with moderate desktop load. Every cell retained
zero Unicode mismatches. Against the earlier pre-certificate directional run,
large-buffer normalization was 2.33% to 5.83% faster and like-for-like cold
large-buffer predicates ranged from 1.72% faster to 3.79% slower. Certified
large-buffer hits were 26.1x to 133.8x faster for normalization and 15.0x to
166.1x faster for predicates. This rejects an obvious material regression but
is not a formal clean-host or dual-VM performance verdict. Exact figures and
scope are retained in
`experiments/unicode/nfd-performance/evidence/2026-08-28-normalization-certificate-informal.txt`.

The slice remains unclosed as a product until the compiler owns named constant
reuse, the experimental sealer is removed, sources/data move out of
`experiments/`, the private implementation and public facade are extracted,
and proportional integration, packaging and sanitizer qualification pass.

## Language Boundary

- Level B `.string` remains valid UTF-8 with codepoint-based operations.
- Level B `.binary` remains arbitrary bytes.
- Level G owns explicit normalization, full case folding, Unicode properties,
  collation and segmentation.
- The first product slice covers NFC, NFD, NFKC and NFKD only.
- Compatibility normalization must be documented as potentially removing
  distinctions.
- Case folding, codecs, grapheme boundaries, properties, security profiles and
  collation are later independently approved surfaces under the same
  `rxunicode` namespace.
- No operation silently normalizes its input or changes ordinary string
  equality.

## Proposed Public Surface

The TUTOR-compatible convenience procedures are:

```rexx
import rxunicode

nfd  = rxunicode..toNFD(text)
nfc  = rxunicode..toNFC(text)
nfkd = rxunicode..toNFKD(text)
nfkc = rxunicode..toNFKC(text)

if rxunicode..isNFD(text) then ...
if rxunicode..isNFC(text) then ...
if rxunicode..isNFKD(text) then ...
if rxunicode..isNFKC(text) then ...
```

Each `to*` procedure accepts and returns `.string`. Each `is*` procedure
accepts `.string` and returns `.boolean`.

A reusable public normalizer class complements the convenience procedures. A
normalizer has one fixed form selected by a named `nfd`, `nfc`, `nfkd` or
`nfkc` factory and provides:

- `normalize(.string) returns .string`;
- `isNormalized(.string) returns .boolean`;
- `form() returns .string`; and
- `version() returns .string`.

One normalizer instance may reuse its scratch storage across serial calls by
one task. It is not concurrently callable. The convenience procedures use
isolated per-call state. A caller that needs parallel normalization creates one
normalizer per task.

The product surface does not expose:

- `.binary` normalization overloads or implicit binary decoding;
- the experimental `lexer_*` or mapping hooks;
- table compiler, generator or sealing classes;
- numeric form selectors such as `normalize(text, 2)`;
- a stringly typed `normalize(text, "NFC")`; or
- mutable access to the prepared data image.

Malformed bytes do not enter through `.string`. Corrupt generated data or an
internal invariant failure raises `FAILURE`; invalid public arguments raise the
ordinary typed/argument signal selected by the final Level G API review.

## Algorithm Contract

All four forms use the prepared Unicode 17.0.0 data and the RXVM `.string`
codepoint path:

- iterate the input with codepoint operations supplied by RXVM;
- do not copy the input through `.binary`;
- do not use a separate re2c UTF-8 decoder or materialize a UTF-32 input array;
- read the shared prepared table directly;
- emit the result as `.string`; and
- retain one exact canonical algorithm as authority for every difficult case.

NFD and NFKD use prepared recursive decomposition plus stable canonical
combining-class ordering. NFC and NFKC feed the same decomposed/ordered stream
directly into the canonical composer, including Hangul, composition exclusions
and blocking.

NFD/NFKD predicates use their exact decomposition/order checks. NFC/NFKC
predicates use official Quick_Check data: `No` rejects immediately, an ordered
all-`Yes` scan accepts, and the first `Maybe` transfers immediately to the
exact allocation-free predicate. The product path must not scan the rest of a
large buffer before taking an inevitable exact fallback.

Transforming `to*` calls do not automatically run a separate predicate pass.
They may first use a trustworthy content certificate, if the separately
approved cache architecture provides one, and otherwise perform one prepared
transformation. A size heuristic or precheck is admitted only by comparative
Release evidence.

## Prepared Data And Build Boundary

- Unicode data is pinned to 17.0.0 with upstream URL, licence and checksum.
- Generation is deterministic and fails closed on unexpected version or input
  shape.
- The shipped normalization runtime reads exactly one linked
  `.const UNICODE_DATA` item. The current prepared image is 11,237,664 bytes.
- No normalizer object owns or copies the image.
- The generator and UCD parser are build tools, not public runtime classes.
- Product sources must not depend permanently on an `experiments/` path.
- The current textual RXAS sealing workaround is not the preferred product
  boundary. A focused compiler-lowering repair should emit one named binary
  constant and make all uses reference that pool item. If that repair is not
  bounded, its replacement requires a separate approval rather than silently
  productising the experimental sealer.

The initial product retains the portable fixed-width table layout. The
host-native packed and load/convert-once layouts remain measured alternatives;
the existing directional evidence does not select their production use.

## Cache Boundary

Normalization certificates describe `.string` contents. They are not
VM-internal UTF-8 bookkeeping, compiler call flags or arbitrary class-local
bits.

The dormant `RXFLAG_VM_NORMAL_*` definitions are rejected as an ownership
decision. The selected `UNICODE-CERT-01` design puts four positive certificates
in a protected language sub-band beside, but logically separate from, the two
compiler call-ABI bits. The class/library flag band remains free for each class
to interpret independently.

The exact selected layout and mutation/copy contract are controlled by
`performance/UNICODE-CERT-01-WORKLIST.md`. Absence of a certificate means
unknown, never false; the algorithms remain correct without cache state; and
cached state is an optimization, not part of the visible result contract.
Numeric conversion provenance/context caching remains separately controlled by
`performance/VALUE-CACHE-01-WORKLIST.md` and is not selected.

Four independent bits are used because one string may satisfy several forms
simultaneously. A separate `NORMAL_KNOWN` bit is unnecessary. Exact
whole-string copies preserve certificates; content mutation clears them; known
ASCII and empty-string production certifies all four forms without scanning.

## User And AI Foundation

The product slice includes:

- RexxDoc for every public procedure, factory and method, including `@param`,
  `@return`, signals, compatibility warnings and examples;
- a user guide explaining when each form is appropriate and when normalization
  is intentionally absent;
- `docs/ai-context/CREXX_UNICODE.md` with the source map, algorithm invariants,
  data provenance, generation command, cache rules and failure modes;
- executable examples using the exact public names;
- a contract test that every public name is documented and exercised; and
- a version query returning the Unicode data version used by the runtime.

## Production Roadmap

The selected closure sequence is:

1. capture this normalization documentation and evidence checkpoint;
2. make `rxc` emit one module-local named binary constant for a source-level
   constant, remove the Unicode RXAS sealing pass, and close normalization
   product extraction;
3. productize default full case folding, with Turkic and simple modes explicit;
4. productize UAX #29 extended-grapheme boundaries, then build explicitly named
   count, slicing and iteration operations on that one boundary engine; and
5. complete final cross-family user, AI, packaging and qualification material.

The RXAS assembler already deduplicates final binary constant-pool values. Step
2 fixes compiler output ownership and textual RXAS scaling: `rxc`, not a
Unicode-specific postprocessor, must emit the existing module-scoped constant
alias once and reference it at every use. It must not replace the direct
read-only operand with a mutable global register or runtime table copy.

The detailed normalization steps are:

1. Preserve the current dirty experimental worktree in recoverable focused
   commits without resetting or discarding any existing evidence.
2. Implement and qualify the selected `UNICODE-CERT-01` four-bit certificate
   contract. Numeric caching remains independent and must not delay the Unicode
   product path.
3. Freeze the eight convenience procedures, reusable normalizer class,
   failure contract and serial-instance ownership.
4. Move pinned inputs and deterministic generation to a product-owned data/tool
   location.
5. Add a minimal named-binary-constant reproducer and repair the one-constant
   compiler lowering, or stop for approval of a bounded alternative.
6. Extract the four-form runtime into a private Level B implementation module
   and remove binary, lexer and generator surfaces from the shipped API.
7. Add the public Level G facade, RexxDoc, user guide and AI context.
8. Pass deterministic generation, complete Unicode 17.0.0 normalization and
   predicate conformance, unlisted-scalar identity, focused Hangul/CCC/Maybe
   regressions, optimized/no-opt compilation and both VMs.
9. Audit generated RXAS for one shared constant, direct codepoint/table access,
   bounded live state and no predicate output construction.
10. Prepare the ordinary profiling-off Release product and stop until the host
    is explicitly clear. The smallest decisive canonical/Apple/current-product
    performance comparison is reported before broad closeout.
11. After an accepted first Release verdict, complete proportional Debug,
    sanitizer, install/package, documentation and integration qualification.

## Approval Gates Still Open

- exact public spelling of the reusable class factories after source-level
  compilation proof;
- canonical production location of pinned UCD inputs;
- compiler constant lowering versus a separately governed build-time fallback;
- any size-based normalization precheck heuristic; and
- every post-normalization Level G Unicode family.

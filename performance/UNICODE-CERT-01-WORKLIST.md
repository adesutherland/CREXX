# UNICODE-CERT-01: intrinsic string normalization certificates

Status: implementation frozen and Release-prepared; informal product-VM screen
green, formal clean-host verdict not run

Branch: `unicode`

Date: 2026-08-28

## Objective

Add four positive, content-derived certificates to ordinary UTF-8 `.string`
values:

- NFC;
- NFD;
- NFKC; and
- NFKD.

The certificates make an already-proved normalization result reusable without
rescanning or transforming the bytes. Absence means unknown, never false.
They do not change string equality, conversion, I/O or normalization semantics.

Numeric conversion provenance and the worker-local context-ID candidate remain
under `VALUE-CACHE-01` and are not part of this implementation.

## Design Selection

### C0: no certificates

Retain the exact algorithms and recheck every input. This is the correctness
baseline and remains the fallback whenever no certificate is present. It is
rejected as the selected performance design because repeated normalization of
unchanged large strings must repeat at least a codepoint scan.

### C1: language-owned certificates in the existing status word — selected

Split the current `0x0000FF00` compiler byte logically:

- `0x00000300`: existing compiler call-ABI flags;
- `0x00003C00`: NFC/NFD/NFKC/NFKD certificates; and
- `0x0000C000`: reserved language metadata for a separately selected future
  design.

The physical status word and `value` layout do not change. The four
certificates are read-only through Level B flag views. Trusted RXAS may assert
them using the existing explicit status instructions; ordinary compiler call
setup must preserve them. No new RXBIN opcode or value field is introduced.

The VM clears the four bits once at the shared logical string-mutation
boundary. Same-length in-place writers that do not pass through that boundary
clear them explicitly. Exact whole-string copy and owner-local move preserve
them. External materialization is conservative: known ASCII sets all four;
otherwise it begins unknown unless the transport independently carries a
trusted certificate.

Known ASCII producers set all four certificates in the same completion helper
that already publishes UTF-8 validity and codepoint count. The empty UTF-8
string is likewise certified for all four forms.

### C2: VM-private normalization bits — rejected

The low byte is for transparent VM implementation state such as UTF-8 validity
and object lifecycle. Unicode normalization is a language-level semantic fact
consumed by Level G algorithms. Keeping the dormant `RXFLAG_VM_NORMAL_*`
allocation would conflate those ownership layers and make the language cache
depend on VM-private write rules.

### C3: class/library flags — rejected

The class band is deliberately available to each class for its own meaning.
An ordinary `.string` certificate must survive outside one wrapper class, so a
globally interpreted allocation in that free class band is unsafe.

### C4: per-value generation, hash or shared version — rejected

Generations still require a complete mutation audit, add writes and state, and
can wrap unless they saturate. Hash validation makes a large-buffer cache hit
linear in the buffer size. Shared atomics solve no current problem because
mutable VM values are worker-owned and cross-worker materialization copies into
receiver-owned storage.

## Correctness Contract

1. A set bit proves the current complete logical UTF-8 byte span is in that
   form.
2. Absence carries no negative meaning.
3. Any operation that can change the logical string bytes clears all four
   before or as it publishes the change.
4. Exact whole-string byte copies may copy all four bits.
5. String slices, concatenation, case mapping, trimming and other transformed
   results begin unknown unless the operation has an independent proof.
6. ASCII and empty UTF-8 results set all four bits without scanning.
7. Numeric, decimal, binary, object and attribute mutation does not by itself
   invalidate certificates for an unchanged string component.
8. Compiler argument/status setup preserves the language-owned sub-band.
9. Unmasked legacy public-flag branches ignore the language sub-band; explicit
   masked reads may inspect it.
10. `NUTF8` builds do not assert Unicode normalization certificates.

## Implemented Mutation And Copy Audit

The frozen production edit covers:

- `rxvm_value_set_string_length_known()` and its checked counterpart;
- raw and validated string setters;
- append, concatenate, slice, truncate and codepoint append;
- in-place case conversion and direct string handlers;
- text file/socket/RXVML/RXPA ingress;
- `copy_value()`, `copy_string_value()`, `set_value_string()` and stem string
  copy;
- owner-local `move_value()`;
- ASCII integer, float, decimal and hexadecimal formatting; and
- native post-call validation, which must conservatively remove an untrusted
  non-ASCII certificate after a plugin may have mutated bytes directly.

Cross-worker channel documents currently carry string bytes, not status-word
metadata. Receiver materialization therefore certifies ASCII and otherwise
starts unknown. Adding certificate metadata to that versioned transport is not
required for correctness and needs separate evidence before changing the
channel format.

## Focused Qualification Gate

Before the Release preparation, the gate required the implementation to:

1. prove the bit partition and legacy public-flag behavior in RXAS;
2. prove empty/ASCII certification;
3. prove non-ASCII mutation clears every certificate;
4. prove exact string copy, whole-value copy and move preserve certificates;
5. prove concatenation, slicing and in-place case conversion fail closed;
6. prove native post-call validation cannot retain a stale non-ASCII
   certificate;
7. prove both VM modes and optimized/no-opt paths agree; and
8. inspect optimized RXAS for explicit certificate tests/assertions without a
   new conversion or copy path.

After those checks passed, the implementation was frozen and the ordinary
profiling-off Release product was prepared. The run harness retained its
AC-only default; the later explicitly requested informal screen overrode that
guard and is qualified separately below. A formal first verdict, if requested,
still compares the certified hit path with the exact no-certificate algorithm
on unchanged small and large normalized strings and includes a miss-heavy
mutation control.

## Retained Qualification Evidence

The frozen implementation passes:

- focused Debug compiler, RXAS, UTF cache, value-copy/mutation and RXPA opaque
  native-mutation tests;
- the complete Unicode 17.0.0 four-form harness under `rxtvm` and `rxbvm`,
  optimized and no-opt, with 400,680 normalization relations and 400,680 exact
  predicate checks producing byte-identical summaries;
- optimized RXAS shape checks showing `GETANDTP`/`SETORTP`, `STRCHAR` codepoint
  iteration, direct prepared-table reads and no string/binary conversion or
  table copy in `normalize` or `is_normalized`; and
- a fresh profiling-off Release preparation at
  `cmake-build-unicode-release/unicode-common-compare/prepared.k83Z3w`, whose
  manifest fingerprints the product and harness artifacts.

The benchmark's `certificates` variant mode compares `shared-cold` with
`shared-hit` on byte-identical normalized inputs. Cold inputs are
re-materialized through validated binary-to-text conversion before the timer;
hit inputs are certified before the timer. Optimized RXAS confirms that neither
reset nor conversion occurs inside the timed pass.

## Informal Release Screen

Adrian requested a quick, informal regression screen. The host was on battery
at 80%, with no thermal warning and load averages moving from
2.12/2.05/1.76 to 2.36/2.14/1.81. The run therefore bypassed the harness's
AC-only guard and is not a formal clean-host verdict.

The screen used the profiling-off Release product VM (`rxvm` resolves to
`rxbvm`), one warmup and three retained samples per cell. Suspicious row cells
were repeated with seven retained samples. All cells reported zero Unicode 17
mismatches.

Against the retained pre-certificate directional medians:

- row normalization moved from 0.13% faster to 4.94% slower;
- large-buffer normalization was 2.33% to 5.83% faster;
- row predicates moved from 17.40% faster to 1.92% slower; and
- like-for-like cold large-buffer predicates moved from 1.72% faster to 3.79%
  slower.

The more stable large-buffer cells therefore show no material regression. The
small-row normalization movements are within the noise already observed for
the non-dedicated host and do not identify an algorithmic slowdown.

On byte-identical normalized large buffers, certified hits versus deliberately
uncertified cold values improved:

- normalization by 26.1x NFD, 45.4x NFKD, 133.8x NFC and 132.2x NFKC; and
- predicates by 15.0x is-NFD, 18.9x is-NFKD, 155.2x is-NFC and 166.1x is-NFKC.

The durable figures, comparison qualifications and raw build-tree evidence
paths are recorded in
`experiments/unicode/nfd-performance/evidence/2026-08-28-normalization-certificate-informal.txt`.

This screen is sufficient for the normalization documentation checkpoint. It
does not replace a requested formal clean-host, dual-VM or cross-platform
performance qualification.

If a formal clean-host certificate panel is later requested, the prepared
command remains:

```sh
CREXX_UNICODE_COMMON_COMPARE_PREPARED_DIR="$PWD/cmake-build-unicode-release/unicode-common-compare/prepared.k83Z3w" \
CREXX_UNICODE_COMMON_COMPARE_VARIANT_MODE=certificates \
CREXX_UNICODE_COMMON_COMPARE_OPERATIONS="nfd nfkd nfc nfkc is-nfd is-nfkd is-nfc is-nfkc" \
experiments/unicode/nfd-performance/run_common_compare.sh
```

## Stop Conditions

Stop for a design decision if implementation requires:

- a new RXBIN opcode;
- a larger `value` layout;
- writable Level B access to the language flag view;
- normalization metadata in the cross-worker channel format;
- implicit normalization; or
- numeric conversion provenance/context caching.

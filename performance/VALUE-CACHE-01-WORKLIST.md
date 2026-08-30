# VALUE-CACHE-01: value-derived cache and invalidation architecture spike

Status: **numeric design and census only; no numeric production implementation
selected — normalization certificates handed off to `UNICODE-CERT-01`**

Started: 2026-08-28

## Question

Can cREXX retain expensive derived value representations and intrinsic string
facts without slowing the much more frequent mutation paths, weakening worker
ownership, or making cache correctness depend on a class interpreting another
class's flag bits?

The motivating candidates are:

- integer, float and decimal to string materialization;
- string to integer/float/decimal parsing where a material caller exists; and
- Unicode normalization-form certificates on ordinary `.string` values.

This spike records architecture options. It does not authorize a value-layout,
numeric cache, numeric-context, RXAS/RXBIN or compiler edit. The intrinsic
normalization-certificate branch of the investigation was separately approved,
implemented and qualified under `UNICODE-CERT-01`; its selected contract is no
longer governed by this numeric-cache spike.

## Current Architecture Facts

1. One VM `value` already carries independent integer, float, decimal, string,
   binary, object, reference and attribute components. `ITOS`, `FTOS` and
   `DTOS` materialize the existing string component; they do not need another
   result object.
2. Normal UTF-8 `value` is currently 176 bytes on 64-bit targets. String,
   decimal, binary and attribute payloads are worker-owned side allocations.
3. Mutable registers, globals, frames and allocations are worker-owned. Tasks
   do not share raw mutable `value` objects. Cross-worker messages copy into
   receiver-owned storage or use an explicitly immutable transferable payload.
   Same-worker references alias the same storage, so a mutation through an
   alias updates the metadata attached to that storage. No concurrent reader
   needs an atomic publication protocol.
4. Static RXAS storage/component SSA is already the primary conversion cache.
   The metadata-driven M01 derivation pass describes all 20 one-register XTOY
   conversions. It removes repeated `ITOS`, `FTOS` and `DTOS`, plus other
   total distinct-component conversions, when source storage, component,
   numeric/plugin context and signal paths are proved unchanged. Numeric-
   context writes deliberately retain the second `FTOS`/`DTOS`. Signalling
   reverse conversions such as `STOI`, `STOF` and `STOD` remain when a
   dominating successful conversion cannot be proved. This static path has no
   runtime hit/miss branch and remains the preferred first owner.
   M01 proves a repeated instance of the same derivation; it does not treat
   opposite conversions as algebraic inverses. For example, it does not erase
   `DTOS` followed by `STOD`, or reuse an `FTOS` result for `STOF`. Formatting,
   rounding, signalling and numeric/plugin context make such round-trip
   cancellation unsafe without a stronger, separately proved rule.
5. The current Level C `RexxValue` class separately uses the class/library flag
   band to describe representations current within a `RexxValue` object. That
   is valid because the object owns the interpretation and its setters replace
   the flag set. It is not a general flag allocation for arbitrary `.string`
   values.
6. Normalization certificates now occupy four protected language-owned bits
   under `UNICODE-CERT-01`; they are not VM-private state or numeric cache
   provenance. Active VM-private state covers UTF-8 validity/count and object
   initialization.
7. `finish_ascii_string_write()` already records byte length, character count
   and UTF-8 validity after numeric formatting. ASCII output is simultaneously
   NFC, NFD, NFKC and NFKD.

## Retained Performance Evidence

- PERF3-10 removed 1,400,000 repeated `ITOS` executions from canonical
  RexxCPS and improved paired median CPS by 10.38%/10.61% on the two VMs. This
  confirms that removing the conversion is valuable and that static proof can
  capture a large fraction without a runtime cache.
- PERF3-11 generalised that proof. Permanent optimized/no-opt RXAS tests prove
  that repeated `FTOS` and `DTOS` are removed, that a numeric-context write
  prevents each removal, and that seven signalling conversion families remain
  fail-closed without a dominating success-edge proof. The runtime cache census
  must therefore inspect the already-optimized residual image, not source-level
  or pre-M01 conversion counts.
- The later equal-work diagnostic still observed 1,120,006 `ITOS`, 1,660,000
  `STOD` and 2,220,000 `DTOS`. These are opportunity counts, not cache-hit
  counts.
- Explicit `STOI`/`STOF` are not currently a broad hot owner: the PERF3-03
  census found two `STOI` and 300 `STOF` in canonical RexxCPS, and one explicit
  `STOI` in each of several other workloads. The material string-to-number cost
  was implicit loose-comparison parsing, for which the accepted first-byte
  prefilter already avoids most provably impossible conversions.
- An isolated allocation-free integer parser was materially faster, but no
  current material explicit caller justified changing its compatibility
  contract. `STOI` caching therefore remains below numeric-to-string and
  normalization in the initial ranking.

## Cache Taxonomy

Different facts must not be forced into one invalidation mechanism.

### Intrinsic content certificates

Examples: valid UTF-8 and NFC/NFD/NFKC/NFKD.

These depend only on the exact string bytes. They survive an exact string copy,
are valid across numeric-context changes and may cross workers with a copied or
immutable byte sequence. Any string-content mutation invalidates them.

### Derived representation provenance

Examples: “the current string component is the formatting of this integer” or
“of this decimal under this numeric context”.

These depend on:

- the exact source component and value;
- the string component remaining unchanged;
- the effective numeric-context tuple;
- for decimal formatting, the provider/generation contract; and
- copy/alias/worker ownership.

A single `STRING_CURRENT` bit is insufficient. It cannot distinguish a source
string from int-, float- or decimal-derived text, and it cannot prove the
formatting context.

### Class-owned representations

`RexxValue` string/int/float/decimal/binary flags describe one class's coherent
logical scalar and remain class-owned. Its cached-string relationship to
mutable numeric settings needs a separate semantic oracle: the current flags
do not tag the numeric context used to produce a cached string. That may be the
intended persistent-string semantics or a stale-cache hazard; it must be tested
against the Level C contract before being reused as the general design.

## Options

### V0 — static proof only

Keep the existing assembler component/value-generation proof and always run
residual conversions.

Advantages: zero runtime check, no layout change, exact context reasoning.
Limit: cannot reuse across unproved calls/aliases or genuinely dynamic paths.

Disposition: retained baseline and first optimisation layer.

### V1 — clear-on-mutation language flags only

Give intrinsic string certificates a protected language-owned mask. String
writers clear that mask; ASCII writers can set all four normalization bits.

Advantages: very small state, no generation comparison, exact copies preserve
facts. The clear/set can normally be folded into the status-word store already
performed by string completion helpers.

Limit: it proves content properties but cannot safely identify a
context-dependent numeric representation.

Disposition: selected and implemented separately as `UNICODE-CERT-01`; retained
here as the architecture comparison, not as an open numeric-cache option.

### V2 — per-value or per-component generation counters

Increment a content generation on writes and store the generation used by each
cache.

Advantages: general dependency language and simple equality checks.

Costs: counters add writes to every mutation, need multiple component
generations for the current multi-component value, require wrap handling and
increase the hot value footprint unless packed into existing space. A single
generation invalidates unrelated components unnecessarily. A version counter
also does not remove the completeness obligation: a writer that forgets to
increment it is just as unsound as a writer that forgets to clear a validity
mask.

Disposition: not recommended as the opening candidate.

### V3 — shared or atomic generations

Use versions shared by values visible to several tasks.

The current architecture deliberately does not share mutable VM values across
workers. Adding atomics would introduce cache-line contention and a new
ownership model to solve a problem that cross-worker copy/immutable transfer
already avoids.

Disposition: rejected for the current worker model. Reopen only with an
independently approved shared-mutable-value architecture.

### V4 — worker-local memo table keyed by value contents

Memoize a pure conversion using `(operation, numeric bits or decimal content,
numeric context, provider)` rather than a mutable value pointer.

Advantages: no mutation invalidation and reuse across registers; naturally
worker-local.

Costs: lookup and replacement on every attempt, hashing/comparison for decimal
or strings, copied result bytes, bounded table policy and lifecycle cost. A
normalization key must inspect the whole string and is unlikely to beat a
content certificate.

Disposition: retain as a comparative numeric-conversion PoC, not for initial
normalization.

### V5 — hybrid intrinsic flags plus lazy numeric provenance

Keep intrinsic normalization certificates as clear-on-string-mutation bits and
add a compact numeric-to-string provenance tag only if the residual census
shows a material reuse rate.

A plausible isolated PoC is:

- split the current compiler byte into the two active compiler-ABI bits and six
  protected language bits;
- use four language bits for NFC/NFD/NFKC/NFKD;
- use the remaining two bits as `none`, `int`, `float` or `decimal` string
  provenance;
- place a 32-bit conversion-context ID in the current 64-bit alignment gap
  after the 32-bit status word, subject to compile-time layout proof on every
  supported target; and
- leave the VM-private, class/library and user bands semantically unchanged.

The frame inherits its caller's effective context ID. An effective change to
digits, fuzz, form, case, standard or relevant provider generation obtains a
new non-zero worker-local ID. Repeating the same effective settings retains the
ID. Zero means uncacheable. IDs are never reused or wrapped. On counter
saturation the worker stops issuing cacheable IDs until restart, preventing
wraparound aliasing without scanning live values.

On an `ITOS`/`FTOS`/`DTOS` attempt, a matching provenance and context ID is a
hit. A miss performs the current conversion, writes ASCII, sets all four
normalization certificates and then records provenance/context. A conservative
numeric-component write clears all numeric-string provenance; a string write
clears provenance and normalization certificates. Exact same-worker
whole-value copy may preserve both. A string-only copy preserves intrinsic
certificates but clears numeric provenance. Cross-worker materialization may
preserve intrinsic certificates for exact bytes but clears worker-local
numeric provenance and context ID.

This avoids mutation versions and atomics. It also keeps cache state in the
same worker-owned value as the materialized string. The expected unchanged
176-byte 64-bit layout is a hypothesis to prove, not a current claim.

Disposition: recommended architecture comparator after the census, not yet a
production selection.

## Mutation Rules To Prove

| Operation family | Intrinsic string certificates | Numeric-string provenance |
| --- | --- | --- |
| Exact whole-value copy/move, same worker | preserve | preserve |
| Exact string-only copy | preserve | clear |
| String append/update/case conversion/substring result | clear initially; later propagate only when proved | clear |
| Known ASCII production | set all four normalization forms | set only for a named numeric source/context |
| Integer/float/decimal write | preserve existing independent string certificate | clear conservatively |
| Numeric-context change | preserve | no scan; new context ID causes a miss |
| Cross-worker copied string | preserve if exact bytes and metadata are transported deliberately | clear |
| Immutable transferable buffer | certificate may live with immutable content | no worker-local provenance transfer |

Clearing occurs once at logical mutation completion, not once per copied byte or
codepoint. Helpers should compute the new protected state and perform one
status-word store. Normalization output clears at construction start and
certifies once after successful completion.

## Cost Model

The main risk is not the integer `AND`/`OR`; it is adding a load, branch or
larger hot value to paths that miss almost every time.

- A string completion helper already updates length/count and VM-private UTF-8
  state. Intrinsic certificate invalidation or known-ASCII certification should
  be combined with that existing status update.
- A runtime conversion cache adds at least a provenance load/mask, context-ID
  compare and branch to every residual conversion. Sequential output of a new
  number each time will miss and can regress.
- A hit can avoid `snprintf`, float formatting or decimal-provider formatting
  and reuse the existing string side allocation without a result copy.
- A side table avoids writer costs but adds lookup, hashing and result-copy
  costs. It needs a high observed reuse rate to win.
- Any extra field or widened status representation must pass value-size,
  allocator-class, RSS, code-layout and zero-hit guards on all supported
  targets. No field is admitted merely because it fits on one host.

## Measurement Plan

1. Add counts-only instrumentation in an isolated build for post-M01 residual
   `ITOS`/`FTOS`/`DTOS`/`STOI`/`STOF`/`STOD` attempts, same-source/context
   repeats, M01 rejection class, invalidations, string sizes and sites. Do not
   infer hit rate from opcode count.
2. Include canonical RexxCPS and representative output-heavy applications, plus
   miss-heavy sequential-number and repeated-same-value controls. Keep this a
   single-mechanism non-representative census until a production candidate
   exists.
3. Establish exact helper ceilings for current conversion, guaranteed hit,
   guaranteed miss and copy-from-worker-memo paths.
4. Compare the no-numeric-runtime-cache baseline with V4 and V5 in isolated
   rebuilds while retaining the selected normalization certificates unchanged.
   V2 is included only if a layout-neutral representation can be demonstrated;
   V3 is excluded by the current ownership contract.
5. Measure both VM modes, value size/layout, instruction count, code size,
   allocation/copy bytes, worker startup/teardown and miss-only guards.
6. Prove numeric-context, provider, copy/move, alias, signal and cross-worker
   semantics before timing an integrated candidate.
7. If a production edit is selected, run the mandatory smallest profiling-off
   Release verdict and stop for Adrian before broad closeout.

No numeric-cache performance run begins until the host is explicitly reported
clear.

## Current Recommendation

- Keep static compiler/assembler representation proof as the normal cREXX
  answer to repeated conversion. The working hypothesis is that M01 already
  catches most useful cases; profile the optimized residual rather than adding
  a runtime cache on source-level conversion counts.
- Retain the separately selected `UNICODE-CERT-01` grouped
  clear-on-mutation language metadata for intrinsic string facts. Do not reopen
  that decision as part of numeric-cache profiling.
- Do not add shared versions or atomic counters.
- Do not add per-component generations to every value before a measured
  residual caller proves they are needed.
- Census residual numeric-to-string reuse after M01, with `DTOS` the first
  likely material candidate but not a presumed cache win. Compare the compact
  V5 provenance/context form against a small worker-local memo table and the V0
  no-runtime-cache baseline.
- Keep `STOI` out of the first cache candidate. Its explicit current frequency
  is low, while the material implicit parsing owner already has a selected
  prefilter.

## Stop Conditions

Stop and request an architecture decision if:

- the language metadata cannot remain protected without exposing VM-private or
  arbitrary class bits;
- the context dependency cannot be represented without growing the selected
  value layout;
- miss-only overhead reaches a governed workload guard;
- cross-worker reuse would require mutable shared values or atomics;
- the census does not show a material residual hit population; or
- a cache changes conversion, normalization, numeric-context or signal
  semantics.

# Complete candidate and owner panel

## Ranked disposition

| Rank | Stable ID | Earliest safe owner | Current ceiling | Decisive cell | Disposition |
| ---: | --- | --- | --- | --- | --- |
| 1 | `PERF2-06-07-V1R01` | compiler/inliner | Richards opt: 73,307,574 copies and 582,076,729 bytes; Permute opt: 10,259,602 and 74,012,810 | Richards opt, both VMs; Permute opt secondary | **advance for Adrian selection** |
| 2 | `PERF2-06-07-V1R02` | compiler/RXAS exact nonescaping reference chain | Bounce retains millions of reference opcodes, but no isolated redundant helper ceiling | Bounce opt/no-opt | defer; mechanism attribution incomplete |
| 3 | `PERF2-06-07-V2R01` | compiler typed selection, then narrow private helper | Generic recursive copying is large, but predominantly induced by V1 full-object copies | Residual Richards/Permute after V1 | defer behind V1; do not retry reset work |
| 4 | `PERF2-06-07-V3R01` | representation setter; compiler only with proved context | correctness bug proved; general retained-cache mechanism count is zero | RexxCPS plus focused V3 matrix | correctness accepted; performance form deferred |
| 5 | `PERF2-06-07-V5R01` | exact payload owner/size class | high churn, only 1.5-1.7 MB high water, narrower V1 cause | residual Richards/RexxCPS after V1/V3 | defer; no generic pool |
| 6 | `PERF2-06-07-C2R03` | procedure/frame architecture | no materially different procedure-affine capacity mechanism proved | call-heavy residual plus Sieve | reject from current panel; architecture gate unmet |
| 7 | `PERF2-06-07-V6R01` | value/ABI architecture | `value` is 248 bytes, but current product does not prove layout as earliest owner | full supported-platform matrix | defer; Adrian architecture/ABI decision required |

## Candidate contracts

### V1R01 — inlined receiver direct placement

- **Mathematical/semantic correctness:** replace copy-in/copy-back only where
  final compiler flow proves the callee receiver and caller object denote the
  same mutable storage for the complete inlined region. The proof must cover
  receiver identity, every mutation, branches and joins, early return,
  signals/unwind, reference/native/object payloads and TRACE-visible register
  anchors. A scalar equality proof is insufficient for an aliased object.
- **Machine ceiling:** Richards opt currently performs 73,307,574 recursive
  copies/582,076,729 bytes and retires about seven billion Apple instructions
  per VM. Permute opt performs 10,259,602/74,012,810. The hand-equivalent
  ceiling is direct use of the caller receiver storage with zero full receiver
  copy-in/copy-back at the proved sites.
- **Owner/placement:** compiler inliner and final register/storage mapping,
  before RXAS. RXAS may confirm the absence of pairs but should not rediscover
  alias semantics.
- **Lifecycle/ownership:** the caller retains cleanup ownership; the inlined
  region borrows the exact receiver location and creates no second payload
  owner. Exit, failure and unwind therefore perform the existing caller cleanup
  exactly once.
- **Both VMs:** identical public stream and semantic proof; `rxvm` and `rxbvm`
  must show the same operation reduction. No runtime quickening or per-site VM
  state.
- **Fallback/failure:** retain the current copy-in/copy-back sequence for any
  escape, uncertain alias, differing storage, join, signal, TRACE or payload
  proof failure.
- **Blast radius:** internal compiler mapping only; no language, public RXAS,
  RXBIN or ABI change is proposed.
- **Verdict cells:** Richards optimized is decisive; Permute optimized is the
  second target. Both VMs are required. Bounce, Base64 and RexxCPS guard
  unrelated reference/string/decimal behavior; Sieve guards zero-call layout.

### V1R02 — residual direct reference reduction

- **Correctness:** any elimination must preserve exact reference identity,
  root/tree/cell ownership, owner discovery, link/relink invalidation, escape,
  alias observation and teardown across normal return, signal and failure.
- **Ceiling:** Bounce still executes 510,000 `MKREF`, 4,222,600 `LINKATTR1`,
  3,065,000 `UNLINK`, 1,000,000 `UNLINKN` and about 510,000 `MINLINK*` in the
  optimized cell. Native samples see reference identity/release helpers, but
  the current attribution does not identify which operation is redundant.
- **Owner/placement:** compiler/RXAS only for an exact nonescaping chain with
  no runtime site state; canonical helpers remain the fallback.
- **Both VMs/failure:** stream-level elimination must be identical in both VMs;
  uncertainty retains the current root/tree/cell path.
- **Blast radius/cell:** private compilation only if proved; Bounce is decisive
  and the PERF2-02 reference fixtures plus Richards are guards.
- **Disposition:** defer without timing. PERF2-02 R1a is already accepted and
  no current residual site has the required static proof.

### V2R01 — payload-shape copy/move/clear fast path

- **Correctness:** a fast path must prove the complete destination cleanup and
  validity state for the exact scalar/string/binary shape. It may not shortcut
  object attributes, native ownership, references, decimal plugins or an
  unknown representation.
- **Ceiling:** the generic copy totals are large, but the largest current work
  is caused by avoidable full-object copies selected by V1. Typed operations
  already exist. Timing a helper before eliminating those operations would
  optimize induced work.
- **Owner/placement:** compiler typed selection first; a private helper only
  for a remaining statically proved shape. Existing general copy/clear is the
  failure fallback in both VMs.
- **Lifecycle/blast/cells:** destination ownership and single cleanup remain
  unchanged; private helper only, no public format. Re-profile Richards and
  Permute after V1, with Sieve and payload/reference fixtures as guards.
- **Disposition:** defer behind V1. Fixed-core reset, reset-needed flags, exact
  reset lists and quickened clearing remain rejected and are not this row.

### V3R01 — representation validity and selective retention

- **Correctness:** the installed completion contract makes byte length,
  codepoint count, cursors and VM-private validity describe exactly the new
  byte span while preserving all other representations and public flags.
- **Ceiling:** the correctness ceiling is exact and now realized. The broad
  performance mechanism has zero current hit count because conversion opcodes
  always materialize and there is no string-representation-validity cache.
- **Owner/placement:** private representation setters now own correctness. A
  future cache would need the representation layer plus compiler knowledge of
  mutations and numeric context.
- **Lifecycle/both VMs/fallback:** no extra retained allocation is introduced;
  both VMs use the same helpers. Arbitrary UTF-8 is recounted, known ASCII uses
  the exact fast completion, and existing conversion remains the fallback.
- **Blast/cell:** no language/public-format/ABI change. RexxCPS is decisive for
  any later performance proposal; the four-cell V3 matrix is mandatory.
- **Disposition:** accept correctness only; defer performance retention.

### V5R01 — size/shape-specific capacity reuse or pooling

- **Correctness:** any reopened form must identify an exact payload owner,
  size class and lifetime, preserve native/plugin/reference/object teardown,
  expose ownership to sanitizers and cap retained capacity.
- **Ceiling:** Richards/RexxCPS show large cumulative churn but only 1.5-1.7 MB
  high water and near-balanced alloc/free totals. Strings and binaries already
  retain capacity. `copy_value` and decimal conversion stacks, not an
  unowned generic allocation layer, dominate the selected samples.
- **Owner/placement:** exact representation or procedure payload owner after
  V1/V3 residual attribution; no general allocator or global pool.
- **Both VMs/fallback:** identical size/lifetime policy and deterministic full
  teardown; system allocation remains the fallback on cap, ownership or size
  mismatch.
- **Blast/cell:** private runtime at most, but sanitizer/lifecycle/RSS risk is
  material. Residual Richards or RexxCPS would be decisive, with Sieve and
  teardown controls.
- **Disposition:** defer without timing; current evidence selects narrower V1.

### C2R03 — procedure-affine slabs plus compact control stack

- **Correctness:** would require strict LIFO slices, non-moving values,
  fixed-call embedded arguments and exact reference/native/object/decimal/
  signal/thread teardown, with no ledger on ordinary mapping writes or reads.
- **Ceiling:** the entrance condition is not met. Low 1.5-1.7 MB high water and
  copy-induced/decimal churn do not prove a materially different reusable
  procedure-affine payload-capacity mechanism.
- **Owner/placement:** procedure/frame architecture only; neither compiler nor
  a local allocator helper can choose this unilaterally.
- **Both VMs/fallback/blast:** both VMs would require the same ownership model
  and canonical fallback. The frame/control and value-lifetime blast radius is
  architectural even without a public ABI change.
- **Cell/disposition:** call-heavy residuals plus zero-call Sieve would be the
  gate. Reject from this panel without timing. Any implementation requires a
  new evidence gate and Adrian's explicit architecture decision; C2-A/B,
  C2R01, reset lists and mapping ledgers remain closed.

### V6R01 — hot/cold `value` or broad layout split

- **Correctness:** every scalar, decimal, string, binary/native, reference,
  object/attribute and public/private flag state must retain identity,
  lifetime, cleanup and observation semantics.
- **Ceiling:** Apple confirms a 248-byte `value`, 168-byte `stack_frame` and
  the current artifact/RSS envelope, but does not establish the broad layout
  as the earliest safe owner of the selected cost. V1 removes a much narrower,
  larger measured mechanism.
- **Owner/placement:** value/VM/ABI architecture, informed by the supported
  hardware/compiler matrix rather than Mac layout alone.
- **Both VMs/fallback/blast:** a complete dual-VM migration and compatibility
  strategy is required. The internal and potentially ABI/platform blast
  radius is broad; failure needs a canonical unsplit representation.
- **Cell/disposition:** full Tier A, lifecycle, RSS, artifact and supported
  platform matrix. Defer. Adrian must make the architecture and any ABI
  decision before implementation.

## Decisions at the mandatory stop

No language or public RXAS/RXBIN decision is exposed by the recommended V1R01
slice. The selection needed now is whether to authorize exactly V1R01's proved
inlined-receiver direct-placement PoC. C2R03 and V6R01 remain separate explicit
architecture decisions and are not implied by a V1R01 selection.


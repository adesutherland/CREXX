# VM-STARTUP-01 — Mike's registry-index patch review

Status: first Release verdict accepted by Adrian on 2026-10-12 with
“Accept verdict; finish validation”. Repair, integration and local validation
are complete on the updated hotfix base. Adrian authorized commit and publication
on 2026-10-12; this change publishes the repair through hotfix to develop.
On 12 October Adrian also directs pulling remote develop's regression fixes.
Hotfix was fast-forwarded to `b1b2a742315738acfdf894ed0d08e93060c6ed8c`;
the repair is reapplied without changing upstream product fixes.

Repair receipt: [first Release verdict and code review](evidence/2026-10-12-vm-startup-repair/README.md).

Evidence: [review and receipts](evidence/2026-10-12-vm-startup-review/README.md).

## Vision and intended outcomes

Validate Mike Beer's 11 October startup report and exact attached patch on the
existing synchronized `hotfix` checkout. Explain the mechanism, actual benefit
boundary, semantic/lifecycle/portability risks, and whether the supplied patch
is suitable for integration. Preserve unrelated develop work. This review uses
a provisional application for local tests, then restores production source.
It does not select a new VM architecture or make a release-wide speed claim.

Adrian subsequently requests fixing allocation failure and TLS/context scope,
retaining the startup improvement, validating the repaired patch, and carefully
reviewing VM architecture impact and source conventions. The complete intended
end state is a safe optional rebuild index on hotfix: allocation failure uses
the legacy lookup path, each index belongs to its explicit context/rebuild,
normal lookup semantics and startup improvement are retained, and reviewed code
fits existing VM allocation/lifetime/build conventions. No new language, RXBIN,
public ABI, dispatch or persistent-cache contract is selected. Preserve the
original review evidence and unrelated develop work. The final delivery now
includes separate reviewable VM and upstream QA commits, published together to
hotfix and develop without changing the release line or unrelated local work.

The selected final source base is now fetched `origin/develop` at `b1b2a7423`,
on the existing hotfix branch. Earlier `105930c7f` observations remain historical
evidence, not qualification of the new base. Retain a matched unpatched Release
baseline from the updated base and validate the repaired final source there.

## Acceptance criteria

1. **AC-01 — passed:** identify exact email/patch, source revision and patch
   applicability; inspect all changed lookup and registry paths against current
   VM/linker contracts.
2. **AC-02 — passed:** compare stock and patched focused interface, signature,
   factory and late-load correctness; resolve `dynamic_interface_host` caveat.
   Retain commands and outputs and distinguish tested behavior from review risks.
3. **AC-03 — passed:** obtain a bounded ordinary profiling-off Release startup
   comparison with identical inputs and both concrete engines. Label it a
   single-mechanism, non-representative observation; retain raw evidence and
   provenance. RexxCPS/portfolio are outside this startup-only review.
4. **AC-04 — passed:** report integration blockers, remaining uncertainty and
   practical implications; restore original production source and leave the
   report/evidence on hotfix without commit, push or email response.
5. **AC-05 — passed in final Release and Debug:** every index allocation failure, including partial build,
   frees temporary storage and completes the legacy-equivalent registry rebuild;
   permanent failure-injection controls compare registry contents/targets and
   published dirty flags, not just exit status.
6. **AC-06 — passed in final Release and Debug:** explicit context/rebuild scope has no new TLS or context
   layout field. Interleaved/nested contexts, wrong-index context fallback,
   independent worker ownership and destruction preserve lookup results and
   leave no live temporary allocations. Permanent C regressions and inspection
   establish this; focused normal Debug follows the first-verdict decision gate.
7. **AC-07 — passed; verdict accepted:** minimum focused Release interface/signature/factory and
   late-load regressions pass; the first profiling-off Release verdict retains
   the startup benefit against the audited stock and contributor-patch controls.
   Reuse their identical retained inputs/binaries, add only needed same-session
   drift/pair controls, and disclose noise and deployment/engine boundaries.
8. **AC-08 — passed; final review complete:** careful source/architecture review verifies original
   traversal/duplicate/case/signature semantics, checked capacities and owned
   allocations; unchanged public interfaces, RXBIN, graph authority, VM dispatch,
   lifecycle and core-object sharing; nearby C style/conventions. Retain review
   findings and fixes, not an unsupported architectural-compatibility claim.
9. **AC-09 — passed:** after Adrian accepts the first Release verdict, complete
   the relevant normal correctness suite, focused Debug checks, final review and
   code-adjacent documentation. Keep cross-platform/sanitizer limits explicit;
   do not dispatch overnight assurance or claim release qualification. Leave
   reviewed source and receipts on hotfix without committing/pushing unless asked.
10. **AC-10 — local gates verified; publication authorized:** publish the reviewed
    source, regression repairs and complete receipts together to remote hotfix
    and develop by fast-forward. Verify remote commit identities and preserve
    the unrelated dirty hotfix2 checkout. Normal automatic publication workflows
    are expected to succeed; their live run records supply hosted results, with
    no manually dispatched overnight gate or CI-status-only follow-up commit.

## Implementation steps

1. **STEP-01 — complete; AC-01:** fetch/check hotfix, read performance policy,
   recover the exact MIME attachment and inspect code and upstream write-up.
2. **STEP-02 — complete; AC-02/03; depends STEP-01:** qualify stock Release VM
   inputs and preserve binaries; retain a minimal startup reproduction and
   focused correctness results.
3. **STEP-03 — complete; AC-02/03; depends STEP-02:** apply the unchanged patch
   provisionally, rebuild affected VMs, run focused tests and bounded matched
   startup samples. Do not redesign or perform broad closeout. Stop/review if
   a correctness failure appears.
4. **STEP-04 — complete; AC-04; depends STEP-03:** retain compact findings,
   restore production source, reconcile checkout state and deliver verdict.
5. **STEP-05 — complete; AC-05/06/08:** implement private optional metadata-index
   helpers with explicit arguments and legacy fallback; preserve the public
   void rebuild API and existing `rxvm_link()` protocol. Add permanent scoped
   allocation-failure and context-isolation regressions. Dependencies: original
   review/proof; no architecture/language decision gate is introduced.
6. **STEP-06 — complete; AC-05/06/07/08:** minimal focused Release build/checks
   and source audit; correct any regression, then freeze code/test/build inputs.
7. **STEP-07 — complete; verdict accepted; AC-07:** immediately run the ordinary profiling-off
   Release startup verdict and report it. **Stop for Adrian's decision** under
   `performance/AGENTS.md` before broad QA or closeout.
8. **STEP-08 — complete; AC-06/09; depends accepted STEP-07:** focused Debug
   and relevant normal correctness suite, without redundant overnight gates.
9. **STEP-09 — complete; AC-08/09; depends STEP-08:** final careful diff/review,
   runtime guide, roadmap and compact qualification receipt; leave on hotfix.
10. **STEP-10 — delivery in this change; AC-10; depends STEP-09:** fetch/check
    remote branch identities, verify unchanged qualified source/test/build hashes,
    commit the distinct upstream QA repair and VM repair separately, publish
    both branches together and verify their remote tips. Advance the unchecked-out
    local develop ref without changing the dirty hotfix2 checkout. This is
    development publication, not a tag or release.

STEP-08 continuation: normal validation on the former base passed 2,376 tests,
with two failures due to missing EXCLUDE_FROM_ALL CMS harnesses; both pass after
building those harnesses. Focused Debug exposed an actual index-cleanup ownership
defect: release must bind the index's owning worker while another context is
active. The helper now uses existing memory enter/leave scope, and the test checks
real release status and restoration, not just attempted frees. The repaired
focused Debug command passes. This is a normal Debug ownership finding, not an
ASan/LSan finding. These receipts do not qualify the new develop base.

After develop integration, rebuild matched Release controls, run the smallest
startup confirmation and focused Release/Debug checks, then the relevant normal
correctness suite. Reuse unchanged source/evidence where valid; the new product
base requires its own qualification. The accepted mechanism is unchanged and
the user authorizes finishing validation; no additional design is selected.

## Repair design selection

- Status quo: linear canonical metadata walks are correct in the original
  product but expensive; the contributor's eager temporary hash index is
  already performance-proved and fails allocation publication/scope review.
- Option A: keep an ambient TLS index, add platform macros, context identity,
  nesting save/restore and fallback. Rejected: introduces additional ambient
  state and its lifecycle even though rebuild callers can pass the index.
- **Option B, selected:** private metadata lookup/index helper, explicit nullable
  index parameters, and a private member cursor used by both builders. Index
  storage uses the existing worker-owned allocator and checked size_t tables;
  NULL/foreign-context indexes select the original ordered metadata scans.
  This isolates lookup mechanics without a new semantic graph or context field.
- Option C: persistent per-context/per-module or shared-per-link index. Deferred:
  broader invalidation, late-load and public rebuild integration are unnecessary
  to repair the reviewed patch. No cache/preparation architecture is selected.

Retain the contributor's bounded eager tables only during each dirty registry
rebuild; outside that phase, allocation-free legacy helpers remain. Lazy tables
would add optional failure states to individual lookups, while a narrower index
leaves known procedure/signature/member/covariance scans in the reviewed path.
Neither is required to repair the already-measured mechanism. The allocation
failure fallback is semantic equivalence, not a performance bypass on success.
These are metadata-building lookups, not a new steady-state opcode primitive;
the current sealed graph dispatch remains authoritative and unchanged.

Temporary build products and complete email MIME remain outside the repository.
Source baseline: `105930c7f` (`hotfix`, equal to fetched `origin/hotfix`).
Attachment baseline: `77ba820c`; `git apply --check` passes without adaptation.

## Acceptance receipts

- AC-01: exact attachment matches the pinned public patch; source applicability
  and all affected lookup/registry/caller paths reviewed against VM/linker docs.
- AC-02: 77/77 focused Release tests on stock and candidate; threaded interface
  and late-load controls pass on both. `dynamic_interface_host` compiles/runs.
  Actual-source diagnostic allocation injection proves silent registry loss
  while `rxvm_link()` reports success; portability/nesting concerns are labelled
  unqualified review risks, not reproduced runtime defects.
- AC-03: 24 balanced pairs per cell, all output/exit/stderr checks pass; default
  extended VM about 5.42x, concrete bare engines about 2.43x/2.45x. Initial noise
  prompted one unchanged extension; residual noise and host activity are retained.
  Three RSS processes per principal cell and artifact sizes are retained.
- AC-04: final report gives a repair recommendation and claim boundaries. Source
  was restored exactly and affected Release targets rebuilt to stock. No broad
  tests repeated, no commit/push/email, unrelated develop work preserved.

Original review complete before the subsequent STEP-05 selection. Production repair/adoption, broader correctness,
platform and sanitizer qualification remain outside this selected review;
the favorable startup observation does not approve the supplied implementation.

## Repaired first-verdict receipts

- All affected ordinary Release product targets build without C warnings.
  Frozen source/build/test hashes and exact commands are in the repair bundle.
- 78/78 focused Release tests pass. The new permanent test covers eight optional
  constructor allocation failures and all 16 positions across method/factory
  rebuilds, partial cleanup, exact registry targets, dirty flags, lookup order,
  interleaved/nested contexts, foreign-index fallback and allocator ownership.
- No new TLS/static mutable production state, context field, public layout/API,
  RXBIN/ISA, graph or dispatch policy is introduced. C99 helper syntax with the
  maintained single-threaded defines passes; native platform execution is not
  claimed. A stricter compact-header unused-function diagnostic is recorded.
- 24 stock/repaired pairs retain an approximately 80% extended / 65% bare startup
  reduction. Contributor/repaired comparisons reach the 36-pair policy cap;
  median differences are approximately zero. The fine-grained threaded mean
  interval remains inconclusive (-1.56% to +7.51%), with +0.937 ms mean absolute
  difference. No timing sample was removed and no confirmed regression is claimed.
- RSS/artifact changes remain below material guards. Adrian accepted the first
  verdict, including the disclosed threaded cost uncertainty, and authorized
  finishing validation. The accepted observations remain historical; the later
  develop integration receives its own matched confirmation and final receipts.

Normal closeout checks: focused Debug interface/selector/allocation-context
regressions; ordinary Release essential/smoke/comprehensive correctness suite
with performance-measurement excluded. Build its required fixtures first.
This is normal product validation; stress/qualification-only and overnight
platform/sanitizer matrices remain separate and are not added to this scope.

## Final acceptance and disposition

All AC-01 through AC-09 are verified for the selected local repair scope.
The final base is fetched `origin/develop` `b1b2a7423` on hotfix. STEP-08/09
complete: focused Release/Debug each pass 78 tests, and 3,129 normal Release
checks are verified through the 3,127-pass broad run plus focused replay of two
upstream test-fixture repairs. The final product source/binaries are unchanged
by those fixture-only corrections, so passing broad results are reused.

The follow-up Debug ownership repair uses the existing allocator worker scope
for index cleanup and restores the caller's worker. The strengthened regression
checks actual successful frees, all partial allocation failures while another
context is active, exact registry targets and final teardown. Final code review
found no remaining blocking issue or public/layout/graph/dispatch policy change.
Runtime documentation is updated and source/documentation `git diff --check`
passes. Raw build logs and the original patch retain emitted whitespace; they
are evidence, not reformatted source.

`VM-STARTUP-01-QA-01` (distinct upstream QA cause): the registered FORTRAN block
test lacked its input and runner; restore them against the already documented
collection contract. The RexxScript test expected an obsolete underscore stem
while its fixture uses dots; correct the assertion. Both tests pass in Release
and Debug. The unpatched current-develop VM produces the dotted output too.
No preprocessor/compiler implementation or syntax was changed.

Current paired observations retain approximately 79% extended and 63–65% bare
startup elapsed reduction, with host activity and short-process variance
disclosed. No general throughput, sanitizer-clean, cross-platform or release
qualification is claimed. Exact final inputs, failed/repair receipts and raw
startup data are retained in the final repair bundle.

Development publication is authorized and includes these completed local gates
and receipts in the first substantive push. Expected final disposition: hotfix
and develop contain both reviewed commits; the normal automatic Release
product/smoke, optimizer-parity and CodeQL workflows succeed. Hosted success is
an expectation here, not a verified local result; GitHub run records are the
evidence. No release is cut and no status-only follow-up commit is planned.

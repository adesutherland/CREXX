# RXVECTOR-02 — binary float32 owner and downstream consolidation

## Vision and selected design

Adrian approved implementing the reviewed generic float32 owner/binary import
and exact-search capability in rxvector, removing USearch, and qualifying RAG
on that basis. SQLite/publication/visibility remain Level-G RAG concerns.
Preserve the existing rxvector double APIs and the RXVIDX version-1 sidecar.
No compiler, language, RXBIN or host ABI change. The first phase qualified the
local implementation. Adrian subsequently authorized CREXX QA/publication via
the available hotfix branch, a normal install, and RAG baseline/publication
against that installed revision. Preserve unrelated checkout changes.

Status quo is the separately incubated C++/USearch provider. Alternatives were
(1) binary exact search with a dependency-free native kernel and (2) binary IVF
with grouped membership. The selected exact route preserves all twenty passage
orders at 0.950 s median versus 0.940 s with USearch; score differences are below
1e-7. Binary IVF adds another format/policy path without a useful advantage for
this corpus. The accepted review is in downstream
`docs/rxvector-binary-review-20260919.md`. This is a single-workload consumer
comparison, not a general CREXX performance portfolio or new optimization stage.

The user's instruction to lock in this reviewed design and QA the RAG solution
accepts that Release verdict and authorizes this complete bounded sequence.
Measure the integrated Release result after minimal correctness, before final
QA. Report any material departure; do not silently trade away results or scope.

## Acceptance

- **AC-01 (passed locally):** Existing decode/encode/cosine/top-k behavior remains green;
  new C owner/codec validates malformed lengths, dimensions, finite values,
  norms, trailing bytes, deterministic ties and safe numeric bounds.
- **AC-02 (passed locally; platform limits below):** C RXPA factory/methods preserve copy/close, embedded-NUL/UTF-8
  text, cleared failed outputs and session isolation. Both VMs, native/static
  packaging and focused native-memory checks pass on this host. Supported
  platform CI remains explicitly separate from local qualification.
- **AC-03 (passed locally):** RAG consumes an installed rxvector with no incubating plugin,
  USearch vendor or private C++ packaging. Existing binary sidecars are readable;
  publication, visibility, multiwindow, preflight/provider receipts, recovery and
  backup tests pass. Legacy IVF reading remains supported.
- **AC-04 (passed):** Frozen Scottish question/matrix comparison retains passage
  order and reference hits with explicit score tolerance; ordinary Release
  timings remain close to the accepted binary route. No re-embedding needed.
- **AC-05 (passed locally):** One stable full required RAG gate plus exact-input audit,
  documentation and dependency review completes; no repeated unchanged suites.

- **AC-06 (passed):** Publish the bounded CREXX code and documentation through
  hotfix to origin/develop after normal Release product and affected functional
  checks. Check the automatic publication workflows for the exact pushed SHA;
  do not dispatch the separate overnight deep/sanitizer matrices.
- **AC-07 (passed):** Install that clean CREXX revision to the normal user prefix,
  including matching native-inference runtime/provider packaging, and verify
  installed rxvector/native consumer behavior and package identity.
- **AC-08 (passed):** Point RAG at that published install, qualify the final
  130-case required selection once for changed toolchain/artifacts, audit exact
  inputs, baseline/publish origin/main, install and smoke the qualified artifact.
  Preserve scratch-only corpus testing and record remaining scope separately.

## Steps

1. **STEP-01:** Preserve/reuse qualified baseline; inspect and extend provider
   contract tests before implementation (AC-01/02).
2. **STEP-02:** Implement a generic C codec/immutable owner and float32 search in
   rxvector. Build/package into an isolated installed cohort (AC-01/02).
3. **STEP-03:** Switch RAG to the installed API, remove incubation/vendor and
   packaging scaffolding, run affected journeys (AC-03).
4. **STEP-04:** Freeze implementation and measure Release downstream against
   the retained accepted controls. Resolve adverse results before broad QA
   (AC-04).
5. **STEP-05:** Finish bounded correctness/native-memory acceptance and one
   full RAG gate, then report exact artifacts and remaining platform limits
   (AC-01–05).

6. **STEP-06 (complete):** Copy only RXVECTOR-02 code/docs to the clean hotfix
   checkout; build and qualify the normal product and focused vector/RXPA
   contracts, publish hotfix/develop, then check automatic gates (AC-06).
7. **STEP-07 (complete):** Install the exact clean published CREXX cohort and
   run installed provider/package acceptance (AC-07).
8. **STEP-08 (complete):** Rebuild RAG against that cohort, complete final QA,
   baseline/publish/install RAG and record both source/artifact identities (AC-08).

Publication evidence: `/Users/adrian/Documents/ScottishHistory/reports/bge-migration-20260918/rxvector-publication-20260919/`.

Initial local evidence: `/Users/adrian/Documents/ScottishHistory/reports/bge-migration-20260918/rxvector-consolidation-20260919/`.


## Initial local evidence

STEP-01 through STEP-05 are complete for the initial local scope. The subsequent
publication and installation evidence is recorded below; STEP-06 through STEP-08
are complete, including terminal automatic CI.

- The new interface failed against the prior provider before implementation;
  existing packed controls passed. `index-baseline.log` retains the missing
  class/procedure reproduction. `provider-checks/` retains all eight old/new,
  optimized/unoptimized VM passes plus native packaging and execution.
- The independent codec/kernel controls pass in ordinary Release. Production
  `vector`, `vector_static` and `rxvector_index_core_test` targets also build
  in the repository Release tree; `crexx-release-build.log` retains this check.
- Isolated ASan/UBSan builds use production C source and the installed SDK.
  `tools/asan-run.sh` runs `index_core` and `index_owner_native`: 2/2 pass in
  0.54 s. Native packaging links the instrumented static provider against the
  ordinary VM; this qualifies the new provider, not the entire runtime. Apple
  LeakSanitizer is unavailable; Linux leaks and other-platform gates remain
  separate. No first-party sanitizer finding was observed.
- `paired-comparison.json` and forty raw output/timing pairs retain the Release
  verdict: both routes median 0.965 s, all twenty passage orders match, 12/20
  known reference hits each, 61 cosine differences at most 9.38e-8. The expected
  backend label and per-call receipt identities differ. A first candidate
  launch of 15.78 s remains in the range; no cold-start claim is made.
- RAG removes its native plugin and USearch vendor. Six focused downstream
  cases pass in 17.34 s. The new 130-case required selection replaces two
  incubation tests with one installed-provider case; generic core coverage now
  belongs here. The separate scale lane is not part of this selection.
- Private installed cohort uses the previously qualified 15c8a3ba4200 runtime
  and JSON fix with the new provider. Dynamic SHA-256
  `918a38ee15d965bfcd70c797248bc29bbbaedfb6f703a7c66324f84af57d6c2f`,
  static archive `0953ef24ef852126fcc6e7679f09792d5d4d44b32277b27a988efc10cf0ec020`.
  RAG native SHA-256 `048845f024305402296b31b4312f7cfd860895bf5b11162fd2adbbb9892a8ef0`.
  Original cohort BUILDINFO is retained and is not a claim that these local
  provider changes are published. Normal installation and Git remain unchanged.

See downstream `docs/rxvector-consolidation-20260919.md` for full consumer
ownership, measurements and final accounting.

Final downstream qualification accounts for 130/130 required passes. The complete
selection took 541.25 s with 124 executions and six retained focused passes.
Two QA findings are retained: the staged launcher selected an old PATH runtime
(fixture corrected, case passes in 32.12 s), and the real-model case hit its
10-second load limit when sharing the parallel lane (unchanged isolated pass
in 10.91 s; scheduling demand corrected from two to eight slots). No product
or provider change followed the Release comparison. Closing documentation and
`qa-final.json` account for the remaining exact-input checks without repeating
the full selection. RAG other-platform and Linux leak-specific qualification
remain separate from the ordinary CREXX platform gates recorded below.

## Published source and normal installation

Source `5949ef27efd813b8bb96d23c58717b9a72aad1b9` was committed in the available
clean hotfix checkout, published to origin/hotfix and origin/develop, and installed
in `~/.local` with matching native-inference runtime/provider packaging.
The installed BUILDINFO identifies that exact source with `dirty=0`. The previous
prefix is preserved in the evidence directory. All pre-existing dirty files in
the main CREXX checkout remain byte-identical to their initial snapshot.

The normal Release product and native/provider targets built successfully.
The unique union of essential/smoke, rxvector and RXPA object correctness cases
passed **210/210 in 51.05 seconds**. The external native consumer built with the
installed driver and returned `RXVECTOR_INDEX_OK`. Both installed dynamic
provider locations hash to
`918a38ee15d965bfcd70c797248bc29bbbaedfb6f703a7c66324f84af57d6c2f`;
both static archive locations hash to
`eff583097f198f7a2e330596433bcdc291f854cee992d888cffa662fb378499e`.

Automatic [Build CREXX](https://github.com/adesutherland/CREXX/actions/runs/35438671112)
and [CodeQL](https://github.com/adesutherland/CREXX/actions/runs/35438670978)
both passed for that exact SHA. All core platforms, MinGW correctness, Linux
optimizer parity and four native-provider packages passed; the development
snapshot was published. The optional comprehensive lane and beta-release job
were planned skips, not passes. `crexx-build-ci.json` and `crexx-codeql-ci.json`
retain terminal per-job conclusions. Separate overnight deep/sanitizer workflows
were not dispatched for this ordinary publication. Documentation-only closeout
commits preserve the qualified product inputs and installed source identity.

RAG source `7bf0c6bbe7b6b7ab4263137f73cbe86bc67206df` is published to origin/main
and installed in `~/.local`, built against this normal CREXX cohort. Its full
required gate passed **130/130 in 720.51 seconds**, with no failures or duplicate
product executions. The updated documentation check passed in 12.75 s, and the
exact-input audit accounts for all 130 passes. Installed hashes match the
qualified artifacts; four fresh CLI and two MCP scratch checks pass. A single
network-denied local-BGE query preserves all twelve ordered passages, scores and
claims from the frozen reference. No master corpus processing or beta tag is
part of this publication. Downstream
`docs/baseline-publication-20260919.md` and this evidence directory retain the
source/tree/artifact manifests and remaining acceptance boundaries.

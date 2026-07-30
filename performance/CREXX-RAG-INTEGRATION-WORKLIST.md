# CREXX-side crexx-rag integration ledger worklist

Status: closed; CRI-01, CRI-02, CRI-04, CRI-05, CRI-06, CRI-08, CRI-09, CRI-13, and CRI-14 fixed; CRI-03 no-CREXX-change; CRI-07, CRI-10, CRI-11, and CRI-12 documented/package-closed

Started: 2026-07-29

Purpose: give every Gate-1A CREXX integration issue `CRI-01` through `CRI-14`
an evidence-backed accepted disposition before the downstream migration resumes.
This worklist is the resumable control plane. At most one CRI item may be
active. A passing downstream build alone does not close the ledger.

Evidence root:
[`evidence/2026-07-29-crexx-rag-integration-ledger/`](evidence/2026-07-29-crexx-rag-integration-ledger/).

## Authority and boundaries

- Home repository: `/Users/adrian/CLionProjects/CREXX`.
- Read-only evidence repository: `/Users/adrian/CLionProjects/crexx-rag`.
- Do not write, build in, reconfigure in, install into, stage, commit, stash,
  clean, or otherwise change the read-only repository.
- Preserve all pre-existing tracked and untracked work in both repositories.
- Candidate installs use only version-matched `mktemp -d` prefixes, never the
  normal user prefix.
- No hosted services, credentials, normal-prefix installation, staging,
  commit, push, pull request, or release publication is authorized.
- No public ABI, RXAS/RXBIN format, language syntax, or architecture choice is
  authorized without Adrian's explicit decision.
- A production performance edit stops at its mandatory first ordinary Release
  verdict before broad closeout.

Accepted dispositions are exactly `fixed`, `documented/package-closed`,
`no-CREXX-change`, or `decision-blocked`.

## Numbered programme plan

1. Freeze live repository, source, installed-toolchain, candidate-build, host,
   compiler, VM, submodule, remote, and user-owned-work identities.
2. Run a clean dedicated focused baseline and full Debug CTest before the first
   production edit.
3. Reproduce all concrete defects with the smallest cREXX sources and retain
   exact optimized/no-opt and dual-VM results where relevant.
4. Resolve compiler correctness and diagnostics in order: CRI-01, CRI-04,
   CRI-05, CRI-06.
5. Classify CRI-03 using deterministic loopback tests only.
6. Register and execute CRI-02 under the performance programme, stopping at the
   mandatory first ordinary Release verdict after any production edit.
7. After approval, close RXPA packaging/const correctness and then the generic
   library/runtime/documentation surfaces CRI-09 through CRI-14.
8. Prove a clean scratch install, external consumer, read-only downstream
   replay, clean out-of-tree downstream build where supported, full validation,
   and final preservation audits.

## Pre-change snapshot

- CREXX: branch `develop`, HEAD
  `d78c6fcfa81ef03fdbea65ff9cc39ed99e8716bf`, aligned with
  `origin/develop`; no tracked or staged diff.
- CREXX pre-existing untracked files: five historical
  `performance/evidence/**/lifecycle/crexx/lifecycle_probe.rxbin` artifacts,
  enumerated in the retained initial audit. They are user-owned.
- crexx-rag: branch `main`, HEAD
  `97cd87e91344d6ac1773a054bd38df23eb128ed2`; the tracked archive/programme
  changes and untracked Gate-1A files shown by the initial audit are user-owned.
- Installed driver: `/Users/adrian/.local/bin/crexx`, reporting
  `crexx-1.0.0-beta.3+local.g057592681c0c` built `20260728`.
- Installed runtime variants: `rxvm`, `rxbvm`, `rxvme`, and `rxbvme` all exist
  as Apple-arm64 Mach-O executables. Exact SHA-256 hashes are in the evidence
  index.
- Source and installed identities differ and must not be conflated.
- Raw pre-change capture: `/tmp/crexx-integration-ledger.mZ5jyp/`.

## Baseline gate

- [x] Read all governing instructions, required architecture/language guides,
      performance roadmap/charter/governance, and Gate-1A evidence.
- [x] Freeze source, installed, and dedicated baseline-build fingerprints
      separately. Scratch-install identity remains deliberately pending until
      the first candidate package proof.
- [x] Configure and build a clean dedicated Debug tree.
- [x] Run the relevant focused pre-change tests and retain exact counts:
      72/72 passed, zero failed or skipped.
- [x] Run the full Debug CTest with `--parallel 30 --output-on-failure`; retain
      exact passed, failed, skipped, and total counts.
- [x] Result: 1925/1925 passed, zero failed or skipped, in 205.06 seconds.
- [x] Re-audit both repositories immediately before the first production edit.
      CREXX contains only the five user-owned artifacts plus this programme's
      worklist/evidence. The crexx-rag branch, HEAD, and complete short status
      exactly match the initial audit.

## CRI ledger

- [x] **CRI-01 — Level G return of an imported Level B record**
  - Disposition: **fixed**.
  - Baseline evidence: provider compiles/assembles; the imported consumer fails
    with `#TYPE_MISMATCH` at `makereprorecord()` in both modes. The emitted
    provider metadata correctly identifies the return type as qualified source
    type `.repro_record..reprorecord`.
  - Root cause: the same imported callable could be discovered through source
    and binary metadata with return types `.repro_record..reprorecord` and
    `.reprorecord`. Duplicate-import consistency compared those raw strings
    before converging them on the same loaded nominal class, and contract
    symbol lookup did not normalize the source-qualified spelling.
  - Change: normalize imported class contract spellings at the symbol-resolution
    boundary and compare duplicate metadata object types through resolved
    nominal identity. Scalar types remain exact; reference/value form and full
    array shape remain exact; distinct record classes still produce
    `#TYPE_MISMATCH`. No syntax, RXAS/RXBIN serialization, runtime, or public ABI
    changed.
  - Regression: `imported_record_return_contract` covers source and binary
    provider imports, Level B and Level G facades, direct procedure assignment,
    method return/assignment/invocation, optimized and non-optimized compilation,
    `rxvm` and `rxbvm`, and a wrong-record negative case.
  - Validation: exact retained downstream source now compiles/assembles in both
    modes; regression 1/1; broader import/interface slice 31/31; clean complete
    Debug suite 1926/1926, zero failed or skipped, 190.29 seconds. Focused ASan
    regression 1/1 in 23.17 seconds with `detect_leaks=0`; the initial
    `detect_leaks=1` runner command was retained and proves Apple ASan reports
    leak detection unsupported on this platform. `git diff --check` passes.
  - Evidence: raw baseline, candidate, full-build, focused/full CTest,
    fingerprint, and sanitizer paths are indexed in the evidence README.
  - Remaining risk: Windows and non-Apple builds are deferred to the programme's
    final cross-platform/CI validation; the current host proves both VM engines
    on Apple arm64.
- [x] **CRI-02 — optimized `.binary` by-value hot-loop regression**
  - Disposition: **fixed**. Adrian accepted frozen V1 on 2026-07-30 and its
    proportional closeout is complete.
  - Baseline/root cause: optimized inlining materialized 614,400 defensive
    binary copies totaling 7,549,747,200 logical bytes and 614,400 matching
    lifetime ends. Non-optimized execution retained the calls and copied no
    binary payload. Fresh optimized by-value medians were 72.9 ms (`rxvm`) and
    77.4 ms (`rxbvm`) versus 18.3/19.7 ms non-optimized and 4.5/5.2 ms exposed.
  - Correctness prerequisite: a typed binary-memory store did not mark its base
    written, so a writable formal could leak mutation in the non-optimized
    path. The candidate fixes write-use tracking before opening any read-only
    alias path.
  - Selected candidate: validated read-only, exact, non-escaping binary inline
    formals may share a direct caller-local register; written, escaping,
    exposed/global/ref, optional, reference, aggregate, and unknown cases stay
    fail-closed. Aliased formals do not end caller storage. No language syntax,
    public ABI, VM instruction, or RXAS/RXBIN format changed.
  - Focused validation: 6/6 registered tests passed after the edit, covering
    optimized/non-optimized compilation, both VMs, codegen, writable mutation/
    rebinding isolation, repeated actuals, references, empty/arbitrary/large
    binaries, and the maintained benchmark. Implementation then froze.
  - First Release verdict: 104/104 balanced/interleaved warmup/recorded child
    executions passed with exact checksums. Optimized by-value improved 92.72%
    (`rxvm`) and 92.65% (`rxbvm`), became 0.2918x/0.2839x non-optimized, and
    removed 99.61%/99.52% of the inversion gap. However, the predeclared
    adjacent-control guard failed on `rxvm`: exposed +14.54% and direct +5.17%;
    `rxbvm` remained within 1%. No outlier was removed.
  - Adjudication: 34 balanced `rxvm` pairs per order/per-variant cell and three
    RSS samples per optimized dual-VM cell all preserved checksum `944025600`.
    Isolated exposed/direct medians changed -2.93%/-1.15%; their RXAS is
    byte-identical baseline/candidate. The apparent slowdown follows whichever
    control runs after the baseline's approximately 73 ms copy phase, proving a
    fixed-order warm-state artefact rather than a compiler regression. Peak RSS
    changed -0.56%/-0.37% on `rxvm`/`rxbvm`.
  - Regression adjudication: the first complete Debug run passed 1932/1934.
    Both failures were `select_dispatch_strings.binary_short` goldens; exact
    comparison proved that only the private I6 formal flag mask changed from
    `464` to `400`, clearing the now-disproved `ESCAPES` bit. Instructions,
    source maps, payload body, and runtime behavior were byte-identical.
  - Countermeasure: the maintained contract now asserts read-only mask `400`
    and writable mask `416`, round-trips an exposed dependency through
    RXAS/RXBIN/RXDAS/RXBIN, proves optimized imported read-only direct binding,
    proves writable copy isolation, retains non-optimized calls, and executes
    optimized/non-optimized artifacts on both VMs. The two exact goldens were
    updated only after that proof.
  - Closeout validation: focused normal 8/8; affected Apple ASan 3/3 with
    supported `detect_leaks=0`; fresh complete Debug CTest 1934/1934, zero
    failures or skips, 213.67 seconds; `git diff --check` passed. Frozen
    production hashes remained unchanged, and the read-only crexx-rag audit
    still matches its retained pre-work state.
  - Evidence: exact baseline, first verdict, adjudication, regression diagnosis,
    focused/ASan/full logs, hashes, commands, and remaining risk are indexed in
    [`CRI02-BINARY-BYVALUE-WORKLIST.md`](CRI02-BINARY-BYVALUE-WORKLIST.md) and
    the evidence README.
- [x] **CRI-03 — historical hosted `rxhttp` timeout classification**
  - Disposition: **no-CREXX-change**.
  - Audit: CREXX's maintained `ts_rxhttp` already covers request framing,
    Unicode byte lengths, content-length/chunked decoding, truncated bodies,
    malformed status lines, and non-2xx bodies. The retained Gate-1A local
    fixture adds the socket-boundary cases: malformed JSON after valid HTTP,
    a 50 ms receive timeout, refused connection, provider error, and success.
  - Fresh replay: the unchanged read-only Gate-1A sources were compiled into
    temporary optimized and non-optimized artifacts with the current candidate
    compiler. All four `rxvm`/`rxbvm` combinations passed against an explicitly
    loopback-bound fixture with `hosted=0`; timeout remained structured status
    `-5` (`socket receive timed out`) and connection refusal remained status
    `-3`. The native fixture and both compiled matrices are fingerprinted.
  - Classification: the historical hosted command completed generation but
    then repeatedly reparsed a 3,072-number embedding response and used an
    ineffective dimension-request shape. The repaired parse-once adapter and
    corrected bounded-dimension field subsequently completed the retained
    authorized canary. No independent `rxhttp` transport failure is
    demonstrated, so timeout semantics and transport code are unchanged.
  - Validation: `ts_rxhttp` passed optimized/non-optimized on `rxvm` and was
    replayed successfully on `rxbvm`; the Gate-1A loopback passed optimized and
    non-optimized on both VMs. No hosted endpoint, credential, normal-prefix
    install, or read-only-repository write was used.
  - Evidence: exact commands, outputs, hashes, the intentionally retained
    build-tree `rxvme`/legacy-module mismatch, and interpretation boundaries
    are indexed in the evidence README and under the temporary evidence root.
  - Remaining risk: the one historical hosted success is not reliability,
    cancellation, streaming, retry, privacy-route, or SLA evidence. Those are
    downstream/provider concerns, not evidence of a CREXX transport defect.
- [x] **CRI-04 — terminal `do forever` false missing-return diagnostic**
  - Disposition: **fixed**.
  - Baseline: the retained source fails optimized and non-optimized compilation
    with `#RETVAL_MISSING`. Its `say` after the loop is part of the procedure by
    the documented callable-boundary rule, but is unreachable; the defect is
    therefore whole-body reachability, not procedure parsing.
  - Root cause: callable structure fixup appended a bare return after the
    lexical final statement without asking whether the end was reachable. The
    later typed flow graph also gave every loop, including unconditional
    `do forever`, a false normal-exit edge.
  - Change: a bounded statement-flow summary now inserts the implicit return
    only when the callable end is reachable. It distinguishes unconditional
    forever loops, reachable/conditional exits, unreachable statements,
    nested-loop `leave` ownership, groups, and explicit returns. The typed flow
    graph omits only the false normal-exit edge for unconditional forever;
    explicit `leave` edges and conditional/bounded loop exits remain.
  - Regression: `do_forever_return_contract` covers the retained unreachable
    tail shape, returning terminal loop, a genuinely nonterminating typed
    routine, an unreachable `leave` after `iterate`, reachable and conditional
    `leave`, nested loops, explicit return after a loop exit, and ordinary
    fall-through. Both compiler modes are checked; terminating positives run on
    both `rxvm` and `rxbvm`.
  - Validation: exact retained source compiles/assembles in both modes;
    regression 1/1; broader control-flow slice 41/41; focused ASan 1/1 in 8.45
    seconds with the host-required `detect_leaks=0`; clean complete Debug suite
    1927/1927, zero failed or skipped, 194.72 seconds; `git diff --check` passes.
  - No syntax, RXAS/RXBIN format, public ABI, or runtime value semantics changed.
    Remaining cross-platform risk is deferred to the final programme gate.
- [x] **CRI-05 — Level G `PARSE VAR` emits forbidden `parseplan`**
  - Disposition: **fixed**.
  - Baseline: the retained Level G source lowers ordinary `PARSE VAR` to the
    certified Level B `parseplan` implementation, then rejects that generated
    instruction with `#ASSEMBLER_ONLY_LEVELB` in optimized and non-optimized
    compilation.
  - Root cause: the attached certified-exit object path discarded its
    registered exit identity by passing a null registry entry into response
    handling. The replacement fragment consequently inherited the Level G
    caller context, and the same generated `assembler parseplan` was rejected
    once during fragment validation and again after grafting into the caller.
  - Change: attached exit objects are associated back to their registered
    entry. Only replacement fragments from the compiler's certified allowlist
    are parsed as Level B and marked with an internal certified-fragment
    semantic context; later assembler validation recognizes that marker.
    Authored Level G `ASSEMBLER` and uncertified replacement text remain hard
    errors. Existing Level B `parseplan` lowering is preserved.
  - Regression: `parse_levelg_contract` covers whitespace-separated fields,
    literal delimiters, multiple/empty fields, missing delimiters, positional
    templates, an empty source, Unicode, repeated source/target aliasing, Level
    G and Level B, both optimization modes, and both `rxvm` and `rxbvm`. Its
    negative case proves authored Level G `assembler` still reports
    `#ASSEMBLER_ONLY_LEVELB`.
  - Validation: the exact retained reproducer compiles, assembles, and prints
    `alpha`, `beta`, the intentional empty field, and `gamma` in all four
    compiler/VM combinations; regression 1/1; broader PARSE/exit slice 29/29;
    focused ASan 1/1 in 7.57 seconds with `detect_leaks=0`; clean complete Debug
    suite 1928/1928, zero failed or skipped, 190.82 seconds; `git diff --check`
    passes.
  - The first clean full run exposed one expected optimized ADDRESS golden
    delta: three private branch labels moved by one ordinal because the
    attached certified fragment now retains its identity. No instruction or
    runtime behavior changed. The golden was updated, its two-mode focused
    check passed 2/2, and the complete suite was rerun from that clean build.
  - Documentation now distinguishes certified compiler-owned Level B lowering
    from authored Level G source. No syntax, public ABI, RXAS/RXBIN format, or
    runtime value semantics changed. Current-host evidence covers both Apple
    arm64 VM engines; final cross-platform validation remains pending.
- [x] **CRI-06 — malformed RXPA signature reports internal error**
  - Disposition: **fixed**.
  - Baseline: a native `ADDPROC` argument declaration `.int,.int` produces
    `#INTERNAL_ERROR_PARSING_IMPORT_AST` at the cREXX call in optimized and
    non-optimized compilation.
  - Root cause: RXPA return/argument metadata was interpolated into a temporary
    Level B declaration without a metadata-boundary check. Any failed parse was
    collapsed into an internal error. Worse, a semicolon could parse as a new
    statement and escape that check, while a trailing comma was silently
    discarded by the argument converter.
  - Change: empty argument components and statement separators outside quoted
    defaults are rejected before synthesis. Other failed declarations are
    classified as return or arguments with a return-only probe. The imported
    function retains a structured `RXPA_IMPORT_SIGNATURE_INVALID` payload with
    routine, plugin, field, original declaration, and reason, emitted at the
    consumer call site. Non-RXPA import errors keep their existing category.
  - Regression: the maintained native fixture proves valid zero-, one-, and
    multi-argument declarations in both compiler modes and on both VMs. Six
    negative consumers cover unnamed type lists, malformed separators,
    malformed argument and return types, consecutive empty components, and a
    trailing empty component. Every negative must be location-bearing and must
    contain no internal-error classification.
  - Validation: exact retained reproducer emits the new diagnostic at `5:5` in
    both modes; regression 1/1 in 7.86 seconds; broader RXPA/catalog slice
    46/46; focused ASan 1/1 in 12.01 seconds with `detect_leaks=0`; fresh clean
    build and complete Debug suite 1929/1929, zero failed or skipped, 222.45
    seconds; `git diff --check` passes.
  - The RXPA programming guide and compiler validation map document the Level B
    declaration contract and diagnostic. Default English, German, and Dutch
    catalogs contain the new key; US English inherits the default entry.
  - No RXPA macro/function signature, binary ABI, language syntax, RXAS/RXBIN
    format, or runtime calling convention changed. Both Apple arm64 VMs are
    proven; final cross-platform validation remains pending.
- [x] **CRI-07 — installed RXPA SDK and external consumer contract**
  - Disposition: **documented/package-closed**.
  - Baseline: a fresh scratch install contained 131 files but exposed only the
    generated `include/crexx_version.h`; external plugins had to copy
    `crexxpa.h`, `rxinteger.h`, generated headers, and the build helper from
    private source/build paths.
  - Package: the install now exports the header-only `CREXX::RXPA` target,
    public transitive headers, a relocatable `CREXXConfig.cmake` and exact-core
    version file, the supported dynamic-plugin helper, imported compiler/VM
    executable targets, exact build/version metadata, and explicit import,
    plugin, and runtime directories. No installed metadata contains a source-
    tree or build-tree path.
  - Consumer proof: the maintained CTest installs to a fresh prefix, copies a
    trivial external C/cREXX consumer out of the source tree, configures and
    builds only through `find_package(CREXX CONFIG)`, rejects an exact version
    mismatch, rejects missing compiler and runtime plugin paths, then compiles
    optimized/non-optimized and loads through `rxvm` and `rxbvm`. All four
    runs report the exact SDK version and result `42`.
  - Clean-build regression: Unix Makefiles exposed a real missing target edge
    for the generated `bin/rexxscript.rxbin` install dependency. The two exit
    fixtures now depend explicitly on the `rexxscript` producer; the exact
    failing targets pass 6/6 and the resumed clean parallel build completes.
  - Validation: focused installed consumer 1/1; RXPA slice 46/46; clean
    `mktemp -d` Debug build and separate `mktemp -d` install; 138-file SDK
    manifest with hashes; copied external consumer; both modes and VMs; full
    clean Debug CTest 1935/1935, zero failures, in 223.96 seconds;
    `git diff --check` passes. ASan is not applicable because no production
    C/C++ source changed.
  - Compatibility: dynamic RXPA callback signatures, plugin filename/loader
    contract, language syntax, public ABI, and RXAS/RXBIN formats are unchanged.
    The new surface is additive. Static helpers remain an in-tree/core-build
    facility; the public external contract intentionally covers dynamic
    plugins.
  - Evidence: exact commands, manifests, hashes, the Makefiles failure and
    countermeasure, full validation, and read-only audit are indexed in the
    evidence README. Cross-platform package execution remains for CI/final
    programme validation.
- [x] **CRI-08 — RXPA `SETSTRING` / `RETURNSIGNAL` const correctness**
  - Disposition: **fixed**.
  - Baseline: a minimized installed-header C consumer fails four times under
    `-Werror -Wwrite-strings`; the C++ consumer fails the same callback/macro
    conversions and an independent public tag/typedef collision.
  - Ownership: `rxvm_setstring` immediately calls `set_null_string`, which
    copies the null-terminated bytes into VM-owned storage and neither mutates
    nor retains the input. No new ownership choice was required.
  - Change: the setter callback, static shim, compiler guard, and VM
    implementation now accept `const char *`. The C tag remains unchanged;
    C++ receives a non-colliding tag behind the same pointer typedef. Dynamic
    C++ initializers receive the loader-required C linkage. A two-stage static
    C++ initializer macro expands the plugin ID before token pasting and opens
    the registration body correctly.
  - Regression: the installed consumer builds C with
    `-Wall -Wextra -Werror -Wwrite-strings` and C++17 dynamic/static
    registration with `-Wall -Wextra -Werror`. Optimized/non-optimized cREXX
    calls load the C and C++ plugins through both VMs. The C plugin mutates its
    source buffer after `SETSTRING`; all runs still return `copy-owned`, proving
    immediate copy ownership.
  - ABI: callback-table size `168` and offsets `setstring=48`,
    `setsayexit=152`, `resetsayexit=160` are identical before/after; the public
    `rxpa_setstring` symbol and calling convention are unchanged. Old-header
    plugins load on candidate `rxvm`/`rxbvm`, and new-header C/C++ plugins load
    on both pre-change VMs. There is no RXAS/RXBIN or generated cREXX change.
  - Source compatibility: ordinary mutable `char *` callers remain valid.
    Callback implementers assigning through `rxpa_func_setstring` must add
    `const` to their input parameter; this is the intended compile-time
    tightening. Binary plugins require no rebuild.
  - Validation: final focused consumer 1/1; RXPA slice 46/46; final Apple ASan
    1/1 in 139.35 seconds with `detect_leaks=0`; cross-built ABI four of four;
    final complete Debug CTest 1935/1935, zero failures or CTest skips, in
    383.38 seconds; `git diff --check` passes. No hosted call occurred.
  - Documentation records const input, immediate copy, caller ownership, and
    the non-null/null-terminated contract. No public binary ABI, language
    syntax, serialized format, or ownership semantics changed. Cross-platform
    compiler/package execution remains for CI/final validation.
- [x] **CRI-09 — fuzzy parsing and validation of noisy typed payloads**
  - Status/disposition: **fixed**. Adrian approved Option B exactly as
    specified, approved A2 and B1, and accepted frozen A2 on 2026-07-30.
  - Existing capability: `jsonvalid`, `jsontype`, `jsonget`, and `jsonmembers`
    can strictly validate complete JSON and implement application-local
    required/missing/null/type/extra-field policy. No public parser boundary or
    retained document exists, so noisy extraction must duplicate lexical rules
    or copy/reparse candidates.
  - Generic regression: `ts_rxjson_noisy_contract` covers Unicode prose,
    fenced JSON, missing/extra/null/type policy, truncation, malformed input,
    semantic ambiguity, optimized/non-optimized compilation, and both VMs.
    The retained 488-character adversarial case performs exactly 4,161 full
    parses over 1,110,760 candidate characters in all four cells. Focused CTest
    passes 5/5 including the linked-runtime fixture.
  - Architecture dependency: Adrian directed that the final JSON production
    surface be selected and implemented now rather than landing the discarded
    narrow span helper and revisiting the boundary in CRI-13. The read-only
    parse-once incubation proves a Level-B index is feasible, but its second
    module, path-only traversal, duplicated parser, and `F32V`/`I64V` serialized
    envelopes are not silently accepted.
  - Recommendation: Option B in
    [`CRI09-JSON-SURFACE-DECISION.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI09-JSON-SURFACE-DECISION.md):
    preserve all current `rxjson` functions, add one immutable indexed
    `.jsondocument` with path compatibility, document-local nodes and strict
    typed getters, and add `jsonscancontainer` returning an already indexed
    document. Keep schema/repair policy and packed numeric serialization out of
    this approval.
  - Approved implementation plan:
    1. retain one strict parser/index implementation in `rxjson` and route all
       existing functional selectors through ephemeral `.jsondocument` values;
    2. add the approved immutable document status, path, document-local node,
       arbitrary-key traversal, and strict typed-access methods;
    3. add the structural `jsonscancontainer` boundary returning an already
       indexed document with public character positions; B1 explicitly accepts
       one allocation-free boundary-validation pass plus one indexing pass for
       the successful slice;
    4. prove malformed, truncated, Unicode, missing/null, arbitrary-key,
       duplicate-key, typed-boundary, optimized/non-optimized, and dual-VM
       correctness; then freeze at the mandatory first Release gate.
  - Governed performance ID: `CAP-01-J01`.
  - Predeclared first Release acceptance rule: all focused correctness cells
    must pass; a retained document's parse plus 30 representative indexed
    accesses must take at most 50% of the pre-edit legacy 30-call repeated-parse
    workload on both optimized VMs; neither VM may show an optimizer-induced
    inversion; and the medians of existing one-shot compatibility operations
    must remain within 25% of the retained pre-edit build. Any miss is reported
    as a failed or mixed first verdict rather than hidden by benchmark changes.
    The noisy-container proof must also replace the 4,161-parse workaround with
    a structural scan and one strict parse of the returned document.
  - Next mandatory stop: after the first production edit, minimum focused
    correctness, frozen ordinary profiling-off Release products, and the
    smallest decisive comparison against the retained pre-edit executable.
  - Minimum focused correctness: 11/11 passed, zero failed or skipped, covering
    legacy selectors plus the document/noisy contracts in optimized and
    non-optimized mode and both VMs. Raw log:
    `/tmp/crexx-integration-ledger.mZ5jyp/cri09-optionb-focused-final.log`
    (SHA-256
    `76ac73fb81fa60eee8247dac8841792dc19a9ca677252ee460d83bf59b451709`).
  - First-verdict freeze: production `rxjson.crexx` SHA-256
    `28d851dd4dc72b175b2381039de3d283d2203ed6fd6f79187fc8930579155165`;
    decisive benchmark SHA-256
    `f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b`.
    The implementation is frozen pending the ordinary Release verdict; broad
    Debug, sanitizer, package/install, and documentation closeout have not run.
  - First Release verdict: **mixed and rejected by the predeclared guard**.
    Retained construction plus 30 indexed path gets uses only 20.86%/20.33%
    of matched repeated-parse time on `rxvm`/`rxbvm`, and there is no material
    optimizer inversion. However, eight of ten legacy one-shot guard cells are
    36.96--53.39% slower; only `count` improves. The adversarial scanner is
    correct but takes 639,531/688,135.5 us because it restarts across 64 invalid
    openers. Complete evidence, cause, alternatives, recommendation, exact
    decision, and continuation prompt:
    [`CRI09-FIRST-RELEASE-VERDICT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI09-FIRST-RELEASE-VERDICT.md).
  - Mandatory decision: approve or reject the recommended internal
    single-parser/result-sink countermeasure. No later CRI item is active and no
    broad closeout may run while this verdict is pending.
  - Decision resolved: Adrian approved recommended countermeasure Option 1 on
    2026-07-30. V1 is retained as a comparator. V2 may change only private
    parser routing: one strict core with full-index, allocation-free legacy
    query/validation, and fail-fast recoverable-boundary result modes. The
    public Option B API and every compatibility/ownership/ABI/format boundary
    remain fixed. V2 uses a hand-written streaming table-driven lexer: a
    256-byte class table and number DFA feed transient spans directly to the
    shared recursive parser without a token-list allocation. Expanded focused
    correctness passes 11/11 across opt/no-opt and both VMs. Frozen production
    SHA-256 is
    `eac3c5cfaa0a05ec32f8300c12b93fb8afe67c9c72b19653cef4ecceceb419d6`;
    the unchanged benchmark SHA-256 is
    `f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b`.
    The next mandatory stop is the V2 ordinary Release verdict.
  - V2 ordinary Release verdict: **mixed and rejected by the unchanged
    guard**. All correctness cells pass. The result-specific sink improves
    every legacy cell 8.65--18.19% versus V1, retained path access is
    75.56%/78.64% faster than repeated pre-edit access, and scanner time falls
    99.92%/99.90% to 413/523 us. Seven of ten legacy cells nevertheless remain
    26.92--40.23% slower than pre-edit. V2 also boundary-validates then indexes
    the one successful scanner slice, so it has two grammar passes rather than
    the original one-pass internal aspiration. B1 is the explicit acceptance
    of that construction: one allocation-free boundary-validation pass plus
    one indexing pass for the successful slice. Root cause, exact alternatives,
    two separate decisions and continuation prompt:
    [`CRI09-V2-RELEASE-VERDICT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI09-V2-RELEASE-VERDICT.md).
  - Current decision A: approve/reject the recommended lean hybrid streaming
    core while retaining one grammar, the table-driven tokenizer and unchanged
    public/ABI/format contracts. Current decision B: separately approve/reject
    accepting the measured allocation-free boundary validation plus one index
    construction for the successful scanner slice. No later CRI item may
    start.
  - Decisions resolved: Adrian approved A2 and B1 on 2026-07-30, explicitly
    sequencing the A2 performance benchmark before B1 closure. V2 remains the
    frozen comparator. A2 alone may now change private lexer/parser hot paths;
    focused correctness, a new freeze and the ordinary Release comparison must
    stop for a verdict before B1 documentation or broad validation begins. B1
    accepts one allocation-free boundary-validation pass plus one indexing pass
    for the successful slice; it is documentation/closure, not another
    tokenizer implementation.
  - A2 correctness freeze: focused legacy/document/noisy coverage passes 11/11
    across opt/no-opt and both VMs. Frozen production SHA-256 is
    `c2ff6f246ecf8f13837cd82a1f6c5e35cc6f855fa18889eef6a48ee0128a9318`;
    the unchanged benchmark SHA-256 is
    `f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b`.
    The implementation is now frozen at the mandatory A2 ordinary Release
    comparison. B1 remains approved but deliberately not yet closed.
  - A2 ordinary Release verdict: **pass**. All ten legacy cells meet the
    unchanged 25% guard; A2 improves them 10.41--14.96% versus V2. Retained
    path access is 76.25%/79.05% faster than repeated pre-edit parsing;
    resolved-node use is 93.87%/94.11% faster; scanner medians are 394.5/480.5
    us; and opt/no-opt differences remain between -2.17% and +2.86% with no
    material inversion. Frozen A2 SHA-256 remains
    `c2ff6f246ecf8f13837cd82a1f6c5e35cc6f855fa18889eef6a48ee0128a9318`.
    Exact products, raw samples, hashes and decision:
    [`CRI09-A2-RELEASE-VERDICT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI09-A2-RELEASE-VERDICT.md).
  - A2/B1 acceptance: Adrian accepted frozen A2 on 2026-07-30. B1 is closed as
    the documented successful-slice lifecycle: one allocation-free
    boundary-validation pass over the original input plus one indexing pass
    that owns the exact slice and builds its sole index. Rejected candidate
    openers construct no candidate document or index; the public call
    initializes one invalid empty output document for deterministic failure.
  - Documentation: both maintained Level B references now specify the complete
    immutable-document, node, typed-getter, scanner, ownership, position,
    error, duplicate-key, B1 and policy-separation contracts. They are
    byte-identical at SHA-256
    `2059827237b4e7cf39e9223058f5af9bfe20d3bc799bee1745fa1d9c1e768188`.
  - Closeout validation: dedicated candidate rebuild passed; focused Debug
    passed 11/11 and focused macOS ASan passed 11/11 across opt/no-opt and both
    VMs; full Debug passed 1,943/1,943, zero failures or skips, in 305.68
    seconds. Apple ASan does not support LeakSanitizer: the leak-on exit 134 is
    retained, and the supported `detect_leaks=0` build/test path passed.
    `git diff --check` passes.
  - Compatibility: legacy functions and results remain supported; the new
    surface is pure Level B and changes no native ABI, language syntax,
    RXAS/RXBIN, serialized format, or packed numeric representation. Node IDs
    and the index layout are explicitly private and ephemeral.
  - Exact source/test/doc hashes, commands, raw log hashes, accepted
    performance verdict and residual risks:
    [`CRI09-CLOSEOUT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI09-CLOSEOUT.md).
- [x] **CRI-10 — ergonomic multiline `ADDRESS` output capture**
  - Status/disposition: **documented/package-closed**. Existing scalar and
    array redirects already capture multiline stdout and stderr independently;
    `rc` preserves command status. No facade or interpreter change is needed.
  - The maintained Level B contract covers scalar/array multiline output,
    multiline errors, success/failure status, fresh empty destinations,
    Unicode and embedded delimiters on opt/no-opt and both VMs. Focused CTest
    passes 5/5, zero failed or skipped.
  - The public statements reference now specifies representation, status and
    text behavior. Array reuse remains explicitly deferred to CRI-12.
  - Exact reproducer, diagnosis, commands, hashes, raw results, compatibility
    and risks: [`CRI10-CLOSEOUT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI10-CLOSEOUT.md).
- [x] **CRI-11 — argument-vector form for `ADDRESS COMMAND`**
  - Status/disposition: **documented/package-closed**. The existing
    `ADDRESS CREXX "run :argv[]"` path preserves argument boundaries and
    directly launches the selected executable; the command-string
    environments remain intentionally distinct.
  - A harmless external fixture proves whitespace, empty arguments, embedded
    quotes, Unicode and shell metacharacters across opt/no-opt and both VMs.
    The metacharacters remain inert data. Focused CTest passes 5/5.
  - Exact reproducer, initial diagnostic, diagnosis, commands, hashes, raw
    results, compatibility and risks:
    [`CRI11-CLOSEOUT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI11-CLOSEOUT.md).
- [x] **CRI-12 — redirect array lifecycle**
  - Status/disposition: **documented/package-closed**. Redirect arrays are
    mutable append targets. Empty streams preserve existing elements, and
    failure output follows the same per-stream rule.
  - Call `arraydrop` independently on reused `OUTPUT` and `ERROR` arrays when
    replacement semantics are wanted. Accumulation without a drop is supported.
  - The maintained Level B opt/no-opt and dual-VM matrix covers first capture,
    reuse, empty output, failure and post-drop reuse; focused CTest passes 5/5.
  - Exact reproducer, diagnosis, commands, hashes, raw results, compatibility
    and risks:
    [`CRI12-CLOSEOUT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI12-CLOSEOUT.md).
- [x] **CRI-13 — parse-once JSON and packed numeric entities**
  - Status/disposition: **fixed**. Adrian accepted raw Option B as the sole
    Release-1 surface and accepted deferral of public C wrappers after their
    measured typed reads were 4.56x--5.10x B and writes 2.41x--3.67x B.
  - Governed performance/decision ID: `CAP-01-J02`.
  - The unchanged existing JSON benchmark remains within every compatibility
    guard; the frozen product is 78.81%/81.97% faster for retained path use and
    94.60%/94.91% faster for once-resolved-node use.
  - The new projection/current-composition focused matrix passes 9/9, and the
    combined CRI-09 behavior/projection matrix passes 13/13. Formal Release is
    40/40 numeric plus 24/24 unchanged-parser samples. B total is 4.01/4.86 ms
    optimized, but the isolated f32 projection misses its prototype ceiling.
  - Recommendation: explicit `node_f32_array`/`node_i64_array` methods returning
    owning, headerless canonical-little-endian `.binary`; no wrapper,
    normalized flag, inference, cache or serialized envelope.
  - Exact reproducer, benchmark, alternatives, correctness, time/memory,
    compatibility, first Release rule, smallest decision and continuation:
    [`CRI13-PACKED-NUMERIC-DECISION.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-PACKED-NUMERIC-DECISION.md).
  - First Release verdict, source-level cause, measured alternatives,
    recommendation and continuation:
    [`CRI13-B-RELEASE-VERDICT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-B-RELEASE-VERDICT.md).
  - Repeated R1 Release verdict and exact recommendation:
    [`CRI13-B-R1-RELEASE-VERDICT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-B-R1-RELEASE-VERDICT.md).
  - Post-R1 conversion edge reproduction, bounded span-helper alternatives,
    public RXAS/RXBIN consequences and exact decision:
    [`CRI13-BOUNDED-NUMERIC-CONVERSION-DECISION.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-BOUNDED-NUMERIC-CONVERSION-DECISION.md).
  - Favorable R2 ordinary Release verdict, hashes, raw results and exact
    acceptance decision:
    [`CRI13-R2-RELEASE-VERDICT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-R2-RELEASE-VERDICT.md).
  - Accepted R2 full Debug/affected ASan/documentation closeout:
    [`CRI13-R2-CLOSEOUT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-R2-CLOSEOUT.md).
  - Option C benchmark-local design and first ordinary Release verdict:
    [`CRI13-C-CLASS-COMPARISON-DESIGN.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-C-CLASS-COMPARISON-DESIGN.md),
    [`CRI13-C-CLASS-RELEASE-VERDICT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-C-CLASS-RELEASE-VERDICT.md).
  - Final accepted production, restored benchmark, validation, compatibility
    and remaining-risk record:
    [`CRI13-CLOSEOUT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-CLOSEOUT.md).
  - RXAS/VM trace: the classifier is fully inlined; two full-source copies per
    element move 359,294,976 logical bytes. A no-loop-copy scratch control is
    1.311/1.755 ms but still fails the prototype ceiling. RXAS already has the
    machine-flow foundation and deliberately rejects generic copies as
    `full-value-ownership-unproved`; bounded follow-on is queued as
    `PERF2-07-B02`:
    [`CRI13-R1-RXAS-TRACE.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-R1-RXAS-TRACE.md).
- [x] **CRI-14 — generic schema/contract surface for external consumers**
  - Status/disposition: **fixed** under approved Option B.
  - Inventory: RXBIN 007 has a useful internal typed graph, but raw class
    attributes are not graph fields, parameter names require descriptor
    reparsing, relative object argument spellings are not canonical graph
    identities, and version/nullability/error/evolution/RexxDoc semantics are
    absent. Level B runtime reflection is value-only.
  - Installed boundary: a fresh 141-file scratch install has no graph headers
    or CMake target. Its copied `librxbin.a` is not a supported development
    surface; the exact external configure fails on missing `CREXX::RXBIN`.
  - Reproducer: the strongest current interface/record convention compiles and
    assembles optimized and non-optimized. Both modes produce the same
    19-type/10-member/12-callable graph, while `rxdas -p` retains nine raw
    private `.attr` records and no RexxDoc prose/tags.
  - Recommendation: Option B, a build-time `crexx-contract` CLI/CMake helper
    emitting deterministic `crexx.operation-contract/1` JSON from existing
    Level B interfaces plus explicit version/nullability/error options. It adds
    no language syntax, runtime reflection, public C ABI or RXBIN 007 change.
  - Exact probe, commands, hashes, mapping/evolution rules, Alternatives 0/A/C,
    consequences, decision and continuation prompt:
    [`CRI14-CONTRACT-SURFACE-DECISION.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI14-CONTRACT-SURFACE-DECISION.md).
  - Approval: Adrian approved Option B on 2026-07-30 and directed that the
    emitted contract remain the long-term public boundary even if its metadata
    acquisition later evolves. RXBIN 007 is therefore the first adapter, not
    part of the public contract identity.
  - Implementation: installed `crexx-contract`, `CREXX::crexx-contract` and
    `crexx_add_operation_contract()` emit deterministic
    `crexx.operation-contract/1` JSON from existing Level B operation/payload
    interfaces plus explicit version/nullability/optional/error facts. The
    model/writer/compatibility checker are metadata-independent; the private
    RXBIN-007 adapter resolves relative graph identities and rejects ambiguity.
  - Regression countermeasure: a global selector-canonicalization attempt
    broke the existing CMS ADDRESS bridge and was reverted. Existing selector
    and ABI spellings are unchanged; canonicalization remains local to the
    exporter adapter.
  - Validation: focused graph/generation/installed consumer 3/3; relevant
    interface slice 14/14; affected Apple ASan 3/3 with supported
    `detect_leaks=0`; clean package/contract/RXPA consumer 3/3; clean complete
    Debug 1,965/1,965, zero failures/skips, 324.77 seconds. A fresh 141-file
    scratch install and no-fallback external consumers pass.
  - Downstream replay: exact concrete reproducers and the binary probe pass in
    both modes/VMs. The read-only project configures/builds with both fallbacks
    off and passes 23/26 tests; the three failures are retained removal seams
    for its legacy `rx_socket` module argument and superseded incubator JSON
    wrapper surface, not CREXX regressions.
  - Exact implementation, commands, hashes, compatibility, downstream seams
    and remaining risks:
    [`CRI14-CLOSEOUT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI14-CLOSEOUT.md).

## Mandatory stop log

- **CRI-09 public JSON architecture stop, 2026-07-30.** Adrian requested that
  the final production JSON surface be considered and implemented to avoid a
  narrow helper that CRI-13 would replace. This requires selecting a broad
  public class/traversal/typed-access contract. The exact alternatives,
  recommendation, consequences, decision, and continuation prompt are retained
  in `CRI09-JSON-SURFACE-DECISION.md`. No later item is active and no
  unapproved production JSON API remains in the worktree.
- **CRI-09 architecture stop resolved, 2026-07-30.** Adrian approved Option B
  exactly as specified. Work resumed at CRI-09 under `CAP-01-J01`; the next
  mandatory stop is the first ordinary Release performance verdict.
- **CRI-09 first Release stop, 2026-07-30.** Frozen V1 passes correctness and
  parse-once performance, but fails the predeclared compatibility guard and has
  an avoidable scanner restart. The implementation remains provisional and
  frozen. Adrian's smallest next decision is whether to authorize one shared
  parser core with full-index, legacy-query, and recoverable-boundary sinks.
- **CRI-09 V1 stop resolved, 2026-07-30.** Adrian approved the recommended
  private result-sink countermeasure. CRI-09 resumed as the sole active item and
  must stop again at the V2 ordinary Release verdict.
- **CRI-09 V2 Release stop, 2026-07-30.** The table-driven streaming V2 passes
  focused correctness, parse-once access, optimizer and scanner-time gates but
  seven legacy compatibility cells remain outside the 25% guard. The frozen
  candidate also uses a boundary validation plus a separate successful-slice
  index construction. The two independent exact decisions are retained in
  `CRI09-V2-RELEASE-VERDICT.md`; no later item is active.
- **CRI-09 V2 stop resolved, 2026-07-30.** Adrian approved A2 and B1, ordering
  the A2 implementation and mandatory Release benchmark before B1 closure.
  CRI-09 resumed as the sole active item and must stop again at the A2 verdict.
- **CRI-09 A2 Release stop, 2026-07-30.** Frozen A2 passes every predeclared
  correctness and performance rule. It remains provisional pending Adrian's
  acceptance. B1 and broad CRI-09 closeout remain deferred.
- **CRI-09 closed, 2026-07-30.** Adrian accepted frozen A2. B1's deliberate
  boundary-validation plus indexing lifecycle is documented; focused Debug and
  macOS ASan pass 11/11, and full Debug passes 1,943/1,943. CRI-09 is fixed;
  CRI-10 becomes the sole active item.
- **CRI-10 closed, 2026-07-30.** Existing scalar/array ADDRESS redirection is
  functionally complete; the documentation and dual-VM/opt matrix now cover
  multiline stdout/stderr, status, empty output, Unicode and delimiters.
  Disposition is documented/package-closed; CRI-11 becomes active.
- **CRI-11 closed, 2026-07-30.** Existing `CREXX run :argv[]` direct execution
  preserves exact argument boundaries without invoking a shell. The maintained
  cross-VM/optimization matrix covers whitespace, empty arguments, quotes,
  Unicode and shell metacharacters. Disposition is documented/package-closed;
  CRI-12 becomes active.
- **CRI-12 closed, 2026-07-30.** Redirect arrays intentionally append to the
  caller-owned mutable array; empty streams do not erase earlier records and
  failures follow the same per-stream rule. `arraydrop` is required for
  replacement-style reuse. Disposition is documented/package-closed; CRI-13
  becomes active.
- **CRI-13 design stop, 2026-07-30.** CRI-09 already supplies the final
  parse-once JSON surface. Current public numeric composition and a renamed
  out-of-tree projection prototype prove that bulk internal traversal can close
  a 13.19x--19.77x mechanism gap, but choosing raw `.binary`, wrapper classes
  or the historical envelope is a public representation decision. CRI-13 is
  decision-blocked and no later item is active.
- **CRI-13 design stop resolved, 2026-07-30.** Adrian approved B as the
  performance/storage primitive and directed a later measured comparison with
  C classes using typed by-value reads and writes. CRI-13 resumed as the sole
  active item. The next mandatory stop is B's first ordinary Release verdict;
  C and CRI-14 remain deferred until that verdict.
- **CRI-13 Option B first Release stop, 2026-07-30.** Frozen B passes focused
  correctness, the end-to-end reduction, optimizer, byte/checksum/RSS and
  unchanged JSON guards, but fails its prototype-ceiling rule in all four
  cells. Per-element string validation includes a full nonzero scan plus
  `lower`, substring and digit work. The recommended bounded countermeasure is
  one private allocation-free f32 source-span classifier. No item is active;
  C and CRI-14 remain deferred.
- **CRI-13 Option B first Release stop resolved, 2026-07-30.** Adrian approved
  the recommended private allocation-free f32 source-span classifier exactly
  as specified. CRI-13 resumed as the sole active item. The next mandatory stop
  is the repeated ordinary Release verdict; C and CRI-14 remain deferred.
- **CRI-13 R1 repeated Release stop, 2026-07-30.** Frozen R1 passes focused
  correctness, exact bytes/checksums, optimizer and unchanged-JSON guards but
  is 20.47x--22.01x its prototype projection ceiling and reaches 25.38% of
  retained current total on optimized `rxbvm`. RXAS proves the classifier is
  already fully inlined and exposes two whole-source copies per element;
  removing the loop copies in scratch improves projection to 1.311/1.755 ms
  but still misses the 2x ceiling. The recommended CRI-13 countermeasure is
  parse-time classification flags in the existing private unused node field.
  The systemic register-copy census and bounded RXAS full-copy proof are queued
  separately as `PERF2-07-B02`. No item is active; C and CRI-14 remain
  deferred.
- **CRI-13 R1 stop initially resolved, then conversion architecture stop,
  2026-07-30.** Adrian approved parse-time Alternative D. Before its production
  edit, review of the proposed binary span conversion exposed and minimized a
  pre-existing dual-VM contract failure: `1e-320`, `1e-324`, and `1.8e308`
  escape `node_f32_array` as `CONVERSION_ERROR` rather than returning `-6`.
  The approved D exponent flags would retain the boundary flaw. Production D
  was therefore not edited. The recommended next rung is a non-throwing,
  allocation-free bounded binary span-to-binary64 operation backed by a
  locale-independent exact converter, compared against a repaired
  f32-specific D fallback. This requires an explicit RXAS/RXBIN architecture
  decision; no item is active.
- **CRI-13 conversion stop resolved and R2 Release stop reached, 2026-07-30.**
  Adrian observed that the conversion signal can be caught. One handler now
  encloses the complete projection loop and translates conversion failure to
  documented status `-6`; `node_float` returns `-4` on the same edges. R2
  computes nonzero/binary64 flags during the existing scanner pass. Focused
  Debug and Release each pass 5/5, the broader Debug JSON matrix passes 17/17,
  and formal Release passes 40/40 numeric plus 24/24 parser samples. Optimized
  f32 projection is 295/326 us, 1.14x/1.22x the prototype; production total is
  9.60%/11.03% of retained current. Every rule passes. No item is active while
  Adrian accepts or rejects frozen R2. The generic bounded conversion review is
  queued separately as `PERF2-07-C01`; it is not authorized by R2 acceptance.
- **CRI-13 R2 accepted, 2026-07-30.** Adrian accepted frozen `rxjson.crexx`
  SHA-256 `7916d23df7cc488adfee54d4b25e504fa3afd47a8746fb7d15a6f401bde83d77`.
  CRI-13 is active for proportional R2 closeout followed by the already
  approved benchmark-local Option C comparison. C uses headerless owning
  packed-f32/packed-i64 prototypes with typed by-value reads and writes; no
  public names, wrapper promotion, envelope or serialized format is selected.
  `PERF2-07-C01` remains queued and inactive.
- **CRI-13 Option C comparison stop, 2026-07-30.** Accepted R2 closeout passes
  full Debug 1,963/1,963 and affected ASan 17/17. The benchmark-local owning
  wrappers pass focused 5/5 and formal Release 40/40, but optimized typed reads
  are 5.01x/4.71x raw B and writes are 3.67x/3.19x on `rxvm`/`rxbvm`. RXAS
  proves direct resolved method calls remain uninlined and pay receiver,
  initialization and attribute-link overhead; there is no per-element binary
  copy. No item is active. Recommendation: retain B only for Release 1 and
  revisit C after a separately governed generic concrete/final method-access
  countermeasure.
- **CRI-13 closed, 2026-07-30.** Adrian accepted B-only for Release 1. The C
  probe was removed after retaining evidence; the maintained benchmark hash
  and both Release image hashes exactly match frozen R2, and the restored
  focused matrix passes 5/5. CRI-13 is fixed. `PERF2-03-F06` records the
  generic class-method ceiling without reopening CRI-13; CRI-14 becomes active.
- **CRI-14 public contract stop, 2026-07-30.** The maximized current-source
  probe proves that the internal RXBIN graph is a viable exporter seed but not
  a supported or complete external contract. A fresh scratch install exposes
  neither headers nor a graph target. Option B is recommended: static,
  versioned `crexx.operation-contract/1` JSON generated at build time from
  existing interfaces, with no runtime reflection, public C ABI, language
  syntax or RXBIN 007 change. The new CLI/CMake and JSON format require Adrian's
  explicit approval; CRI-14 is decision-blocked and no item is active.
- **CRI-14 contract stop resolved, 2026-07-30.** Adrian approved Option B and
  explicitly made `crexx.operation-contract/1` the durable long-term surface
  even if metadata acquisition evolves. CRI-14 is active; RXBIN 007 is the
  initial private adapter rather than a public dependency of the artifact.
- **CRI-14 and ledger closed, 2026-07-30.** The accepted artifact/CLI/CMake
  surface, strict mapping/evolution rules, installed consumers, clean full
  suite and read-only downstream replay are complete. CRI-14 is fixed. All 14
  CRI items now have one accepted disposition; the final downstream removal
  seams and Phase-1B stop prompt are retained in
  `CREXX-RAG-INTEGRATION-CLOSEOUT.md`.

## Final closeout gate

- [x] Every CRI item has exactly one accepted disposition and linked raw proof.
- [x] Clean dedicated compiler/package build and scratch-install consumer pass.
- [x] Read-only downstream reproducers and out-of-tree downstream validation
      pass or retain the exact downstream removal seam.
- [x] Full required correctness/performance/sanitizer/package validation passes.
- [x] `git diff --check` passes.
- [x] CREXX final branch/HEAD/status/diff/test-count audit retained.
- [x] crexx-rag final audit proves its complete pre-existing state unchanged.
- [x] Downstream unblock matrix, issue-closure text, and resumption prompt are
      complete.

Final consolidated deliverable:
[`CREXX-RAG-INTEGRATION-CLOSEOUT.md`](evidence/2026-07-29-crexx-rag-integration-ledger/CREXX-RAG-INTEGRATION-CLOSEOUT.md).

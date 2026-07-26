# PERF2-05 RXAS semantic assists and instruction improvement worklist

Status: P05-CF1 complete — broader PERF2-05 remains in progress

Started: 2026-07-26

Purpose: prove whether `rxc` can partially evaluate ordinary Level B callable
bodies from certified RXAS semantics, so equivalent user-written and future
functions receive the same optimization without adding BIF-specific evaluator
code. The accepted PERF2-04 certified-call implementation remains the product
baseline and comparison oracle until the generic path proves exact equivalence.

## Decision boundary and first stop

P05-CF1 began as an architecture proof before production replacement. Adrian
approved the favorable generic implementation on 2026-07-26, with optimizing
user-written and future functions "for free" as the justification. That
approval does not authorize a public RXAS opcode, RXBIN format/ABI change or
changed legacy opcode semantics.

**First stop:** report the `LENGTH` plumbing and `SUBSTR` composition verdict,
the cursor/effect contract review, and whether `WORD` can serve as the generic
acceptance case. If the proof requires a new public instruction or changes an
observable legacy cursor effect, stop for Adrian's explicit ISA decision before
installing it. After any selected production compiler edit, obey the mandatory
first profiling-off Release verdict in `performance/AGENTS.md` and stop again
before broad closeout.

## Approved objective and acceptance criterion

Adrian approved the investigation order on 2026-07-26. The justification is
not another closed list of core BIF cases: the selected design must optimize
semantically equivalent user-written and future functions "for free" once
their ordinary instruction bodies and inputs provide sufficient proof.

The proof order is:

1. `LENGTH` proves the minimal `rxc` evaluation plumbing.
2. `SUBSTR` proves useful multi-instruction composition against the existing
   hard-coded certified evaluator.
3. The RXAS instruction review classifies logical value effects, cursor inputs,
   specified cursor outputs and incidental/cache cursor mutations, then locks
   the result into metadata, documentation and focused tests.
4. `WORD` is the architecture acceptance case for branches, a bounded loop and
   string scans. A certified `WORD` call and an equivalent user-written
   function must fold with the hard-coded BIF evaluator disabled.
5. Only a favorable proof advances to production implementation and migration.

## Exact starting state

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- HEAD: `22dd01a5b2e98e7e05682141b025633f8aecdd0a`
- HEAD subject: `docs: close PERF2-04 BIF campaign`
- Accepted PERF2-04 production commit: `f8f34092e`
- Upstream: `origin/develop` at `d1c5245d4`; local branch ahead 3, behind 0
- Starting worktree: clean
- Host: `Adrians-MacBook-Air.local`, `Mac17,3`, Apple arm64, 10 logical CPUs,
  24 GiB RAM
- OS: macOS 26.5.2 build 25F84; Darwin 25.5.0
- Toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2, Git 2.50.1
- Power at start: AC attached, battery 80%, AC/Battery low-power mode 0
- Load at start: 1.70 / 1.53 / 1.71; approximately 630 GiB free
- Existing ordinary build: Ninja Release, `CREXX_VM_PROFILING=OFF`
- Existing attribution build: Ninja Release, `CREXX_VM_PROFILING=ON`
- Existing QA build: Ninja Debug, `CREXX_VM_PROFILING=OFF`

Recapture power, thermal/load and exact product hashes around any timing block.

## Scratch and comparison products

No competing prototype may overwrite the accepted ordinary product or share a
mutable build directory with another candidate.

| ID | Source/build identity | Purpose |
| --- | --- | --- |
| P05-B0 | clean detached `22dd01a5b`, independently named Release/Debug builds | accepted PERF2-04 oracle and fallback |
| P05-LEN1 | detached B0 plus generic evaluator plumbing | single-instruction `LENGTH` proof |
| P05-SLC1 | detached LEN1 plus stateful composition | current Level B `SUBSTR` proof |
| P05-EFX1 | source/effect audit and isolated metadata/test candidate | cursor/effect contract proof; no implicit ISA selection |
| P05-WRD1 | selected clean-effect candidate plus bounded control-flow evaluation | certified and user-written `WORD` acceptance |
| P05-PROD1 | separately selected production patch after the panel | migration from the PERF2-04 oracle, if approved |

Exact scratch roots, patch hashes, configure options and executable hashes are
recorded before each candidate is used as evidence.

Initial detached root:
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-perf2-05.KM0ZeuBw5m`.
Its `baseline-src` and `candidate-src` worktrees both resolve exactly to
`22dd01a5b2e98e7e05682141b025633f8aecdd0a`; their build directories remain
separate.

## Semantic and state invariant matrix

| Invariant | Proof requirement |
| --- | --- |
| Text unit | Valid UTF-8 and Unicode codepoint behavior, including multibyte, combining and embedded U+0000 cases, matches both VMs. |
| Indexing | Public one-based positions and internal zero-based cursors convert exactly once; empty, boundary, after-end, zero and negative cases retain current signals/results. |
| Value state | Payload, logical byte/codepoint lengths, byte/codepoint cursor and representation validity relevant to later instructions are modeled, not merely the destination text. |
| Cursor contract | Each instruction is classified as cursor preserving, consuming, resetting or clobbering/unspecified for every operand. Observable legacy behavior is preserved unless separately approved. |
| Aliasing | Destination/source and repeated-register forms reproduce the VM's read-before-write and cursor behavior. |
| Control flow | Only proved constant branches and finitely bounded loops execute; ambiguity, unsupported flow or resource-budget exhaustion retains the original call. |
| Signals | Invalid UTF-8, range, numeric and other signal identity/order are never converted into a compile-time success. Initially only proved non-signalling cells fold. |
| Observation | The caller source event remains on a folded optimized result; no-opt retains the normal call/body. Reached signals and unsupported observable operations fail closed. |
| Provenance | Certification authenticates first-party callable identity/body where required; foldability itself is derived from the body and instruction semantics so user functions can qualify. |
| Forms | Local, source-import and binary-import bodies plus `rxvm` and `rxbvm` are distinguished wherever transported evidence is used. |

## Instruction review schema

For every instruction reachable in the selected bodies, record:

- opcode/form and exact operand types;
- explicit payload reads, writes and kills;
- cursor/representation inputs and outputs for every operand;
- logical effect versus incidental cache mutation;
- alias/read-before-write behavior;
- signal and control-flow behavior;
- whether compile-time evaluation is exact, bounded and context-independent;
- proposed metadata classification and canonical evaluator owner; and
- legacy public, compiler-private or new public placement disposition.

The first known discrepancy is bounded and must be resolved before trusting the
database for evaluation: `fndblnk` and `fndnblnk` are currently classified as
reading their source register, while the documented UTF implementation may
change that source's cursor and `getstrpos` can observe it.

### Completed cursor/effect review

The review confirms that cursor state is observable value state rather than an
optimizer-private cache. The current public semantics remain unchanged:

| Family | Cursor contract | P05 disposition |
| --- | --- | --- |
| `setstrpos` / `getstrpos` | functional write / observable read | record separately and evaluate exactly |
| `substr` / `substring` | read source cursor; reset destination cursor | record and evaluate exactly |
| indexed `strchar` | incidental source-cursor write in the UTF path | record conservatively; inlining must isolate a by-value formal |
| `fndblnk` / `fndnblnk` | UTF scan may write source cursor; ASCII fast path preserves it | record conservative source-cursor write; keep data-dependent runtime behavior |
| `hexchar`, `poschar`, `concchar`, `transchar`, `dropchar` | existing UTF implementations may move source cursors | record current effects; do not change public behavior in P05-CF1 |
| string copy/case/append/pad/trim/truncate destinations | reset, advance or otherwise write destination cursor state | record current destination effect alongside payload writes |

The generic evaluator keeps cursor state in its detached private environment,
so `WORD` reaches the literal ceiling without a new instruction. A focused
dynamic inlining guard proves that the newly recorded scan-source write causes
the required by-value isolation; the caller's cursor remains unchanged on both
VMs. Removing incidental cursor effects remains a possible separately approved
ISA cleanup, not a prerequisite or implicit part of this slice.

## Design-selection panel

| ID | Candidate | Generality | Compatibility/maintenance | Decision gate |
| --- | --- | --- | --- | --- |
| CF-A | Keep the PERF2-04 per-BIF C evaluators | none beyond registered cases | accepted oracle, but semantic logic can drift from Level B/RXAS | Control only; not the target architecture. |
| CF-B | Invoke the compiled Level B callable in a compiler-hosted VM | user/future bodies if safely sandboxed | reuses runtime semantics but adds compiler bootstrap, signal, determinism and lifecycle costs | Compare only if direct instruction evaluation cannot remain small and exact. |
| CF-C | `rxc` detached-body partial evaluator driven by the canonical opcode database | ordinary certified and user functions | earliest stable owner; requires exact state/evaluator metadata and bounded CFG execution | Preferred proof candidate. |
| CF-D | Add explicit-position/result-only string instructions, retaining cursorful legacy forms | reusable at runtime and compile time | possible handler/ISA surface; may be compiler-private or public | Consider only where existing composition cannot express a clean contract; public form needs separate approval. |
| CF-E | Change existing cursorful instruction semantics in place | broad but potentially breaking | observable through `getstrpos`; RXBIN/RXAS compatibility risk | Rejected unless the review proves the effect formally unspecified and Adrian approves the contract change. |

### P05-CF1 proof verdict and selected production scope

CF-C is selected. The bounded evaluator executes the resolved callable's exact
Level B body in `rxc`; it has no callable-name or BIF registry. Only constant
actuals/defaults, supported typed expressions, bounded branches/loops and
opcode-database entries carrying an exact compile-time evaluator are admitted.
All unsupported statements, reached signals, negative scan cases not yet
proved, resource exhaustion and unknown instructions retain the original call.

The P05-LEN1, P05-SLC1 and P05-WRD1 proofs all succeed with the hard-coded
PERF2-04 evaluator removed. Equivalent unregistered user `LENGTH`, complete
selected-domain `SUBSTR`, cursor set/get and Unicode `WORD` bodies fold. The
same engine folds all 32 selected PERF2-04 calls. Same-summary spoof providers
now fold according to their actual opposite bodies, eliminating identity/body
drift rather than trusting a BIF name.

PoC artifact comparison against the accepted compiler:

| Cell | Accepted | Generic candidate | Result |
| --- | ---: | ---: | --- |
| ordinary user-body executable RXAS | 196 | 7 | -189 (-96.43%) |
| ordinary user-body RXAS bytes | 33,992 | 864 | -33,128 (-97.46%) |
| ordinary user-body RXBIN bytes | 13,144 | 1,552 | -11,592 (-88.19%) |
| existing selected PERF2-04 RXAS | 3,311 bytes | 3,311 bytes | byte-identical |

Both ordinary-user artifacts produce identical output on `rxvm` and `rxbvm`.
The selected production scope replaces the hard-coded registry/evaluators with
the bounded body evaluator, adds exact cursor/evaluator opcode metadata, makes
cursor effects participate in formal read/write proof, and installs focused
local/import/no-opt/dual-VM/cursor guards. It does not change Level B sources,
public RXAS/RXBIN/ABI, VM handlers or legacy cursor behavior.

## Advancement ledger

- [x] Exact live repository, upstream, dirty scope, host/power/toolchain and
      build configurations recorded.
- [x] Approved objective and generic acceptance criterion recorded.
- [x] Independently identifiable comparison products defined.
- [x] Semantic/state matrix, instruction-review schema and design panel written
      before compiler production coding.
- [x] P05-LEN1 matches the accepted evaluator and folds an equivalent
      user-written `strlen` body.
- [x] P05-SLC1 matches the accepted `SUBSTR` evaluator for the admitted semantic
      domain and fails closed elsewhere.
- [x] P05-EFX1 classifies and mechanically checks every instruction needed by
      LEN1/SLC1/WRD1.
- [x] P05-WRD1 folds certified and equivalent user-written bodies with the
      hard-coded BIF evaluator disabled.
- [x] Production Release comparison records compiler time in addition to the
      completed PoC instruction/artifact/correctness comparison.
- [x] Adrian approved the favorable generic production implementation; stop
      reached at its mandatory first ordinary profiling-off Release verdict.
- [x] Adrian accepted the first Release verdict on 2026-07-26.
- [x] Review-derived scalar-result, callable-scope and `EXPOSE` fences added;
      intended generated-output changes replayed against runtime companions.
- [x] Final complete Debug CTest is 1,920/1,920; focused Release CTest is 6/6
      plus the opcode-metadata audit.
- [x] Final reviewed RXAS and same-input-path RXBIN are byte-identical to the
      timed candidate, preserving the accepted 22-pair verdict exactly.
- [x] Raw decisive/guard samples, generated artifacts, QA logs, provenance and
      recursive checksums retained under
      `performance/evidence/2026-07-26-perf2-05-generic-partial-evaluation/`.

## P05-CF1 first ordinary Release verdict

The implementation froze after the focused Debug checks. The ordinary Release
product used `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF` and
`-O3 -DNDEBUG`. The smallest decisive cell calls an unregistered user `WORD`
body with constant actuals two million times and validates its checksum.

| Measure | Accepted compiler | P05-CF1 | Change |
| --- | ---: | ---: | ---: |
| executable RXAS instructions | 62 | 17 | -45 (-72.58%) |
| RXAS bytes | 10,438 | 2,674 | -7,764 (-74.38%) |
| RXBIN bytes | 5,207 | 2,311 | -2,896 (-55.62%) |
| single observed `rxc` wall time | 0.27 s | 0.24 s | observation only |
| `rxvm` median, 22 balanced pairs | 184.5295 ms | 7.6735 ms | -95.868% paired median |
| `rxbvm` median, 22 balanced pairs | 199.8640 ms | 9.5755 ms | -95.128% paired median |

All 92 recorded decisive-cell samples passed. The paired mean 95% intervals are
[-95.965%, -95.650%] for `rxvm` and [-95.215%, -95.057%] for `rxbvm`.

The unchanged accepted RexxCPS image and library then guarded the changed
runtime executable layout. At the governed cap of 36 balanced pairs per VM,
`rxvm` has paired median +0.028% and mean -0.549% (95% interval
[-1.627%, +0.530%]); `rxbvm` has paired median -0.733% and mean -0.445%
(95% interval [-1.330%, +0.440%]). Both are neutral/inconclusive with no
regression guard. The accepted library and exit image are byte-identical; VM
file sizes are unchanged.

Focused correctness at the decision stop was 6/6 CTests plus the Release
opcode-metadata test. Adrian accepted that verdict on 2026-07-26. Review then
found and corrected an array-result admission bug, an over-broad early
inlining-summary pass and a missing procedure-level `EXPOSE` fence. The final
complete Debug suite passes 1,920/1,920 and the final focused Release suite
passes 6/6 plus opcode metadata.

The final reviewed compiler reproduces the timed RXAS byte-for-byte. RXBIN
stores the RXAS input pathname; using the original pathname reproduces the
timed 2,311-byte RXBIN byte-for-byte. The accepted wall-clock evidence is
therefore exact for the final generated program. Retained closeout evidence is
under
`performance/evidence/2026-07-26-perf2-05-generic-partial-evaluation/`.

P05-CF1 is complete and independently revertable. The broader PERF2-05
activity stays open for separately profile-selected semantic assists or
instruction placement. This slice neither selects nor pre-approves a public
opcode, RXBIN/ABI change, VM handler or change to observable cursor semantics.

## P05-SA1 profile-selected semantic-assist panel

Status: in progress — R2a accepted and closed green; R1a remains a separate rung

Started: 2026-07-26

Purpose: complete the remaining PERF2-05 search against the post-P05-CF1
product. Rank reusable semantic units and instruction improvements from fresh
static, dynamic and RXSEQ evidence; compare their efficient owners; retain
neutral and rejected cases; and stop for Adrian's production selection before
installing any ladder.

### Exact starting state and approval stop

- Branch: `develop`
- HEAD: `537d3b3d276606767535ecb84ad2a3c80073e5dd`
- Subject: `perf: generalize constant call evaluation`
- Upstream: `origin/develop` at
  `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`
- Ahead/behind: `+4/-0`
- Starting worktree: clean
- P05-CF1 retained manifest: 52/52 entries replayed successfully from the
  bundle root with `shasum -a 256 -c checksums.sha256`

This slice may create isolated analysis products and reversible PoCs, but it
must not install production code or select an irreversible owner. It stops
after the refreshed ranking, semantic/effect panel, placement comparison,
neutral/rejected ledger and recommended independently measurable ladder are
complete. A public RXAS instruction, serialized RXBIN/ABI change, observable
legacy cursor change, language decision or irreversible architectural owner
requires Adrian's separate approval.

No full formal portfolio, broad closeout, commit or push belongs before that
selection. `WORD` is an acceptance/semantic guard rather than a preferred
candidate. P05-CF1 is reopened only if an exact new profile proves a deficit.

### Independently identifiable products

| ID | Identity | Purpose |
| --- | --- | --- |
| P05-SA-B0 | clean detached `537d3b3d2`; separately named profiling-off Release and profiling-on attribution builds | post-P05 accepted current-product baseline |
| P05-SA-NO0 | B0 source and libraries compiled with ordinary no-opt workload images | expose sequences hidden by current compiler/RXAS composition; diagnostic only |
| P05-SA-C1 | separate detached/scratch product for compiler-owned result-only or existing-composition controls selected by the completed panel | earliest-owner ceiling; never overwrites B0 |
| P05-SA-R1 | separate detached/scratch product for a canonical private/runtime form or quickening control selected by the completed panel | runtime-placement ceiling with canonical RXBIN unchanged |
| P05-SA-V1 | separate detached/scratch product for a cleaned/new public RXAS-form control, built only if the panel needs it | public semantic-contract ceiling; no production assignment or serialized change |
| P05-SA-N1 | isolated native helper/control for the exact shortlisted semantic unit, when useful | upper bound only, not a default owner |

Exact scratch roots, source commits/patch hashes, configure options, executable
hashes, workload/image hashes and profiling modes are recorded before a product
is used as evidence. Candidate products remain separable and revertable; no
candidate shares a mutable build directory with B0 or another candidate.

### Evidence and ranking contract

The refresh distinguishes optimized and no-opt images where they answer the
selection question and reports `rxvm` and `rxbvm` separately. N=2/3/4 RXSEQ
windows are used only where the additional length changes semantic-unit or
placement selection. Each candidate row records:

- dynamic executions, static sites and distinct modules;
- optimized/no-opt and dual-VM persistence;
- the non-overlapping machine-work ceiling rather than a sum of overlapping
  sequence windows;
- native/instruction time, calls, scans, copies, allocations, representation
  crossings and artifact/local pressure where available;
- exact value and cursor effects, liveness, aliases/references and
  intermediate-write observability;
- validation, signal and throw order, TRACE/source identity and local,
  source-import and binary-import implications; and
- the disposition and evidence needed to reopen any neutral or rejected case.

Every shortlist compares existing compiler composition, compiler-owned
result-only lowering, cleaned/new public RXAS form, canonical private/runtime
form, quickening where appropriate and a native upper bound. A PoC advances to
ordinary profiling-off Release timing only after exact semantic correctness
and fewer instructions, scans, copies, allocations or representation
crossings are both proved. The smallest decisive cell is used; neutral and
negative evidence is retained.

### P05-SA1 advancement ledger

- [x] Exact repository/upstream/dirty state verified.
- [x] P05-CF1 checksum manifest independently replayed 52/52.
- [x] Numbered execution plan presented to Adrian.
- [x] Bounded successor scope, product identities and approval stop recorded
      in the existing PERF2-05 worklist.
- [x] Exact B0/NO0 scratch roots, build identities and product hashes recorded.
- [x] Post-P05 optimized/no-opt dual-VM profile and selected N=2/3/4 RXSEQ
      evidence retained.
- [x] Ranked candidate panel completed without overlap double-counting.
- [x] Shortlisted semantic/effect and placement panels completed.
- [x] Bounded PoCs pass correctness plus machine-work gates; neutral/negative
      results retained.
- [x] Recommended separable production ladder and justified breadth presented
      at the mandatory Adrian selection stop.

### P05-SA1 selection stop

The exact List reference-getter slice supports partial materialization of the
weak-reference descriptor, not the target object. Its canonical direct
attribute-copy ceiling removes 7,641,200 dynamic instructions and improves the
ordinary Release List cell by 6.173% on `rxvm` and 6.154% on `rxbvm`. Exact
relink independently removes 3,827,800 instructions and improves it by 2.253%
and 1.623%. The combined compatibility cell is favorable, but the production
rungs remain independently revertable.

The `ICOPY; BR` control removes 1,366,001 Base64 instructions but is neutral:
the ordinary Release medians are +0.318% on `rxvm` and -4.070% on `rxbvm` with
5.295-8.609% relative MAD. It does not justify a public or private fused form.

Recommendation for Adrian's decision:

1. select R2a canonical direct reference-descriptor materialization first;
2. select R1a exact relink second as a separate rung;
3. assign eligibility/TRACE/fallback proof to compiler/RXAS analysis and use a
   private execution form only where existing composition cannot express the
   result without the temporary;
4. keep canonical RXBIN and public RXAS unchanged; and
5. leave B1 neutral unless a later compiler-owned result-forwarding PoC gives
   stable multi-workload evidence.

The retained, recursive-checksum-closed package is
`performance/evidence/2026-07-26-perf2-05-semantic-assist-panel/`. No production
code, public opcode, full portfolio, commit or push was authorized or made.

### Adrian selection and R2a production gate

Adrian approved recommendations 1-4 on 2026-07-26: implement R2a first, keep
R1a as the second independently revertable rung, assign eligibility and
fallback proof to the compiler/RXAS/canonical-sequence layer with private
execution only where required, and keep public RXAS plus canonical RXBIN
unchanged. Adrian confirmed that recommendation 5 is R2b and directed that its
need be reviewed at the end of R2a.

The mandatory first ordinary Release gate applies before R1a begins. R2a must
freeze after minimum focused correctness, run the smallest decisive List
comparison against the retained valid baseline, review the residual canonical
copy cost/R2b case, report, and stop for Adrian's direction.

#### R2a production design selection

| ID | Form | Compatibility and semantic reading | Decision |
| --- | --- | --- | --- |
| R2-0 | Retain `LINKATTR1; COPY; UNLINK` | Exact public composition and fallback; three dispatches and a temporary alias | Baseline/fallback. |
| R2-C | Change compiler/RXAS output to a new direct operation | Earliest authored owner, but requires a public spelling or serialized marker to remove runtime dispatch | Rejected for R2a because public RXAS/RXBIN must remain unchanged. |
| R2-P | Recognize the exact canonical sequence while preparing the process-local execution image; select a private handler that retains canonical `copy_value` | No public opcode or serialized change; exact-shape eligibility, reference-descriptor guard and complete canonical fallback | Selected production form. |
| R2-Q | Learn/patch a site after execution | Adds state, transition and invalidation cost although the sequence shape is statically stable | Rejected; preparation-time selection is earlier and sufficient. |
| R2b | Copy only the reference payload/cell fields | Potentially narrower than canonical `copy_value`, but adds a separate shape/identity/cleanup proof | Review only after the R2a Release verdict shows residual cost. |

R2-P is eligible only for the exact immediate-attribute sequence whose `COPY`
source and final `UNLINK` are the `LINKATTR1` temporary and whose copy
destination is distinct. The private handler bounds-checks before mutation,
uses canonical `copy_value` for reference descriptors, restores the temporary's
base mapping, and skips the two remaining canonical instructions. It falls
back to the untouched sequence for non-reference payloads, debug/breakpoint
execution and every non-matching shape. Canonical instruction cells and
metadata remain the source of signal/source identity.

#### R2a ledger

- [x] Adrian approved R2a then independently revertable R1a; R2b review is at
      the end of R2a.
- [x] Status quo, compiler/public, private preparation, learned and R2b forms
      compared before production coding.
- [x] Exact process-local recognizer/private handler implemented with canonical
      fallback and no public/RXBIN change.
- [x] Focused reference, bounds, fallback, TRACE/breakpoint, import, no-opt and
      dual-VM correctness passes.
- [x] Implementation frozen and ordinary profiling-off Release product built.
- [x] Smallest decisive exact-input List verdict retained against the accepted
      R2 control.
- [x] Residual canonical-copy evidence reviewed for R2b and first Release
      verdict reported; stop reached before R1a.
- [x] Adrian accepted the favorable R2a verdict and authorized completion.
- [x] Full Debug and ordinary profiling-off Release builds pass; both broad
      CTest configurations pass 1,922/1,922.
- [x] Worklist, roadmap and retained evidence reconciled with R2b deferred and
      R1a not started.

#### R2a first Release verdict and R2b review

The accepted work-100 List cell is clearly favorable in 12 serial,
position-balanced pairs after one warmup per cell. The paired median is
-2.731% on `rxvm` with 11/12 favorable pairs and a mean 95% interval of
-3.230% to -1.665%. It is -1.745% on `rxbvm` with 12/12 favorable pairs and a
mean 95% interval of -2.565% to -1.478%. All four absolute cells remain below
the noise rerun thresholds. Focused Debug passes 10/10, the compiler/import/
no-opt matrix passes 49/49, and the ordinary Release compatibility guard passes
both integrated and retained baseline VMs.

R2b is deferred, not selected. The scratch public `COPYATTR1` ceiling and R2a
both use canonical `copy_value`, so the smaller production gain does not
isolate canonical value-copy cost and cannot justify a payload-only helper.
R2b requires separate post-acceptance attribution plus complete reference-shape
and lifetime proof. Evidence is retained under
`performance/evidence/2026-07-26-perf2-05-r2a-first-release-verdict/`.

Adrian accepted R2a on 2026-07-26. Post-verdict closeout is green: the full
Debug and ordinary profiling-off Release products build, and both broad CTest
configurations pass 1,922/1,922. R2b remains deferred for separate attribution
and proof; it was not selected or implemented. R1a remains the next separate
rung and was not started. R2a is complete, while the broader PERF2-05 activity
stays open for that separately authorized work.

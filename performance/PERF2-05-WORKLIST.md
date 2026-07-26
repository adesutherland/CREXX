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

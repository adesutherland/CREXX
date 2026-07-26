# PERF2-04 inlining-first core Level B BIF campaign worklist

Status: complete — accepted ladder closed locally

Started: 2026-07-25

Purpose: resumable control plane for the current-profile Level B callable
census, semantic/machine-ceiling comparison, bounded PoC panel and production
ladder recommendation. The maintained Level B implementation remains the
complete fallback and behavior documentation. This activity does not authorize
a production BIF, compiler, RXAS, RXBIN, VM or native implementation.

## Decision gate and mandatory stop

PERF2-04's current deliverable is the completed decision package: current BIF
census, per-family semantic proof, cleaned/hand/algorithm/compiler/assist/native
comparison, focused correctness and bounded ordinary Release evidence, final
dispositions and a recommended ordered production ladder.

**Stop point:** after the ranked panel and recommended ladder are complete,
set the activity to `decision required`, report to Adrian and stop. Do not
install the production ladder, run the full formal portfolio, commit or push.
A public RXAS instruction, serialized RXBIN change, ABI change, language change
or irreversible architecture choice additionally requires explicit approval
before any production implementation.

If Adrian later selects a production slice, the mandatory first profiling-off
Release verdict in `performance/AGENTS.md` applies before broad QA or closeout.

On 2026-07-25 Adrian approved the complete ordered ladder: P04-CAS1,
P04-SLC1, then P04-WRD1. This authorization does not remove the mandatory
per-slice first-verdict boundary. Implement P04-CAS1 provisionally, freeze it
after minimum focused correctness, compare the ordinary profiling-off Release
product with retained valid baseline evidence, report the verdict and stop.
Do not begin P04-SLC1 until Adrian accepts that verdict.

Adrian accepted P04-CAS1's mandatory first Release verdict on 2026-07-25.
The accepted implementation removes `arg expose` from the maintainable
`UPPER` body and uses general classified RXAS operand effects, owned scalar
result placement and bounded candidate-local default-initialization proof; it
contains no UPPER/STRUPPER identity rule. The ordinary profiling-off Release
RexxCPS medians are 31,658,995/29,725,073 clauses/s, +7.900820%/+7.745957%
against the retained valid baseline on `rxvm`/`rxbvm`, with 0.334306%/0.477375%
relative MAD. Adrian accepted that verdict and authorized P04-SLC1 to begin.

Before the first P04-SLC1 compiler edit, Adrian reopened and approved its
placement as a general **certified-call constant-evaluation** facility rather
than a `SUBSTR`-specific rule. The certification universe is the deterministic,
non-I/O, non-random core Level B BIF surface, but membership in that universe
does not itself certify a callable: every enabled entry still requires an
exact callable/signature/body proof, declared context/effect/signal/ownership
contract and a canonical evaluator. P04-SLC1 initially enables separable
`UPPER` and proved-domain `SUBSTR` certificates. All other core BIFs remain
eligible but uncertified and retain their complete Level B implementation.
No public RXAS/RXBIN/ABI metadata change or general loop-invariant motion is
authorized by this decision.

P04-SLC1's frozen provisional implementation completed its mandatory first
ordinary profiling-off Release verdict on 2026-07-25. Against the accepted
P04-CAS1 medians, RexxCPS improves by 14.436115%/14.060120% on
`rxvm`/`rxbvm`, to 36,229,324/33,904,454 clauses/s. All four warmups and all
14 recorded executions pass; relative MAD is 0.863585%/0.077412% and neither
cell requests a rerun. The exact product removes 102 executable RXAS
instructions, all four constant `strupper` scans and both selected constant
`SUBSTR` bodies; main locals fall 105 to 100 and RXBIN falls 78,162 to 72,106
bytes. The accepted linked library and both VMs remain byte-identical. This is
the mandatory stop before P04-WRD1, broad QA, commit or push.

Adrian accepted the favorable P04-SLC1 verdict on 2026-07-25 and directed a
bounded generalization pass before P04-WRD1. P04-CEX1 qualifies the exact
current `LENGTH`, `LEFT`, `RIGHT` and `LOWER` core Level B bodies for the same
general certified-call registry. Each callable must independently satisfy the
existing exact identity/signature/I6/body/effect contract, preserve the Level B
body as the complete fallback and remove machine work at its admitted constant
use sites. Current Tier A neutrality is recorded separately and does not reject
a generally advantageous certificate. Invalid,
signalling, unsupported or compile-time-unprofitable domains retain the normal
call. P04-CEX1 receives its own focused correctness, generated-artifact and
first ordinary profiling-off Release verdict before any WRD1 work begins.

Adrian also selected P04-WRD1 as the successor once P04-CEX1 is complete. That
selection removes the need for another architecture-selection round, but not
the mandatory P04-CEX1 first-verdict report and stop. WRD1 remains unstarted
until Adrian accepts that verdict.

P04-CEX1's frozen provisional implementation completed its mandatory first
ordinary profiling-off Release verdict on 2026-07-25. The exact focused cell
drops 286 to 20 executable RXAS instructions, 12 to zero locals and 16,336 to
2,400 RXBIN bytes. On the repeated four-family cell, all 17 retained samples
give medians of 1,092,735,000 to 51,803,000 ns on `rxvm` and 1,250,143,000 to
56,995,000 ns on `rxbvm`, 21.094049x/21.934257x faster. The required ten-sample
append is retained because the short candidate processes remain noisy; every
candidate maximum nevertheless remains at least 18.156x below the matching
SLC1 minimum. The current linked library, RexxCPS and rxpp products are
byte-identical to SLC1, so no Tier A or RexxCPS gain is claimed. This is the
mandatory stop before the already-selected P04-WRD1 successor.

Adrian accepted the favorable P04-CEX1 verdict on 2026-07-25. P04-WRD1 is now
the active provisional slice under the previously approved ladder. Its scope
is the compiler-owned `WORD(source, 1)` value/consumer composition selected by
the decision panel; the complete Level B body remains the fallback. Freeze
after minimum focused correctness, run the first ordinary profiling-off
Release verdict against the accepted CEX1 product, report it and stop before
broad QA, closeout, commit or push.

P04-WRD1's frozen provisional implementation completed that first verdict on
2026-07-25. The general certified-call registry now has one exact `WORD`
certificate, and the ordinary fold/propagate fixed point runs before and after
certified evaluation; there is no WORD-specific consumer rule. The timed
`word("Key Bee", 1) = "?"` path disappears while the setup-only dynamic
`word(version_info, 1)` call remains. Against accepted CEX1, RexxCPS improves
by 7.650234%/8.812155% to 39,000,952/36,892,167 clauses/s on
`rxvm`/`rxbvm`. All 18 invocations pass, relative MAD is
0.881617%/0.547875%, and neither cell requests a rerun. This is the mandatory
stop before broad QA, closeout, local slice commits or push.

Adrian accepted P04-WRD1's favorable first verdict on 2026-07-26. The complete
accepted ladder then completed the bounded closeout path: complete Debug
rebuild, focused affected-surface checks, full Debug CTest, retained-evidence
replay, diff review and local production packaging. The accepted cumulative
source is atomic commit `f8f34092e`; the four stable slice IDs retain separate
incremental verdict evidence. No push is authorized.

## Hard boundaries

- [x] Start from the maintainable Level B body and PERF2-03's accepted cleaned
      inliner; do not assume native ownership or the smallest compiler change.
- [x] Preserve the Level B source as complete fallback and behavior
      documentation for every prototype.
- [x] Keep the accepted PERF2-03 production compiler at
      `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`; the following HEAD is a
      documentation-only closeout until live evidence proves otherwise.
- [x] Treat PERF2-01 profiles as orientation only and refresh current-product
      BIF attribution after all five PERF2-03 slices.
- [x] Separate RexxCPS setup/reporting from its timed kernel. Known timed
      controls are `LENGTH`, `SUBSTR` and `WORD`; formatting output is not a
      RexxCPS cause.
- [x] Inspect generated optimized/no-opt RXAS and linked/imported forms before
      assigning cost to a Level B body or call scaffold.
- [x] Admit a PoC to product timing only after mathematical semantic
      equivalence and reduced instructions, scans, copies or allocations.
- [x] Keep every candidate separable and retain neutral/negative evidence.
- [x] Use only existing profiling/evidence orchestration where it answers the
      question. Any new maintained census, analysis or orchestration program is
      cREXX Level B, not Python.
- [x] Preserve public RXAS, canonical RXBIN, ABI, signal, source/TRACE and both
      VM contracts unless Adrian explicitly selects a changed boundary.
- [x] Preserve existing RexxDoc blocks and API companions; production behavior,
      signature, backing or return changes would update them in the same slice.
- [x] Do not reopen PERF2-03 generally. Import F03/F05 only for a selected hot
      case that supplies the exact reopen proof; route F01/F02/F04 normally.
- [x] Do not attribute already-enabled receiver/accessor inlining as a new
      PERF2-04 gain; guard it if a selected library refactor exposes it.

## Numbered execution plan

1. Freeze exact repository/upstream/host/power/toolchain state and predecessor
   evidence; define independently named baseline and PoC scratch products.
2. Inventory the complete current Level B library callable surface, imported
   inline evidence and current standalone/linked artifacts.
3. Recompile current governed workloads and focused cells against exact HEAD;
   capture static sites and current inlining results plus both-VM dynamic BIF,
   procedure, instruction, scan/copy/conversion/allocation evidence.
4. Separate setup/reporting from timed kernels, then rank every hot or
   size-significant family with a deciding end-to-end cell.
5. Complete each selected family panel: clean inline, hand-equivalent ceiling,
   best Level B algorithm, compiler composition/lowering, reusable assist,
   native upper bound and placement.
6. Run narrow semantic/generated-code checks after each isolated PoC, then the
   smallest bounded profiling-off Release target and one or two guards.
7. Retain exact sources, patches, commands, hashes, raw serial samples and
   neutral/negative outcomes; select or reject every measured candidate.
8. Recommend the justified ordered production ladder, set `decision required`,
   report to Adrian and stop without production installation or publication.

## Stage 0 - exact baseline and isolation

### Repository state at start

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- HEAD: `6567f0ba23f20623e01322f5a62323b2347ab09d`
- HEAD subject: `docs: close PERF2-03 and hand over PERF2-04`
- Upstream: `origin/develop` at
  `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`; ahead/behind `+1/-0`
- Accepted production predecessor: exact upstream commit above, subject
  `perf: complete proved receiver inlining`
- The HEAD-to-predecessor delta is documentation/evidence only: 12 paths,
  413 insertions and 32 deletions; binary diff SHA-256
  `7eaf1ba251515bb830cf95a07189975e40e0ac64784a6a23207eb9a76d3d66a3`.
- Starting worktree: clean; no staged or unstaged paths before this worklist and
  roadmap start record.

### Host, power and tool state at start

- Host: `Mac.lan`, model `Mac17,3`, Apple arm64, 10 logical CPUs, 24 GiB RAM.
- OS: macOS 26.5.2 build 25F84; Darwin 25.5.0.
- Toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2, Git 2.50.1.
- Power: AC attached, battery 80%, AC/Battery low-power mode `0`.
- Capture load: `3.06 2.22 1.81`; uptime 11 days 1:49.
- Storage: approximately 632 GiB free on the data volume and `/private/tmp`.
- Existing ordinary build configuration: Ninja Release,
  `CREXX_VM_PROFILING=OFF`.
- Existing attribution build configuration: Ninja Release,
  `CREXX_VM_PROFILING=ON`.
- Existing Debug configuration: Ninja Debug, profiling off. These existing
  trees are orientation only until exact-HEAD product hashes are rebuilt and
  recorded.

Formal or decision timing blocks must recapture pre/post power, low-power,
thermal/load state, run serially, and use ordinary profiling-off Release
binaries.

### Scratch product contract

Resolve actual absolute paths and hashes before first use. No PoC shares a
mutable build directory or overwrites the ordinary product baseline.

| ID | Source identity | Build identity | Purpose | Status |
| --- | --- | --- | --- | --- |
| B0-R | `/private/tmp/crexx-perf2-04.3k3Dfw/baseline-src` | `build-release`, Ninja Release, profiling off | authoritative exact-HEAD product and bounded wall baseline | complete; hashes in retained provenance |
| B0-P | same detached exact HEAD | `build-profile`, Ninja Release, profiling on | attribution only | complete; both VM hashes and current census retained |
| C0 | B0 source and products | no source change | clean source-inline control | complete for every selected family |
| H-* | detached exact HEAD plus one retained patch | `upper-direct-src`, `length-direct-src`, `substr-direct-src`, `word-predicate-first-src` | hand-equivalent ceilings | complete and separable |
| A-* | detached/source controls plus retained patch | Base64 codepoint/POSCHAR and codepoint-arithmetic Level B controls; safe WORD predicate control | best maintainable Level B algorithms | complete; Base64 A2 routed to CAP-03 |
| L-* | detached exact HEAD plus one retained patch | `upper-inline-src`, `upper-const-src`, `substr-const-src`, `word-constant-false-src` | compiler composition/constant/value ceilings | complete and separable |
| V-* | existing primitives and adoption-gate comparison | no new public/private instruction candidate built | reusable assist bound | complete; every selected family rejects a new assist |
| N-* | canonical existing VM primitive handlers; fixed-valid Base64 C control | no production/native BIF build | native/intrinsic upper bounds | complete; native ownership rejected |
| CF-COMB1 | `/private/tmp/crexx-perf2-04.3k3Dfw/rexxcps-combined-src` | exact CAS-L2 + SLC-CF1 + WRD-CF1 scratch image | cumulative ceiling and overlap check | complete; no production source change |

Every patch receives a stable candidate ID, exact baseline commit, patch
SHA-256, compiler/assembler/VM/library hashes, focused commands and result row.
Combination candidates are separately identified; no result is attributed to
an unlabeled mixture.

## Semantic-invariant matrix

The selected family records link every applicable invariant to at least one
positive and one distinguishing boundary/error case. Missing proof rejects only
that candidate/case and retains the complete Level B path.

| Invariant | Required proof and observations | Required forms |
| --- | --- | --- |
| Text unit | `.string` operations use Unicode codepoints over valid UTF-8, never storage bytes; combining codepoints remain separate | ASCII, multibyte, combining and embedded U+0000 |
| Index origin | Public positions are 1-based; VM cursors are 0-based; conversions occur exactly once and preserve end/out-of-range behavior | first, last, after-end, empty and negative/zero invalid cases |
| Width/length | Omitted length differs from supplied zero; supplied length/width is non-negative; exact boundary and beyond-end behavior is preserved | direct, source-import, binary-import and no-opt |
| Padding | Default blank and an explicit pad contain exactly one codepoint; output side and exact width are preserved | ASCII/multibyte pad, empty and multi-codepoint errors |
| Word semantics | VM Unicode whitespace defines word boundaries; blank runs are equivalent; word text remains strict/case-sensitive | empty, blank-only, mixed Unicode whitespace and phrase cases |
| Search semantics | Empty operands, positive start, whole match, overlap/repetition, embedded U+0000 and codepoint result position are exact | `rxvm`, `rxbvm`, optimized and no-opt |
| Case mapping | Current limited locale-independent simple table, including unchanged characters and embedded U+0000, is preserved; no new full-folding claim | direct/imported/no-opt and both VMs |
| Optional/status | `?arg`, defaults, omitted holes/status and evaluation order match a real call; default expressions execute only when required | local, source-import and binary-import |
| Numeric/typed conversion | Numeric context, exact integer range, representation crossing and conversion/signal identity/order are unchanged | success, boundary and typed conversion errors |
| Value ownership | By-value isolation, exposed/reference alias lifetime, result ownership, repeated/overlapping actuals and source cursors remain valid | alias/reference fixtures and both VMs |
| Observation | Output, validation, signal name/order/payload, source identity and TRACE anchors remain equivalent | optimized/no-opt and retained-source forms |
| Imported evidence | Any new I6 fact is reconstructed independently from body/declaration, compared exactly and contradicted by a focused CTest | local/source/binary plus missing/old/malformed evidence |

## Current BIF census schema

One row represents an exact callable in one current product/module form; a
separate workload-site table prevents static, dynamic and timing dimensions
from being conflated.

| Field group | Required fields |
| --- | --- |
| Identity | stable row ID, namespace/callable/signature, source and RexxDoc/API path, bootstrap module, direct/source-import/binary-import form |
| Inline state | body metadata/schema, eligibility, accepted/rejected result and exact reason, residual normal call sites |
| Static product | workload/module/site, optimized/no-opt instructions, calls, scans/slices/searches, copies/conversions/allocations, peak locals/registers, RXAS/RXBIN/linked bytes and hashes |
| Dynamic product | workload phase (`setup`, `timed kernel`, `reporting`, `whole cell`), both-VM call count, procedure self/child/native time, opcode counts/time and site/callee identity |
| Value work | formal/default/result initialization, general/typed copies, representation conversions/materializations, scan/slice/search count and bytes, allocation/value-transfer count and bytes |
| Semantics | output/checksum, signals, TRACE/source identity, optional/default/status, numeric context, alias/reference/result ownership and dual-VM status |
| Decision | smallest decisive end-to-end cell, machine ceiling, disposition, selected/rejected owner and evidence limitation |

Allowed initial dispositions are `clean Level B already at ceiling`,
`inline/cleanup opportunity`, `algorithm opportunity`, `general assist
candidate`, `native upper bound only`, `not currently material`, and `evidence
missing`.

## Complete design-selection table

This table is the bounded panel before production coding. A family may be
removed after the current census only with an explicit `not currently material`
or `evidence missing` result. Stronger companion designs receive a new stable
ID and remain separable.

| Family | Current clean inline | Hand-equivalent ceiling | Best Level B algorithm | Compiler lowering/composition | General assist control | Native control | Placement question |
| --- | --- | --- | --- | --- | --- | --- | --- |
| LEN - `LENGTH` | LEN-C0 exact body through I6/PERF2-03 | LEN-H1 direct destination `strlen` plus exact observation contract | LEN-A1 same source body with avoidable init/copy removed if source can express it | LEN-L1 proved result-only direct `strlen` composition | LEN-V1 none unless C0 cannot reach H1 for multiple sites | LEN-N1 direct runtime helper upper bound | Level B inline vs compiler-owned result placement; reopen F03 only on a material residual gap |
| SLC - certified constants, initially `UPPER` and `SUBSTR` | SLC-C0 complete Level B bodies through the accepted inliner | SLC-H1 exact canonical VM-value result for the certified semantic domain | SLC-A1 current maintainable source algorithms remain the fallback | SLC-L1 general pre-inline certified-call evaluator; `UPPER` constant mapping and `SUBSTR` supplied-positive/in-range cases are separate entries | SLC-V1 no assist; certificates use existing canonical value/string helpers | SLC-N1 native bodies remain upper bounds and cannot beat zero runtime work | compiler certificate registry with exact provider/body proof; eligible deterministic core BIFs remain disabled until individually certified |
| WRD - `WORD`/`WORDS`/`WORDPOS` plus span siblings | WRD-C0 current `fndnblnk`/`fndblnk` bodies | WRD-H1 one forward cursor plan with no repeated prefix scan or temporary substring | WRD-A1 shared word-span/count/phrase algorithm in Level B | WRD-L1 compiler composition when phrase/count/result facts are static | WRD-V1 reusable word-span/count/extract/search kernel | WRD-N1 direct runtime family upper bound | Level B algorithm vs reusable assist; public RXAS only after multi-site and authored-use proof |
| SRC - `POS`/`LASTPOS` and related search | SRC-C0 current wrapper over search primitives | SRC-H1 validated direct codepoint search with one result path | SRC-A1 only if current primitive composition repeats work | SRC-L1 inline validation plus direct existing `strpos`/reverse-search primitive | SRC-V1 general codepoint search only if current primitive does not own the ceiling | SRC-N1 direct runtime search upper bound | likely clean inline/compiler composition; reject opcode duplication if primitive dominates |
| CAS - `UPPER`/`LOWER` | CAS-C0 current exposed input plus one `strupper`/`strlower` | CAS-H1 one destination allocation/copy and one bounded simple-case pass | CAS-A1 current source unless a distinct allocation/pass algorithm wins | CAS-L1 direct compiler intrinsic only if call/inliner residual is material | CAS-V1 none unless reusable operation cannot reach H1 | CAS-N1 direct runtime helper/control | include only with current timed-product attribution; reporting-only RexxCPS use is non-material |
| CNV - profile-selected typed conversions | CNV-C0 exact current typed Level B body/primitives | CNV-H1 representation-preserving conversion with exact numeric-context/error path | CNV-A1 best typed algorithm avoiding repeated text crossings | CNV-L1 existing conversion opcode/composition under proved type/context | CNV-V1 private representation-preserving assist if multiple real sites remain | CNV-N1 direct conversion helper upper bound | prefer compiler/value owner; native is a bound, and broad representation work routes to PERF2-07 |
| B64 - Base64 position/copy path control | B64-C0 unchanged codec and current BIF calls | B64-H1 simplest equivalent byte/string update loop | B64-A1 Level B binary/string algorithm using existing direct byte primitives | B64-L1 compiler composition for proved binary/string operations | B64-V1 reusable semantic unit only after C0/A1/L1 comparison | B64-N1 native codec/control only | retain unchanged canonical workload; route value copying to PERF2-07 and semantic instruction work to PERF2-05 |

### Candidate advancement gate

- [x] Complete exact contract and mathematical equivalence for selected cases.
- [x] Reduce executable instructions, scans, copies or allocations against the
      best safe current form; image/register costs remain explicit.
- [x] Pass the narrow semantic/generated-RXAS checks in both VM modes.
- [x] Improve the smallest decisive ordinary profiling-off Release cell for
      every selected winner; retain LEN-H1 as a neutral rejection.
- [x] For an assist: prove multiple real sites, generality, fallback, startup,
      image/state, RSS, maintenance and a better owner than composition/private
      placement. No candidate passes this gate, so no assist advances.
- [x] Retain public-format/ABI/language decisions for Adrian rather than
      allowing a private PoC to create a de facto contract.

## Decision-gate result

- Current surface: 560 maintained Level B exported callables across 142
  modules; 238 carry current I6 bodies. Exact signature/module/artifact tables
  and optimized/no-opt current workload images are retained.
- Selected ladder: P04-CAS1 (`UPPER` exact-body composition/constant result),
  P04-SLC1 (`SUBSTR` proved composition/constant result), P04-CEX1 (bounded
  exact-certificate expansion to `LENGTH`, `LEFT`, `RIGHT` and `LOWER`), then
  P04-WRD1 (`WORD` consumer/value composition). Each is compiler-owned,
  independently testable/revertable, and retains the complete Level B body.
- Cumulative bound: CF-COMB1 reduces normalized instructions by
  46.523717%/46.534811% and improves ordinary Release RexxCPS by
  32.877919%/34.400093% on `rxvm`/`rxbvm`. All 36 invocations and all 28
  recorded samples pass; the candidate wins 7/7 paired rounds on each VM.
- Rejections/routes: dynamic LEN-H1 is neutral in the RexxCPS wall cell;
  constant `LENGTH` remains valuable through P04-CEX1. Base64 A2
  belongs to a separate Level B CAP-03 API; POS is already at its current
  primitive ceiling; word siblings and typed BIFs are not current timed
  material. No RXAS/RXBIN/ABI/VM/native production surface advances.
- Correctness route: valid empty-initialized `dcopy; dtos; strlen` exposes a
  stale codepoint-count bug on both VMs. The exact known-failing regression is
  retained as `PERF2-07-V3-R01`; PERF2-04 installs no fix.
- Evidence closure: the existing Level B inventory tool wrote and verified the
  original 1,442-entry recursive `checksums.sha256`; an independent replay was
  1,442/1,442. P04-CEX1's first-verdict
  sub-bundle independently verifies 11/11 with manifest SHA-256
  `c6eaff910e05432d98d1d2bac94dbb1bb92e7e8f9f3d9eb543d7781b2af48485`;
  P04-WRD1's first-verdict sub-bundle independently verifies 12/12 with
  manifest SHA-256
  `2c48472d9e48217ea876f081fd3c691aa9e407e9f74ccde8e7a46cef52ac53ee`.
  P04-SLC1 verifies 11/11 with repaired manifest SHA-256
  `98dc650f0d885cdee7a2c2c8b584c915c516c40bf9618f38611a7b38da58a0fc`.
  The maintained Level B inventory tool regenerated and verified the final
  1,485-entry recursive root manifest. Its SHA-256 is
  `ecadcd3e0ff3b81695897602a6c00d3db16feb49f1f5b15f2dfa12e97eca2d4f`.
- Scope stop: the accepted ladder is production commit `f8f34092e`; focused
  QA is 24/24 and final broad Debug QA is 1,919/1,919. No full formal
  portfolio, sanitizer/install/cross-platform expansion or push was performed.

## Approved production ladder ledger

- [x] Adrian approved P04-CAS1, P04-SLC1 and P04-WRD1 in that order.
- [x] Reject and remove the BIF-specific exact-body diagnostic implementation.
- [x] Adrian selects the revised general P04-CAS1 architecture.
- [x] P04-CAS1 provisional implementation and mandatory first Release verdict.
- [x] Adrian accepts the P04-CAS1 verdict before P04-SLC1 begins.
- [x] Adrian approves the revised general certified-call architecture and the
      deterministic non-I/O/non-random core Level B certification universe.
- [x] P04-SLC1 provisional implementation and mandatory first Release verdict.
- [x] Adrian accepts the P04-SLC1 verdict and requests bounded certificate
      expansion before P04-WRD1.
- [x] P04-CEX1 qualifies `LENGTH`, `LEFT`, `RIGHT` and `LOWER`, then completes
      its focused correctness and mandatory first Release verdict.
- [x] Adrian selects P04-WRD1 as the successor after P04-CEX1 completes.
- [x] Adrian accepts the P04-CEX1 verdict before P04-WRD1 begins.
- [x] P04-WRD1 provisional implementation and mandatory first Release verdict.
- [x] Adrian accepts the P04-WRD1 verdict before broad QA or closeout begins.
- [x] Broad QA, retained closeout evidence and one atomic local production
      commit for the cumulatively shared accepted mechanism; no push without
      separate authorization.

## Stage ledger

### Stage 0 - current state and census

- [x] Mandatory repository/performance/predecessor/governance reading complete.
- [x] Exact HEAD, accepted production predecessor and upstream relation verified.
- [x] Starting host, power, toolchain and existing build configurations recorded.
- [x] Resolve scratch paths and build exact-HEAD B0-R/B0-P products.
- [x] Verify retained PERF2-03 checksum bundle and record current artifact hashes.
- [x] Inventory every exported Level B callable and inline metadata surface.
- [x] Capture current optimized/no-opt static sites and both-VM dynamic counts.
- [x] Separate setup/reporting from timed kernels and rank dispositions.

### Stage 1 - semantic proof and full candidate panel

- [x] Freeze shortlisted families from current evidence.
- [x] Complete per-family invariant/test matrix and exact machine ceilings.
- [x] Build and compare every applicable C0/H/A/L/V/N variant, or retain the
      explicit gate that rejected an inapplicable assist/native build.
- [x] Record selected/rejected placement and successor ownership.

### Stage 2 - efficient PoC and measurement loop

- [x] Run target-only builds and narrow semantic/generated-code checks first.
- [x] Compare instructions/scans/copies/locals/artifacts before product timing.
- [x] Run deciding target plus one or two mechanism guards on both VMs.
- [x] Retain raw serial correctness-qualified profiling-off Release evidence.

### Decision package

- [x] Worklist complete through the decision gate.
- [x] Current BIF census and ranked dynamic/static ownership panel retained.
- [x] Per-family semantic and six-control comparison retained.
- [x] Focused correctness/generated-code/artifact evidence retained.
- [x] Bounded ordinary Release target/guard evidence retained.
- [x] Every measured candidate has a selected/rejected/neutral disposition.
- [x] Ordered independently provable/revertable production ladder recommended.
- [x] Roadmap set to `decision required`; report to Adrian and stop.

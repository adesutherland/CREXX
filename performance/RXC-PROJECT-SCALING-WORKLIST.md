# RXC-PROJECT-01: compiler and project-build scaling

Selected by Adrian on 2026-09-06. Status: both Release verdicts accepted by Adrian;
the repair is qualified on macOS and integrated into local develop. No global
installation or remote publication. Local develop was fast-forwarded from 3f5d9ffb6 to
e0e67ad3e89e19f98ab09676ffa2bbc99fad7010 with unrelated work preserved.

## Evidence and scope

Scratch root: `/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795`.
The read-only RAG donor is frozen there as `donor-frozen`; the SHA-256 manifest
is `evidence/donor-source-manifest.json` (131 files, manifest digest
bf8322b4f5c588729d17f1d5070950ee76be590ff6f606b0673511a53a82e921).
No recovered corpus or hosted provider is used. Diagnostic captures, including
NUL-delimited per-member argv, process samples and native time/RSS, are under
`evidence/`. Diagnostic captures with competing jobs are not formal timing.

The installed rxc and native crexx wrapper byte-match the occupied checkout's
Debug binaries. Their hashes are respectively
ee3ef98fb33d405024745ae10c34931ca1ff98eb217af73e3b0b83275620e422 and
2ad3a090b5050e49d693d5a8024ac6310d1c2ee10fa917a6a3b4c1fb94cf5d89.
rxc reports e908adeae093.dirty; installed BUILDINFO reports 32efa89a2daa.dirty.
The package combines artifacts of different ages. The exact historical dirty
source cannot be inferred from either version string. Tracked compiler core
is identical between e908adeae093 and e0e67ad3e; their compiler tests differ.
The clean e0e67ad3e Release package is the attributable comparison baseline.

## Mechanism and historical escape

* September 5 sample: 2095/2200 top-of-stack samples are in
  `ast_declares_local_contract`, called from inline reference-attribute checks.
  The declaration query recursively scans executable bodies. Repeated inline
  preparation multiplies these scans as those bodies expand. Diagnostic
  Release counters in imported ragproduct show 26,501 eligibility nodes at
  2.85 CPU seconds versus 55,604 at 48.75 CPU seconds in a later preparation.
* Declaration guard introduced by 9c1ed04bf3b27d109e8a1164704c9b383a2d3091
  (2026-04-21). Optimising imported source introduced by
  182a3f552b208bd60f56508be1def3e7a127aac2 (2026-04-29). Reference-bearing
  class eligibility checking added by 6d77cbf4b (2026-06-09). These are
  introduction points, not yet a complete workload bisect.
* Live diagnostic also samples `add_unused_import_warnings` traversing every
  imported function/class for repeated namespace pairs. Introduced by
  08b41eafe (2026-06-06). Keep this distinct from the inlining mechanism.
* Project action keys include every declared source and every source-root
  file; each member key derives from that global key. This guarantees a full
  wave on a single edit. ad2f98e4413fcd68d18d8268ca29598ef389060e
  (2026-09-01) introduced it, including a test requiring that full wave after
  appending a comment. The replacement and its selection, source-shadowing, option/toolchain
  invalidation tests are described below.

## Design selection before production edits

1. Status quo: repeat full mutable AST scans for declaration existence. No
   retained state, but body growth changes lookup cost despite declarations
   being unchanged. Measured pathological on the frozen input.
2. Narrow declaration lookup: inspect declaration-bearing file children after
   source structure normalization. `ast_source_structure_walker` requires
   classes/interfaces directly under PROGRAM_FILE; bodies cannot declare
   contracts. Preserve late local declaration protection and the current
   class/interface name comparison. No cache, allocation or cross-pass state;
   each query observes current declarations, including inserted declarations.
   This is the first isolated prototype.
3. Cache reference-class or eligibility answers: eager preparation, lazy
   memoization and per-pass storage can avoid more repeated work, but require
   invalidation for lazy import loading, rewritten ASTs, scopes and options.
   Do not select until the narrower form is measured; misses cannot be cached
   across import changes. Cross-process caching is outside this slice.
4. Stop eligibility scanning at the existing 300-node cutoff: safe for already
   rejected oversized bodies, but leaves expensive declaration scans in
   supported small bodies. Compare only if the owning declaration fix leaves
   this cost material; do not change the accepted inline shape or threshold.

This is a non-representative compiler/application workload; RexxCPS runtime
does not exercise this source-import/compiler mechanism. Ordinary profiling-off
Release compiler timings, accepted optimized output, source/binary import and
stale-declaration checks are the first verdict cells. Broad QA, downstream
offline execution and the requested local develop commit follow acceptance.

## First verdict

The declaration-scoped candidate passed 12 focused checks and reduced the
matched 48-member Release build from 509.45 to 107.23 seconds, with all 48
member assemblies and the linked image byte-identical. See
[evidence and limitations](evidence/2026-09-06-rxc-project-scaling-first-verdict/README.md).
No cache or cutoff change is selected. Adrian accepted this verdict and approved
continuation on 2026-09-06.

## Incremental selection design

The global key remains the cheap unchanged-project gate and final-link key.
Member action keys instead include their primary source, toolchain, compiler
options and ordered search roots. On a changed project, the compiler checks a
snapshot of its own selected import candidates before reusing each member.
The snapshot records full contents for loaded sources and binary candidates,
and the resolver's namespace header for unloaded sources. The check reruns the
existing ordered discovery and header scanner without parsing/optimising
callable bodies. New, removed, reordered or shadowing candidates therefore
invalidate the snapshot. Imported implementations remain dependencies because
they can be inlined. RXAS timestamp selection is checked even on an otherwise
unchanged project. Missing or malformed snapshots fail closed to compilation.

Alternatives rejected: deriving every member from the global key preserves the
measured full wave; parsing import syntax again in the wrapper risks differing
language semantics; tracking only loaded files misses namespace changes in an
excluded source; hashing all candidate source bodies recreates the full wave;
tracking only exported signatures misses cross-file inline implementation edits.
The retained snapshot belongs to one successful member action and is discarded
with its private member directory. It is never a compiler-process AST cache.

## Application incremental escape and second compiler mechanism

The first dependency-snapshot implementation passed independent-source tests,
but the full application's private `ragimprove` edit still selected 48 members.
Tracing `provider_contract` proved that validating a binary function declaration
from `library.rxbin` loads `rag_address_environment.crexx` (namespace `_rxsysb`),
whose imports then load nearly the whole application. Its snapshot legitimately
marks 46 of 48 source candidates as loaded. Do not omit those dependencies.

The first hypothesis was eager inline-payload decoding. Deferring that decoding
passed 12 focused checks but did not change the tiny reproducer's dependency
closure, so that candidate was discarded. A retained stack and class-name trace
then identified `node_to_type` -> `ensure_class_imported` during `rxcp_bvl` of a
synthetic `_address_new_request` declaration. `_rxsysb.addressrequest` belongs
to the same binary module, but its class stub is registered only after all
function metadata has been read. The unresolved lookup searches a same-namespace
application source even though the binary already declares the class.

Selected repair: collect the current module's META_CLASS/META_INTERFACE
names before reading declarations. While that module is being read, recognize
its forward declarations before external lookup, analogous to the existing
source forward-declaration guard. The stack is scoped to the synchronous module
read, including nested imports, and is discarded afterwards. All metadata,
eager inline attachment and consumer validation are retained. The tiny
`_rxsysb` extension changes from a full-content dependency to a namespace-only
candidate; consumer assembly is byte-identical. Twelve focused source/binary,
class/interface and reference-inline checks pass. The accepted Release verdict
is recorded below.

The downstream source ADDRESS environment entered in
5ba2bd0135aea984e7b921f93ccd57546761e9d6 on 2026-08-24, with follow-up
0591799f21fa1058059fa6e2b82f7a2ab262b2f3 on August 25. The failure requires the
metadata declaration-order gap and a same-namespace source extension; it is
not evidence that every private implementation edit has only one dependant.

## Forward-declaration candidate verdict

The newly isolated metadata-order candidate reduces the clean optimized wave
from the preceding candidate's 102.56 to 70.98 seconds (original baseline:
509.45 seconds). The private ragimprove edit selects 14 members instead of 48,
48.70 seconds versus 93.92 seconds. Unchanged: zero compiler work, 0.27 seconds.
Peak per-process RSS is essentially unchanged. This second implementation is
accepted by Adrian on 2026-09-06; completing the regression matrix, broad QA
and final human/AI documentation sync. See the
[retained verdict](evidence/2026-09-06-rxc-project-scaling-forward-verdict/README.md).

## Qualification finding: stale RXBIN unknown-feature fixtures

The broad Debug run exposed four pre-existing failures: nr21_fixed_call_contract,
nr14_frozen_parse_contract, nr15_native_stem_contract and gate_f_channel_contract.
They still treated bit 0x40 as unsupported, but 72d376e295 (2026-09-01) assigned
that bit to autoload hints. The loader correctly accepted it. The production
feature mask and those tests were unchanged from the e0e67 baseline. The first
Release sweep excluded them through their performance label; they were not
Release passes. The tests now inject unassigned bit 0x80000000, retain the
precise rejection check, and label their non-timed format/codegen assertions as
correctness so the maintained sanitizer and broad Release lanes include them.
No loader validation is relaxed and this is not an ASan memory finding.

## Local qualification outcome

The final full-application matrix passes: unchanged repeat 0 compiles; private
body and exported-contract edits 14 of 48; compiler options/tool identity 48;
repeats under those new identities 0. The separate uncontended ADDRESS library
wave is 156.18 -> 11.53 seconds with essentially unchanged RSS. The final
compiler and wrapper hashes, frozen source hashes, exact commands and all
per-member timing/selection evidence are in the
[qualification record](evidence/2026-09-06-rxc-project-scaling-qualification/README.md).

Release has 2,189 broad passes plus four corrected format checks and the final
wrapper replay. Debug accounts for all 2,457 tests after preparing the omitted
measurement fixtures and replaying the affected tests. Apple-ASan accounts for
all 2,271 applicable checks as 2,270 broad passes plus the project-contract
replay. SAN-QA-013 records that expanded matrix's old 180-second aggregate
timeout; every assertion is retained under a 600-second bound, and the same
contract passes normal Debug (77.01 s) and Apple-ASan (193.84 s). No sanitizer
memory diagnostic was found. This does not close unrelated SAN items or claim
Linux ASan/LSan qualification.

A fresh scratch install builds the frozen downstream application and separate
ADDRESS library. Linked/native init/status, optimized and unoptimized offline
configuration checks on both VMs, and ADDRESS OPEN/STATUS/CLOSE pass. The donor's
worker/ingestion/maintenance test sections are excluded by an explicit scratch
harness; the donor itself and user libraries remain untouched. Human compiler
and wrapper guides plus AI architecture/debugging references are synchronized.

The wider performance programme remains distinct: repeated unused-import
warning walks are observed but unmodified; this is not a portfolio or release
claim. Linux/Windows and hosted publication gates are not run here. The change
is intended for the requested local develop commit only; no push, global
installation, corpus access, provider call or automation change is performed.

# RXC-PROJECT-01 first Release verdict

Subsequent disposition: Adrian accepted this verdict on 2026-09-06. See the
[qualification record](../2026-09-06-rxc-project-scaling-qualification/README.md)
for the final implementation and follow-through; the text below retains the
state at the original decision gate.

2026-09-06; provisional, awaiting Adrian's acceptance. This is a single matched
macOS ARM64 application comparison, not a formal 12-pair statistical campaign,
portfolio result, Linux qualification or full task closure.

The ordinary Release compiler is built with Apple Clang 21.0.0, `-O3 -DNDEBUG`,
Ninja, and `CREXX_VM_PROFILING=OFF`, on AC with low-power mode off. Both packages
use e0e67ad3e89e19f98ab09676ffa2bbc99fad7010's native wrapper and the same frozen
libraries/providers. They differ only in the compiler binary. The candidate
changes one private declaration lookup in `compiler/rxcpfunc.c`; it does not
change eligibility rules, inline thresholds, optimisation options or metadata.

## Result

Whole supported `crexx --program` build, 48 unchanged declared sources,
`--jobs auto --noexec --nocolor --verbose1`, identical source/output paths and
corresponding scratch install roots:

* Elapsed: **509.45 s baseline -> 107.23 s candidate** (78.95% reduction, 4.75x).
* User CPU, including children: 1078.60 s -> 573.07 s; system: 7.54 s -> 6.89 s.
* Native time maximum RSS: 559.75 MiB -> 556.38 MiB. This is the maximum
  per-process rusage value, not the sum of concurrent compiler footprints.
* All 48 member RXAS files are byte-identical; the linked program is also
  byte-identical, SHA-256
  `6fd6e73cc63d7c66aaaa47a697bfaa6101ea8319f999ae897b56b72d6bb80551`.
* Candidate unchanged repeat: `SKIP: project current`, zero compiler children.

| Member | Baseline seconds | Candidate seconds | Baseline RSS MiB | Candidate RSS MiB |
| --- | ---: | ---: | ---: | ---: |
| crexxrag_cli | 172.91 | 25.90 | 396.64 | 397.62 |
| ragproduct | 173.38 | 22.01 | 559.75 | 556.38 |
| ragmcp | 133.95 | 13.07 | 392.39 | 389.58 |

All member timing/RSS rows and exit codes are in the adjacent CSV and JSON.
Native `/usr/bin/time -lp` measured each member through an identical shell shim;
its NUL-delimited argv captures and process samples are retained in scratch.
The shim is diagnostic infrastructure, not a product wrapper replacement. Its
hash is constant between the compared packages; toolchain-key invalidation is
not qualified through this shim. The original real rxc is `rxc.actual` in each
measured package, beside the unchanged runtime and provider artifacts.

The candidate ran first, then the baseline, without concurrent builds or tests.
No extra full baseline rerun is justified without changed measurement inputs.
An earlier incomplete-package run omitted rxvector and failed two members; it
is excluded from this comparison. Explicitly preparing `stage-c1-toolchain`
and `stage-b1-substrate` plus `vector_static` supplied the missing baseline
provider/VM/tool artifacts. The existing library, classlib, rxfnsg and rxfnsl
images remained byte-identical. This install-stage issue is retained as a
separate limitation, not hidden by the compiler result.

## Mechanism and correctness

The September 5 sample has 2095/2200 top-of-stack samples in
`ast_declares_local_contract`, below repeated inline eligibility analysis.
That lookup recursively walked executable bodies to answer a declaration
question. Inlining enlarged those bodies without adding legal nested class or
interface declarations. Restricting descent to file nodes makes query cost
depend on live declaration lists rather than expanded executable statements.

`rxcp_prepare_source_ast` runs `ast_source_structure_walker` before structure
symbol lookup; that walker requires legal classes/interfaces directly beneath
PROGRAM_FILE. The change also retains declaration inspection under IMPORTED_FILE
as before. The existing label comparison and current namespace checks remain.
No result is cached, so late declarations and changed import/AST state are
observed afresh. Historical introduction points and rejected broader designs
are in `performance/RXC-PROJECT-SCALING-WORKLIST.md`; they are not a claimed
exact timing bisect of the complete modern RAG source against old compilers.

12 focused Release tests pass: source/binary cross-file inlining, paired
reference lifetime tests, source interfaces, named factories, split namespace
providers, source contract stubs and covariant cycles. The first attempt lacked
rxvm/rxdas preparation; it failed on absent executables. After building those
targets the exact focused suite passed. Logs are retained at
`/tmp/crexx-scaling-focused-prep.WLFpT3` and copied into the scratch evidence.

## Provenance and remaining work

Baseline rxc SHA-256:
`896690d2529e5de9232a4b55a71d2ac38e67cac54cfed09855dddea3ed4644c4`.
Candidate rxc SHA-256:
`aac5dcda2d6f8898b3ff496a37ef441a13be672b3d5e5225694a8718d33616b8`.
Common Release wrapper SHA-256:
`b9b1ec023748c7ca0aab7026948c1d997cd87799bdd9d68091aee701911356bf`.
Both rxc version strings show the clean base version: incremental binary
rebuild does not refresh that generated header. The candidate's dirty source
scope and binary digest, not its version string alone, identify this change.

The original installed Debug binaries/package mismatch, exact donor manifest,
source diff, and package preparation are recorded in the adjacent provenance
files. The frozen donor and live donor still match all 131 source hashes; the
occupied develop checkout's unrelated patch remains byte-identical. Develop is
fast-forwarded to e0e67ad3e; the fix is isolated on temp/rxc-project-scaling.

This verdict does **not** close the project-build request. Still required after
acceptance: dependency-aware member selection and edit/options/shadowing tests;
a small permanent scaling regression; disposition of repeated unused-import
warning analysis; separate ADDRESS-library timing; scratch downstream offline
checks; required broader Debug/Release and ownership-relevant sanitizer QA;
native/installed external-consumer proof; platform-labelled limitations and the
requested local develop commit. No commit, push, global install, corpus access,
provider call or RAG automation change has occurred.

Raw evidence root:
`/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/evidence/`.
Measured commands: `run-release-candidate.sh` and `run-release-baseline.sh` at
the scratch root. Original snapshots and bounded diagnostic failures are
retained, including the ten-minute copied-Debug wave and 240-CPU-second
instrumented runs. No inference of an infinite loop is made.

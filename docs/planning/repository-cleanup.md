# Repository structure and cleanup

**Status: approved cleanup implemented and locally verified on 17 October 2026;
S370 retained pending its external-port disposition. Delivery: develop with this change.**

This is the single plan for the repository-wide review requested on 17 October
2026. The preceding Level L reorganisation is published on `develop` as
`70e8c300f94c2c8e9698d5789491a8e8f80f3746`; local and remote tips were verified.
Its implementation and qualification remain owned by
[the Level L plan](../../research/level-l/PLAN.md). CI monitoring remains stopped.

## Vision and intended outcome

Make the checkout understandable from its directories: each product, reusable
library, development tool, example, experiment and document has one canonical
owner. Builds and installs must use those owners, rather than accidentally
collecting whatever happens to be in an output directory. Keep maintained source,
tests and meaningful documentation; remove completed investigation debris and
stop its producers recreating it. The result should be easier to navigate in
CLion and safe to build incrementally from the existing main `develop` checkout.

Preserve commands, APIs, language behaviour, target identities, installed paths,
platform support, required test coverage, licence notices and open programme
criteria. Keep the newly established Level L SDK/tool/research separation.
Compiler completion, native compiler redesign, a new build system, release
qualification and history rewriting are outside this cleanup. No blanket
`git clean`, deletion of build trees or removal of another task's work is proposed.

The root [AGENTS.md](../../AGENTS.md) already supplies the enduring no-junk rule.
This work removes conflicting instructions and fixes producers; it does not add
another standing-policy document or an archive of the material being deleted.

## Review basis and findings

Read-only inspection covered the complete tracked-file inventory, top-level
CMake composition and installation, component build declarations, automation,
documentation authorities, output-producing tools, and references to candidate
obsolete paths. This is a structural review, not a line-by-line correctness audit
of every component or fresh hosted/platform qualification.

At the published revision there are **11,475 tracked files, 152.76 MiB** of blob
content. `re2c` alone accounts for 3,904 files / 46.76 MiB and includes legitimate
upstream tests and bootstrap inputs. File count and extension are not deletion
criteria. All 11 tracked symlinks currently resolve.

| ID | Observed finding | Proposed disposition |
|---|---|---|
| RC-F01 | `docs/qa` has 737 files / 22.35 MiB; `performance/evidence` has 109 / 3.12 MiB. Together they contain 242 `.log` files / 9.74 MiB, plus run JSON, CSV, traces, generated code and copied workloads. | Extract current facts into their existing owners, migrate actual input files, then remove the dated output trees. Do not replace them with another archive. |
| RC-F02 | `tests/native-inference/local_model_compatibility.py` compiles `tests/native-inference/fixtures/gemma4-common.crexx`. Two mainframe packers include review documents from `docs/qa`. | Move the live model fixture to `tests/native-inference/fixtures`; consolidate maintained mainframe guidance under `ports/single-threaded`, and update the exact packer inputs before removing the old notes. |
| RC-F03 | `performance/tools/run_lifecycle.crexx` defaults to `performance/evidence/lifecycle`. Concurrency runners create permanent evidence bundles; `concurrency/qa/EVIDENCE.md` explicitly says to commit evidence/digests. `concurrency/README.md` also instructs keeping raw logs. | Use transient workspaces with deliberate active-failure inspection and cleanup; preserve useful summaries in current plans. Rewrite these conflicting passages in place and retire the obsolete evidence contract. |
| RC-F04 | `docs/DOCS_MAP.md` permits retired working notes and claims HEAD is already free of obsolete runs. Compiler, concurrency, performance and native-inference plans mix current decisions with repeated dated execution histories. | Correct the map; consolidate current ownership, decisions, acceptance and qualification limits. Delete superseded reports/handoffs and completed worklists after preserving their lasting content. |
| RC-F05 | `driver/CMakeLists.txt` declares `project(driver)` but also owns `rxbuild`, whose source and grammar live in `tools/build`. `tools/build-system` combines the production G builder with build-graph observation tools. | Rename source `bin/` to `driver/`; make `tools/rxbuild/` own the rxbuild source/grammar/build/tests/docs; separate builder and audit subdirectories within `tools/build-system`. Keep executable names and build/install `bin/` unchanged. |
| RC-F06 | User demonstrations are split between `examples`, `demos`, `tests/demo`, component examples and Level L examples. Three local-platform workflows ship `tests/demo/*`, including deliberately invalid examples. | Move user demos into categorized `examples`; keep negative/test-only inputs with tests. Preserve Level L examples in `langtools/examples` and compiler examples in research. Replace wildcard demo shipment with deliberate selections. |
| RC-F07 | Root `re2c`, `lemon`, `avl_tree`, and `utf8` obscure dependency ownership. `utf8` mixes an upstream header with the project's scanner definition. | Put vendored sources under `third_party`; move the first-party scanner definition to `platform`. Preserve upstream trees, licences, local patches, generator dependencies and offline build inputs. No vendor pruning or upgrading. |
| RC-F08 | `experiments/Pipe`, `experiments/PipeStreaming` and `contrib/crexx-pipes` split one unshipped proposal and its prototypes. `interpreter/rxvml/rxvm.h` is an actively included VM lifecycle header in an otherwise singleton root. | Consolidate the pipe research under `research/pipes`; give the other surviving experiments named research owners. Move the lifecycle header to `interpreter/rxvml`, updating its consumers without changing API or install exposure. |
| RC-F09 | Root installation recursively copies `${CMAKE_BINARY_DIR}/bin`, with growing exclusions for stale test/research products. | Replace catch-all ownership with component-owned explicit installs, including generated runtime packages and native support archives. Keep existing destination conventions and optional selections. Prove stale artifacts cannot enter a package. |
| RC-F10 | `tools/docker_scripts/build_image/Dockerfile` checks out `feature/DEV2507`; its Jenkinsfile fetches/builds NetRexx. `bin/boot.sh` references absent `lib/rxfns`; its `ccomp` companion uses the obsolete archive layout. | Remove these obsolete recipes after updating any human references; the maintained CMake/native-driver and workflow routes remain canonical. `crxc.rexx` is actively tested and is retained. |
| RC-F11 | `Utilities` describes itself as unstructured notes. Its plugin generator has no repository caller. `lib/rxmath` is unbuilt by `lib/CMakeLists.txt`, contains two empty placeholders, and is distinct from the live rxfloat-provided `rxmath` compatibility namespace. `lib/veclib_test` contains only a newline. | Review unique useful content against current RXPA/math/examples before deletion. Remove the placeholders and blank `veclib_test`; delete superseded notes/code, or promote a still-required example/tool with a named consumer. Do not delete the live compatibility namespace or built `veclib` library. |
| RC-F12 | `S370` has a separate old CMS/Docker recipe; normal constrained builds use `ports/single-threaded` and `platform`. No current build entry references `S370`; this does not establish absence of external users. | Keep this as a visible hold pending a port-entry audit. Move maintained CMS-specific inputs into `ports/cms`; remove only demonstrated superseded recipes. Do not drop CMS support or qualify it from desktop tests. |

There are 183 source-like files in the two output trees. Only one was an exact
byte copy of a maintained input in the comparison. Therefore the deletion pass
must assess purpose and consumers, not assume these are all duplicate source.
Unique generated/probe files are not automatically permanent fixtures either.

## Proposed ownership after cleanup

| Owner | Responsibility |
|---|---|
| `compiler`, `assembler`, `linker`, `disassembler`, `interpreter`, `preprocessor`, `debugger`, `contract`, `cpacker` | Established toolchain components, their implementation and component tests. Keep these useful roots; do not introduce a generic `src/` layer. |
| `binutils`, `platform`, `rxpa`, `messages` | Shared binary contracts/operations, platform services, native-provider API, diagnostic catalogues. `contract` remains the command/model layer and `binutils` the independent reader; similar names are not proof of duplication. |
| `driver` | `crexx`, its build worker, supported `crxc` wrapper and driver tests. Source directory only; all user-facing command names remain unchanged. |
| `langtools`, `lib/rxfnsl`, `research/level-l/compilers` | Published Level L tooling/examples/guides, generic SDK implementation, and experimental source compilers respectively. Preserve the approved C versus typed B/G boundaries. |
| `lib/<package>` | Runtime libraries/providers and their owned tests/API documentation. Do not impose the special Level L example arrangement on every unrelated runtime library. |
| `tools` | Named maintained tools: `rxbuild`, build-system builder/audits, RexxDoc, TextTools, editor support and developer QA helpers. A built tool owns its CMake registration rather than borrowing the driver. |
| `scripts`, `packaging`, `.github` | Repository/CI/release orchestration, installer templates/assets, workflow composition respectively. Keep scripts' tests under `scripts/tests`; no mass script rename merely for uniform spelling. |
| `examples` | Runnable user examples grouped by purpose; one source per example, with tests consuming it. Component-specific teaching stays with its owner where already deliberate. |
| `tests` and component `tests` directories | Cross-component regressions/benchmarks and component-specific tests respectively. Authored RXAS, empty expected outputs and malformed fixtures remain legitimate. |
| `research` | Active experimental implementations and their current plans/tests, including pipe prototypes. No holding pen for obsolete files. |
| `ports` | Maintained constrained-host build/package adapters; shared platform behaviour stays in `platform`. |
| `third_party` | Root-level vendored re2c, Lemon, AVL and UTF-8 dependencies. Existing package-local vendors such as SQLite and nlohmann stay with their package. |
| `docs`, `performance`, `concurrency` | Current documentation, performance programme, and cross-component concurrency programme. Retain these real owners, while removing duplicate chronology and discarded runs. |

Specific tool/example moves proposed:

- `bin/` → `driver/`, retaining tested `crxc.rexx`, removing only obsolete boot/ccomp.
- `tools/build/` → `tools/rxbuild/`, with the rxbuild CMake rules moved out of the driver.
- `tools/build-system/build_stage_g*` → `tools/build-system/levelg/`; graph-audit
  scripts/schemas → `tools/build-system/audit/`. Preserve the Level B graph owner.
- `docs/texttools/` and `docs/generate-books.crexx` → `tools/texttools/`;
  `docs/BUILDING-DOCS.md` remains the build guide, books remain under `docs/books`.
  Update authorship/provenance without assigning a new licence to upstream work.
- `demos/{cms,llm,native,rexxscript}` → matching `examples/` categories;
  positive `tests/demo` tutorials → `examples/basic`, while deliberately invalid
  inputs remain regression fixtures. Preserve build target compatibility.
- `re2c/`, `lemon/`, `avl_tree/`, upstream `utf8/utf.h` → their named
  `third_party/` directories; `utf8/encoding.re` → `platform/encoding.re`.
- `inc/rxvm.h` → `interpreter/rxvml/rxvm.h`; no new public embedding promise.
- Pipe proposal/prototypes → `research/pipes`. Decide the surviving rxqueue and
  buildinto experiments by actual purpose before removing the `experiments` root.

## Deletion, preservation and unresolved decisions

**Delete after consolidation and consumer migration:** tracked run output under
`docs/qa`, `performance/evidence`, and `concurrency/evidence`; obsolete evidence
instructions, completed handoffs/review receipts and copied execution narration;
the old Docker/Jenkins setup, boot/ccomp scripts, empty rxmath placeholders and
blank `lib/veclib_test`.
Do not retain an external archive or a replacement per-file cleanup manifest.
Use Git history for already-published history; do not create preservation commits.

**Keep explicitly:** compiler assembly/runtime goldens; malformed-format fixtures;
licensed data and required vendor tests/bootstrap sources; random-weight GGUF
test fixtures and their reproducibility manifests; source licences and RexxDoc
tags; book sources, figures, guide symlinks, SQLite instruction inputs and
checked-in generated assets whose replacement generation is not qualified.
Book assets are governed by [BUILDING-DOCS.md](../BUILDING-DOCS.md), not suffix.

Two Unicode 17.0.0 inputs are byte-identical across G and L:
`DerivedCoreProperties.txt` (1,134,783 bytes) and `UnicodeData.txt` (2,198,209 bytes).
This duplication is documented: L keeps its bootstrap independent of G.
**Keep it in this cleanup.** A neutral shared raw-data owner could later remove
the duplication, but must preserve standalone bootstrap and separate generator
hash/provenance contracts. It is not disposable run output.

**Holds requiring a concrete disposition during implementation:** the unique
Utilities generator/notes, unbuilt mathematical prototypes, remaining small
experiments and S370's external entry points. A missing CMake caller alone does
not authorise loss of capability. Summarise useful contracts in the actual owner;
promote only genuinely maintained source, otherwise delete after confirming its
replacement. Unresolved holds remain visible and prevent a claim of total cleanup.

**Current local work:** the float-constant-folding worklist has uncommitted
numeric-review changes; its untracked review output, the Lotto planning draft,
mainframe `.rxas`/driver locks and Python caches were excluded from publication.
Do not disturb these while another task owns them. This does not exempt the
*tracked completed* float QA output from the proposed cleanup.

The filesystem also contains build trees, `output/`, `:out/`, and signed installers
under `dist/`. They are not in the tracked inventory. Inspect activity/ownership
before removing any finished diagnostic output. Preserve build state, in-use
outputs and deliverable installers; do not treat all ignored files as waste or
silently extend this review into deleting other checkouts or external storage.

## Acceptance criteria

The local implementation and qualification results are recorded below. The
external CMS hold remains explicit; no new hosted or guest qualification is claimed.

1. **RC-AC01 — single ownership:** the approved directory map matches CMake,
   imports, documentation and package composition. Every moved item has one
   canonical source; reference/link checks find no abandoned old owner.
2. **RC-AC02 — no disposable tracked output:** remove the identified raw-output
   trees after purpose/consumer checks. Preserve genuine fixtures with their
   actual reader and concise rationale. No archive, renamed dump or new raw
   receipt replaces them. Resolve or visibly hold every ambiguous item.
3. **RC-AC03 — durable documentation and rules:** one current scope/decision/
   contract account per area; remove superseded notes and conflicting retention
   guidance. Preserve open acceptance IDs, issue/SAN identities, original failure
   mechanisms, closure regressions and qualification limits. Do not mark findings
   closed or invent new qualification from this review.
4. **RC-AC04 — output lifecycle:** changed research/QA/performance helpers use
   owned temporary/build workspaces, expose failures for diagnosis and support
   safe cleanup after summary. Tests cover success, failure and interruption
   without deleting someone else's data. Hosted CI logs remain hosted evidence.
5. **RC-AC05 — build and test continuity:** product and affected optional targets
   build from the new paths. Focused tool/driver/examples/header/generator tests
   pass; test registrations, producer dependencies and no-op/missing-output
   behaviour remain correct. Reuse unchanged tests and input identities.
6. **RC-AC06 — explicit installation:** ordinary fresh and incremental installs
   have the intended commands, libraries, static archives, plugins, schemas and
   examples; optional products remain selected explicitly. Deliberately stale
   test/research artifacts cannot leak into an install. Installed/native smoke
   checks preserve the documented `bin/` layout and paths with spaces.
7. **RC-AC07 — vendor/port integrity:** vendor moves preserve contents and notices,
   scanner generation works offline, and port adapters still resolve their
   sources. Record desktop/static proof separately from actual target runs;
   do not remove a supported external port entry without a resolved disposition.
8. **RC-AC08 — hygiene and publication:** a small first-party tracked-file check
   prevents recurrence of known run-output locations without blocking real test
   assets or upstream trees. No blanket extension ignores. Preserve unrelated
   work and the stopped monitor; publish only after implementation approval and
   the agreed local checks, with concise results in this plan.

## Implementation sequence and gates

| Step | Work | Criteria and dependencies |
|---|---|---|
| **RC-STEP01 — approved** | Adrian approved this map, deletion policy, proposed moves and held-item process on 17 October 2026. | AC01–08; preserve active work and genuine fixtures; delete obsolete material without archiving. |
| **RC-STEP02 — complete** | Move the known live QA inputs, consolidate enduring decisions/current qualification, delete completed output and obsolete instructions/recipes. Reconcile active-task ownership first. | AC02/03; after STEP01. Keep every unresolved requirement visible. |
| **RC-STEP03 — complete** | Correct output-producing helpers and add narrow hygiene/lifecycle checks. | AC02/04/08; after the consumer inventory in STEP02. Keep diagnostics available during an active failure. |
| **RC-STEP04 — complete locally** | Reorganise driver, named tools, examples, research and the VM header; update all CMake, workflow, test and guide references. | AC01/05; after STEP01, in coherent bounded changes. No language/runtime changes. |
| **RC-STEP05 — implemented except external CMS hold** | Move root vendors, resolve the old CMS/Utilities/math/experiment holds, and retain required data/fixtures. | AC01/02/07; after their concrete consumer/replacement checks. Stop on an unresolved capability decision. |
| **RC-STEP06 — complete locally** | Replace broad bin harvesting with explicit component installs and preserve fresh/incremental/selected product inventory. | AC01/05/06; after source-owner moves. Includes installed native archive and plugin dependencies. |
| **RC-STEP07 — complete; publication accompanies this change** | Run the smallest decisive affected checks, update the current guides/map and this result summary, inspect the diff and publish the approved cleanup. | AC01–08. Do not dispatch deep/overnight QA or require a formal performance verdict for a path/housekeeping change. Normal automatic workflows remain separate. |

Keep source-only moves separate from obsolete-output deletion and install-graph
changes where that aids review. No history rewrite and no post-CI status commit.
Once implemented, fold the lasting map into the repository documentation and
retire this completed plan when it no longer owns open work.

## Implementation decisions

- The model compiler input is now `tests/native-inference/fixtures/gemma4-common.crexx`.
  Both mainframe packers select `ports/single-threaded/README.md`, which now owns
  the provider closure and PDOS command-vocabulary requirements.
- Deleted the tracked QA/performance/concurrency output trees, retired tracer
  and highlighting notes, completed VM-startup/structured-exit worklists and old
  worker handoff. Current scope, stable acceptance/SAN identifiers, failure
  mechanisms and regression boundaries stay in their existing authorities.
- Utilities' untested generator and old unsafe parameter-mutation advice are
  superseded by `rxpa/CMakePluginTemplate.txt`, the current provider guide and
  executable RXPA examples. The unbuilt pure-Rexx math prototypes did not define
  a shipped namespace; the live float provider's `rxmath` remains. The scratch
  queue example is covered by `ts_rxqueue_{noopt,opt}`; the personal-path buildinto
  experiment is superseded by `crexx_rxpp_builddir` and the maintained guide.
- **CMS hold:** keep `S370` and its external VM/370 recipes. No maintained
  consumer/guest replacement has been demonstrated; removal would risk an
  external workflow. Source references are rebased, with no guest qualification
  claim. This is the sole unresolved legacy-root disposition, not an archive.
- Explicit target/bytecode installs preserve native archives in `bin/`. Provider
  metadata selects and hash-checks only its declared dependency closure.
  The product stage builds all selected installed products, including RXPP helpers and
  the retained rxmath aliases; test harnesses and
  stale files are never inferred to be products. Research installs remain opt-in.
- Concurrency runners use uniquely owned temporary captures, delete successful
  runs and expose failed/interrupted runs for diagnosis and explicit cleanup.
  TextTools also deletes successful owned test workspaces.
  The Level B lifecycle runner defaults to its build tree. No release timing or
  overnight matrix is required for these ownership/path changes.

### Bounded rxbuild repair (RC-AC05)

The installed smoke reproduced unbounded `/..` growth when rxbuild creates a
previously absent output directory: `rxfs.abspath` cannot normalize a parent
through that nonexistent child. Replace its recursive helper with the existing
Level B `ADDRESS CREXX mkdir -p` command; preserve error propagation and verify
nested missing directories, a file blocking creation, and use from a repository
subdirectory. Source-root detection must recognize `driver/` as well as installed
`bin/`. No grammar, compiler or filesystem-provider contract changes are needed.

## Local qualification and remaining boundary

This change relocates 4,014 tracked inputs and deletes 891 obsolete tracked
files, without an archive. All 3,913 relocated vendor blobs match their original
Git identities; all 11 tracked symlinks resolve. The 112 protected active files
match their initial SHA-256 values. The unrelated numeric-review worklist/draft,
untracked review files, mainframe outputs/locks, build trees and installers stay
with their owners. One unreferenced generated demo RXAS file dated May 2026 was
also removed from the retired demo directory after inspection.

| Criterion | Disposition and decisive checks |
|---|---|
| RC-AC01 | Verified locally: component CMake, workflow staging, imports and documentation use the canonical owners. Changed Markdown relative links resolve; old-source reference review passed. |
| RC-AC02 | Verified for the approved deletion set: consumer fixture moved, mainframe guidance consolidated, obsolete tracked captures/notes removed. S370 is explicitly retained, not classified as disposable. |
| RC-AC03 | Current architecture, ownership map and user/tool guides updated. Open SAN/acceptance identities and platform boundaries preserved. Historical qualification citations resolve through existing Git history; no new archive is made. Conflicting raw-log retention instructions were corrected in place. |
| RC-AC04 | Five focused ownership/hygiene unit checks pass, including a terminated producer, failing producers, sibling preservation and symlink/renamed-marker rejection. Lifecycle tool compiles/assembles and its self-test passes. Linux runner syntax and Windows runner source reviewed; native qualification campaigns were not launched. |
| RC-AC05 | Debug stage-product succeeds with the existing selected llama/research configuration. Twenty-three distinct focused CTests pass; the updated cleanup test contains five checks. Eighteen build-graph unit tests and the 45-action TextTools harness pass. The final no-op build reports no work; deleting the generated rxbuild linked image recreates identical bytecode. |
| RC-AC06 | Fresh and repeated installs into a prefix containing spaces produce the same 387-file inventory. Deliberately stale bytecode/plugin sentinels are excluded; provider dependency hashes, RXPP inputs and rxmath aliases pass. Installed native B and Classic C compilation/relocation, rxbuild grammar/reader execution, invalid substitution and the lifecycle self-test pass. Local-platform workflows now use the same explicit installation route. |
| RC-AC07 | Vendor blob identity and offline product/scanner generation verified. Port source references and shell syntax checked; the external CMS hold is unchanged. No native guest, Linux, Windows, GTK or ODBC execution is claimed by this macOS check. |
| RC-AC08 | Tracked-file hygiene passes, preserving vendor inputs and authored test fixtures. Active work is unchanged and CI monitoring remains stopped. Task-created temporary captures are deleted after this summary. Publication uses only the approved cleanup paths. |

The focused CTest selection covers driver help/headerless B/C/explicit-header,
project builds and RXPP build directories, output policy/locking, compile
progress and worker diagnostics; VM file/worker and CMS text controls; generated
Level L consumers on both VMs, tool smoke and product/layout checks; repository
hygiene, cleanup ownership, installation and rxbuild missing/blocked-directory
errors. Earlier valid passes were reused for unchanged inputs. TextTools uses a
mock typesetter and does not qualify PDF output. No compiler/runtime semantics,
formal performance programme or broad sanitizer campaign was added.

The rxbuild failure controls initially rejected legitimate parent components in
an already bounded repository-root spelling; the test now compares resolved
source paths and bounds output growth. The install alias check accounts for
CMake's platform ranlib step while verifying the compatibility archive against
its original build input. Neither test correction masks a product failure.

The normal publication workflows are expected to pass, but this local result
is not hosted success or release qualification. No CI-only follow-up commit or
monitor restart is needed. The only remaining structural decision is S370's
external ownership/replacement; remove this plan once that hold has a durable
disposition in the port documentation.

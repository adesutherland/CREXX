# Level L development integration - 12 October 2026

This receipt qualifies PUB-AC01-04 in `research/level-l/PLAN.md`. Adrian
authorized publication to `develop` after the local gates passed. The enclosing
commit is the publication payload; verify its remote identity when pushing.
Normal automatic Release/product smoke, optimizer-parity and CodeQL workflows
are expected to pass. Their actual results remain in GitHub; this receipt does
not claim hosted results or close the wider research/LANG programme.

## Inputs and changes

- Manual/example commit: `fb4972050`.
- Integration base: `2121969ce`, merging `a415090dc` from current `develop`.
- `qualification-inputs.json` identifies the five changed build/package files
  and twelve tested tool/library artifacts; `integration.patch` retains the
  exact production change over that base. README/plan/receipts are documentation.
- Fresh build directory: `cmake-build-levell-integration`, native macOS ARM64
  Debug, AppleClang/Ninja, inference and GTK disabled. This is development
  qualification, not a new release or cross-platform sanitizer claim.

`stage-l-libraries` now builds the Level L command as well as its library;
`stage-product` also builds the separately selected modern RexxScript command.
The generated-language and modern interpreter tests register their producer/VM
dependencies and join ordinary smoke QA. The existing binary-surface test also
registers its inputs. No nested-build CTest workload was added or expanded.

Permanent package smoke checks now use installed tools to generate a grammar,
compile/assemble/link a consumer and run it on each available concrete VM. They
check Unicode values, malformed grammar refusal, modern RexxScript expression
and BIF results, and no execution after a syntax error. The existing native and
Classic stream package checks continue to execute.

## Results

| Gate | Result and evidence |
|---|---|
| Fresh product | Pass; `configure.log`, `product-build.log`. Both commands and libraries built through `stage-product`, with no compiler warnings. |
| Dependency closure | Pass; `build-graph.json`. Product commands and QA images/VMs are reachable. The Level L stage has no Level G dependency. |
| Bootstrap | Pass; `bootstrap-images.json`. The owning build gate and an independent comparison confirm identical L1/L2 generated RXAS. |
| Normal QA preparation | Pass; `qa-prep.log`. Standard smoke preparation plus the 58 declared producer targets for affected tests, starting with no test images. |
| Normal correctness | **249/249 pass**, 248.37 seconds; `correctness.log`, `normal-qa.json`, `selected-tests.txt`. Includes essential/smoke and affected Level L, legacy/modern RexxScript, compiler exits, RXPP, numeric and Unicode tests. |
| Teaching/backend panel | 486 observations, 702 commands; pass. Includes calculator, backend selection, PEG priority and CFG ambiguity controls, source/RXAS and optimized/unoptimized generated consumers on both VMs. |
| Diagnostics | 78 target cases across six backends; pass. |
| Recovery | 160 cases and two specification-resynchronization controls; pass. |
| Comments | 1,124 observation records / 1,285 commands; pass. The panel includes 1,150 runtime cases and 14 expected build refusals, using the current optimized library. |
| Standalone comment lexer | 48 cases; pass. |
| Unicode generator compatibility | 17 inputs across eight reference/native/optimizer/VM cells; pass, including the already documented NUL correction. |
| Packaging unit suite | 13 tests; pass, `package-unit.log`. |
| Installed product | Pass; `install-commands.json`, `install.log`, `installed-smoke/summary.json`: 24 commands including the two expected exit-1 refusals. The installed consumer runs on rxbvm/rxtvm; native and Classic programs run after relocation. |
| Manual/PDF | 80 guide links valid. The requested 46-page PDF has contents links, bookmarks, embedded Unicode fonts and checked page renderings; `manual-pdf.json`. The PDF is a local output artifact, not a repository binary. |

`backend-panels.json` records the exact six focused invocations and their exits.
`panels/` retains raw command logs, panel receipts, authored grammars and inputs;
`panel-files.json` hashes these files. Compiled binaries and generated assembly
remain build outputs. Earlier complete-library opt/no-opt research panels remain
historical evidence; the current integration panels use the newly built optimized
library and the generated-consumer modes stated by each receipt.

## Disposition of preparation checks

The first `cmake --install` attempt stopped at an unbuilt optional example
(`arrayformatdemo.rxbin`). Building `stage-optional`, `sqlite`, `sqlite_static`,
`vector` and `vector_static`, as the publication workflow does, supplied the
installation payload. Installation and all installed checks then passed. The
initial log is retained as `install-before-example-prep.log`; this required no
product change and does not invalidate the earlier correctness run.

An initial ad hoc bootstrap inspection compared raw RXBIN files. The documented
reproducibility contract and owning CMake gate compare generated RXAS, which
matches. The corrected independent check uses that contract. No generator or
bootstrap repair was needed.

The PDF renderer initially lacked the Chinese glyphs in an example. Embedding
the Unicode font corrected rendering; all 46 final pages were inspected and
text-margin checks passed. The Markdown source was unchanged.

No product failure, first-party sanitizer finding or unresolved publication
blocker was observed. The checks above passed before publication. Full hosted
deep/stress/sanitizer assurance follows the repository's overnight policy.

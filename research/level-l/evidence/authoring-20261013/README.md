# Level L authoring and build refinement qualification

The approved authoring increment is implemented and locally qualified on
`develop`, based on `54b610b95367`. This receipt covers UX-AC01–05 in
[the authoritative plan](../../PLAN.md). Adrian subsequently authorized commit and push to `develop`. The enclosing
commit is the publication payload, including the incoming packaging fixes
through `1031ef530`. Normal automatic CI is expected to pass; observed workflow
results are reported in the conversation without a status-only follow-up push.

## Implemented behavior

- `.crexx` with `options levell`, `.rxl` with an optional header, and legacy
  `.levell` sources use the same frontend. Conflicting explicit levels fail.
  Header omission preserves original source locations through L0/L1/L2.
- `crexx` generates grammar providers before compiling consumers. Combined
  program builds, optional reusable libraries, implicit build/run and native
  packaging work with spaces in paths and either source order. Libraries keep
  independent authored modules; programs supply the shared parser runtime once.
- Grammar, generator, assembler and runtime identities govern rebuilding.
  Failed grammar generation preserves both the last good grammar image and
  published program. Explicit bytecode `-l` inputs participate in project links.
- Settings and RexxScript sequences use repetition, with ordered flat named
  children. The named-tree builder materializes each named parent's visible
  children without copying every transparent list suffix. Its exact-budget
  regression covers 3,000 items, ordering, spans and a transparent root.
- The manual teaches the combined build first and explains separate libraries,
  repetition, captures, tree shape and the calculator. All 80 guide links resolve.

## Results

| Gate | Result | Evidence |
|---|---|---|
| Product and bootstrap | Successful Debug product build and final driver rebuild; L1/L2 RXAS hashes equal; no compiler warnings | `product-build.log`, `driver-build.log`, `inputs.json` |
| Existing runtime regressions | 6/6 pass: generated languages and modern RexxScript on both VMs, plus TinyExpr opt/noopt | `runtime-tests.log` |
| Existing wrapper/import regressions | 24/24 finally pass; the project contract was corrected and replayed separately | `wrapper-tests.log`, `project-contract-final.log` |
| Installed authoring/build panel | 56 commands pass in 107.24 seconds | `authoring.json`, `installed-authoring/` |
| Old/new list equivalence | 384 observations pass across LL(1), LR(1), packrat and Earley, both VMs and opt/noopt consumers | `lists.json`, `lists.log` |
| Recovery and custom diagnostics | 160 target cases and two specification-resynchronization controls pass | `recovery.json`, `recovery.log` |
| Captures and trees | 80 cases, three expected lexical refusals and four worked-example cases pass | `captures.json`, `captures.log` |
| Installed package workflow | 22 commands pass, including expected invalid-input refusals, both concrete VMs, native relocation and Classic runtime | `installed-smoke/summary.json` and logs |
| Packaging unit tests | 13/13 pass | `package-unit.log` |

The authoring panel includes a combined library of two independent grammars,
two separately built grammar libraries in one program, native consumers of
both source and library routes, and the same-basename calculator. A private
launcher tests generator invalidation without modifying a signed executable.
Adding a valid unrelated module to a private runtime container tests content
invalidation without changing parser behavior. Missing-tool and last-good-output
failures are expected test outcomes.

These are native macOS ARM64 Debug checks. They qualify the current product and
bootstrap, with the generated-artifact/consumer modes recorded in each panel.
They do not claim new Linux, Windows or sanitizer results. The normal installed
package smoke now exercises these source forms in subsequent publication runs.
No aggregate nested-build CTest was added or enlarged.

## Failures resolved during implementation

1. Repetition exposed quadratic intermediate named-tree storage. The existing
   3,000-statement RexxScript case exhausted `tree.entries`. Direct visible-child
   collection repaired it; the unchanged interpreter case and new exact-budget
   regression pass without raising allowances.
2. Reusing the generator argv array accidentally retained its entries for the
   assembler. Separate argument arrays fixed generation/assembly sequencing.
3. Native packaging revealed duplicate standard-library providers. Reusable
   grammar libraries now retain their own modules, programs add the shared
   parser runtime once, and native packaging supplies its usual standard
   libraries. Already-linked explicit bytecode libraries are not added again.
4. Library roots were initially interspersed with linker inputs. `rxlink` reads
   options first. Moving every `-r` before the inputs fixed the two-grammar
   control and existing project contract. The initial 23/24 wrapper run is
   retained; the corrected project test passes in 84.27 seconds.
5. Two harness setup errors were corrected: the runtime-identity fixture needed
   its installed import root, and installed smoke needed the actual configured
   `rxbvm` product selection. The final installed panels pass.

`inputs.json` records the base, product hashes, relevant source hashes and
bootstrap identity. Rich panel receipts retain command arguments and outcomes;
compiled scratch outputs remain under `research/level-l/build/authoring-20261013`.
The authoring script's file I/O was subsequently made explicitly UTF-8 for
Windows portability; this preserves the bytes used by the qualified UTF-8 host.

## Publication integration

Before committing, develop was fast-forwarded to `1031ef530b6e21d0a2a2b9f764c50e40cced28c6`.
The incoming console/static-provider packaging fixes were preserved, including
their installed native-console checks. Our Level L smoke changes merged cleanly.
The combined `stage-product` build and isolated installation pass, and all
24 installed smoke commands pass. Packaging unit tests pass 13/13; the
tracked Windows-path guard passes 3/3. `publication.json` identifies the combined
inputs and tools, and `publication-*.log`/`publication-smoke/` retain results.

The Level L runtime and standard-library bytecode remain byte-identical to the
prior qualified images, so the unchanged backend/list/recovery evidence is reused.
Native commands were regenerated after the upstream/version changes; the
combined installed workflow exercises them. L1/L2 generated RXAS still agree.
The commit includes the proposed next AST stage for review, explicitly awaiting
syntax/API approval. Actual CI monitoring is a conversation follow-up, with no
status-only repository commit planned.

# Issue #699 assessment — 16 September 2026

**Confirmed compiler defect on current hotfix, with two independent reduced
reproducers. No product repair or publication in this assessment.**

This is the historical assessment. The subsequently authorized correction and
its live qualification status are recorded in [repair.md](repair.md).

Authoritative scope/status: [planning record](../../planning/issue-699-source-import-diagnosis.md).

## Baseline and controls

The fetched `hotfix`, `origin/hotfix` and `origin/develop` were all
`17e844441ed87e1f6e0d5f1f0d3bb4bee8db6187`. Compiler implementation and rxfnsb
sources are unchanged from the issue's compiler revision
`037e7939bc29eb91b29ed41e9b1b8debdef6353d` (only compiler test registration differs).
The exact installed current Release compiler and the retained previous installed
compiler were both tested. Paths, versions and binary hashes are in
`evidence/compiler-versions.json`.

Downstream inputs were reconstructed from RAG commit
`ab7943d42d459c7d8fc6e146f7709dfac7a2d827`, extracting its `crexx` tree to a
temporary directory and replacing only `call lineout stream` with
`call closefile stream` in `ragtrace.crexx`. This is a documented reconstruction,
not a claim to have recovered the original uncommitted tree byte for byte.
The source roots are the seven unique parent directories of that tree's sorted
`.crexx` paths; their exact order is retained in the provenance JSON. Each
recorded `result.json` gives the full direct `rxc` command and elapsed time.

| Current compiler, reconstructed original modules | ragcommand | ragprocess |
|---|---|---|
| Source imports, closefile, optimized | 255: convergence flag 0x0002 | 2: provider mismatch at 588:103 |
| Source imports, closefile, no optimization | Same failure | Same failure |
| Source imports, lineout control | Pass | Pass |
| Source imports, only ADDRESS extension file excluded | Pass | Pass |
| Retained built interfaces | Pass | Pass |

The interface control uses the current downstream build's 84 member RXBIN files;
these are not claimed to be the original September 15 build artifacts. Source
inputs and binaries in the downstream checkout were only read. No project build,
application runtime, model workload or downstream edit was performed.

## Mechanism

1. `closefile` has an exported inline payload with an exact dependency on
   `_rxsysb._close`; `lineout` has no inline payload in the inspected generated
   library. Import attachment resolves inline dependencies even with `rxc -n`.
   Relevant code is `inline_meta_resolve_callable_dependency_symbol()` in
   `compiler/rxcp_inline_payload.c` and `sym_imfn_impl()` in `compiler/rxcpfunc.c`.
2. This changes available namespace/source-loading state. The retained stack
   shows `validate_symbol_in_scope()` calling `sym_imva()` in `ragtrace`, which
   calls `load_another_file()` and recursively validates
   `rag_address_environment.crexx`. Its imports lead back into declarations
   whose owning source modules have not finished validation.
3. `parseRexxFileForFunctions()` validates an imported body before extracting its
   procedure/class signatures. A file already being loaded is skipped rather
   than supplying its unfinished declaration contract. The resulting incomplete
   contracts produce the two observed symptoms below.

**Convergence:** `ragcommandutil.commandeffectrecords` obtains `fields` from
`records[1].fields()` while the record contract is unavailable. The retained
diagnostic trace shows the symbol entering validation with unknown type and one
dimension, then being reset to unknown type and zero dimensions, on iterations
14 and 15. Array use restores the dimension, so `FLAG_VAL_SYM` remains set until
the 16-iteration limit aborts. This is not evidence that the compiler merely
needs a larger iteration limit.

**Provider mismatch:** the class signature collector cannot resolve
`.ragworkprovider` while validating `ragapplicationprovider`. It nevertheless
exports the class with `implements_count=0` while its source AST contains errors.
The retained trace records `MISSINGIFACEDETAIL`, `SIGNATUREDETAIL ... count=0
errors=1`, then the master `ragprocess` context rejects
`ragapplicationprovider.ragapplicationprovider` as assignable to
`ragworktypes.ragworkprovider`. Both symbols are found and correctly classified
as class/interface; the missing implementation relationship causes rejection.

These share the recursive import/unfinished-contract trigger, but their failing
compiler stages differ. The claim is not that all source-import cycles fail,
that `closefile` is invalid, or that the runtime close operation is broken.

## Independent reproductions

`reproducer/` has three files (32 lines total): a record with an array-valued
method, an exposed-global cleanup procedure calling `closefile`, and an `_rxsysb`
extension referring back to the record. Compiling `model.crexx` fails with the
same convergence diagnostic.

`provider-reproducer/` has five files: the primary caller, a provider interface,
its implementation, cleanup and an `_rxsysb` extension. Compiling `main.crexx`
fails with the same `TYPE_MISMATCH` on `provider`.

For **both exact installed compiler versions**, both reproducers fail in opt
and noopt. Changing only `closefile` to `lineout`, or removing only the extension
file, passes in both modes. All 24 observed outcomes matched these expectations.
Each compiler process is independent; no timing, retries or concurrency is
needed. No program is executed, so the fixture cannot close an agent's stream.

Run from hotfix:

```sh
python3 docs/qa/issue-699-20260916/diagnose.py /Users/adrian/.local/bin/rxc
python3 docs/qa/issue-699-20260916/diagnose.py /Users/adrian/.local/bin/rxc provider
```

The script deliberately asserts the **defective baseline**, writes all outputs
to a new temporary directory and records exact commands/source hashes. Its zero
status means the expected failure/control pattern was observed, not that the
product passed a regression. For repair qualification, turn the failure cases
into ordinary compile-success regressions and keep these baseline logs.

## Repair recommendation and limits

Keep the repair in #699. Start with exact inline-dependency resolution and
recursive source-declaration availability: importing a known callable's internal
dependency must not leave dependent compilation using incomplete contracts.
Do not publish a source class as a complete imported contract while its declared
interfaces remain unresolved. Preserve legitimate namespace extensions and
actual dependencies rather than globally excluding `_rxsysb` sources, disabling
inlining, increasing the iteration cap, or changing application cleanup calls.

The two reproducers should become separate focused regressions, with opt/noopt,
source/binary import controls and genuine invalid-interface negative controls.
Check existing source-import, binary-forward-dependency, global-import and inline
metadata tests as appropriate to the eventual edit. A concrete patch has not
been selected or qualified. If the repair needs a new multi-phase import
architecture rather than a bounded correction to existing lookup/validation,
present that design for Adrian's approval first.

This assessment used local macOS compilation only. It does not claim Linux,
Windows, sanitizer or runtime qualification. No deep/overnight workflows were
dispatched: those would not improve this diagnosis. No compiler/runtime source,
installed binary, Git branch tip, main checkout or GitHub issue was changed.
At the end of the assessment, the planning record, fixtures and evidence were
uncommitted on hotfix.

The diagnostic compiler was linked in scratch from the existing normal Debug
archives with instrumented copies of three translation units placed before the
archive on the link line. Product source and build outputs were not replaced.
`evidence/diagnostic-instrumentation.patch.gz` retains those observational changes;
`initial-diagnostic-build-command.json` records the initial link and flags (the
later trace added `rxcpfunc.c.o` alongside the two listed overrides). This is
debugging evidence, not a proposed repair patch.

# OPT-BOUNDARY-01 correction

Date: 2026-09-18. Base revision: `15c8a3ba42009ab8b5a9b447aa8c06ce86b9b392`.
Status: published to develop in `f7a8b08c1`; combined code head `e99136a1d`
passed normal Build CREXX and CodeQL. See the defect-batch receipt below.
The regression contract is valid handwritten assembly whose incoming arguments
may share storage. The repair preserves the current ISA and calling convention.

The shared SSA value model now distinguishes exact StorageId equality from
possible aliasing between incoming argument/global bases. Linked aliases and
phis can retain incoming origins. A write through another possible entry alias
invalidates the affected component; it does not incorrectly transfer a value
from a merely possible alias. Exact identities and private frame-local storage
remain distinct. Fresh queries and cache revalidation share the same component
write-resolution helper. Call windows include the additional incoming-alias
relation. The existing dynamic/reference effect model is preserved.

The permanent `tests/rxas_optimizer/entry_alias_runtime.rxas` fixture covers
eight runtime cases: aliased arguments, distinct arguments, a detached local
copy, explicit links, a join, repeated constant writes, argument/global aliases,
and a call that mutates an aliased global. Both optimized and unoptimized
variants are registered in CTest through the existing runtime matrix helper.

Evidence:

- [before.log](before.log): permanent unoptimized regression passes and the
  optimized regression fails before the code repair.
- [initial-overconservative-repair.log](initial-overconservative-repair.log):
  a broader possible-alias treatment lost one valid attribute-path reuse.
  This variant was narrowed to add entry-base aliasing without replacing the
  existing dynamic/reference model. The old expectation was preserved.
- [validation.log](validation.log): final affected tools/runtime fixtures build
  and **102/102** RXAS optimizer, flow graph and runtime tests pass. This includes
  the attribute-path positive control and the permanent alias regression.
- [original-reproducer.json](original-reproducer.json): archived original
  [assembly](../artifacts/entry-alias.rxas) now prints `42\n42\n` in both modes, including the independent
  global-write control. The second conversion is retained in
  [optimized disassembly](opt.disassembly).
- [manifest.json](manifest.json): qualified source and tool hashes.
  [repair.patch](repair.patch) records tracked code/build-input changes;
  the new runtime fixture is retained in the repository tests directory.

Validation command after building the named tools and eight runtime targets:

```sh
ctest --test-dir cmake-build-debug --parallel 4 --output-on-failure -R '^(rxas_optimizer_|rxas_flow_graph_contract|((entry_alias|whole_procedure|nr18_flow|storage_identity|redundant_itos|signal_contract|copied_xtoy_component_placement|branch_thread)_runtime_))'
```

No production VM fusion or compiler AST change was made. This repairs wrong
code under the current calling convention; it is not a new performance
enhancement. This local evidence does not claim a benchmark verdict, sanitizer
qualification, hosted or cross-platform result, or general proof of the alias
model. The combined publication check is recorded in the
[defect-batch evidence](../../beta3-defect-batch-2026-09-18/README.md).

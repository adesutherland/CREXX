# cREXX Linker Architecture (`rxlink`)

The `rxlink` tool combines one or more assembled `.rxbin` modules into a linked image that keeps module boundaries while deduplicating compatible constant-pool leaves into one shared pool.

## Purpose

Use `rxlink` when you want to:

- bundle a root module with the providers it needs
- turn a loose module set into one deployable linked image
- shrink downstream `rxcpack` / wrapped artifacts by removing duplicated pool entries
- optionally strip source/TRACE debug metadata from deployable images

This is now the normal native-packaging route for the shipped drivers too:
`crexx`, `crxc`, `rxpp`, and related wrapped tools link a deployable image
first and then pass that linked image to `rxcpack`.

`rxlink` is not a replacement for the VM loader. The output still contains multiple module records, and `rxvm` still performs the final runtime link/load work.

## Native-provider requirements

Selected `META_PROVIDER` records are preserved and checked against their
matching `META_FUNC` signatures. `rxlink` rejects the same callable when inputs
associate it with different stable providers, return types, or argument
signatures. Requiring-module provenance is retained for diagnostics.

Use `-p requirements-file`, or `PROVIDERS requirements-file` in a control file,
to write the packaging projection without loading native code:

```text
CREXX-RXPA-REQUIREMENTS 1
required<TAB>provider<TAB>callable<TAB>return-type<TAB>arguments<TAB>module
```

This is the authoritative input to `crexx -native`. Each provider value is the
canonical artifact stem: native packaging prefers `<provider>.a`/`.lib` and
falls back to the historical `<provider>_static.a`/`.lib` name. A native
package therefore does not maintain a second hand-written provider list.

## Module initializers

Selected `META_INITIALIZER` records are runtime contract metadata. `rxlink`
checks that each record names a local `META_FUNC`/procedure with `.void` return
and no arguments, remaps its symbol and procedure references into the output
pool, and preserves metadata order. Initializers do not make their procedures
exports or selection roots.

Linked images retain their individual module records. This is essential for
the once-per-mutable-module-instance state machine: multiple initializers in
one module retain declaration order, while the VM can still initialize and
poison each module overlay independently. Source and inline stripping never
removes initializer metadata.

## Output Format

RXLINK reads and writes only RXBIN 007. The complete toolchain moved atomically;
006 inputs are rejected and rebuilt rather than decoded through a compatibility
path. The format and semantic graph design are in
[RXBIN_007_SEMANTIC_GRAPH.md](RXBIN_007_SEMANTIC_GRAPH.md).

The 007 output is one fixed-width sectioned container with a module directory,
module instruction ranges, shared constant/canonical metadata data, and one
image-wide semantic graph. The current shared-pool/module record stream is not
carried forward.

Milestone 1 retains metadata-driven module selection. For the selected modules
RXLINK rebuilds canonical text-backed type/member/callable/factory
nodes, preserves declaration origin, assigns new image-local dense IDs,
rewrites graph-bearing instruction references, and rebuilds every adjacency,
name, declaration, dispatch, callable, factory, and provider index. It never
concatenates module-local indexes.

The common `rxbin` graph library owns structural merge, remap, validation, and
fast rule-neutral traversal. Future language-policy adapters may decide
inheritance, assignability, override/default conflicts, and provider selection
without embedding those language rules in RXBIN itself.

## Selection Model

Inputs are read as a container stream, so one input file may contain a linked
multi-module container or concatenated standalone library containers. Module
selection then happens in this order:

1. apply `OMIT`
2. apply `INCLUDE`
3. apply explicit `ROOT`
4. if no roots were chosen, select modules containing `main`
5. if there is still no root, select modules from the first input file
6. walk imports, `srcfprocsel` interface references, and interface relationships to pull in required providers
7. reject duplicate selected exports

Selectors match by:

- full module name
- basename
- filename stem with trailing `.rxas` removed
- `input_path::member`

## What The Linker Reads From Metadata

`rxlink` does not need all metadata equally:

- `proc_head` is used to find procedures and detect `main`
- `expose_head` is used to discover imports and exports
- `meta_head` is scanned for `META_INTERFACE` and `META_IMPLEMENTS` so interface definitions and implementations pull each other in
- the instruction stream is scanned for `srcfprocsel` descriptor strings so
  modules referenced only through runtime interface-factory lookup are still retained
- the instruction stream is scanned for `srcmethodsel` member names. Because the
  receiver's concrete class can be known only at runtime, modules that expose
  or declare a matching member name are selected conservatively.
- selected class/interface contracts are then checked against callable
  descriptors so a provider with the right name but wrong return or argument
  signature is rejected before output is written.

The linker preserves the metadata chain in output because the VM and tooling still consume it at runtime.

Task metadata is also runtime contract metadata. `.task1`, `.task2` and
`.task3` entries carry an 80-byte sealed binding containing image digest,
callable id, signature digest and the optional adapter callable slot. Because
RXLINK rebuilds and renumbers the semantic graph, it must regenerate these
bindings from the linked graph rather than copy module-local bytes. Kind `2`
relocates the receiver `from_channel` factory and kind `3` relocates the
`.taskwork.run` method. Missing, malformed, stale or signature-incompatible
bindings fail the link; they are never weakened to procedure-name dispatch.
An imported task call contains the deterministic 80-byte relocation
placeholder but does not duplicate the defining module's `META_TASK_TARGET`.
The RXBIN writer therefore reseals both the metadata binding and every matching
placeholder constant across all selected constant pools. This is required for
separately compiled task-method clients: copying the defining module's old seal
would retain the wrong final graph digest, while leaving the use-site
placeholder would fail the `RXTB` magic check.

## Constant-Pool Rewriting

Leaf constants are deduplicated across selected modules when their serialized bytes match:

- `STRING_CONST`
- `BINARY_CONST`
- `DECIMAL_CONST`
- `FLOAT_CONST`

Structured constants are rewritten into the shared pool with all referenced offsets updated:

- procedures
- exposed register/procedure entries
- metadata entries
- instruction operands that point into the pool

Instruction rewriting derives its operand count and kind from the canonical
variable-length opcode signature. Linker scans and remaps therefore have no
three-operand format switch; wide instructions retain every inline operand
while constant and graph references are rewritten normally.

## Strip Support

Current conservative strip support has two independent axes:

- CLI: `-s`
- control file: `STRIP SOURCE`
- CLI: `-i`
- control file: `PRESERVE INLINE` / `STRIP INLINE`

`STRIP SOURCE` removes:

- `META_SOURCE_STEP`
- `META_TRACE_EVENT`

Trace-event metadata is source-level debugging metadata. Without source-step
anchors, classic `TRACE` value events are not coherent enough to keep in a
deployable stripped image, and they may still expose variable names, compound
names, constants, or live values. Keep the linked image unstripped when TRACE,
RXDB source stepping, or source-level diagnostics are needed.

Inline-body metadata is different from runtime contract metadata. It is useful
to libraries consumed by `rxc`, but it is not needed once a final linked image
has been built. `rxlink` therefore strips `META_INLINE` by default. Use `-i` or
`PRESERVE INLINE` only for diagnostic/tooling builds that need to inspect the
inline transport after linking.

It intentionally does not remove runtime contract metadata such as:

- `META_CLASS`
- `META_ATTR`
- `META_INTERFACE`
- `META_IMPLEMENTS`
- `META_MEMBER`
- `META_INITIALIZER`
- sealed `.task1`/`.task2`/`.task3` bindings

That keeps interface/class dispatch and metadata-aware tooling behaviour stable while still removing source text/file path payloads and source-level TRACE value metadata.

## Control Files

Supported directives are:

- `INPUT path`
- `ROOT selector`
- `INCLUDE selector`
- `OMIT selector`
- `OUTPUT path`
- `MAP path`
- `STRIP SOURCE`
- `STRIP INLINE`
- `PRESERVE INLINE`

## Testing Guidance

When changing `rxlink`, keep three layers of coverage in mind:

1. format tests: shared-pool/shared-module record layout
2. behavioural tests: linked images run correctly through product `rxvm`; add
   the concrete `rxbvm`/`rxtvm` dispatch contract only for execution-image or
   dispatch-sensitive changes
3. toolchain tests: `rxdas` can still disassemble linked images, including stripped ones

For broader confidence, the runtime `_opt` path is now wired through linked
images in normal `ctest` coverage. For a focused rerun, use:

- `ctest -L linked_opt --output-on-failure`
- `cmake --build <build-dir> --target linked_opt_sweep`

Be conservative with stripping. If a proposed change removes anything beyond
the current source-level debug set (`META_SOURCE_STEP` and `META_TRACE_EVENT`),
verify both runtime contract lookup and metadata introspection in `rxvm` before
assuming it is safe.

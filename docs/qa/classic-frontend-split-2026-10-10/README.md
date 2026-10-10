# Classic frontend split — local qualification, 10 October 2026

The approved C-first implementation and bounded local qualification are complete
on `develop`, based on synchronized
`91313bcc5a447e9403f95b1f0bfc4dd9503d0618`. Delivery is local and unpublished;
no push, hosted pass or release is claimed. The authoritative scope is
[COMP-PIPE-01's C-first phase](../../ROADMAP.md#proposed-c-first-extraction-phase--2026-10-10).
The broader B/G migration and Level C compatibility programme remain open.

Adrian explicitly assigned **SAN-011 and SAN-012 full closure to Codex at the
next Release 1 release-QA gate**. Both repairs have permanent regressions and
focused normal/maintained-sanitizer proof. They remain open and release-blocking,
alongside the previously assigned SAN-QA-018. This assignment permits bounded
local closeout; it supplies no sanitizer-clean or release-ready claim. The
[sanitizer register](../../SANITIZER-WORKLIST.md) owns the remaining actions.

The subsequent [CLion clean-rebuild repair](clion-clean-rebuild.md) records a
missing TRACE runtime dependency in two benchmark consumers, both successful
Debug/Release product builds and five focused checks. Its one build-input delta
is recorded separately; the original qualification below retains its own inputs.

## Delivered boundary

`rxc-c`/`rxcc_lib` combine the Classic frontend and shared backend. Full
`rxc`/`rxclib` retain the typed frontend and desktop editor API. The three object
components compile common code once per configuration. Stage/install include
`rxc-c`; normal `rxc`/`crexx` routing remains compatible.

Common and C sources use checked stable token identities in `rxcp_tokens.h`.
The C-only build generates no B header/scanner; final Release symbols contain
the Classic parser and exclude the typed parser, G task walkers and typed exit
host. Typed source imports and editor requests use the installed owning `rxc`
outside the C process, through explicit argv/private temporary RXAS. The broker
retains import-root order, RXAS opt-in and disabled-exit policy.

The original direct-to-AST metadata-field prototype was replaced with the agreed
independent binutils `rxcontract` reader and separate `rxcp_contract` AST adapter.
The reader returns owned language-neutral records and links without compiler
Context/AST or source parsers. The [metadata conformance boundary](../../../compiler/docs/metadata_conformance.md)
documents admitted type/signature/class/layout fields, malformed input,
ownership, versions and separately validated optional inline payloads. RXBIN
007, callable descriptors and enum/payload identities are unchanged.

Both B- and G-produced signed activation-argument providers execute through the
existing Classic external CALL bridge. Future C-to-G argument conversions or
task invocation remain separate language decisions. Supported imported inlining
is retained, including the distinction between factory body `.void` contracts
and object-result call-site contracts.

The internal exact-length `rxcc_parse` API owns the input, mapped tree, diagnostics
and teardown. Frontend success precedes common backend validation/emission.
Tests cover embedded literal NUL, caller-buffer mutation, two live units,
failure/success repetition, unsupported INTERPRET and backend import-failure
cleanup. It is a prerequisite for later runtime compilation; this phase does
not implement INTERPRET, source-to-RXBIN embedding or concurrent compilation.

## Qualification and evidence reuse

[qualification.json](qualification.json) records log/result hashes, physical
closure, scheduling and limits. [qualified-inputs.json](qualified-inputs.json)
records the baseline and 78 changed code/test/build input hashes. The retained
complete manifest covers 6,976 inputs and matches the Linux focused snapshot.
No code/test/build input changed during documentation closeout. Test-name lists
and [evidence-summary.txt](evidence-summary.txt) retain compact permanent proof;
full logs remain at their recorded local paths.

| Check | Observed result |
| --- | --- |
| Normal Debug core product and QA preparation | Pass; final ownership change rebuilt and exercised below |
| Relevant compiler/Classic/import/provider/library correctness | **1,048/1,048**, 2,691.64 seconds |
| Final affected import/inline/editor/metadata controls | **84/84**, 349.57 seconds |
| Release product stage, install, installed direct/linked parity | **109 tool commands pass**, 37.27 tool seconds |
| Clean C-only Debug build, parser mode/testing off | Pass, no generated B header/scanner |
| Maintained single-threaded compiler | **6 comparisons pass**, exits enabled; invalid literal rejected by both compilers |
| Isolated expanded Debug parity measurement | **109 commands pass**, 88.81 tool seconds |
| Isolated maintained Apple ASan parity/API/builder/reader measurement | **109 commands pass**, 184.79 tool seconds, plus unit checks |
| Final ownership regression, normal Debug | **1/1 pass** |
| Final ownership regression, maintained Apple ASan | **1/1 pass**, run `20261010-171147-ctest` |
| Final ownership regression, Linux ARM64 Clang ASan/LSan | **1/1 pass**, run `20261010-161146-ctest`, leaks enabled |
| Independent reader/builder, Linux ASan/LSan | **2/2 pass**, run `20261010-154901-ctest` |

The 1,048-test sweep preceded the last broker-policy and detached-anchor cleanup
changes. The 84-test final replay covers those affected imports, source/binary
inlines, editor controls and contract ownership; final Release/package parity
and normal/Apple/Linux ownership checks use those final inputs. Reuse the
unchanged wider evidence rather than claiming a repeated full normal suite.
The subsequently registered `classic_frontend_parity` adds one test to that
normal selection and is included in the final replay.

`classic_frontend_parity` is `RUN_SERIAL` with a 1,800-second timeout, chosen
after isolated normal/maintained-ASan measurements and retained after its
109-command expansion. The generous timeout protects against hangs rather than
establishing a performance requirement. No production performance claim is made.

The broad normal selection can be reproduced with:

```sh
ctest --test-dir cmake-build-debug \
  -L 'compiler|levelc|imports|rxpa|classes|interfaces|library' \
  -LE 'performance-measurement|optimizer-parity' \
  -E '^classic_frontend_parity$' --parallel 30 --output-on-failure
```

The final affected replay was:

```sh
ctest --test-dir cmake-build-debug \
  -R 'classic_frontend_parity|compiler_contract_builder|metadata_contract_conformance|^source_import_|inline_cross_file_(source|binary)_opt|class_import_opt|18_array_class_opt|syntaxhighlight_|highlight_editor_diagnostics' \
  --parallel 15 --output-on-failure
```

The parity workload additionally checks B/G source-only providers, a G provider
with a source-only RXAS dependency, default/explicit C mode, routine mode,
optimized/no-opt assembly equivalence, linked execution, portable/threaded VMs,
invalid IF and unsupported INTERPRET diagnostics, source NUL, Unicode identities,
and paths containing spaces/non-ASCII text.

## Reconciled diagnostics and goldens

Parser snapshots no longer contain two unused eager stream-inline debug traces.
The two ADDRESS parser snapshots show neutral imported declarations rather than
synthetic B source; authored AST prefixes remain unchanged. Optional type
placeholders no longer manufacture executable literal defaults absent from the
contract. Existing source and binary inline suites preserve supported shapes.

The optimized `class_import` and `18_array_class` fixtures change source-step
coordinates to actual provider declarations. Every executable instruction is
unchanged when source-step/trace metadata is removed for comparison. The
function-shadowing warning retains its imported identity and reports
`global="1"` instead of a fabricated line 3, column 1. Runtime output is retained.

An earlier partial broad sweep exposed genuine factory-inline, syntax-diagnostic
and provenance regressions in the first prototype. It is superseded by the
recorded repairs and passing current checks; it is not full-pass evidence.

## Sanitizer repairs and remaining gate

SAN-011 copied declaration anchors across Contexts without owning them, causing
a use-after-free during runtime-library teardown. Both AST duplication and
generated-node anchor transfer now copy marked metadata anchors into their
receiving Context. The permanent builder regression frees the declaration owner
before consumers. The original runtime trigger and parity target passed after
repair; the final builder proof passes on normal, Apple and Linux configurations.

SAN-012's visible-tree cleanup missed annotations on detached owned anchors.
Semantic cleanup now visits the Context allocation list. The permanent regression
creates an empty visible tree, adds a detached copied anchor, annotates it,
clears/resynchronizes semantics and destroys the Context. Final Linux focused
proof has LSan enabled. Original diagnostics are retained as
[SAN-011](san-011-original.log) and [SAN-012](san-012-original.log).

Full Apple run `20261010-171343-full` completed build/preparation and was stopped
at CTest. Full Linux run `20261010-161345-full` was stopped during preparation.
Both were intentionally stopped following Adrian's later-gate assignment;
neither is a completed full-platform pass. Earlier full attempts also stopped
for substantive input changes/findings. No suppression or first-party test
exclusion was introduced. Apple LSan is unsupported; leaks-off there is a
platform capability limit, covered by supported Linux leak-enabled proof.

At the next Release 1 release-QA gate, **Codex** must replay permanent focused
regressions and original runtime-library triggers, then complete broad supported
Linux ASan/LSan and Apple sanitizer closure on the release candidate inputs.
SAN-011, SAN-012 and SAN-QA-018 stay release-blocking until their conditions pass.

Windows/MSVC, Intel, native CMS/TSO, 24-bit resident memory and hosted publication
workflows were not run here. Full B/G migration, fast/lite products, direct
assembler handoff, runtime module reclamation and INTERPRET remain separately
owned open work. The delivered status is a locally qualified, unpublished
compiler extraction with the explicit remaining release-QA obligations above.

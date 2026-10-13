# Level L attached AST mappings — first delivery

The approved `then tree` surface, common walks and checked reconstruction are
implemented in the existing Level L develop checkout, based on `ec481f8e6`.
This is local macOS ARM64 Debug qualification. Adrian authorized publication
to `develop` and light CI monitoring on 13 October. The enclosing commit is the
publication payload. Qualified product/test inputs match the retained hashes;
only publication wording and its manifest changed during closeout. Normal
automatic CI is expected to pass; actual hosted results belong to the workflow
records and conversation. This receipt does not claim hosted/platform QA.
The authoritative scope and remaining criteria are in [PLAN.md](../../PLAN.md).

## Delivered behavior

- `then tree (fields)` inherits the declaration's label. An explicit kind, such
  as `then tree Add(left = left, right = right)`, renames the node. Captures
  supply children or original text; `?`, `*` and `+` express optional and list
  fields. Inferred contracts check names and counts with grammar locations.
- `request.ast()` constructs an owned application tree from a successful unique
  parse. Only the selected derivation is projected. Failed/ambiguous requests
  cannot become executable ASTs implicitly. Existing named views and binary
  projections remain available.
- Small borrowed handles and iterative walks avoid per-node arena copies.
  Children-first reconstruction sees rewritten children, seals accepted drafts,
  preserves its input and original source, and publishes only a complete checked
  result. Resource limits include discarded rewrite intermediates.
- The calculator uses named AST fields and a common walk instead of a private
  traversal stack. `fold.crexx` demonstrates constant folding into a new Number.
  Settings lesson 9 combines lists, named/text fields, Unicode, comments and
  embedded expressions. Both combined and separate-library builds work.
- The manual explains the representations, grammar-to-tree mapping, lifetime,
  walking, rewriting, source origins and complete build/run commands. The
  [contract note](../../AST-CONTRACT.md) separates structural termination and
  storage proofs from application-specific semantic obligations.

A regional parser does not retain an `operators` wrapper. Attaching a mapping
to that absent wrapper is rejected at build time with `LL_AST_REGION`; map its
tokens, infix/prefix operators or an enclosing parser rule instead. The worked
composite settings example uses those supported mappings.

## Qualification

| Check | Result | Evidence |
|---|---|---|
| Product and Level B bootstrap | Debug stage-product passes without compiler warnings; L1/L2 RXAS agree | `product-build.log`, `admission-build.log`, `inspection.json` |
| Independent unoptimized Level L | Standalone Level B build passes; optimized/unoptimized self-description RXAS agree | `noopt-build.log`, `admission-noopt-build.log`, `inspection.json` |
| Runtime and normal integration | 26 CTests pass: 20 new direct VM cases, existing TinyExpr/generated-language cases and both RexxScript VMs | `runtime-tests.log` |
| Runtime with unoptimized Level L | Four opt/noopt consumer × VM cells pass, including shared-child rejection | `runtime-noopt.json` |
| Generated AST surface | 216 target observations per Level L optimization setting, across LL(1), LR(1), packrat, Earley, Pratt and regional examples; both source/RXAS routes and both VMs | `surface-opt.json`, `surface-noopt.json` |
| Final grammar diagnostics | 27 cases per optimization setting: nine refusals through L0/L1/L2, with checked line/byte locations and no emitted module | `diagnostics-opt.json`, `diagnostics-noopt.json` |
| Calculator behavior | 176 positive/error observations pass, including decimal arithmetic, comments, grouping, denominator locations and parse-before-evaluation | `calculator.json` |
| Binary projection compatibility | 64 original projection observations pass across four backends and both emission routes | `binary-projection.json` |
| Installed build story | 14 commands pass: calculator, folding and settings programs on both VMs, no-op rebuild and optional reusable grammar library | `installed.json`, `check-installed.py` |
| Manual and portability | 85 local manual links resolve; three Windows-path guard unit tests pass | `inspection.json`, `inputs.json` |

The original full surface receipts contain 240 observations each: 216 target
observations and 24 grammar refusals. Final diagnostic receipts supersede those
24 refusals and add the three regional-wrapper refusals. The nonempty-list
message now correctly identifies the possibility of an absent item. Do not add
the original and superseding diagnostic counts as independent coverage.

Those final edits are confined to frontend admission/diagnostics. Every runtime
module in `runtime-components.json` is byte-identical before and after them.
Retained runtime, calculator and binary-projection results therefore remain
applicable. `final-controls.json` checks the final generator against unchanged
valid output and records installed post-update controls. The complete product
and bootstrap were rebuilt after the admission edit. No unchanged broad or
Unicode matrix was rerun merely to obtain another green result.

The permanent CTests generate and link fixtures during the build, then each
CTest launches one VM. They add no nested build/test matrix to the broad pool.
The larger explicit surface panel remains available as:

```sh
python3 lib/rxfnsl/tests_functional/check-ast-surface.py BUILD OUTPUT
```

Use `--diagnostics-only` for a focused admission/wording replay. The existing
standalone adapter supports the same panel with the entire Level L library and
generator compiled without optimization.

## Remaining programme work

The reviewed first AST surface and examples are complete. Wider AST criteria
remain open for synthetic/recovered origins, general analysis scheduling and
fact invalidation, grouping/lowering, a bounded RexxScript migration and a
compiler-shaped builder/validator example. This delivery does not migrate the
production cREXX AST or prove arbitrary application rewrites equivalent.

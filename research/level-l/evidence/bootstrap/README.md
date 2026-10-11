# Checkpoint 1: Level B bootstrap and native Unicode lexing

Local review delivery on `levell`, 11 October 2026. The owning scope and
BOOT-AC01–08 ledger are in [PLAN.md](../../PLAN.md); build/use and the scalar
lowering argument are in [the bootstrap guide](../../../../lib/rxfnsl/doc/bootstrap.md).
The unchanged existing C toolchain is a prerequisite. This is not a
repository-wide re2c removal, a merge, publication or release qualification.

## Results

| Check | Retained result |
|---|---|
| Isolated build lifecycle | [Seven phases](lifecycle-final/receipt.json): configure, cold, no-op, grammar edit, runtime edit, invalid grammar, restoration. The source snapshot contains no re2c or Level G; a re2c blocker records zero invocations. Failure preserves the last valid generated image/library. |
| Self-hosted frontend | [17 cases × 3 stages](frontend/receipt.json), including lexer-only, parser-only graph inspection, admission failures and LF/CR/CRLF locations. L1/L2 generated RXAS agree; fully optimized/unoptimized bootstrap outputs also agree. |
| Independent lexing controls | [Optimized core](controls/receipt.json) and [unoptimized core](noopt-controls/receipt.json): 5 panels, 2,700 cases per core configuration, each with compiler/assembler opt/no-opt probes on both VMs. Scalar samples/boundaries, subtraction, bounded Unicode expressions, historical TinyExpr and gennorm2. These are bounded samples, not exhaustive enumeration of all scalars or regexes. |
| Owning Unicode API | [17 cases × 8 cells](gennorm2-compat/receipt.json): historical/native scanner, opt/no-opt adapter, both VMs; token spans, EOF, typed records and failure observations agree except the documented NUL correction. |
| Product consumers | [8/8 CTests](logs/consumers.log): generated consumer on both VMs, RexxScript's 60-case panel on both VMs, four owning Unicode normalization cells. Normalization regeneration leaves its logical generated source unchanged; existing executor and pinned Unicode data inputs are unchanged. |
| Tutorial and diagnostics | [46 commands](user-guide/receipt.json): settings source-to-tree output on both VMs, ten damaged inputs on both VMs, named lexical action, frontend failures, ineffective tree annotation and no execution following RexxScript parse failure. This verifies first-error rejection, not recovery. |
| Source/RXAS emission | [2 panels, 160 inputs × 4 cells](source-emission/receipt.json): source output matches the independent RXAS controls exactly, including Unicode, named error messages and EOF. Compiler output has no warnings. |
| General generator rejection | [9 cases](generator/receipt.json): deterministic artifacts, unsupported grammar profiles, recursion/nullability/conflicts, resource exhaustion, I/O/UTF-8 and atomic failure. |
| Historical research consumers | [125 frontend cases on each VM](logs/research-front.log) using the maintained generated frontend. Other research engines were not rerun; their existing compiler warnings remain visible in the retained [build log](logs/research-front-build.log). |

The self-description generates a 50-state DFA and 1,969-state canonical LR
parser. Its L1/L2 RXAS SHA-256 is
`b51533a152b81879d5b6dd30ff1b5654d1d119fccb9ef2967c1dc1356982b58e`.
This equality is a reproducibility check; independent oracles and historical
controls supply separate correctness evidence.

## Provenance and evidence reuse

[receipt.json](receipt.json) identifies current sources, tools, stage artifacts,
Unicode inputs and generated output. [unicode-inputs.json](unicode-inputs.json)
records the nine checked Unicode 17.0.0 input hashes. Historical grammar inputs
come from `9a86fb37c0d0d7b9cd395d676e8a0a3ef45e39b8`, under
`experiments/unicode/level-l-bootstrap/specs/`. TinyExpr is unchanged; gennorm2
changes only its namespace to permit the ABI adapter to retain the old name.
The old generated scanner is retained as `reference-gennorm2.crexx` solely for
comparison and is absent from the product build.

The final source-emission check exposed unnecessary parser/stage imports in
lexer-only source output. The emitter now adds them only when used. This cleanup
occurs after the RXAS emission branch, and rebuilt optimized/unoptimized tools
retain the exact self-description hash. The public runtime image is unchanged
from the successful tutorial/consumer controls. Those controls are reused;
source emission was retested after the change. The isolated lifecycle test's
[current-source delta](lifecycle-final/current-source-deltas.json) identifies
this cleanup and the deliberate diagnostic-literal mutation inside its private
runtime-edit test. It does not represent the mutated test copy as product code.

Initial failures are retained in `logs/initial-*.log` rather than erased:

- The first independent bounded-regex oracle allowed partitioning a longer
  maximal-munch match into shorter tokens. It now uses greedy token selection.
- A test diagnostic name collided with a rule name; the test was corrected.
- The initial runtime-edit check appended a stripped comment and wrongly
  expected a bytecode relink. It now changes a diagnostic literal to make the
  runtime edit observable.
- The updated tutorial fixture shifted the reference line by one; the check
  now derives the expected line from the authored source.
- The source-emitter unused imports were repaired and retested.
- The historical gennorm2 NUL discrepancy is a product compatibility correction:
  NUL in a comment is invalid input, not an EOF sentinel. The comparison retains
  and explicitly asserts the differing old/new outcomes. Accepted prefix tokens
  and all other owning parser observations are preserved.

## Reproduce focused checks

Use an existing native build containing the unchanged C tools and Level B
library. For the isolated cold-build test, supply a new work-directory name.
These are standalone QA scripts, not newly registered nested CTest aggregates.

```sh
python3 lib/rxfnsl/tests_functional/check-bootstrap-build.py \
  research/level-l/build/frozen-tools-20261010 \
  research/level-l/build/bootstrap-lifecycle-new \
  research/level-l/evidence/bootstrap/lifecycle-new
python3 lib/rxfnsl/tests_functional/check-front-bootstrap.py \
  cmake-build-levell-product research/level-l/evidence/bootstrap/frontend
python3 lib/rxfnsl/tests_functional/check-bootstrap.py \
  cmake-build-levell-product research/level-l/evidence/bootstrap/controls
python3 lib/rxfnsl/tests_functional/check-gennorm2-compat.py \
  cmake-build-levell-product/bin research/level-l/evidence/bootstrap/gennorm2-compat
python3 lib/rxfnsl/tests_functional/check-source-emission.py \
  cmake-build-levell-product research/level-l/evidence/bootstrap/controls \
  research/level-l/evidence/bootstrap/source-emission
python3 research/level-l/cases/check-user-guide.py
```

The fully unoptimized core is built by `lib/rxfnsl/tools/build-local.sh noopt`;
pass its `product-noopt/bootstrap-build` directory to `check-bootstrap.py` with
`--tools research/level-l/build/frozen-tools-20261010` and a separate output path.
The frontend check expects that no-opt self-description to exist.

## Remaining scope

Native macOS ARM64 only. No fresh broad sanitizer, installation, Release or
cross-platform claim. Cross builds reject before executing target tools;
separate host/target packaging remains unqualified. Runtime recovery, richer
syntax explanations, tree declaration admission and the final backend portfolio
belong to the next checkpoint. The seven original open research criteria,
RS-F01/RS-AC04 and full LL-D01–06 obligations remain open. The normal product
builds used here are warning-free; historical research module warnings are not
claimed repaired.

# User-manual review — 12 October 2026

This receipt verifies DOC-AC01–04 in the authoritative plan. It covers the
manual rewrite, practical backend starting points, one new PEG teaching
grammar and its focused regression panel. It does not change the Level L
generator/runtime or close wider LANG/research criteria.

## Documentation review

The manual follows introduction → complete example → progressive extensions →
backend guidance → reference. Technical vocabulary is defined in context and
in a glossary. Dated implementation history, qualification reports, roadmap
items and repetitive proof caveats are omitted from the manual. Required
usage restrictions remain next to the affected API or declaration.

Owning sources checked include the self-grammar and lowering, generator,
language request, tree/view/projection, forest, recovery, source/comment
coordinator, token stages and resource policy. Corrections include:

- Strict status 5 and the separate recovery status table; recovery's status 5
  does not supply a forest accessor.
- Forest positions count tokens; ordinary tree/token spans count bytes.
- Metadata string lookup retains case; capture fields fold case.
- Charset subtraction precedes union. The unparenthesized basic example
  accepts line endings inside strings; that behavior was also executed.
- `unless next` examines the second input token even for a two-token `take`.
- Standalone Pratt rejects parser-rule diagnostic attachments.
- Projection arity counts retained children, with one selected result root.
- `scan.work` bounds both source-comment work and DFA work, separately.
- Adjacent implementation notes no longer give stale construction limits or
  describe the resource policy as only its first six settings.

`document-check.json` records 80 resolved links, 61 paired code fences, exact
agreement between the complete printed grammar and `01-basic.levell`, and all
18 defaults checked against the live CLI. It also retains second-VM tutorial
output, multiline-string output and the documented missing-value location.

## Executable checks

- `build.log`: the configured Debug `levell_cli` target builds successfully.
  The optional fresh CMake configure command was inspected against the owning
  build graph; a fresh full product build was not needed for this edit.
- `settings.sh`, `calculator.sh`, `backend.sh`: copied verbatim from the guide
  with `set -e`. Their logs retain expected output from the native toolchain
  and `rxtvm`; `document-check.json` records matching `rxbvm` output.
- `projection/`: a complete minimal consumer of the newly explained projection
  example, including its source, commands and both-VM output. Seven commands
  pass, checking kind 10, no children, source range `[0,2)` and an origin node.
- `priorities/`: 140 commands, **72 runtime observations and two expected build
  refusals**. The PEG example distinguishes Boolean/bare/quoted values, longest
  lexical matching and trailing input. Reversing parser priority changes the
  selected node. Earley exposes two derivations for `true`; LL/LR refuse the
  conflicting CFG. Source/RXAS output, optimized/unoptimized generated modules
  and consumers, and both concrete VMs are exercised.

The new panel is part of `check-teaching-examples.py --part priorities` and its
default `all` run. It adds no aggregate CTest or nested-build scheduling change.
The existing calculator/backend panels have unchanged implementation inputs
and retain their earlier valid evidence.

The initial temporary projection probe imported an unused library. Removing
that unused import made its warning check pass; no product repair was needed.
The first document-check draft counted the two-line header fragment as a full
grammar; restricting that comparison to a block containing a lexer corrected
the check. Final evidence contains the corrected checks and inputs.

## Reproduction and identity

Run from the repository root after building `levell_cli`:

```sh
bash research/level-l/evidence/manual-review-20261012/settings.sh
bash research/level-l/evidence/manual-review-20261012/calculator.sh
bash research/level-l/evidence/manual-review-20261012/backend.sh
python3 research/level-l/evidence/manual-review-20261012/check-document.py
python3 research/level-l/evidence/manual-review-20261012/check-projection.py
python3 lib/rxfnsl/tests_functional/check-teaching-examples.py \
  cmake-build-levell-product research/level-l/build/manual-review/priorities \
  --part priorities
```

`inputs.json` identifies 58 source/document/example/tool/library files.
The baseline branch commit is `bf9aacc085f98ff97dd42f01b8bc906cc5e96ab8`;
the manifest identifies the edited documentation and new example/test inputs.
This is native macOS ARM64 Debug functional evidence with the existing optimized
Level L library. Broader product qualification is unchanged.

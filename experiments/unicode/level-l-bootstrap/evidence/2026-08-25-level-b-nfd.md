# Portable Level B NFD Evidence — 2026-08-25

Revision under test: local `unicode` branch work following `5e56ca1ae`.

Command, from the repository root:

```sh
experiments/unicode/level-l-bootstrap/run.sh
```

The command compiles the retained typed `gennorm2` AST into one versioned
portable Level B binary image, validates that image, and normalizes the complete
Unicode 17.0.0 NFD corpus under both `rxtvm` and `rxbvm`. It also regenerates
the independent C++ implementation from the retained re2c source and requires
its conformance output to match the retained Phase 2 result.

The Level B result is retained in `nfd-result.txt`. Its key evidence is:

```text
NFD table format: crexx-nfd-portable-at-v1
NFD table bytes: 1186472
NFD table SHA-256: dd7e03a88172dd451bf4c353771c565d07ade91f64ec8cbab81b9e6b33362944
NormalizationTest rows by Part 0..5: 45/17086/1936/194/735/38
NFD corpus invariant checks: 100170
Unlisted scalar identity checks: 1094978
Focused NFD fixtures: PASS
Result: PASS
```

Focused checks cover empty and ASCII identity, direct and recursive canonical
decomposition, stable combining-class ordering, Hangul LV/LVT decomposition,
non-BMP identity, CCC lookup, and cyclic mapping rejection. Both VMs emitted
byte-identical summaries and the independent C++/re2c oracle reproduced its
retained conformance evidence.

The implementation exposed one important Level B authoring requirement:
table-page lookup, binary search, and Hangul arithmetic must use the named
`<idiv>` and `<mod>` operators. Their meanings are independent of the source
file's `OPTIONS NUMERIC_*` mode, unlike `%`.

This evidence proves a correctness baseline, not a performance or product
format verdict. The image uses portable `<at..type>` fields and is regenerated
in memory; it is not committed as a binary. Host-native packed and
load/convert-once layouts still require measured comparison. NFC composition,
compatibility normalization, case folding, streaming, the general Unicode
property compiler, and the public Level G API are outside this result.

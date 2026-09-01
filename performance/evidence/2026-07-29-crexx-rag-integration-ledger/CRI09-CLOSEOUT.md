# CRI-09 closeout

Status: **fixed**

Date: 2026-07-30

## Accepted surface

Adrian approved Option B and accepted frozen A2 at `rxjson.crexx` SHA-256
`c2ff6f246ecf8f13837cd82a1f6c5e35cc6f855fa18889eef6a48ee0128a9318`.
The final generic Level B surface preserves every legacy `rxjson` function and
adds:

- one immutable `.jsondocument` with stable parse status, path compatibility,
  document-local node traversal, exact object-key lookup, ordered child
  traversal and strict typed getters; and
- `jsonscancontainer(text, from, document, start, after)`, returning an already
  indexed strict JSON object or array with one-based public character
  positions.

The parser is a hand-written streaming lexer/parser. A 256-byte table
classifies token-boundary bytes while tight loops consume string and number
interiors. Full-index, legacy-query/validation and recoverable-boundary sinks
share one recursive grammar without a token-list allocation.

B1 accepts the successful scanner construction exactly as follows: one
allocation-free boundary-validation pass over the original input plus one
indexing pass for the successful slice. Rejected candidate openers create no
candidate document or index; the public call initializes one invalid empty
output document so failure is deterministic. This is a deliberate two-pass
successful-slice lifecycle, not another implementation phase.

Schema/repair policy, packed numeric representations, public C ABI, language
syntax, RXAS/RXBIN and serialized formats remain outside CRI-09.

## Frozen maintained artifacts

| Artifact | SHA-256 |
| --- | --- |
| `lib/rxfnsb/rexx/rxjson.crexx` | `c2ff6f246ecf8f13837cd82a1f6c5e35cc6f855fa18889eef6a48ee0128a9318` |
| `lib/rxfnsb/rexx/rxjson.md` | `2059827237b4e7cf39e9223058f5af9bfe20d3bc799bee1745fa1d9c1e768188` |
| `docs/books/crexx_library_reference/levelb-rxjson.md` | `2059827237b4e7cf39e9223058f5af9bfe20d3bc799bee1745fa1d9c1e768188` |
| `ts_rxjson_document.crexx` | `aac116c42b6e19cc9d92268abd160c66044a4c10ee76968bee21ad251dadb21b` |
| `ts_rxjson_noisy_contract.crexx` | `70f69c54423ccc94521674151e7859705c678d250f408a454bd8925468af3076` |
| functional CMake registration | `bdf0f0172426d91d24feebd776fe079e89264bdfcbda77028964af5a15691912` |

The documentation defines immutability, node lifetime, byte versus character
sizes, parse codes, path and arbitrary-key traversal, typed getter statuses,
scanner continuation, deterministic failure output, B1's two-pass lifecycle,
duplicate-key behavior, and the separation between JSON semantics and schema
or packed-numeric policy.

## Correctness closeout

The dedicated candidate Debug tree is
`/tmp/crexx-integration-ledger.mZ5jyp/candidate-debug`.

```sh
cmake --build /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug --parallel 10
ctest --test-dir /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug \
  -R '^ts_rxjson' --parallel 10 --output-on-failure
ctest --test-dir /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug \
  --parallel 30 --output-on-failure
```

Results:

- complete candidate rebuild passed; raw log
  `/tmp/cri09-closeout-debug-build.log`, SHA-256
  `9e2fa441be36b5819c6a67e762813909d29a7162d20ffcbca2517d912017464f`;
- focused inventory is exactly 11 tests; list
  `/tmp/cri09-closeout-focused-list.txt`, SHA-256
  `493bca20ad21de68c76ce14139f3cbaee99f0a033042d9222829ff2ba3af2c6c`;
- focused Debug passed 11/11, zero failed or skipped, across legacy,
  document and noisy contracts, optimized/non-optimized compilation and both
  VMs; raw log `/tmp/cri09-closeout-focused-debug.log`, SHA-256
  `fe623f3e2c2ea036ca6f9558c397cc133e4aa922ff3205253edf669c336906ca`;
- complete Debug inventory is exactly 1,943 tests; list
  `/tmp/cri09-closeout-full-list.txt`, SHA-256
  `6bd71c155b3fb3564d501df454652dac18d2de832a6247c407332a424adf8b4b`;
- complete Debug passed 1,943/1,943, zero failed or skipped, in 305.68
  seconds; raw log `/tmp/cri09-closeout-full-debug.log`, SHA-256
  `65eda1ca90c664b475340b5a150a3dd150462a2a688305c9e11ca4971d444c5d`;
- `git diff --check` passed.

## Sanitizer closeout

The dedicated tree is
`/tmp/crexx-integration-ledger.mZ5jyp/candidate-debugasan`. All sanitizer work
used `tools/asan-run.sh`.

The first leak-on build stopped with exit 134 because Apple AddressSanitizer
reported `detect_leaks is not supported on this platform`. That platform
limitation is retained rather than hidden:

- run directory
  `/tmp/crexx-integration-ledger.mZ5jyp/cri09-asan-logs/20260730-142325-build`;
- `build.log` SHA-256
  `267f8e647a48798a2d9f8f79a2adfab3e03126ef73ce217d11303ea3775b4b43`.

The supported macOS ASan path then passed:

```sh
tools/asan-run.sh \
  --build-dir /tmp/crexx-integration-ledger.mZ5jyp/candidate-debugasan \
  --log-root /tmp/crexx-integration-ledger.mZ5jyp/cri09-asan-logs \
  --phase build --build-jobs 6 --build-leaks off --no-live-tail
tools/asan-run.sh \
  --build-dir /tmp/crexx-integration-ledger.mZ5jyp/candidate-debugasan \
  --log-root /tmp/crexx-integration-ledger.mZ5jyp/cri09-asan-logs \
  --phase ctest --regex '^ts_rxjson' --leaks off --test-jobs 10 \
  --no-live-tail
```

- complete ASan-instrumented build passed; `build.log` SHA-256
  `c503c6f40e1e8e0d013318a3949277a985550bba0531a2f11a5a3454d30927ae`;
- focused ASan passed 11/11, zero failed or skipped, with
  `ASAN_OPTIONS=detect_leaks=0`; `ctest.log` SHA-256
  `f10aa33965ae4ec99b69c5e757b1c080d3494b7b36e01a36ef0dbcaad0a0f4b3`.

CRI-09 changes Level B source rather than C/C++ ownership. LeakSanitizer's
macOS unavailability remains a disclosed platform limitation; address
sanitization covers the compiler, generated tools and both VMs executing the
focused contracts.

## Performance verdict

The accepted ordinary Release verdict and raw balanced samples are retained in
[`CRI09-A2-RELEASE-VERDICT.md`](CRI09-A2-RELEASE-VERDICT.md). All ten legacy
cells meet the unchanged 25% guard; A2 improves every cell by 10.41--14.96%
versus V2. Construction plus 30 indexed path gets reduces matched repeated
parsing by 76.25% (`rxvm`) and 79.05% (`rxbvm`); scanner medians are 394.5 and
480.5 microseconds; optimized/non-optimized differences span -2.17% to +2.86%
with no material inversion.

## Compatibility and remaining risk

Existing `rxjson` functions and their string/path results are preserved. The
new surface is pure Level B and adds no native symbol or public binary ABI. No
language syntax, RXAS/RXBIN, serialized index, ownership contract, or packed
numeric representation changed.

The index layout and node identifiers are intentionally private and ephemeral.
Duplicate keys retain first-match compatibility. `jsonscancontainer` returns
the first structurally valid object or array, not the first value matching an
application schema. Packed numeric semantics and the relationship between
parse-once traversal and large typed materialization remain for CRI-13. Final
programme-wide clean install, downstream replay and non-macOS validation remain
at the ledger closeout gate.

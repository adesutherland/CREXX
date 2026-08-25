# Phase 2: re2c NFD Reference Proof

This proof uses the vendored re2c to parse ICU 78.3's Unicode 17.0.0
`gennorm2` rule file and Unicode 17.0.0's normalization test corpus. It is a
reference oracle and requirements probe, not a CREXX product library.

Run from any directory:

```sh
experiments/unicode/poc/run.sh
```

The script:

1. verifies every retained input against `inputs/SHA256SUMS`;
2. builds the repository's vendored re2c as a standalone dependency when
   needed, without configuring the CREXX product or fetching unrelated inputs;
3. regenerates `generated/nfd_reference.cpp` from `nfd_reference.re` and
   requires a byte-for-byte match;
4. compiles the regenerated C++ reference program;
5. runs every applicable NFD relation in `NormalizationTest.txt`, plus the
   required identity check for all scalar values absent from Part 1; and
6. requires the deterministic summary to match
   `evidence/nfd-conformance.txt`.

Optional environment variables:

- `RE2C_BIN`: an existing re2c 4.5.1 executable;
- `CXX`: the C++ compiler (default `c++`); and
- `CREXX_UNICODE_BUILD_DIR`: the untracked build directory (default
  `cmake-build-unicode-poc` at repository root).

The proof implements only NFD: recursive canonical decomposition, algorithmic
Hangul decomposition, and stable canonical combining-class ordering. It does
not implement canonical composition, compatibility decomposition, case
folding, streaming UTF-8 APIs, or a product table format.

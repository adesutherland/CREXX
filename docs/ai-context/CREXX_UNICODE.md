# cREXX Unicode implementation context

Use this note when changing the production `rxunicode` library, its generated
data, or its conformance tests. The general language boundary remains in
`docs/books/crexx_language_reference/unicode.md`; the normalization roadmap and
cache decisions remain in `docs/planning/unicode/PRODUCT-SURFACE-AND-ROADMAP.md`.

## Production source map

- `lib/rxfnsg/rexx/unicode.crexx`: pure Level G public facade and `.casefolder`.
- `lib/rxfnsg/rexx/unicode_casefold.crexx.in`: private Level B executor template.
- `lib/rxfnsg/unicode/tools/casefold_table.crexx`: deterministic prepared-table
  compiler and test oracle.
- `lib/rxfnsg/unicode/tools/generate_casefold.crexx`: injects the prepared image
  into one generated source constant.
- `lib/rxfnsg/unicode/data/unicode-17.0.0/CaseFolding.txt`: pinned UCD source.
- `lib/rxfnsg/tests_functional/ts_unicode_casefold.crexx`: complete four-mode
  record and unlisted-scalar product conformance.
- `lib/rxfnsg/unicode/check_casefold_contract.cmake`: public source, user
  reference and conformance-test surface lock.
- `lib/rxfnsg/unicode/check_casefold_rxas.cmake`: optimized-shape audit.

The generated `unicode_casefold.crexx` is a build-tree artifact. Do not edit or
commit it.

## Language boundary

Level G cannot contain inline assembler. The public `rxunicode` module is
therefore a pure typed facade. It delegates once per whole string to the private
Level B `_rxunicode.casefold(text, mode)` procedure, which owns the VM-adjacent
prepared-table algorithm. The private namespace is not a supported user API.

The four internal mode values are:

| Value | Public operation | Records selected |
| --- | --- | --- |
| `1` | simple default | `C + S` |
| `2` | full default | `C + F` |
| `3` | simple Turkic | `C + S`, with `T` override |
| `4` | full Turkic | `C + F`, with `T` override |

`toCasefold` deliberately means mode 2. That is Unicode Default Case Folding
and matches the useful TUTOR vocabulary. The other names are explicit cREXX
extensions.

## Runtime algorithm

The executor obtains the codepoint count with RXVM `STRLEN`, reads each scalar
with `STRCHAR`, performs one dense descriptor lookup, and emits with
`APPENDCHAR`:

1. Descriptor zero emits the input scalar unchanged.
2. Simple modes read the record's single default scalar; Turkic simple uses its
   `T` scalar when present.
3. Full modes read the default component span; Turkic full substitutes its `T`
   span when present.
4. A missing full span emits the source scalar.

The executor never converts the input to `.binary`, runs a re2c decoder, or
materializes UTF-32. It does not own an image attribute. Every table read names
the one compiler-emitted binary constant directly. The public wrapper call uses
ordinary register transfer; an optimized consumer may inline the wrapper while
retaining one private call per input string.

Case folding does not perform normalization and does not promise source/result
index correspondence. Full mappings may contain up to three scalars in Unicode
17.0.0.

## Prepared image

The portable little-endian image is 4,501,380 bytes:

- 64-byte header (`CUCF`, layout version 1, Unicode 17.0.0);
- 1,114,112-entry dense `u32` descriptor table;
- 1,585 fixed 24-byte records; and
- 1,707 `u32` mapping components.

The audited source contains 1,481 common, 31 simple, 104 full, and 2 Turkic
records; the maximum mapping length is 3. The build verifies the pinned input
SHA-256 before generation. The generator rechecks every scalar, status,
mapping, duplicate, record relationship, count, and final layout. A private
module initializer checks the immutable runtime header and raises `FAILURE` if
the generated data is corrupt or unsupported.

## Build and qualification

The normal `rxfnsg` build compiles the Level B table compiler and generator,
runs the generator with `rxbvm`, compiles and assembles the generated private
module, then links it with the public facade into `rxfnsg.rxbin`.

Focused qualification is:

```text
cmake --build cmake-build-debug --target ts_unicode_casefold --parallel 10
ctest --test-dir cmake-build-debug --parallel 4 --output-on-failure \
  -FS linked_opt_runtime_artifacts \
  -R 'rxunicode_casefold|ts_unicode_casefold'
```

The test checks all 1,585 listed records in all four modes and all 1,110,479
unlisted Unicode scalars for identity under optimized and no-opt builds on both
`rxbvm` and `rxtvm`. The RXAS audit requires one named constant plus
`STRCHAR`, prepared `BGETU32`/`BGETU8` reads and `APPENDCHAR`, and rejects table
attributes and byte conversion/copy instructions.

The generated constant is also a deliberate large-literal product test for
`rxdas`. Ordinary disassembly must emit the complete value once as a private
`.const` alias and use the alias at every direct operand, rather than expanding
the image at every read, overflowing, or truncating its ordinary line buffer.
The generic `rxdas_dump_support` regression locks that boundary independently
of the Unicode image. `rxfnsg_rxdas_inline_preserve_smoke` exercises the real
linked library and caps the textual result so repeated-table expansion cannot
return unnoticed.

Do not add a precheck pass to transforming calls without comparative Release
evidence. Do not merge case folding with normalization or caseless hashing
semantics implicitly; those are separate public contracts.

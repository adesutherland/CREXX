# CRI-14 generic operation-contract closeout

Disposition: **fixed**

Date: 2026-07-30

## Accepted production result

Adrian approved Option B and made the emitted
`crexx.operation-contract/1` document the durable long-term public boundary.
Its metadata acquisition may evolve without changing that consumer contract.
The first implementation therefore separates the contract model, JSON writer
and compatibility checker from a private RXBIN-007 graph adapter.

The installed surface is:

- `crexx-contract`, a build-time CLI that accepts one compiled Level B
  interface operation plus explicit contract version, nullable/optional field
  and structured-error declarations;
- `CREXX::crexx-contract` and `crexx_add_operation_contract()` in the installed
  CREXX CMake package; and
- deterministic UTF-8 JSON containing format/version identity, ordered inputs,
  result, structured errors and a closed, sorted set of interface-record
  schemas.

Format 1 supports strings, booleans, signed 64-bit integers, finite numbers,
exact decimal strings, base64 binary, one-dimensional dynamic arrays, record
interfaces and a void operation result. It fails closed on concrete payload
classes, references, varargs, fixed or multidimensional arrays, object/unknown
types, ambiguous relative identities and non-field interface methods.

The source convention, CLI, installed helper, exact mapping and major/minor
evolution rules are documented in
`docs/books/crexx_programming_guide/operation_contracts.md`.

## Implementation and regression countermeasure

The production implementation is under `contract/`; package integration is in
the root CMake install and `cmake/CrexxOperationContract.cmake`. The model and
writer do not expose graph structures. The private adapter resolves a relative
object spelling against its owning namespace first, then accepts a unique
suffix match, and rejects ambiguity.

Two private graph prerequisites were corrected:

- callable argument names are retained while parsing RXBIN signatures; and
- commas inside array brackets remain part of an array type rather than being
  treated as argument separators. The graph also seeds `.boolean` as a built-in
  scalar.

An attempted global compiler canonicalization of object argument selectors was
rejected during validation. It changed existing selector descriptors and broke
the CMS ADDRESS exit bridge. That change was reverted before the candidate was
measured. Existing selector and ABI spellings remain unchanged; canonical
contract identity belongs only in the exporter adapter. The recovered ADDRESS
target rebuilt, 14/14 relevant interface tests passed, and the complete suite
then passed.

Focused tests cover optimized/non-optimized byte-identical output, the golden
document, Unicode identifiers, every format-1 mapping, optional/null/error
facts, inheritance, strict malformed prior JSON, unchanged/additive/breaking
evolution, and deterministic negatives for every rejected shape. The external
consumer test installs CREXX to a fresh prefix, uses only `find_package(CREXX)`
and installed imported targets, builds a Level B operation, generates the
contract and compares it with the golden.

## Validation

### Focused and sanitizer

- normal focused graph/generation/installed-consumer: 3/3 passed;
- relevant interface regression slice: 14/14 passed;
- Apple ASan focused graph/generation/installed-consumer: 3/3 passed in 8.86
  seconds with the supported `detect_leaks=0` configuration;
- the retained `detect_leaks=1` attempt stops before testing because this Apple
  ASan reports that leak detection is unsupported on the platform; and
- final clean package focus, including the RXPA external SDK consumer: 3/3
  passed in 9.87 seconds.

Sanitizer logs:

- `/private/tmp/crexx-cri14-asan-logs/20260730-202939-build/`;
- `/private/tmp/crexx-cri14-asan-logs/20260730-202944-ctest/`; and
- unsupported leak-detection evidence:
  `/private/tmp/crexx-cri14-asan-logs/20260730-201759-ctest/`.

### Clean full build and suite

The final dedicated build is
`/private/tmp/crexx-gate1a-final-debug.uYte13`, configured with CMake 4.3.2,
Unix Makefiles, AppleClang 21.0.0.21000101 and Debug mode. It identifies the
source as `d78c6fcfa81e` plus the retained dirty worktree and reports
`crexx-1.0.0-beta.3+local.gd78c6fcfa81e.dirty`, built 20260730.

The clean build passed, CTest discovered exactly 1,965 tests, and
`ctest --parallel 30 --output-on-failure` passed 1,965/1,965 with zero failures
or skips in 324.77 seconds. Raw logs are in
`/private/tmp/crexx-gate1a-final-logs.v1A6Ld/`:

| Evidence | SHA-256 |
| --- | --- |
| `configure.log` | `81f05bca1959b2ceaffe61a12b990452432a5780842fafb28340ac228c7ffe96` |
| `build.log` | `f8813db8a0b128f10d5cbfc62a366d9020ee3d058219f1af88e00457db706897` |
| `ctest-list.txt` | `738468b636d656040bb22aa821f9c291e416bec4f3f701a7ad2530749f1263a0` |
| `ctest.log` | `1387243d0ae7bddaf61cbf6fcbda110f746f8bc1e2977bb5e4a4497c2eb5e559` |
| `install.log` | `1e505be08b6984d1504fa78dae297480db3053fcac4ed68edad3fb5f82cefa7d` |
| `focused-package-ctest.log` | `4ca494fb592313df3ed1a0aa1a69690663c506752b5e0cb22512ad0a6f7791e5` |

### Scratch install and installed consumers

The exact clean build was installed into the fresh 141-file prefix
`/private/tmp/crexx-gate1a-final-prefix.c4mpPQ`. No normal user prefix was
used. A separately configured external contract consumer at
`/private/tmp/crexx-gate1a-external-consumer.RhaVO3` found only that package,
built its optimized-independent contract target and matched the golden. The
installed CMake files contain no CREXX source-tree path.

Important artifact hashes are:

| Artifact | SHA-256 |
| --- | --- |
| installed `crexx-contract` | `0f8bc4c37197e7ab54741b440599f45f207c4194d8a7f2df677443ae37aec043` |
| installed `CREXXConfig.cmake` | `08ad335c3b952d504634e15205ac0dc555ea2d67707ddf77773dc05301b5a838` |
| installed contract helper | `294cb459ddd9f481a9822719eaad57df81b2a773aa2b1791b34fb8237cec7d39` |
| external contract JSON | `b1ba5c05ce5c439fb566e3bf1c979ed49b90c4eb599b24aa91425759db8e5bb9` |
| installed `rxpa/crexxpa.h` | `1e5483d094cc059d46fb3f6a5345ae18029db8bbfc8a79a57a14f9a8fc164c44` |
| installed `rxpa/crexx_version.h` | `6a51aec5c2d8fb168d88eb12a41df093a8189f627e058f2bd8e74388e8d07e77` |
| installed RXPA helper | `c00716429bdb07b6b4017fa5c9ba16f8447a713f5cd509de0a1c51c47d8a62d8` |

The complete contract manifest is
`/private/tmp/crexx-gate1a-final-logs.v1A6Ld/scratch-artifact-manifest.sha256`.
The clean RXPA consumer manifest is
`/private/tmp/crexx-gate1a-final-debug.uYte13/tests/rxpa/rxpa_external_sdk_consumer_ws/artifact-manifest.txt`.
That test also proves explicit compiler import/runtime plugin paths, optimized
and non-optimized importers, both VMs, C/C++ `-Werror` consumers, missing-path
failure and version-mismatch rejection.

## Read-only downstream replay

The read-only `crexx-rag` source configured and built out of tree against the
scratch prefix with the vendored-header and CREXX-source fallbacks both `OFF`.
All compiler, assembler, VM and header cache entries resolve inside the scratch
prefix. The build completed 25/25 targets.

The full deterministic/loopback-only CTest discovered 26 tests and passed
23/26 with zero skips in 45.37 seconds. Its three failures are retained
downstream migration seams rather than CREXX regressions:

1. `phase0_component_benchmark` still names the retired `rx_socket` runtime
   module. Replaying the exact compiled image after removing only that module
   argument passed `rxvme`, `rxbvm` and the loopback provider with matching
   component/checksum output.
2. `p1a_json_document` tests the superseded incubator `jsondocument`, envelope
   vector classes and `as_f32_vector`/`as_i64_vector` methods while also
   importing the now-production `rxjson.jsondocument`. It must be retired in
   favor of the approved production parse-once and raw packed-binary surface.
3. `p1a_provider_boundary` imports the same competing incubator class and calls
   `as_f32_vector`; it must migrate to `node_f32_array` and the downstream
   schema-owned type/count contract.

The exact retained Level G imported-record, `do forever` and Level G PARSE
reproducers compile and assemble in optimized and non-optimized modes. Runtime
relevant sources execute on `rxvm` and `rxbvm`. The exact downstream binary
optimizer probe passes all four mode/VM cells with checksum 47,201,280; its
optimized by-value path is faster than its non-optimized path, so the historical
optimizer inversion is absent.

Raw downstream logs and hashes:

| Evidence | Result | SHA-256 |
| --- | --- | --- |
| `crexx-rag-configure.log` | fallbacks off; exact scratch tools | `5a614701044be81b9e854c70a08f658e8badb41242caefd02b0708fbf1946855` |
| `crexx-rag-build.log` | 25/25 targets | `26803d6292ab1a4787c0eaa2da83bb32fbcdaffd731942e121a9feb4ea96b8ac` |
| `crexx-rag-ctest-list.txt` | 26 tests | `0215895f72bd591e8cd125b279d31fe4ee996d90c5f259a33bf8c5a0fbfaa1c5` |
| `crexx-rag-ctest.log` | 23 passed, 3 migration seams | `147a2460fb928fdf6d18130084433f7ed5ff39923dc13be819acf556f1825348` |
| `crexx-rag-reproducer-replay.log` | all concrete reproducers pass | `bad674c09cab29d66a34a490231455520f28dfff6c3ba937fa5245832c090532` |
| `crexx-rag-binary-probe-replay.log` | four cells/checksum pass | `0bce30e882953c83ef43f0ebaecc5969a75b475f584ca1e61ceffefa93de4065` |

No hosted call, credential, normal-prefix install or write to the downstream
checkout was used.

## Compatibility and remaining risk

The CLI, CMake helper and JSON document are additive public development
surfaces. The format-1 JSON contract is the only new serialized format; it was
explicitly approved. No language syntax, runtime reflection, public C ABI,
runtime allocation path, selector spelling, RXAS encoding, RXBIN encoding or
RXBIN-007 schema changed. The graph signature changes are private and preserve
existing callable descriptors.

Format 1 is intentionally narrow. It does not yet describe nullable operation
inputs/results, maps/unions, fixed arrays, transport envelopes or runtime error
dispatch. Adding those is a future format/API decision, not an implicit adapter
extension. Cross-platform CI remains required before release publication; this
host proves Apple arm64 and both VM engines. The downstream incubator JSON and
legacy socket-module accommodations must be removed in `crexx-rag` before its
own suite becomes 26/26.

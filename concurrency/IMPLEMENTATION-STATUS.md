# cREXX concurrency implementation status

Status date: 2026-08-16
Source baseline: `183a730dc` plus the documentation-only concurrency control
plane at `794b0112f`

This is the source-to-documentation truth matrix for the concurrency surface on
`develop`. It prevents a declaration, historical plan or passing test from
being mistaken for a published capability.

## How to read this matrix

- **Implemented and directly tested** means production source contains the
  behavior and an executable test reaches the stated public contract.
- **Implemented; indirect coverage** means source contains the behavior and
  adjacent paths exercise its components, but no focused public-contract
  assertion was found in this review.
- **Declared but unsupported** means a public declaration deliberately signals
  unsupported status `19` when called.
- **Reserved** means a name, type code or architectural position is retained
  for future compatibility; users cannot rely on an implementation.
- **Qualified on Mac** describes the recorded local closeout. It does not imply
  Linux/Windows public conformance, release publication or stability.

The current code and executable tests remain more authoritative than this
snapshot. When either changes, update this matrix in the same change.

## Language and compiler surface

| Surface | Current status | Evidence and boundary |
| --- | --- | --- |
| `task` procedures | Implemented and directly tested; Level G only | Parser/lowering and runtime coverage is in `compiler/tests/rexx_src/gate_f_levelg_conformance.crexx`, executed through `rxc`, `rxas`, `rxlink`, `rxbvm` and, where built, `rxtvm` by `compiler/tests/CMakeLists.txt`. Level B rejection is tested by `gate_f_task_levelb`. |
| Ordinary calls to task procedures | Implemented and directly tested | A task call has ordinary Rexx expression syntax; the compiler submits independent sibling task calls and joins where their values are consumed. Operator meaning and left-to-right expression semantics are not changed. |
| `DO PARALLEL` and `DO PARALLEL expr` | Implemented and directly tested; Level G only | Positive conformance covers both forms. `gate_f_parallel_levelb` and `gate_f_parallel_expr_levelb` prove that the syntax is rejected below Level G. Ordinary clauses in the block still execute on the controller; only task calls/submissions create child work. |
| Explicit `task target` expressions | Implemented and directly tested; Level G only | Positive lowering is covered by Level G and import tests. Dynamic targets and Level B usage have negative tests (`gate_f_dynamic_task_target`, `gate_f_task_target_levelb`, `gate_f_task_factory_target_levelb`). |
| Transferable task methods | Implemented and directly tested | `gate_f_task_method_import_*` compiles separate modules, assembles, links, runs both optimization modes and reconstructs exact `to_channel`/`from_channel` arguments and results. Receiver rules are statically enforced. |
| `.taskwork` factory targets | Implemented and directly tested | `gate_f_taskwork_import_*` proves sealed metadata across module boundaries and both VM families. Negative tests require the `.taskwork` adapter and transferable factory arguments. |
| Structured scope safety | Implemented and directly tested | Negative compiler tests cover escaping pending results, mutation while pending, reference/exposed arguments, nested waits, reused scopes and invalid result types. Direct self-recursion is lowered as an ordinary synchronous procedure call; blocking waits across distinct task callables remain rejected. |
| Enduring documentation examples | Implemented and directly tested | Four sources under `docs/books/crexx_programming_guide/examples/` cover simple calls, explicit local/process pools, transferable objects and `.taskwork`; CMake compiles, assembles, links and runs them on both VMs in both optimization modes. |

Historical `gate_f_*` identifiers above are executable test names, not public
feature names.

## Level B class surface

The declarations and implementation are in `lib/classlib/Concurrency.crexx`.
The compiler contract test assembles and disassembles the public names; the
functional test exercises the value, channel, endpoint, pool and scope
behavior.

| Class or interface | Current status | Notes |
| --- | --- | --- |
| `.taskpool` | Implemented and directly tested, except telemetry | `local()` and `process()` create bounded providers; capacity, admission capacity, provider type and lifecycle are exercised. `queued()` and `running()` are declared but deliberately unsupported. |
| `.taskscope` | Implemented and directly tested, except services | Default/fail-fast/collect-all creation, submit, observation, join, cancellation, finish/abort and typed result bridges back structured lifetime. `ask()` is declared but deliberately unsupported. |
| `.task`, `.completion` | Implemented and directly tested | Request identity, completion state/value/error and request-specific wait are exercised by class and Level G functional paths. |
| `.tasktarget` | Implemented and directly tested through compiler lowering | Public-looking `binding()` is compiler-reserved: applications must use `task target` syntax, not author an 80-byte binding. Linker-sealed validation and its cache are tested in compiler/interpreter suites. |
| `.taskarguments` | Implemented compiler bridge | This exposed concrete helper carries compiler-lowered argument documents into `submit_arguments()`. It is not the preferred ordinary user API; typed task calls and `.taskwork` targets are. |
| `.taskwork` | Implemented and directly tested | Advanced users implement `run(request, context)`; task factory targets supply construction arguments separately from the request. |
| `.taskcontext` | Timeout/cancellation/trace access implemented and used; `endpoint()` has indirect coverage only | HTTP taskwork implementations receive contexts. `endpoint(reference)` delegates to the proven `.byteendpoint.from_reference()` adapter, but this review found no test that directly calls the context method. Keep it provisional until a conformance assertion is added. |
| `.channel`, `.channelrequest` | Implemented and directly tested | Bounded start/wait/cancel/close behavior, nonblocking observation, request-specific wait, invalid targets and close rules are exercised. |
| `.channelvalue` | Implemented and directly tested | Canonical RXCV null, boolean, integer, float, decimal, string, binary, array, record and provider-reference documents are validated, including canonical record ordering and depth limits. |
| `.channelcodec` | Interface contract implemented; no general codec registry | The exact encode/decode interface exists. Compiler-supported transferable types use their statically resolved `to_channel`/`from_channel` contract; there is no advertised dynamic registry. |
| `.byteendpoint` | Implemented and directly tested | Memory/null endpoints, provider references, encoded-reference adapters, bounded reads/writes, half-close, cancellation and close use the common channel instructions. |
| `.transferbuffer` | Implemented and directly tested | Allocate/copy/write/read, move invalidation, immutable snapshot sealing and RXCV conversion are covered. Sealing prevents later buffer writes; it is not encryption, authentication or a checksum. |
| `.serviceref` | Interface reserved; no concrete public service | The identity shape is declared for future single-owner services. No service implementation or usable `.taskscope.ask()` path exists. |

## RXAS, RXBIN and VM surface

| Surface | Current status | Evidence and boundary |
| --- | --- | --- |
| `chanopen`, `chanstart`, `chanwait`, `chancancel`, `chanclose` | Implemented and directly tested | RXAS syntax/effects are documented in `docs/reference/rxas/instructions/09-io-sockets-processes-and-time.md`. Compiler/assembler malformed-image and both-VM execution tests preserve the common instruction shape. |
| RXBIN channel feature and opcodes | Implemented and directly tested | RXBIN version 007 uses feature bit `1 << 3`; opcodes `650..654` are decoded by both interpreter variants. Compatibility checks reject unsupported images. |
| Provider registry | Implemented as an internal VM seam | `interpreter/tests/test_rxvmchannel_registry.c` proves atomic registration, duplicate rejection, channel pinning and safe unload. It is not a supported public plugin ABI. |
| Provider type `1`: local task pool | Implemented and Mac-qualified | Functional tests execute local task work on `rxbvm` and `rxtvm`; bounded admission, cancellation and teardown are covered. Portable public qualification remains CONC-11. |
| Provider type `2`: process task pool | Implemented and Mac-qualified | Process execution and crash replacement are covered by `interpreter/tests/test_rxvmchannel_process.c` plus linked class/compiler tests. Portable public qualification remains CONC-11. |
| Provider type `3`: open host | Reserved | No interoperable protocol or public provider exists. |
| Provider type `4`: byte endpoint | Implemented and directly tested | `interpreter/tests/test_rxvmchannel_byte.c` and class tests cover bounded endpoint behavior and reference capabilities. |
| Provider type `5`: child process | Implemented and directly tested locally | Child-process I/O redirects are built from provider references and endpoints rather than a separate redirect opcode family. Cross-platform publication still depends on CONC-11. |
| Sealed task bindings and validation cache | Implemented and directly tested | The 80-byte RXTB binding identifies the resolved callable and detects a stale or mismatched linked shape before dispatch. Cache tests preserve one full validation per image/target and fast subsequent submissions. It is integrity metadata, not encryption. |

## Concurrent HTTP surface

| Surface | Current status | Evidence and boundary |
| --- | --- | --- |
| `.rxfnsg..httpclient` pooling and admission | Implemented and directly tested; experimental | `lib/rxfnsg/tests_functional/ts_http_pooled.crexx` exercises bounded concurrent requests and connection ownership through both VMs and optimization modes. |
| Safe headers and policy | Implemented and directly tested | `ts_http_policy.crexx` covers header validation, redirects, retries, idempotency and ambiguous outcomes. |
| Fixed/chunked request bodies and streamed responses | Implemented and directly tested | `ts_http_streaming.crexx` covers bounded endpoints, fixed and chunked framing, response streaming and trailers. |
| gzip, zlib and raw DEFLATE response decoding | Implemented and directly tested | `ts_http_codec.crexx` exercises bounded decode behavior and malformed/limit paths. |
| Verified TLS | Implemented and Mac-qualified | `ts_http_tls_live.crexx` provides live verified-TLS coverage where enabled. Portable backend/package qualification remains CONC-11. |
| `crexx-rag` generation/embedding shapes | Implemented and directly tested locally | `ts_http_crexx_rag.crexx` validates authentication, idempotency, request fields and response shapes using a loopback fixture. |
| Synchronous `.rxhttp..httpclient` | Existing independent implementation | `lib/rxfnsb/rexx/rxhttp.crexx` remains the Level B synchronous client and is used by current LLM providers. It is not a wrapper around the concurrent client. Architecture rationalisation is CONC-16. |

## Publication boundary

The matrix proves an implemented experimental surface on `develop` and records
local Mac qualification. It does not establish that concurrency is present in
an existing release tag, portable across every supported platform, or stable.
Those claims require the CONC-11 matrix, install/package proof and an explicit
release decision.

## Reconciliation gaps to close

1. Add a direct conformance assertion for `.taskcontext.endpoint()` before
   documenting it as supported.
2. Add a direct unsupported-contract assertion for `.taskscope.ask()`; source
   already signals status `19`.
3. Complete Linux and Windows qualification before public portability claims.

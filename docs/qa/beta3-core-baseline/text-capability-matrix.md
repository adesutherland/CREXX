# Beta 3 external-text boundary audit (in progress)

28 September 2026. This is the AC-04/AC-11 capability register for the local
`temp/beta3-core-baseline` candidate, not a native-package qualification. The
current code and host mocks are still uncommitted. The exact native raw backend
and CMS31/TSO31/TSO64 package checks remain separate open dependencies.

## Encoding and physical storage

| Selection | Shared cREXX mapping | Default native text storage | Host evidence |
| --- | --- | --- | --- |
| UTF8 | Strict validated UTF-8 identity | Byte stream with explicit LF | `text_codec`, `native_raw_cms/tso` |
| ASCII | US-ASCII, bytes above 127 invalid | Byte stream | `text_codec` full byte range |
| Latin1 | ISO-8859-1 | Byte stream | `text_codec` full byte range |
| Windows-1252 | Retained map, undefined CP1252 positions remain C1 controls | Byte stream | `text_codec` full byte range and Euro/C1 controls |
| IBM437 | Retained CP437 map | Byte stream | `text_codec` full byte range |
| IBM850 | Retained CP850 map | Byte stream | `text_codec` full byte range |
| IBM1047 | Retained IBM1047 map | Native logical records | `text_codec` full byte range; raw host record mock |

Codec and physical storage are distinct stream properties. The selected codec
is captured at open; `platform_fopen_storage` can explicitly request byte or
record text independently of page. Binary C modes always request raw byte mode
and never invoke a codec. Native console and names remain IBM1047 regardless of
file selection. The defaults preserve the existing native IBM1047-record and
UTF-8 exchange-byte behavior; the other listed exchange pages default to byte
streams. Mainframe backend format recognition and package compatibility are
still unverified. There is no UTF16/32 tool-stream selector.

## Supported crossing inventory

| Component/boundary | cREXX route and status | Remaining proof or gap |
| --- | --- | --- |
| Shared codec | `platform/text_codec.c` owns strict UTF8 and seven selected pages; generated tables derive from retained `rxunicode` data. | Linux and actual native combined checks open. |
| Compiler source, imports, RXPP-fed source | `rxc` textual opens use `openfile`; the raw platform adapter decodes and inserts logical LF for native records. Binary RXBIN imports stay `rb`. Host mocks now exercise `file2buf` on a nonseekable raw record stream and injected read error. | Native source/import package proof remains open. RXPP's own stream crossings need a separate supported-surface audit. |
| Assembler input/output | `rxas` uses `openfile` text input and binary RXBIN output. `-E` selects the file page. | Raw native end-to-end assembly with non-ASCII source/output open. |
| Linker/disassembler | RXBIN loads/writes use binary modes. Linker control now uses `platform_fopen`; disassembler output and linker map/report use textual `openfile`. Both tools accept the existing `-E` file-page selector. | Non-ASCII control/report/disassembly and failure checks open. |
| VM file API | Native `rxvm_private_fopen` routes via `platform_fopen`; `.binary` remains byte exact in the raw adapter. The CLI accepts `-E` for application text files. | Application text APIs, mixed page/layout lifetime and actual runtime backend require combined proof. |
| Compiler/VM/tool stdio and diagnostics | OOM panic has bounded cREXX UTF8-to-IBM1047 conversion and raw `stderr`. An owned `crexx_native_standard(0/1/2)` wrapper now decodes/encodes IBM1047 console records independently of the selected file page; host mocks cover input/output and release. | Ordinary product `stdin`, `stdout`, `stderr`, SAY, TRACE and diagnostic callers still use C stdio or existing runtime routes; attach the owned wrapper at the supported process/VM boundaries and qualify them. |
| Arguments and environment | `crexx_native_arguments` and `crexx_native_environment` decode the raw IBM1047 process services to owned UTF-8, retry exact ERANGE lengths, distinguish absent/empty/unsupported values and reject embedded NUL. | Startup/tool/VM callers are not yet routed; old already-decoded compatibility inputs must not be decoded twice. |
| Native names and discovery | TSO `root.TYPE(NAME)` mapping, CMS `NAME TYPE MODE` mapping, IBM1047 name encoding and raw directory adapters are in cREXX platform code. CMS `fileexists` probes the same raw byte route. Compiler preserves root/provider policy and propagates iterator errors. | CMS 18-byte FID iterator shape is provisional pending lab backend. Native close/error and package evidence open. |
| Driver, RXPP, crexxsaa, profiler and ancillary tools | Audit identified direct `fopen`, `getenv` and stdio sites; no blanket parity claim is made. | Identify supported native build sets and route every maintained text crossing or record explicit unavailability. |
| Binary RXBIN, packages and explicit binary files | `rb`/`wb` raw mock passes embedded zero, high bytes and LF without conversion. | Native package and actual binary storage proof open. |

The raw host fixture compiles the CMS and TSO `platform.c` routes and exercises
record/byte text, short writes, capacity rejection before record commit,
read/write/flush/close errors, binary bytes, names, directory EOF/errors,
panic output, console wrappers and argument/environment decoding. Latest Debug
result after the CMS raw existence-probe correction:
`/tmp/beta3-fileexists-build.log` and `/tmp/beta3-fileexists-test.log` (7/7
selected crossings). Latest Apple ASan host raw result:
`cmake-build-debugasan/asan-logs/20260928-122744-build` and
`20260928-122754-ctest` 2/2. The constrained single-threaded host build of
`rxbvm_single`, `test_single_state` and `test_single_embed` passed at
`/tmp/beta3-single-config.log`, `/tmp/beta3-single-build.log`; its focused
`single_vm_state` passed at `/tmp/beta3-single-test.log`.
The full local Debug build passed at `/tmp/beta3-platform-all-build2.log` and
nine focused product-crossing tests passed at
`/tmp/beta3-platform-crossing-test.log`; these remain host correctness checks,
not the frozen combined suite or native package gates.
The added nonseekable/failed-read checks passed the two raw host tests in Debug
at `/tmp/beta3-sequential-test2.log` and Apple ASan at
`cmake-build-debugasan/asan-logs/20260928-123014-ctest`.
The shared `-E` CLI selector is now accepted by `rxc`, `rxas`, `rxlink`, `rxdas`
and `rxbvm`. A measured serialized regression accepts UTF8 and rejects an
unsupported page in each tool: Debug `/tmp/beta3-selector-final-ctest.log`,
Apple ASan `cmake-build-debugasan/asan-logs/20260928-123436-ctest`. This proves
parsing and selection on desktop, not non-ASCII native tool I/O.
These mocks do not establish an implemented or qualified lab backend. Apple
LeakSanitizer is unavailable; the approved final Linux ASan/LSan plus macOS
ASan matrix remains open.

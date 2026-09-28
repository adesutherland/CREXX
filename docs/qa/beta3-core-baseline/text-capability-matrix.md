# Beta 3 external-text boundary audit (in progress)

28 September 2026. This is the AC-04/AC-11 capability register for the
feature-gated cREXX implementation merged through PR #709 at published
`9f2f44cfd888d324858769809b0381e524850b4c`, not a native-package
qualification. The exact native raw backend and CMS31/TSO31/TSO64 package
checks remain separate open dependencies. A later local documentation-only
flush proposal does not alter the qualified product/test inputs.

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
| Shared codec | `platform/text_codec.c` owns strict UTF8 and seven selected pages; generated tables derive from retained `rxunicode` data. | Linux ASan/LSan codec and raw CMS/TSO host fixtures pass in run `36448935081` on `9f2f44cfd`; actual native combined checks remain open. |
| Compiler source, imports, RXPP-fed source | `rxc` textual opens use `openfile`; the raw platform adapter decodes and inserts logical LF for native records. Binary RXBIN imports stay `rb`. Host mocks now exercise `file2buf` on a nonseekable raw record stream and injected read error. | Native source/import package proof remains open. RXPP's own stream crossings need a separate supported-surface audit. |
| Assembler input/output | `rxas` uses `openfile` text input and binary RXBIN output. `-E` selects the file page. | Raw native end-to-end assembly with non-ASCII source/output open. |
| Linker/disassembler | RXBIN loads/writes use binary modes. Linker control now uses `platform_fopen`; disassembler output and linker map/report use textual `openfile`. Both tools accept the existing `-E` file-page selector. | Non-ASCII control/report/disassembly and failure checks open. |
| VM file API | Native `rxvm_private_fopen` routes via `platform_fopen`; `.binary` remains byte exact in the raw adapter. The CLI accepts `-E` for application text files. | Application text APIs, mixed page/layout lifetime and actual runtime backend require combined proof. |
| Compiler/VM/tool stdio and diagnostics | OOM panic has bounded cREXX UTF8-to-IBM1047 conversion and raw `stderr`. An owned `crexx_native_standard(0/1/2)` wrapper now decodes/encodes IBM1047 console records independently of the selected file page; host mocks cover input/output and release. | Ordinary product `stdin`, `stdout`, `stderr`, SAY, TRACE and diagnostic callers still use C stdio or existing runtime routes. Startup bind/restore and explicit flush delivery remain open: the console probe demonstrates an invisible partial prompt and deferred raw error at `fflush`; the platform-only and generic per-stream proposals are in `native-raw-services.md`. |
| Arguments and environment | `crexx_native_arguments` and `crexx_native_environment` decode the raw IBM1047 process services to owned UTF-8, retry exact ERANGE lengths, distinguish absent/empty/unsupported values and reject embedded NUL. VM `GETENV` now uses the native adapter and signals `NOTREADY` on backend failure; optimized/unoptimized desktop VM cases remain passing. | Standalone startup arguments and direct tool/configuration `getenv` callers are not yet routed; old already-decoded compatibility inputs must not be decoded twice. |
| Native names and discovery | TSO `root.TYPE(NAME)` mapping, CMS `NAME TYPE MODE` mapping, IBM1047 name encoding and raw directory adapters are in cREXX platform code. CMS `fileexists` probes the same raw byte route. Compiler preserves root/provider policy and propagates iterator errors. | CMS 18-byte FID iterator shape is provisional pending lab backend. Native close/error and package evidence open. |
| Driver, RXPP, crexxsaa, profiler and ancillary tools | Current lab package evidence lists RXC/RXAS/RXVM; it does not establish native RXPP, RXLINK, RXDAS, `crexxsaa`, profiler or ancillary packages. Desktop RXPP's `precomp.readallx`/`writeall` use direct `fopen` and the helper is both a static RXPP provider and a dynamic compiler import. | No native parity claim for the unbuilt components. Before any native RXPP packaging, route its text opens through the selected cREXX platform without duplicating codec state in the dynamic module; verify read/write/close failures and its generated text. Audit driver and embedding stream ownership separately. |
| Binary RXBIN, packages and explicit binary files | `rb`/`wb` raw mock passes embedded zero, high bytes and LF without conversion. | Native package and actual binary storage proof open. |

### Caller-level native audit

The present lab package pipeline names RXC, RXAS and RXVM. This table records
their remaining crossings even when a host mock has already proved the adapter.
Other desktop components remain in the supported-surface audit above, but have
no current CMS31/TSO31/TSO64 package evidence.

| Caller | Current boundary | Native disposition still needed |
| --- | --- | --- |
| `compiler/rxc_main.c` → `rxcmain`, `compiler/rxcp_ast_print.c`, `compiler/rxcp_exit.c` | `argv`, C `stdout`/`stderr`, compiler-exit SAY and diagnostics | Decode raw process arguments once at standalone startup; attach IBM1047 console wrappers before first output. Keep direct embedding of `rxcmain` host-owned. |
| `assembler/rxasmain.c` | `argv`, C `stdout`/`stderr`; source text through `openfile` | Same process input and console startup contract; `-E` already selects source text page. |
| `interpreter/rxvmmain.c`, `interpreter/exitfunc.c`, `interpreter/rxvmintp.c` | `argv`, C `stdin`/`stdout`/`stderr`, SAY/TRACE and diagnostics | Same standalone startup contract; `rxvml` embedding must retain host streams. VM application files already pass through the native adapter. |
| `interpreter/rxenv.c` VM `GETENV` | cREXX native environment decode on CMS/TSO; desktop native `getenv` otherwise | Host mock plus normal/ASan VM regressions pass; native service/package proof open. |
| Compiler configuration (`rxcp_project_dependencies.c`, `rxcp_exit.c`, `rxcpmain.c`, `rxcp_diag.c`) and VM configuration (`rxvmmain.c`) | Direct `getenv` calls for path, exit and diagnostic options | Route supported native options through the raw environment decoder with owned UTF-8 lifetime, or declare each option unavailable on the native profile. Do not treat ENOTSUP as absence. |
| `preprocessor/precomp.c` in RXPP | Direct `fopen`/`fgets`/`fputs`/`fclose`; dynamic compiler import and static RXPP provider | Native RXPP is not yet packaged. A future native build must use the same platform-selected codec in both provider shapes and propagate read/write/close errors. |
| `linker/rxlinkmain.c`, `disassembler/rxdamain.c`, `interpreter/rxseqmain.c` | Text files now use `platform_fopen`/`openfile` where applicable; C stdio and `argv` remain | No current native package evidence. Before enabling any of these native tools, apply the standalone startup contract and qualify their explicit file-page selection. |
| `interpreter/crexxsaa.c`, `interpreter/rxvmlib.c`, `bin/crexx.crexx`, `preprocessor/rxpp_sh.c` | Host embedding, driver and editor surfaces; direct C/environment or VM file services | No current native package evidence. Embedding must not rebind a host's C standard streams; native availability and each text crossing need explicit packaging and tests before parity claims. |

The current lab VM profile uses static providers and has no published raw
environment backend yet. `CREXX_PROVIDER_PATH` is optional in that profile;
making its lookup a mandatory startup step would reject otherwise usable
native VMs on `ENOTSUP`. The raw `GETENV` instruction is already checked and
fails explicitly when called. Configuration lookups need a per-option native
capability decision and backend proof before routing them; `ENOTSUP` must
remain distinguishable from an absent variable.

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
Native VM `GETENV` is now routed through the platform decoder at `2df36e28e`.
CMS/TSO mock and optimized/unoptimized VM getenv tests pass 4/4 in Debug at
`/tmp/beta3-getenv-final-build.log` and `/tmp/beta3-getenv-final-test.log`,
and 4/4 under Apple ASan at
`cmake-build-debugasan/asan-logs/20260928-124747-build` and
`20260928-124751-ctest` (generated test fixture built at `20260928-124517-build`).
The shared `-E` CLI selector is now accepted by `rxc`, `rxas`, `rxlink`, `rxdas`
and `rxbvm`. A measured serialized regression accepts UTF8 and rejects an
unsupported page in each tool: Debug `/tmp/beta3-selector-final-ctest.log`,
Apple ASan `cmake-build-debugasan/asan-logs/20260928-123436-ctest`. This proves
parsing and selection on desktop, not non-ASCII native tool I/O.
These mocks do not establish an implemented or qualified lab backend. Apple
LeakSanitizer is unavailable. The approved hosted matrix now passes
2,376/2,376 on both Linux ASan/LSan and macOS ASan at `9f2f44cfd`;
the [retained receipt](hosted-sanitizer-2026-09-28.md) records exact logs,
settings and hashes. Those host results do not close the native gaps above.

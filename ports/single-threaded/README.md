# Minimal single-thread RXVM experiment

This opt-in build supports the Mainframe Lab application-port experiment. It
uses the actual switch interpreter and RXBIN 007 loader, with unchanged 64-bit
Rexx integers, values, procedure calls and synchronous language signals.
The normal top-level desktop build is unchanged.

This directory owns the maintained CMS/TSO build and package recipes. Shared
services and target selection live in `platform`; select `CREXX_CMS_ELF` or
`CREXX_TSO_ELF` (the documented Mainframe Lab profile aliases remain accepted).
The obsolete root S370 Docker/GCCLIB recipe and its `__CMS__` compatibility
hooks are retired. That route is not an alternative build of the current port.
The maintained recipe reads headers from their canonical source owners,
including `third_party/avl_tree`, `third_party/utf8` and `interpreter/rxvml`;
these roots also participate in its input-freshness checks.

The intended application C baseline is C99. The primary CMS route is the
maintained GCC 16.2 cross-toolchain hosted on macOS; its retained target recipes
use GNU99. A host build requesting C99 does not establish complete strict-C99
or C90 qualification. Clean, justified legacy GCCCMS/GCCMVS compatibility is
welcome. Reproduce limitations against the exact compiler/runtime and assess
the appropriate repair; do not infer new cREXX restrictions from a legacy
failure. Uncertain trade-offs or design changes require project-owner approval.

`NTHREADED` selects switch dispatch only. It does **not** disable OS workers in
the ordinary `rxbvm`. This leaf owns the coherent, private set of
`CREXX_VM_SINGLE_THREADED`, static-only and unavailable-service definitions;
these are not independently qualified product feature switches. Keep their
values consistent across all linked VM and embedding translation units.

The process must have one execution thread and no asynchronous callbacks into
RXVM. Nested execution and multiple contexts on that thread retain their own
memory owners and restore the active context on return. Internal `rxvm_worker`
objects remain execution-owner bookkeeping; no executor or OS worker is built.
This configuration is not safe to embed in a multithreaded caller.

The build excludes worker executors, all channel providers, process launch and
redirection, sockets, dynamic RXPA/VM plugin loading, and the OS interrupt bridge.
Channel, socket and clock instructions raise existing `NOT_IMPLEMENTED` (12),
which a Rexx signal handler can catch. Automatic random seeding also requires an
unavailable clock; explicit seeding remains available. Dynamic loading and the
private process-worker command entry fail with diagnostics. Ordinary VM signals
do not require an OS handler. No green-thread scheduler is provided.

Static decimal support remains selected. This is not a reduction of serialized
types or integer widths. Additional services can be restored explicitly after
their host implementation is qualified. There is no fake pthread/atomic API.

The shared checked-invocation owner is part of the core in every profile. The
port has no native command-resource owner: its optional execution-directory
snapshot returns failure without consulting the process directory or filesystem.
That fallback occupies a separate static-archive member so an embedding's
explicit resource-service definition can resolve the symbol instead.

The private synchronization operations are empty only in this selected build;
the desktop implementations are retained. Compatibility callbacks keep nesting
and lifecycle bookkeeping without a transition that waits on another thread.
The compact build also removes forced helper inlining and flattening, allowing
a size-optimizing compiler to share code across handlers. It keeps the existing
profile-20 handler placement; no opcode or computational type is removed by
this size policy.

The portable slab allocator uses ISO C allocation with explicit alignment and
retains the original pointer for freeing. Its extra alignment reservation must
be included in working-memory measurements. A 32-bit-pointer layout preserves
the existing 64-byte slab header; this does not alter the RXBIN format.
This build selects 4 KiB slabs and a 2 KiB maximum pooled allocation; larger
byte blocks and value arrays use the existing individually allocated extent
path. Desktop defaults remain 64 KiB and 16 KiB. Small pools avoid reserving
up to twice 64 KiB merely to serve a new small allocation class on CMS.
Configure these with `-DCREXX_VM_SLAB_SIZE=4096` and
`-DCREXX_VM_MAX_POOLED_SIZE=2048`. Compile-time checks require power-of-two
sizes, a pooled limit from 16 through 16384 bytes, room for the slab header
and reference cells, and a slot count that fits its header. These are build
settings; slab alignment is part of pointer-to-owner lookup and cannot change
while a VM is running. Builds outside this CMake leaf can set the underlying
`RXVM_MEMORY_SLAB_SIZE` and `RXVM_MEMORY_MAX_STANDARD_SIZE` macros consistently
for all VM translation units.

Build on the Mac after building normal RXAS and `rxbvm`:

```sh
cmake -S ports/single-threaded -B build-single -G Ninja \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCREXX_GENERATED_DIR="$PWD/build/generated" \
  -DCREXX_RXAS="$PWD/build/bin/rxas" \
  -DCREXX_REFERENCE_VM="$PWD/build/bin/rxbvm"
cmake --build build-single
ctest --test-dir build-single --output-on-failure
```

The reference comparisons exercise optimized and unoptimized argument aliasing,
copying and nested signal fixtures. Separate controls cover unavailable services,
worker entry, dynamic loading and execution-owner/memory lifetimes. Native
success is not proof of CMS compilation, linking, memory capacity or execution.

## Embedded VM and RXC

The opt-in mainframe ADDRESS provider is documented in
[`mainframe_address`](../../lib/plugins/mainframe_address/README.md). Native
RXC/RXAS/RXLINK/RXVM builds use `mainframe-build.crexx` with the selected
verified SDK overlay, root generated source manifests and this leaf's VM
manifest. Native RXC links declaration metadata separately from the runtime
provider; combining `CREXX_MAINFRAME_ADDRESS` and `CREXX_BUILD_COMPILER` in
the leaf is rejected in favor of that explicit recipe. The recipe checks and
hashes actual target objects and detects source changes during compilation.
Native guest qualification remains distinct from these host/export checks.

The TSO31 batch consumer uses `mainframe-batch-pack.crexx` and the normal
`build/mainframe-batch` location in this source checkout. It rebuilds the four
tools in place from a version-8 DD-backed SDK, preserving the SDK and host
build inputs. Each tool checks the batch entry independently; the pack checks
that its four host libraries did not change during the build. Only directories
marked as generated by these recipes can be cleaned. The Mainframe Operator
owns z/OS and PDOS configuration and execution; paired guest qualification
and released packages remain separate gates.

A missing or unreadable input module is a CLI invocation failure with RC 2.
It previously called `exit(-1)`, observed as host status 255 and native z/OS
condition code 4095. The CLI input-load branch now uses the same positive
status as its other invocation errors; application return codes are unchanged.
Focused bytecode/threaded CLI controls check absent and malformed inputs plus
the successful version interface. The repaired shared TSO31 package passed
native z/OS 1.5, then PDOS line/model2 qualification on 14 October, including
exact generated bytecode, native ADDRESS, compiler exits and failure controls.
This is the selected PD-033 Stage 3 development result; released packages and
other profiles remain separate.

Native RXVM also selects `CREXX_VM_CLASSIC_PROVIDERS`: the existing stream
provider uses explicit registration and sequential native record FILEs, and a
private `rxfs` subset supplies the three signatures required by the current
Classic/G closure. Positional streams, native absolute-path resolution and
executable-path discovery are not qualified; unavailable pathname calls signal
FAILURE. The desktop filesystem provider remains separate. Link the matching
`classlib.rxbin` alongside library/rxfnsc/rxfnsg so deployment does not depend
on finding a desktop autoload file. `mainframe-classic-pack.crexx` retains the
RXVM-only amendment, exact libraries and canonical-library smoke separately
from the original compiler campaign.

`rxvm_single_embed` adds the public `rxvml` API to the same core without a VM
CLI entry point. The legacy native ADDRESS redirection endpoint returns an
explicit error in this build; structured endpoints and ordinary console output
retain their existing implementation. The one-thread contract still applies.

`CREXX_VM_CLASSIC_FULL_PROVIDERS=ON` selects the full static provider closure
for a standalone Classic research interpreter: `rxcstream`, the ordinary `rxfs`
provider (including bounded source acquisition and the resource-view entrypoint),
and `rxcontract`. The resource-view entrypoint still reports failure without
an embedding-supplied command-resource owner; the fallback does not synthesize
an execution directory. It is mutually exclusive with the reduced native
`CREXX_VM_CLASSIC_PROVIDERS` profile. Both options default to OFF, so parser-only
consumers retain their existing core. CLI and RXVML initialize the full profile
explicitly; a linker constructor is not required, and each VM owns its provider
sessions. This reuses the existing provider algorithms and adds no compiler,
assembler, dynamic provider or child-process service.
The single-thread clock/socket/process/dynamic-loading exclusions above remain
in force, including unavailable automatic random seeding.

The intended browser embedding runs this profile in one Emscripten Worker with
Worker-local MEMFS and no host mounts. Ordinary reads, writes and temporary
files use that virtual filesystem; the embedding owns its population and
lifetime. The Emscripten core and RXVML embedding target privately select
`_POSIX_C_SOURCE=200809L` so strict C99 exposes the existing POSIX stream and
string declarations. Browser opens keep ordinary C file modes. Emscripten fileguard
acquisition returns the existing initialized failure (`held()=0`, `busy()=0`,
`status()=-8`) without opening or creating the lock file, because no interprocess
lock is available. Desktop fileguard behavior is unchanged. The existing bounded
source reader still accepts only regular files, checks its byte allowance and
closes the descriptor before publishing owned bytes.

With testing enabled, an explicit `CREXX_BROWSER_FIXTURE` path to the prepared
`llcbrowser.rxbin` adds `test_single_browser` and `single_vm_browser_entry`.
This small RXVML consumer checks static provider resolution, the exact entry
descriptor, separate empty/Unicode/NUL arguments, logical SOURCE and the LLB1
success/condition boundary. The default leaf adds no browser fixture or test.
The prepared entry and native RXVML consumer have focused CREXX qualification;
the Emscripten core/embed archives compile separately. THE's actual Wasm Worker
host, stream integration and browser execution remain pending with THE.

The optional `CREXX_BUILD_COMPILER=ON` builds `rxc_single` with this static
embedding library. Build normal `rxc` and `compiler_exit_bin` first, then set
`CREXX_HOST_BUILD` to that build directory. This supplies the generated compiler
parsers/scanners, RXAS archive and reference bytecode. Keep these inputs from
the same pinned source; do not substitute an unrelated installed compiler.
The source closure in `compiler.cmake` mirrors the normal compiler, retaining
Level B/C and compiler-exit behavior. It is an experimental leaf, not the normal
desktop build or a completed CMS port.

For embedding tests, compile `embedding.crexx` with the normal compiler's
`-x --no-exe-import` options and assemble it with normal RXAS. Set
`CREXX_EMBEDDING_FIXTURE` to the resulting RXBIN and run `single_vm_embedding`.
Its eight create/load/call/free/destroy rounds include 64-bit results, arrays,
missing procedures, incompatible signatures and unavailable legacy redirects.
The fixture's `-x` does not disable exits in the compiler comparison.

Set `CREXX_REFERENCE_COMPILER` to the pinned normal `rxc` and build target
`check_single_rxc` for six optimized/unoptimized source comparisons with exits
enabled, identical assembly/bytecode/execution results, and an invalid-literal
control. Run this target in MinSizeRel, Debug and ASan/UBSan builds. The prebuilt
normal RXAS archive remains outside the sanitizer instrumentation boundary;
compiler, embedding and VM core are instrumented. On Apple hosts whose runtime
rejects LeakSanitizer, use `tools/asan-run.sh --build-leaks off` and `--leaks off`;
retain ASan/UBSan and do not claim leak qualification.

## CMS file discovery

The selected CMS31/TSO31 ADDRESS build retains the native guest qualification
boundaries described above.
Native TSO file operands passed to Rexx file operations use an explicit
native spelling such as `DSN:MFA12.INPUT`, a parenthesized PDS member or a
leading quoted dataset name. Bare `NAME.TYPE` is cREXX logical naming and
requires a configured search root; the VM's `-l` module location does not
supply roots to a separate file open. The real OS and emulated personality
use the same rule. A failed open is a file result, not evidence of heap
exhaustion.

For the experimental CMS ELF platform only, defining `CREXX_CMS_DIRENT`
selects the existing prefix/type directory filter over a host-supplied
`dirent.h`, `opendir`, `readdir` and `closedir`. Put that host header ahead of
generic libc headers. The default CMS configuration still reports `ENOSYS`;
ordinary desktop behavior is unchanged.

The Mainframe Lab runtime supplies independent, bounded snapshots of the A1
minidisk, presenting alphanumeric CMS names/types as lowercase `name.type`.
Only `.` and its empty/`./` aliases are supported; this does not create a POSIX
directory hierarchy. The runtime owns metadata, record I/O, naming limits and
mutation semantics. In particular, CMS rename does not replace an existing
destination atomically, and RXC report temporary naming/publication remains
unqualified. Enabling discovery does not establish a working CMS compiler.

Host CTests `platform_cms_selection`, `platform_cms31_selection` and
`platform_cms_directory` cover selection precedence, source buffering,
optional directory filters and iterator lifetimes. The two platform selection
tests use separate working directories so parallel runs cannot remove each
other's files. `platform_cms_text` exercises the real compiler import path
against an injected stream provider; see [CMS-TEXT.md](CMS-TEXT.md).

Native runtime tests and two-origin CMS service fixtures remain in Mainframe
Lab beside that host runtime. A complete CMS RXC import proof still requires
its separately packaged executable. Historical RXC `-X`/RXAS/RXVM demonstrations
do not establish 31-bit application execution, compiler-exit memory fit or
release packaging; small 31-bit runtime/text fixtures establish only their
tested service boundaries.

## Classic provider closure and guest qualification

The native Classic image requires `rxcstream`, the private `rxfs` subset
(`abspath`, `isfile`, `loadpath`), and the matching `classlib.rxbin` alongside
`library`, `rxfnsc`, and `rxfnsg`. Use `rxlink -p` to inspect that closure.
CLI and embedding initialization share per-VM provider sessions; the constrained
implementation is single-threaded and requires neither TLS nor constructors.
Native record FILEs are sequential and non-positionable: do not probe them with
`fileno`/`fstat` or claim positional replacement. Native `isfile` uses the
logical-name/binary boundary. Unsupported absolute/executable path discovery
signals FAILURE; it must not invent POSIX names. This is dependency support,
not qualification of every Classic STREAM operation.

PDOS fixtures use `VERSION` through `ADDRESS PDOS`. `VER` belongs to PCOMM;
using it through PDOS caused RC14 and was misclassified as a console watchdog.
Keep capture, nonzero child result, contained fault, subsequent recovery, the
64 KiB caller-file tail, wide guard, and Classic RC/.RC/.RS/handler assertions.
The Classic and PDOS packers include this maintained contract. A host build or
syntax check does not establish guest acceptance: native compile, assemble,
link and execute with the canonical libraries on each named personality remain
the gate. Existing CMS31/TSO31 and PDOS results belong to their qualified
revisions; this directory cleanup does not repeat or extend those results.

## Source-layout validation

The 17 October 2026 legacy retirement was checked on macOS Debug against
`062061f13786c44d56975ac80dd907faa9e2a9be` plus the S370 removal, obsolete-hook
cleanup and maintained include-path correction. `cmake --build cmake-build-debug
--target stage-product --parallel 8` passed without warnings. Fifteen focused
CTests passed: `platform_cms_selection`, `platform_cms31_selection`,
`platform_cms_directory`, `platform_cms_text`, `native_raw_cms`, `native_raw_tso`,
`mainframe_handlers_utf8`, `mainframe_handlers_byte`, `mainframe_stdio`,
`mainframe_batch_stdio`, `mainframe_console_signals`, both
`mainframe_address_{cms,tso}_policy` tests, `repository_source_hygiene` and
`repository_cleanup_contracts`.

All eleven recipe include roots exist. Host C99 syntax probes using those roots
resolve `platform.h`, `avl_tree.h`, `utf.h` and `rxvm.h` under both explicit
CMS/TSO profiles even with an inherited Linux define. The Level B recipe also
passes the native `rxc` → `rxas` → `rxlink` pipeline; five invalid-invocation
controls return RC 2 without creating build outputs. RC-AC09 (legacy retirement)
passes the source-reference check; RC-AC10 (maintained port continuity) passes
the checks above; RC-AC11 (preservation and documentation) retains all 114
pre-existing active files byte-for-byte and updates the owning guides.
These are source-layout, host and command-validation checks, not fresh SDK,
native guest, hosted CI or release qualification. Temporary captures were
discarded; the completed repository cleanup plan is retired.

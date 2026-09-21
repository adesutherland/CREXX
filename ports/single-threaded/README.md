# Minimal single-thread RXVM experiment

This opt-in build supports the Mainframe Lab application-port experiment. It
uses the actual switch interpreter and RXBIN 007 loader, with unchanged 64-bit
Rexx integers, values, procedure calls and synchronous language signals.
The normal top-level desktop build is unchanged.

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

`rxvm_single_embed` adds the public `rxvml` API to the same core without a VM
CLI entry point. The legacy native ADDRESS redirection endpoint returns an
explicit error in this build; structured endpoints and ordinary console output
retain their existing implementation. The one-thread contract still applies.

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

For the experimental CMS ELF platform only, defining `CREXX_CMS_DIRENT`
selects the existing prefix/type directory filter over a host-supplied
`dirent.h`, `opendir`, `readdir` and `closedir`. Put that host header ahead of
generic libc headers. The default CMS configuration still reports `ENOSYS`;
ordinary desktop and legacy CMS selections are unchanged.

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

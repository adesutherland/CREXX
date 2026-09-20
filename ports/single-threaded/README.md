# Minimal single-thread RXVM experiment

This opt-in build supports the Mainframe Lab application-port experiment. It
uses the actual switch interpreter and RXBIN 007 loader, with unchanged 64-bit
Rexx integers, values, procedure calls and synchronous language signals.
The normal top-level desktop build is unchanged.

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

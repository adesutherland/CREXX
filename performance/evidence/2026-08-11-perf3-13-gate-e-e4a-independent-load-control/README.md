# PERF3-13 Gate E E4a independent-load control

Date: 2026-08-11

Base commit: `99753ba54427` (`feat: complete RXPA multithreaded plugin sessions`)

Result: **E4a complete; E4b not implemented and requires separate approval.**

E4a retains the current fully independent VM/module load as the correctness
control before sealed program storage may be shared. The new internal CTest
loads the same control RXBIN into two VM contexts and runs it through the
compiler-selected `rxvml`, explicit `rxbvml` and a test-only direct-threaded
RXVML executable.

The control proves:

- canonical instruction and constant content is byte-equivalent but stored at
  different addresses in independent contexts;
- runtime domains, allocator workers, modules, semantic graphs, globals,
  procedure/frame state, execution images, bindings and caches are distinct;
- mutating either context's module global cannot affect the other;
- a late load advances only its receiving context's module table and semantic
  generation, and both contexts execute equivalently after independent loads;
- both contexts destroy cleanly with the normal live-allocation teardown check.

For the deliberately small fixture, one context materializes 176 canonical
instruction bytes plus 2,304 canonical constant bytes. The second context
therefore repeats a conservative 2,480-byte immutable candidate floor. The
counted module/global/procedure/execution/cache structural overlay floor is 569
bytes per context. Semantic graph contents, names/descriptions, values, symbol
trees, binding rows, frame contents and allocator bookkeeping are excluded, so
the figures are lower bounds and not a representative-program RSS estimate.

No production loading, execution or dispatch source changed. No public API,
RXAS/RXBIN surface, worker thread or channel was added. The evidence is
structural rather than timed, so the host was not reserved and elapsed values
are not used as a verdict.

Focused Debug, ordinary profiling-off Release and Apple AddressSanitizer pass
3/3 across the product, switch and direct-threaded forms. Apple ASan uses
`detect_leaks=0` because LeakSanitizer is unsupported on this platform; Debug
teardown retains its exact live-allocation assertion. The adjacent panel passes
10/10 and the final full Debug suite passes 2,037/2,037.

The selected E4b direction is a runtime-owned reference-counted sealed
generation containing audited immutable descriptors, canonical instructions,
constant/metadata pools and semantic graphs. Every VM keeps a worker-owned
overlay for globals, procedure runtimes/frame recyclers, execution images,
bindings and caches. Late loading constructs and validates a derived generation
before publication while active executions retain the old generation. E4b must
start with a separately approved numbered production plan and ordinary-Release
performance verdict.

Files:

- [`COMMANDS.md`](COMMANDS.md): exact qualification commands
- [`DEBUG-CONTROL.txt`](DEBUG-CONTROL.txt): retained Debug output
- [`RELEASE-CONTROL.txt`](RELEASE-CONTROL.txt): retained profiling-off Release output
- [`ASAN-CONTROL.txt`](ASAN-CONTROL.txt): retained Apple AddressSanitizer output
- [`ADJACENT-DEBUG.txt`](ADJACENT-DEBUG.txt): reentrancy/dispatch/late-load panel
- [`FULL-DEBUG.txt`](FULL-DEBUG.txt): complete Debug CTest result
- [`HOST.md`](HOST.md): structural-evidence host/configuration record
- [`SOURCE-SHA256SUMS`](SOURCE-SHA256SUMS): reviewed source and fixture hashes

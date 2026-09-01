# Native system-sampling lane

These are Apple `sample` captures against the uninstrumented, profiling-off
formal Release `rxvm`/`rxbvm` binaries. They are diagnostic and never product
timing samples.

Coverage:

- the eleven-workload rxvm portfolio once;
- repeated rxvm Bounce, Richards and Base64 captures;
- rxbvm confirmation for Bounce, Richards and Base64;
- benchmark stdout/stderr retained beside every sample for correctness review.

The first Mandelbrot size-2000 attempt lacked a governed checksum reference
and is rejected but retained. `mandelbrot-rxvm-valid.*` is the accepted
size-750 checksum-gated replacement. No other passing capture is removed as an
outlier.

Stable selected stacks are summarized in the workload dossiers. In particular,
Bounce is dominated by `rxvm_reference_storage_in_value_tree`; Richards by
`copy_value`; Storage/Towers by copy, clear/reset and allocator helpers; Base64
by the interpreter run loop with string-library helpers; RexxCPS by the run
loop, `memmove` and decimal/string parse/format helpers.

Host limitation: `xcrun xctrace` is not installed and no supported hardware
event counter path is available on this host. Cycles, native instructions,
branch misses, i-cache and iTLB event counts therefore remain unavailable. The
large generated interpreter translation unit also limits handler-level symbol
resolution in statistical stacks; explicit schema-5 opcode timing supplies the
selected handler attribution without treating it as product timing.

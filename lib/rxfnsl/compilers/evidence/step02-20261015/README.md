# STEP02 IR foundation qualification

15 October 2026, Apple M1 Pro, macOS ARM64. Baseline commit
`d13a91c1bfe4b47e7164ba16015e2ab00285c322` is committed locally on `develop`.
This record qualifies the STEP02 increment approved for publication to
`develop`, including that baseline. Its source/tool hashes were rechecked before
publication. Normal automatic Build CREXX and CodeQL results are tracked in
GitHub and the lightweight monitor; this receipt establishes local evidence,
not a hosted CI pass or completion of the source compilers.

## Result and input boundary

The Level B-only `llir` and `llirservices` modules implement numeric graph
storage, checked schemas/builders/views, exact codecs, semantic service dispatch,
revision facts, flow analysis, checked replacement/lowering and chunked output.
The separate `llcrexxir` consumer supplies initial C/G families, including SAY.
The independent arithmetic fixture imports no compiler family.

[inputs.json](inputs.json) pins twelve implementation/test/build inputs and the
actual tool/library/bootstrap images. Product build banners contain an older
configured SHA; they are not used as source identity. The product build includes
pre-existing unrelated assembler and test-registration edits, whose hashes are
recorded. This task changed no C implementation and preserved those edits.

During publication preparation, the owning task committed the already-qualified
assembler repair and its native-package helper as `a9c080f54`. Adrian also
requested inclusion of the sensible assembler fix. The STEP02 publication
preserves that commit as its parent; the recorded source hashes still match.
An additional [assembler boundary check](assembler-boundary.json) recompiled
the pre-fix committed assembler source into a private tool and reproduced all
twelve STEP02 assembly products byte-for-byte. This establishes that STEP02
does not rely on the label repair; no checkout or qualified product binary was
replaced by the check. The initial absolute-input-path probe changed embedded
path metadata; matching the original input spelling gave exact equality.

The foundation uses a pure straight-line target-lowering profile. Stateful,
failing or control-changing lowerings remain later backend obligations. Classic
contextual arithmetic has a declared operation but no evaluator. A successful
STEP02 is not completion of LLCOMP-AC01–15 or STEP03–10.

## Checks

| Check | Result and retained evidence |
|---|---|
| Ordinary product build | `cmake --build cmake-build-levell-integration --parallel 10` completed; final incremental build completed after fixture corrections. Final changed sources compile without warnings. |
| New IR/semantic/example tests | 16/16 CTests, opt/no-opt consumers on both concrete VMs. [Qualification log](qualification.log). |
| Existing relevant behaviour | 113/113 CTests: AST, analysis, general backends, structured flow, condition facts, lowering, source origins, generated consumers and tinyexpr; same log. |
| Level B closure | Standalone bootstrap with `CREXX_LEVEL_L_NOOPT=OFF` and `ON`; both verified L0/L1/L2 regeneration. Import roots contain Level B and Level L only. [Bootstrap log](bootstrap.log). This is seed-built L, not the future proper-B self-hosting proof. |
| Whole no-opt delivery | Compiled the compiler families and four consumers against the no-opt bootstrap library. 22 successful compiler/assembler/linker/VM commands, eight executions across `rxbvm` and `rxtvm`. [Command receipts](delivery.json). |
| Installed tools | Isolated prefix under `build/levell-ir-step02/installed`; manual library/program commands and installed `rxvm` print `5` and `IR_DEMO|OK`. The last three command receipts are the corrected guide sequence. |
| Layout screen | Correct field values, post-rewrite checksum 8014000 and identical 48,000-byte payload/49,152-byte capacity for 1,000 twelve-field rows in both layouts. [Raw observations](storage.json). |

Regression cases include malformed ranges/ownership/reserved fields, truncated
and incompatible images, typed arguments/multiple results/attributes, stale and
foreign handles, frozen mutation, exact CRLF/NUL/non-UTF-8 source bytes, semantic
rejection, malformed evaluator/lowering outcomes, failed budgets, int64 extremes,
unknown effects, joins/loops and unreachable blocks. Failed transformations keep
the original image; failed loads publish no successful graph. UTF-8 text codecs
reject malformed and overlong encodings. SAY retains output effects and ordering.

The multi-result fixture uses an explicit `.llir..opview` binding: native type
inference did not converge for the inferred spelling. The isolated reproducer is
`build/levell-ir-step02/signature-repro.crexx`; the typed form passes both modes.
No native compiler repair is claimed. Initial wrapper attempts incorrectly bundled
`rxfnsl` into the reusable library and caused duplicate providers when the wrapper
also supplied the program runtime. The corrected guide leaves runtime imports
external in the library. Failed development attempts remain in the command
receipts and are not counted as passes.

## Storage decision and measurement boundary

The Level B harness is [levell_ir_storage.crexx](../../../../../tests/performance/levell_ir_storage.crexx).
It constructs 1,000 twelve-u32 rows, checks every field, scans field five, increments
that field and checks the checksum. Each concrete Release VM ran serially in one
process with sample zero as warmup and three retained samples, row then column.
Compiler/assembler optimisation was enabled; linking used `-s`. VMs were rebuilt
from the current checkout with profiling off before measurement. A stale Release
VM first rejected the current image format; no timing from that attempt is used.
The fixture was compiled by the Debug-built native compiler; its bytecode is the
same fixture for both Release VM executions. Tool, image and source hashes are in
`storage.json`. Process elapsed time is separate from kernel microseconds.

Observed construction microseconds:

| VM | Row samples | Column samples |
|---|---|---|
| rxbvm | 3878, 3369, 2984 | 5833, 5330, 4834 |
| rxtvm | 4116, 3536, 3136 | 6353, 5649, 5157 |

Columns were slightly quicker in the single-field scan/update; all raw timings
are retained. Both layouts warmed across samples, so these short runs are not a
steady-state speed estimate. Retain row records for the initial append-oriented
foundation: construction is simpler and the observed tradeoff supports it.
Revisit with representative compiler workloads before tuning indexes or packing.
The column mode remains only a comparison implementation.

This is a non-representative mechanism screen. RexxCPS has no corresponding IR
arena workload and is inapplicable. Payload/capacity exclude VM objects, schemas,
indexes, pool-object overhead, allocator overhead and simultaneous rewrite
snapshots. No process peak-memory, compiler throughput or cross-platform claim is
made. Normal hosted and overnight assurance remain separate from these local
checks; no extra matrices were dispatched.

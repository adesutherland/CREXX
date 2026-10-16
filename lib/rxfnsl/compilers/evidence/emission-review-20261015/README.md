# STEP04 static emission review evidence

15 October 2026, macOS ARM64; source commit
`bf74c72111fbd466d1f93c421e6d5dadbbedcc2c`, develop. Compiler/test/build sources
were unchanged during the review. Documentation and these receipts are the
only task edits. This is a small structural/functional review, not a timing,
memory, full TRACE, sanitizer, hosted-CI or replacement qualification.

## Inputs and execution

The four source inputs are the versioned `examples/step04/literal-{c,g}.crexx`
and `examples/step01/pipeline-{c,g}.crexx`. B controls replace only
`options levelg` with `options levelb` in the corresponding G source. The
sources were copied to a fresh temporary directory so every compiler received
an absolute filename; this deliberately exercises emitted source naming.

Reused the qualified Debug build tools in
`cmake-build-levell-integration/bin`. No build was required. Native version
banners contain an older embedded label; actual tool/runtime hashes are in
[manifest.json](manifest.json), along with source hashes, host, raw work
directory and generated artifact hashes. The existing STEP04 input receipt
remains the broader qualification record.
All eleven tool/runtime inputs used here match that receipt byte-for-byte,
and its complete source/test/build hash set still matches. This verifies reuse
of the qualified implementation despite the stale version-banner text.

[commands.json](commands.json) records all 16 actual compiler invocations and
exit statuses. [review.json](review.json) records structural counts, frames,
all assembler/linker/VM commands and exit statuses, expected/actual stdout,
stderr and compiler diagnostics. Every compiler, assembler, linker and VM
invocation succeeded; all 16 outputs matched the stated expectations.

For each of `literal` and `pipeline`, routes were:

- Native B and G: `rxc [-n] -i BIN -o OUTPUT.rxas SOURCE`.
- Native C: `rxc-c [-n] -i BIN -o OUTPUT.rxas SOURCE`.
- New C/G: `llrxc-c` or `llrxc-g -i BIN -o OUTPUT.rxas SOURCE`.

Each output then used `rxas -n`, `rxlink` without stripping metadata, and
`rxbvm`. Typed programs link `library.rxbin`; native C also links `rxfnsc.rxbin`;
new C additionally links `llctarget.rxbin`. The exact argument vectors in the
JSON remove ambiguity about output stems and runtime paths. No normal-assembler
or second-VM rerun was needed for this static review; the original STEP04 receipt
retains those distinct execution checks. New repairs will need their own
affected-mode regressions.

## Counts and retained examples

The counts describe readable RXAS **before assembly**:

- An instruction line excludes blank/comment lines, `.locals` declarations,
  labels, imports and dot-directives. Fused instructions are one line.
- Calls include ordinary `call*` and fused `settp*call` instructions.
- Source/TRACE records are counted separately. Consecutive equal source records
  compare consecutive entries in the source-record stream; labels or execution
  between them mean this is a candidate count, not proven deletions.
- Classic location loads match literal `file:line:column` loads. Distinct loads
  are distinct emitted texts. `.locals` values are listed per function, not
  added together or treated as simultaneous live values.
- RXAS byte counts include physical filenames and all metadata. They are not
  bytecode instruction sizes, linked runtime sizes or runtime measurements.

Eight small/representative raw RXAS files are retained in [samples](samples/):
native literal B opt/noopt, native literal C noopt, new literal C/G, native
pipeline G opt/noopt and native pipeline C noopt. Other new outputs have the
same code shape as the existing STEP04 examples, with the supplied absolute
filename instead of the example's relative name. All sixteen raw output and
assembled/linked hashes are retained; raw scratch outputs are also identified
in the manifest. The absolute paths inside the two new literal samples are
intentional evidence of the current filename defect, not the proposed format.

The [review](../../doc/EMISSION-QUALITY-REVIEW.md) explains the findings. No
golden file, compiler implementation, permanent test or build configuration
was changed. All quality/repair gates remain open in the authoritative plan.

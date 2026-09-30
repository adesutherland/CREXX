# Mainframe text I/O boundary repair

## Vision and intended outcomes

I want one cREXX source tree that keeps UTF-8 inside the compiler, assembler
and VM while using native IBM1047 at mainframe standard streams. cREXX owns
conversion with the existing shared codec; the SDK conversion setter retains
its canonical default and cREXX entry points explicitly select raw I/O.
Selected external file encodings remain independent of the console. Text file
and console input/output must work, and binary files and standard streams must
preserve exact bytes. Native macOS build and regression proof complete this
first repair step; mainframe builds, guests and release delivery are later steps.

## Acceptance criteria

1. **AC-01:** Text stdin reads decode IBM1047 once for READLINE, FREADLINE and
   FREADCDPT, including LF, empty lines and accented characters. Focused host
   simulation and handler inspection establish this component scope.
2. **AC-02:** Console text output and first-party tool diagnostics encode
   IBM1047 independently of `-E`. Focused host simulation checks both stdout
   and stderr, format return values and long diagnostics.
3. **AC-03:** Mainframe text files use the existing codec wrapper, retain
   selected encoding, and reject malformed or unrepresentable data. Binary
   file and standard-stream operations retain exact byte I/O.
4. **AC-04:** Native macOS builds and relevant regressions pass from this sole
   implementation tree. Broad validation and handoff are coordinator-owned;
   no mainframe guest qualification is claimed by the host simulation.

## Implementation steps

1. **STEP-01:** Connect UTF VM text stdin to the shared codec and correct
   exchange-file append and unsupported update-mode handling (AC-01, AC-03),
   preserving the existing CMS/TSO naming and file codec adapters.
2. **STEP-02:** Forward mainframe first-party diagnostic stdio to the existing
   console writer; preserve ordinary non-console stdio (AC-02).
3. **STEP-03:** Add permanent focused host simulation and update the I/O guide
   (AC-01 to AC-03). The coordinator builds and validates macOS from this tree
   and records AC-04 proof before pausing after the first repair step.

## Acceptance results

1. **AC-01 — passed, host simulation:** `mainframe_handlers_utf8` and
   `mainframe_handlers_byte` compile and execute the actual three handler
   bodies. They verify IBM1047 accented input, LF, empty lines and EOF, with
   UTF-8 text in the UTF VM and raw bytes in the BYTE VM. `mainframe_stdio`
   independently verifies the shared platform getter and file-codec isolation.
2. **AC-02 — passed, host simulation:** `mainframe_stdio` verifies stdout and
   stderr, direct `vfprintf` forwarding, formatted return counts, long output,
   ordinary FILE passthrough, and malformed/unrepresentable text.
   `oom_mainframe` verifies native low-level diagnostics, interruption, short
   writes and errno preservation. Independent mainframe-defined syntax checks
   pass for the tool entry points and both VM handler variants.
3. **AC-03 — passed, host simulation:** `mainframe_stdio` verifies native file
   conversion, retained open-stream codec selection, UTF8/Windows1252 append,
   exact 00–ff binary file round trips, raw standard-stream byte operations,
   text error/close behavior, and rejection of text `+` before truncation.
   `platform_cms_text` passes all 69 nested source/import/assembly/execution
   and fault checks with identical optimized/unoptimized products. Existing
   CMS/TSO raw adapter and text selector regressions also pass.
4. **AC-04 — passed, macOS scope:** The core build passes; essential/smoke
   passes **167/167**, and the affected focused union passes **32/32**, including
   file characters/binary, interactive stdin, console decoding, both handler
   modes and the existing `version()` tests. No new version instruction was
   added; the existing platform whitelist and guide now include `tso`.

All three implementation steps are complete for this bounded host repair.
The coordinator retains full commands and logs in
`crexx-release/work/io-repair/`, including `host-core-final-build.log`,
`host-smoke-ctest.log`, `host-focused-ctest.log`,
`host-final-platform-ctest.log`, `host-final-handlers-ctest.log` and `review.md`.
The overall delivery plan remains `crexx-release/IO-REPAIR-PLAN.md` in the Lab.
This source plan owns the repair details; the Lab plan owns later delivery.

Mainframe builds, guest execution, sanitizer qualification, release delivery
and publication have not been performed for this repair. Mainframe build and
guest qualification remain the next steps after the requested pause. The host
fixtures do not establish full guest VM or signal-transport behavior.

## CMS compiler failure repair (authorised 30 September)

The supplied CMS VM passes the curated automatic program, but RXC importing
the same LIBRARY reports invalid record metadata and then a protection
exception. I want the supported native compile/assemble/run chain to succeed
and allocation failures to terminate with an accurate diagnostic. The exact
cause must be established before selecting the repair. Existing qualified
inputs and unrelated source changes remain intact.

1. **CMS-AC-01 — passed:** One bounded diagnostic application captures the
   failing allocation/heap state or establishes another precise cause, with
   guest PSW/register evidence if it still abends.
2. **CMS-AC-02 — passed, focused host scope:** The minimal repair preserves reader ownership and
   error cleanup. Focused host checks cover the repaired failure mechanism;
   no broad unchanged suites are repeated.
3. **CMS-AC-03 — passed, CMS31 guest scope:** The affected applications use
   reviewed canonical inputs with 64 MiB heaps and 3 MiB stacks. Native RXC
   to default optimised RXAS to fresh IOGUE3 RXBIN returned 0/0/0; the new
   RXBIN automatic qualification returned 8 PASS, 0 FAIL, 3 SKIP. No `-N`
   option or diagnostic wrapper is used in the final chain.

The observed 24 MiB application failed a 2 MiB record-table realloc at
23,661,856 heap bytes, then failed the AST factory's 320-byte malloc at
25,165,636 bytes. The AST factory dereferenced that NULL result. Reader
allocation failure had been overwritten with a misleading format diagnostic.
The coordinator independently reviewed the diagnostic transcript. This
establishes capacity exhaustion and the unchecked allocation mechanism;
post-abend CMS PSW/display data do not identify the interrupted C instruction.

The minimal common repair preserves record allocation detail and applies the
existing terminal OOM guard before AST-node initialization. Permanent
`record_allocation_failure` checks pool allocation and both table growth
failures, partial cleanup and reuse of the reader on a valid container.
`ast_allocation_failure` checks the terminal guard and healthy factory.
The coordinator approved a distinct CMS RXC 64 MiB syscall member built from
unchanged canonical source bytes, retaining the old 24 MiB member; RXVM keeps
24 MiB and RXAS the ordinary 16 MiB archive policy. This capacity policy is
owned by the Lab runtime inputs, not a cREXX runtime interface change. Those
were the first repair candidate's sizes; the later user-approved policy
selects 64 MiB heaps for all three CMS31 applications. Original inputs remain
retained.

The affected host build and seven focused allocation/error tests pass in
`host-cms-guards-build.log` and `host-cms-guards-ctest.log`. Independent CMS/TSO
defined UTF/BYTE syntax checks pass in `host-cms-guards-syntax.log`. Existing
167 smoke, 32 I/O and 90 qualification CLI results remain retained evidence
for unchanged behavior; this focused repeat does not replace those records.
The final guarded 24 MiB RXC also passes the native failure check: accurate
2 MiB record-table OOM, bounded 320-byte AST OOM diagnostic with errno 12,
RC -1, and no protection exception. The CMS guest report retains that
transcript separately from the old failure.

1. **CMS-STEP-01:** Instrument existing allocation calls in a diagnostic
   application only, reusing the retained link closure and 24 MiB runtime
   member (CMS-AC-01). The CMS worker owns lease and terminal execution.
2. **CMS-STEP-02:** Implement the proved source repair and permanent focused
   regression; have the coordinator independently review it (CMS-AC-02).
3. **CMS-STEP-03:** Rebuild affected application objects only and repeat the
   affected guest checks; retain the old identities and diagnostic evidence
   (CMS-AC-03). Runtime prerequisite changes require coordinator review.

The repaired 64 MiB RXC subsequently compiled the full qualification source
with RC 0. Ordinary RXAS failed on the optimisation path; the bounded `-N`
split passed on the same source. One task-owned 1 MiB entry-stack experiment,
using the same assembler objects, WAIT archive and 16 MiB heap, then passed
normal optimisation with RC 0. It measured 113,356 bytes used and intact
`5aa55aa5` low/high guards. This proves that the original 64 KiB CMS stack
was insufficient. No optimiser code change is indicated. Canonical startup
was unchanged during the measurement; the final capacity selection and
native acceptance are recorded below.

The coordinator reviewed a final 3 MiB CMS31 stack for RXC, RXAS and RXVM,
with 64 MiB heaps for all three. The canonical SDK entry changes only the
stack/scan size and paint-block count. This gives over 27 times the measured
RXAS stack use and keeps the largest application image within the existing
8 MiB CMS exporter limit. The affected final links pass instruction audit,
MODULE verification and the three-member transfer-tape scan. Original
capacity inputs and diagnostic applications remain retained. Final native
default compiler/assembler/VM and interactive qualification passes; no new
optimiser or exporter code was introduced.

The supplied RXBIN automatic check also returned 8 PASS, 0 FAIL, 3 SKIP.
The full native interactive check returned 9 PASS, 0 FAIL, 2 SKIP, with
observed input lengths 11/0/10/1 and exactly one Enter for each of the four
inputs, including the empty line. All current Q12 diagnostic checks pass.
After closing the binary output, independent guest-file readback confirms
the exact 256-byte sequence. The final guarded 24 MiB RXC negative probe
remains a separate passing failure-path check: accurate allocation OOM,
bounded terminal AST diagnostic, RC -1 and no protection exception. The
larger final capacities permit the supported normal chain; they do not
replace that guard proof. The Lab CMS report retains exact product identities,
native receipts and guest handback status. All CMS acceptance criteria here
are complete for this bounded repair; unchanged host and TSO evidence is
retained without another suite run.

## Upstream integration follow-up

Adrian authorised upstream review, develop integration and beta 3 preparation
on 30 September. The [integration record](beta-3/mainframe-integration-2026-09-30.md)
owns that delivery and its new console-signal criterion. The Lab source and
its earlier guest artifacts remain unchanged; upstream signal propagation is
a separately approved follow-up and needs exact-package native qualification.
Historical receipts above retain their original source and scope.

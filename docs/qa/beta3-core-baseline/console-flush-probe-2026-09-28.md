# Native console `fflush` risk probe

28 September 2026. Read-only implementation input: local HEAD
`30d72305459ca230befffcdc5208cfce83982a86`; no production or test input
was edited. This is a host-mock observation, not a CMS/TSO native-backend or
package qualification. AC-04/AC-11 and the standalone console binding remain
open.

The existing `platform/tests/test_native_raw_adapter.c` backend was copied to
`/tmp/beta3-flush-probe.UIaDkG/probe.c`. Only the temporary mock's standard
record capacity and matching pending-buffer bound were raised from 4 to 32 so
the exact eight-byte `prompt> ` fits; its existing `main` was renamed and a
small probe `main` was appended. The temporary source SHA-256 is
`3a71578cc23f35f29006b4c634241c208b881cda68e560aeb910ec94b4870bc8`.
The probe was compiled with the Debug target's CMS and TSO definitions and
linked to the corresponding existing `test_native_raw_cms` and
`test_native_raw_tso` platform objects. It used the real
`crexx_native_standard(1)` adapter and the raw backend mock; no guest ran.

Each profile ran three observations, asserting return values, raw writes,
committed records and visible output:

| Scenario | `fflush` observation | `fclose` observation |
| --- | --- | --- |
| `fputs("prompt> ", stream)` | Return 0, errno 0; **zero raw writes, zero committed records, zero visible bytes** | Return 0; one eight-byte record becomes visible. |
| Same prompt, with mock `RAW(flush)` set to fail with `EIO` before `fflush` | Return 0, errno 0; no visible prompt | Return -1, errno `EIO`; the prompt record was committed before the failed raw flush. |
| `fputs("A\n", stream)` with the same deferred raw flush error | Return 0, errno 0; one record and one byte already visible | Return -1, errno `EIO`. |

Both binaries passed their assertions with identical output. The CMS and TSO
logs are `/tmp/beta3-flush-probe.UIaDkG/result.log` and `result-tso.log`,
each SHA-256
`06e5594b554174b912f7d3d0bc5b97d972dd3e7d68ba04e8ba9cbbcf4b32274c`.
An initial temporary run raised only the advertised capacity, leaving the
mock's internal four-byte pending bound unchanged; its close failed with
`EOVERFLOW`. The final probe aligned both temporary limits and passed. That
initial mock setup error is not a product finding.

The mechanism is visible in `platform/platform_native.c`: record-mode
`put_scalar` retains a partial line until newline; `stream_close` commits a
final partial record and is the only caller of `RAW(flush)`. The `funopen` and
`fopencookie` cookies have read/write/close callbacks but no flush callback.
Thus `fflush` can deliver the C `FILE` buffer into the cREXX record buffer
without committing it or checking the raw backend's deferred flush error.
The raw-service handoff says raw `flush` reports deferred errors and does not
itself end a record. The current behavior may be acceptable for a complete
line, but it does not make a partial prompt visible before an input read, and
`fflush` does not report a deferred raw error even after a complete line.

For comparison, the existing converted TSO runtime explicitly calls
`fflush(stdout)`, `fflush(stderr)` **and** drains its native partial line before
`TGET` (`mainframe-lab/runtime/tso/io.c`, `read_terminal`). The converted CMS
runtime emits console output on newline or a full buffer and drains a residual
line at finish (`mainframe-lab/runtime/cms/newlib_syscalls.c`); the inspected
input path has no analogous pre-read drain. Neither older converted path is
the new raw backend or proof that arbitrary `fflush` has the desired contract.

Two small platform-local directions need native-runtime reconciliation before
any code change: a native stdio flush/bind mechanism that can distinguish
`fflush`, commit a pending console prompt and then call raw `flush`; or an
explicit pre-read console drain for interactive prompts. The latter alone does
not make arbitrary `fflush(stream)` report deferred raw errors. Committing a
record on every cookie write would make record boundaries depend on C stdio
buffering and would break logical lines. Standard C clients and embedding
retain their host-owned streams; no FILE-slot assignment or API was made here.

The recorded AC-11 is specifically for maintained CMS31/TSO31/TSO64 text
boundaries. Desktop `platform_text_encoding` accepted UTF8 spellings and
rejected other `-E` selectors already at published base `143921e11e`; that
behavior is unchanged here. Desktop preservation and audit still matter under
the overall baseline, but extending desktop legacy `-E` pages is not an
implicit native AC-11 deliverable or part of this risk probe.

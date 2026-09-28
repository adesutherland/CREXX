# Native raw-service implementation target (version 1)

The current implementation target is [`platform/native_raw.h`](../../../platform/native_raw.h),
shared signatures with separate `lab_cms_raw_*` and `lab_tso_raw_*` symbols.
This is the concrete CMS/TSO backend handoff, not a provider registry. Minor
`platform_fopen`/supporting-header/backend reconciliation is allowed as native
details settle, with focused upstream QA and the combined native gates still
required. It implements
[the agreed text boundary](text-boundary-2026-09-28.md). It is a service contract;
no current runtime package is claimed to supply it yet.

All name, member, argument, environment and console payloads are native IBM1047
bytes, with explicit lengths and no required terminator. File payloads are raw
bytes in the cREXX-selected external page. No routine decodes, encodes, casefolds,
adds suffixes, chooses namespace roots or rewrites cREXX paths. cREXX performs
those operations. The numeric flags are encoding-neutral; the header is usable
by ordinary C clients. An embedded zero is a payload byte, not an implicit end.
Native-name restrictions may reject it with EINVAL. There are no codec setters.

## File and console operations

- `open` returns an owned handle or NULL with errno. Flags select READ or WRITE,
  optional APPEND with WRITE, and optional RECORDS. Native names are already
  mapped/encoded. Unsupported operations return ENOTSUP. WRITE without APPEND
  truncates/creates according to native access rules. Existing dataset attributes
  remain native policy. The returned `record_capacity` is the maximum writable
  logical-record length, or zero for byte mode; it must be nonzero in record mode.
  cREXX rejects an overlong line before supplying any bytes of that record.
- `standard(0/1/2, flags, &capacity)` returns an owned wrapper around standard
  input/output/error, respectively. It uses the same record service contract;
  release closes the wrapper, never the process's underlying standard service.
  Console wrappers use RECORDS. They preserve native bytes without conversion.
- `read` returns 0 on success or -1 with errno. It initializes count/end/eof on
  every call. Positive capacity is required. It never crosses a record boundary
  in RECORDS mode; short reads may occur and callers continue until record_end.
  An empty record is count=0, record_end=1, eof=0. EOF is count=0, record_end=0,
  eof=1. A nonempty final record is returned normally before a subsequent EOF.
  Byte mode always returns record_end=0. A zero-count non-EOF/non-record result
  is forbidden. A read error returns no new bytes, count/end/eof all zero.
- `write` returns consumed bytes or -1 with errno. Short positive writes are
  allowed. `record_end` commits the record **only when the entire supplied chunk
  was consumed**; on a short write, the caller resubmits the remaining bytes with
  the same end flag. A zero-length write with end=1 commits an empty record (or
  finishes pending bytes) and returns zero. Nonempty requests must make progress
  or fail; zero is not success. In byte mode record_end must be zero. The backend
  must reject a record exceeding capacity with EOVERFLOW and must never silently
  split, truncate or commit it. After a write error, the caller closes the stream;
  already committed records cannot be rolled back.
- `flush` reports deferred I/O failures; it does not implicitly end a record.
  `close` always releases the handle, including on error, and reports deferred
  write/flush/native-close errors via -1/errno. Pending unterminated record data
  is discarded and reported as EINVAL; cREXX must finish its last line first.
  Handles must not be used or closed again after close. On both operations a
  successful result is zero. Preserve the earliest meaningful I/O failure.
- `stderr` is the emergency record-output path. It follows write's consumption
  and record-end rules, takes already encoded native bytes and must allocate no
  heap memory, including its first call. It must remain callable after allocation
  failure and must not require stdio initialization. It uses fixed native work
  buffers; reports errors via -1/errno. cREXX bounds/converts diagnostic chunks.

## Directory and process inputs

- `directory_open` returns an owned iterator or NULL/errno. Missing optional
  roots use ENOENT; unavailable enumeration uses ENOTSUP, and damaged directory
  data or service errors use an appropriate failure such as EIO. Native dataset
  and member handling belongs below; cREXX suffix/case/root policy stays above.
- `directory_next` returns 1 for an entry, 0 for clean EOF, -1/errno for error.
  Entries are actual native member spellings, without a terminator. `length` is
  the actual byte count. Insufficient capacity returns ERANGE, sets required
  length and retains the entry for retry. EOF sets length=0 and errno=0. No entry
  may be truncated. `directory_close` always releases; 0 success, -1/errno failure.
  cREXX propagates enumeration and close errors; NULL must not mean both EOF and
  silently ignored damage.
- The cREXX host adapter currently treats a CMS directory entry as the actual
  18-byte native file identifier: padded 8-byte name, padded 8-byte type and
  2-byte mode, each IBM1047. It treats a TSO entry as the actual member name
  without a cREXX suffix. The CMS payload shape is provisional until the lab
  backend supplies and tests its iterator; changing that small ABI detail
  requires matching adapter tests and does not transfer suffix/root policy
  into the runtime.
- `argument_count` returns the process argument count (including argument zero),
  or -1/errno. `argument(index, ...)` copies native bytes and returns 0 on success,
  -1/errno otherwise. The count and arguments remain stable for process lifetime.
- `environment(name, ...)` returns 1 for a present value (including empty), 0 if
  absent, or -1/errno. No name/value conversion occurs below this boundary.
  Copy interfaces set required length on ERANGE and never truncate. On other
  errors length=0. The caller owns output buffers; the backend owns no returned
  strings. Absent environment support is ENOTSUP, not false absence.

Successful calls leave errno unspecified except directory EOF. Callers inspect
return values first, not stale errno. Every failed operation sets errno. EINTR
is retryable only when the return contract reports no consumed bytes. Stream
handles are single-owner unless the backend explicitly documents stronger
thread guarantees; no callback/provider framework is required.

## Delivery and acceptance

The lab agent implements this raw route and explicitly bypasses its existing
converted stdio/name/argv compatibility path for cREXX builds. Existing plain-C
converted entry points can remain independently selected. They must never be
stacked under the cREXX decoder, and any transitional already-decoded arguments
or names remain a named open gap. Host mocks prove the cREXX adapter only.
Combined backend tests and CMS31/TSO31/TSO64 native package gates remain OPEN.

Three backend details still need an exact joint implementation receipt:

1. **Standalone standard streams:** existing RXC/RXAS/RXVM product code uses
   C `stdin`/`stdout`/`stderr` for diagnostics, SAY and TRACE. cREXX can create
   owned IBM1047 record `FILE *` wrappers with `crexx_native_standard(0/1/2)`,
   but the lab must identify a safe process-local newlib bind/restore point,
   flush/close ownership and startup order. A cREXX platform-local startup
   call in standalone tool mains is proposed; embedding APIs and ordinary C
   clients must retain their host-owned streams. No FILE-slot assignment is
   part of the current accepted implementation.
   The [host console probe](../../qa/beta3-core-baseline/console-flush-probe-2026-09-28.md)
   additionally confirms that the current cookie adapter cannot make a partial
   prompt visible through `fflush`, and only reports deferred raw flush errors
   at close. The binding/flush design must make a prompt visible before input
   and propagate flush failures. Preserve logical file records independently
   of stdio buffer chunk sizes; committing a record on every cookie write is
   not a valid fix. A pre-read console drain alone does not establish the full
   `fflush` error contract. Resolve this within the cREXX platform/native stdio
   boundary and qualify the result before attaching the wrappers to product
   standard streams. Existing host-mock passes do not close this requirement.
2. **CMS directory payload:** the adapter currently expects an actual 18-byte
   IBM1047 FID (8-byte padded name, 8-byte padded type, 2-byte mode). Confirm
   this against the raw iterator or supply its exact actual-entry shape;
   reconcile the small header/adapter detail with focused tests. The runtime
   must not invent a cREXX suffix or provider selection.
3. **Emergency stderr capacity:** `stderr` is allocation-free and record-based,
   but v1 gives no guaranteed maximum complete record length. State its
   capacity (or an equivalent explicit chunk/continuation contract) so the
   cREXX bounded panic formatter can avoid an uncommittable record after OOM.
   Until then, host mock success is not native emergency-output proof.

### Standard-stream flush reconciliation (proposal only)

The approved behavior is prompt visibility before native console input,
observable `fflush` errors, unchanged file-record boundaries and host-owned
embedding streams. No new runtime ABI or generic-newlib fork is selected here.
The [host probe](../../qa/beta3-core-baseline/console-flush-probe-2026-09-28.md)
establishes the gap in the current cookie adapter. The read-only lab source
at `dc2fad44259d4a81e4e147af5f1f078d0cd9eb6f`
(`build/compilers/libc-poc/newlib/source/newlib/libc/stdio/fflush.c`) invokes
a cookie's write callback only when its own
`FILE` buffer contains bytes; `funopen` supplies no explicit-flush callback.
Thus `setvbuf`, a pre-read drain or raw `flush` inside the existing close
callback cannot alone make an ordinary `fflush(file)` observe an adapter-held
partial console record or a deferred raw error.

One cREXX-owned route is `platform_fflush(FILE *)`. It would call libc
`fflush` first, then locate an owned raw wrapper, commit a pending **console**
partial record on explicit flush and call raw `flush`. File-record streams
would call raw `flush` without ending their pending logical record; byte and
binary streams would retain their current byte semantics. `platform_fflush(NULL)`
would first flush libc streams, then visit every live owned wrapper, including
ones with an empty libc buffer, and return an error if either stage failed.
`wrap_raw_stream` would register only successfully created `FILE *` wrappers;
the close callback would unregister before freeing the cookie. The registry's
iteration, close/error ordering and concurrency must be defined before use.
Unowned streams pass through to libc unchanged. A standard-input pre-read
drain could call this same route for bound output wrappers; it supplements,
but does not replace, explicit flush error handling. The console-only record
commit is tied to the requested flush, never to an arbitrary cookie-write
chunk, and close must not commit that prompt twice.

That route can cover audited first-party callers only if they use it. Current
direct libc `fflush` calls include `compiler/rxcp_exit.c` and `rxcpmain.c`,
`interpreter/exitfunc.c`, `rxcrexxcmd.c`, `rxspawn.c`, `rxvml.c` and
`rxvmprogram.c`, plus `lib/plugins/console/console.c`; the keyaccess plugin
also flushes file handles. Only actually packaged native components would
be routed in this phase. Libraries and plugins may call libc directly, and
newlib's own automatic line/buffer/exit flushing does not call a
`platform_fflush` function. Replacing product call sites or applying a macro
to cREXX sources does not intercept an unmodified C client calling the
standard `fflush` on an exposed `crexx_native_standard` or `platform_fopen`
wrapper. `fclose` remains covered by the current close callback, but it cannot
repair the earlier explicit-flush return value. Therefore a platform-only
route is useful for owned call sites and host qualification, but does not by
itself satisfy the full ordinary-`fflush(FILE *)` contract for exposed wrappers.

The alternative is an application-independent, **generic per-stream**
explicit-flush callback supplied by the native stdio/link boundary. It would
run after libc drains the selected `FILE` buffer, including an empty buffer
and each owned stream visited by `fflush(NULL)`, propagate callback failure
with errno, and unregister safely on close. It must distinguish explicit
`fflush` from incidental buffer-full/line-buffer writes so physical record
boundaries do not depend on buffering. The runtime callback knows neither
cREXX text nor IBM1047; cREXX's platform callback alone decides console
record completion and calls the existing raw `flush`. A generic link shim or
other lab-owned stdio mechanism may meet this contract without forking newlib;
its feasibility, static/dynamic call coverage and bind/restore lifecycle are
for native reconciliation. The current raw `write`/`flush` signatures need no
change for this proposal: a completed console record must become visible by a
successful raw `flush`, which must report deferred failure and must never
implicitly end a file record.

The smallest complete direction is to retain cREXX's per-stream flush logic
in the platform layer while the lab confirms a generic explicit-flush delivery
mechanism and process-local standard-stream bind/restore. A platform-only
`platform_fflush` may qualify first-party call paths in the meantime, but the
direct standard-C call gap and AC-04/AC-11 remain open. No FILE-slot write,
hook registration, runtime change or product startup edit is authorized by
this proposal; both alternatives require focused host and combined native
tests before acceptance.

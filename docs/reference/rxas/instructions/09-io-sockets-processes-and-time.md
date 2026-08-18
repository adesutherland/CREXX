# I/O, Sockets, Processes, And Time

These instructions expose provider channels, C-stream I/O, console I/O,
environment and time queries, VM-managed TCP/TLS sockets, and structured child
process execution. File handles are opaque `FILE *` values stored in integer
payloads; socket and channel handles are VM-managed integers. Unless stated
otherwise, low-level C-stream failures are returned or left in stream status
rather than translated into VM signals.

## `chancancel`

Request cancellation of one operation owned by a channel.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x028d` | `chancancel rStatus,rChannel,rTicket,rReason` | Return cancellation status. |

### Operands And Semantics

The channel and ticket are execution-local integer capabilities. `rReason` is
a canonical RXCV binary reason document. Only `rStatus` is written. Providers
may report a race with an already-terminal operation; cancellation never
transfers or forcibly shares a live VM value.

### Signals

Operational failures, stale capabilities and wrong ownership are returned in
`rStatus`; the instruction itself has no VM signal continuation.

### Example

<!-- rxas-example name="channel-chancancel" test="assemble" -->
```rxas
.globals=0
main() .locals=4
    chancancel r0,r1,r2,r3
    ret
```

### Related

`chanopen`, `chanstart`, `chanwait`, `chanclose`.

## `chanclose`

Close a channel using drain or cancel semantics.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x028e` | `chanclose rStatus,rChannel,rMode` | Close and release the channel lifecycle. |

### Operands And Semantics

Mode `1` drains admitted operations; mode `2` cancels them. Close is the
structured teardown boundary: provider workers, tickets and attached resources
are joined or released before successful completion. Only `rStatus` is
written.

### Signals

Invalid modes, stale handles and provider failures are status results, not VM
signals.

### Example

<!-- rxas-example name="channel-chanclose" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    chanclose r0,r1,r2
    ret
```

### Related

`chanopen`, `chanstart`, `chanwait`, `chancancel`.

## `chanopen`

Open one provider channel with explicit capability requirements.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x028a` | `chanopen rStatus,rChannel,rProviderType,rRequiredCapabilities,rConfiguration` | Return status and a channel capability. |

### Operands And Semantics

Provider type is one mutually exclusive implementation code; required
capabilities are a bit mask. Configuration is a canonical versioned RXCV
binary document. The two output registers must be distinct, though outputs may
alias inputs because the VM snapshots all inputs first. Failure leaves the
channel output at `0` and no partially opened provider resource.

### Signals

Malformed configuration, unavailable providers, unsupported capabilities and
allocation failures are returned in `rStatus`; the instruction raises no VM
signal.

### Example

<!-- rxas-example name="channel-chanopen" test="assemble" -->
```rxas
.globals=0
main() .locals=5
    chanopen r0,r1,r2,r3,r4
    ret
```

### Related

`chanstart`, `chanwait`, `chancancel`, `chanclose`.

## `chanstart`

Submit one canonical operation envelope to an open channel.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x028b` | `chanstart rStatus,rTicket,rChannel,rEnvelope,rWaitMicroseconds` | Return admission status and ticket. |

### Operands And Semantics

The envelope is a canonical RXCV binary value. A wait of `-1` waits forever,
`0` is nonblocking, and a positive value is a relative microsecond budget.
Admission and backpressure are provider-bounded. The two output registers must
be distinct; failure leaves the ticket at `0`.

### Signals

Invalid envelopes, timeouts, backpressure and lifecycle errors are status
results, not VM signals.

### Example

<!-- rxas-example name="channel-chanstart" test="assemble" -->
```rxas
.globals=0
main() .locals=5
    chanstart r0,r1,r2,r3,r4
    ret
```

### Related

`chanopen`, `chanwait`, `chancancel`, `chanclose`.

## `chanwait`

Observe the next completion from a channel.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x028c` | `chanwait rStatus,rCompletion,rChannel,rWaitMicroseconds` | Return status and canonical completion. |

### Operands And Semantics

The wait budget uses the same `-1`, `0`, or positive-relative-microseconds
contract as `chanstart`. Successful observation writes a receiver-owned RXCV
binary completion and consumes that observation. The two outputs must be
distinct; failure leaves the completion as an empty binary value.

### Signals

Would-block, timeout, stale capability and provider failure are returned in
`rStatus`; the instruction raises no VM signal.

### Example

<!-- rxas-example name="channel-chanwait" test="assemble" -->
```rxas
.globals=0
main() .locals=4
    chanwait r0,r1,r2,r3
    ret
```

### Related

`chanopen`, `chanstart`, `chancancel`, `chanclose`.

## `fclearerr`

Clear EOF and error indicators on an open C stream.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01e3` | `fclearerr rFile` | Call `clearerr((FILE *)rFile.int)`. |

### Operands And Semantics

The stream state changes; the handle register and all its payloads are
unchanged. The handle must be nonzero and valid.

### Signals

No handle validation or VM signal is provided; invalid pointers have undefined
host-library behavior.

### Example

<!-- rxas-example name="io-fclearerr" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r1,"stdin"
    load r2,"r"
    fopen r0,r1,r2
    fclearerr r0
    ret
```

### Related

`feof`, `ferror`, `fopen`.

## `fclose`

Close an open C stream and return its C-library status.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01d9` | `fclose rStatus,rFile` | Store `fclose` return code. |

### Operands And Semantics

Only `rStatus.int` changes. The stream is closed and `rFile.int` is left as a
stale pointer; it is not cleared and must not be reused.

### Signals

The C return code reports failure. Invalid pointers are not checked or signaled.

### Example

<!-- rxas-example name="io-fclose" test="run" -->
```rxas
.globals=0
main() .locals=4
    load r2,"stdin"
    load r3,"r"
    fopen r1,r2,r3
    fclose r0,r1
    ret
```

### Related

`fopen`, `fflush`.

## `feof`

Read an open stream's end-of-file indicator.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01e4` | `feof rStatus,rFile` | Store the C `feof` result. |

### Operands And Semantics

Only `rStatus.int` changes; the stream and handle register are
otherwise unchanged. Any nonzero result means EOF has been observed.

### Signals

No handle validation or VM signal is provided.

### Example

<!-- rxas-example name="io-feof" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r1,"stdin"
    load r2,"r"
    fopen r0,r1,r2
    feof r1,r0
    ret
```

### Related

`ferror`, `fclearerr`, `freadline`.

## `ferror`

Read an open stream's error indicator.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01e5` | `ferror rStatus,rFile` | Store the C `ferror` result. |

### Operands And Semantics

Only `rStatus.int` changes; the stream indicator is not cleared and the handle
register is unchanged.

### Signals

No handle validation or VM signal is provided.

### Example

<!-- rxas-example name="io-ferror" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r1,"stdin"
    load r2,"r"
    fopen r0,r1,r2
    ferror r1,r0
    ret
```

### Related

`feof`, `fclearerr`.

## `fflush`

Flush buffered output on a C stream.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01da` | `fflush rStatus,rFile` | Store the C `fflush` return code. |

### Operands And Semantics

Only `rStatus.int` changes; the stream is flushed and `rFile` remains unchanged.

### Signals

The C return code reports failure. Invalid handles are not checked or signaled.

### Example

<!-- rxas-example name="io-fflush" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r1,"stdout"
    load r2,"w"
    fopen r0,r1,r2
    fflush r1,r0
    ret
```

### Related

`fwrite`, `fclose`.

## `fopen`

Open a C stream or obtain a standard-stream handle.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01d8` | `fopen rFile,rFilename,rMode` | Store an opaque `FILE *`. |

### Operands And Semantics

Filename and mode strings are copied and NUL-terminated. Exact filenames
`stdin`, `stdout`, and `stderr` return the corresponding runtime streams;
otherwise C `fopen` is used and successful files are marked non-inheritable
across spawned commands. Only `rFile.int` changes; sources are unchanged.

### Signals

Open failure stores integer zero; it does not signal. Temporary-string
allocation failure is not translated.

### Example

<!-- rxas-example name="io-fopen" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r1,"stdin"
    load r2,"r"
    fopen r0,r1,r2
    ret
```

### Related

`fclose`, `freadline`, `fwrite`.

## `freadb`

Read up to a requested byte count into a binary payload.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01db` | `freadb rBinary,rFile,rCount` | Store bytes actually read. |

### Operands And Semantics

`rFile.int` is an open `FILE *`; `rCount.int` is the maximum byte count. The
destination binary length becomes the C `fread` count and VM-private binary
flags refresh. Sources are unchanged.

### Signals

Raises `FAILURE` for a negative count or allocation failure. Short reads and
stream errors are reflected by length/stream status rather than a VM signal.

### Example

<!-- rxas-example name="io-freadb" test="run" -->
```rxas
.globals=0
main() .locals=4
    load r1,"stdin"
    load r2,"rb"
    fopen r0,r1,r2
    load r3,0
    freadb r1,r0,r3
    ret
```

### Related

`fwriteb`, `freadbyte`, `feof`.

## `freadbyte`

Read one raw byte from a C stream.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01dd` | `freadbyte rByte,rFile` | Store `fgetc` result. |

### Operands And Semantics

`rFile.int` is an open `FILE *`. Only `rByte.int` changes; it receives `0..255`
or C `EOF` (normally `-1`). The stream position advances on a byte read.

### Signals

EOF and stream errors do not signal; inspect `feof`/`ferror`. Handles are not
validated.

### Example

<!-- rxas-example name="io-freadbyte" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    freadbyte r0,r1
    ret
```

### Related

`freadb`, `freadcdpt`, `feof`.

## `freadcdpt`

Read one UTF-8 code point from a C stream.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01de` | `freadcdpt rCodepoint,rFile` | Store code point integer and UTF-8 string. |

### Operands And Semantics

The destination string becomes the one code point and its integer payload the
Unicode scalar. At EOF integer becomes `-1` and the
string becomes empty. The stream advances by the encoded byte count.

### Signals

Raises `UNICODE_ERROR` for an invalid or truncated UTF-8 sequence. EOF does not
signal; handles are not validated.

### Example

<!-- rxas-example name="io-freadcdpt" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    freadcdpt r0,r1
    ret
```

### Related

`freadbyte`, `fwritecdpt`, `freadline`.

## `freadline`

Read one text line from a C stream without its line ending.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01dc` | `freadline rLine,rFile` | Read through LF, CRLF, CR, or EOF. |

### Operands And Semantics

The destination string is cleared and filled byte by byte.
Line terminators are consumed but omitted. The source handle is unchanged while
the stream position advances.

### Signals

Raises `FAILURE` for a zero handle and `UNICODE_ERROR` if collected bytes are
not valid UTF-8. EOF can return an empty string without signaling.

### Example

<!-- rxas-example name="io-freadline" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    freadline r0,r1
    ret
```

### Related

`readline`, `freadcdpt`, `feof`.

## `fwrite`

Write a register's complete string bytes to a C stream.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01df` | `fwrite rFile,rString` | Write the logical string byte length. |

### Operands And Semantics

The stream position advances by bytes written. Neither register nor payload is
changed; no terminator or newline is added.

### Signals

The C write result is discarded. Short writes/errors do not signal and must be
observed through stream status; handles are not validated.

### Example

<!-- rxas-example name="io-fwrite" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    load r1,"text"
    fwrite r0,r1
    ret
```

### Related

`fwriteb`, `fwritebyte`, `fflush`.

## `fwriteb`

Write a register's complete binary payload to a C stream.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01e0` | `fwriteb rFile,rBinary` | Write the logical binary byte length. |

### Operands And Semantics

The stream advances by bytes written. Registers are unchanged.

### Signals

The C write result is discarded; short writes, errors, and invalid handles are
not translated to VM signals.

### Example

<!-- rxas-example name="io-fwriteb" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    load r1,0x0011
    fwriteb r0,r1
    ret
```

### Related

`freadb`, `fwrite`, `fflush`.

## `fwritebyte`

Write the low byte of an integer payload to a C stream.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01e1` | `fwritebyte rFile,rByte` | Call `fputc(rByte.int,stream)`. |

### Operands And Semantics

The stream advances by one byte on success. Registers are unchanged;
the C library converts the integer as for `fputc`.

### Signals

The C result is discarded; invalid handles and write errors do not signal.

### Example

<!-- rxas-example name="io-fwritebyte" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    load r1,65
    fwritebyte r0,r1
    ret
```

### Related

`freadbyte`, `fwritecdpt`.

## `fwritecdpt`

Encode and write one Unicode code point to a C stream.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01e2` | `fwritecdpt rFile,rCodepoint` | Write UTF-8 bytes for the integer. |

### Operands And Semantics

The source integer is encoded into one to four UTF-8 bytes and written; the
stream advances while both registers remain unchanged.

### Signals

The code point, handle, and C write result are not validated or translated into
a VM signal.

### Example

<!-- rxas-example name="io-fwritecdpt" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    load r1,65
    fwritecdpt r0,r1
    ret
```

### Related

`freadcdpt`, `fwritebyte`.

## `getenv`

Read an environment variable by name.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01ce` | `getenv rValue,rName` | Name from a register string. |
| `0x01cf` | `getenv rValue,"NAME"` | Name from a string constant. |

### Operands And Semantics

The destination string is replaced with the environment value supplied by the
platform helper. The name register is unchanged.

### Signals

Missing-variable behavior is the platform helper's returned string; no VM
signal is raised. Allocation failure is fatal.

### Example

<!-- rxas-example name="io-getenv" test="run" -->
```rxas
.globals=0
main() .locals=1
    getenv r0,"PATH"
    ret
```

## `mtime`

Read local time-of-day as microseconds since local midnight.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01cb` | `mtime rMicroseconds` | Store local wall-clock time of day. |

### Operands And Semantics

Only the destination integer changes. The value combines local hour/minute/
second with `gettimeofday` microseconds and can jump with wall-clock adjustment.

### Signals

System-call errors are not checked or signaled.

### Example

<!-- rxas-example name="io-mtime" test="run" -->
```rxas
.globals=0
main() .locals=1
    mtime r0
    ret
```

### Related

`time`, `sleep`.

## `readline`

Read one LF-terminated line from standard input.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01c9` | `readline rLine` | Read from `stdin` through LF or EOF. |

### Operands And Semantics

The destination string length is reset and bytes before LF are stored. LF is
consumed and omitted; EOF may produce empty text.

### Signals

Invalid UTF-8 is recorded in VM-private validity flags rather than signaled;
allocation failure is fatal.

### Example

<!-- rxas-example name="io-readline" test="assemble" -->
```rxas
.globals=0
main() .locals=1
    readline r0
    ret
```

### Related

`freadline`, `say`.

## `say`

Write a value to VM standard output followed by a newline.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01c2` | `say rString` | Write register string bytes. |
| `0x01c5` | `say integer` | Format signed integer. |
| `0x01c6` | `say float` | Format float with `%.15g`. |
| `0x01c7` | `say "string"` | Write string constant. |
| `0x01c8` | `say 'c'` | Write character literal. |

### Operands And Semantics

Output uses the VM print hook and adds LF. Operands and payloads are
unchanged; the register form reads its complete logical string.

### Signals

Output errors are not translated into VM signals.

### Example

<!-- rxas-example name="io-say" test="run" -->
```rxas
.globals=0
main() .locals=0
    say "ok"
    ret
```

### Related

`sayx`, `fwrite`.

## `sayx`

Write string bytes to VM standard output without a newline.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01c3` | `sayx rString` | Write register string. |
| `0x01c4` | `sayx "string"` | Write string constant. |

### Operands And Semantics

The complete logical string is passed to the VM print hook with no terminator.
Operands are unchanged.

### Signals

Output errors are not translated into VM signals.

### Example

<!-- rxas-example name="io-sayx" test="run" -->
```rxas
.globals=0
main() .locals=0
    sayx "ok"
    ret
```

### Related

`say`, `fwrite`.

## `sockaccept`

Accept one client from a VM-managed listening socket.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0043` | `sockaccept rClient,rServer` | Store accepted client handle or error code. |

### Operands And Semantics

`rServer.int` identifies a listening socket and remains unchanged. Only
`rClient.int` is replaced; blocking behavior follows the server configuration.

### Signals

Socket-layer failures are returned as its integer result and last-error state,
not VM signals.

### Example

<!-- rxas-example name="io-sockaccept" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    sockaccept r0,r1
    ret
```

### Related

`socklisten`, `sockbind`, `sockclose`.

## `sockbind`

Bind a VM-managed socket to a host/address string and port.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0041` | `sockbind rSocket,rHost,rPort` | Bind the socket endpoint. |

### Operands And Semantics

The socket handle and port come from integer payloads; host comes from the full
string. No register is changed. The implementation discards the
socket-layer return code; inspect `sockstatus`/`sockerror` afterward.

### Signals

Socket errors are retained in socket state and are not raised as VM signals.

### Example

<!-- rxas-example name="io-sockbind" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    sockbind r0,r1,r2
    ret
```

### Related

`socknew`, `socklisten`, `socklocal`.

## `sockblocking`

Enable or disable blocking mode on a VM-managed socket.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x004b` | `sockblocking rStatus,rSocket,rEnabled` | Store socket-layer status. |

### Operands And Semantics

`rSocket.int` is the handle and `rEnabled.int` is interpreted as false/true.
Only `rStatus.int` changes; sources are unchanged.

### Signals

Failures are returned in `rStatus` and socket last-error state, not VM signals.

### Example

<!-- rxas-example name="io-sockblocking" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    sockblocking r0,r1,r2
    ret
```

### Related

`socktimeout`, `sockpending`, `socknew`.

## `sockclose`

Close a VM-managed socket handle.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x003f` | `sockclose rStatus,rSocket` | Close handle and store status. |

### Operands And Semantics

Only `rStatus.int` changes. `rSocket.int` remains as a stale handle; the VM
socket entry and native/TLS resources are released.

### Signals

Failures are returned in the status/last-error state, not VM signals.

### Example

<!-- rxas-example name="io-sockclose" test="run" -->
```rxas
.globals=0
main() .locals=2
    socknew r1
    sockclose r0,r1
    ret
```

### Related

`socknew`, `sockshutdown`.

## `sockconnect`

Connect a VM-managed TCP socket to a host and port.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0040` | `sockconnect rSocket,rHost,rPort` | Attempt a plain TCP connection. |

### Operands And Semantics

Handle and port are integer payloads; host is the complete string. Registers
are unchanged. The socket entry records status and error text.

### Signals

The socket-layer return is discarded; failures do not signal and must be read
with `sockstatus`/`sockerror`.

### Example

<!-- rxas-example name="io-sockconnect" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    sockconnect r0,r1,r2
    ret
```

### Related

`sockconnecttls`, `socknew`, `sockstatus`.

## `sockconnecttls`

Connect a VM-managed socket and establish client TLS in one operation.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0053` | `sockconnecttls rSocket,rHost,rPort` | TCP connect plus TLS handshake. |

### Operands And Semantics

The host string supplies both endpoint/SNI context and the port is an integer.
Registers are unchanged while socket state gains a TLS session on
success.

### Signals

The return is discarded; unsupported TLS or connection/handshake failures are
available through socket status/error, not VM signals.

### Example

<!-- rxas-example name="io-sockconnecttls" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    sockconnecttls r0,r1,r2
    ret
```

### Related

`sockconnect`, `sockstarttls`, `sockerror`.

## `sockerror`

Read the last error text stored for a VM-managed socket.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0051` | `sockerror rText,rSocket` | Format last error into destination string. |

### Operands And Semantics

The destination string is replaced. The handle register
and socket status are unchanged.

### Signals

Invalid handles and missing errors are represented by helper output; no VM
signal is raised. Allocation failure is fatal.

### Example

<!-- rxas-example name="io-sockerror" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    sockerror r0,r1
    ret
```

### Related

`sockstatus`, `sockconnect`.

## `sockkeepalive`

Set the native `SO_KEEPALIVE` option on a socket.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x004d` | `sockkeepalive rStatus,rSocket,rEnabled` | Store option status. |

### Operands And Semantics

Handle and Boolean enable flag come from integer payloads. Only `rStatus.int`
changes; sources remain unchanged.

### Signals

Failures are returned and recorded in socket state, not raised as VM signals.

### Example

<!-- rxas-example name="io-sockkeepalive" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    sockkeepalive r0,r1,r2
    ret
```

### Related

`socknodelay`, `sockblocking`.

## `socklisten`

Put a bound socket into listening state.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0042` | `socklisten rStatus,rSocket,rBacklog` | Store listen status. |

### Operands And Semantics

Socket handle and backlog are integer payloads. Only `rStatus.int` changes;
sources are unchanged.

### Signals

Failures are returned/recorded rather than raised as VM signals.

### Example

<!-- rxas-example name="io-socklisten" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    socklisten r0,r1,r2
    ret
```

### Related

`sockbind`, `sockaccept`.

## `socklocal`

Format a socket's local endpoint.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x004f` | `socklocal rEndpoint,rSocket` | Store local host/port text. |

### Operands And Semantics

The destination string is replaced. The handle register
and socket are unchanged.

### Signals

Helper failures are represented in output/socket state, not VM signals.

### Example

<!-- rxas-example name="io-socklocal" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    socklocal r0,r1
    ret
```

### Related

`sockpeer`, `sockbind`.

## `socknew`

Create a VM-managed IPv4/IPv6-capable TCP socket entry.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x003e` | `socknew rSocket` | Store a new handle or failure code. |

### Operands And Semantics

The VM allocates and tracks native socket state. Only the destination integer
payload changes; the handle must later be closed.

### Signals

Creation failure is represented by the returned handle/status and socket error
state, not a VM signal.

### Example

<!-- rxas-example name="io-socknew" test="run" -->
```rxas
.globals=0
main() .locals=1
    socknew r0
    ret
```

### Related

`sockclose`, `sockconnect`, `sockbind`.

## `socknodelay`

Set the native `TCP_NODELAY` option.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x004c` | `socknodelay rStatus,rSocket,rEnabled` | Store option status. |

### Operands And Semantics

Handle and Boolean flag are integer payloads. Only `rStatus.int` changes.

### Signals

Failures are returned/recorded, not raised as VM signals.

### Example

<!-- rxas-example name="io-socknodelay" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    socknodelay r0,r1,r2
    ret
```

### Related

`sockkeepalive`, `sockblocking`.

## `sockpeer`

Format a connected socket's peer endpoint.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x004e` | `sockpeer rEndpoint,rSocket` | Store peer host/port text. |

### Operands And Semantics

The destination string is replaced. Socket/handle source
is unchanged.

### Signals

Helper failures are represented by output/socket state, not VM signals.

### Example

<!-- rxas-example name="io-sockpeer" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    sockpeer r0,r1
    ret
```

### Related

`socklocal`, `sockconnect`.

## `sockpending`

Query bytes immediately available to receive from a socket.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0049` | `sockpending rCount,rSocket` | Store pending-byte count/status. |

### Operands And Semantics

Only `rCount.int` changes; the handle and socket receive buffer are unchanged.

### Signals

Failures return an integer/status and update socket error state, not VM signals.

### Example

<!-- rxas-example name="io-sockpending" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    sockpending r0,r1
    ret
```

### Related

`sockrecv`, `sockblocking`, `socktimeout`.

## `sockrecv`

Receive up to a requested byte count as UTF-8 text.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0047` | `sockrecv rString,rSocket,rMaximum` | Receive into string payload. |

### Operands And Semantics

Handle/count are integer payloads. The destination string is replaced and
received bytes are consumed from the socket; sources are unchanged.

### Signals

Invalid UTF-8 or socket failures are represented by helper result/socket state
according to the socket layer; this instruction raises no direct VM signal.

### Example

<!-- rxas-example name="io-sockrecv" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    sockrecv r0,r1,r2
    ret
```

### Related

`sockrecvb`, `socksend`, `sockpending`.

## `sockrecvb`

Receive up to a requested byte count as binary data.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0048` | `sockrecvb rBinary,rSocket,rMaximum` | Receive into binary payload. |

### Operands And Semantics

The destination binary is replaced with received bytes.
Handle/count registers remain unchanged; bytes are consumed from the socket.

### Signals

Allocation/socket failures are represented by helper/socket state rather than a
direct VM signal.

### Example

<!-- rxas-example name="io-sockrecvb" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    sockrecvb r0,r1,r2
    ret
```

### Related

`sockrecv`, `socksendb`, `sockpending`.

## `socksend`

Send a complete string payload on a connected socket.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0045` | `socksend rCount,rSocket,rString` | Store bytes sent/status. |

### Operands And Semantics

The full logical string byte range is offered to the socket. Only `rCount.int`
changes; handle and string remain unchanged.

### Signals

Short sends and failures are returned/recorded in socket state, not raised as
VM signals.

### Example

<!-- rxas-example name="io-socksend" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    socksend r0,r1,r2
    ret
```

### Related

`socksendb`, `sockrecv`.

## `socksendb`

Send a complete binary payload on a connected socket.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0046` | `socksendb rCount,rSocket,rBinary` | Store bytes sent/status. |

### Operands And Semantics

The full logical binary byte range is offered. Only `rCount.int` changes;
handle and binary payload remain unchanged.

### Signals

Short sends/failures are returned and recorded, not raised as VM signals.

### Example

<!-- rxas-example name="io-socksendb" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    socksendb r0,r1,r2
    ret
```

### Related

`socksend`, `sockrecvb`.

## `sockshutdown`

Shut down socket receive, send, or both directions.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0044` | `sockshutdown rStatus,rSocket,rMode` | Store shutdown status. |

### Operands And Semantics

Handle and platform shutdown mode are integer payloads. Only `rStatus.int`
changes; the handle remains open until `sockclose`.

### Signals

Invalid modes/handles and native failures are returned/recorded, not signaled.

### Example

<!-- rxas-example name="io-sockshutdown" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    sockshutdown r0,r1,r2
    ret
```

### Related

`sockclose`, `socksend`, `sockrecv`.

## `sockstarttls`

Upgrade an already connected socket to client TLS.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0052` | `sockstarttls rStatus,rSocket,rHost` | Store TLS-handshake status. |

### Operands And Semantics

The host string supplies TLS peer/SNI context. Only `rStatus.int` changes;
sources are unchanged and successful socket state gains TLS.

### Signals

Unsupported TLS and handshake failures return status/error text, not VM signals.

### Example

<!-- rxas-example name="io-sockstarttls" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    sockstarttls r0,r1,r2
    ret
```

### Related

`sockconnecttls`, `sockerror`.

## `sockstatus`

Read the last numeric status stored for a socket.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0050` | `sockstatus rStatus,rSocket` | Store socket-layer status. |

### Operands And Semantics

Only `rStatus.int` changes; the socket handle and stored error state remain
unchanged.

### Signals

Invalid handles are represented by the helper result, not a VM signal.

### Example

<!-- rxas-example name="io-sockstatus" test="assemble" -->
```rxas
.globals=0
main() .locals=2
    sockstatus r0,r1
    ret
```

### Related

`sockerror`, `sockconnect`.

## `socktimeout`

Set socket send/receive timeout in milliseconds.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x004a` | `socktimeout rStatus,rSocket,rMilliseconds` | Store option status. |

### Operands And Semantics

Handle and timeout are integer payloads. Only `rStatus.int` changes; source
registers remain unchanged.

### Signals

Invalid values/handles and platform failures return status, not VM signals.

### Example

<!-- rxas-example name="io-socktimeout" test="assemble" -->
```rxas
.globals=0
main() .locals=3
    socktimeout r0,r1,r2
    ret
```

### Related

`sockblocking`, `sockpending`.

## `time`

Read wall-clock seconds adjusted by the platform timezone offset.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01ca` | `time rSeconds` | Store `gettimeofday().tv_sec - timezone`. |

### Operands And Semantics

Only the destination integer changes. The result is epoch seconds shifted by
the C runtime's standard-timezone offset, not a monotonic clock, and can jump
with wall-clock adjustment.

### Signals

System-call failures are not checked or signaled.

### Example

<!-- rxas-example name="io-time" test="run" -->
```rxas
.globals=0
main() .locals=1
    time r0
    ret
```

### Related

`mtime`, `xtime`.

## `xtime`

Read one selected platform time property.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01cc` | `xtime rValue,"selector"` | Select by the first constant byte. |

### Operands And Semantics

Selectors are `Z` (C `timezone` offset seconds), `T` (process `clock()` ticks),
`C` (`CLOCKS_PER_SEC`), `N` (standard and daylight timezone names joined by
`;`), and `U` (UTC-style time-of-day microseconds computed from local fields
plus `timezone`). Integer selectors write only `rValue.int`; `N` replaces its
string. Unknown/empty selectors leave it unchanged.

### Signals

System/library failures and unknown selectors do not signal; string allocation
failure is fatal.

### Example

<!-- rxas-example name="io-xtime" test="run" -->
```rxas
.globals=0
main() .locals=1
    xtime r0,"C"
    ret
```

### Related

`time`, `mtime`.

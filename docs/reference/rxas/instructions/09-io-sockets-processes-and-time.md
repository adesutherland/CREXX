# I/O, Sockets, Processes, And Time

These instructions expose C-stream I/O, console I/O, environment and time
queries, VM-managed TCP/TLS sockets, and process redirection. File handles are
opaque `FILE *` values stored in integer payloads; socket handles are VM-managed
integers. Unless stated otherwise, low-level C-stream failures are returned or
left in stream status rather than translated into VM signals.

## `fclearerr`

Clear EOF and error indicators on an open C stream.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01e3` | `fclearerr rFile` | Call `clearerr((FILE *)rFile.int)`. |

### Operands And Semantics

The stream state changes; the handle register and all its payloads/cursors are
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

Only `rStatus.int` changes; the stream, handle register, and cursors are
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
across spawned commands. Only `rFile.int` changes; sources/cursors are unchanged.

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
destination binary length becomes the C `fread` count, its binary cursor resets
to zero, and VM-private binary flags refresh. Sources are unchanged.

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
Unicode scalar; both string cursors reset. At EOF integer becomes `-1` and the
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

The destination string is cleared, filled byte by byte, and its cursor reset.
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

The stream position advances by bytes written. Neither register, string cursor,
nor payload is changed; no terminator or newline is added.

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

The stream advances by bytes written. Registers and binary cursor are unchanged.

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

The stream advances by one byte on success. Registers and cursors are unchanged;
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
platform helper and its cursor resets. The name register is unchanged.

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

### Related

`spawn`.

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

The destination string length is reset, bytes before LF are stored, and both
string cursors reset. LF is consumed and omitted; EOF may produce empty text.

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

Output uses the VM print hook and adds LF. Operands, payloads, and cursors are
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
Operands and cursors are unchanged.

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
string. No register or cursor is changed. The implementation discards the
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
Only `rStatus.int` changes; sources and cursors are unchanged.

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

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x003f` | `{REG,REG}` | Close op2 socket handle, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockconnect`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0040` | `{REG,REG,REG}` | Connect op1 socket handle to host op2 and port op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockconnecttls`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0053` | `{REG,REG,REG}` | Connect op1 socket handle to host op2 and port op3 with client TLS |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockerror`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0051` | `{REG,REG}` | Read last socket error text for op2 into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockkeepalive`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004d` | `{REG,REG,REG}` | Set op2 socket SO_KEEPALIVE flag op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socklisten`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0042` | `{REG,REG,REG}` | Listen on op2 socket handle with backlog op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socklocal`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004f` | `{REG,REG}` | Format op2 socket local endpoint into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socknew`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x003e` | `{REG}` | Create a VM-managed TCP socket handle |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socknodelay`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004c` | `{REG,REG,REG}` | Set op2 socket TCP_NODELAY flag op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockpeer`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004e` | `{REG,REG}` | Format op2 socket peer endpoint into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockpending`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0049` | `{REG,REG}` | Query pending bytes on op2 socket handle into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockrecv`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0047` | `{REG,REG,REG}` | Receive up to op3 bytes from op2 socket handle as string into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockrecvb`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0048` | `{REG,REG,REG}` | Receive up to op3 bytes from op2 socket handle as binary into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socksend`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0045` | `{REG,REG,REG}` | Send string op3 on op2 socket handle, bytes in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socksendb`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0046` | `{REG,REG,REG}` | Send binary op3 on op2 socket handle, bytes in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockshutdown`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0044` | `{REG,REG,REG}` | Shutdown op2 socket handle mode op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockstarttls`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0052` | `{REG,REG,REG}` | Start client TLS on op2 socket handle using host op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockstatus`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0050` | `{REG,REG}` | Read last socket status for op2 into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socktimeout`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004a` | `{REG,REG,REG}` | Set op2 socket timeout in milliseconds op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `spawn`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01d2` | `{REG,REG,REG}` | Spawn Process op1 = exec op2 redirect op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `time`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01ca` | `{REG}` | Put time into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `xtime`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01cc` | `{REG,STRING}` | put special time properties into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

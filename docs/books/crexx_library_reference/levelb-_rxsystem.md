## System Primitives

`_rxsystem.crexx` supplies three low-level Level B runtime helpers. They are
support functions used by the Level B file library, not Classic Level C BIFs.

## `_exit`

```rexx
call _exit(return_code)
```

`return_code` is an `.int` and defaults to zero when omitted. `_exit` terminates
the VM process with that exact code and does not return.

### `_open`

```rexx
fileid = _open(name, mode)
```

`name` and `mode` are strings; the result is an `.int` VM file handle. Mode is
one of these lowercase bootstrap tokens:

| Mode | Cached stream behavior |
|---|---|
| `r` | read |
| `w` | append-capable output |
| `a` | append-capable output |

The `w` and `a` spellings share one cached C append mode. This is intentional:
Level B `lineout` and `charout` keep the handle open between calls and must not
truncate the stream on each call. Code that needs explicit truncation uses a
direct write-mode open, as `eraseFile` does.

Repeated opens of the same name and canonical mode return the cached handle.
Changing between read and output closes the old handle and reopens the file.
The first free cache slot is reused. A failed OS open returns zero and is not
cached.

Blank names and modes other than `r`, `w`, or `a` raise
`INVALID_ARGUMENTS`. A failure while closing for a mode change raises
`NOTREADY`.

### `_close`

```rexx
call _close(name [, nomsg])
```

`name` is a string and `nomsg` is an optional `.int` flag restricted
to 0 or 1 and defaulting to 0. This preserves the established optional-call
surface while avoiding string coercion at the Level B boundary.
The cached handle and all of its cache metadata are cleared. A blank name raises
`INVALID_ARGUMENTS`; a close failure or a name that is not open raises
`NOTREADY`. Passing 1 for `nomsg` makes a missing/already-closed
name a no-op and suppresses a native close failure. This form is used only by
cleanup paths such as `eraseFile`.

### Example

```rexx
fileid = _open("events.log", "w")
assembler fwrite fileid,"started"
call _close("events.log")
```

### Coverage and performance

`lib/rxfnsb/tests_functional/ts_rxsystem.crexx` exercises handle reuse, mode
changes, data preservation, failed-open cache behavior, typed suppression, and
the documented signals in opt and noopt modes. `ts_rxsystem_exit.crexx` checks
the exact integer process status in both modes.

The file cache remains a compact linear array because normal Level B programs
hold few simultaneous files. Searches allocate nothing, compare each stored
name once, reuse the first empty slot, canonicalize output modes before lookup,
and never retain a failed handle.

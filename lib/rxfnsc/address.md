# ADDRESS — Level C Classic BIF

The Level C `ADDRESS()` BIF is implemented independently of the Level B
ADDRESS statement protocol in `RexxClassicBifAddress.crexx`.

## Contract

```text
ADDRESS([option])
CheckArgs: oEINO
```

The optional, case-insensitive option has these results:

| Option | Result |
|---|---|
| omitted or `N` | active command-environment name |
| `I` | input `position type resource` |
| `O` | output `position type resource` |
| `E` | error `position type resource` |

The three connection fields are separated by one blank. A null resource is
still represented, so a default connection result has a trailing blank.

The default activation state is:

| State | Default |
|---|---|
| environment | `CREXX` |
| input | `INPUT NORMAL ` |
| output | `REPLACE NORMAL ` |
| error | `REPLACE NORMAL ` |

This follows the Classic contract described by the
[ANSI X3J18 draft, section 9.5.1](https://www.rexxla.org/rexxlang/standards/j18pub.pdf).

## Direct Level C API

```rexx
result = rexxclassicbif_address(reference context)
```

`context` is a `RexxBifCallContext`. Arguments and provided/omitted positions
are carried as `RexxValue` and presence arrays. The caller's
`RexxVariablePool` owns its `RexxAddressState`; a child activation can copy the
parent state with `inheritAddressState(reference parent_pool)`.

The function clears the context error, validates the call, reads only the
caller pool's ADDRESS state, and returns a new `RexxValue`. It does not call the
Level B command dispatcher and does not perform name-based BIF dispatch.

## Errors

Level C errors are recorded on the call context and the function returns a
blank `RexxValue`:

| Case | Error |
|---|---|
| a provided option is empty | `RXC-LC-40.21` |
| option is not `E`, `I`, `N`, or `O` | `RXC-LC-40.28` |
| more than one argument | `RXC-LC-40.4` |

## Compiler compatibility boundary

This library programme does not change `rxc` lowering. Current compiler output
may continue to use the deprecated `rexxclassicbif_call` compatibility
dispatcher for already-supported BIFs. Direct compiler calls and removal of
that artifact belong to a later bulk Level C lowering change. `ADDRESS()` is
therefore validated here through its direct library harness, not through newly
emitted compiler code.

## Coverage

`lib/rxfnsc/tests_functional/testRexxClassicBifAddress.crexx` calls the direct
function in optimized and unoptimized modes. It covers every option, default
and custom state, omitted arguments, state inheritance, connection formatting,
and each documented error.

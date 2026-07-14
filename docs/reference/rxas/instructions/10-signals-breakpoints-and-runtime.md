# Signals, Breakpoints, And Runtime

These instructions raise and configure VM signals, control the persistent
breakpoint interrupt, expose build information, and manage reference lifetime.
Signal names are case-sensitive string constants or, for dynamic `signal`
forms, string registers. Handler settings belong to the current frame and are
copied into child frames on entry.

## `bpoff`

Disable breakpoint delivery.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01ef` | `bpoff` | Clear the pending `BREAKPOINT` bit. |

### Operands And Semantics

There are no operands. The configured breakpoint handler is preserved, but no
further instruction-boundary breakpoint is delivered until `bpon` runs again.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="runtime-bpoff" test="run" -->
```rxas
.globals=0

main() .locals=0
    bpon
    bpoff
    ret
```

### Related

`bpon`, `sigcall`.

## `bpon`

Enable instruction-boundary breakpoint delivery, optionally installing a call
handler first.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01ed` | `bpon handler()` | Install `handler` for `BREAKPOINT` and enable delivery. |
| `0x01ee` | `bpon` | Enable delivery with the existing handler. |

### Operands And Semantics

The procedure form accepts a linked procedure constant and sets the current
frame's breakpoint response to call it. Both forms set the global pending
breakpoint bit. Breakpoints remain pending across dispatches until `bpoff`;
call handlers therefore normally execute `bpoff` or otherwise control stepping.

### Signals

The instruction itself does not signal. A procedure form must resolve during
assembly/linking; a non-executable handler later raises `FUNCTION_NOT_FOUND`.

### Example

<!-- rxas-example name="runtime-bpon" test="run" -->
```rxas
.globals=0

main() .locals=0
    bpon
    bpoff
    ret
```

### Related

`bpoff`, `sigcall`, `signal`.

## `endlife`

End the storage lifetime represented by a register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0202` | `endlife rStorage` | Invalidate references to this storage tree. |

### Operands And Semantics

References targeting `rStorage` or its nested attributes become invalid, and a
reference payload held by `rStorage` is released. Ordinary payload, attributes,
cursor, type metadata, and flags are not cleared. Compilers use this at lexical
scope exit before reusing local storage.

### Signals

This instruction does not signal. Later `deref`, `linkref`, or `setref` through
an invalidated reference raises `REFERENCE_INVALID`.

### Example

<!-- rxas-example name="runtime-endlife" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,7
    endlife r0
    ret
```

### Related

`mkref`, `refvalid`, `unref`.

## `rxhash`

Compute the conventional 32-bit FNV-1a hash of a string payload.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0201` | `rxhash rHash,rString,rLength` | Store the hash in `rHash.int`. |

### Operands And Semantics

The VM validates `rString` as UTF-8 and hashes its first `rLength` encoded bytes
in forward order with the 32-bit FNV-1a offset basis and prime. The byte count is
clamped to `0..rString.string_length`; zero or a negative value therefore hashes
the empty byte sequence. The unsigned result is returned as an integer in
`0..4294967295`. Only the destination integer payload is changed; sources and
cursors are unchanged.

The stable vectors include `2166136261` for an empty byte sequence and
`440920331` for `"abc"`.

### Signals

Raises `UNICODE_ERROR` when the source register does not contain valid UTF-8.
There is no numeric overflow signal because hashing uses unsigned arithmetic.

### Example

<!-- rxas-example name="runtime-rxhash" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"abc"
    load r2,3
    rxhash r0,r1,r2
    ret
```

### Related

`rxvers`, `strlen`.

## `rxvers`

Return a compact description of the running VM build.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01cd` | `rxvers rVersion` | Store platform, pointer width, version, and build date. |

### Operands And Semantics

The destination string becomes four space-separated fields: platform
(`linux`, `windows`, `macOS`, `cms`, or `unknown`), `32` or `64`, the cREXX
version string, and the compile date. String replacement resets its string
cursor; no source register is read.

### Signals

This instruction has no translated VM signal. Failure to allocate the returned
string is fatal.

### Example

<!-- rxas-example name="runtime-rxvers" test="run" -->
```rxas
.globals=0

main() .locals=1
    rxvers r0
    ret
```

### Related

`rxhash`.

## `sigbr`

Install a label branch as a signal handler.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01f3` | `sigbr label,"NAME"` | Branch to `label` when `NAME` is selected. |

### Operands And Semantics

The label is an assembler identifier in the current procedure; the signal name
is a string constant. Delivery unwinds to the installing frame and transfers
control to the label. No signal object is bound.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name or `KILL`, which cannot be
masked by a branch handler.

### Example

<!-- rxas-example name="runtime-sigbr" test="run" -->
```rxas
.globals=0

main() .locals=0
    sigbr caught,"OTHER"
    signal "OTHER"
caught:
    ret
```

### Related

`sigbrv`, `sigcallbr`, `signal`.

## `sigbrv`

Install a branch handler that also binds a public runtime signal object.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0222` | `sigbrv label,rSignal,"NAME"` | Bind the signal, then branch to `label`. |

### Operands And Semantics

Delivery unwinds to the installing frame, clears `rSignal`, constructs a
`rxfnsb.runtime_signal` value there, and branches. Its wrapped raw record carries
signal code, module number, instruction address, name, and payload/message.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name or `KILL`. Runtime object
allocation failure is fatal.

### Example

<!-- rxas-example name="runtime-sigbrv" test="run" -->
```rxas
.globals=0

main() .locals=1
    sigbrv caught,r0,"OTHER"
    signal "OTHER","example"
caught:
    ret
```

### Related

`sigbr`, `sigcall`, `signal`.

## `sigcall`

Install a procedure call as a signal handler.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01f4` | `sigcall handler(),"NAME"` | Call `handler` when `NAME` is selected. |

### Operands And Semantics

The handler receives one raw object argument with five attributes: signal code,
module number, instruction address, signal name, and payload/message. Returning
normally resumes after the signal point. Handler configuration is written only
to the current frame.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name or `KILL`. Delivery raises
`FUNCTION_NOT_FOUND` if the linked handler is not executable, and `FAILURE` if
the handler frame cannot be allocated.

### Example

<!-- rxas-example name="runtime-sigcall" test="run" -->
```rxas
.globals=0

main() .locals=0
    sigcall handler(),"OTHER"
    ret

handler() .locals=0
    ret
```

### Related

`sigcalla`, `sigcallbr`, `sigbr`.

## `sigcalla`

Install an action-aware procedure call as a signal handler.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x021d` | `sigcalla handler(),"NAME"` | Let `handler` choose skip, retry, or fail. |

### Operands And Semantics

The handler receives the same five-attribute raw object as `sigcall`. Its return
string is interpreted as `__rxsignal_skip` (resume after the signal point),
`__rxsignal_retry` (retry the recorded instruction), or `__rxsignal_fail`
(default panic). A missing or unknown return action also selects the panic path.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name or `KILL`.
`FUNCTION_NOT_FOUND` and `FAILURE` have the same delivery conditions as
`sigcall`.

### Example

<!-- rxas-example name="runtime-sigcalla" test="run" -->
```rxas
.globals=0

main() .locals=0
    sigcalla handler(),"OTHER"
    ret

handler() .locals=0
    ret "__rxsignal_skip"
```

### Related

`sigcall`, `signal`.

## `sigcallbr`

Install a call handler that branches to a label after the handler returns.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01f5` | `sigcallbr label,handler(),"NAME"` | Call, then continue at `label`. |

### Operands And Semantics

Delivery first unwinds to the installing frame, fixes the handler's return
address to `label`, then calls the procedure with the five-attribute raw signal
object. A normal handler return therefore continues at the label instead of
after the signal instruction.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name or `KILL`; handler resolution
and allocation can raise `FUNCTION_NOT_FOUND` or `FAILURE` during delivery.

### Example

<!-- rxas-example name="runtime-sigcallbr" test="run" -->
```rxas
.globals=0

main() .locals=0
    sigcallbr after,handler(),"OTHER"
after:
    ret

handler() .locals=0
    ret
```

### Related

`sigcall`, `sigbr`, `signal`.

## `sighalt`

Configure a signal to stop the VM with a panic diagnostic.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01f1` | `sighalt "NAME"` | Install the normal halt response. |

### Operands And Semantics

The current frame's handler entry is replaced and the corresponding native
interrupt is enabled. Delivery prints the payload/message when present,
otherwise prints the signal and source location, and returns the signal code as
the VM status. `KILL` may be configured this way.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name.

### Example

<!-- rxas-example name="runtime-sighalt" test="run" -->
```rxas
.globals=0

main() .locals=0
    sighalt "OTHER"
    ret
```

### Related

`sigshalt`, `sigignore`, `signal`.

## `sigignore`

Configure a signal to be discarded.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01f0` | `sigignore "NAME"` | Install the ignore response. |

### Operands And Semantics

The current frame's handler entry is cleared to ignore and the corresponding
native interrupt is masked. Pending ignored signals are cleared during the
interpreter's signal scan.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name or `KILL`; `KILL` cannot be
ignored.

### Example

<!-- rxas-example name="runtime-sigignore" test="run" -->
```rxas
.globals=0

main() .locals=0
    sigignore "OTHER"
    signal "OTHER"
    ret
```

### Related

`sighalt`, `sigshalt`, `sigpush`.

## `signal`

Raise a named VM signal immediately, optionally with a message or value payload.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01e6` | `signal "NAME"` | Raise a literal-name signal with an empty payload. |
| `0x01e9` | `signal "NAME","message"` | Raise with a literal message. |
| `0x01ea` | `signal "NAME",rPayload` | Raise with a copied value payload. |
| `0x021e` | `signal rName` | Read the signal name from a register string. |
| `0x021f` | `signal rName,rPayload` | Raise a dynamic-name signal with a copied payload. |

### Operands And Semantics

Register names are NUL-terminated in place if needed; their content and cursor
otherwise remain unchanged. Payload values are copied into the pending signal
object. Dispatch then applies the current frame's handler response; non-breakpoint
signals are cleared when selected.

### Signals

An unknown literal or dynamic name raises `INVALID_SIGNAL_CODE` instead. The
eventual handler may ignore, halt, return, branch, or call as configured.

### Example

<!-- rxas-example name="runtime-signal" test="run" -->
```rxas
.globals=0

main() .locals=1
    sigignore "OTHER"
    load r0,"OTHER"
    signal r0
    ret
```

### Related

`signalt`, `signalf`, `sigignore`.

## `signalf`

Raise a literal-name signal only when a register integer is zero.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01e8` | `signalf "NAME",rCondition` | Raise without a payload if false. |
| `0x01ec` | `signalf "NAME","message",rCondition` | Raise with a message if false. |

### Operands And Semantics

The integer payload of `rCondition` is tested; zero raises and nonzero falls
through. No operand or cursor is changed. A false condition uses the same
pending-signal and handler process as `signal`.

### Signals

When the condition is false, an unknown name becomes `INVALID_SIGNAL_CODE`.
When true, the name is not evaluated by the runtime and no signal is raised.

### Example

<!-- rxas-example name="runtime-signalf" test="run" -->
```rxas
.globals=0

main() .locals=1
    sigignore "OTHER"
    load r0,0
    signalf "OTHER",r0
    ret
```

### Related

`signalt`, `signal`.

## `signalt`

Raise a literal-name signal only when a register integer is nonzero.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01e7` | `signalt "NAME",rCondition` | Raise without a payload if true. |
| `0x01eb` | `signalt "NAME","message",rCondition` | Raise with a message if true. |

### Operands And Semantics

The integer payload of `rCondition` is tested; nonzero raises and zero falls
through. Operands and cursors are unchanged.

### Signals

When the condition is true, an unknown name becomes `INVALID_SIGNAL_CODE`.
When false, no signal is raised.

### Example

<!-- rxas-example name="runtime-signalt" test="run" -->
```rxas
.globals=0

main() .locals=1
    sigignore "OTHER"
    load r0,1
    signalt "OTHER",r0
    ret
```

### Related

`signalf`, `signal`.

## `sigpop`

Restore the most recently saved handler for one signal in the current frame.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0221` | `sigpop "NAME"` | Remove and restore the latest matching saved entry. |

### Operands And Semantics

The handler stack may interleave different signal names; `sigpop` searches for
the newest matching entry, restores the complete handler configuration, removes
that saved node, and reapplies its native interrupt mode.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name, `KILL`, or when no matching
saved entry exists.

### Example

<!-- rxas-example name="runtime-sigpop" test="run" -->
```rxas
.globals=0

main() .locals=0
    sigignore "OTHER"
    sigpush "OTHER"
    sighalt "OTHER"
    sigpop "OTHER"
    ret
```

### Related

`sigpush`, `sigignore`.

## `sigpush`

Save one current handler entry on the current frame's handler stack.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0220` | `sigpush "NAME"` | Push a copy of the named handler. |

### Operands And Semantics

The complete response, procedure, label, installing frame, and value-register
binding are saved. The active handler is not changed. This supports block-scoped
handler replacement; remaining saved entries are cleaned up at frame teardown.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name or `KILL`. If allocation of the
saved entry fails, the current implementation silently leaves the stack
unchanged.

### Example

<!-- rxas-example name="runtime-sigpush" test="run" -->
```rxas
.globals=0

main() .locals=0
    sigignore "OTHER"
    sigpush "OTHER"
    sigpop "OTHER"
    ret
```

### Related

`sigpop`, `sigignore`.

## `sigret`

Configure a signal to return from the frame in which it is delivered.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01f6` | `sigret "NAME"` | Install the return response. |

### Operands And Semantics

On delivery the current frame is destroyed and execution continues at its
caller's return address, as though that procedure returned without a value.
The handler entry belongs to the current frame and is inherited by later child
frames.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name or `KILL`.

### Example

<!-- rxas-example name="runtime-sigret" test="run" -->
```rxas
.globals=0

main() .locals=0
    sigret "OTHER"
    call worker()
    ret

worker() .locals=0
    signal "OTHER"
    ret
```

### Related

`sigbr`, `sigcall`, `signal`.

## `sigshalt`

Configure a signal to stop the VM successfully without a panic diagnostic.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01f2` | `sigshalt "NAME"` | Install the silent-halt response. |

### Operands And Semantics

The current frame's handler entry is replaced and the corresponding native
interrupt is enabled. Delivery terminates interpretation with status zero and
does not print the signal payload. `KILL` may be configured this way.

### Signals

Raises `INVALID_SIGNAL_CODE` for an unknown name.

### Example

<!-- rxas-example name="runtime-sigshalt" test="run" -->
```rxas
.globals=0

main() .locals=0
    sigshalt "OTHER"
    ret
```

### Related

`sighalt`, `sigignore`, `signal`.

# Native stem contract audit

## Shared handler route

The four opcodes validate their string operands and then call the shared
`rxstem_get_parts()` or `rxstem_set_parts()` implementation through
`RXSTEM_REQUIRE_RESULT`. UTF-8 rejection raises `UNICODE_ERROR`. Allocation,
capacity overflow and corrupt private representation map to `FAILURE`.
`RXSTEM_INVALID_INDEX` maps to `INVALID_ARGUMENTS` in the shared macro, but the
get/set part functions never return that result; only `STEMKEYAT` and
`STEMVALUEAT` perform indexed access.

| Opcode | UTF-8 operands | Success write | Exact signals | Failure phase |
| --- | --- | --- | --- | --- |
| `STEMGET dst,stem,key` | joined key | destination string; clear destination reference/native payload | `UNICODE_ERROR`, `FAILURE` | before logical writes |
| `STEMSET stem,key,value` | joined key and value | indirect logical stem mutation | `UNICODE_ERROR`, `FAILURE` | before logical writes |
| `STEMGET2 dst,stem,left,right` | left and right separately | destination string; clear destination reference/native payload | `UNICODE_ERROR`, `FAILURE` | before logical writes |
| `STEMSET2 stem,left,right,value` | left and right separately plus value | indirect logical stem mutation | `UNICODE_ERROR`, `FAILURE` | before logical writes |

All four depend on external mutable stem state and allocation availability.
None receives `RXOP_SIGNAL_PROP_SUCCESS_STABLE`.

## Failure atomicity and private capacity

`rxstem_set_value_string()` allocates and copies before releasing destination
payloads or changing its length, so failed GET output allocation preserves the
destination. Update SET follows the same rule for the stored value. New-entry
SET publishes the hash entry, bucket and count only after the key and value
copies succeed. Tests inject failures into lazy initialization, capacity
growth, attribute growth, key/value allocation and GET destination allocation.

Initialization and growth own a private binary/attribute representation. A
failure can retain an allocated buffer or a larger private capacity while
logical count, generation, keys, values and default stay unchanged. That is an
implementation-capacity effect, not a failure-visible language stem write.
The signal metadata therefore records no failure operand/component/context
writes but retains `RXOP_SIGNAL_DEP_EXTERNAL_STATE`.

## UTF-8 equivalence boundary

`STEMGET` validates only the already joined key. `STEMGET2` validates left and
right independently, then hashes and compares `left || "." || right` without
materializing it. Arbitrary invalid byte fragments can join into valid UTF-8,
so a general concat-to-segmented rewrite is unsound without segment validity or
failure equivalence.

The five measured RexxCPS sites satisfy the stronger condition: `"Key Bee"` is
a valid literal and the right segment is the ASCII decimal string produced by
total `ITOS`. This proves UTF-8 eligibility for those sites; the production
proof must still derive it from opcode/value provenance and fail closed for
arbitrary strings.

## Aliases and TRACE

Operand overlap is supported by the runtime helper only to the extent proven
by its copy-before-release behavior. An optimizer must still use storage and
component identities, not register-number inequality, because destination,
stem, key and value may name overlapping storage.

Every measured concat also feeds a `C` TRACE record for `acompound`. The record
is an ordered observation separate from the later native stem access. Correct
signal metadata does not authorize its deletion or movement. Each PoC must
retain the materialization for TRACE, prove an accepted T1 event rewrite, or
reject the candidate.

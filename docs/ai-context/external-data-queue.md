# Implement Classic Rexx External Data Queue (PUSH / QUEUE / PULL)

## Summary

CREXX has partial front-end recognition for the classic Rexx queue
instructions, but it does not yet provide the runtime external-data-queue
service required for complete execution of:

- `PUSH`
- `QUEUE`
- `PULL`
- `PARSE PULL`
- `QUEUED()`

This proposal implements a local external data queue first, while placing the
runtime operations behind a queue-service interface that can later be backed by
a TCP queue manager.

The existing parser/highlighter recognition must not be described as complete
runtime support. The project inventory currently records the classic queue
forms as parsed/highlighted, with broad execution incomplete; `QUEUED()` is
listed in the typed reference but has no typed callable implementation.

## Motivation

The external data queue is a standard Rexx facility used to exchange character
data between procedures and programs. Within one execution context, procedures
use the same queue service.

Typical applications include:

- producer/consumer processing;
- deferred command execution;
- temporary data exchange between procedures;
- replacing intermediate stem variables; and
- compatibility with existing Rexx applications.

Adding the local queue improves classic Rexx compatibility and provides a
portable first backend for a later external queue manager.

## Proposed Initial Implementation

The first backend should provide one local queue per CREXX execution context.
The queue belongs to the execution context rather than to an individual
procedure. Nested procedures in that context therefore observe the same queue.

The queue must not be implicitly shared with another VM execution, spawned
child process, native thread, or unrelated host context.

Conceptually:

```text
CREXX execution context
│
├── Main program
├── Procedure A
├── Procedure B
└── Queue service
    └── Local deque backend
```

The local queue is destroyed when its execution context is released.

The queue service should be an internal abstraction from the beginning:

```text
PUSH / QUEUE / PULL / QUEUED
              │
       Queue-service interface
          ┌───┴────┐
       Local      TCP
       deque    manager
```

The local deque is the initial implementation. A later TCP backend can then be
added without changing the language instruction surface.

## Queue Semantics

The queue stores character strings. Values are converted to character strings
before insertion.

| Instruction | Description |
| --- | --- |
| `QUEUE expr` | Append the value to the end of the queue. |
| `PUSH expr` | Insert the value at the front of the queue. |
| `PULL` | Remove the first queue entry, or obtain input from the host default input source when the queue is empty. |
| `PARSE PULL ...` | Obtain input using the same queue/default-input rule and apply the requested parse template. |
| `QUEUED()` | Return the current number of queued entries. |

`PUSH` and `QUEUE` with no expression should enqueue a null string, subject to
the final Level C compatibility decision.

For a nonempty queue, `PULL` removes the oldest entry at the front. `PUSH`
therefore has stack-like ordering relative to other pushed entries, while
`QUEUE` appends in FIFO order.

The implementation must document the classic case-conversion distinction:

- `PULL` has the classic uppercase behavior;
- `PARSE PULL` uses parsing semantics and must not silently apply a different
  transformation than the selected classic contract.

The exact interaction between queue entries, input records, and parse templates
must be covered by focused compatibility tests.

## Queue Service Contract

The queue service should expose operations equivalent to:

```text
push(value)
queue(value)
pull() -> value or input-needed/EOF status
queued() -> count
close()
```

The service contract should keep language semantics separate from backend
transport. In particular, the language layer should not know whether a value
came from a local deque or a remote manager.

The initial local backend should be deterministic and nonblocking with respect
to the queue itself. A `PULL` from an empty local queue follows the host default
input policy described below rather than waiting on a local condition variable.

## Internal Design

The local backend should use a double-ended queue. Conceptually it maintains:

- a head position;
- a tail position; and
- an entry count.

This permits constant-time operations:

- `PUSH` inserts before the current head;
- `QUEUE` appends after the current tail;
- `PULL` removes the current head; and
- `QUEUED()` returns the entry count.

The representation is internal and must not become part of the language or
RXBIN ABI.

## Empty Queue and Input Behaviour

When the queue is empty, `PULL` and `PARSE PULL` use the execution host's
default input source according to the configured input policy.

The implementation must define behavior for:

- interactive input;
- redirected or file input;
- end of file;
- unavailable input;
- blocking versus nonblocking host input; and
- input conversion and line termination.

The initial implementation may restrict the supported host policy, but it must
return a documented status or signal rather than leaving the result
implementation-defined. The policy should be explicit in the runtime
configuration service.

## Error and Lifecycle Behaviour

The implementation must specify and test behavior for:

- allocation failure;
- input EOF;
- queue-service failure;
- execution-context shutdown;
- a `PULL` issued after queue-service shutdown; and
- malformed or unavailable backend state.

The local queue must be released with the execution context and must not retain
references to values or host resources after teardown.

## Future Extension: External Named Queues

Independent CREXX executions run in separate operating-system address spaces,
so they cannot share the local queue directly. A future external backend may
connect to a queue manager over TCP:

```text
                 Queue manager
                      │
           ┌──────────┴──────────┐
           │                     │
       CREXX VM 1           CREXX VM 2
```

The queue manager would own named queues, while clients would identify a queue
by name and exchange framed messages over the connection.

That future design requires decisions about:

- queue-name scope and creation/open semantics;
- queue lifetime and ownership;
- message framing and character encoding;
- blocking pulls and timeouts;
- concurrent producers and consumers;
- connection loss and retry behavior;
- authentication and authorization; and
- manager restart and queue persistence.

The TCP backend should perform an application-level handshake after connecting.
A successful TCP connection alone is insufficient because another service may be
listening on the same endpoint.

External named queues are outside the initial local-backend scope. They should
be added through the queue-service interface rather than by changing the
semantics of the local deque.

## Testing Strategy

The local implementation should have focused tests covering:

- FIFO ordering for repeated `QUEUE` operations;
- LIFO ordering for repeated `PUSH` operations;
- mixed `PUSH` and `QUEUE` ordering;
- empty queue and default-input behavior;
- EOF behavior;
- `PULL` case conversion;
- `PARSE PULL` templates;
- `QUEUED()` after every mutation;
- sharing across nested procedures;
- isolation between separate execution contexts; and
- cleanup at execution termination.

The future TCP backend should additionally test:

- handshake success and protocol-version rejection;
- named-queue open/create behavior;
- concurrent clients;
- timeout and disconnect handling;
- retry behavior; and
- manager restart semantics.

## Benefits

Implementing the local external data queue provides:

- improved classic Rexx compatibility;
- a runtime basis for `PUSH`, `QUEUE`, `PULL`, `PARSE PULL`, and `QUEUED()`;
- a simple producer/consumer mechanism within one execution context;
- portable behavior across supported operating systems; and
- a clean architectural foundation for a future TCP queue manager.

The local queue establishes the standard language semantics while keeping
transport, process ownership, and external queue lifetime out of the initial
implementation.

## Repository References

- [Classic and typed syntax inventory](../../planning/release-1/component-catalogue/raw-language-syntax.md)
- [Typed intrinsic and documentation-only BIF inventory](../../planning/release-1/component-catalogue/raw-intrinsic-and-reserved-bifs.md)
- [Level C compliance reference](../../../compiler/docs/levelc_compliance_reference.md)

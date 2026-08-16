# cREXX concurrency decisions

Status: accepted design boundaries for the implemented experimental surface.

This file records durable decisions without the former internal gate names.
The dated discussion and approval trail remains in [`history/`](history/).

## User model

1. A task is independently schedulable work declared with a procedure-like
   Level G surface. Calling a task uses ordinary Rexx call syntax.
2. Independent task calls in one expression may overlap, but ordinary
   left-to-right argument evaluation, short-circuiting and operator semantics
   remain unchanged.
3. `DO PARALLEL` extends one structured task scope across statements. Ordinary
   clauses still run in the controlling execution; only task calls or explicit
   submissions create child work.
4. A task cannot detach from its owning expression or scope. Leaving a scope
   accounts for every child through completion, cancellation or failure.
5. A task's direct call to itself is synchronous recursion in the current
   worker. Blocking nested waits between distinct task callables remain
   rejected in the initial surface.
6. Task declarations, task methods, task-target expressions and `DO PARALLEL`
   are Level G-only. The explicit class and assembler layers remain Level B.

## Ownership and transfer

7. Each execution owns its mutable globals, frames, registers, references,
   ordinary objects and runtime overlays. Live VM storage never crosses an
   execution boundary.
8. Values cross as canonical `ChannelValue` documents and are materialized in
   receiver-owned storage. Transferable objects use an exact `to_channel`
   method and statically resolved `from_channel` factory.
9. Tasks are stateless invocations. Durable mutable state belongs to a
   single-owner service or actor identity whose accepted calls are serialized.
10. Large binary transfer uses explicit copy, move or immutable-seal states;
    it does not create writable shared Rexx memory.

## Layering

11. The Level G surface lowers through the public Level B pool, scope, task,
    completion, channel, value and endpoint classes.
12. Those classes reach RXVM only through `chanopen`, `chanstart`, `chanwait`,
    `chancancel` and `chanclose`. There is no public RXPA task ABI or hidden
    native-payload contract.
13. RXAS channel and ticket integers are execution-local capabilities. They
    are not transferable values or public worker identities.
14. A task target is a linker-sealed descriptor for a statically resolved task
    procedure, transferable receiver method or `.taskwork` factory. It is not a
    user-authored procedure-name string or native pointer.
15. Provider type and required capabilities are separate values. Core provider
    types use one transport-neutral instruction family; future plugins must not
    add provider-specific task opcodes.
16. Reusable bounded byte endpoints are the common redirect and streaming
    building block for child processes, HTTP and later providers.
17. HTTP remains a Rexx library over tasks, channels, endpoints and sockets. It
    is not an RXAS instruction or a special HTTP provider type.

## Deliberately absent

The initial public model does not expose OS thread creation or IDs, numeric
worker affinity, locks, condition variables, atomics, fences, arbitrary
writable shared memory, detached tasks, implicit replay of ambiguous remote
work, or `async`/`await` suspended frames.

## Decisions still open

- portable publication and compatibility level;
- the service/actor implementation behind `.taskscope.ask`;
- the open-host protocol and a public provider-plugin ABI;
- event, topic and projection libraries;
- pool saturation telemetry.

# cREXX Pipes: Proposed Layered Architecture and High-Level Implementation Plan

Status: discussion draft for an in-tree contribution project

Date: 31 August 2026

## 1. Purpose

This note brings together the current cREXX pipe experiments, the email
discussion, the existing cREXX concurrency facilities, VM/CMS Pipelines
compatibility, and the requirement for a Rexx implementation of the execution
core behind CognitivePipelines.

The recommended direction is an in-tree `contrib/crexx-pipes` project with:

- a Rexx-defined pipeline plan, stage contract, record protocol and supervisor;
- a deterministic synchronous executor;
- a task-backed executor using cREXX structured tasks and bounded endpoints;
- separate public facades for CMS-compatible pipelines, native cREXX pipelines,
  and CognitivePipelines-style graphs; and
- optional compiler-exit syntax which lowers to the same public object model.

The public surface must not expose or depend on whether the implementation uses
one execution, one task per stage, fused stages, isolated processes, or a future
resident pipeline service.

This is an architecture proposal, not an approved language-syntax proposal. The
class and method names below are illustrative and can be refined after the
execution contracts have been proved.

## 2. Requirements brought forward from the discussion

The design should provide for:

1. A `PipeStep`-like interface, with stages implemented as separate classes.
2. A record-stream abstraction rather than only whole-array transformations.
3. Sequential execution for simple and deterministic use.
4. The option for each running stage to execute in its own cREXX task.
5. Multiple named inputs and outputs, including paths such as `ok` and `bad`.
6. Transforming, filtering, expanding, aggregating and buffering stages.
7. Fan-in, fan-out, splitters, joins, controlled loops and nested pipelines.
8. Backpressure, cancellation, error propagation and deadlock detection.
9. Rexx-written user stages and a general RexxScript stage.
10. A credible VM/CMS Pipelines syntax and behaviour compatibility route.
11. A Rexx execution core capable of representing CognitivePipelines-style
    graphs, including named pins, readiness rules and LLM/RAG stages.
12. Foreground execution, background execution within a live owner, and a
    future route to resident or reconnectable pipeline runs.
13. A self-contained contribution project Peter can develop in the main CREXX
    tree without waiting for every planned cREXX I/O stream facility or
    requiring early compiler/VM changes.

## 3. Review of the current experiments

### 3.1 Checked-in `Pipe` experiment

The current [Pipe.crexx](../../experiments/Pipe/Pipe.crexx) is a useful semantic
proof of concept. It establishes a sizeable initial stage vocabulary and tests
record ordering, empty records, filtering, transformation, expansion,
aggregation, sorting and file I/O.

It is currently an eager collection transformer rather than a pipeline runtime:

- a single `Pipe` object owns the complete array of records;
- `pipe(command, ...)` is a large string-command dispatcher;
- stages mutate or replace the complete current array;
- there are no stage objects, ports, edges or immutable graph;
- there is no bounded transport, backpressure or concurrent execution; and
- there is no multi-input lifecycle, EOF protocol or deadlock model.

The existing stage algorithms and their examples remain valuable. They should
be extracted behind the new stage interface rather than discarded.

The current `WRITE` implementation also records that it does not explicitly
truncate an existing file. That behaviour should be corrected or made an
explicit output policy when the stage is carried forward.

### 3.2 Emailed `PipeStreaming` experiment

Peter's second experiment is architecturally closer to the proposed reference
executor:

- a source reads one record at a time;
- ordinary transforms emit records immediately;
- `SPLIT` can emit more than one record;
- `COUNT` consumes records and emits at end-of-stream; and
- `SORT` buffers until end-of-stream and then emits its result.

This is a sound starting point for a synchronous push executor. It demonstrates
that stages need both a record operation and an end-of-input operation.

The emailed implementation is described as synchronous, linear and single-task,
without named multiple inputs/outputs, loops, cancellation or deadlock
detection. Its source is not present in the current checkout, so this part of
the assessment is based on the email description rather than a source review.

## 4. Proposed conceptual model

```text
Main program
    |
    +-- PipelineExecutor
          |
          +-- PipelineRun / supervisor
                +-- Stage runner A
                +-- connection A -> B (passive and bounded)
                +-- Stage runner B
                +-- connection B -> C (passive and bounded)
                +-- Stage runner C
```

The important distinction is between logical components and physical tasks:

- A **stage** is a logical processing node.
- A **stage runner** executes a stage and may, depending on executor policy,
  occupy its own task.
- A **connection** is normally a passive bounded queue or endpoint. It does not
  need its own task.
- A **pipeline supervisor** owns one run and is responsible for lifecycle,
  scheduling state, completion, cancellation and deadlock detection.
- A **pipeline executor** chooses the execution strategy and starts runs.

A connection only needs an active pump task when adapting an inherently active
external facility such as a child process, socket or remote queue.

Under the current cREXX task rules, the supervisor is a logical,
controller-owned component and not necessarily another worker task. A future
durable service implementation could place that role in a resident executor
task without changing the public plan, stage or run interfaces.

### 4.1 Logical stage versus physical task

One task per stage is a good reference execution policy because it makes
blocking, buffering and CMS-like stage independence visible. It should not be a
public guarantee. Executors should be free to select among:

- **direct execution**: all stages run synchronously in one Rexx execution;
- **per-stage execution**: each stage has a runner task;
- **fused execution**: adjacent stateless stages share a runner task;
- **isolated execution**: selected stages run in process tasks; and
- **external execution**: a stage delegates to ADDRESS, a child process, HTTP,
  MQTT, a queue server, or another provider.

All policies must execute the same immutable plan and pass the same semantic
conformance suite.

## 5. Layered architecture

```text
 CMS PIPE text       Rexx builder/API       Cognitive graph/import
      |                     |                       |
      +------------- syntax and facades -----------+
                            |
                       immutable PipePlan
                            |
                 orchestration and lifecycle
                   +--------+---------+
                   |                  |
             direct executor     task executor
                   |                  |
             direct delivery    bounded endpoints
                   +--------+---------+
                       framed records
                            |
                    cREXX runtime substrate
```

### 5.1 cREXX runtime substrate

This remains part of CREXX rather than `crexx-pipes`:

- local and isolated-process structured tasks;
- task pools, scopes and completion handles;
- bounded channels and transferable byte endpoints;
- ADDRESS and structured child-process support;
- provider and plugin facilities; and
- compiler exits.

The current task surface is initial. Tasks are owned by a structured scope,
ordinary tasks cannot detach, values cross execution boundaries by value, and
a task cannot submit another task and then perform a blocking nested wait. The
controller therefore owns the dependency graph.

The current tree has bounded byte endpoints, but the broader public `rxio.*`
stream classes and reusable pipeline helpers remain future work. `crexx-pipes`
should use an internal record protocol over the facilities available now and be
adaptable to `rxio.stream` later.

### 5.2 Transport layer

Define a small transport contract independent of stage logic. Initial
implementations should be:

1. direct, in-execution delivery for the synchronous executor;
2. bounded local byte endpoints for task runners; and
3. bounded process endpoints for isolated stages where supported.

Later transports can include `rxio.stream`, named queue services, sockets or
remote pipeline links.

The transport owns capacity and delivery. It does not interpret the payload or
implement stage behaviour.

### 5.3 Record and control protocol

Record boundaries must be preserved when carried over byte endpoints. The
framing protocol should distinguish at least:

- `DATA`;
- `END` for orderly end-of-input;
- `ERROR`;
- `CANCEL`; and
- optionally `CREDIT` or equivalent flow-control information.

Control events must not be represented by magic record strings.

A data envelope should initially carry a string or binary logical record plus
optional metadata such as sequence, route and correlation identity. The CMS
profile can constrain this to ordered logical records. A Cognitive profile can
later allow richer serializable payloads without changing the transport.

This record protocol is not the same abstraction as the planned general
`rxio.stream` API:

- `.byteendpoint` is current bounded byte communication;
- `rxio.stream` is a planned general I/O abstraction; and
- a pipe record stream is a `crexx-pipes` protocol layered over a transport.

### 5.4 Stage service-provider interface

An illustrative stage contract is:

```text
descriptor()
open(context)
onRecord(inputPort, record, emitter)
onEnd(inputPort, emitter)
onCancel(reason)
close()
```

The descriptor should declare:

- named input and output ports;
- accepted and produced payload contracts;
- whether input order is preserved;
- whether the stage is stateless or stateful;
- whether it emits immediately or buffers;
- which inputs must reach EOF before final output;
- readiness rules for multiple inputs; and
- whether the stage is safe to fuse or requires isolation.

The emitter publishes records and control events to named output ports. A stage
must not know whether those ports are direct calls, local endpoints, process
endpoints or remote links.

### 5.5 Immutable graph and plan

The canonical runtime input should be an immutable `PipePlan` containing:

- node identities and stage factories;
- named ports;
- directed connections;
- capacity and ordering policies;
- error and cancellation routes;
- permitted feedback edges; and
- executor hints which do not change semantics.

Validation occurs before execution and should reject missing ports,
incompatible payload contracts, disconnected required inputs, ambiguous
producers and uncontrolled cycles.

Parsers, builders, compiler exits and CognitivePipelines importers all produce
this plan. They do not implement runtime behaviour.

### 5.6 Per-run supervisor and executor

The `PipelineExecutor` starts an immutable plan and returns a `PipelineRun`.
The run supervisor owns:

- stage-runner creation and completion;
- connection allocation and closure;
- state transitions;
- propagation of EOF, failure and cancellation;
- aggregation of diagnostics and return status;
- backpressure observations; and
- deadlock detection.

Illustrative public usage, not proposed final syntax:

```text
plan = cmsPipelines.parse("READ input | SELECT warning | WRITE output")
run = executor.start(plan)

/* Foreground use */
result = run.wait()

/* Or background use while the owner remains alive */
run = executor.start(plan)
doOtherWork()
if run.complete() then result = run.result()
else result = run.wait()
```

A `PipelineRun` surface should provide equivalents of:

- `status()`;
- `complete()`;
- `wait()`;
- `cancel(reason)`;
- `result()`;
- `error()`; and
- access to explicitly retained or collecting outputs.

### 5.7 Public facades and compatibility surfaces

Different public surfaces should lower to the same plan:

```text
.cmsPipelines       .crexxPipelines       .cognitivePipelines
       |                    |                       |
       +------------ produce PipePlan -------------+
                              |
                    common execution core
```

- `.cmsPipelines` supplies CMS syntax, abbreviations, logical-record
  conventions, standard compatibility stages and CMS-specific host adapters.
- `.crexxPipelines` supplies a native builder, named semantic ports, structured
  errors and cREXX extensions.
- `.cognitivePipelines` supplies named or typed pins, readiness policies,
  graph scopes, routers, joins and LLM/RAG-oriented stages.

A future compiler exit may provide convenient `PIPE` or `STEP` syntax, but it
should only lower that syntax to the public object model. It must not own
execution semantics, transports or stage implementations.

## 6. Foreground, background and resident execution

Three lifecycle levels should be distinguished.

### 6.1 Foreground

The caller starts a run and immediately waits. This is the simplest behaviour
and should be implemented first.

### 6.2 Background within a live owner

The caller starts a run, retains its `PipelineRun`, performs other work, and
later polls, cancels or waits. The owning task scope must remain alive. Leaving
that scope joins or cancels the run according to its explicit policy.

This is feasible with the current structured-task model, provided the
controller creates all stage tasks and retains their scope and completion
handles.

### 6.3 Resident or reconnectable execution

A pipeline which survives the submitting program, or which can later be found
by run identity from another execution, requires a durable single-owner service
or external pipeline server. Detached ordinary tasks are deliberately not part
of the current cREXX model.

The public `PipelineRun` interface can be designed so that a later resident
backend remains possible, but the first implementation must not imply that
process-independent reconnection already exists.

### 6.4 Late attachment to output

Reconnecting to lifecycle state is different from attaching a new consumer to
a data stream after records have started flowing. Late stream attachment needs
an explicit policy:

- receive live records only;
- replay all retained records;
- replay a bounded recent window;
- connect only before execution starts; or
- retrieve a collecting sink after completion.

Connections should not retain unlimited history by default. Replay and durable
history should be supplied by explicit `COLLECT`, `REPLAY`, spool or persistent
queue stages.

## 7. Scheduling, backpressure and deadlock detection

The per-run supervisor is the correct owner for scheduling state and deadlock
detection because individual stages cannot see the complete graph.

Each runner or connection should expose sufficient state for the supervisor to
know:

- whether a stage is runnable, executing or complete;
- which input a stage is waiting for;
- which output is blocked because its bounded connection is full;
- how many records are buffered;
- which producers remain open;
- whether EOF, error or cancellation is pending; and
- whether a scheduled external operation can still make progress.

A deadlock is not merely a timeout. It is a state in which:

1. the run is incomplete;
2. every remaining stage is blocked;
3. no buffered record or control event is deliverable; and
4. no open producer or pending operation can change the state.

The supervisor can then construct and report a wait-for cycle naming the stages
and ports involved.

Static validation should reject accidental graph cycles. Intentional feedback
requires an explicit feedback/delay stage, initial record, bounded iteration
rule, or another construct which defines how the cycle can first make progress
and how it terminates.

## 8. VM/ESA and z/VM CMS Pipelines compatibility

The compatibility matrix should state the reference level being tested,
including whether a behaviour comes from the historical VM/ESA environment or
the current z/VM CMS Pipelines documentation. Syntax, stage catalogues and host
facilities should not be assumed identical across versions without fixtures.

VM/CMS Pipelines compatibility should be treated as several independently
measurable dimensions:

1. **Syntax compatibility**: pipe specification, separator, arguments and
   abbreviations.
2. **Stage compatibility**: names, options and output behaviour of the standard
   stage catalogue.
3. **Execution compatibility**: logical records, ordering, EOF, return codes,
   multiple streams, fan-in/fan-out and composition.
4. **Host compatibility**: CMS commands, files, variables and other
   environment-specific services.

Matching class names or accepting CMS-looking text is not sufficient for a
compatibility claim. The core must be capable of preserving the required record
and lifecycle semantics, while the CMS facade supplies the syntax, catalogue
and host adapters.

Suggested profiles are:

### CMS Core

- ordered logical records;
- basic source, transform and sink stages;
- standard separator and argument parsing;
- compatible EOF and return-code behaviour; and
- Rexx user stages.

### CMS Multistream

- multiple named streams and connectors;
- fan-in, fan-in-any and fan-out;
- selection of stage input and output streams; and
- `CALLPIPE`-like nested composition.

### cREXX Extensions

- semantic port names such as `ok`, `bad` and `retry`;
- structured errors and cancellation;
- task and process execution policies;
- binary or typed serializable payloads; and
- HTTP, MQTT, queue, LLM and other provider adapters.

CMS-specific facilities which cannot be reproduced faithfully should be
documented as unsupported host capabilities rather than silently assigned
different semantics.

## 9. Rexx CognitivePipelines layer

The requirement is not merely to call the existing C++ CognitivePipelines
application from Rexx. The graph-execution core should be expressible in Rexx
over the common pipeline plan and stage contracts.

The Cognitive facade and library should add:

- named and optionally typed input/output pins;
- per-node readiness rules rather than only strict linear flow;
- asynchronous fan-out and controlled joins;
- routing, validation, retry and error outputs;
- loop and nested-scope constructs;
- graph save/load metadata;
- a RexxScript stage;
- LLM, retrieval, embedding and provider adapters; and
- provenance or correlation metadata where required.

The graph scheduler, plan, routing and ordinary stages can be Rexx. Native or
host adapters may still provide HTTP clients, model providers, databases or a
Qt user interface. This preserves a Rexx execution core without requiring every
external facility to be reimplemented in Rexx.

The Cognitive layer should consume the generic plan rather than creating a
second scheduler. That prevents the CMS pipeline and Cognitive graph directions
from becoming competing architectures.

## 10. In-tree `contrib/crexx-pipes` project

The work should live in the main CREXX source tree as a contribution project.
Most development concerns Rexx library contracts, stages, parsing and
conformance tests rather than compiler or VM internals, so it can remain
self-contained without becoming a separate repository.

`examples/` alone is not the right home for the implementation: this is a
reusable subsystem with its own API, executors and tests rather than a single
demonstration. The contribution should contain its own examples showing how to
consume it. Promotion into the core class library or installed product can be a
later, separately approved decision based on maturity and compatibility
evidence.

A possible layout is:

```text
contrib/crexx-pipes/
  CMakeLists.txt          contribution-local build and test entry point
  README.md               status, scope, build instructions and limitations
  src/
    api/                 public plans, stages, runs and results
    core/                validation, framing and supervision
    executors/
      direct/            deterministic synchronous executor
      task/              cREXX task and endpoint executor
    compatibility/
      cms/               parser, facade and CMS stages
    stages/
      standard/          portable record stages
      process/           ADDRESS and child-process adapters
      cognitive/         graph, LLM and RAG-oriented stages
  tests/
    contracts/           executor-independent semantic fixtures
    cms/                 compatibility examples and profiles
    deadlock/            bounded-flow and cycle diagnostics
    lifecycle/           EOF, cancellation and failure cases
  examples/
  docs/
```

The project builds against the CREXX revision which contains it. Initial
top-level build integration should be opt-in or clearly experimental so the
contribution does not silently become a supported release surface. Compiler
exit integration, new VM facilities, installation or standard-library
promotion can be added separately after the object contracts have proved
stable.

Every library fixture should exercise the full normal product path through
`rxc`, `rxas`, `rxlink` and `rxvm`. Executor-independent contract fixtures
should run against both the direct and task executors and, where applicable,
both VM implementations.

## 11. High-level implementation plan

Each phase has an explicit proof point. Later phases should not require stages
written for earlier phases to be redesigned.

### Phase 0: Confirm scope and semantic contracts

Deliverables:

- establish the `contrib/crexx-pipes` scaffold, status and build boundary;
- agree terminology for plan, run, stage, port, record and control event;
- record the CMS compatibility profiles and initial exclusions;
- decide the initial record payload contract;
- identify representative stages for each behaviour category; and
- establish executor-independent contract fixtures.

Proof point:

- the same fixtures can describe map, filter, split, aggregate and EOF-buffering
  behaviour without referring to a particular executor.

### Phase 1: Extract stage interfaces and synchronous reference executor

Deliverables:

- define the illustrative `PipeStep`, context and emitter interfaces;
- separate four representative stages from the monolithic `Pipe` class:
  transform, filter, split and sort;
- implement explicit `DATA`, `END`, `ERROR` and `CANCEL` events; and
- implement a deterministic linear synchronous executor.

Proof point:

- existing examples produce equivalent records, while `SORT` proves buffering
  and EOF finalization and `SPLIT` proves one-to-many emission.

### Phase 2: Immutable graph and named ports

Deliverables:

- implement `PipePlan`, nodes, named ports and connections;
- validate plans before execution;
- add fan-out and deterministic fan-in;
- add named success and failure routes; and
- specify readiness rules for multiple inputs.

Proof point:

- one plan demonstrates branching to `ok` and `bad`, fan-out to two consumers,
  and a join which completes only under its declared input policy.

### Phase 3: Run supervisor and complete lifecycle

Deliverables:

- implement `PipelineRun` state and results;
- implement foreground wait, background polling and cancellation within an
  owning execution;
- define EOF closure for single and multiple producers;
- propagate structured failures; and
- add static cycle validation and synchronous quiescence diagnostics.

Proof point:

- success, stage failure, cancellation, early consumer close and an invalid
  cycle all terminate deterministically with useful diagnostics.

### Phase 4: Task and bounded-endpoint executor

Deliverables:

- define the binary framing of records and control events;
- allocate bounded endpoints for graph edges;
- submit stage runners from the controller-owned scope;
- implement per-stage task execution initially;
- report runner and connection state to the supervisor; and
- implement wait-for-graph deadlock detection.

Proof point:

- the Phase 1-3 contract suite passes unchanged against both executors, and a
  deliberately blocked cyclic graph reports the responsible stages and ports.

### Phase 5: CMS Core compatibility facade

Deliverables:

- parse a documented CMS Core subset into `PipePlan`;
- implement compatible stage names, options and abbreviations;
- add Rexx user-stage adaptation;
- retain examples derived from IBM documentation; and
- publish a compatibility matrix separating syntax, stages, execution and host
  facilities.

Proof point:

- the declared CMS Core examples produce the documented record sequences and
  return behaviour. Anything not tested is described as unverified or
  unsupported rather than compatible.

### Phase 6: CMS Multistream and composition

Deliverables:

- connectors and named streams;
- fan-in, fan-in-any and fan-out stages;
- `CALLPIPE`-like nested plan composition;
- explicit feedback/delay constructs; and
- further compatibility stages prioritised from real use cases.

Proof point:

- multi-input, multi-output and nested-pipeline fixtures pass under both
  executors without CMS-specific logic entering the core supervisor.

### Phase 7: Rexx CognitivePipelines demonstrator

Deliverables:

- a Cognitive facade over `PipePlan`;
- typed or schema-described named pins;
- readiness, routing, retry and join stages;
- RexxScript execution;
- graph save/load representation; and
- a small LLM/RAG workflow using replaceable provider adapters or test doubles.

Proof point:

- a non-linear Cognitive graph runs through the same supervisor and executors,
  with no second graph scheduler.

### Phase 8: Optimisation and extended execution policies

Deliverables:

- safe fusion of adjacent stateless stages;
- selection of local-task versus process-task execution;
- process, ADDRESS, HTTP, MQTT and queue adapters as priorities require;
- measurement of scheduling and buffering costs; and
- executor hints which preserve observable semantics.

Proof point:

- optimised policies pass the unchanged semantic suite and show a measured
  benefit on representative workloads.

### Phase 9: Resident pipeline service, if approved

This phase depends on an approved durable cREXX service model or an external
pipeline server. It is intentionally not part of the initial implementation.

Possible deliverables:

- stable pipeline run identity;
- reconnectable lifecycle status;
- persistent queues or spools;
- owner failure and restart policy; and
- authentication and resource controls for remote clients.

Proof point:

- a submitting program can terminate and an authorised later client can
  inspect, wait for or cancel the same run without relying on detached ordinary
  tasks.

## 12. Decisions to make before Phase 1

The architecture does not require final language syntax, but the following
contract decisions should be made early:

1. Whether the first data envelope contains only strings or supports binary
   payloads immediately.
2. The exact EOF rule for a port with multiple producers.
3. Whether errors are always control events, named data outputs, or both under
   an explicit stage policy.
4. The minimum readiness rules for multi-input stages.
5. The default connection capacity and whether it is records, bytes, or both.
6. The initial CMS Core stage set and abbreviation rules.
7. The allowed semantics of intentional cycles.
8. The ownership rule for retained outputs and late attachment.
9. Whether `contrib/crexx-pipes` participates in the default top-level build
   immediately or remains an explicitly enabled contribution until its first
   contract milestone passes.

## 13. Recommended immediate next step

Peter's streaming experiment should become the direct reference executor, but
only after extracting the stage contract and the four representative stages.
The first milestone should not introduce task execution or new syntax.

The milestone is complete when one immutable linear plan can execute transform,
filter, split and sort stages through explicit record and EOF events, with the
stage code unaware of how delivery occurs. That gives the task executor, CMS
facade and Cognitive layer a stable foundation rather than three separate
implementations.

## 14. References

Repository material:

- [Current Pipe experiment](../../experiments/Pipe/Pipe.crexx)
- [Current Pipe experiment notes](../../experiments/Pipe/pipe.md)
- [cREXX concurrency roadmap](../../docs/ROADMAP.md)
- [cREXX I/O and pipe working plan](../../docs/ai-context/CREXX_IO_PIPE_WORKING.md)
- [cREXX concurrent programming guide](../../docs/books/crexx_programming_guide/concurrency.md)
- [cREXX compiler exit protocol](../../compiler/docs/exit_protocol_v2.md)

External compatibility references:

- [IBM: Basic Concepts and Functions of CMS Pipelines](https://www.ibm.com/docs/en/zvm/7.4.0?topic=pipelines-basic-concepts-functions-cms)
- [IBM: PIPE command](https://www.ibm.com/docs/en/zvm/7.4.0?topic=commands-pipe)
- [IBM: Writing Your Own Stage Commands](https://www.ibm.com/docs/en/zvm/7.4.0?topic=pipelines-writing-your-own-stage-commands)
- [XFL CMS Pipelines workalike](https://github.com/trothtech/xfl)
- [NetRexx pipeline stage library](https://github.com/RexxLA/NetRexx/tree/master/src/org/netrexx/njpipes/stages)

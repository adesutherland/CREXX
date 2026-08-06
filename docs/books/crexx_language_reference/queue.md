# Queue, Push and Pull

The current implementation supports the following language features:

- `PUSH`
- `QUEUE`
- `PULL`
- `PARSE PULL`

The queue is local to the current cRexx execution and is shared by the main program and all nested procedures.

At this stage, the implementation does not support[^1]:

- named queues;
- externally managed queues;
- queue sharing between independent cRexx executions.

`PARSE PULL` has been integrated into the existing `PARSE` implementation and behaves as expected.

An example:

```rexx <!--queuexample.crexx-->
options levelb

main: procedure
  first = .string
  second = .string
  parsed_first = .string
  push "front"
  queue "back"
 
  pull first
  pull second

  say first
  say second

  queue "classic-value"
  parse pull parsed_first

  say parsed_first
```

<!--splice--crexx queuexample-->


[^1]: A future enhancement may introduce externally managed named queues. One possible approach would be to implement a lightweight TCP-based queue manager, allowing independent CREXX executions to access the same named queues without changing the Rexx language interface.


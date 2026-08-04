# Classic Rexx Data Queue Functions

## Overview

CREXX provides the classic Rexx data queue through the following functions:

* `PUSH`
* `QUEUE`
* `PULL`
* `QUEUED`

The data queue stores character strings and is shared by the running CREXX execution and all nested procedures.

The queue is independent of the procedure variable pool, allowing procedures to exchange data without using exposed variables or stems.

---

# PUSH

## Syntax

```rexx
call push expression
```

## Description

`PUSH` inserts a string at the **front** of the data queue.

Items added using `PUSH` are returned before items added with `QUEUE`.

This provides **Last-In, First-Out (LIFO)** behaviour.

## Example

```rexx
call push "One"
call push "Two"

say pull()
say pull()
```

Output:

```text
Two
One
```

---

# QUEUE

## Syntax

```rexx
call queue expression
```

## Description

`QUEUE` appends a string to the **end** of the data queue.

This provides **First-In, First-Out (FIFO)** behaviour.

## Example

```rexx
call queue "One"
call queue "Two"

say pull()
say pull()
```

Output:

```text
One
Two
```

---

# PULL

## Syntax

```rexx
value = pull()
```

## Description

`PULL` removes and returns the first string from the data queue.

If the queue is empty, an empty string is currently returned.

Future releases may optionally provide the classic Rexx behaviour of reading from the program's standard input when the queue is empty.

## Example

```rexx
call queue "Hello"

text = pull()

say text
```

Output:

```text
Hello
```

---

# QUEUED

## Syntax

```rexx
count = queued()
```

## Description

Returns the current number of entries stored in the data queue.

## Example

```rexx
call queue "One"
call queue "Two"

say queued()
```

Output:

```text
2
```

---

# Queue Behaviour

The queue combines both stack and queue semantics.

| Operation | Behaviour                     |
| --------- | ----------------------------- |
| `PUSH`    | Insert at the front (LIFO)    |
| `QUEUE`   | Append at the end (FIFO)      |
| `PULL`    | Remove from the front         |
| `QUEUED`  | Return the current queue size |

For example:

```rexx
call queue "A"
call queue "B"
call push  "C"

do while queued() > 0
    say pull()
end
```

Output:

```text
C
A
B
```

---

# Scope

The data queue belongs to the current CREXX execution.

It is shared by:

* the main program;
* all nested procedures;
* all functions executing within the same CREXX execution.

The queue is created automatically when a CREXX execution starts and is destroyed when that execution terminates.

Future CREXX releases may support externally managed named data queues shared between independent CREXX executions without changing the language interface.

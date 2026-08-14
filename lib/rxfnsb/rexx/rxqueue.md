# Classic Rexx Data Queue Functions

## Overview

CREXX provides the classic Rexx data queue through the following functions:

- `PUSH`
- `QUEUE`
- `PULL`
- `PEEK`
- `QUEUED`

The data queue stores character strings and is shared by the running CREXX execution and all nested procedures.

The queue is independent of the procedure variable pool, allowing procedures to exchange data without using exposed variables or stems.

When the data queue is empty, `PULL` automatically reads one line from the default input stream, matching classic Rexx behaviour.

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

If one or more entries are available in the queue, the first queued string is returned and removed from the queue.

If the queue is empty, `PULL` reads and returns one line from the default input stream (typically standard input).

This behaviour is compatible with classic Rexx and allows queued data and interactive input to be used transparently.

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

If the queue is empty:

```rexx
text = pull()
```

the user is prompted for one line of input, and the entered line is returned by `PULL`.

---

# PEEK

## Syntax

```rexx
value = peek()
value = peek(position)
```

## Description

`PEEK` returns a queued string without removing it.

Without an argument, `PEEK()` returns the first queued entry, which is the same value that the next `PULL()` would return.

With a positive `position`, `PEEK(position)` returns the corresponding entry in **PULL order**:

- `PEEK(1)` returns the next entry.
- `PEEK(2)` returns the entry after that.
- `PEEK(3)` returns the third entry, and so on.

The numbering is independent of whether entries were added using `PUSH` or `QUEUE`. It always describes the order in which `PULL` would retrieve them.

If `position` is less than `1` or greater than the current queue size, `PEEK` returns an empty string.

Unlike `PULL`, `PEEK` does **not** read from the default input stream when the queue is empty.

## Example

```rexx
call queue "job:alpha"
call queue "job:beta"
call push  "job:urgent"

say peek()
say peek(1)
say peek(2)
say peek(3)
say peek(4)
say queued()
```

Output:

```text
job:urgent
job:urgent
job:alpha
job:beta

3
```

The queue remains unchanged.

A subsequent:

```rexx
say pull()
```

returns:

```text
job:urgent
```

## Boundary Behaviour

For a queue containing three entries:

```rexx
say "'" || peek(-1) || "'"
say "'" || peek(0)  || "'"
say "'" || peek(1)  || "'"
say "'" || peek(2)  || "'"
say "'" || peek(3)  || "'"
say "'" || peek(4)  || "'"
```

the result is:

```text
''
''
'job:urgent'
'job:alpha'
'job:beta'
''
```

`PEEK()` is equivalent to `PEEK(1)`.
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

| Operation | Behaviour |
|-----------|-----------|
| `PUSH` | Insert at the front (LIFO) |
| `QUEUE` | Append at the end (FIFO) |
| `PULL` | Remove and return the first queued entry; if the queue is empty, read one line from the default input stream |
| `PEEK` | Return the first queued entry without removing it; if the queue is empty, return an empty string |
| `QUEUED` | Return the current queue size |

For example:

```rexx
call queue "A"
call queue "B"
call push "C"

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

- the main program;
- all nested procedures;
- all functions executing within the same CREXX execution.

The queue is created automatically when a CREXX execution starts and is destroyed when that execution terminates.

When the queue becomes empty, subsequent `PULL` operations read from the default input stream instead of the queue.

Future CREXX releases may support externally managed named data queues shared between independent CREXX executions without changing the Rexx language interface.

## Named Queues

Each execution also provides a named queue manager. `SESSION` is created and
selected automatically, so existing programs continue to use the standard queue
functions unchanged.

Queue names are managed independently from queue entries. `PUSH`, `QUEUE`,
`PULL`, `PEEK`, and `QUEUED` do not take a queue-name argument; they always
operate on the queue selected by `RXQUEUE SET`.

```rexx
rc = rxqueue("CREATE", "WORK")
rc = rxqueue("SET", "WORK")
call queue "A"
call push "B"
say "queue=" rxqueue("QUERY") "size=" queued()
say "queues=" rxqueue("LIST")
say rxqueue("INFO", "WORK")
rc = rxqueue("CLEAR", "WORK")
say "first=" pull()
rc = rxqueue("EXPORT", "WORK", "work.queue")
rc = rxqueue("IMPORT", "ARCHIVE", "work.queue")
rc = rxqueue("SET", "SESSION")
rc = rxqueue("DELETE", "WORK")
```

The supported operations are:

| Operation | Result |
|-----------|--------|
| `CREATE`, name | Creates an empty named queue. |
| `SET`, name | Selects an existing queue for the standard queue functions. |
| `QUERY` | Returns the selected queue name. |
| `LIST` | Returns all queue names, separated by blanks. |
| `INFO`, name | Returns `NAME=... QUEUED=... CURRENT=...` for the named queue. If the name is omitted, reports the selected queue. |
| `CLEAR`, name | Removes all entries from the named queue without deleting it. If the name is omitted, clears the selected queue. |
| `DELETE`, name | Deletes a named queue. Deleting the selected named queue returns selection to `SESSION`. |
| `EXPORT`, name, path | Writes the queue to a UTF-8 text file without draining it. |
| `IMPORT`, name, path | Replaces or creates the named queue from a UTF-8 text file. |

`CREATE`, `SET`, `CLEAR`, `DELETE`, `EXPORT`, and `IMPORT` return string status `"0"`
on success and `"4"` for invalid arguments, duplicate or missing queues, a
protected queue, or a file operation failure. `SESSION` is always present and
cannot be deleted. Queue names are case-insensitive and are stored in
uppercase.

`LIST` returns the queue names separated by blanks. `INFO` returns an empty
string for a missing queue. The `CURRENT` field is `1` for the selected queue
and `0` otherwise.

Export files contain one queue entry per UTF-8 text line, in the order that
`PULL` would return them. Import replaces the destination contents; it does not
append to an existing queue. Empty queue entries are preserved as empty lines.

For a complete example using `SESSION`, `WORK`, and `REPORT` as a small
execution-local job pipeline, see
[`examples/rxqueue_named.crexx`](../../../examples/rxqueue_named.crexx).

Queue instances and their contents belong only to the current CREXX
execution. They are not shared with another process and require no external
queue service.

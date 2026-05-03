# Process API Reference (REXX Binding)

> **Version:** 2025-10-16  
> **Platforms:** Windows and Linux/UNIX  
> **Purpose:** Handle-based API to create and control external processes from REXX.

---

## 0. Processes vs. Threads (Subtasks)

### 0.1 Overview

In operating systems, **processes** and **threads** represent two fundamental execution models.  
Both enable concurrent activity, but they differ in their **resource isolation**, **lifecycle**, and **communication mechanisms**.

This distinction is critical when designing interfaces such as this Process API, which intentionally operates at the **process level**, not at the thread (or subtask) level.

---

### 0.2 Processes

A **process** is an independent program instance with its own:
- **Address space** (private memory)
- **File descriptors / handles**
- **Environment variables**
- **Program counter and stack**

Each process is protected from others by the operating system.  
Communication between processes therefore requires explicit **inter-process communication (IPC)** mechanisms, such as pipes, sockets, or shared memory.

Creating a process is comparatively **expensive** (in time and system resources), but offers **strong isolation** and **fault containment**.  
If one process fails, it does not directly corrupt the memory or state of another.

**Typical use cases:**
- Running independent commands or tools.
- Executing user programs from a script.
- Isolating untrusted or long-running components.

---

### 0.3 Threads (Subtasks)

A **thread** (called a *subtask* in MVS and z/OS terminology) is a **lightweight execution path within a process**.  
Multiple threads share the same:
- **Memory space**
- **File handles**
- **Global variables and heap**

Each thread has its own:
- **Stack**
- **Program counter**
- **Execution context**

Threads communicate by directly accessing shared variables or using synchronization primitives (mutexes, semaphores, events).  
Creating or switching threads is **faster** than creating processes, but shared memory introduces **risk of interference**: a faulty thread can corrupt data used by others in the same process.

**Typical use cases:**
- Concurrent I/O handling.
- Background computation.
- Parallelism within the same program.

---

### 0.4 Practical Distinction

| Feature | Process | Thread (Subtask) |
|----------|----------|-----------------|
| Memory space | Separate | Shared |
| Failure isolation | High (independent) | Low (shared state) |
| Communication | IPC (pipes, sockets, etc.) | Shared memory / synchronization |
| Creation overhead | Higher | Lower |
| Typical use | Separate program | Parallel tasks within one program |
| OS-level resource | Process handle / PID | Thread handle / TID |
| In MVS / z/OS terms | Independent address space | Subtask in the same address space |

---

### 0.5 Implications for the Process API

The **Process API** operates entirely at the *process* level.  
Each call to `processcreate()` launches a separate operating system process with its own memory and environment.  
This design provides:
- Simplicity (no need for shared-state synchronization)
- Robustness (a failed child process cannot corrupt the parent)
- A unified, cross-platform interface for both Windows and UNIX systems.

If future enhancements include a **thread-based** API, it would complement this one by enabling in-process concurrency (true subtasks) rather than independent execution contexts.

---
# Process API Reference (REXX Binding)

> **Version:** 2025-10-16  
> **Platforms:** Windows and Linux/UNIX  
> **Purpose:** Handle-based API to create and control external processes from REXX.

---

## 1. Overview

This document describes a handle-based process API exposed to REXX. The API supports process creation, non-blocking output capture (line and chunk modes), stdin streaming, waiting and termination controls, timeouts via watchdog, and lifecycle management. All functions are registered as `process*` procedures in REXX.

The API is designed to be minimal and deterministic. Each created process is tracked in an internal table of fixed size (`MAX_PROCS`). The returned **handle** identifies the process in all subsequent calls.

---

## 2. Architecture Summary

- **Creation:** Processes are started with `processcreate(cmdline, capture_output)`.
- **Pipes:** When `capture_output = 1`, stdout is connected to a pipe for reading. Stdin is connected to a pipe for writing by the parent. Standard error is merged into stdout.
- **Buffers:** Each process maintains an internal bounded buffer for stdout. `processreadoutput` retrieves complete lines; `processreadchunk` returns arbitrary data.
- **Watchdog:** `processsettimeout` enables a periodic check that terminates processes exceeding the configured runtime.
- **Handles:** Process objects are stored in a table. Functions receive a handle (integer) and operate on the associated process.

---

## 3. REXX Function Reference

All functions are synchronous from the caller’s perspective. Reading functions are non-blocking with respect to output availability and return an empty string when no data is available at the moment of the call.

### 3.1 `processcreate(cmdline=.string, capture_output=.int) -> .int`

Create a new process.

- **Parameters**
    - `cmdline` — The command line to execute (e.g., `'cmd /C dir'`, `'ls -l'`).
    - `capture_output` — `1` to capture stdout (and merged stderr); `0` to inherit the parent console.
- **Returns**  
  A positive integer **handle** on success; a negative value on failure.
- **Notes**
    - When `capture_output = 1`, the child’s stdout is readable via `processreadoutput`/`processreadchunk`.
    - Always check for negative return values.

**REXX example**
```rexx
pid = processcreate('cmd /C echo Hello', 1)
if pid < 0 then do
  say 'Create failed:' pid
  exit 1
end
call processfree pid
```

---

### 3.2 `processisrunning(handle=.int) -> .int`

Query whether a process is still active.

- **Returns**: `1` if running; `0` if exited.

**REXX example**
```rexx
do while processisrunning(pid)
  call processwait pid, 100
end
```

---

### 3.3 `processwait(handle=.int, timeout_ms=.int) -> .int`

Wait for completion up to the specified timeout.

- **Parameters**
    - `timeout_ms` — Maximum wait in milliseconds. `0` waits indefinitely.
- **Returns**
    - `0` if the process exited within the timeout.
    - `-1` if the timeout elapsed and the process is still running.

**REXX example**
```rexx
rc = processwait(pid, 5000)   /* wait up to 5 seconds */
if rc = -1 then say 'Process still running after 5s'
```

---

### 3.4 `processkill(handle=.int) -> .int`

Terminate the process immediately.

- **Returns**: `0` on success; negative on error.
- **Note**: This does not free the handle. Call `processfree` afterwards.

**REXX example**
```rexx
call processkill pid
call processfree pid
```

---

### 3.5 `processgetexitcode(handle=.int) -> .int`

Obtain the process exit code if available.

- **Returns**
    - `>= 0` — Exit code.
    - `-1` — Still running.
    - `-2` — Error obtaining the exit code.

**REXX example**
```rexx
code = processgetexitcode(pid)
if code >= 0 then say 'Exit code:' code
```

---

### 3.6 `processreadoutput(handle=.int) -> .string`

Read a single complete line from stdout. Line endings are removed.

- **Returns**: A string without trailing CR/LF; an empty string if no complete line is available at the moment of the call.

**REXX example**
```rexx
do forever
  line = processreadoutput(pid)
  if line = '' then do
    call processwait pid, 100
    if processgetexitcode(pid) \= -1 then leave
    iterate
  end
  say 'OUT>' line
end
```

---

### 3.7 `processreadchunk(handle=.int, maxbytes=.int) -> .string`

Read up to `maxbytes` bytes from stdout. No line parsing is performed.

- **Parameters**
    - `maxbytes` — Maximum bytes to return in a single call. If omitted or invalid, a default (e.g., 4096) is applied.
- **Returns**: A string of length `0..maxbytes`; empty if no data is available at the moment of the call.

**REXX example**
```rexx
do while processisrunning(pid)
  chunk = processreadchunk(pid, 8192)
  if chunk \= '' then say 'Read' length(chunk) 'bytes'
  call processwait pid, 50
end
```

---

### 3.8 `processpeekoutput(handle=.int) -> .string`

Return current buffered stdout **without consuming** it.

- **Returns**: A snapshot string; empty if buffer is currently empty.

**REXX example**
```rexx
peek = processpeekoutput(pid)
if peek \= '' then say 'Buffer preview:' peek
```

---

### 3.9 `processgetoutput(handle=.int) -> .string`

Return the entire internal stdout buffer as-is. This call does not clear the buffer or advance cursors. Intended for debugging or bulk reads.

**REXX example**
```rexx
data = processgetoutput(pid)
say 'Buffered bytes:' length(data)
```

---

### 3.10 `processsendinput(handle=.int, data=.string) -> .int`

Write data to the child’s stdin.

- **Returns**: Number of bytes written; negative on error.
- **Notes**: For text protocols on Windows, append `"0d0a"x` if the child expects CRLF.

**REXX example**
```rexx
call processsendinput pid, 'hello' || "0a"x
```

---

### 3.11 `processcloseinput(handle=.int) -> .int`

Close the child’s stdin to signal EOF.

- **Returns**: `0` on success; negative on error.
- **Note**: Required for many programs to complete processing.

**REXX example**
```rexx
call processcloseinput pid
```

---

### 3.12 `processwaitkill(handle=.int, timeout_ms=.int) -> .int`

Wait for completion up to a timeout; if still running, terminate the process.

- **Returns**
    - `0` — Process ended normally within `timeout_ms`.
    - `-1` — Wait failed or invalid handle.
    - `-2` — Process was terminated by this call.

**REXX example**
```rexx
rc = processwaitkill(pid, 10000)
select
  when rc = 0 then say 'Completed within time'
  when rc = -2 then say 'Terminated by waitkill'
  otherwise say 'Wait failed'
end
```

---

### 3.13 `processsettimeout(handle=.int, timeout_ms=.int) -> .int`

Configure a per-process timeout enforced by a watchdog thread.

- **Behavior**: The watchdog inspects processes periodically and kills any that exceed `timeout_ms` since creation. The watchdog is started automatically.
- **Returns**: `0` on success; negative on error.


**REXX example**
```rexx
call processsettimeout pid, 15000   /* 15 seconds */
```

---

### 3.14 `processlist() -> .int[]`

Return an array of active process handles.

**REXX example**
```rexx
handles = processlist()
do i = 1 to handles.0
  say 'Handle:' handles.i
end
```

---

### 3.15 `processfree(handle=.int) -> .int`

Release all resources associated with a handle.

- **Returns**: `0` on success. Safe to call on already-freed handles.

**REXX example**
```rexx
call processfree pid
```

---

### 3.16 `processfreeall() -> .int`

Release all active processes and clear the table.

- **Returns**: Count of freed entries.

**REXX example**
```rexx
n = processfreeall()
say 'Released:' n 'processes'
```

---

### 3.17 `processlasterror(handle=.int) -> .string`

Return the last recorded error message for the process. Returns `"n/a"` if none is available.

**REXX example**
```rexx
msg = processlasterror(pid)
if msg \= 'n/a' then say 'Last error:' msg
```

---

### 3.18 `processstackinfo() -> .int`

Print runtime stack information for diagnostics (platform-specific). Return value is informational (`0` on success).

**REXX example**
```rexx
call processstackinfo
```

---

### 3.19 `wait(wait_ms=.int) -> .int`

Sleep for the specified number of milliseconds. Returns `0`.

**REXX example**
```rexx
call wait 250
```

---

## 4. Example Scripts

### 4.1 Line-Oriented Processing
```rexx
pid = processcreate('cmd /C ping 127.0.0.1 -n 3', 1)
do forever
  line = processreadoutput(pid)
  if line = '' then do
    call processwait pid, 200
    if processgetexitcode(pid) \= -1 then leave
    iterate
  end
  say 'PING>' line
end
say 'Exit code:' processgetexitcode(pid)
call processfree pid
```

### 4.2 Chunk Reading (Binary-Friendly)
```rexx
pid = processcreate('cmd /C type my.bin', 1)  /* use "cat my.bin" on POSIX */
do while processisrunning(pid)
  chunk = processreadchunk(pid, 16384)
  if chunk \= '' then say 'Collected' length(chunk) 'bytes'
  call processwait pid, 50
end
call processfree pid
```

### 4.3 Interactive Stdin/Stdout
```rexx
pid = processcreate('cmd /C more', 1)   /* POSIX: "cat" */
call processsendinput pid, 'Line A' || "0a"x
call processsendinput pid, 'Line B' || "0a"x
call processcloseinput pid

do forever
  line = processreadoutput(pid)
  if line = '' then do
    call processwait pid, 100
    if processgetexitcode(pid) \= -1 then leave
    iterate
  end
  say 'ECHO>' line
end
call processfree pid
```

### 4.4 Enforce Time Budget
```rexx
pid = processcreate('cmd /C timeout /T 30 /NOBREAK', 1)  /* POSIX: "sleep 30" */
call processsettimeout pid, 5000                          /* 5 seconds */
rc = processwaitkill(pid, 6000)
if rc = -2 then say 'Process terminated due to timeout'
call processfree pid
```

### 4.5 Enumerate and Cleanup
```rexx
h = processlist()
do i = 1 to h.0
  say 'Active handle:' h.i
end
n = processfreeall()
say 'Freed:' n 'entries'
```
### 4.6 Interactive Conversation with a Regina REXX Child (via CREXX Process API)

This sample demonstrates an interactive, line-by-line conversation between a **CREXX parent** and a **Regina REXX child** using the handle-based process API.  
Key points:
- Use **CRLF** (`0D0A` hex) as line terminator when sending to the child on Windows, so `PULL` sees a complete line.
- The child must **flush** its `STDOUT` after each response so the parent can read it immediately (without waiting for process exit).

---

## 1) Parent (CREXX) Script

```rexx
/* CREXX parent */
Regina = processcreate('C:\Program Files\rexx.org\Regina\regina.exe "C:\Temp\pipeget.rexx"', 1)
if regina < 0 then do
    say "Process couldn't be started"
    exit 1
end

say 'Process handle:' regina

/* Send some lines to stdin (CRLF on Windows) */
rc = processsendinput(regina, 'Hello from CREXX!' || "0d0a"x)
say 1 processreadoutput(regina)

rc = processsendinput(regina, 'I need your help' ||  "0d0a"x)
say 2 processreadoutput(regina)

rc = processsendinput(regina, 'Goodbye!' ||  "0d0a"x)
say 3 processreadoutput(regina)
say 4 processreadoutput(regina)

/* Close stdin to signal EOF (optional once conversation is done) */
call processcloseinput regina

say '--- Output remaining messages from Regina ---'
do forever
    line = processreadoutput(regina)
    if line = '' then do
        call processwait regina, 100
        if processgetexitcode(regina) \= -1 then leave
        iterate
    end
    say 'Regina> ' line
end

cc = processgetexitcode(regina)
say 'Regina> returned with exit code=' cc
```

---

## 2) Child (Regina REXX) Script: `C:\Temp\pipeget.rexx`

```rexx
/* Regina child: pipeget.rexx */
do forever
   pull msg
   call lineout 'STDOUT', 'Regina> I received <'msg'> from you, how can I help'
   call stream  'STDOUT', 'C', 'FLUSH'
   if pos('GOODBYE', translate(msg)) = 0 then iterate
   call lineout 'STDOUT', 'Regina> Hello CREXX, was nice talking to you, cheers see you soon'
   call stream  'STDOUT', 'C', 'FLUSH'
   leave
end
say "Regina terminates now"
exit 4711
```

---

## Notes

- **CRLF required for `PULL`**: The parent must send `"\r\n"` (`"0d0a"x`) so `PULL` receives a complete line immediately.
- **Flush on every response**: `call stream 'STDOUT','C','FLUSH'` ensures output is visible to the parent before the child exits.
- **Reading in the parent**: `processreadoutput()` returns a single line (without CR/LF) if one is available **now**; otherwise it returns an empty string. Use short waits between reads (`processwait`) to avoid busy-waiting.
- **Exit code**: The Regina script ends with exit code `4711`, which the parent can retrieve via `processgetexitcode`.

---

## Expected Interaction (Illustrative)

```
Process handle: 147
1 Regina> I received <Hello from CREXX!> from you, how can I help
2 Regina> I received <I need your help> from you, how can I help
3 Regina> Hello CREXX, was nice talking to you, cheers see you soon
4 Regina terminates now
Regina> returned with exit code= 4711
```

This confirms immediate, interactive responses prior to stdin closure, and a clean termination sequence.




---

## 5. Behavior Notes

- When `capture_output = 1`, stderr is merged into stdout. Both streams are available through the same read functions.
- Reading functions are non-blocking with respect to output availability. They return an empty string if no data is currently ready.
- The internal stdout buffer is bounded. Retrieve output regularly to avoid back-pressure in long-running processes.
- Always close stdin (`processcloseinput`) after finishing writes for programs that read until EOF.

---

## 6. Error Handling

- Negative return values indicate errors or timeouts depending on the function.
- `processgetexitcode` returns `-1` while a process is still running, and `-2` if the exit code cannot be obtained.
- `processlasterror` provides a text message describing the last OS-level failure recorded for the handle.

---

## 7. Summary

This API provides a concise and deterministic interface for process control from REXX. It supports cross-platform creation, controlled termination, non-blocking output retrieval in line and chunk modes, stdin streaming, per-process timeouts, and reliable lifecycle management with explicit cleanup.

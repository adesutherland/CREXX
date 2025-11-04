# Rexx/C Cross-Platform Async Pipe Process Control

This library provides a **portable, asynchronous, and robust API** for managing child processes from Rexx scripts, via a C extension.  
It works on **Windows** and **Linux/Unix**, allowing you to start, monitor, cancel, read, and close external processes through a set of simple functions.

---

## Table of Contents

- [Overview](#overview)
- [API Quick Reference](#api-quick-reference)
- [Usage Pattern Example](#usage-pattern-example)
- [Function Reference](#function-reference)
- [Error Handling](#error-handling)
- [Handle Lifecycle](#handle-lifecycle)
- [Platform Notes](#platform-notes)
- [Example: Cancelling a Process](#example-cancelling-a-process)
- [Design Notes](#design-notes)
- [Authors & Credits](#authors--credits)
- [License](#license)

---

## Overview

This module lets your Rexx code asynchronously launch and control OS child processes—**non-blocking**, and with full access to process output.  
All resources are handled safely via a token system; you always clean up explicitly.

---

## API Quick Reference

| Function      | Description                                              |
|---------------|---------------------------------------------------------|
| `pipecreate`  | Allocate a new process handle token (no process started)|
| `piperun`     | Start a child process for a given handle and command    |
| `pipewait`    | Wait for completion, with optional max timeout (ms)     |
| `pipeget`     | Collect all process output lines into a Rexx array      |
| `pipestatus`  | Poll non-blocking: is process still running?            |
| `pipecancel`  | Forcefully terminate the process                        |
| `pipeclose`   | Free resources and handle                               |

---

## Usage Pattern Example

```rexx
token = pipeCreate()                             -- Allocate process handle
call pipeRun token, 'ping -t 127.0.0.1'         -- Start long-running command
do while pipeStatus(token)                      -- Poll process status
   say "Still running..."
   call sleep 1
end
rc = pipeWait(token, 5000)                      -- Wait up to 5 seconds for exit
rc = pipeGet(token, array)                      -- Collect all output lines
call pipeClose token                            -- Free resources
```

---

## Function Reference

### 1. `pipecreate`

**Syntax:**
```rexx
token = pipeCreate()
```
Allocates and returns a new process handle (token).  
No process is started yet; use with `pipeRun`.

---

### 2. `piperun`

**Syntax:**
```rexx
rc = pipeRun(token, command)
```
Starts a child process for the given `command` string (e.g., `"ping -t 127.0.0.1"`).  
Returns `0` on success, `<0` on error.

---

### 3. `pipewait`

**Syntax:**
```rexx
rc = pipeWait(token, maxwait)
```
Waits asynchronously for the process to finish, polling every 200ms, up to `maxwait` milliseconds.  
Returns `0` if process exited, `4` if maxwait reached, other values on error.

---

### 4. `pipeget`

**Syntax:**
```rexx
rc = pipeGet(token, array)
```
Collects all output lines produced by the process into the provided Rexx array.  
The array is filled such that `array[1]` is the first line, etc.  
Returns `0` on success, `4` if process not running, other values on error.

---

### 5. `pipestatus`

**Syntax:**
```rexx
running = pipeStatus(token)
```
Polls the child process status **non-blocking**:
- Returns `1` if still running
- Returns `0` if exited or error

---

### 6. `pipecancel`

**Syntax:**
```rexx
rc = pipeCancel(token)
```
Forcibly terminates the running child process (`SIGKILL`/`TerminateProcess`).  
Returns `0` on success, negative on error, `-2` if not running.

---

### 7. `pipeclose`

**Syntax:**
```rexx
call pipeClose token
```
Frees all resources, closes handles, and releases memory for the process handle.  
**Always call this to avoid memory/resource leaks!**

---

## Error Handling

- All functions return `0` on success.
- Negative values indicate allocation or process errors.
- Special return values (like `4` for timeout in `pipewait`/`pipeget`) are documented above.

---

## Handle Lifecycle

- **Do not reuse a token** after `pipeclose`.
- Always use `pipewait` or poll with `pipestatus` before calling `pipeget`.
- After `pipecancel`, always call `pipewait` and `pipeclose` to fully clean up.

---

## Platform Notes

- **Windows:** For cancellation, use the raw executable (e.g., `ping -t 127.0.0.1`), not `cmd /c ...`, if you want reliable termination.
- **Linux/Unix:** `tail -f /dev/null` is a safe long-running test process.  
  Use `yes` for a command that produces infinite output (caution: it floods your pipe).

---

## Example: Cancelling a Process

```rexx
token = pipeCreate()
call pipeRun token, 'ping -t 127.0.0.1'
call sleep 2
call pipeCancel token
call pipeWait token, 1000
call pipeGet token, array
call pipeClose token
```

---

## Design Notes

- Internally, the process handle is an allocated pointer (`ChildProcess *`) cast to an integer token.
- Do **not** attempt to use the token after closing/freeing.
- The interface is safe for concurrent use and robust to process exit or cancellation.
---


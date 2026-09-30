# rxconsole: Level B terminal primitives

This standard C provider owns terminal mechanism, not widgets or application
policy. It needs no GTK, ncurses, JSON or Level G library. The old `console.c`
demonstration is retained unbuilt; `rxconsole` is the new maintained namespace.

| Function | Result |
| --- | --- |
| `open(modes)` | Positive lease or negative error; raw input is always enabled |
| `close(handle)` | Restore terminal state and release the lease |
| `modes(handle, modes)` | Change optional presentation/input reporting modes |
| `size(handle, expose columns, expose rows)` | Current viewport in cells |
| `capabilities(handle)` | Capability mask; 0 queries the platform profile before open |
| `poll(handle, timeout, expose fields[], expose text)` | One input event, timeout or error |
| `write(handle, text)` | One batched UTF-8/VT output, at most 1 MiB |

Handles, masks, statuses and numeric outputs are `.int`; `fields` is `.int[]`;
`text` is `.string`. The caller owns all returned data. Leases are local to the
creating VM and invalid after close. One physical console lease is allowed per
process. Open requires foreground terminal stdin/stdout; it never launches a
window or steals redirected input. Current RXPA session teardown also closes
an abandoned lease. The old demo's `console.*` calls are not aliases.

Mode bits: alternate screen **1**, button/drag mouse **2**, all-motion mouse
**4**, bracketed paste **8**. Capability bits: raw **1**, VT output **2**,
resize **4**, mouse **8**, bracketed paste **16**, UTF-8 input **32**. Native
Windows console input delivers committed text but does not preserve paste
boundaries, so the ANSI host does not advertise `ui.paste` there. VT profile
capabilities do not prove that every remote client or
multiplexer forwards the requested reports; all widgets remain keyboard-usable.

`poll` timeout is milliseconds: **0** immediate, positive bounded wait, **-1**
wait until input/resize. Size is checked every 25 ms during waiting, avoiding
replacement of process-wide SIGWINCH handlers. Duplicate sizes are coalesced.
Fragments are retained; a 40 ms ambiguity deadline resolves lone Escape and
incomplete non-paste sequences. Paste is bounded to 16,384 UTF-8 bytes; oversize
paste is drained through the end marker before reporting a limit error, never
reinterpreted as commands. Malformed UTF-8 becomes U+FFFD.

Events: **0** timeout, **1** key, **2** committed text, **3** paste, **4** resize,
**5** mouse, **6** wheel, **7** interruption/Ctrl-C, **8** disconnected.
Errors: **-1** invalid argument/lease, **-2** no usable terminal, **-3** busy or
not foreground, **-4** I/O, **-5** bounded limit, **-6** unsupported/malformed
escape sequence. Output fields are reset on every call, including errors.

| Array positions (one-based) | Meaning |
| --- | --- |
| 1, 2 | Zero-based viewport x, y in cells |
| 3, 4 | Columns, rows on resize |
| 5 | Shift=1, Alt=2, Ctrl=4 modifier bits |
| 6 | Held buttons: left=1, middle=2, right=4 |
| 7 | Changed button: left=1, middle=2, right=3 |
| 8 | Mouse press=1/release=2/move=3; key press=1/repeat=2 |
| 9, 10 | Horizontal/vertical wheel delta; positive right/down |

`text` contains a key name (Enter, Escape, Tab, Backspace, arrows, Home/End,
PageUp/Down, Insert/Delete, F1–F12), committed Unicode text, or pasted text.
Ordinary typing is text, not a fabricated physical-key event. Key release is
not advertised in this first API. Ctrl-C is an interruption event: the owner
chooses whether to cancel a dialog or close. There are no OS-thread callbacks
into cREXX. Close before handing the terminal to a child process.

The Windows implementation normalizes native console mouse coordinates and
wheel direction to these same fields; see Microsoft's
[MOUSE_EVENT_RECORD contract](https://learn.microsoft.com/en-us/windows/console/mouse-event-record-str).
That backend still needs native Windows QA. `write` uses the current RXPA
NUL-terminated string surface; embedded NUL ends output, so it is not a binary writer.

Normal/session teardown restores saved terminal modes plus cursor/screen state.
SIGKILL, process corruption and loss of foreground ownership cannot promise
restoration. Existing platform terminal-ownership safeguards remain unchanged.

Build `rxconsole_test_artifacts` before `ctest -R '^rxconsole_'`. The provider
ships as both dynamic and static RXPA artifacts through normal product packaging.

# Host information and timing with `rxplatform`

The optional Level G `rxplatform` provider contains the narrow portable
host-information and timing subset extracted from the retired `system` plugin:

| Procedure | Contract |
|---|---|
| `uptime()` | Host uptime in seconds. |
| `user()` | Current operating-system user name. |
| `host()` | Host name. |
| `osname()` | Portable family name such as `Windows`, `Linux`, or `macOS`. |
| `sleep(milliseconds)` | Sleep for 0 through 4,294,967,295 milliseconds and return zero. |
| `message(text)` | Display an informational desktop dialog and return zero. |
| `confirm(text)` | Display a Yes/No desktop dialog; return `1` for Yes and `0` for No. |
| `input(prompt)` | Request one text line; return entered text or an empty string on Cancel. |

`message()` and `confirm()` use native Windows dialogs on Windows, `osascript`
on macOS, and `zenity` or `kdialog` on Linux. Linux therefore requires one of
those desktop dialog helpers to be installed. They are synchronous and return
only after the user closes the dialog. If the platform cannot provide a
desktop dialog, they raise `FAILURE`.

The class library `Os` wrapper also provides:

| Method | Contract |
|---|---|
| `x.warning(text)` | Display the text as an informational dialog prefixed with `Warning:`. |
| `x.error(text)` | Display the text as an informational dialog prefixed with `Error:`. |

Both wrapper methods return zero on success and use the same platform dialog
support as `x.message()`.

Unavailable host information raises `FAILURE`; invalid sleep values raise
`INVALID_ARGUMENTS`. Clipboard, beep, process-global values, pipe operations,
and developer inspection are intentionally not included. The process-reentrant
provider ships in automatic dynamic and static forms under provider ID
`rxplatform`.

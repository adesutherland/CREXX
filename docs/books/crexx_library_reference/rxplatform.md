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

Unavailable host information raises `FAILURE`; invalid sleep values raise
`INVALID_ARGUMENTS`. Clipboard, beep, process-global values, pipe operations,
and developer inspection are intentionally not included. The process-reentrant
provider ships in automatic dynamic and static forms under provider ID
`rxplatform`.

# Filesystem operations with `rxfs`

The standard/default `rxfs` provider owns filename and directory conveniences.
It is callable from Level B source when installed, but it is not part of the
minimal bootstrap core.

| Procedure | Contract |
|---|---|
| `cwd()` | Current working directory. |
| `loadpath()` | Directory containing the running executable. |
| `chdir(path)` | Change directory. |
| `isdir(path)` / `isfile(path)` | Return 1 when the path has the requested type, otherwise 0. |
| `mkdir(path)` / `rmdir(path)` | Create or remove one directory. |
| `delete(path)` / `rename(source, target)` | Delete or rename a file. |
| `listdir(path, expose entries)` | Populate a `.string[]` and return its entry count. |
| `append(source, target)` | Append source bytes to target and return the byte count. |

Mutating functions retain the historical integer status convention: zero is
success where no count is returned; negative values distinguish missing,
permission, general I/O, open, and write failures. Invalid arity or an
unrepresentably long path raises `INVALID_ARGUMENTS`.

`rxfs` replaces the filesystem subset of the retired broad `system` plugin.
The `crexx` compiler driver now links only `rxfs`, its actual native provider
dependency. Dynamic execution and native packaging resolve it automatically
from RXBIN provider metadata.

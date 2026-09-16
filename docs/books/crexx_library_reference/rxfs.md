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
| `pathkind(path)` | Inspect the final path without following a symlink: 0 missing, 1 regular file, 2 directory, -1 inaccessible, symlink/reparse point or other unsupported type. |
| `abspath(path)` | Make a path absolute without resolving symlinks. Returns empty on failure; this is not a security/ownership check. |
| `copy(source, target)` | Copy a regular file, refusing an existing target. Return 0 on success, -8 on failure. |
| `hardlink(source, target)` | Create another name for a regular file on the same filesystem, refusing an existing target. Return 0 or -8. |
| `move(source, target)` | Rename a file/directory without replacing an existing target. Return 0 or -8. Supported on Windows, macOS and Linux with `renameat2`; fail explicitly elsewhere. |

The older mutating functions retain their historical integer status convention:
zero is success where no count is returned; negative values distinguish missing,
permission, general I/O, open, and write failures. The added transfer operations
use the simpler 0/-8 contract shown above; `abspath` returns empty on failure.
Invalid arity raises `INVALID_ARGUMENTS`.

`rxfs` replaces the filesystem subset of the retired broad `system` plugin.
The `crexx` compiler driver now links only `rxfs`, its actual native provider
dependency. Dynamic execution and native packaging resolve it automatically
from RXBIN provider metadata.

## Owning file guards

```rexx
options levelb
import rxfs

guard = .rxfs..fileguard('operation.lock', 'exclusive')
if guard.held() then do
  /* Work while the guard owns the lock. */
  call guard.close()
end
```

The native `fileguard(path, mode)` factory takes an explicit mode and never waits:

- `exclusive` opens/creates a lock file. Competing guards fail to acquire it.
  The file remains after close; its existence does not mean it is locked.
- `lease` opens an existing regular file for read/write. On Windows it permits
  deletion/rename while denying new readers/writers; existing incompatible opens
  or loaded images prevent acquisition. On Unix it is a cooperative `flock`
  and does **not** exclude unrelated readers or running programs.

`held()` returns 1 while the OS handle is held. `status()` returns 0 after
successful acquisition/close or -8 on acquisition/I/O failure. Failed
acquisition still returns an initialized object. `close()` is idempotent and
returns 0 or -8. Invalid modes raise `INVALID_ARGUMENTS`.

Copies share ownership: closing any alias releases the lock for all aliases.
The last native-value finalizer, including VM teardown, releases an unclosed
handle. Cached VM values may outlive a lexical scope, so call `close()` for
timely release. Guards belong to their VM session; pass filenames/data between
workers and acquire guards inside each worker, never transfer a live guard.
The factory and methods use the C RXPA bindings directly; no Rexx forwarding
class is required. Supported native-object host services are required.

New transfer/guard operations reject a final symlink/reparse point. Parent
directory traversal, ownership, permissions, crash recovery and transactional
publication remain application responsibilities. Hard links share file content;
they are not independent copies. A copy can fall back when hard linking is
unsupported, but callers must choose that explicitly.

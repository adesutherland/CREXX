## version

`version()` returns a compact description of the running VM build:

```rexx
version() = .string
```

The result contains exactly four space-separated fields:

```text
platform bits crexx-version build-date
```

- `platform` is `linux`, `windows`, `macOS`, `cms`, or `unknown`.
- `bits` is the VM pointer width, `32` or `64`.
- `crexx-version` starts with `crexx-` and may include prerelease, build-channel,
  commit, or dirty-worktree metadata.
- `build-date` is the VM compile date in `yyyymmdd` form.

Example shape:

```text
macOS 64 crexx-1.0.0-beta.3+local.g123456789abc 20260713
```

The implementation directly executes `rxvers`; it has no arguments and does
not inspect compiler or source-tree state. The instruction has no translated
VM signal. Failure to allocate the returned string is fatal.

`lib/rxfnsb/tests_functional/ts_version.crexx` checks every stable field rule
without hard-coding a particular release identifier.

# crexxsaa host integration

`crexxsaa` is the initial CREXX compatibility API for C hosts that want a
REXXSAA-shaped entry point without taking on the full historical REXXSAA ABI.
It is deliberately small: the API is a stable facade over the current CREXX
`rxvml`, `ADDRESS`, sandbox, and exposed-variable contracts.

The first supported host model is command-environment execution:

- host C code creates a `crexxsaa_context`
- the host registers one or more `ADDRESS` callback environments
- the host configures the CREXX compiler, assembler, import directory, and
  optional cache directory
- the host runs either an existing `.rxbin` or a source file that `crexxsaa`
  compiles and caches

This is not a full `RexxStart()` or `RexxVariablePool()` clone. Future adapter
entry points may be added where real integrations need them, but the internal
runtime model remains the modern CREXX `ADDRESS` model.

## Writing hosted source

`crexxsaa` compiles source exactly as supplied. It does not add `OPTIONS
LEVELB`, an `ADDRESS` clause, imports, or host-specific boilerplate.

Hosted source should therefore declare the language level and command
environment it needs:

```rexx
options levelb
address the
'emsg CREXX_PROFILE_HOSTED'
```

The script owns its language mode and its command routing. `crexxsaa` owns only
compile, cache, load, and run orchestration.

## Minimal host flow

```c
crexxsaa_context *ctx = NULL;

crexxsaa_create(location, library_rxbin, &ctx);
crexxsaa_set_compiler(ctx, rxc_path, rxas_path, import_dir);
crexxsaa_register_address_environment(ctx, "THE", the_callback, userdata);
crexxsaa_set_address_environment(ctx, "THE");
crexxsaa_run_source(ctx, profile_path, "THE", 0, argc, argv, &program_rc);
crexxsaa_destroy(ctx);
```

The cache namespace argument, `"THE"` in this example, is part of the cache
identity. Different hosts can compile the same source path without sharing a
cache bucket accidentally.

## ADDRESS callbacks

A host registers an environment with
`crexxsaa_register_address_environment()`. When CREXX executes a command in
that environment, the callback receives a `crexxsaa_address_request` containing
the environment name, command text, and active context.

The callback fills a `crexxsaa_address_response`:

- `rc`: command return code
- `condition_name`: optional CREXX condition name
- `diagnostic`: optional diagnostic text

The callback should return zero for a successfully handled dispatch. Non-zero
callback return values indicate host-side failure in the bridge itself.

## Variable access during callbacks

`crexxsaa` exposes simple name-based helpers for active `ADDRESS` callbacks:

- `crexxsaa_address_variable_get_alloc()`
- `crexxsaa_address_variable_set()`
- `crexxsaa_free()`

These helpers are valid only while a native `ADDRESS` callback is active. They
resolve variables in this order:

1. Direct scalar `ADDRESS ... EXPOSE name` bindings.
2. Direct stem/array `ADDRESS ... EXPOSE name[]` bindings.
3. The active `ADDRESS ... SANDBOX pool` object.
4. The request's standard sandbox fallback.

The host gets one API, while the CREXX script chooses whether a command uses
direct exposure or sandbox storage.

Compound names such as `FILENAME.1` are mapped to exposed stems by splitting at
the first dot and using the remaining text as the stem key. Names are matched
case-insensitively at this facade layer to fit legacy host expectations. The
standard CREXX sandbox already normalises keys internally.

For compatibility with command processors that treat a scalar result as a
one-item stem, an exposed scalar also has a limited compound view:

- `name.0` reads as `"1"`
- `name.1` reads the scalar value
- writes to `name.0` are accepted and ignored
- writes to any other `name.tail` update the scalar value

This rule is applied only when no exposed stem with the same base name exists.
Real `EXPOSE name[]` stems keep their normal stem semantics.

The variable facade is not a general variable-pool enumerator and does not
provide arbitrary access to unexposed CREXX locals.

## Source cache

`crexxsaa_run_source()` compiles source through `rxc` and `rxas`, then stores the
resulting `.rxbin` in a disposable cache. Normal source edits, CREXX rebuilds,
compiler path changes, and library rebuilds cause a recompile without manual
cache clearing.

Cache location rules:

- `CREXXSAA_CACHE_DIR` overrides the platform default
- a host may call `crexxsaa_set_cache_dir()`
- if both `CREXXSAA_CACHE_DIR` and a host-provided cache directory are present,
  `CREXXSAA_CACHE_DIR` wins
- platform defaults:
  - macOS: `$HOME/Library/Caches/crexx/crexxsaa`
  - Windows: `%LOCALAPPDATA%\crexx\crexxsaa`, falling back under
    `%USERPROFILE%\AppData\Local`
  - other Unix-like systems: `$XDG_CACHE_HOME/crexx/crexxsaa`, falling back to
    `$HOME/.cache/crexx/crexxsaa`

Runtime cache controls:

- `CREXXSAA_CACHE_DISABLE=1`: compile source through temporary files only
- `CREXXSAA_CACHE_REFRESH=1`: ignore a valid cache hit and replace it
- `CREXXSAA_CACHE_TRACE=1`: write cache decisions to `stderr`

The trace currently emits events such as `miss`, `hit`, `stale`, `refresh`, and
`disabled`.

Compiler path controls:

- hosts normally call `crexxsaa_set_compiler(ctx, rxc, rxas, import_dir)`
- `CREXXSAA_RXC`, `CREXXSAA_RXAS`, and `CREXXSAA_IMPORT_DIR` override the
  configured compiler values

## Cache maintenance tool

The CREXX build/install includes a `crexxsaa` maintenance binary in the normal
`bin` directory. It is a troubleshooting tool for the compiled-script cache,
not a script runner.

Common commands:

```sh
crexxsaa --location
crexxsaa --list
crexxsaa --clear
crexxsaa --cache-dir /tmp/crexxsaa-cache --list
crexxsaa --cache-dir /tmp/crexxsaa-cache --clear --list
```

Default invocation with no arguments prints the cache location and lists cache
entries. `--location` alone prints only the resolved cache directory.

Example list output:

```text
cache: /Users/adrian/Library/Caches/crexx/crexxsaa
source: /path/to/profile.the
  bucket: 0379ad70148bf7ca
  rxbin: /Users/adrian/Library/Caches/crexx/crexxsaa/v1/0379ad70148bf7ca/b0ec3f1ffcc51e4c.rxbin
  rxbin_size: 1216
  source_hash: 9100aa6921615883
  config_hash: 5af0757a39c4eba1
```

## Technical cache layout

The cache schema is versioned. Current entries live under `v1`:

```text
<cache-root>/v1/<source-key>/
  manifest
  <object-hash>.rxbin
```

`source-key` is an FNV-1a 64-bit hash rendered as 16 hex characters. It
includes the host namespace and canonical source path when available.

`object-hash` is also an FNV-1a 64-bit hash rendered as 16 hex characters. It
includes the source content hash and compiler/library configuration hash.

Manifest fields:

```text
version=1
source_path=/absolute/or/supplied/source/path
source_hash=<16-hex-content-hash>
source_size=<bytes>
source_mtime=<mtime>
config_hash=<16-hex-config-hash>
rxbin=<absolute/cache/path/to/object.rxbin>
```

The content hash, not only the timestamp, decides whether source changed. Size
and mtime are recorded for diagnostics and human inspection.

The configuration hash includes:

- cache schema and `CREXXSAA_ABI_VERSION`
- configured or environment-overridden `rxc` path
- configured or environment-overridden `rxas` path
- configured or environment-overridden import directory
- loaded `library.rxbin` path
- file size and mtime for compiler, assembler, import directory, and library
  path where the platform can stat them

Compile and update sequence:

- compute source hash and configuration hash
- read the source bucket manifest if it exists
- on a valid hit, load the cached `.rxbin`
- on miss, stale, or refresh:
  - run `rxc -i <import-dir> -o <temp-base> <source>`
  - run `rxas -o <temp-base> <temp-base>.rxas`
  - rename the temporary `.rxbin` into the source bucket
  - write a new manifest through a temporary file and rename it into place
  - remove the previous cached `.rxbin` for the bucket when it has been
    superseded

Invalidation:

- `crexxsaa_invalidate_source(ctx, source_path, namespace)`
- `crexxsaa_invalidate_all(ctx)`
- `crexxsaa_clear_cache(cache_dir_override)`
- `crexxsaa --clear`
- `crexxsaa --cache-dir DIR --clear`

The cache is disposable. Deleting it should affect performance only, not
program semantics. The current implementation uses atomic file replacement for
compiled objects and manifests, but it is not a full cross-process locking
protocol. For troubleshooting, clear the cache while the host application is
idle.


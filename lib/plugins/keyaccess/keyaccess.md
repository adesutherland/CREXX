# KeyAccess Direct-Access Storage Documentation

## Overview

KeyAccess is a lightweight, self-contained direct-access storage facility for
persistent keyed data. It is intended for small embedded applications that
require key-based access without an external database system.

KeyAccess provides persistent key-value storage, transaction handling,
sequential key traversal, caching, locking, and storage maintenance operations.
It has no external dependencies, requires no server or database setup, and
integrates directly with the cREXX runtime.

KeyAccess is a direct-access storage facility, not a database management
system. It is not intended to replace database functionality. For applications
requiring SQL, relational queries, schemas, or more advanced database
functionality, an embedded database such as SQLite is the appropriate choice.

KeyAccess has two API layers:

- The **keyaccess plugin** provides the low-level native storage functions.
- The **`.KeyAccess` cREXX class** provides an object-oriented wrapper around
  the native KeyAccess functions.

This document primarily describes the native KeyAccess interface. Where useful,
the corresponding `.KeyAccess` methods are also shown.


## Table of Contents

- [Features](#features)
  - [Lookup Performance](#lookup-performance)
  - [Representative Performance](#representative-performance)
- [File Structure](#file-structure)
- [API Layers](#api-layers)
- [Available Functions](#available-functions)
- [Transaction Management](#transaction-management)
- [Storage Maintenance](#storage-maintenance)
- [Error Codes](#error-codes)
- [Usage Examples](#usage-examples)
- [Version History](#version-history)


## Features

- Persistent keyed storage with CRUD operations
- Transaction support (`begin`, `commit`, `rollback`)
- Sequential traversal of active keys
- Key selection using prefix or substring matching through the `.KeyAccess`
  wrapper
- Lazy in-memory hash index for keyed access
- Incremental in-memory index maintenance after writes and deletes
- LRU cache for improved read performance
- Full-key verification of hash matches
- File locking for concurrent access
- Storage maintenance (`backup`, `compact`, `validate`)
- Error logging and statistics
- No external database or server dependency


### Lookup Performance

KeyAccess builds an in-memory hash index lazily on the first keyed read or
write. The index stores a 64-bit hash and the corresponding index-file offset.

A matching hash is always verified with the complete key, so hash collisions
do not change the lookup result. A hash value is used only to locate candidate
index records; it is never treated as proof that two keys are equal.

Successful new writes and deletes maintain the in-memory index incrementally.
Existing records can therefore be located through the same indexed lookup path
used by normal keyed reads, avoiding repeated scans of the complete index
during sequences of writes.

The in-memory index is discarded when its contents can no longer safely
describe the current index state, including after rollback, compaction, reset,
close, and reopening the store. It is rebuilt lazily when keyed access next
requires it.

The handle-local LRU cache uses the same hash-directed lookup strategy. Cache
collisions are resolved by comparing the complete key. A cache miss continues
to the authoritative keyed lookup using the in-memory hash index. Hash
candidates are verified against the complete key stored in the index file; the
cache does not change storage semantics or durability.

A missing key is a normal lookup result and is not treated as an operational
error. `readkey` therefore does not write an error-log entry for
`KA_ERROR_NOTFOUND`. Parameter, I/O, memory, and index failures continue to be
logged normally. This avoids unnecessary log-file operations during negative
lookups.

The first keyed lookup may include the cost of scanning the index to build the
in-memory structure. Subsequent keyed lookups avoid that scan while the
structure remains valid for the open handle.

`listkey` and sequential traversal continue to read index records directly and
are unaffected by the keyed lookup cache and hash index.

### Representative Performance

KeyAccess is designed primarily for simplicity and self-contained keyed access,
rather than as a replacement for a general-purpose database system. The
implementation is nevertheless intended to remain practical for moderately
sized local stores.

As an illustrative measurement, the supplied KeyAccess lookup benchmark was
run on Windows with 500,000 active keys:

| Operation | Count | Elapsed |
|-----------|------:|--------:|
| Insert new keys | 500,000 | 14.19 s |
| `ONLY` key selection | 500,000 scanned | 3.19 s |
| `ANY` key selection | 500,000 scanned | 3.46 s |
| Present keyed lookup | 500,000 | 10.84 s |
| Missing keyed lookup | 500,000 | 9.69 s |
| Repeated cached lookup | 10,000 | 0.030 s |

This corresponds to approximately 35,000 new-key inserts per second and
46,000 successful keyed lookups per second in this particular run.

The `ONLY` and `ANY` measurements perform sequential traversal of all active
keys; they are not secondary-index searches.

These figures are representative measurements rather than performance
guarantees. Results depend on hardware, operating system, compiler settings,
storage, cache state, key/value sizes, and workload.

The benchmark can be reproduced using the
`examples/keydb_lookup_benchmark.crexx` sample.

## File Structure

A KeyAccess store uses separate files for data and index information.

- **Data file**: Stores the actual key-value data.
- **Index file**: Stores key locations and associated index metadata.
- **Log file**: Records diagnostic and error information where logging is
  enabled.

The index is maintained in physical record order rather than key order.
Direct keyed access is provided through the handle-local in-memory hash index,
while sequential operations traverse active index records in storage order.

Deleted index records may remain present until the store is compacted. Normal
keyed access and sequential traversal expose only active records.

## API Layers

The low-level KeyAccess plugin functions are wrapped by the `.KeyAccess` cREXX
class.

Typical mappings are:

| Native function | `.KeyAccess` method |
|-----------------|---------------------|
| `openkey` | `open()` |
| `closekey` | `close()` |
| `readkey` | `get()` |
| `writekey` | `put()` |
| `deletekey` | `remove()` |
| `firstkey` | `firstKey()` |
| `nextkey` | `nextKey()` |
| — | `findKeys()` |
| — | `clear()` |
| — | `reset()` |

The `.KeyAccess` wrapper also provides `findKeys(keyPart, matchMode)` as a
convenience method for selecting active keys.

The supported match modes are:

| Mode | Meaning |
|------|---------|
| `ONLY` | `keyPart` must occur at the beginning of the key |
| `ANY` | `keyPart` may occur anywhere in the key |

`findKeys()` scans the active keys using `firstKey()` and `nextKey()` and
returns a `.string[]` containing the matching keys. Results follow index
traversal order and are not sorted.

An empty `keyPart` matches every active key.

The native KeyAccess plugin has no separate key-search function;
`findKeys()` is implemented by the `.KeyAccess` cREXX wrapper.

The native interface operates on an integer storage handle. The `.KeyAccess`
class owns this handle and provides the corresponding operations as methods.


## Available Functions

### Storage Operations

#### openkey

Opens or creates a KeyAccess store.

When using the `.KeyAccess` wrapper, calling `open()` on an already-open object
closes the previous handle before opening the new store.

**Parameters:**

- `filename` (string): Path to the data file.
- `mode` (string): File access mode, for example `"r"`, `"w"`, or `"w+"`.

**Returns:**

Integer handle for the opened store, or a negative error code.

**Example:**

```rexx
handle = openkey("mystore.dat", "w+")
```


---

#### closekey

Closes an open KeyAccess store.

Closing the store also releases handle-local resources such as the cache,
in-memory lookup index, and traversal state.

**Parameters:**

- `handle` (int): Storage handle returned by `openkey`.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = closekey(handle)
```


---

#### writekey

Writes a key-value pair to the store.

If the key already exists, its active index record is replaced; a second
active record is not created.

Key existence is determined through the in-memory hash index when available.
Hash candidates are always verified using the complete key.

Successful writes update the in-memory lookup index incrementally rather than
discarding and rebuilding the complete index after each write.

**Parameters:**

- `handle` (int): Storage handle.
- `key` (string): Key to write.
- `value` (string): Value to store.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = writekey(handle, "user.name", "John Doe")
```


---

#### readkey

Reads a value by key.

Keyed lookup first checks the handle-local cache. If the value is not cached,
the in-memory hash index is used to locate candidate index records. Every hash
candidate is verified using the complete key before a value is returned.

A key that does not exist is a normal lookup result. `KA_ERROR_NOTFOUND` is
therefore not written to the error log by `readkey`.

**Parameters:**

- `handle` (int): Storage handle.
- `key` (string): Key to read.

**Returns:**

String value or an error/not-found indication.

**Example:**

```rexx
value = readkey(handle, "user.name")
```


---

#### deletekey

Deletes a key-value pair from the store.

When the in-memory lookup index is available, the key is located through that
index and the corresponding entry is removed incrementally after a successful
delete.

Hash candidates continue to be verified using the complete key.

**Parameters:**

- `handle` (int): Storage handle.
- `key` (string): Key to delete.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = deletekey(handle, "user.name")
```


---

#### listkey

Returns the number of active keys in the store.

Deleted index records are not included in the count.

`listkey` scans index records directly; it does not depend on the in-memory
keyed lookup index.

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

Integer count of active keys.

**Example:**

```rexx
count = listkey(handle)
```

The `.KeyAccess` wrapper also provides `clear()`, which removes all active keys
in one transaction and returns the transaction status code.

The wrapper also provides `reset()`. This is a destructive, non-transactional
operation that truncates both storage files directly, without scanning keys.
It clears the cache and in-memory lookup index and rejects an active
transaction.

Use `clear()` when rollback-safe logical deletion is required and `reset()`
when the store should be emptied immediately.


---

#### firstkey

Starts sequential traversal of the KeyAccess index and returns the first active
key.

Deleted index records are skipped.

Traversal state is maintained independently on the storage handle and does not
depend on the current `FILE` position used internally by other KeyAccess
operations.

Calling `firstkey()` restarts sequential traversal for that handle.

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

- The first active key.
- `"NOT_FOUND"` when no active key exists.
- `"ERROR"` for an invalid handle or traversal failure.

**Example:**

```rexx
key = firstkey(handle)
```


---

#### nextkey

Continues sequential traversal previously started by `firstkey()` and returns
the next active key.

Deleted index records are skipped.

Other KeyAccess operations such as `readkey` may access or reposition the
index file without disturbing the traversal cursor.

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

- The next active key.
- `"NOT_FOUND"` when the end of traversal has been reached.
- `"ERROR"` for an invalid handle or traversal failure.

**Example:**

```rexx
key = nextkey(handle)
```

A storage handle has one sequential traversal cursor. Calling `firstkey()`
again restarts traversal for that handle.


### Key Selection Using `.KeyAccess`

#### findKeys

Returns active keys matching a specified part of the key.

`findKeys()` is a convenience method implemented by the `.KeyAccess` wrapper.
It scans active keys using `firstKey()` and `nextKey()`; it does not introduce
a separate native KeyAccess search operation or secondary index.

**Parameters:**

- `keyPart` (string): Text to match against active keys.
- `matchMode` (string): Determines how `keyPart` is matched.

Supported match modes:

| Mode | Meaning |
|------|---------|
| `ONLY` | The key must begin with `keyPart` |
| `ANY` | `keyPart` may occur anywhere in the key |

An empty `keyPart` matches every active key.

**Returns:**

A `.string[]` containing all matching active keys.

Results follow index traversal order and are not sorted. An empty array is
returned when no keys match.

**Examples:**

Select keys beginning with `type:`:

```rexx
keys = db.findKeys("type:", "ONLY")
```

Select keys containing `.Module17` anywhere in the key:

```rexx
keys = db.findKeys(".Module17", "ANY")
```

For structured keys such as:

```text
type:.Module17
ref:calling:.Module16:.Module17
ref:calling:.Module17:.Module18
```

all `calling` reference records can be selected using:

```rexx
keys = db.findKeys("ref:calling:", "ONLY")
```

All keys mentioning `.Module17` can be selected using:

```rexx
keys = db.findKeys(".Module17", "ANY")
```

`findKeys()` performs a sequential scan of active keys. It is therefore a
convenience selection operation rather than an indexed secondary-key search.


## Transaction Management

#### txbegin

Begins a new transaction.

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = txbegin(handle)
```


---

#### txcommit

Commits the current transaction.

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = txcommit(handle)
```


---

#### txrollback

Rolls back the current transaction.

Rollback restores the data and index files to their state at `txbegin`,
including index records modified by updates or deletes.

Cached values and the in-memory lookup index are invalidated after a
successful rollback. The lookup index is rebuilt lazily when subsequent keyed
access requires it.

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = txrollback(handle)
```


## Storage Maintenance

#### stats

Retrieves KeyAccess statistics.

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

String containing storage statistics.

**Example:**

```rexx
storageStats = stats(handle)
say storageStats
```


---

#### backup

Creates a backup of the KeyAccess store.

**Parameters:**

- `handle` (int): Storage handle.
- `path` (string): Backup file path.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = backup(handle, "backup.dat")
```


---

#### validate

Validates storage integrity.

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

Integer status indicating the number of errors found.

**Example:**

```rexx
errors = validate(handle)
```


---

#### compact

Compacts the store by removing deleted entries.

Compaction changes the physical index layout and therefore invalidates the
in-memory keyed lookup index. The index is rebuilt lazily when subsequent
keyed access requires it.

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = compact(handle)
```


## Error Codes

The native KeyAccess API uses the following status codes:

| Code | Name | Meaning |
|-----:|------|---------|
| 0 | `KA_SUCCESS` | Operation successful |
| -1 | `KA_ERROR_PARAM` | Invalid parameters |
| -2 | `KA_ERROR_MEMORY` | Memory allocation failed |
| -3 | `KA_ERROR_IO` | I/O operation failed |
| -4 | `KA_ERROR_LOCK` | File locking failed |
| -5 | `KA_ERROR_NOTFOUND` | Key not found |
| -6 | `KA_ERROR_EXISTS` | Key already exists |
| -7 | `KA_ERROR_CORRUPT` | Storage corruption detected |
| -8 | `KA_ERROR_TXACTIVE` | Transaction already active |
| -9 | `KA_ERROR_TXINACTIVE` | No active transaction |
| -10 | `KA_ERROR_TOOLONG` | Key or value too long |

`KA_ERROR_NOTFOUND` represents a normal negative keyed lookup and is not logged
as an operational error by `readkey`.

Functions returning strings cannot directly return these integer error codes.
In particular, the sequential traversal functions use `"NOT_FOUND"` to
indicate the end of traversal and `"ERROR"` to indicate a traversal failure.


## Usage Examples

### Basic Key-Value Operations

```rexx
/* Open store */
handle = openkey("users.dat", "w+")

/* Write a value */
call writekey handle, "user.name", "John Doe"

/* Read the value */
name = readkey(handle, "user.name")
say "User name:" name

/* Close store */
call closekey handle
```


### Transaction Example

```rexx
/* Open store */
handle = openkey("users.dat", "w+")

/* Start transaction */
call txbegin handle

/* Write multiple values */
call writekey handle, "user.1.name", "John Doe"
call writekey handle, "user.1.email", "john@example.com"

/* Commit changes */
call txcommit handle

/* Get statistics */
storageStats = stats(handle)
say storageStats

/* Close store */
call closekey handle
```


### Storage Maintenance Example

```rexx
/* Open store */
handle = openkey("users.dat", "w+")

/* Create backup */
call backup handle, "users.backup.dat"

/* Validate store */
errors = validate(handle)

if errors > 0 then do
    say "Found" errors "errors in store"

    /* Compact to remove deleted entries */
    call compact handle
end

/* Close store */
call closekey handle
```


### Sequential Key Traversal Using the Native API

The native `firstkey` and `nextkey` functions can be used to enumerate all
active keys:

```rexx
key = firstkey(handle)

do while key <> "NOT_FOUND"

    value = readkey(handle, key)
    say key value

    key = nextkey(handle)

end
```

Calling `readkey()` inside the loop does not disturb sequential traversal.

Traversal follows index-record order and returns only active keys.


### Sequential Key Traversal Using `.KeyAccess`

The `.KeyAccess` cREXX wrapper exposes the traversal operations as `firstKey()`
and `nextKey()`:

```rexx
key = db.firstKey()

do while key <> "NOT_FOUND"

    value = db.get(key)
    say key value

    key = db.nextKey()

end
```

The traversal cursor belongs to the underlying KeyAccess handle. Index
operations performed by `get()`, `containsKey()`, or other `.KeyAccess` methods
do not change the next key returned by `nextKey()`.

Calling `firstKey()` again restarts traversal.


### Selecting Keys Using `.KeyAccess`

The `.KeyAccess` wrapper provides `findKeys()` for simple selection of active
keys.

To select all keys beginning with a particular prefix:

```rexx
keys = db.findKeys("type:", "ONLY")

do i = 1 to keys[0]
    say keys[i]
end
```

For example, a store containing:

```text
type:.Module01
type:.Module02
type:.Module03
ref:calling:.Module01:.Module02
```

would return only the three `type:` keys.

To select keys containing text anywhere in the key:

```rexx
keys = db.findKeys(".Module17", "ANY")

do i = 1 to keys[0]
    say keys[i]
end
```

This is particularly useful when applications use structured keys. For
example, given:

```text
type:.Module17
ref:calling:.Module16:.Module17
ref:calling:.Module17:.Module18
ref:calling:.Module17:.MissingModule
```

the call:

```rexx
keys = db.findKeys(".Module17", "ANY")
```

selects all four keys because `.Module17` occurs somewhere in each key.

A more selective query can take advantage of the application's key structure.
For example:

```rexx
keys = db.findKeys("ref:calling:", "ONLY")
```

selects all `calling` reference records.

`findKeys()` does not provide a secondary index. Both `ONLY` and `ANY`
selection are implemented by sequential traversal of the active keys.


## Version History

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2024-03-20 | Initial documentation |
| 1.1 | 2026-08-25 | Added sequential active-key traversal and documented the KeyAccess/`.KeyAccess` API relationship |
| 1.2 | 2026-08-26 | Added `.KeyAccess.findKeys()` with `ONLY` and `ANY` key selection |
| 1.3 | 2026-08-26 | Added lazy in-memory hash indexing with incremental write/delete maintenance |
| 1.4 | 2026-08-26 | Improved negative lookup handling; `NOT_FOUND` is no longer logged as an operational error |
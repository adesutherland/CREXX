# KeyAccess Direct-Access Storage Documentation

## Overview

KeyAccess is a lightweight, self-contained direct-access storage facility for
persistent keyed data. It is intended for small embedded applications that
require key-based access without an external database system.

KeyAccess provides persistent key-value storage, transaction handling,
sequential key traversal, caching, locking, and storage maintenance operations.

It is a direct-access storage facility, not a database management system.

KeyAccess has two API layers:

- The **keyaccess plugin** provides the low-level native storage functions.
- The **`.KeyAccess` cREXX class** provides an object-oriented wrapper around the
  native KeyAccess functions.

This document primarily describes the native KeyAccess interface. Where useful,
the corresponding `.KeyAccess` methods are also shown.

## Table of Contents

- [Features](#features)
- [File Structure](#file-structure)
- [API Layers](#api-layers)
- [Available Functions](#available-functions)
- [Error Codes](#error-codes)
- [Usage Examples](#usage-examples)
- [Version History](#version-history)

## Features

- Persistent keyed storage with CRUD operations
- Transaction support (`begin`, `commit`, `rollback`)
- Sequential traversal of active keys
- LRU cache for improved read performance
- File locking for concurrent access
- Storage maintenance (`backup`, `compact`, `validate`)
- Error logging and statistics
- No external database or server dependency

### Lookup Performance

KeyAccess builds an in-memory hash index lazily on the first keyed read. The
index stores a 64-bit hash and the corresponding index-file offset. A matching
hash is always verified with the complete key, so hash collisions do not change
the lookup result. The index is discarded and rebuilt after writes, deletes,
rollback, compaction, and reopening the store.

The handle-local LRU cache uses the same hash-directed lookup strategy. Cache
collisions are resolved by comparing the complete key. A cache miss continues
to the authoritative on-disk index; the cache does not change storage
semantics or durability.

The first keyed lookup may therefore include the cost of scanning the index to
build the in-memory structure. Subsequent lookups avoid that scan while the
handle remains open. `listkey` and sequential traversal continue to read the
index records directly and are unaffected by the lookup cache.

## File Structure

A KeyAccess store uses separate files for data and index information.

- **Data file**: Stores the actual key-value data.
- **Index file**: Stores key locations and associated index metadata.
- **Log file**: Records operations and errors where logging is enabled.

The index may contain deleted records. Normal keyed access and sequential
traversal expose only active records.

## API Layers

The low-level KeyAccess plugin functions are wrapped by the `.KeyAccess` cREXX
class.

Typical mappings are:

| Native function | `.KeyAccess` method |
|-----------------|-----------------|
| `openkey` | `open()` |
| `closekey` | `close()` |
| `readkey` | `get()` |
| `writekey` | `put()` |
| `deletekey` | `remove()` |
| `firstkey` | `firstKey()` |
| `nextkey` | `nextKey()` |

The native interface operates on an integer storage handle. The `.KeyAccess` class
owns this handle and provides the corresponding operations as methods.

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

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

Integer count of active keys.

**Example:**

```rexx
count = listkey(handle)
```

The `.KeyAccess` wrapper also provides `clear()`, which removes all active keys in
one transaction and returns the transaction status code.

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

### Transaction Management

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
including index records modified by updates or deletes. Cached values are
invalidated after a successful rollback.

**Parameters:**

- `handle` (int): Storage handle.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = txrollback(handle)
```

### Storage Maintenance

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

The `.KeyAccess` cREXX wrapper exposes the traversal operations as `firstKey()` and
`nextKey()`:

```rexx
key = db.firstKey()

do while key <> "NOT_FOUND"

    value = db.get(key)
    say key value

    key = db.nextKey()

end
```

The traversal cursor belongs to the underlying KeyAccess handle. Index
operations performed by `get()`, `containsKey()`, or other `.KeyAccess` methods do
not change the next key returned by `nextKey()`.

Calling `firstKey()` again restarts traversal.

## Version History

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2024-03-20 | Initial documentation |
| 1.1 | 2026-08-25 | Added sequential active-key traversal and documented the KeyAccess/`.KeyAccess` API relationship |

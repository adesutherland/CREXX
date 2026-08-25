# Key-Value Database Documentation

## Overview

KeyDB is a basic, self-contained key-value database system providing persistent
key-value storage, transaction handling, sequential key traversal, caching, and
database maintenance operations.

The database operates independently and does not require an external database
management system.

KeyDB consists of two API layers:

- The **keyaccess plugin** provides the low-level native database functions.
- The **`.KeyDB` cREXX class** provides an object-oriented wrapper around the
  native functions.

This document primarily describes the native keyaccess interface. Where useful,
the corresponding `.KeyDB` methods are also shown.

## Table of Contents

- [Features](#features)
- [File Structure](#file-structure)
- [API Layers](#api-layers)
- [Available Functions](#available-functions)
- [Error Codes](#error-codes)
- [Usage Examples](#usage-examples)
- [Version History](#version-history)

## Features

- Key-value storage with CRUD operations
- Transaction support (`begin`, `commit`, `rollback`)
- Sequential traversal of active keys
- LRU cache for improved read performance
- File locking for concurrent access
- Database maintenance (`backup`, `compact`, `validate`)
- Error logging and statistics

## File Structure

A KeyDB database uses separate files for data and index information.

- **Data file**: Stores the actual key-value data.
- **Index file**: Stores key locations and associated index metadata.
- **Log file**: Records operations and errors where logging is enabled.

The index may contain deleted records. Normal key lookup and sequential
traversal expose only active records.

## API Layers

The low-level keyaccess plugin functions are wrapped by the `.KeyDB` class.

Typical mappings are:

| Native function | `.KeyDB` method |
|-----------------|-----------------|
| `openkey` | `open()` |
| `closekey` | `close()` |
| `readkey` | `get()` |
| `writekey` | `put()` |
| `deletekey` | `remove()` |
| `firstkey` | `firstKey()` |
| `nextkey` | `nextKey()` |

The native interface operates on an integer database handle. The `.KeyDB`
class owns this handle and provides the corresponding operations as methods.

## Available Functions

### Database Operations

#### openkey

Opens or creates a key-value database.

When using the `.KeyDB` wrapper, calling `open()` on an already-open object
closes the previous handle before opening the new database.

**Parameters:**

- `filename` (string): Path to the database file.
- `mode` (string): File access mode, for example `"r"`, `"w"`, or `"w+"`.

**Returns:**

Integer handle for the opened database, or a negative error code.

**Example:**

```rexx
handle = openkey("mydb.dat", "w+")
```

---

#### closekey

Closes an open database.

**Parameters:**

- `handle` (int): Database handle returned by `openkey`.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = closekey(handle)
```

---

#### writekey

Writes a key-value pair to the database.

If the key already exists, its active index record is replaced; a second
active record is not created.

**Parameters:**

- `handle` (int): Database handle.
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

- `handle` (int): Database handle.
- `key` (string): Key to read.

**Returns:**

String value or an error/not-found indication.

**Example:**

```rexx
value = readkey(handle, "user.name")
```

---

#### deletekey

Deletes a key-value pair from the database.

**Parameters:**

- `handle` (int): Database handle.
- `key` (string): Key to delete.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = deletekey(handle, "user.name")
```

---

#### listkey

Returns the number of active keys in the database.

Deleted index records are not included in the count.

**Parameters:**

- `handle` (int): Database handle.

**Returns:**

Integer count of active keys.

**Example:**

```rexx
count = listkey(handle)
```

The `.KeyDB` wrapper also provides `clear()`, which removes all active keys in
one transaction and returns the transaction status code.

---

#### firstkey

Starts sequential traversal of the database index and returns the first active
key.

Deleted index records are skipped.

Traversal state is maintained independently on the database handle and does
not depend on the current `FILE` position used internally by other database
operations.

Calling `firstkey()` restarts sequential traversal for that handle.

**Parameters:**

- `handle` (int): Database handle.

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

Other database operations such as `readkey` may access or reposition the index
file without disturbing the traversal cursor.

**Parameters:**

- `handle` (int): Database handle.

**Returns:**

- The next active key.
- `"NOT_FOUND"` when the end of traversal has been reached.
- `"ERROR"` for an invalid handle or traversal failure.

**Example:**

```rexx
key = nextkey(handle)
```

A database handle has one sequential traversal cursor. Calling `firstkey()`
again restarts traversal for that handle.

### Transaction Management

#### txbegin

Begins a new transaction.

**Parameters:**

- `handle` (int): Database handle.

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

- `handle` (int): Database handle.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = txcommit(handle)
```

---

#### txrollback

Rolls back the current transaction.

Rollback restores the data and index files to their state at
`txbegin`, including index records modified by updates or deletes. Cached
values are invalidated after a successful rollback.

**Parameters:**

- `handle` (int): Database handle.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = txrollback(handle)
```

### Database Maintenance

#### stats

Retrieves database statistics.

**Parameters:**

- `handle` (int): Database handle.

**Returns:**

String containing database statistics.

**Example:**

```rexx
databaseStats = stats(handle)
say databaseStats
```

---

#### backup

Creates a backup of the database.

**Parameters:**

- `handle` (int): Database handle.
- `path` (string): Backup file path.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = backup(handle, "backup.dat")
```

---

#### validate

Validates database integrity.

**Parameters:**

- `handle` (int): Database handle.

**Returns:**

Integer status indicating the number of errors found.

**Example:**

```rexx
errors = validate(handle)
```

---

#### compact

Compacts the database by removing deleted entries.

**Parameters:**

- `handle` (int): Database handle.

**Returns:**

Integer status code.

**Example:**

```rexx
rc = compact(handle)
```

## Error Codes

The native API uses the following status codes:

| Code | Name | Meaning |
|-----:|------|---------|
| 0 | `KA_SUCCESS` | Operation successful |
| -1 | `KA_ERROR_PARAM` | Invalid parameters |
| -2 | `KA_ERROR_MEMORY` | Memory allocation failed |
| -3 | `KA_ERROR_IO` | I/O operation failed |
| -4 | `KA_ERROR_LOCK` | File locking failed |
| -5 | `KA_ERROR_NOTFOUND` | Key not found |
| -6 | `KA_ERROR_EXISTS` | Key already exists |
| -7 | `KA_ERROR_CORRUPT` | Database corruption detected |
| -8 | `KA_ERROR_TXACTIVE` | Transaction already active |
| -9 | `KA_ERROR_TXINACTIVE` | No active transaction |
| -10 | `KA_ERROR_TOOLONG` | Key or value too long |

Functions returning strings cannot directly return these integer error codes.
In particular, the sequential traversal functions use `"NOT_FOUND"` to
indicate the end of traversal and `"ERROR"` to indicate a traversal failure.

## Usage Examples

### Basic Key-Value Operations

```rexx
/* Open database */
handle = openkey("users.dat", "w+")

/* Write a value */
call writekey handle, "user.name", "John Doe"

/* Read the value */
name = readkey(handle, "user.name")
say "User name:" name

/* Close database */
call closekey handle
```

### Transaction Example

```rexx
/* Open database */
handle = openkey("users.dat", "w+")

/* Start transaction */
call txbegin handle

/* Write multiple values */
call writekey handle, "user.1.name", "John Doe"
call writekey handle, "user.1.email", "john@example.com"

/* Commit changes */
call txcommit handle

/* Get statistics */
databaseStats = stats(handle)
say databaseStats

/* Close database */
call closekey handle
```

### Database Maintenance Example

```rexx
/* Open database */
handle = openkey("users.dat", "w+")

/* Create backup */
call backup handle, "users.backup.dat"

/* Validate database */
errors = validate(handle)

if errors > 0 then do
    say "Found" errors "errors in database"

    /* Compact to remove deleted entries */
    call compact handle
end

/* Close database */
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

### Sequential Key Traversal Using `.KeyDB`

The `.KeyDB` class exposes the traversal operations as `firstKey()` and
`nextKey()`:

```rexx
key = db.firstKey()

do while key <> "NOT_FOUND"

    value = db.get(key)
    say key value

    key = db.nextKey()

end
```

The traversal cursor belongs to the database handle. Index operations performed
by `get()`, `containsKey()`, or other database methods do not change the next
key returned by `nextKey()`.

Calling `firstKey()` again restarts traversal.

## Version History

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2024-03-20 | Initial documentation |
| 1.1 | 2026-08-25 | Added sequential active-key traversal and documented the native/`.KeyDB` API relationship |

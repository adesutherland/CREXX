# Key-Value Database Documentation

## Overview
This document describes the implementation of a basic key-value database system, including operations for database management, transaction handling, and maintenance. The system is self-contained and operates independently without requiring additional software, which imposes certain limitations.
## Table of Contents
- [Features](#features)
- [File Structure](#file-structure)
- [Available Functions](#available-functions)
- [Error Codes](#error-codes)
- [Usage Examples](#usage-examples)

## Features
- Key-value storage with CRUD operations
- Transaction support (begin, commit, rollback)
- LRU cache for improved read performance
- File locking for concurrent access
- Database maintenance (backup, compact, validate)
- Error logging and statistics

## File Structure
- **Data file**: Stores actual key-value data
- **Index file**: Maintains key locations and metadata
- **Log file**: Records operations and errors

## Available Functions

### Database Operations

#### openkey
Opens or creates a key-value database file
- **Parameters:**
  - filename (string): Path to the database file
  - mode (string): File access mode ("r" for read, "w" for write, "w+" for read/write)
- **Returns:** Integer handle for the opened database
- **Example:**
rexx
handle = openkey("mydb.dat", "w+")

#### closekey
Closes an open database
- **Parameters:**
  - handle (int): Database handle from openkey
- **Returns:** Integer status code
- **Example:**
```rexx
rc = closekey(handle)
```

#### writekey
Writes a key-value pair to the database
- **Parameters:**
  - handle (int): Database handle
  - key (string): Key to write
  - value (string): Value to store
- **Returns:** Integer status code
- **Example:**
```rexx
rc = writekey(handle, "user.name", "John Doe")
```

#### readkey
Reads a value by key from the database
- **Parameters:**
  - handle (int): Database handle
  - key (string): Key to read
- **Returns:** String value or error message
- **Example:**
```rexx
value = readkey(handle, "user.name")
```

#### deletekey
Deletes a key-value pair from the database
- **Parameters:**
  - handle (int): Database handle
  - key (string): Key to delete
- **Returns:** Integer status code
- **Example:**
```rexx
rc = deletekey(handle, "user.name")
```

#### listkey
Lists the number of keys in the database
- **Parameters:**
  - handle (int): Database handle
- **Returns:** Integer count of keys
- **Example:**
```rexx
count = listkey(handle)
```

### Transaction Management

#### txbegin
Begins a new transaction
- **Parameters:**
  - handle (int): Database handle
- **Returns:** Integer status code
- **Example:**
```rexx
rc = txbegin(handle)
```

#### txcommit
Commits the current transaction
- **Parameters:**
  - handle (int): Database handle
- **Returns:** Integer status code
- **Example:**
```rexx
rc = txcommit(handle)
```

#### txrollback
Rolls back the current transaction
- **Parameters:**
  - handle (int): Database handle
- **Returns:** Integer status code
- **Example:**
```rexx
rc = txrollback(handle)
```

### Database Maintenance

#### stats
Retrieves database statistics
- **Parameters:**
  - handle (int): Database handle
- **Returns:** String containing statistics
- **Example:**
```rexx
stats = stats(handle)
```

#### backup
Creates a backup of the database
- **Parameters:**
  - handle (int): Database handle
  - path (string): Backup file path
- **Returns:** Integer status code
- **Example:**
```rexx
rc = backup(handle, "backup.dat")
```

#### validate
Validates database integrity
- **Parameters:**
  - handle (int): Database handle
- **Returns:** Integer status code (number of errors found)
- **Example:**
```rexx
errors = validate(handle)
```

#### compact
Compacts the database by removing deleted entries
- **Parameters:**
  - handle (int): Database handle
- **Returns:** Integer status code
- **Example:**
```rexx
rc = compact(handle)
```

## Error Codes
- KA_SUCCESS (0): Operation successful
- KA_ERROR_PARAM (-1): Invalid parameters
- KA_ERROR_MEMORY (-2): Memory allocation failed
- KA_ERROR_IO (-3): I/O operation failed
- KA_ERROR_LOCK (-4): File locking failed
- KA_ERROR_NOTFOUND (-5): Key not found
- KA_ERROR_EXISTS (-6): Key already exists
- KA_ERROR_CORRUPT (-7): Database corruption detected
- KA_ERROR_TXACTIVE (-8): Transaction already active
- KA_ERROR_TXINACTIVE (-9): No active transaction
- KA_ERROR_TOOLONG (-10): Key or value too long

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
stats = stats(handle)
say stats

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

## Version History
| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2024-03-20 | Initial documentation |

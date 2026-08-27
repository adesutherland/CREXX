/**
 * @file fileio.c
 * @brief Key-Value Database Plugin for crexx/pa
 *
 * This module implements a transactional key-value database with the following features:
 * - Key-value storage with CRUD operations
 * - Transaction support (begin, commit, rollback)
 * - LRU cache for improved read performance
 * - File locking for concurrent access
 * - Database maintenance (backup, compact, validate)
 * - Error logging and statistics
 *
 * File Structure:
 * - Data file: Stores actual key-value data
 * - Index file: Maintains key locations and metadata
 * - Log file: Records operations and errors
 *
 * REXX Interface Functions:
 * - keyaccess.openkey(filename, mode) -> handle
 * - keyaccess.closekey(handle) -> rc
 * - keyaccess.writekey(handle, key, value) -> rc
 * - keyaccess.readkey(handle, key) -> value
 * - keyaccess.deletekey(handle, key) -> rc
 * - keyaccess.listkey(handle) -> count
 * - keyaccess.firstkey(handle) -> first active key
 * - keyaccess.nextkey(handle) -> next active key
 * - keyaccess.txbegin(handle) -> rc
 * - keyaccess.txcommit(handle) -> rc
 * - keyaccess.txrollback(handle) -> rc
 * - keyaccess.stats(handle) -> string
 * - keyaccess.backup(handle, path) -> rc
 * - keyaccess.validate(handle) -> rc
 * - keyaccess.compact(handle) -> rc
 *
 * Error Codes:
 * - KA_SUCCESS (0): Operation successful
 * - KA_ERROR_PARAM (-1): Invalid parameters
 * - KA_ERROR_MEMORY (-2): Memory allocation failed
 * - KA_ERROR_IO (-3): I/O operation failed
 * - KA_ERROR_LOCK (-4): File locking failed
 * - KA_ERROR_NOTFOUND (-5): Key not found
 * - KA_ERROR_EXISTS (-6): Key already exists
 * - KA_ERROR_CORRUPT (-7): Database corruption detected
 * - KA_ERROR_TXACTIVE (-8): Transaction already active
 * - KA_ERROR_TXINACTIVE (-9): No active transaction
 * - KA_ERROR_TOOLONG (-10): Key or value too long
 *
 * Example Usage:
 * ```rexx
 * handle = keyaccess.openkey("mydb.dat", "w+")
 * call keyaccess.txbegin handle
 * call keyaccess.writekey handle, "name", "John Doe"
 * call keyaccess.txcommit handle
 * value = keyaccess.readkey(handle, "name")
 * call keyaccess.closekey handle
 * ```
 *
 * @author Peter Jacobs
 * @date 2024-03-20
 * @version 1.0
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include "crexxpa.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
    #include <sys/file.h>
#endif

// Constants and definitions
#define MAX_KEY_LENGTH 256
#define MAX_VALUE_LENGTH 4096
#define MAX_LINE_LENGTH (MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 2)
#define LOG_FILENAME "keyaccess.log"
#define CACHE_SIZE 1024
#define CACHE_BUCKET_COUNT 1024

// Error codes
#define KA_SUCCESS           0
#define KA_ERROR_PARAM      -1  // Invalid parameters
#define KA_ERROR_MEMORY     -2  // Memory allocation failed
#define KA_ERROR_IO         -3  // I/O operation failed
#define KA_ERROR_LOCK       -4  // File locking failed
#define KA_ERROR_NOTFOUND   -5  // Key not found
#define KA_ERROR_EXISTS     -6  // Key already exists
#define KA_ERROR_CORRUPT    -7  // Database corruption detected
#define KA_ERROR_TXACTIVE   -8  // Transaction already active
#define KA_ERROR_TXINACTIVE -9  // No active transaction
#define KA_ERROR_TOOLONG    -10 // Key or value too long

// Error message structure
struct ErrorMessage {
    int code;
    const char* message;
} error_messages[] = {
        {KA_SUCCESS,      "Operation successful"},
        {KA_ERROR_PARAM,  "Invalid parameters"},
        {KA_ERROR_MEMORY, "Memory allocation failed"},
        {KA_ERROR_IO,     "I/O operation failed"},
        {KA_ERROR_LOCK,   "File locking failed"},
        {KA_ERROR_NOTFOUND, "Key not found"},
        {KA_ERROR_EXISTS,   "Key already exists"},
        {KA_ERROR_CORRUPT,  "Database corruption detected"},
        {KA_ERROR_TXACTIVE, "Transaction already active"},
        {KA_ERROR_TXINACTIVE, "No active transaction"},
        {KA_ERROR_TOOLONG,   "Key or value too long"}
};

// Cache entry structure
struct CacheEntry {
    char key[MAX_KEY_LENGTH];
    uint64_t hash;
    char* value;
    time_t timestamp;
    int hits;
};

// Statistics structure
struct Statistics {
    unsigned long reads;
    unsigned long writes;
    unsigned long deletes;
    unsigned long transactions;
    time_t startTime;
    size_t totalBytesWritten;
    size_t totalBytesRead;
};

struct HashIndexEntry {
    uint64_t hash;
    long indexOffset;
    int next;
};

struct HashIndex {
    int* buckets;
    size_t bucketCount;
    struct HashIndexEntry* entries;
    size_t entryCount;
    size_t entryCapacity;
    int built;
};

// File handle structure
struct FileHandle {
    FILE* dataFile;     // Main data file
    FILE* indexFile;    // Index file
    FILE* logFile;      // Transaction log
    char* dataPath;     // Path to data file
    char* indexPath;    // Path to index file
    int locked;         // File lock status
    int transaction;    // Transaction status
    long indexCursor;   // Explicit index traversal offset
    long transactionDataSize;
    long transactionIndexSize;
    unsigned char* transactionIndexSnapshot;
    struct HashIndex hashIndex;
    struct CacheEntry cache[CACHE_SIZE];
    int cacheBuckets[CACHE_BUCKET_COUNT];
    int cacheNext[CACHE_SIZE];
    int cacheCount;
    unsigned long cacheHits;
    unsigned long cacheMisses;
    struct Statistics stats;
};

// Index record structure
struct IndexRecord {
    char key[MAX_KEY_LENGTH];
    long offset;        // Offset in data file
    int length;         // Length of value
    char deleted;       // Deletion flag
    int version;        // Version number
    time_t timestamp;   // Last modified
};

// Function prototypes
static void log_error(const char* operation, int error_code, const char* details);
static int lock_file(struct FileHandle* handle);
static int unlock_file(struct FileHandle* handle);
static int truncate_file(FILE* file, long size);
static const char* binary_file_mode(const char* mode);
static char* create_index_path(const char* dataPath);
static void cache_init(struct FileHandle* handle);
static void cache_clear(struct FileHandle* handle);
static int restore_transaction(struct FileHandle* handle);
static void hash_index_discard(struct FileHandle* handle);
static int find_key_indexed(struct FileHandle* handle, const char* searchKey,
                            struct IndexRecord* result, long* resultOffset);
static int hash_index_remove(struct FileHandle* handle, uint64_t hash,
                             long indexOffset);
static int hash_index_add(struct FileHandle* handle, uint64_t hash,
                           long indexOffset);
static uint64_t hash_key(const char* key);
static size_t cache_bucket(uint64_t hash);
static void cache_put(struct FileHandle* handle, const char* key, const char* value);
static char* cache_get(struct FileHandle* handle, const char* key);
static int write_record(struct FileHandle* handle, const struct IndexRecord* record, const char* value);

static const char* binary_file_mode(const char* mode) {
    if (strcmp(mode, "r") == 0) {
        return "rb";
    }
    if (strcmp(mode, "w") == 0) {
        return "wb";
    }
    /*
     * "w+" is handled specially by openfile()
     * because CREXX semantics preserve an existing database.
     */
    return mode;
}


// Error logging
static void log_error(const char* operation, int error_code, const char* details) {
    FILE* log = fopen(LOG_FILENAME, "a");
    int i;
    if (log) {
        time_t now = time(NULL);
        char timestamp[26];
#ifdef _WIN32
        ctime_s(timestamp, sizeof(timestamp), &now);
#else
        ctime_r(&now, timestamp);
#endif
        timestamp[24] = '\0';  // Remove newline

        const char* error_msg = "Unknown error";
        for (i = 0; i < sizeof(error_messages)/sizeof(error_messages[0]); i++) {
            if (error_messages[i].code == error_code) {
                error_msg = error_messages[i].message;
                break;
            }
        }

        fprintf(log, "[%s] Operation: %s, Error: %d (%s), Details: %s\n",
                timestamp, operation, error_code, error_msg,
                details ? details : "No additional details");

        if (errno != 0) {
            fprintf(log, "System error: %s\n", strerror(errno));
        }

        fclose(log);
    }
}

/**
 * Remove a key from the handle-local cache.
 */
static void cache_remove(struct FileHandle *handle, const char *key)
{
    uint64_t hash = hash_key(key);
    size_t bucket = cache_bucket(hash);
    int entry = handle->cacheBuckets[bucket];
    int previous = -1;

    while (entry >= 0) {
        if (handle->cache[entry].value != NULL &&
            handle->cache[entry].hash == hash &&
            strcmp(handle->cache[entry].key, key) == 0) {
            if (previous < 0) {
                handle->cacheBuckets[bucket] = handle->cacheNext[entry];
            } else {
                handle->cacheNext[previous] = handle->cacheNext[entry];
            }
            free(handle->cache[entry].value);
            handle->cache[entry].value = NULL;
            handle->cache[entry].key[0] = '\0';
            handle->cache[entry].hash = 0;
            handle->cache[entry].timestamp = 0;
            handle->cache[entry].hits = 0;
            handle->cacheNext[entry] = -1;
            return;
        }
        previous = entry;
        entry = handle->cacheNext[entry];
    }
}

PROCEDURE(openfile) {
    char* filename = GETSTRING(ARG0);
    char* mode = GETSTRING(ARG1);
    const char* file_mode;
    struct FileHandle* handle;

    if (!filename || !mode) {
        log_error("openfile", KA_ERROR_PARAM, "NULL filename or mode");
        RETURNINTX(KA_ERROR_PARAM);
    }

    /*
     * Index records and data offsets are binary, so Windows
     * text mode is unsafe.
     *
     * Note:
     * CREXX "w+" is handled specially below because standard
     * C "w+b" truncates an existing file.
     */
    file_mode = binary_file_mode(mode);

    handle = (struct FileHandle*)malloc(sizeof(struct FileHandle));
    if (!handle) {
        log_error("openfile", KA_ERROR_MEMORY, "Failed to allocate handle");
        RETURNINTX(KA_ERROR_MEMORY);
    }

    /* Initialize handle structure. */
    memset(handle, 0, sizeof(struct FileHandle));

    handle->dataPath = strdup(filename);
    handle->indexPath = create_index_path(filename);

    if (!handle->dataPath || !handle->indexPath) {
        log_error("openfile",
                  KA_ERROR_MEMORY,
                  "Failed to allocate path strings");

        free(handle->dataPath);
        free(handle->indexPath);
        free(handle);

        RETURNINTX(KA_ERROR_MEMORY);
    }

    /*
     * Open data file.
     *
     * CREXX "w+" semantics:
     *
     * - open an existing file read/write
     * - preserve existing contents
     * - create the file if it does not yet exist
     *
     * Standard C "w+b" cannot be used directly for an existing
     * file because it truncates the file.
     */
    if (strcmp(mode, "w+") == 0) {
        handle->dataFile = fopen(handle->dataPath, "r+b");

        if (!handle->dataFile) {
            handle->dataFile = fopen(handle->dataPath, "w+b");
        }
    }
    else {
        handle->dataFile = fopen(handle->dataPath, file_mode);
    }

    if (!handle->dataFile) {
        log_error("openfile",
                  KA_ERROR_IO,
                  "Failed to open data file");

        free(handle->dataPath);
        free(handle->indexPath);
        free(handle);

        RETURNINTX(KA_ERROR_IO);
    }

    /*
     * Open index file using the same preserve-or-create semantics.
     */
    if (strcmp(mode, "w+") == 0) {
        handle->indexFile = fopen(handle->indexPath, "r+b");

        if (!handle->indexFile) {
            handle->indexFile = fopen(handle->indexPath, "w+b");
        }
    }
    else {
        handle->indexFile = fopen(handle->indexPath, file_mode);
    }

    if (!handle->indexFile) {
        log_error("openfile",
                  KA_ERROR_IO,
                  "Failed to open index file");

        fclose(handle->dataFile);

        free(handle->dataPath);
        free(handle->indexPath);
        free(handle);

        RETURNINTX(KA_ERROR_IO);
    }

    /* Initialize cache. */
    cache_init(handle);

    /* Initialize statistics. */
    handle->stats.startTime = time(NULL);
    handle->stats.reads = 0;
    handle->stats.writes = 0;
    handle->stats.deletes = 0;
    handle->stats.transactions = 0;
    handle->stats.totalBytesRead = 0;
    handle->stats.totalBytesWritten = 0;

    /* Return handle as integer. */
    RETURNINTX((intptr_t)handle);

    ENDPROC
}

PROCEDURE(writekey) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    char* key = GETSTRING(ARG1);
    char* value = GETSTRING(ARG2);
    struct IndexRecord record;
    long dataOffset;
    long indexOffset = -1;
    int lookupResult;
    int isNew = 0;

    if (!handle || !key || !value) {
        log_error("writekey", KA_ERROR_PARAM, "Invalid parameters");
        RETURNINTX(KA_ERROR_PARAM);
    }

    if (strlen(key) >= MAX_KEY_LENGTH || strlen(value) >= MAX_VALUE_LENGTH) {
        log_error("writekey", KA_ERROR_TOOLONG, "Key or value too long");
        RETURNINTX(KA_ERROR_TOOLONG);
    }

    if (!handle->transaction) {
        log_error("writekey", KA_ERROR_TXINACTIVE, "Write attempted outside transaction");
        RETURNINTX(KA_ERROR_TXINACTIVE);
    }

    /* Use the lazy hash index for existence checks.  It is built once per
     * handle state and then maintained as this transaction adds records. */
    lookupResult = find_key_indexed(handle, key, &record, &indexOffset);
    if (lookupResult == KA_SUCCESS) {
        record.version++;
        record.deleted = 0;
    } else if (lookupResult == KA_ERROR_NOTFOUND) {
        memset(&record, 0, sizeof(record));
        strncpy(record.key, key, MAX_KEY_LENGTH - 1);
        record.version = 1;
        isNew = 1;
    } else {
        log_error("writekey", lookupResult, "Failed to find key in index");
        RETURNINTX(lookupResult);
    }

    // Write data
    if (fseek(handle->dataFile, 0, SEEK_END) != 0) {
        log_error("writekey", KA_ERROR_IO, "Failed to seek data file");
        RETURNINTX(KA_ERROR_IO);
    }
    dataOffset = ftell(handle->dataFile);
    if (dataOffset < 0) {
        log_error("writekey", KA_ERROR_IO, "Failed to determine data offset");
        RETURNINTX(KA_ERROR_IO);
    }

    record.offset = dataOffset;
    record.length = strlen(value);
    record.timestamp = time(NULL);

    if (indexOffset < 0) {
        if (fseek(handle->indexFile, 0, SEEK_END) != 0) {
            log_error("writekey", KA_ERROR_IO, "Failed to seek index file");
            RETURNINTX(KA_ERROR_IO);
        }
        indexOffset = ftell(handle->indexFile);
        if (indexOffset < 0) {
            log_error("writekey", KA_ERROR_IO, "Failed to determine index offset");
            RETURNINTX(KA_ERROR_IO);
        }
    }

    if (!isNew) {
        if (fwrite(value, 1, strlen(value), handle->dataFile) != strlen(value)) {
            log_error("writekey", KA_ERROR_IO, "Failed to write value to data file");
            RETURNINTX(KA_ERROR_IO);
        }

        /* Replace an existing index record instead of appending a duplicate. */
        if (fseek(handle->indexFile, indexOffset, SEEK_SET) != 0 ||
            fwrite(&record, sizeof(record), 1, handle->indexFile) != 1) {
            log_error("writekey", KA_ERROR_IO, "Failed to replace index record");
            RETURNINTX(KA_ERROR_IO);
        }
    }
    else if (write_record(handle, &record, value) != KA_SUCCESS) {
        RETURNINTX(KA_ERROR_IO);
    }

    if (isNew && hash_index_add(handle, hash_key(key), indexOffset) != KA_SUCCESS) {
        hash_index_discard(handle);
        log_error("writekey", KA_ERROR_MEMORY, "Failed to update hash index");
        RETURNINTX(KA_ERROR_MEMORY);
    }

    // Update statistics
    handle->stats.writes++;
    handle->stats.totalBytesWritten += strlen(value);

    // Update cache
    cache_put(handle, key, value);

    RETURNINTX(KA_SUCCESS);
    ENDPROC
}

PROCEDURE(readkey) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    char* key = GETSTRING(ARG1);
    struct IndexRecord record;
    char* value;
    char* cached;
    int lookupResult;

    if (!handle || !key) {
        log_error("readkey", KA_ERROR_PARAM, "Invalid parameters");
        RETURNSTRX("ERROR");
    }

    // Check cache first
    cached = cache_get(handle, key);
    if (cached) {
        RETURNSTRX(cached);
    }

    // Look up in index
    lookupResult = find_key_indexed(handle, key, &record, NULL);
    if (lookupResult == KA_ERROR_NOTFOUND) {
        RETURNSTRX("NOT_FOUND");
    }
    if (lookupResult != KA_SUCCESS) {
        log_error("readkey", lookupResult, "Failed to find key in index");
        RETURNSTRX("ERROR");
    }

    // Read value
    value = (char*)malloc(record.length + 1);
    if (!value) {
        log_error("readkey", KA_ERROR_MEMORY, "Failed to allocate value buffer");
        RETURNSTRX("ERROR");
    }

    if (fseek(handle->dataFile, record.offset, SEEK_SET) != 0 ||
        fread(value, 1, record.length, handle->dataFile) != (size_t)record.length) {
        free(value);
        log_error("readkey", KA_ERROR_IO, "Failed to read value");
        RETURNSTRX("ERROR");
    }
    value[record.length] = '\0';

    // Update statistics
    handle->stats.reads++;
    handle->stats.totalBytesRead += record.length;

    // Update cache
    cache_put(handle, key, value);

    SETSTRING(RETURN, value);
    free(value);
    PROCRETURN
    ENDPROC
}

PROCEDURE(deletekey) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    char* key = GETSTRING(ARG1);
    struct IndexRecord record;
    long indexOffset;
    int lookupResult;

    if (!handle || !key) {
        log_error("deletekey", KA_ERROR_PARAM, "Invalid parameters");
        RETURNINTX(KA_ERROR_PARAM);
    }

    if (!handle->transaction) {
        log_error("deletekey", KA_ERROR_TXINACTIVE, "Delete attempted outside transaction");
        RETURNINTX(KA_ERROR_TXINACTIVE);
    }

    lookupResult = find_key_indexed(handle, key, &record, &indexOffset);
    if (lookupResult == KA_ERROR_NOTFOUND) {
        log_error("deletekey", KA_ERROR_NOTFOUND, "Key not found");
        RETURNINTX(KA_ERROR_NOTFOUND);
    }
    if (lookupResult != KA_SUCCESS) {
        log_error("deletekey", lookupResult, "Failed to find key in index");
        RETURNINTX(lookupResult);
    }

    // Mark as deleted
    record.deleted = 1;
    record.timestamp = time(NULL);

    // Write updated record
    if (fseek(handle->indexFile, indexOffset, SEEK_SET) != 0) {
        log_error("deletekey", KA_ERROR_IO, "Failed to seek index record");
        RETURNINTX(KA_ERROR_IO);
    }
    if (fwrite(&record, sizeof(record), 1, handle->indexFile) != 1) {
        log_error("deletekey", KA_ERROR_IO, "Failed to update index");
        RETURNINTX(KA_ERROR_IO);
    }

    cache_remove(handle, key);
    if (hash_index_remove(handle, hash_key(key), indexOffset) != KA_SUCCESS) {
        hash_index_discard(handle);
    }
    // Update statistics
    handle->stats.deletes++;

    RETURNINTX(KA_SUCCESS);
    ENDPROC
}

PROCEDURE(listkeys) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    struct IndexRecord record;
    int count = 0;

    if (!handle) {
        log_error("listkeys", KA_ERROR_PARAM, "Invalid handle");
        RETURNINTX(KA_ERROR_PARAM);
    }

    rewind(handle->indexFile);
    while (fread(&record, sizeof(record), 1, handle->indexFile) == 1) {
        if (!record.deleted) {
            count++;
        }
    }

    RETURNINTX(count);
    ENDPROC
}

/**
 * Return the first non-deleted key and reset the handle-local traversal.
 *
 * The cursor is maintained separately from indexFile's FILE position because
 * lookups and other database operations seek on the same stream.
 */
PROCEDURE(firstkey) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    struct IndexRecord record;

    if (!handle) {
        log_error("firstkey", KA_ERROR_PARAM, "Invalid handle");
        RETURNSTRX("ERROR");
    }

    handle->indexCursor = 0;
    while (1) {
        if (fseek(handle->indexFile, handle->indexCursor, SEEK_SET) != 0) {
            log_error("firstkey", KA_ERROR_IO, "Failed to seek index file");
            RETURNSTRX("ERROR");
        }

        if (fread(&record, sizeof(record), 1, handle->indexFile) != 1) {
            if (feof(handle->indexFile)) {
                RETURNSTRX("NOT_FOUND");
            }
            log_error("firstkey", KA_ERROR_IO, "Failed to read index record");
            RETURNSTRX("ERROR");
        }

        handle->indexCursor += (long)sizeof(record);
        if (!record.deleted) {
            RETURNSTRX(record.key);
        }
    }
    ENDPROC
}

/**
 * Return the next non-deleted key from the handle-local traversal.
 */
PROCEDURE(nextkey) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    struct IndexRecord record;

    if (!handle) {
        log_error("nextkey", KA_ERROR_PARAM, "Invalid handle");
        RETURNSTRX("ERROR");
    }

    while (1) {
        if (fseek(handle->indexFile, handle->indexCursor, SEEK_SET) != 0) {
            log_error("nextkey", KA_ERROR_IO, "Failed to seek index file");
            RETURNSTRX("ERROR");
        }

        if (fread(&record, sizeof(record), 1, handle->indexFile) != 1) {
            if (feof(handle->indexFile)) {
                RETURNSTRX("NOT_FOUND");
            }
            log_error("nextkey", KA_ERROR_IO, "Failed to read index record");
            RETURNSTRX("ERROR");
        }

        handle->indexCursor += (long)sizeof(record);
        if (!record.deleted) {
            RETURNSTRX(record.key);
        }
    }
    ENDPROC
}

PROCEDURE(validate_database) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    struct IndexRecord record;
    int errors = 0;
    char buffer[MAX_VALUE_LENGTH];

    if (!handle) {
        log_error("validate_database", KA_ERROR_PARAM, "Invalid handle");
        RETURNINTX(KA_ERROR_PARAM);
    }

    rewind(handle->indexFile);
    while (fread(&record, sizeof(record), 1, handle->indexFile) == 1) {
        if (record.deleted) continue;

        // Verify data can be read
        fseek(handle->dataFile, record.offset, SEEK_SET);
        if (fread(buffer, 1, record.length, handle->dataFile) != record.length) {
            errors++;
        }
    }

    RETURNINTX(errors);
    ENDPROC
}

/**
 * Destructively reset a store without scanning its index.
 *
 * The operation is deliberately rejected during a transaction because the
 * current rollback snapshot protects appended data, not data discarded by a
 * file truncation.
 */
PROCEDURE(reset_database) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);

    if (!handle) {
        log_error("reset_database", KA_ERROR_PARAM, "Invalid handle");
        RETURNINTX(KA_ERROR_PARAM);
    }

    if (handle->transaction) {
        log_error("reset_database", KA_ERROR_TXACTIVE,
                  "Cannot reset during transaction");
        RETURNINTX(KA_ERROR_TXACTIVE);
    }

    if (lock_file(handle) != KA_SUCCESS) {
        RETURNINTX(KA_ERROR_LOCK);
    }

    if (fflush(handle->dataFile) != 0 || fflush(handle->indexFile) != 0 ||
        truncate_file(handle->indexFile, 0) != 0 ||
        truncate_file(handle->dataFile, 0) != 0 ||
        fflush(handle->dataFile) != 0 || fflush(handle->indexFile) != 0 ||
        fseek(handle->dataFile, 0, SEEK_SET) != 0 ||
        fseek(handle->indexFile, 0, SEEK_SET) != 0) {
        log_error("reset_database", KA_ERROR_IO,
                  "Failed to truncate database files");
        unlock_file(handle);
        RETURNINTX(KA_ERROR_IO);
    }

    cache_clear(handle);
    hash_index_discard(handle);
    handle->indexCursor = 0;
    unlock_file(handle);

    RETURNINTX(KA_SUCCESS);
    ENDPROC
}

PROCEDURE(compact_database) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    char tempData[] = "temp_data.XXXXXX";
    char tempIndex[] = "temp_index.XXXXXX";
    FILE *newData, *newIndex;
    struct IndexRecord record;
    char* value;
    long newOffset = 0;
    int closeDataRc;
    int closeIndexRc;
    int tempDataCloseRc;
    int tempIndexCloseRc;
    int renameDataRc;
    int renameIndexRc;

    if (!handle) {
        log_error("compact_database", KA_ERROR_PARAM, "Invalid handle");
        RETURNINTX(KA_ERROR_PARAM);
    }

    if (handle->transaction) {
        log_error("compact_database", KA_ERROR_TXACTIVE, "Cannot compact during transaction");
        RETURNINTX(KA_ERROR_TXACTIVE);
    }

    // Create temporary files
    newData = fopen(tempData, "wb+");
    newIndex = fopen(tempIndex, "wb+");
    if (!newData || !newIndex) {
        log_error("compact_database", KA_ERROR_IO, "Failed to create temporary files");
        if (newData) fclose(newData);
        if (newIndex) fclose(newIndex);
        remove(tempData);
        remove(tempIndex);
        RETURNINTX(KA_ERROR_IO);
    }

    // Copy valid records
    rewind(handle->indexFile);
    while (fread(&record, sizeof(record), 1, handle->indexFile) == 1) {
        if (record.deleted) continue;

        value = malloc(record.length);
        if (!value) {
            log_error("compact_database", KA_ERROR_MEMORY, "Failed to allocate buffer");
            continue;
        }

        if (fseek(handle->dataFile, record.offset, SEEK_SET) != 0 ||
            fread(value, 1, record.length, handle->dataFile) != record.length) {
            log_error("compact_database", KA_ERROR_IO, "Failed to read record during compaction");
            free(value);
            fclose(newData);
            fclose(newIndex);
            RETURNINTX(KA_ERROR_IO);
        }

        record.offset = newOffset;
        if (fwrite(value, 1, record.length, newData) != record.length ||
            fwrite(&record, sizeof(record), 1, newIndex) != 1) {
            log_error("compact_database", KA_ERROR_IO, "Failed to write record during compaction");
            free(value);
            fclose(newData);
            fclose(newIndex);
            RETURNINTX(KA_ERROR_IO);
        }

        newOffset += record.length;
        free(value);
    }

    if (ferror(handle->indexFile) || fflush(newData) != 0 || fflush(newIndex) != 0) {
        log_error("compact_database", KA_ERROR_IO, "Failed while reading or flushing compacted files");
        fclose(newData);
        fclose(newIndex);
        remove(tempData);
        remove(tempIndex);
        RETURNINTX(KA_ERROR_IO);
    }

    // Replace original files
    closeDataRc = fclose(handle->dataFile);
    closeIndexRc = fclose(handle->indexFile);
    handle->dataFile = NULL;
    handle->indexFile = NULL;
    if (closeDataRc != 0 || closeIndexRc != 0) {
        log_error("compact_database", KA_ERROR_IO, "Failed to close database files before replacement");
        fclose(newData);
        fclose(newIndex);
        remove(tempData);
        remove(tempIndex);
        RETURNINTX(KA_ERROR_IO);
    }
    tempDataCloseRc = fclose(newData);
    tempIndexCloseRc = fclose(newIndex);
    renameDataRc = -1;
    renameIndexRc = -1;
    if (tempDataCloseRc == 0 && tempIndexCloseRc == 0) {
        renameDataRc = rename(tempData, handle->dataPath);
        renameIndexRc = rename(tempIndex, handle->indexPath);
    }
    if (tempDataCloseRc != 0 || tempIndexCloseRc != 0 ||
        renameDataRc != 0 || renameIndexRc != 0) {
        log_error("compact_database", KA_ERROR_IO, "Failed to replace compacted database files");
        remove(tempData);
        remove(tempIndex);
        RETURNINTX(KA_ERROR_IO);
    }

    // Reopen files
    handle->dataFile = fopen(handle->dataPath, "rb+");
    handle->indexFile = fopen(handle->indexPath, "rb+");

    if (!handle->dataFile || !handle->indexFile) {
        log_error("compact_database", KA_ERROR_IO, "Failed to reopen files after compaction");
        if (handle->dataFile) fclose(handle->dataFile);
        if (handle->indexFile) fclose(handle->indexFile);
        handle->dataFile = NULL;
        handle->indexFile = NULL;
        RETURNINTX(KA_ERROR_IO);
    }

    handle->indexCursor = 0;
    hash_index_discard(handle);

    RETURNINTX(KA_SUCCESS);
    ENDPROC
}

PROCEDURE(backup) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    char* backupPath = GETSTRING(ARG1);
    FILE* backupFile;
    char buffer[8192];
    size_t bytesRead;

    if (!handle || !backupPath) {
        log_error("backup", KA_ERROR_PARAM, "Invalid parameters");
        RETURNINTX(KA_ERROR_PARAM);
    }

    backupFile = fopen(backupPath, "wb");
    if (!backupFile) {
        log_error("backup", KA_ERROR_IO, "Failed to create backup file");
        RETURNINTX(KA_ERROR_IO);
    }

    // Write header
    fprintf(backupFile, "KVDB_BACKUP_V1\n%ld\n", time(NULL));

    // Copy data file
    rewind(handle->dataFile);
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), handle->dataFile)) > 0) {
        if (fwrite(buffer, 1, bytesRead, backupFile) != bytesRead) {
            log_error("backup", KA_ERROR_IO, "Failed to write to backup file");
            fclose(backupFile);
            RETURNINTX(KA_ERROR_IO);
        }
    }

    // Copy index file
    fprintf(backupFile, "\nINDEX_START\n");
    rewind(handle->indexFile);
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), handle->indexFile)) > 0) {
        if (fwrite(buffer, 1, bytesRead, backupFile) != bytesRead) {
            log_error("backup", KA_ERROR_IO, "Failed to write to backup file");
            fclose(backupFile);
            RETURNINTX(KA_ERROR_IO);
        }
    }

    fclose(backupFile);
    RETURNINTX(KA_SUCCESS);
    ENDPROC
}

// Add error checking to file operations
static int write_record(struct FileHandle* handle,
                        const struct IndexRecord* record,
                        const char* value) {
    if (fwrite(value, 1, strlen(value), handle->dataFile) != strlen(value)) {
        log_error("write_record", KA_ERROR_IO,
                  "Failed to write value to data file");
        return KA_ERROR_IO;
    }

    if (fwrite(record, sizeof(*record), 1, handle->indexFile) != 1) {
        log_error("write_record", KA_ERROR_IO,
                  "Failed to write record to index file");
        return KA_ERROR_IO;
    }

    return KA_SUCCESS;
}

// File locking
static int lock_file(struct FileHandle* handle) {
#ifdef _WIN32
    HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(handle->dataFile));
    if (LockFile(hFile, 0, 0, MAXDWORD, MAXDWORD)) {
        handle->locked = 1;
        return KA_SUCCESS;
    }
#else
    if (flock(fileno(handle->dataFile), LOCK_EX | LOCK_NB) == 0) {
        handle->locked = 1;
        return KA_SUCCESS;
    }
#endif
    log_error("lock_file", KA_ERROR_LOCK, "Failed to lock file");
    return KA_ERROR_LOCK;
}

static int unlock_file(struct FileHandle* handle) {
#ifdef _WIN32
    HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(handle->dataFile));
    if (UnlockFile(hFile, 0, 0, MAXDWORD, MAXDWORD)) {
        handle->locked = 0;
        return KA_SUCCESS;
    }
#else
    if (flock(fileno(handle->dataFile), LOCK_UN) == 0) {
        handle->locked = 0;
        return KA_SUCCESS;
    }
#endif
    log_error("unlock_file", KA_ERROR_LOCK, "Failed to unlock file");
    return KA_ERROR_LOCK;
}

// Create index filename from data filename
static char* create_index_path(const char* dataPath) {
    size_t len = strlen(dataPath);
    char* indexPath = (char*)malloc(len + 5); // +5 for ".idx\0"
    if (!indexPath) {
        log_error("create_index_path", KA_ERROR_MEMORY, "Failed to allocate index path");
        return NULL;
    }
    strcpy(indexPath, dataPath);
    strcat(indexPath, ".idx");
    return indexPath;
}

// In-memory hash index helpers
static uint64_t hash_key(const char* key) {
    uint64_t hash = UINT64_C(1469598103934665603);

    while (*key) {
        hash ^= (unsigned char)*key++;
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void hash_index_discard(struct FileHandle* handle) {
    free(handle->hashIndex.buckets);
    free(handle->hashIndex.entries);
    memset(&handle->hashIndex, 0, sizeof(handle->hashIndex));
}

static int hash_index_add(struct FileHandle* handle, uint64_t hash,
                          long indexOffset) {
    struct HashIndex* index = &handle->hashIndex;
    struct HashIndexEntry* entries;
    int entry;
    size_t newCapacity;

    if (index->entryCount == index->entryCapacity) {
        newCapacity = index->entryCapacity == 0 ? 256 : index->entryCapacity * 2;
        entries = (struct HashIndexEntry*)realloc(
            index->entries, newCapacity * sizeof(*entries));
        if (!entries) {
            return KA_ERROR_MEMORY;
        }
        index->entries = entries;
        index->entryCapacity = newCapacity;
    }

    entry = (int)index->entryCount;
    index->entries[entry].hash = hash;
    index->entries[entry].indexOffset = indexOffset;
    index->entries[entry].next = index->buckets[hash % index->bucketCount];
    index->buckets[hash % index->bucketCount] = entry;
    index->entryCount++;
    return KA_SUCCESS;
}

static int hash_index_remove(struct FileHandle* handle, uint64_t hash,
                             long indexOffset) {
    struct HashIndex* index = &handle->hashIndex;
    size_t bucket;
    int entry;
    int previous = -1;

    if (!index->built) {
        return KA_SUCCESS;
    }

    bucket = hash % index->bucketCount;
    entry = index->buckets[bucket];
    while (entry >= 0) {
        if (index->entries[entry].hash == hash &&
            index->entries[entry].indexOffset == indexOffset) {
            if (previous < 0) {
                index->buckets[bucket] = index->entries[entry].next;
            } else {
                index->entries[previous].next = index->entries[entry].next;
            }
            index->entries[entry].next = -1;
            return KA_SUCCESS;
        }
        previous = entry;
        entry = index->entries[entry].next;
    }

    return KA_ERROR_NOTFOUND;
}

static int hash_index_build(struct FileHandle* handle) {
    struct IndexRecord record;
    long indexOffset = 0;
    int rc;
    size_t bucket;

    hash_index_discard(handle);
    handle->hashIndex.bucketCount = 1024;
    handle->hashIndex.buckets = (int*)malloc(
        handle->hashIndex.bucketCount * sizeof(*handle->hashIndex.buckets));
    if (!handle->hashIndex.buckets) {
        hash_index_discard(handle);
        return KA_ERROR_MEMORY;
    }
    for (bucket = 0; bucket < handle->hashIndex.bucketCount; bucket++) {
        handle->hashIndex.buckets[bucket] = -1;
    }

    rewind(handle->indexFile);
    while (fread(&record, sizeof(record), 1, handle->indexFile) == 1) {
        if (!record.deleted) {
            rc = hash_index_add(handle, hash_key(record.key), indexOffset);
            if (rc != KA_SUCCESS) {
                hash_index_discard(handle);
                return rc;
            }
        }
        indexOffset += (long)sizeof(record);
    }
    if (ferror(handle->indexFile)) {
        hash_index_discard(handle);
        return KA_ERROR_IO;
    }

    handle->hashIndex.built = 1;
    return KA_SUCCESS;
}

static int find_key_indexed(struct FileHandle* handle, const char* searchKey,
                            struct IndexRecord* result, long* resultOffset) {
    struct HashIndex* index = &handle->hashIndex;
    uint64_t hash = hash_key(searchKey);
    int entry;
    int rc;

    if (!index->built) {
        rc = hash_index_build(handle);
        if (rc != KA_SUCCESS) {
            return rc;
        }
    }

    entry = index->buckets[hash % index->bucketCount];
    while (entry >= 0) {
        if (index->entries[entry].hash == hash) {
            if (fseek(handle->indexFile, index->entries[entry].indexOffset,
                      SEEK_SET) != 0 ||
                fread(result, sizeof(*result), 1, handle->indexFile) != 1) {
                if (feof(handle->indexFile)) {
                    return KA_ERROR_NOTFOUND;
                }
                return KA_ERROR_IO;
            }
            if (!result->deleted && strcmp(result->key, searchKey) == 0) {
                if (resultOffset) {
                    *resultOffset = index->entries[entry].indexOffset;
                }
                return KA_SUCCESS;
            }
        }
        entry = index->entries[entry].next;
    }

    return KA_ERROR_NOTFOUND;
}

// Cache management
static void cache_init(struct FileHandle* handle) {
    int i;

    memset(handle->cache, 0, sizeof(struct CacheEntry) * CACHE_SIZE);
    for (i = 0; i < CACHE_BUCKET_COUNT; i++) {
        handle->cacheBuckets[i] = -1;
    }
    for (i = 0; i < CACHE_SIZE; i++) {
        handle->cacheNext[i] = -1;
    }
    handle->cacheCount = 0;
    handle->cacheHits = 0;
    handle->cacheMisses = 0;
}

static size_t cache_bucket(uint64_t hash) {
    return (size_t)(hash % CACHE_BUCKET_COUNT);
}

static void cache_clear(struct FileHandle* handle) {
    int i;

    for (i = 0; i < CACHE_SIZE; i++) {
        free(handle->cache[i].value);
        handle->cache[i].value = NULL;
        handle->cache[i].key[0] = '\0';
        handle->cache[i].hash = 0;
        handle->cache[i].timestamp = 0;
        handle->cache[i].hits = 0;
        handle->cacheNext[i] = -1;
    }
    for (i = 0; i < CACHE_BUCKET_COUNT; i++) {
        handle->cacheBuckets[i] = -1;
    }
    handle->cacheCount = 0;
}


static void cache_put(struct FileHandle *handle,
                      const char *key,
                      const char *value)
{
    int oldest = 0;
    time_t oldestTime = time(NULL);
    uint64_t hash = hash_key(key);
    size_t bucket = cache_bucket(hash);
    int i;
    int entry;
    int previous;

    /* Replace an existing entry first. */
    entry = handle->cacheBuckets[bucket];
    while (entry >= 0) {
        i = entry;
        if (handle->cache[i].value != NULL &&
            handle->cache[i].hash == hash &&
            strcmp(handle->cache[i].key, key) == 0) {

            char *new_value = strdup(value);
            if (!new_value) {
                return;
            }

            free(handle->cache[i].value);
            handle->cache[i].value = new_value;
            handle->cache[i].hash = hash;
            handle->cache[i].timestamp = time(NULL);
            handle->cache[i].hits = 1;
            return;
            }
        entry = handle->cacheNext[entry];
    }

    /* Find an empty slot or the oldest entry. */
    for (i = 0; i < CACHE_SIZE; i++) {
        if (handle->cache[i].value == NULL) {
            oldest = i;
            break;
        }

        if (handle->cache[i].timestamp < oldestTime) {
            oldest = i;
            oldestTime = handle->cache[i].timestamp;
        }
    }

    if (handle->cache[oldest].value) {
        size_t old_bucket = cache_bucket(handle->cache[oldest].hash);
        entry = handle->cacheBuckets[old_bucket];
        previous = -1;
        while (entry >= 0) {
            if (entry == oldest) {
                if (previous < 0) {
                    handle->cacheBuckets[old_bucket] = handle->cacheNext[entry];
                } else {
                    handle->cacheNext[previous] = handle->cacheNext[entry];
                }
                break;
            }
            previous = entry;
            entry = handle->cacheNext[entry];
        }
        free(handle->cache[oldest].value);
    }

    strncpy(handle->cache[oldest].key, key, MAX_KEY_LENGTH - 1);
    handle->cache[oldest].key[MAX_KEY_LENGTH - 1] = '\0';

    handle->cache[oldest].hash = hash;
    handle->cache[oldest].value = strdup(value);
    handle->cache[oldest].timestamp = time(NULL);
    handle->cache[oldest].hits = 1;
    handle->cacheNext[oldest] = handle->cacheBuckets[bucket];
    handle->cacheBuckets[bucket] = oldest;

    if (handle->cacheCount < CACHE_SIZE) {
        handle->cacheCount++;
    }
}

static char *cache_get(struct FileHandle *handle, const char *key)
{
    uint64_t hash = hash_key(key);
    size_t bucket = cache_bucket(hash);
    int entry = handle->cacheBuckets[bucket];

    while (entry >= 0) {
        if (handle->cache[entry].value != NULL &&
            handle->cache[entry].hash == hash &&
            strcmp(handle->cache[entry].key, key) == 0) {

            handle->cache[entry].hits++;
            handle->cache[entry].timestamp = time(NULL);
            handle->cacheHits++;
            return handle->cache[entry].value;
            }
        entry = handle->cacheNext[entry];
    }

    handle->cacheMisses++;
    return NULL;
}

PROCEDURE(closefile) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    int i;
    int close_rc = KA_SUCCESS;

    if (!handle) {
        log_error("closefile", KA_ERROR_PARAM, "Invalid handle");
        RETURNINTX(KA_ERROR_PARAM);
    }

    // If transaction is active, roll it back before closing.
    if (handle->transaction) {
        if (restore_transaction(handle) != KA_SUCCESS) {
            close_rc = KA_ERROR_IO;
        }
        handle->transaction = 0;
        unlock_file(handle);
    }

    // Free cache entries
    for (i = 0; i < CACHE_SIZE; i++) {
        if (handle->cache[i].value) {
            free(handle->cache[i].value);
        }
    }

    free(handle->transactionIndexSnapshot);
    hash_index_discard(handle);

    fclose(handle->dataFile);
    fclose(handle->indexFile);
    free(handle->dataPath);
    free(handle->indexPath);
    free(handle);

    RETURNINTX(close_rc);
    ENDPROC
}

// Transaction management
PROCEDURE(begin_transaction) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    long indexSize;

    if (!handle) {
        log_error("begin_transaction", KA_ERROR_PARAM, "Invalid handle");
        RETURNINTX(KA_ERROR_PARAM);
    }

    if (handle->transaction) {
        log_error("begin_transaction", KA_ERROR_TXACTIVE, "Transaction already active");
        RETURNINTX(KA_ERROR_TXACTIVE);
    }

    if (lock_file(handle) != KA_SUCCESS) {
        RETURNINTX(KA_ERROR_LOCK);
    }

    if (fseek(handle->dataFile, 0, SEEK_END) != 0) {
        log_error("begin_transaction", KA_ERROR_IO, "Failed to seek data file");
        unlock_file(handle);
        RETURNINTX(KA_ERROR_IO);
    }
    handle->transactionDataSize = ftell(handle->dataFile);

    if (fseek(handle->indexFile, 0, SEEK_END) != 0) {
        log_error("begin_transaction", KA_ERROR_IO, "Failed to seek index file");
        unlock_file(handle);
        RETURNINTX(KA_ERROR_IO);
    }
    indexSize = ftell(handle->indexFile);
    if (indexSize < 0) {
        log_error("begin_transaction", KA_ERROR_IO, "Failed to determine index size");
        unlock_file(handle);
        RETURNINTX(KA_ERROR_IO);
    }

    free(handle->transactionIndexSnapshot);
    handle->transactionIndexSnapshot = NULL;
    if (indexSize > 0) {
        handle->transactionIndexSnapshot = (unsigned char*)malloc((size_t)indexSize);
        if (!handle->transactionIndexSnapshot) {
            log_error("begin_transaction", KA_ERROR_MEMORY, "Failed to snapshot index");
            unlock_file(handle);
            RETURNINTX(KA_ERROR_MEMORY);
        }

        rewind(handle->indexFile);
        if (fread(handle->transactionIndexSnapshot, 1, (size_t)indexSize,
                  handle->indexFile) != (size_t)indexSize) {
            log_error("begin_transaction", KA_ERROR_IO, "Failed to snapshot index");
            free(handle->transactionIndexSnapshot);
            handle->transactionIndexSnapshot = NULL;
            unlock_file(handle);
            RETURNINTX(KA_ERROR_IO);
        }
    }
    handle->transactionIndexSize = indexSize;

    handle->transaction = 1;
    handle->stats.transactions++;
    RETURNINTX(KA_SUCCESS);
    ENDPROC
}

PROCEDURE(commit_transaction) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);

    if (!handle) {
        log_error("commit_transaction", KA_ERROR_PARAM, "Invalid handle");
        RETURNINTX(KA_ERROR_PARAM);
    }

    if (!handle->transaction) {
        log_error("commit_transaction", KA_ERROR_TXINACTIVE, "No active transaction");
        RETURNINTX(KA_ERROR_TXINACTIVE);
    }

    if (fflush(handle->dataFile) != 0 || fflush(handle->indexFile) != 0) {
        log_error("commit_transaction", KA_ERROR_IO, "Failed to flush database files");
        RETURNINTX(KA_ERROR_IO);
    }

    free(handle->transactionIndexSnapshot);
    handle->transactionIndexSnapshot = NULL;
    handle->transactionDataSize = 0;
    handle->transactionIndexSize = 0;
    handle->transaction = 0;
    unlock_file(handle);
    RETURNINTX(KA_SUCCESS);
    ENDPROC
}

// Platform-specific file truncation
static int truncate_file(FILE* file, long size) {
#ifdef _WIN32
    return _chsize(_fileno(file), size);
#else
    return ftruncate(fileno(file), size);
#endif
}

static int restore_transaction(struct FileHandle* handle) {
    if (truncate_file(handle->dataFile, handle->transactionDataSize) != 0) {
        log_error("restore_transaction", KA_ERROR_IO, "Failed to truncate data file");
        return KA_ERROR_IO;
    }

    if (fseek(handle->indexFile, 0, SEEK_SET) != 0 ||
        (handle->transactionIndexSize > 0 &&
         fwrite(handle->transactionIndexSnapshot, 1,
                (size_t)handle->transactionIndexSize,
                handle->indexFile) != (size_t)handle->transactionIndexSize) ||
        truncate_file(handle->indexFile, handle->transactionIndexSize) != 0) {
        log_error("restore_transaction", KA_ERROR_IO, "Failed to restore index file");
        return KA_ERROR_IO;
    }

    fflush(handle->dataFile);
    fflush(handle->indexFile);
    cache_clear(handle);
    hash_index_discard(handle);
    free(handle->transactionIndexSnapshot);
    handle->transactionIndexSnapshot = NULL;
    handle->transactionDataSize = 0;
    handle->transactionIndexSize = 0;
    return KA_SUCCESS;
}

PROCEDURE(rollback_transaction) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);

    if (!handle) {
        log_error("rollback_transaction", KA_ERROR_PARAM, "Invalid handle");
        RETURNINTX(KA_ERROR_PARAM);
    }

    if (!handle->transaction) {
        log_error("rollback_transaction", KA_ERROR_TXINACTIVE, "No active transaction");
        RETURNINTX(KA_ERROR_TXINACTIVE);
    }

    if (restore_transaction(handle) != KA_SUCCESS) {
        RETURNINTX(KA_ERROR_IO);
    }

    handle->transaction = 0;
    unlock_file(handle);
    RETURNINTX(KA_SUCCESS);
    ENDPROC
}

/**
 * @brief Retrieves database statistics and performance metrics
 *
 * Returns a formatted string containing:
 * - Uptime
 * - Operation counts (reads, writes, deletes)
 * - Transaction count
 * - Total bytes read/written
 * - Cache performance (hits, misses, ratio)
 *
 * @param handle Database handle
 * @return Formatted statistics string or "ERROR" on failure
 */
PROCEDURE(get_statistics) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    char stats[4096];
    time_t uptime;
    double cacheHitRatio;

    if (!handle) {
        log_error("get_statistics", KA_ERROR_PARAM, "Invalid handle");
        RETURNSTRX("ERROR");
    }

    uptime = time(NULL) - handle->stats.startTime;
    cacheHitRatio = (handle->cacheHits + handle->cacheMisses) > 0 ?
                    (double)handle->cacheHits / (handle->cacheHits + handle->cacheMisses) : 0.0;

    snprintf(stats, sizeof(stats),
             "Statistics:\n"
             "Uptime: %ld seconds\n"
             "Reads: %lu\n"
             "Writes: %lu\n"
             "Deletes: %lu\n"
             "Transactions: %lu\n"
             "Total bytes written: %zu\n"
             "Total bytes read: %zu\n"
             "Cache hits: %lu\n"
             "Cache misses: %lu\n"
             "Cache hit ratio: %.2f%%\n",
             uptime,
             handle->stats.reads,
             handle->stats.writes,
             handle->stats.deletes,
             handle->stats.transactions,
             handle->stats.totalBytesWritten,
             handle->stats.totalBytesRead,
             handle->cacheHits,
             handle->cacheMisses,
             cacheHitRatio * 100.0
    );

    RETURNSTRX(stats);
    ENDPROC
}

/* -------------------------------------------------------------------------------------
 * Functions to be provided to rexx
 * -------------------------------------------------------------------------------------
 */
LOADFUNCS
    ADDPROC(openfile,    "keyaccess._openkey",     "b", ".int",    "filename=.string,mode=.string");
    ADDPROC(closefile,   "keyaccess._closekey",    "b", ".int",    "handle=.int");
    ADDPROC(writekey,    "keyaccess._writekey",    "b", ".int",    "handle=.int,key=.string,value=.string");
    ADDPROC(readkey,     "keyaccess._readkey",     "b", ".string", "handle=.int,key=.string");
    ADDPROC(deletekey,   "keyaccess._deletekey",   "b", ".int",    "handle=.int,key=.string");
    ADDPROC(listkeys,    "keyaccess._listkey",     "b", ".int",    "handle=.int");
    ADDPROC(firstkey,    "keyaccess._firstkey",    "b", ".string", "handle=.int");
    ADDPROC(nextkey,     "keyaccess._nextkey",     "b", ".string", "handle=.int");
    ADDPROC(begin_transaction, "keyaccess._txbegin",   "b", ".int",    "handle=.int");
    ADDPROC(commit_transaction, "keyaccess._txcommit", "b", ".int",    "handle=.int");
    ADDPROC(rollback_transaction, "keyaccess._txrollback", "b", ".int", "handle=.int");
    ADDPROC(get_statistics, "keyaccess._stats",   "b", ".string", "handle=.int");
    ADDPROC(backup,        "keyaccess._backup",  "b", ".int",    "handle=.int,path=.string");
    ADDPROC(validate_database, "keyaccess._validate", "b", ".int", "handle=.int");
    ADDPROC(reset_database, "keyaccess._reset", "b", ".int", "handle=.int");
    ADDPROC(compact_database, "keyaccess._compact",  "b", ".int", "handle=.int");
ENDLOADFUNCS

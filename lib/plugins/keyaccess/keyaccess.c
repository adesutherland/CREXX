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
 * @author Your Name
 * @date YYYY-MM-DD
 * @version 1.0
 */

#include <stdio.h>
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
#define LOG_FILENAME "c:\\temp\\crexx\\keyaccess.log"
#define CACHE_SIZE 1024

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

// File handle structure
struct FileHandle {
    FILE* dataFile;     // Main data file
    FILE* indexFile;    // Index file
    FILE* logFile;      // Transaction log
    char* dataPath;     // Path to data file
    char* indexPath;    // Path to index file
    int locked;         // File lock status
    int transaction;    // Transaction status
    struct CacheEntry cache[CACHE_SIZE];
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
static char* create_index_path(const char* dataPath);
static struct IndexRecord* find_key(FILE* indexFile, const char* searchKey);
static void cache_init(struct FileHandle* handle);
static void cache_put(struct FileHandle* handle, const char* key, const char* value);
static char* cache_get(struct FileHandle* handle, const char* key);
static int write_record(struct FileHandle* handle, const struct IndexRecord* record, const char* value);

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

// Update error handling in functions
PROCEDURE(openfile) {
    char* filename = GETSTRING(ARG0);
    char* mode = GETSTRING(ARG1);
    struct FileHandle* handle;

    if (!filename || !mode) {
        log_error("openfile", KA_ERROR_PARAM, "NULL filename or mode");
        RETURNINTX(KA_ERROR_PARAM);
    }

    handle = (struct FileHandle*)malloc(sizeof(struct FileHandle));
    if (!handle) {
        log_error("openfile", KA_ERROR_MEMORY, "Failed to allocate handle");
        RETURNINTX(KA_ERROR_MEMORY);
    }

    // Initialize handle structure
    memset(handle, 0, sizeof(struct FileHandle));
    handle->dataPath = strdup(filename);
    handle->indexPath = create_index_path(filename);

    if (!handle->dataPath || !handle->indexPath) {
        log_error("openfile", KA_ERROR_MEMORY, "Failed to allocate path strings");
        free(handle->dataPath);
        free(handle->indexPath);
        free(handle);
        RETURNINTX(KA_ERROR_MEMORY);
    }

    // Open data file
    handle->dataFile = fopen(handle->dataPath, mode);
    if (!handle->dataFile) {
        log_error("openfile", KA_ERROR_IO, "Failed to open data file");
        free(handle->dataPath);
        free(handle->indexPath);
        free(handle);
        RETURNINTX(KA_ERROR_IO);
    }

    // Open or create index file
    handle->indexFile = fopen(handle->indexPath, mode);
    if (!handle->indexFile) {
        log_error("openfile", KA_ERROR_IO, "Failed to open index file");
        fclose(handle->dataFile);
        free(handle->dataPath);
        free(handle->indexPath);
        free(handle);
        RETURNINTX(KA_ERROR_IO);
    }

    // Initialize cache
    cache_init(handle);

    // Initialize statistics
    handle->stats.startTime = time(NULL);
    handle->stats.reads = 0;
    handle->stats.writes = 0;
    handle->stats.deletes = 0;
    handle->stats.transactions = 0;
    handle->stats.totalBytesRead = 0;
    handle->stats.totalBytesWritten = 0;

    // Return handle as integer
    RETURNINTX((intptr_t)handle);
ENDPROC
}

PROCEDURE(writekey) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    char* key = GETSTRING(ARG1);
    char* value = GETSTRING(ARG2);
    struct IndexRecord record;
    long dataOffset;

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

    // Update existing or create new record
    struct IndexRecord* existing = find_key(handle->indexFile, key);
    if (existing) {
        record = *existing;
        record.version++;
        record.deleted = 0;
    } else {
        memset(&record, 0, sizeof(record));
        strncpy(record.key, key, MAX_KEY_LENGTH - 1);
        record.version = 1;
    }

    // Write data
    fseek(handle->dataFile, 0, SEEK_END);
    dataOffset = ftell(handle->dataFile);

    record.offset = dataOffset;
    record.length = strlen(value);
    record.timestamp = time(NULL);

    if (write_record(handle, &record, value) != KA_SUCCESS) {
        RETURNINTX(KA_ERROR_IO);
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
    struct IndexRecord* record;
    char* value;
    char* cached;

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
    record = find_key(handle->indexFile, key);
    if (!record) {
        log_error("readkey", KA_ERROR_NOTFOUND, "Key not found");
        RETURNSTRX("NOT_FOUND");
    }

    // Read value
    value = (char*)malloc(record->length + 1);
    if (!value) {
        log_error("readkey", KA_ERROR_MEMORY, "Failed to allocate value buffer");
        RETURNSTRX("ERROR");
    }

    fseek(handle->dataFile, record->offset, SEEK_SET);
    if (fread(value, 1, record->length, handle->dataFile) != record->length) {
        free(value);
        log_error("readkey", KA_ERROR_IO, "Failed to read value");
        RETURNSTRX("ERROR");
    }
    value[record->length] = '\0';

    // Update statistics
    handle->stats.reads++;
    handle->stats.totalBytesRead += record->length;

    // Update cache
    cache_put(handle, key, value);

    RETURNSTRX(value);
    ENDPROC
}

PROCEDURE(deletekey) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    char* key = GETSTRING(ARG1);
    struct IndexRecord* record;

    if (!handle || !key) {
        log_error("deletekey", KA_ERROR_PARAM, "Invalid parameters");
        RETURNINTX(KA_ERROR_PARAM);
    }

    if (!handle->transaction) {
        log_error("deletekey", KA_ERROR_TXINACTIVE, "Delete attempted outside transaction");
        RETURNINTX(KA_ERROR_TXINACTIVE);
    }

    record = find_key(handle->indexFile, key);
    if (!record) {
        log_error("deletekey", KA_ERROR_NOTFOUND, "Key not found");
        RETURNINTX(KA_ERROR_NOTFOUND);
    }

    // Mark as deleted
    record->deleted = 1;
    record->timestamp = time(NULL);

    // Write updated record
    fseek(handle->indexFile, -sizeof(struct IndexRecord), SEEK_CUR);
    if (fwrite(record, sizeof(*record), 1, handle->indexFile) != 1) {
        log_error("deletekey", KA_ERROR_IO, "Failed to update index");
        RETURNINTX(KA_ERROR_IO);
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

PROCEDURE(compact_database) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    char tempData[] = "temp_data.XXXXXX";
    char tempIndex[] = "temp_index.XXXXXX";
    FILE *newData, *newIndex;
    struct IndexRecord record;
    char* value;
    long newOffset = 0;

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

        fseek(handle->dataFile, record.offset, SEEK_SET);
        fread(value, 1, record.length, handle->dataFile);

        record.offset = newOffset;
        fwrite(value, 1, record.length, newData);
        fwrite(&record, sizeof(record), 1, newIndex);

        newOffset += record.length;
        free(value);
    }

    // Replace original files
    fclose(handle->dataFile);
    fclose(handle->indexFile);
    rename(tempData, handle->dataPath);
    rename(tempIndex, handle->indexPath);

    // Reopen files
    handle->dataFile = fopen(handle->dataPath, "rb+");
    handle->indexFile = fopen(handle->indexPath, "rb+");

    if (!handle->dataFile || !handle->indexFile) {
        log_error("compact_database", KA_ERROR_IO, "Failed to reopen files after compaction");
        RETURNINTX(KA_ERROR_IO);
    }

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

// Find key in index
static struct IndexRecord* find_key(FILE* indexFile, const char* searchKey) {
    static struct IndexRecord record;

    rewind(indexFile);
    while (fread(&record, sizeof(record), 1, indexFile) == 1) {
        if (strcmp(record.key, searchKey) == 0 && !record.deleted) {
            return &record;
        }
    }
    return NULL;
}

// Cache management
static void cache_init(struct FileHandle* handle) {
    memset(handle->cache, 0, sizeof(struct CacheEntry) * CACHE_SIZE);
    handle->cacheCount = 0;
    handle->cacheHits = 0;
    handle->cacheMisses = 0;
}

static void cache_put(struct FileHandle* handle, const char* key, const char* value) {
    int oldest = 0;
    time_t oldestTime = time(NULL);
    int i;

    // Find empty slot or oldest entry
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

    // Free old entry if exists
    if (handle->cache[oldest].value) {
        free(handle->cache[oldest].value);
    }

    // Store new entry
    strncpy(handle->cache[oldest].key, key, MAX_KEY_LENGTH - 1);
    handle->cache[oldest].value = strdup(value);
    handle->cache[oldest].timestamp = time(NULL);
    handle->cache[oldest].hits = 1;

    if (handle->cacheCount < CACHE_SIZE) handle->cacheCount++;
}

static char* cache_get(struct FileHandle* handle, const char* key) {
    int i;

    for (i = 0; i < handle->cacheCount; i++) {
        if (strcmp(handle->cache[i].key, key) == 0) {
            handle->cache[i].hits++;
            handle->cache[i].timestamp = time(NULL);
            handle->cacheHits++;
            return handle->cache[i].value;
        }
    }
    handle->cacheMisses++;
    return NULL;
}

PROCEDURE(closefile) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);
    int i;

    if (!handle) {
        log_error("closefile", KA_ERROR_PARAM, "Invalid handle");
        RETURNINTX(KA_ERROR_PARAM);
    }

    // If transaction is active, roll it back
    if (handle->transaction) {
        handle->transaction = 0;
        unlock_file(handle);
    }

    // Free cache entries
    for (i = 0; i < CACHE_SIZE; i++) {
        if (handle->cache[i].value) {
            free(handle->cache[i].value);
        }
    }

    fclose(handle->dataFile);
    fclose(handle->indexFile);
    free(handle->dataPath);
    free(handle->indexPath);
    free(handle);

    RETURNINTX(KA_SUCCESS);
    ENDPROC
}

// Transaction management
PROCEDURE(begin_transaction) {
    struct FileHandle* handle = (struct FileHandle*)GETINT(ARG0);

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

    fflush(handle->dataFile);
    fflush(handle->indexFile);

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

    // Revert to last commit point
    fseek(handle->dataFile, 0, SEEK_END);
    long dataSize = ftell(handle->dataFile);

    if (truncate_file(handle->dataFile, dataSize) != 0) {
        log_error("rollback_transaction", KA_ERROR_IO, "Failed to truncate file");
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
    ADDPROC(openfile,    "keyaccess.openkey",     "b", ".int",    "filename=.string,mode=.string");
    ADDPROC(closefile,   "keyaccess.closekey",    "b", ".int",    "handle=.int");
    ADDPROC(writekey,    "keyaccess.writekey",    "b", ".int",    "handle=.int,key=.string,value=.string");
    ADDPROC(readkey,     "keyaccess.readkey",     "b", ".string", "handle=.int,key=.string");
    ADDPROC(deletekey,   "keyaccess.deletekey",   "b", ".int",    "handle=.int,key=.string");
    ADDPROC(listkeys,    "keyaccess.listkey",     "b", ".int",    "handle=.int");
    ADDPROC(begin_transaction, "keyaccess.txbegin",   "b", ".int",    "handle=.int");
    ADDPROC(commit_transaction, "keyaccess.txcommit", "b", ".int",    "handle=.int");
    ADDPROC(rollback_transaction, "keyaccess.txrollback", "b", ".int", "handle=.int");
    ADDPROC(get_statistics, "keyaccess.stats",   "b", ".string", "handle=.int");
    ADDPROC(backup,        "keyaccess.backup",  "b", ".int",    "handle=.int,path=.string");
    ADDPROC(validate_database, "keyaccess.validate", "b", ".int", "handle=.int");
    ADDPROC(compact_database, "keyaccess.compact",  "b", ".int", "handle=.int");
ENDLOADFUNCS
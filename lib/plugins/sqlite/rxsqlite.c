/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland
 *
 * Generic, typed SQLite RXPA provider for cREXX.  The provider deliberately
 * contains no application vocabulary, schema, migration or repository policy.
 *
 * Donated from crexx-rag revision
 * d0a3b26f7ea22a745f5b5828bbce2f5ac1d0070a and adapted as the supported
 * CREXX rxsqlite component under Adrian Sutherland's licensing authority.
 */

#include "crexxpa.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

static void *rxsqlite_session_create(void);
static void rxsqlite_session_destroy(void *opaque_session);
static int rxsqlite_session_enter(void *opaque_session,
                                         uint32_t capabilities,
                                         void **previous);
static void rxsqlite_session_leave(void *previous);

static uint32_t rxsqlite_procedure_capabilities(
        const char *procedure_name) {
    if (!procedure_name || sqlite3_threadsafe() == 0) return 0u;
    return RXPA_PROCEDURE_CAP_SESSION_AFFINE;
}

/* Publish provider identity for VM autoload and native archive selection.
 * A current host gives each VM its own registry and diagnostic session.  A
 * SQLite build without mutex support fails closed to the legacy serialized
 * lane; older hosts ignore this optional V2 declaration and do the same. */
RXPA_PLUGIN_SESSION_AWARE(rxsqlite_session_create,
                          rxsqlite_session_destroy,
                          rxsqlite_session_enter,
                          rxsqlite_session_leave,
                          rxsqlite_procedure_capabilities)

enum {
    RXSQLITE_OK = 0,
    RXSQLITE_ROW = 100,
    RXSQLITE_DONE = 101,
    RXSQLITE_INVALID_ARGUMENT = -1,
    RXSQLITE_INVALID_HANDLE = -2,
    RXSQLITE_CLOSED_HANDLE = -3,
    RXSQLITE_WRONG_HANDLE_KIND = -4,
    RXSQLITE_NO_MEMORY = -5,
    RXSQLITE_SQLITE_ERROR = -6,
    RXSQLITE_TYPE_MISMATCH = -7
};

enum handle_kind {
    HANDLE_DATABASE = 1,
    HANDLE_STATEMENT = 2
};

typedef struct rxsqlite_handle rxsqlite_handle;
typedef struct rxsqlite_session rxsqlite_session;

typedef struct error_state {
    int code;
    int sqlite_code;
    int sqlite_extended_code;
    char operation[64];
    char message[768];
} error_state;

struct rxsqlite_handle {
    enum handle_kind kind;
    int closed;
    size_t references;
    union {
        sqlite3 *database;
        sqlite3_stmt *statement;
    } resource;
    rxsqlite_session *owner;
    rxsqlite_handle *parent;
    rxsqlite_handle *next;
};

typedef struct handle_payload {
    uint64_t magic;
    rxsqlite_session *owner;
    rxsqlite_handle *handle;
} handle_payload;

struct rxsqlite_session {
    rxsqlite_handle *live_handles;
    error_state last_error;
};

#define HANDLE_MAGIC UINT64_C(0x53514c3141484e44)

#if defined(_MSC_VER)
#define RXSQLITE_THREAD_LOCAL __declspec(thread)
#else
#define RXSQLITE_THREAD_LOCAL __thread
#endif

static RXSQLITE_THREAD_LOCAL rxsqlite_session
        *rxsqlite_current_session;
static rxsqlite_session rxsqlite_default_session;

static void handle_payload_copy(void *destination, void *source);
static void handle_payload_finalize(void *value);

static const rxpa_native_payload_ops handle_payload_ops = {
    "crexx.rxsqlite.handle.v1",
    handle_payload_copy,
    handle_payload_finalize
};

static rxsqlite_session *current_session(void) {
    return rxsqlite_current_session
            ? rxsqlite_current_session
            : &rxsqlite_default_session;
}

static void clear_error(void) {
    rxsqlite_session *session = current_session();
    memset(&session->last_error, 0, sizeof(session->last_error));
}

static int fail_with(int code, int sqlite_code, const char *operation,
                     const char *message) {
    error_state *last_error = &current_session()->last_error;
    last_error->code = code;
    last_error->sqlite_code = sqlite_code;
    last_error->sqlite_extended_code = sqlite_code;
    snprintf(last_error->operation, sizeof(last_error->operation), "%s",
             operation ? operation : "unknown");
    snprintf(last_error->message, sizeof(last_error->message), "%s",
             message ? message : "unspecified error");
    return code;
}

static int fail_sqlite(sqlite3 *database, int sqlite_code,
                       const char *operation) {
    const char *message = database ? sqlite3_errmsg(database)
                                   : sqlite3_errstr(sqlite_code);
    int primary_code = sqlite_code & 0xff;
    int status = fail_with(RXSQLITE_SQLITE_ERROR, primary_code, operation,
                           message);
    current_session()->last_error.sqlite_extended_code = database
            ? sqlite3_extended_errcode(database) : sqlite_code;
    return status;
}

static int handle_is_registered(const rxsqlite_session *session,
                                const rxsqlite_handle *candidate) {
    const rxsqlite_handle *cursor;
    if (!session) return 0;
    for (cursor = session->live_handles; cursor; cursor = cursor->next) {
        if (cursor == candidate) return 1;
    }
    return 0;
}

static void register_handle(rxsqlite_handle *handle) {
    rxsqlite_session *session = handle ? handle->owner : NULL;
    if (!session) return;
    handle->next = session->live_handles;
    session->live_handles = handle;
}

static void unregister_handle(rxsqlite_handle *handle) {
    rxsqlite_session *session = handle ? handle->owner : NULL;
    rxsqlite_handle **cursor;
    if (!session) return;
    cursor = &session->live_handles;
    while (*cursor) {
        if (*cursor == handle) {
            *cursor = handle->next;
            return;
        }
        cursor = &(*cursor)->next;
    }
}

static void retain_handle(rxsqlite_handle *handle) {
    if (handle) handle->references++;
}

static void close_statement_resource(rxsqlite_handle *handle) {
    if (!handle || handle->kind != HANDLE_STATEMENT || handle->closed) return;
    if (handle->resource.statement) sqlite3_finalize(handle->resource.statement);
    handle->resource.statement = NULL;
    handle->closed = 1;
}

static void close_database_resource(rxsqlite_handle *handle) {
    rxsqlite_handle *cursor;
    if (!handle || handle->kind != HANDLE_DATABASE || handle->closed) return;

    for (cursor = handle->owner->live_handles; cursor; cursor = cursor->next) {
        if (cursor->kind == HANDLE_STATEMENT && cursor->parent == handle) {
            close_statement_resource(cursor);
        }
    }
    if (handle->resource.database) sqlite3_close_v2(handle->resource.database);
    handle->resource.database = NULL;
    handle->closed = 1;
}

static void release_handle(rxsqlite_handle *handle) {
    rxsqlite_handle *parent;
    if (!handle || !handle_is_registered(handle->owner, handle) ||
        handle->references == 0) return;
    handle->references--;
    if (handle->references != 0) return;

    if (handle->kind == HANDLE_STATEMENT) close_statement_resource(handle);
    if (handle->kind == HANDLE_DATABASE) close_database_resource(handle);
    parent = handle->parent;
    unregister_handle(handle);
    free(handle);
    if (parent) release_handle(parent);
}

static void handle_payload_copy(void *destination, void *source) {
    const rxpa_native_payload_ops *ops = NULL;
    handle_payload *source_payload;
    handle_payload copied_payload;
    size_t length = 0;

    source_payload = (handle_payload *)GETNATIVEPAYLOAD(
        source, &length, &ops, NULL);
    if (!source_payload || length != sizeof(*source_payload) ||
        ops != &handle_payload_ops || source_payload->magic != HANDLE_MAGIC ||
        !source_payload->owner || !source_payload->handle ||
        !handle_is_registered(source_payload->owner,
                              source_payload->handle) ||
        source_payload->handle->owner != source_payload->owner) {
        return;
    }

    copied_payload = *source_payload;
    retain_handle(copied_payload.handle);
    if (SETNATIVEPAYLOAD(destination, &copied_payload, sizeof(copied_payload),
                         &handle_payload_ops, 0) != 0) {
        release_handle(copied_payload.handle);
    }
}

static void handle_payload_finalize(void *value) {
    const rxpa_native_payload_ops *ops = NULL;
    handle_payload *payload;
    size_t length = 0;

    payload = (handle_payload *)GETNATIVEPAYLOAD(value, &length, &ops, NULL);
    if (payload && length == sizeof(*payload) && ops == &handle_payload_ops &&
        payload->magic == HANDLE_MAGIC && payload->owner && payload->handle &&
        handle_is_registered(payload->owner, payload->handle) &&
        payload->handle->owner == payload->owner) {
        release_handle(payload->handle);
    }
}

static rxsqlite_handle *new_handle(enum handle_kind kind) {
    rxsqlite_handle *handle = (rxsqlite_handle *)calloc(1, sizeof(*handle));
    if (!handle) return NULL;
    handle->kind = kind;
    handle->references = 1;
    handle->owner = current_session();
    register_handle(handle);
    return handle;
}

static int publish_handle(rxpa_attribute_value destination,
                          rxsqlite_handle *handle) {
    handle_payload payload;
    payload.magic = HANDLE_MAGIC;
    payload.owner = handle->owner;
    payload.handle = handle;
    if (SETNATIVEPAYLOAD(destination, &payload, sizeof(payload),
                         &handle_payload_ops, 0) != 0) {
        release_handle(handle);
        return fail_with(RXSQLITE_NO_MEMORY, SQLITE_NOMEM, "publish_handle",
                         "could not publish native handle payload");
    }
    return RXSQLITE_OK;
}

static int get_handle(rxpa_attribute_value value, enum handle_kind expected,
                      const char *operation, rxsqlite_handle **result) {
    const rxpa_native_payload_ops *ops = NULL;
    handle_payload *payload;
    size_t length = 0;

    payload = (handle_payload *)GETNATIVEPAYLOAD(value, &length, &ops, NULL);
    if (!payload || length != sizeof(*payload) || ops != &handle_payload_ops ||
        payload->magic != HANDLE_MAGIC ||
        payload->owner != current_session() || !payload->handle ||
        !handle_is_registered(current_session(), payload->handle) ||
        payload->handle->owner != current_session()) {
        return fail_with(RXSQLITE_INVALID_HANDLE, 0, operation,
                         "invalid or stale opaque handle");
    }
    if (payload->handle->kind != expected) {
        return fail_with(RXSQLITE_WRONG_HANDLE_KIND, 0, operation,
                         "opaque handle has the wrong kind");
    }
    if (payload->handle->closed) {
        return fail_with(RXSQLITE_CLOSED_HANDLE, 0, operation,
                         "opaque handle is closed");
    }
    *result = payload->handle;
    return RXSQLITE_OK;
}

static int valid_column(rxsqlite_handle *statement, int column,
                        const char *operation) {
    int count = sqlite3_column_count(statement->resource.statement);
    if (column < 0 || column >= count) {
        return fail_with(RXSQLITE_INVALID_ARGUMENT, 0, operation,
                         "column index is outside the current row");
    }
    return RXSQLITE_OK;
}

static const char *column_type_name(int type) {
    switch (type) {
        case SQLITE_NULL: return "null";
        case SQLITE_INTEGER: return "integer";
        case SQLITE_FLOAT: return "real";
        case SQLITE_TEXT: return "text";
        case SQLITE_BLOB: return "blob";
        default: return "unknown";
    }
}

PROCEDURE(rxsqlite_open) {
    sqlite3 *database = NULL;
    rxsqlite_handle *handle;
    int result;

    if (NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    clear_error();
    SETNATIVEPAYLOAD(ARG(1), NULL, 0, NULL, 0);
    result = sqlite3_open_v2(GETSTRING(ARG(0)), &database,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                             SQLITE_OPEN_FULLMUTEX, NULL);
    if (result != SQLITE_OK) {
        result = fail_sqlite(database, result, "open");
        if (database) sqlite3_close_v2(database);
        SETINT(RETURN, result);
        RESETSIGNAL
        return;
    }

    (void)sqlite3_extended_result_codes(database, 1);

    handle = new_handle(HANDLE_DATABASE);
    if (!handle) {
        sqlite3_close_v2(database);
        SETINT(RETURN, fail_with(RXSQLITE_NO_MEMORY, SQLITE_NOMEM, "open",
                                 "could not allocate database handle"));
        RESETSIGNAL
        return;
    }
    handle->resource.database = database;
    SETINT(RETURN, publish_handle(ARG(1), handle));
    RESETSIGNAL
}

PROCEDURE(rxsqlite_open_mode) {
    sqlite3 *database = NULL;
    rxsqlite_handle *handle;
    const char *mode;
    int flags;
    int result;

    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    SETNATIVEPAYLOAD(ARG(2), NULL, 0, NULL, 0);
    mode = GETSTRING(ARG(1));
    if (strcmp(mode, "readonly") == 0) {
        flags = SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX;
    } else if (strcmp(mode, "readwrite") == 0) {
        flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;
    } else if (strcmp(mode, "create") == 0) {
        flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                SQLITE_OPEN_FULLMUTEX;
    } else {
        SETINT(RETURN,
               fail_with(RXSQLITE_INVALID_ARGUMENT, 0, "open_mode",
                         "open mode must be readonly, readwrite, or create"));
        RESETSIGNAL
        return;
    }

    result = sqlite3_open_v2(GETSTRING(ARG(0)), &database, flags, NULL);
    if (result != SQLITE_OK) {
        result = fail_sqlite(database, result, "open_mode");
        if (database) sqlite3_close_v2(database);
        SETINT(RETURN, result);
        RESETSIGNAL
        return;
    }

    (void)sqlite3_extended_result_codes(database, 1);

    handle = new_handle(HANDLE_DATABASE);
    if (!handle) {
        sqlite3_close_v2(database);
        SETINT(RETURN,
               fail_with(RXSQLITE_NO_MEMORY, SQLITE_NOMEM, "open_mode",
                         "could not allocate database handle"));
        RESETSIGNAL
        return;
    }
    handle->resource.database = database;
    SETINT(RETURN, publish_handle(ARG(2), handle));
    RESETSIGNAL
}

PROCEDURE(rxsqlite_close) {
    rxsqlite_handle *handle;
    int status;
    if (NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    clear_error();
    status = get_handle(ARG(0), HANDLE_DATABASE, "close", &handle);
    if (status == RXSQLITE_OK) close_database_resource(handle);
    SETINT(RETURN, status);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_exec) {
    rxsqlite_handle *handle;
    char *error_message = NULL;
    int result;
    if (NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    clear_error();
    result = get_handle(ARG(0), HANDLE_DATABASE, "exec", &handle);
    if (result == RXSQLITE_OK) {
        result = sqlite3_exec(handle->resource.database, GETSTRING(ARG(1)),
                              NULL, NULL, &error_message);
        if (result != SQLITE_OK) {
            int status = fail_with(RXSQLITE_SQLITE_ERROR,
                                   result & 0xff,
                                   "exec",
                                   error_message ? error_message :
                                   sqlite3_errmsg(handle->resource.database));
            current_session()->last_error.sqlite_extended_code =
                sqlite3_extended_errcode(handle->resource.database);
            sqlite3_free(error_message);
            result = status;
        } else {
            result = RXSQLITE_OK;
        }
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_capability) {
    rxsqlite_handle *handle;
    sqlite3_stmt *probe = NULL;
    const char *name;
    int available = 0;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_handle(ARG(0), HANDLE_DATABASE, "capability", &handle);
    if (result == RXSQLITE_OK) {
        name = GETSTRING(ARG(1));
        if (strcmp(name, "fts5") == 0) {
            available = sqlite3_compileoption_used("ENABLE_FTS5") != 0;
        } else if (strcmp(name, "threadsafe") == 0) {
            available = sqlite3_threadsafe() != 0;
        } else if (strcmp(name, "session_affinity") == 0) {
            available = rxsqlite_current_session != NULL;
        } else if (strcmp(name, "json1") == 0) {
            result = sqlite3_prepare_v2(handle->resource.database,
                                        "SELECT json_valid('null')", -1,
                                        &probe, NULL);
            available = result == SQLITE_OK;
            if (probe) sqlite3_finalize(probe);
            result = RXSQLITE_OK;
        } else {
            result = fail_with(RXSQLITE_INVALID_ARGUMENT, 0, "capability",
                               "unknown SQLite capability name");
        }
        if (result == RXSQLITE_OK) SETINT(ARG(2), available);
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_busy_timeout) {
    rxsqlite_handle *handle;
    rxinteger milliseconds;
    int result;
    if (NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    clear_error();
    result = get_handle(ARG(0), HANDLE_DATABASE, "busy_timeout", &handle);
    milliseconds = GETINT(ARG(1));
    if (result == RXSQLITE_OK &&
        (milliseconds < 0 || milliseconds > INT_MAX)) {
        result = fail_with(RXSQLITE_INVALID_ARGUMENT, 0, "busy_timeout",
                           "timeout milliseconds are outside the supported range");
    }
    if (result == RXSQLITE_OK) {
        result = sqlite3_busy_timeout(handle->resource.database,
                                      (int)milliseconds);
        if (result != SQLITE_OK) {
            result = fail_sqlite(handle->resource.database, result,
                                 "busy_timeout");
        }
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_checkpoint) {
    rxsqlite_handle *handle;
    const char *mode_name;
    int mode;
    int log_frames = 0;
    int checkpointed_frames = 0;
    int result;
    if (NUM_ARGS != 4) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "4 arguments expected")
    clear_error();
    result = get_handle(ARG(0), HANDLE_DATABASE, "checkpoint", &handle);
    if (result == RXSQLITE_OK) {
        mode_name = GETSTRING(ARG(1));
        if (strcmp(mode_name, "passive") == 0) mode = SQLITE_CHECKPOINT_PASSIVE;
        else if (strcmp(mode_name, "full") == 0) mode = SQLITE_CHECKPOINT_FULL;
        else if (strcmp(mode_name, "restart") == 0) mode = SQLITE_CHECKPOINT_RESTART;
        else if (strcmp(mode_name, "truncate") == 0) mode = SQLITE_CHECKPOINT_TRUNCATE;
        else {
            mode = SQLITE_CHECKPOINT_PASSIVE;
            result = fail_with(RXSQLITE_INVALID_ARGUMENT, 0, "checkpoint",
                               "checkpoint mode must be passive, full, restart, or truncate");
        }
    }
    if (result == RXSQLITE_OK) {
        result = sqlite3_wal_checkpoint_v2(handle->resource.database, NULL, mode,
                                           &log_frames,
                                           &checkpointed_frames);
        if (result != SQLITE_OK) {
            result = fail_sqlite(handle->resource.database, result,
                                 "checkpoint");
        } else {
            SETINT(ARG(2), log_frames);
            SETINT(ARG(3), checkpointed_frames);
        }
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_backup) {
    rxsqlite_handle *source;
    rxsqlite_handle *destination;
    sqlite3_backup *backup = NULL;
    rxinteger pages_per_step;
    rxinteger busy_retries;
    rxinteger sleep_milliseconds;
    int step_result = SQLITE_OK;
    int finish_result;
    int retries = 0;
    int remaining = 0;
    int page_count = 0;
    int result;

    if (NUM_ARGS != 7) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "7 arguments expected")
    clear_error();
    result = get_handle(ARG(0), HANDLE_DATABASE, "backup", &source);
    if (result == RXSQLITE_OK) {
        result = get_handle(ARG(1), HANDLE_DATABASE, "backup", &destination);
    }
    pages_per_step = GETINT(ARG(2));
    busy_retries = GETINT(ARG(3));
    sleep_milliseconds = GETINT(ARG(4));
    if (result == RXSQLITE_OK &&
        (pages_per_step <= 0 || pages_per_step > INT_MAX ||
         busy_retries < 0 || busy_retries > INT_MAX ||
         sleep_milliseconds < 0 || sleep_milliseconds > INT_MAX)) {
        result = fail_with(RXSQLITE_INVALID_ARGUMENT, 0, "backup",
                           "backup bounds are outside the supported range");
    }
    if (result == RXSQLITE_OK && source == destination) {
        result = fail_with(RXSQLITE_INVALID_ARGUMENT, 0, "backup",
                           "source and destination must be different handles");
    }
    if (result == RXSQLITE_OK) {
        backup = sqlite3_backup_init(destination->resource.database, "main",
                                     source->resource.database, "main");
        if (!backup) {
            result = fail_sqlite(destination->resource.database,
                                 sqlite3_errcode(destination->resource.database),
                                 "backup_init");
        }
    }
    if (result == RXSQLITE_OK) {
        for (;;) {
            step_result = sqlite3_backup_step(backup, (int)pages_per_step);
            remaining = sqlite3_backup_remaining(backup);
            page_count = sqlite3_backup_pagecount(backup);
            if (step_result == SQLITE_DONE) break;
            if (step_result == SQLITE_OK) continue;
            if ((step_result == SQLITE_BUSY || step_result == SQLITE_LOCKED) &&
                retries < (int)busy_retries) {
                retries++;
                if (sleep_milliseconds > 0) {
                    sqlite3_sleep((int)sleep_milliseconds);
                }
                continue;
            }
            result = fail_sqlite(destination->resource.database, step_result,
                                 "backup_step");
            break;
        }
    }
    if (backup) {
        finish_result = sqlite3_backup_finish(backup);
        if (result == RXSQLITE_OK && finish_result != SQLITE_OK) {
            result = fail_sqlite(destination->resource.database, finish_result,
                                 "backup_finish");
        }
    }
    if (result == RXSQLITE_OK) {
        SETINT(ARG(5), remaining);
        SETINT(ARG(6), page_count);
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_integrity) {
    rxsqlite_handle *handle;
    sqlite3_stmt *statement = NULL;
    const char *mode;
    char sql[96];
    char details[4096];
    size_t used = 0;
    rxinteger max_errors;
    int rows = 0;
    int result;

    if (NUM_ARGS != 5) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "5 arguments expected")
    clear_error();
    result = get_handle(ARG(0), HANDLE_DATABASE, "integrity", &handle);
    mode = GETSTRING(ARG(1));
    max_errors = GETINT(ARG(2));
    if (result == RXSQLITE_OK &&
        strcmp(mode, "quick") != 0 && strcmp(mode, "full") != 0) {
        result = fail_with(RXSQLITE_INVALID_ARGUMENT, 0, "integrity",
                           "integrity mode must be quick or full");
    }
    if (result == RXSQLITE_OK &&
        (max_errors <= 0 || max_errors > 1000)) {
        result = fail_with(RXSQLITE_INVALID_ARGUMENT, 0, "integrity",
                           "maximum integrity errors must be from 1 through 1000");
    }
    if (result == RXSQLITE_OK) {
        snprintf(sql, sizeof(sql), "PRAGMA %s_check(%lld)",
                 strcmp(mode, "quick") == 0 ? "quick" : "integrity",
                 (long long)max_errors);
        result = sqlite3_prepare_v2(handle->resource.database, sql, -1,
                                    &statement, NULL);
        if (result != SQLITE_OK) {
            result = fail_sqlite(handle->resource.database, result,
                                 "integrity_prepare");
        }
    }
    details[0] = '\0';
    while (result == RXSQLITE_OK &&
           (result = sqlite3_step(statement)) == SQLITE_ROW) {
        const unsigned char *row = sqlite3_column_text(statement, 0);
        size_t length = row ? strlen((const char *)row) : 0;
        if (rows > 0 && used + 1 < sizeof(details)) details[used++] = '\n';
        if (length > sizeof(details) - used - 1) {
            length = sizeof(details) - used - 1;
        }
        if (length > 0) {
            memcpy(details + used, row, length);
            used += length;
        }
        details[used] = '\0';
        rows++;
        result = RXSQLITE_OK;
    }
    if (statement) {
        int step_status = result;
        int finalize_status = sqlite3_finalize(statement);
        statement = NULL;
        if (step_status == SQLITE_DONE) {
            result = finalize_status == SQLITE_OK ? RXSQLITE_OK :
                     fail_sqlite(handle->resource.database, finalize_status,
                                 "integrity_finalize");
        } else if (step_status != RXSQLITE_OK) {
            result = fail_sqlite(handle->resource.database, step_status,
                                 "integrity_step");
        }
    }
    if (result == RXSQLITE_OK) {
        SETINT(ARG(3), rows == 1 && strcmp(details, "ok") == 0);
        SETSTRING(ARG(4), details);
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_prepare) {
    rxsqlite_handle *database_handle;
    rxsqlite_handle *statement_handle;
    sqlite3_stmt *statement = NULL;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    SETNATIVEPAYLOAD(ARG(2), NULL, 0, NULL, 0);
    result = get_handle(ARG(0), HANDLE_DATABASE, "prepare", &database_handle);
    if (result != RXSQLITE_OK) {
        SETINT(RETURN, result);
        RESETSIGNAL
        return;
    }
    result = sqlite3_prepare_v2(database_handle->resource.database,
                                GETSTRING(ARG(1)), -1, &statement, NULL);
    if (result != SQLITE_OK) {
        SETINT(RETURN, fail_sqlite(database_handle->resource.database, result,
                                   "prepare"));
        RESETSIGNAL
        return;
    }
    statement_handle = new_handle(HANDLE_STATEMENT);
    if (!statement_handle) {
        sqlite3_finalize(statement);
        SETINT(RETURN, fail_with(RXSQLITE_NO_MEMORY, SQLITE_NOMEM, "prepare",
                                 "could not allocate statement handle"));
        RESETSIGNAL
        return;
    }
    statement_handle->resource.statement = statement;
    statement_handle->parent = database_handle;
    retain_handle(database_handle);
    SETINT(RETURN, publish_handle(ARG(2), statement_handle));
    RESETSIGNAL
}

PROCEDURE(rxsqlite_finalize) {
    rxsqlite_handle *handle;
    int status;
    if (NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    clear_error();
    status = get_handle(ARG(0), HANDLE_STATEMENT, "finalize", &handle);
    if (status == RXSQLITE_OK) close_statement_resource(handle);
    SETINT(RETURN, status);
    RESETSIGNAL
}

static int get_statement(rxpa_attribute_value value, const char *operation,
                         rxsqlite_handle **result) {
    return get_handle(value, HANDLE_STATEMENT, operation, result);
}

PROCEDURE(rxsqlite_bind_null) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "bind_null", &handle);
    if (result == RXSQLITE_OK) {
        result = sqlite3_bind_null(handle->resource.statement, (int)GETINT(ARG(1)));
        if (result != SQLITE_OK) result = fail_sqlite(sqlite3_db_handle(handle->resource.statement), result, "bind_null");
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_bind_parameter_index) {
    rxsqlite_handle *handle;
    int index;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "bind_parameter_index", &handle);
    if (result == RXSQLITE_OK) {
        index = sqlite3_bind_parameter_index(handle->resource.statement,
                                             GETSTRING(ARG(1)));
        if (index == 0) {
            result = fail_with(RXSQLITE_INVALID_ARGUMENT, 0,
                               "bind_parameter_index",
                               "named parameter does not exist");
        } else {
            SETINT(ARG(2), index);
        }
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_bind_parameter_count) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "bind_parameter_count", &handle);
    if (result == RXSQLITE_OK) {
        SETINT(ARG(1), sqlite3_bind_parameter_count(handle->resource.statement));
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_bind_parameter_name) {
    rxsqlite_handle *handle;
    const char *name;
    int index;
    int count;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "bind_parameter_name", &handle);
    index = (int)GETINT(ARG(1));
    if (result == RXSQLITE_OK) {
        count = sqlite3_bind_parameter_count(handle->resource.statement);
        if (index < 1 || index > count) {
            result = fail_with(RXSQLITE_INVALID_ARGUMENT, 0,
                               "bind_parameter_name",
                               "bind parameter index is outside the statement");
        }
    }
    if (result == RXSQLITE_OK) {
        name = sqlite3_bind_parameter_name(handle->resource.statement, index);
        SETSTRING(ARG(2), name ? name : "");
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_bind_int) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "bind_int", &handle);
    if (result == RXSQLITE_OK) {
        result = sqlite3_bind_int64(handle->resource.statement,
                                    (int)GETINT(ARG(1)),
                                    (sqlite3_int64)GETINT(ARG(2)));
        if (result != SQLITE_OK) result = fail_sqlite(sqlite3_db_handle(handle->resource.statement), result, "bind_int");
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_bind_real) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "bind_real", &handle);
    if (result == RXSQLITE_OK) {
        result = sqlite3_bind_double(handle->resource.statement,
                                     (int)GETINT(ARG(1)), GETFLOAT(ARG(2)));
        if (result != SQLITE_OK) result = fail_sqlite(sqlite3_db_handle(handle->resource.statement), result, "bind_real");
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_bind_text) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "bind_text", &handle);
    if (result == RXSQLITE_OK) {
        result = sqlite3_bind_text(handle->resource.statement,
                                   (int)GETINT(ARG(1)), GETSTRING(ARG(2)), -1,
                                   SQLITE_TRANSIENT);
        if (result != SQLITE_OK) result = fail_sqlite(sqlite3_db_handle(handle->resource.statement), result, "bind_text");
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_bind_blob) {
    rxsqlite_handle *handle;
    const rxpa_native_payload_ops *ops = NULL;
    void *bytes;
    size_t length = 0;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "bind_blob", &handle);
    if (result == RXSQLITE_OK) {
        bytes = GETNATIVEPAYLOAD(ARG(2), &length, &ops, NULL);
        if (ops != NULL || length > (size_t)INT_MAX) {
            result = fail_with(RXSQLITE_TYPE_MISMATCH, 0, "bind_blob",
                               "value is not an ordinary bounded binary value");
        } else {
            static const unsigned char empty_blob = 0;
            const void *sqlite_bytes = length == 0 ? &empty_blob : bytes;
            result = sqlite3_bind_blob(handle->resource.statement,
                                       (int)GETINT(ARG(1)), sqlite_bytes,
                                       (int)length,
                                       SQLITE_TRANSIENT);
            if (result != SQLITE_OK) result = fail_sqlite(sqlite3_db_handle(handle->resource.statement), result, "bind_blob");
        }
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_clear_bindings) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    clear_error();
    result = get_statement(ARG(0), "clear_bindings", &handle);
    if (result == RXSQLITE_OK) {
        result = sqlite3_clear_bindings(handle->resource.statement);
        if (result != SQLITE_OK) result = fail_sqlite(sqlite3_db_handle(handle->resource.statement), result, "clear_bindings");
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_reset) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    clear_error();
    result = get_statement(ARG(0), "reset", &handle);
    if (result == RXSQLITE_OK) {
        result = sqlite3_reset(handle->resource.statement);
        if (result != SQLITE_OK) result = fail_sqlite(sqlite3_db_handle(handle->resource.statement), result, "reset");
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_step) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    clear_error();
    result = get_statement(ARG(0), "step", &handle);
    if (result == RXSQLITE_OK) {
        result = sqlite3_step(handle->resource.statement);
        if (result == SQLITE_ROW) result = RXSQLITE_ROW;
        else if (result == SQLITE_DONE) result = RXSQLITE_DONE;
        else result = fail_sqlite(sqlite3_db_handle(handle->resource.statement), result, "step");
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_column_count) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "column_count", &handle);
    if (result == RXSQLITE_OK) {
        SETINT(ARG(1), sqlite3_column_count(handle->resource.statement));
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_column_name) {
    rxsqlite_handle *handle;
    const char *name;
    int column;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "column_name", &handle);
    column = (int)GETINT(ARG(1));
    if (result == RXSQLITE_OK) result = valid_column(handle, column, "column_name");
    if (result == RXSQLITE_OK) {
        name = sqlite3_column_name(handle->resource.statement, column);
        SETSTRING(ARG(2), name ? name : "");
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_column_type) {
    rxsqlite_handle *handle;
    int column;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "column_type", &handle);
    column = (int)GETINT(ARG(1));
    if (result == RXSQLITE_OK) result = valid_column(handle, column, "column_type");
    if (result == RXSQLITE_OK) {
        SETSTRING(ARG(2), column_type_name(sqlite3_column_type(handle->resource.statement, column)));
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_column_is_null) {
    rxsqlite_handle *handle;
    int column;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "column_is_null", &handle);
    column = (int)GETINT(ARG(1));
    if (result == RXSQLITE_OK) result = valid_column(handle, column, "column_is_null");
    if (result == RXSQLITE_OK) SETINT(ARG(2), sqlite3_column_type(handle->resource.statement, column) == SQLITE_NULL);
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_column_int) {
    rxsqlite_handle *handle;
    int column;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "column_int", &handle);
    column = (int)GETINT(ARG(1));
    if (result == RXSQLITE_OK) result = valid_column(handle, column, "column_int");
    if (result == RXSQLITE_OK && sqlite3_column_type(handle->resource.statement, column) != SQLITE_INTEGER) {
        result = fail_with(RXSQLITE_TYPE_MISMATCH, 0, "column_int", "column is not integer");
    }
    if (result == RXSQLITE_OK) SETINT(ARG(2), (rxinteger)sqlite3_column_int64(handle->resource.statement, column));
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_column_real) {
    rxsqlite_handle *handle;
    int column;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "column_real", &handle);
    column = (int)GETINT(ARG(1));
    if (result == RXSQLITE_OK) result = valid_column(handle, column, "column_real");
    if (result == RXSQLITE_OK && sqlite3_column_type(handle->resource.statement, column) != SQLITE_FLOAT) {
        result = fail_with(RXSQLITE_TYPE_MISMATCH, 0, "column_real", "column is not real");
    }
    if (result == RXSQLITE_OK) SETFLOAT(ARG(2), sqlite3_column_double(handle->resource.statement, column));
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_column_text) {
    rxsqlite_handle *handle;
    const unsigned char *text;
    int column;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "column_text", &handle);
    column = (int)GETINT(ARG(1));
    if (result == RXSQLITE_OK) result = valid_column(handle, column, "column_text");
    if (result == RXSQLITE_OK && sqlite3_column_type(handle->resource.statement, column) != SQLITE_TEXT) {
        result = fail_with(RXSQLITE_TYPE_MISMATCH, 0, "column_text", "column is not text");
    }
    if (result == RXSQLITE_OK) {
        text = sqlite3_column_text(handle->resource.statement, column);
        SETSTRING(ARG(2), (const char *)(text ? text : (const unsigned char *)""));
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_column_blob) {
    rxsqlite_handle *handle;
    const void *bytes;
    int length;
    int column;
    int result;
    if (NUM_ARGS != 3) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "3 arguments expected")
    clear_error();
    result = get_statement(ARG(0), "column_blob", &handle);
    column = (int)GETINT(ARG(1));
    if (result == RXSQLITE_OK) result = valid_column(handle, column, "column_blob");
    if (result == RXSQLITE_OK && sqlite3_column_type(handle->resource.statement, column) != SQLITE_BLOB) {
        result = fail_with(RXSQLITE_TYPE_MISMATCH, 0, "column_blob", "column is not blob");
    }
    if (result == RXSQLITE_OK) {
        bytes = sqlite3_column_blob(handle->resource.statement, column);
        length = sqlite3_column_bytes(handle->resource.statement, column);
        if (SETNATIVEPAYLOAD(ARG(2), bytes, (size_t)length, NULL, 0) != 0) {
            result = fail_with(RXSQLITE_NO_MEMORY, SQLITE_NOMEM, "column_blob",
                               "could not materialize binary column");
        }
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_version) {
    if (NUM_ARGS != 0) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "no arguments expected")
    clear_error();
    SETSTRING(RETURN, sqlite3_libversion());
    RESETSIGNAL
}

PROCEDURE(rxsqlite_changes) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    clear_error();
    result = get_handle(ARG(0), HANDLE_DATABASE, "changes", &handle);
    if (result == RXSQLITE_OK) {
        SETINT(ARG(1), (rxinteger)sqlite3_changes64(handle->resource.database));
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_last_insert_rowid) {
    rxsqlite_handle *handle;
    int result;
    if (NUM_ARGS != 2) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    clear_error();
    result = get_handle(ARG(0), HANDLE_DATABASE, "last_insert_rowid", &handle);
    if (result == RXSQLITE_OK) {
        SETINT(ARG(1), (rxinteger)sqlite3_last_insert_rowid(handle->resource.database));
    }
    SETINT(RETURN, result);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_error) {
    error_state *last_error = &current_session()->last_error;
    if (NUM_ARGS != 4) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "4 arguments expected")
    SETINT(ARG(0), last_error->code);
    SETINT(ARG(1), last_error->sqlite_code);
    SETSTRING(ARG(2), last_error->operation);
    SETSTRING(ARG(3), last_error->message);
    SETINT(RETURN, last_error->code);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_error_extended) {
    error_state *last_error = &current_session()->last_error;
    if (NUM_ARGS != 5) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "5 arguments expected")
    SETINT(ARG(0), last_error->code);
    SETINT(ARG(1), last_error->sqlite_code);
    SETINT(ARG(2), last_error->sqlite_extended_code);
    SETSTRING(ARG(3), last_error->operation);
    SETSTRING(ARG(4), last_error->message);
    SETINT(RETURN, last_error->code);
    RESETSIGNAL
}

static void append_json_char(char *output, size_t capacity, size_t *used, char ch) {
    if (*used + 1 < capacity) output[(*used)++] = ch;
}

static void append_json_string(char *output, size_t capacity, size_t *used,
                               const char *input) {
    const unsigned char *cursor = (const unsigned char *)(input ? input : "");
    append_json_char(output, capacity, used, '"');
    while (*cursor && *used + 8 < capacity) {
        if (*cursor == '"' || *cursor == '\\') {
            append_json_char(output, capacity, used, '\\');
            append_json_char(output, capacity, used, (char)*cursor);
        } else if (*cursor == '\n') {
            append_json_char(output, capacity, used, '\\');
            append_json_char(output, capacity, used, 'n');
        } else if (*cursor == '\r') {
            append_json_char(output, capacity, used, '\\');
            append_json_char(output, capacity, used, 'r');
        } else if (*cursor == '\t') {
            append_json_char(output, capacity, used, '\\');
            append_json_char(output, capacity, used, 't');
        } else if (*cursor >= 0x20) {
            append_json_char(output, capacity, used, (char)*cursor);
        }
        cursor++;
    }
    append_json_char(output, capacity, used, '"');
}

PROCEDURE(rxsqlite_error_json) {
    error_state *last_error = &current_session()->last_error;
    char json[1280];
    size_t used = 0;
    char number[64];
    if (NUM_ARGS != 0) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "no arguments expected")
    append_json_char(json, sizeof(json), &used, '{');
    snprintf(number, sizeof(number),
             "\"code\":%d,\"sqlite_code\":%d,\"sqlite_extended_code\":%d,\"operation\":",
             last_error->code, last_error->sqlite_code,
             last_error->sqlite_extended_code);
    if (used + strlen(number) < sizeof(json)) {
        memcpy(json + used, number, strlen(number));
        used += strlen(number);
    }
    append_json_string(json, sizeof(json), &used, last_error->operation);
    if (used + 11 < sizeof(json)) {
        memcpy(json + used, ",\"message\":", 11);
        used += 11;
    }
    append_json_string(json, sizeof(json), &used, last_error->message);
    append_json_char(json, sizeof(json), &used, '}');
    json[used] = '\0';
    SETSTRING(RETURN, json);
    RESETSIGNAL
}

PROCEDURE(rxsqlite_cleanup) {
    rxsqlite_session *session = current_session();
    rxsqlite_handle *cursor;
    int closed = 0;
    if (NUM_ARGS != 0) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "no arguments expected")
    clear_error();
    for (cursor = session->live_handles; cursor; cursor = cursor->next) {
        if (cursor->kind == HANDLE_STATEMENT && !cursor->closed) {
            close_statement_resource(cursor);
            closed++;
        }
    }
    for (cursor = session->live_handles; cursor; cursor = cursor->next) {
        if (cursor->kind == HANDLE_DATABASE && !cursor->closed) {
            close_database_resource(cursor);
            closed++;
        }
    }
    SETINT(RETURN, closed);
    RESETSIGNAL
}

static void close_session_resources(rxsqlite_session *session) {
    rxsqlite_handle *cursor;
    rxsqlite_handle *next;
    if (!session) return;
    for (cursor = session->live_handles; cursor; cursor = cursor->next) {
        if (cursor->kind == HANDLE_STATEMENT) close_statement_resource(cursor);
    }
    for (cursor = session->live_handles; cursor; cursor = cursor->next) {
        if (cursor->kind == HANDLE_DATABASE) close_database_resource(cursor);
    }
    cursor = session->live_handles;
    while (cursor) {
        next = cursor->next;
        free(cursor);
        cursor = next;
    }
    session->live_handles = NULL;
    memset(&session->last_error, 0, sizeof(session->last_error));
}

static void *rxsqlite_session_create(void) {
    return calloc(1, sizeof(rxsqlite_session));
}

static void rxsqlite_session_destroy(void *opaque_session) {
    rxsqlite_session *session =
        (rxsqlite_session *)opaque_session;
    if (!session) return;
    close_session_resources(session);
    free(session);
}

static int rxsqlite_session_enter(void *opaque_session,
                                         uint32_t capabilities,
                                         void **previous) {
    if (!opaque_session || !previous ||
        capabilities != RXPA_PROCEDURE_CAP_SESSION_AFFINE) return -1;
    *previous = rxsqlite_current_session;
    rxsqlite_current_session =
        (rxsqlite_session *)opaque_session;
    return 0;
}

static void rxsqlite_session_leave(void *previous) {
    rxsqlite_current_session =
        (rxsqlite_session *)previous;
}

FINALIZER(rxsqlite_shutdown)
    close_session_resources(&rxsqlite_default_session);
}

LOADFUNCS
    ADDPROC(rxsqlite_open, "rxsqlite.sqliteopen", "b", ".int",
            "path=.string, expose handle=.binary");
    ADDPROC(rxsqlite_open_mode, "rxsqlite.sqliteopenmode", "b", ".int",
            "path=.string, mode=.string, expose handle=.binary");
    ADDPROC(rxsqlite_close, "rxsqlite.sqliteclose", "b", ".int",
            "handle=.binary");
    ADDPROC(rxsqlite_exec, "rxsqlite.sqliteexec", "b", ".int",
            "handle=.binary, sql=.string");
    ADDPROC(rxsqlite_capability, "rxsqlite.sqlitecapability", "b", ".int",
            "handle=.binary, name=.string, expose available=.int");
    ADDPROC(rxsqlite_busy_timeout, "rxsqlite.sqlitebusytimeout", "b", ".int",
            "handle=.binary, milliseconds=.int");
    ADDPROC(rxsqlite_checkpoint, "rxsqlite.sqlitecheckpoint", "b", ".int",
            "handle=.binary, mode=.string, expose log_frames=.int, expose checkpointed_frames=.int");
    ADDPROC(rxsqlite_backup, "rxsqlite.sqlitebackup", "b", ".int",
            "source=.binary, destination=.binary, pages_per_step=.int, busy_retries=.int, sleep_milliseconds=.int, expose remaining=.int, expose page_count=.int");
    ADDPROC(rxsqlite_integrity, "rxsqlite.sqliteintegrity", "b", ".int",
            "handle=.binary, mode=.string, max_errors=.int, expose ok=.int, expose details=.string");
    ADDPROC(rxsqlite_prepare, "rxsqlite.sqliteprepare", "b", ".int",
            "handle=.binary, sql=.string, expose statement=.binary");
    ADDPROC(rxsqlite_finalize, "rxsqlite.sqlitefinalize", "b", ".int",
            "statement=.binary");
    ADDPROC(rxsqlite_bind_null, "rxsqlite.sqlitebindnull", "b", ".int",
            "statement=.binary, index=.int");
    ADDPROC(rxsqlite_bind_parameter_index, "rxsqlite.sqlitebindindex", "b", ".int",
            "statement=.binary, name=.string, expose index=.int");
    ADDPROC(rxsqlite_bind_parameter_count, "rxsqlite.sqlitebindcount", "b", ".int",
            "statement=.binary, expose count=.int");
    ADDPROC(rxsqlite_bind_parameter_name, "rxsqlite.sqlitebindname", "b", ".int",
            "statement=.binary, index=.int, expose name=.string");
    ADDPROC(rxsqlite_bind_int, "rxsqlite.sqlitebindint", "b", ".int",
            "statement=.binary, index=.int, value=.int");
    ADDPROC(rxsqlite_bind_real, "rxsqlite.sqlitebindreal", "b", ".int",
            "statement=.binary, index=.int, value=.float");
    ADDPROC(rxsqlite_bind_text, "rxsqlite.sqlitebindtext", "b", ".int",
            "statement=.binary, index=.int, value=.string");
    ADDPROC(rxsqlite_bind_blob, "rxsqlite.sqlitebindblob", "b", ".int",
            "statement=.binary, index=.int, value=.binary");
    ADDPROC(rxsqlite_clear_bindings, "rxsqlite.sqliteclearbindings", "b", ".int",
            "statement=.binary");
    ADDPROC(rxsqlite_reset, "rxsqlite.sqlitereset", "b", ".int",
            "statement=.binary");
    ADDPROC(rxsqlite_step, "rxsqlite.sqlitestep", "b", ".int",
            "statement=.binary");
    ADDPROC(rxsqlite_column_count, "rxsqlite.sqlitecolumncount", "b", ".int",
            "statement=.binary, expose count=.int");
    ADDPROC(rxsqlite_column_name, "rxsqlite.sqlitecolumnname", "b", ".int",
            "statement=.binary, column=.int, expose name=.string");
    ADDPROC(rxsqlite_column_type, "rxsqlite.sqlitecolumntype", "b", ".int",
            "statement=.binary, column=.int, expose value=.string");
    ADDPROC(rxsqlite_column_is_null, "rxsqlite.sqlitecolumnisnull", "b", ".int",
            "statement=.binary, column=.int, expose value=.int");
    ADDPROC(rxsqlite_column_int, "rxsqlite.sqlitecolumnint", "b", ".int",
            "statement=.binary, column=.int, expose value=.int");
    ADDPROC(rxsqlite_column_real, "rxsqlite.sqlitecolumnreal", "b", ".int",
            "statement=.binary, column=.int, expose value=.float");
    ADDPROC(rxsqlite_column_text, "rxsqlite.sqlitecolumntext", "b", ".int",
            "statement=.binary, column=.int, expose value=.string");
    ADDPROC(rxsqlite_column_blob, "rxsqlite.sqlitecolumnblob", "b", ".int",
            "statement=.binary, column=.int, expose value=.binary");
    ADDPROC(rxsqlite_version, "rxsqlite.sqliteversion", "b", ".string", "");
    ADDPROC(rxsqlite_changes, "rxsqlite.sqlitechanges", "b", ".int",
            "handle=.binary, expose changes=.int");
    ADDPROC(rxsqlite_last_insert_rowid, "rxsqlite.sqlitelastinsertrowid", "b", ".int",
            "handle=.binary, expose rowid=.int");
    ADDPROC(rxsqlite_error, "rxsqlite.sqliteerror", "b", ".int",
            "expose code=.int, expose sqlite_code=.int, expose operation=.string, expose message=.string");
    ADDPROC(rxsqlite_error_extended, "rxsqlite.sqliteerrorextended", "b", ".int",
            "expose code=.int, expose sqlite_code=.int, expose sqlite_extended_code=.int, expose operation=.string, expose message=.string");
    ADDPROC(rxsqlite_error_json, "rxsqlite.sqliteerrorjson", "b", ".string", "");
    ADDPROC(rxsqlite_cleanup, "rxsqlite.sqlitecleanup", "b", ".int", "");
ENDLOADFUNCS

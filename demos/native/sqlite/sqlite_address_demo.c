#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvml.h"
#include "sqlite3.h"

#ifndef CREXX_NATIVE_SQLITE_LIBRARY_PATH
#define CREXX_NATIVE_SQLITE_LIBRARY_PATH "library"
#endif

#ifndef CREXX_NATIVE_SQLITE_DEMO_MODULE
#define CREXX_NATIVE_SQLITE_DEMO_MODULE "sqlite_address_demo.rxbin"
#endif

#define DB_NAME_SIZE 64
#define DB_PATH_SIZE 512
#define DB_VALUE_SIZE 512
#define DB_SQL_SIZE 4096
#define DB_RESULT_SIZE 8192

typedef struct db_state db_state;

typedef struct db_driver {
    const char* name;
    int (*open)(db_state* state, const char* path);
    void (*close)(db_state* state);
    int (*exec)(db_state* state, const rxvml_address_request* request, const char* sql);
    int (*query)(db_state* state, const rxvml_address_request* request, const char* sql);
    int (*scalar)(db_state* state, const rxvml_address_request* request, const char* sql, char* out, size_t out_len);
    const char* (*version)(void);
    int (*changes)(db_state* state);
    long long (*last_insert_id)(db_state* state);
} db_driver;

struct db_state {
    const db_driver* opened_driver;
    void* handle;
    char instance_id[DB_NAME_SIZE];
    char diagnostic[DB_RESULT_SIZE];
    char result[DB_RESULT_SIZE];
    char update_name[DB_NAME_SIZE];
    char update_value[DB_VALUE_SIZE];
    rxvml_address_binding update_binding;
};

static void copy_text(char* dest, size_t dest_len, const char* src) {
    if (!dest || dest_len == 0) return;
    if (!src) src = "";
    strncpy(dest, src, dest_len - 1);
    dest[dest_len - 1] = 0;
}

static void copy_trimmed_text(char* dest, size_t dest_len, const char* src) {
    const char* start = src ? src : "";
    const char* end;
    size_t len;

    if (!dest || dest_len == 0) return;
    while (*start && isspace((unsigned char)*start)) start++;
    end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    len = (size_t)(end - start);
    if (len >= dest_len) len = dest_len - 1;
    memcpy(dest, start, len);
    dest[len] = 0;
}

static char upper_char(char ch) {
    return (char)toupper((unsigned char)ch);
}

static void normalize_word(char* text) {
    size_t i;
    if (!text) return;
    for (i = 0; text[i]; i++) text[i] = upper_char(text[i]);
}

static int text_equal_ci(const char* left, const char* right) {
    if (!left || !right) return 0;
    while (*left && *right) {
        if (upper_char(*left) != upper_char(*right)) return 0;
        left++;
        right++;
    }
    return *left == 0 && *right == 0;
}

static const char* skip_spaces(const char* text) {
    while (text && *text && isspace((unsigned char)*text)) text++;
    return text ? text : "";
}

static const char* read_word(const char* text, char* out, size_t out_len) {
    size_t i = 0;

    if (out && out_len > 0) out[0] = 0;
    text = skip_spaces(text);
    while (*text && !isspace((unsigned char)*text)) {
        if (out && out_len > 0 && i + 1 < out_len) out[i++] = *text;
        text++;
    }
    if (out && out_len > 0) out[i] = 0;
    return skip_spaces(text);
}

static void append_text(char* dest, size_t dest_len, const char* text) {
    size_t used;
    if (!dest || dest_len == 0 || !text) return;
    used = strlen(dest);
    if (used + 1 >= dest_len) return;
    strncat(dest, text, dest_len - used - 1);
}

static int anchor_name_start(char ch) {
    return ch == '_' || isalpha((unsigned char)ch);
}

static int anchor_name_part(char ch) {
    return anchor_name_start(ch) || isdigit((unsigned char)ch);
}

static int copy_anchor_name(const char* name, size_t len, char* out, size_t out_len) {
    size_t i;

    if (!out || out_len == 0) return 0;
    out[0] = 0;
    if (!name || len == 0 || len >= out_len) return 0;
    if (!anchor_name_start(name[0])) return 0;
    for (i = 1; i < len; i++) {
        if (!anchor_name_part(name[i])) return 0;
    }

    memcpy(out, name, len);
    out[len] = 0;
    return 1;
}

static int anchor_name(const char* token, char* out, size_t out_len) {
    const char* close;

    if (out && out_len > 0) out[0] = 0;
    if (!token || !*token) return 0;

    if (token[0] == ':') {
        return copy_anchor_name(token + 1, strlen(token + 1), out, out_len);
    }

    if (token[0] == '$' && token[1] == '{') {
        close = strchr(token + 2, '}');
        if (close && close[1] == 0) {
            return copy_anchor_name(token + 2, (size_t)(close - (token + 2)), out, out_len);
        }
    }

    return 0;
}

static int resolve_token(
    const rxvml_address_request* request,
    const char* token,
    char* out,
    size_t out_len) {

    char anchor[DB_NAME_SIZE];

    if (anchor_name(token, anchor, sizeof(anchor))) {
        return rxvml_address_binding_get(request, anchor, out, out_len) == 0 ? 0 : -1;
    }

    copy_text(out, out_len, token);
    return 0;
}

static int fail_response(db_state* state, rxvml_address_response* response, int rc, const char* message) {
    if (!state || !response) return -1;
    copy_text(state->diagnostic, sizeof(state->diagnostic), message);
    response->rc = rc;
    response->condition_name = "FAILURE";
    response->diagnostic = state->diagnostic;
    return 0;
}

static int fail_sqlite(db_state* state, rxvml_address_response* response, int rc, const char* prefix) {
    sqlite3* db = state ? (sqlite3*)state->handle : NULL;
    snprintf(state->diagnostic, sizeof(state->diagnostic), "%s: %s",
             prefix ? prefix : "SQLite error",
             db ? sqlite3_errmsg(db) : "database is not open");
    response->rc = rc;
    response->condition_name = "FAILURE";
    response->diagnostic = state->diagnostic;
    return 0;
}

static int ensure_open(db_state* state, const db_driver* driver) {
    if (!state || !driver) return -1;
    if (state->handle) return 0;
    if (driver->open(state, ":memory:") != 0) return -1;
    state->opened_driver = driver;
    return 0;
}

static int bind_host_parameters(
    sqlite3_stmt* stmt,
    const rxvml_address_request* request,
    char* diagnostic,
    size_t diagnostic_len) {

    int i;
    int count;

    if (!stmt || !request) return 0;

    count = sqlite3_bind_parameter_count(stmt);
    for (i = 1; i <= count; i++) {
        const char* parameter = sqlite3_bind_parameter_name(stmt, i);
        const char* name = parameter;
        char value[DB_VALUE_SIZE];

        if (!parameter || parameter[0] == 0) continue;
        if (parameter[0] == ':' || parameter[0] == '$' || parameter[0] == '@') name = parameter + 1;
        if (rxvml_address_binding_get(request, name, value, sizeof(value)) != 0) {
            snprintf(diagnostic, diagnostic_len, "Missing host variable for SQL parameter %s", parameter);
            return -1;
        }
        if (sqlite3_bind_text(stmt, i, value, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            snprintf(diagnostic, diagnostic_len, "Failed to bind SQL parameter %s", parameter);
            return -1;
        }
    }

    return 0;
}

static int sqlite_driver_open(db_state* state, const char* path) {
    sqlite3* db = NULL;
    int rc;

    if (!state) return -1;
    if (state->handle) sqlite3_close((sqlite3*)state->handle);
    state->handle = NULL;

    rc = sqlite3_open(path && *path ? path : ":memory:", &db);
    if (rc != SQLITE_OK) {
        if (db) {
            snprintf(state->diagnostic, sizeof(state->diagnostic), "sqlite3_open failed: %s", sqlite3_errmsg(db));
            sqlite3_close(db);
        }
        return -1;
    }

    state->handle = db;
    return 0;
}

static void sqlite_driver_close(db_state* state) {
    if (!state || !state->handle) return;
    sqlite3_close((sqlite3*)state->handle);
    state->handle = NULL;
}

static int sqlite_driver_exec(db_state* state, const rxvml_address_request* request, const char* sql) {
    sqlite3* db = (sqlite3*)state->handle;
    sqlite3_stmt* stmt = NULL;
    int rc;

    (void)request;
    if (!db || !sql || !*sql) return -1;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;
    if (bind_host_parameters(stmt, request, state->diagnostic, sizeof(state->diagnostic)) != 0) {
        sqlite3_finalize(stmt);
        return -2;
    }

    do {
        rc = sqlite3_step(stmt);
    } while (rc == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

static int sqlite_driver_query(db_state* state, const rxvml_address_request* request, const char* sql) {
    sqlite3* db = (sqlite3*)state->handle;
    sqlite3_stmt* stmt = NULL;
    int rc;
    int columns;

    if (!db || !sql || !*sql) return -1;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;
    if (bind_host_parameters(stmt, request, state->diagnostic, sizeof(state->diagnostic)) != 0) {
        sqlite3_finalize(stmt);
        return -2;
    }

    columns = sqlite3_column_count(stmt);
    state->result[0] = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int i;
        for (i = 0; i < columns; i++) {
            const unsigned char* text;
            if (i > 0) append_text(state->result, sizeof(state->result), "|");
            text = sqlite3_column_text(stmt, i);
            append_text(state->result, sizeof(state->result), text ? (const char*)text : "");
        }
        append_text(state->result, sizeof(state->result), "\n");
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

static int sqlite_driver_scalar(
    db_state* state,
    const rxvml_address_request* request,
    const char* sql,
    char* out,
    size_t out_len) {

    sqlite3* db = (sqlite3*)state->handle;
    sqlite3_stmt* stmt = NULL;
    int rc;

    if (out && out_len > 0) out[0] = 0;
    if (!db || !sql || !*sql || !out || out_len == 0) return -1;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;
    if (bind_host_parameters(stmt, request, state->diagnostic, sizeof(state->diagnostic)) != 0) {
        sqlite3_finalize(stmt);
        return -2;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        copy_text(out, out_len, text ? (const char*)text : "");
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

static const char* sqlite_driver_version(void) {
    return sqlite3_libversion();
}

static int sqlite_driver_changes(db_state* state) {
    if (!state || !state->handle) return 0;
    return sqlite3_changes((sqlite3*)state->handle);
}

static long long sqlite_driver_last_insert_id(db_state* state) {
    if (!state || !state->handle) return 0;
    return (long long)sqlite3_last_insert_rowid((sqlite3*)state->handle);
}

static const db_driver SQLITE_DRIVER = {
    "sqlite",
    sqlite_driver_open,
    sqlite_driver_close,
    sqlite_driver_exec,
    sqlite_driver_query,
    sqlite_driver_scalar,
    sqlite_driver_version,
    sqlite_driver_changes,
    sqlite_driver_last_insert_id
};

static const db_driver* find_driver_for_environment(const char* env_name) {
    if (text_equal_ci(env_name, "SQLITE") || text_equal_ci(env_name, "SQLITE3")) {
        return &SQLITE_DRIVER;
    }
    return NULL;
}

static int prepare_var_update(
    db_state* state,
    const rxvml_address_request* request,
    rxvml_address_response* response,
    const char* target,
    const char* value) {

    char current[DB_VALUE_SIZE];

    if (rxvml_address_binding_get(request, target, current, sizeof(current)) != 0) return -1;

    copy_text(state->update_name, sizeof(state->update_name), target);
    copy_text(state->update_value, sizeof(state->update_value), value);
    state->update_binding.kind = "var";
    state->update_binding.internal_name = state->update_name;
    state->update_binding.external_alias = state->update_name;
    state->update_binding.value = state->update_value;
    state->update_binding.value_object = NULL;
    state->update_binding.flags = "";

    response->updated_binding_count = 1;
    response->updated_bindings = &state->update_binding;
    response->rc = 0;
    return 0;
}

static const char* find_last_into(const char* text) {
    const char* last = NULL;
    size_t len;
    size_t i;

    if (!text) return NULL;
    len = strlen(text);
    if (len < 6) return NULL;

    for (i = 0; i + 6 <= len; i++) {
        if ((i == 0 || isspace((unsigned char)text[i - 1])) &&
            upper_char(text[i]) == 'I' &&
            upper_char(text[i + 1]) == 'N' &&
            upper_char(text[i + 2]) == 'T' &&
            upper_char(text[i + 3]) == 'O' &&
            isspace((unsigned char)text[i + 4])) {
            last = text + i;
        }
    }

    return last;
}

static int parse_scalar_into(
    const char* text,
    char* sql,
    size_t sql_len,
    char* target,
    size_t target_len) {

    const char* into_pos = find_last_into(text);
    char target_token[DB_NAME_SIZE];
    size_t len;

    if (!into_pos) return -1;
    len = (size_t)(into_pos - text);
    if (len >= sql_len) len = sql_len - 1;
    memcpy(sql, text, len);
    sql[len] = 0;
    copy_trimmed_text(sql, sql_len, sql);

    copy_trimmed_text(target_token, sizeof(target_token), into_pos + 4);
    if (!anchor_name(target_token, target, target_len)) return -1;
    return 0;
}

static int db_emit(rxvml_context* ctx, const rxvml_address_request* request, const char* text) {
    return rxvml_address_emit_output(ctx, request, text);
}

static int db_command(
    rxvml_context* ctx,
    const rxvml_address_request* request,
    rxvml_address_response* response,
    void* userdata) {

    db_state* state = (db_state*)userdata;
    const db_driver* driver;
    char op[32];
    char path_token[DB_PATH_SIZE];
    char path[DB_PATH_SIZE];
    char sql[DB_SQL_SIZE];
    char target[DB_NAME_SIZE];
    char scalar_value[DB_VALUE_SIZE];
    const char* rest;
    int rc;

    if (!state || !request || !response) return -1;

    driver = find_driver_for_environment(request->environment_name);
    if (!driver) return fail_response(state, response, -3, "No database driver registered for ADDRESS environment");

    rest = read_word(request->command, op, sizeof(op));
    normalize_word(op);

    if (strcmp(op, "OPEN") == 0 || strcmp(op, "CONNECT") == 0) {
        read_word(rest, path_token, sizeof(path_token));
        if (path_token[0] == 0) copy_text(path_token, sizeof(path_token), ":memory:");
        if (resolve_token(request, path_token, path, sizeof(path)) != 0)
            return fail_response(state, response, 8, "OPEN path anchor was not exposed");
        if (state->opened_driver && state->opened_driver != driver) state->opened_driver->close(state);
        if (driver->open(state, path) != 0)
            return fail_response(state, response, 8, state->diagnostic);
        state->opened_driver = driver;
        response->rc = 0;
        return 0;
    }

    if (strcmp(op, "CLOSE") == 0) {
        if (state->opened_driver) state->opened_driver->close(state);
        state->opened_driver = NULL;
        response->rc = 0;
        return 0;
    }

    if (strcmp(op, "EXEC") == 0 || strcmp(op, "EXECUTE") == 0) {
        if (ensure_open(state, driver) != 0) return fail_response(state, response, 8, "Could not open default database");
        rc = driver->exec(state, request, rest);
        if (rc == -2) return fail_response(state, response, 8, state->diagnostic);
        if (rc != 0) return fail_sqlite(state, response, 8, "EXEC failed");
        response->rc = 0;
        return 0;
    }

    if (strcmp(op, "QUERY") == 0) {
        if (ensure_open(state, driver) != 0) return fail_response(state, response, 8, "Could not open default database");
        rc = driver->query(state, request, rest);
        if (rc == -2) return fail_response(state, response, 8, state->diagnostic);
        if (rc != 0) return fail_sqlite(state, response, 8, "QUERY failed");
        return db_emit(ctx, request, state->result);
    }

    if (strcmp(op, "VALUE") == 0 || strcmp(op, "SCALAR") == 0) {
        if (parse_scalar_into(rest, sql, sizeof(sql), target, sizeof(target)) != 0)
            return fail_response(state, response, 8, "VALUE requires SQL followed by INTO ${target}");
        if (ensure_open(state, driver) != 0) return fail_response(state, response, 8, "Could not open default database");
        rc = driver->scalar(state, request, sql, scalar_value, sizeof(scalar_value));
        if (rc == -2) return fail_response(state, response, 8, state->diagnostic);
        if (rc != 0) return fail_sqlite(state, response, 8, "VALUE failed");
        if (prepare_var_update(state, request, response, target, scalar_value) != 0)
            return fail_response(state, response, 8, "VALUE INTO target was not exposed");
        return 0;
    }

    if (strcmp(op, "HELP") == 0 || op[0] == 0) {
        return db_emit(ctx, request, "OPEN path | EXEC sql | QUERY sql | VALUE sql INTO ${target} | CLOSE\n");
    }

    return fail_response(state, response, -3, "Unknown database command");
}

static int db_function(
    rxvml_context* ctx,
    const rxvml_address_function_request* request,
    rxvml_address_function_response* response,
    void* userdata) {

    db_state* state = (db_state*)userdata;
    const db_driver* driver;
    char name[64];

    (void)ctx;
    if (!state || !request || !response) return -1;

    driver = find_driver_for_environment(request->environment_name);
    if (!driver) {
        response->rc = -3;
        response->condition_name = "FAILURE";
        response->diagnostic = "No database driver registered for ADDRESS function environment";
        response->result = "";
        return 0;
    }

    copy_text(name, sizeof(name), request->function_name);
    normalize_word(name);

    if (strcmp(name, "DRIVER") == 0) {
        response->result = driver->name;
        return 0;
    }

    if (strcmp(name, "VERSION") == 0) {
        response->result = driver->version();
        return 0;
    }

    if (strcmp(name, "CHANGES") == 0) {
        snprintf(state->result, sizeof(state->result), "%d", driver->changes(state));
        response->result = state->result;
        return 0;
    }

    if (strcmp(name, "LAST_INSERT_ROWID") == 0 || strcmp(name, "LAST_INSERT_ID") == 0) {
        snprintf(state->result, sizeof(state->result), "%lld", driver->last_insert_id(state));
        response->result = state->result;
        return 0;
    }

    if (strcmp(name, "SCALAR") == 0) {
        if (request->argc < 1) {
            response->rc = 8;
            response->condition_name = "FAILURE";
            response->diagnostic = "SCALAR requires a SQL argument";
            response->result = "";
            return 0;
        }
        if (ensure_open(state, driver) != 0 ||
            driver->scalar(state, NULL, request->args[0], state->result, sizeof(state->result)) != 0) {
            response->rc = 8;
            response->condition_name = "FAILURE";
            response->diagnostic = "SCALAR failed";
            response->result = "";
            return 0;
        }
        response->result = state->result;
        return 0;
    }

    response->rc = -3;
    response->condition_name = "FAILURE";
    response->diagnostic = "Unknown database function";
    response->result = "";
    return 0;
}

static void print_last_error(rxvml_context* ctx, const char* prefix) {
    const char* err = NULL;
    rxvml_last_error(ctx, &err);
    fprintf(stderr, "%s: %s\n", prefix, err ? err : "unknown error");
}

int main(int argc, const char** argv) {
    rxvml_context* ctx = NULL;
    db_state state;
    int program_rc = 1;
    int status = 1;

    (void)argc;
    (void)argv;

    memset(&state, 0, sizeof(state));
    copy_text(state.instance_id, sizeof(state.instance_id), "native-sqlite-demo");

    ctx = rxvml_create(NULL, 0);
    if (!ctx) {
        fprintf(stderr, "Failed to create rxvml context\n");
        return 1;
    }

    if (rxvml_load_module_file(ctx, CREXX_NATIVE_SQLITE_LIBRARY_PATH) <= 0) {
        print_last_error(ctx, "Failed to load library");
        goto cleanup;
    }

    if (rxvml_load_module_file(ctx, CREXX_NATIVE_SQLITE_DEMO_MODULE) <= 0) {
        print_last_error(ctx, "Failed to load SQLite Rexx demo module");
        goto cleanup;
    }

    if (rxvml_address_register_callback_environment(
            ctx, "SQLITE", state.instance_id, db_command, db_function, &state) != 0) {
        print_last_error(ctx, "Failed to register SQLite ADDRESS environment");
        goto cleanup;
    }

    if (rxvml_run(ctx, 0, NULL, &program_rc) != 0) {
        print_last_error(ctx, "Failed to run SQLite ADDRESS demo");
        goto cleanup;
    }

    status = program_rc;

cleanup:
    if (state.opened_driver) state.opened_driver->close(&state);
    rxvml_destroy(ctx);
    return status;
}

/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rxcpmain.h"
#include "rxcp_import_publish.h"
#include "rxcp_import_report.h"
#include "rxsha256.h"

typedef struct RxcpImportEvent {
    size_t sequence;
    char *module;
    char *path;
    file_type type;
    RxcpImportRootKind root_kind;
    size_t root_index;
    char digest[RX_SHA256_DIGEST_SIZE * 2 + 1];
    const char *digest_status;
    time_t mtime;
    char *decision;
    char *reason;
    char *replaced_path;
} RxcpImportEvent;

struct RxcpImportReport {
    RxcpImportEvent *events;
    size_t event_count;
};

static char *report_strdup(const char *value, const char *operation) {
    char *copy;
    size_t size;

    if (!value) return 0;
    size = strlen(value) + 1;
    copy = (char *)malloc(size);
    if (!copy) RX_PANIC_OOM(operation, size, value);
    memcpy(copy, value, size);
    return copy;
}

static size_t module_stem_length(const char *name) {
    const char *dot;

    if (!name) return 0;
    dot = strrchr(name, '.');
    return dot ? (size_t)(dot - name) : strlen(name);
}

static char *module_name_copy(const char *name) {
    char *module;
    size_t length;

    length = module_stem_length(name);
    module = (char *)malloc(length + 1);
    if (!module) RX_PANIC_OOM("malloc import report module", length + 1, name);
    if (length) memcpy(module, name, length);
    module[length] = 0;
    return module;
}

static const char *root_kind_name(RxcpImportRootKind kind) {
    switch (kind) {
        case RXCP_IMPORT_ROOT_PRIMARY_SOURCE: return "primary-source";
        case RXCP_IMPORT_ROOT_SOURCE: return "source";
        case RXCP_IMPORT_ROOT_BINARY: return "binary";
        case RXCP_IMPORT_ROOT_EXECUTABLE: return "executable";
        default: return "binary";
    }
}

static const char *artifact_kind_name(file_type type) {
    switch (type) {
        case REXX_FILE: return "source";
        case RXAS_FILE: return "rxas";
        case RXBIN_FILE: return "rxbin";
        case NATIVE_FILE: return "rxplugin";
        default: return "rxbin";
    }
}

static const char *stage_name(const importable_file *file) {
    if (file->type == NATIVE_FILE) return "native";
    if (file->source_root) return "source";
    return "binary";
}

static char *logical_root_name(RxcpImportRootKind kind, size_t index) {
    const char *kind_name;
    char buffer[96];
    int written;

    kind_name = root_kind_name(kind);
    written = snprintf(buffer, sizeof(buffer), "@%s/%lu", kind_name, (unsigned long)index);
    if (written < 0 || (size_t)written >= sizeof(buffer)) {
        RX_PANIC_OOM("format import report logical root", sizeof(buffer), kind_name);
    }
    return report_strdup(buffer, "strdup import report logical root");
}

static char *logical_candidate_path(const importable_file *file) {
    char *root;
    char *path;
    size_t size;

    root = logical_root_name(file->root_kind, file->root_index);
    size = strlen(root) + strlen(file->name) + 2;
    path = (char *)malloc(size);
    if (!path) {
        free(root);
        RX_PANIC_OOM("malloc import report candidate path", size, file->name);
    }
    snprintf(path, size, "%s/%s", root, file->name);
    free(root);
    return path;
}

static char *physical_candidate_path(const importable_file *file) {
    size_t directory_length;
    size_t name_length;
    size_t size;
    int needs_separator;
    char *path;

    if (!file->location || !file->location[0]) {
        return report_strdup(file->name, "strdup import report physical path");
    }

    directory_length = strlen(file->location);
    name_length = strlen(file->name);
    needs_separator = file->location[directory_length - 1] != '/' &&
                      file->location[directory_length - 1] != '\\';
    size = directory_length + (size_t)needs_separator + name_length + 1;
    path = (char *)malloc(size);
    if (!path) RX_PANIC_OOM("malloc import report physical path", size, file->name);
    memcpy(path, file->location, directory_length);
    if (needs_separator) path[directory_length++] = '/';
    memcpy(path + directory_length, file->name, name_length);
    path[directory_length + name_length] = 0;
    return path;
}

static void digest_to_hex(const unsigned char digest[RX_SHA256_DIGEST_SIZE], char *hex) {
    static const char digits[] = "0123456789abcdef";
    size_t i;

    for (i = 0; i < RX_SHA256_DIGEST_SIZE; i++) {
        hex[i * 2] = digits[(digest[i] >> 4) & 0x0f];
        hex[i * 2 + 1] = digits[digest[i] & 0x0f];
    }
    hex[RX_SHA256_DIGEST_SIZE * 2] = 0;
}

static void capture_digest(const importable_file *file, char *hex, const char **status_name) {
    unsigned char digest[RX_SHA256_DIGEST_SIZE];
    rx_sha256_file_status status;
    char *path;
    FILE *probe;

    hex[0] = 0;
    path = physical_candidate_path(file);
    status = rx_sha256_file(path, digest);
    if (status == RX_SHA256_FILE_OK) {
        digest_to_hex(digest, hex);
        *status_name = "available";
    } else if (status == RX_SHA256_FILE_OPEN_FAILURE) {
        probe = fopen(path, "rb");
        if (probe) {
            fclose(probe);
            *status_name = "read-error";
        } else if (errno == ENOENT) {
            *status_name = "missing";
        } else {
            *status_name = "read-error";
        }
    } else {
        *status_name = "read-error";
    }
    free(path);
}

static RxcpImportReport *report_for(Context *context, int create) {
    Context *owner;

    if (!context) return 0;
    owner = context->master_context ? context->master_context : context;
    if (!owner->import_resolution_report_path) return 0;
    if (!owner->import_resolution_report && create) {
        owner->import_resolution_report = (RxcpImportReport *)calloc(1, sizeof(RxcpImportReport));
        if (!owner->import_resolution_report) {
            RX_PANIC_OOM("calloc import resolution report", sizeof(RxcpImportReport),
                         owner->import_resolution_report_path);
        }
    }
    return owner->import_resolution_report;
}

static const RxcpImportEvent *find_captured_candidate(const RxcpImportReport *report,
                                                       const char *path,
                                                       file_type type,
                                                       RxcpImportRootKind root_kind,
                                                       size_t root_index,
                                                       time_t mtime) {
    size_t index;
    const RxcpImportEvent *event;

    if (!report || !path) return 0;
    index = report->event_count;
    while (index) {
        event = &report->events[--index];
        if (event->type == type && event->root_kind == root_kind &&
            event->root_index == root_index && event->mtime == mtime &&
            strcmp(event->path, path) == 0) return event;
    }
    return 0;
}

void rxcp_import_report_record(Context *context,
                               const importable_file *candidate,
                               const char *decision,
                               const char *reason,
                               const importable_file *replaced) {
    RxcpImportReport *report;
    RxcpImportEvent *resized;
    RxcpImportEvent *event;

    if (!candidate) return;
    report = report_for(context, 1);
    if (!report) return;

    resized = (RxcpImportEvent *)realloc(
            report->events, (report->event_count + 1) * sizeof(RxcpImportEvent));
    if (!resized) {
        RX_PANIC_OOM("realloc import resolution events",
                     (report->event_count + 1) * sizeof(RxcpImportEvent), candidate->name);
    }
    report->events = resized;
    event = &report->events[report->event_count];
    memset(event, 0, sizeof(*event));
    event->sequence = report->event_count;
    event->module = module_name_copy(candidate->name);
    event->path = logical_candidate_path(candidate);
    event->type = candidate->type;
    event->root_kind = candidate->root_kind;
    event->root_index = candidate->root_index;
    event->mtime = candidate->mtime;
    event->decision = report_strdup(decision, "strdup import report decision");
    event->reason = report_strdup(reason, "strdup import report reason");
    if (replaced) event->replaced_path = logical_candidate_path(replaced);
    {
        const RxcpImportEvent *captured;

        captured = find_captured_candidate(report, event->path, event->type,
                                           event->root_kind, event->root_index,
                                           event->mtime);
        if (captured) {
            memcpy(event->digest, captured->digest, sizeof(event->digest));
            event->digest_status = captured->digest_status;
        } else {
            capture_digest(candidate, event->digest, &event->digest_status);
        }
    }
    report->event_count++;
}

static void write_json_string(FILE *file, const char *value) {
    const unsigned char *cursor;

    fputc('"', file);
    cursor = (const unsigned char *)(value ? value : "");
    while (*cursor) {
        switch (*cursor) {
            case '"': fputs("\\\"", file); break;
            case '\\': fputs("\\\\", file); break;
            case '\b': fputs("\\b", file); break;
            case '\f': fputs("\\f", file); break;
            case '\n': fputs("\\n", file); break;
            case '\r': fputs("\\r", file); break;
            case '\t': fputs("\\t", file); break;
            default:
                if (*cursor < 0x20) fprintf(file, "\\u%04x", (unsigned int)*cursor);
                else fputc((int)*cursor, file);
                break;
        }
        cursor++;
    }
    fputc('"', file);
}

static void write_candidate_fields(FILE *file,
                                   const char *path,
                                   file_type type,
                                   RxcpImportRootKind root_kind,
                                   size_t root_index,
                                   const char *digest,
                                   const char *digest_status,
                                   time_t mtime,
                                   const char *indent) {
    fprintf(file, "%s\"path\": ", indent);
    write_json_string(file, path);
    fprintf(file, ",\n%s\"kind\": ", indent);
    write_json_string(file, artifact_kind_name(type));
    fprintf(file, ",\n%s\"root_kind\": ", indent);
    write_json_string(file, root_kind_name(root_kind));
    fprintf(file, ",\n%s\"root_index\": %lu,\n%s\"sha256\": ",
            indent, (unsigned long)root_index, indent);
    if (digest && digest[0]) write_json_string(file, digest);
    else fputs("null", file);
    fprintf(file, ",\n%s\"digest_status\": ", indent);
    write_json_string(file, digest_status);
    fprintf(file, ",\n%s\"mtime_seconds\": %lld\n", indent, (long long)mtime);
}

static void write_logical_roots(FILE *file, Context *context, int source_roots) {
    size_t index;
    int first;
    char *logical;
    char **roots;

    first = 1;
    roots = source_roots ? context->source_import_locations : context->import_locations;
    fputc('[', file);
    if (source_roots) {
        logical = logical_root_name(RXCP_IMPORT_ROOT_PRIMARY_SOURCE, 0);
        write_json_string(file, logical);
        free(logical);
        first = 0;
    }
    if (roots) {
        for (index = 0; roots[index]; index++) {
            RxcpImportRootKind kind;
            size_t logical_index;

            kind = source_roots ? RXCP_IMPORT_ROOT_SOURCE : RXCP_IMPORT_ROOT_BINARY;
            logical_index = index;
            if (!source_roots && context->executable_import_included &&
                index == context->executable_import_root_index) {
                kind = RXCP_IMPORT_ROOT_EXECUTABLE;
                logical_index = 0;
            }
            if (!first) fputs(", ", file);
            logical = logical_root_name(kind, logical_index);
            write_json_string(file, logical);
            free(logical);
            first = 0;
        }
    }
    fputc(']', file);
}

static void write_event(FILE *file, const RxcpImportEvent *event, int trailing_comma) {
    fprintf(file, "    {\n      \"sequence\": %lu,\n      \"stage\": ",
            (unsigned long)event->sequence);
    if (event->type == NATIVE_FILE) write_json_string(file, "native");
    else if (event->root_kind == RXCP_IMPORT_ROOT_PRIMARY_SOURCE ||
             event->root_kind == RXCP_IMPORT_ROOT_SOURCE) write_json_string(file, "source");
    else write_json_string(file, "binary");
    fputs(",\n      \"module\": ", file);
    write_json_string(file, event->module);
    fputs(",\n      \"candidate\": {\n", file);
    write_candidate_fields(file, event->path, event->type, event->root_kind,
                           event->root_index, event->digest, event->digest_status,
                           event->mtime, "        ");
    fputs("      },\n      \"decision\": ", file);
    write_json_string(file, event->decision);
    fputs(",\n      \"reason\": ", file);
    write_json_string(file, event->reason);
    if (event->replaced_path) {
        fputs(",\n      \"replaced_path\": ", file);
        write_json_string(file, event->replaced_path);
    }
    fprintf(file, "\n    }%s\n", trailing_comma ? "," : "");
}

static void write_selected_candidate(FILE *file, const RxcpImportReport *report,
                                     const importable_file *candidate, int trailing_comma) {
    char *path;
    char *module;
    char digest[RX_SHA256_DIGEST_SIZE * 2 + 1];
    const char *digest_status;
    const RxcpImportEvent *captured;

    path = logical_candidate_path(candidate);
    module = module_name_copy(candidate->name);
    captured = find_captured_candidate(report, path, candidate->type,
                                       candidate->root_kind, candidate->root_index,
                                       candidate->mtime);
    if (captured) {
        memcpy(digest, captured->digest, sizeof(digest));
        digest_status = captured->digest_status;
    } else {
        capture_digest(candidate, digest, &digest_status);
    }
    fputs("    {\n      \"stage\": ", file);
    write_json_string(file, stage_name(candidate));
    fputs(",\n      \"module\": ", file);
    write_json_string(file, module);
    fputs(",\n      \"candidate\": {\n", file);
    write_candidate_fields(file, path, candidate->type, candidate->root_kind,
                           candidate->root_index, digest, digest_status,
                           candidate->mtime, "        ");
    fputs("      }\n", file);
    fprintf(file, "    }%s\n", trailing_comma ? "," : "");
    free(module);
    free(path);
}

int rxcp_import_report_write(Context *context) {
    Context *owner;
    RxcpImportReport *report;
    FILE *file;
    char *temporary_path;
    char *primary_path;
    size_t path_size;
    size_t index;
    size_t selected_count;
    int failed;

    if (!context) return 0;
    owner = context->master_context ? context->master_context : context;
    if (!owner->import_resolution_report_path) return 0;
    report = report_for(owner, 1);

    path_size = strlen(owner->import_resolution_report_path) + 5;
    temporary_path = (char *)malloc(path_size);
    if (!temporary_path) {
        RX_REPORT_OOM("malloc import resolution temporary path", path_size,
                      owner->import_resolution_report_path);
        return -1;
    }
    snprintf(temporary_path, path_size, "%s.tmp", owner->import_resolution_report_path);
    file = fopen(temporary_path, "wb");
    if (!file) {
        free(temporary_path);
        return -1;
    }

    primary_path = (char *)malloc(strlen(owner->file_name ? owner->file_name : "unknown") + 20);
    if (!primary_path) {
        fclose(file);
        remove(temporary_path);
        free(temporary_path);
        RX_REPORT_OOM("malloc import report primary path", RX_OOM_UNKNOWN_SIZE, owner->file_name);
        return -1;
    }
    sprintf(primary_path, "@primary-source/0/%s", owner->file_name ? owner->file_name : "unknown");

    fputs("{\n  \"schema_version\": \"crexx.import-resolution-report/v1\",\n", file);
    fputs("  \"status\": \"observed\",\n  \"tool\": {\n    \"name\": \"rxc\",\n    \"version\": ", file);
    write_json_string(file, rxversion);
    fputs("\n  },\n  \"primary_source\": ", file);
    write_json_string(file, primary_path);
    fputs(",\n  \"options\": {\n    \"source_roots\": ", file);
    write_logical_roots(file, owner, 1);
    fputs(",\n    \"binary_roots\": ", file);
    write_logical_roots(file, owner, 0);
    fprintf(file, ",\n    \"auto_import_rxas\": %s,\n    \"executable_directory\": \"%s\"\n  },\n",
            owner->auto_import_rxas ? "true" : "false",
            owner->executable_import_included ? "included" : "excluded");

    fputs("  \"candidate_events\": [\n", file);
    for (index = 0; index < report->event_count; index++) {
        write_event(file, &report->events[index], index + 1 < report->event_count);
    }
    fputs("  ],\n  \"selected_candidates\": [\n", file);
    selected_count = 0;
    if (owner->importable_file_list) {
        while (owner->importable_file_list[selected_count]) selected_count++;
        for (index = 0; index < selected_count; index++) {
            write_selected_candidate(file, report, owner->importable_file_list[index],
                                     index + 1 < selected_count);
        }
    }
    fputs("  ],\n  \"provider_bindings\": []\n}\n", file);

    free(primary_path);
    failed = ferror(file);
    if (fclose(file) != 0) failed = 1;
    if (!failed && rxcp_import_report_publish(owner->import_resolution_report_path,
                                              temporary_path) != 0) failed = 1;
    if (failed) remove(temporary_path);
    free(temporary_path);
    return failed ? -1 : 0;
}

void rxcp_import_report_free(Context *context) {
    RxcpImportReport *report;
    size_t index;

    if (!context) return;
    if (context->master_context && context != context->master_context) return;
    report = context->import_resolution_report;
    if (!report) return;
    for (index = 0; index < report->event_count; index++) {
        free(report->events[index].module);
        free(report->events[index].path);
        free(report->events[index].decision);
        free(report->events[index].reason);
        free(report->events[index].replaced_path);
    }
    free(report->events);
    free(report);
    context->import_resolution_report = 0;
}

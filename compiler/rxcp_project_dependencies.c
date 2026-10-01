/*
 * cREXX License (MIT)
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxcpmain.h"
#include "rxcp_import_publish.h"
#include "rxsha256.h"

/* Private, versioned driver protocol. The file contains the magic, a binary
 * SHA-256 digest, and one 'F' (full content) or 'H' (namespace header) byte per
 * ordered selected candidate. No paths are serialized or parsed. Re-running
 * rxfl_lst retains all precedence, extension and RXAS mtime rules, including
 * candidate additions/removals and namespace changes in excluded sources.
 * The controller additionally owns the toolchain and complete action key.
 */
static const char dependency_magic[] = "crexx.project-dependencies/v1\n";

static int hash_text(rx_sha256_context *state, const char *text) {
    return !rx_sha256_update(state, text ? text : "", text ? strlen(text) + 1 : 1);
}

static int hash_file(rx_sha256_context *state, const char *location, const char *name) {
    unsigned char digest[RX_SHA256_DIGEST_SIZE];
    char *path;
    int failed;
    path = mprintf("%s%s%s", location ? location : "",
                   location && location[0] ? "/" : "", name);
    if (!path) return 1;
    failed = rx_sha256_file(path, digest) != RX_SHA256_FILE_OK;
    free(path);
    return failed || !rx_sha256_update(state, digest, sizeof(digest));
}

static int hash_roots(rx_sha256_context *state, char **roots) {
    size_t i;
    if (roots) {
        for (i = 0; roots[i]; i++) if (hash_text(state, roots[i])) return 1;
    }
    return hash_text(state, "");
}

int rxcp_project_dependencies(Context *context, const char *path, int check) {
    rx_sha256_context state;
    unsigned char digest[RX_SHA256_DIGEST_SIZE], expected[RX_SHA256_DIGEST_SIZE];
    char magic[sizeof(dependency_magic) - 1];
    char options[96];
    char *modes = 0, *temporary = 0;
    importable_file **candidates;
    importable_file *candidate;
    FILE *file = 0;
    size_t count = 0, i;
    int result = 1, mode;
    const char *environment;
    static const char *environment_names[] = {
        "RXCP_DISABLE_EXIT", "RXCP_EXIT_MODULE", "CREXX_DIAGNOSTICS",
        "CREXX_DIAGNOSTIC_LOCALE", "CREXX_MESSAGE_PATH", "LC_ALL",
        "LC_MESSAGES", "LANG", 0
    };

    if (!context->importable_file_list) context->importable_file_list = rxfl_lst(context);
    candidates = context->importable_file_list;
    if (!candidates) return 1;
    while (candidates[count]) count++;
    modes = (char *)malloc(count ? count : 1);
    if (!modes) RX_PANIC_OOM("malloc project dependency modes", count, path);
    if (check) {
        file = fopen(path, "rb");
        if (!file || fread(magic, 1, sizeof(magic), file) != sizeof(magic) ||
            memcmp(magic, dependency_magic, sizeof(magic)) != 0 ||
            fread(expected, 1, sizeof(expected), file) != sizeof(expected) ||
            fread(modes, 1, count, file) != count || fgetc(file) != EOF || ferror(file)) goto done;
    }
    rx_sha256_init(&state);
    for (i = 0; environment_names[i]; i++) {
        environment = getenv(environment_names[i]);
        if (hash_text(&state, environment_names[i]) ||
            hash_text(&state, environment ? "present" : "absent") ||
            hash_text(&state, environment)) goto done;
    }
    if (hash_text(&state, dependency_magic)) goto done;
    snprintf(options, sizeof(options), "%d/%d/%d/%d/%d",
             context->optimise, context->auto_import_rxas, context->disable_exits,
             (int)context->cli_level_override, context->emit_autoload_hints);
    if (hash_text(&state, options) || hash_text(&state, context->initial_source_extension) ||
        hash_text(&state, context->location) || hash_text(&state, context->file_name) ||
        hash_file(&state, context->location, context->file_name) ||
        hash_roots(&state, context->source_import_locations) ||
        hash_roots(&state, context->import_locations)) goto done;
    for (i = 0; i < context->cli_import_count; i++) {
        if (hash_text(&state, context->cli_import_names[i])) goto done;
    }
    if (hash_text(&state, "")) goto done;
    for (i = 0; i < count; i++) {
        candidate = candidates[i];
        mode = check ? modes[i] :
               (candidate->type != REXX_FILE || candidate->imported ? 'F' : 'H');
        if ((mode != 'F' && mode != 'H') ||
            (mode == 'H' && candidate->type != REXX_FILE)) goto done;
        modes[i] = (char)mode;
        if (hash_text(&state, candidate->location) || hash_text(&state, candidate->name) ||
            !rx_sha256_update(&state, &modes[i], 1)) goto done;
        if (mode == 'H') {
            if (hash_text(&state, rxcp_importable_source_namespace(context, candidate))) goto done;
        } else {
            if (hash_file(&state, candidate->location, candidate->name)) goto done;
        }
    }
    rx_sha256_final(&state, digest);
    result = 1;
    if (check) {
        result = memcmp(digest, expected, sizeof(digest)) != 0;
        goto done;
    }
    temporary = mprintf("%s.tmp", path);
    if (!temporary) goto done;
    file = fopen(temporary, "wb");
    if (!file) goto done;
    if (fwrite(dependency_magic, 1, sizeof(magic), file) != sizeof(magic) ||
        fwrite(digest, 1, sizeof(digest), file) != sizeof(digest) ||
        fwrite(modes, 1, count, file) != count || ferror(file)) goto done;
    result = fclose(file) != 0;
    file = 0;
    if (!result) result = rxcp_import_report_publish(path, temporary) != 0;
done:
    if (file && fclose(file) != 0) result = 1;
    if (temporary) {
        if (result) remove(temporary);
        free(temporary);
    }
    free(modes);
    return result;
}

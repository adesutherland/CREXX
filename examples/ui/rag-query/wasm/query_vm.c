/* One browser Worker owns a RAG query VM; RAG owns the linked retrieval code. */
#include <rxvml.h>
#include <rxbin.h>
#include <emscripten.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct Image { void *bytes; struct Image *next; } Image;
static rxvml_context *context;
static Image *images;
static size_t image_bytes;
static int running, program_rc;
static char error_text[1024];

extern void crexx_fs_init(void);
static int fail(const char *message);

/* Prime FTS5 on its own connection before the VM enters the read-only query.
 * This avoids a cold fts5vocab/data_version re-entry in the Wasm VM path. */
EMSCRIPTEN_KEEPALIVE int rag_vm_prepare_fts(void) {
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_open_v2("/library/library.sqlite", &db,
                             SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, NULL);
    if (rc == SQLITE_OK) rc = sqlite3_prepare_v2(db,
        "SELECT count(*) FROM chunks_fts WHERE chunks_fts MATCH 'heart'", -1, &stmt, NULL);
    if (rc == SQLITE_OK) rc = sqlite3_step(stmt);
    if (stmt) sqlite3_finalize(stmt);
    if (db) sqlite3_close_v2(db);
    return rc == SQLITE_ROW ? 0 : fail("Cannot prepare corpus FTS index");
}

static int fail(const char *message) {
    snprintf(error_text, sizeof(error_text), "%s", message ? message : "RAG VM failure");
    return -1;
}

EMSCRIPTEN_KEEPALIVE const char *rag_vm_error(void) { return error_text; }
EMSCRIPTEN_KEEPALIVE int rag_vm_program_rc(void) { return program_rc; }

EMSCRIPTEN_KEEPALIVE int rag_vm_destroy(void) {
    if (running) return fail("VM is running");
    if (context) rxvml_destroy(context);
    context = NULL;
    while (images) {
        Image *image = images;
        images = image->next;
        free(image->bytes);
        free(image);
    }
    image_bytes = 0;
    return 0;
}

EMSCRIPTEN_KEEPALIVE int rag_vm_create(void) {
    if (running) return fail("VM is running");
    rag_vm_destroy();
    error_text[0] = 0;
    program_rc = 0;
    crexx_fs_init();
    context = rxvml_create(NULL, 0);
    return context ? 0 : fail("Cannot create RAG VM");
}

EMSCRIPTEN_KEEPALIVE int rag_vm_load(const void *bytes, size_t length) {
    if (!context || running) return fail("VM cannot load now");
    if (!bytes || !length || length > 16u * 1024u * 1024u - image_bytes)
        return fail("RXBIN image budget exceeded");
    Image *image = calloc(1, sizeof(*image));
    if (!image) return fail("Cannot allocate image record");
    image->bytes = malloc(length);
    if (!image->bytes) { free(image); return fail("Cannot allocate image"); }
    memcpy(image->bytes, bytes, length);
    image->next = images;
    images = image;
    image_bytes += length;
    if (rxvml_load_module_buffer(context, image->bytes, length) <= 0)
        return fail(rxbin_last_error());
    return 0;
}

EMSCRIPTEN_KEEPALIVE int rag_vm_run(const char *request_json) {
    const char *args[3];
    const char *message = NULL;
    size_t length;
    int rc;
    if (!context || running || !request_json) return fail("Invalid RAG query start");
    length = 0;
    while (length <= 8192 && request_json[length]) length++;
    if (!length || length > 8192 || strchr(request_json, '\n') || strchr(request_json, '\r'))
        return fail("Invalid RAG request");
    args[0] = "/library";
    args[1] = "/profile.tsv";
    args[2] = request_json;
    error_text[0] = 0;
    program_rc = 0;
    running = 1;
    rc = rxvml_run(context, 3, args, &program_rc);
    running = 0;
    if (rc) {
        rxvml_last_error(context, &message);
        fail(message ? message : "RAG VM run failed");
    }
    if (fflush(stdout) || fflush(stderr)) return fail("RAG stream flush failed");
    return rc;
}

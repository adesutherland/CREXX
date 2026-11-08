/* Single-owner RXBIN process image for the browser UI Worker. */
#include <rxvml.h>
#include <rxbin.h>
#include <emscripten.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Image { void *bytes; struct Image *next; } Image;
static rxvml_context *context;
static Image *images;
static size_t image_bytes;
static int running, program_rc;
static char error_text[1024];

static int fail(const char *message) {
    snprintf(error_text, sizeof(error_text), "%s", message ? message : "UI VM failure");
    return -1;
}
EMSCRIPTEN_KEEPALIVE const char *ui_vm_error(void) { return error_text; }
EMSCRIPTEN_KEEPALIVE int ui_vm_program_rc(void) { return program_rc; }

EMSCRIPTEN_KEEPALIVE int ui_vm_destroy(void) {
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

EMSCRIPTEN_KEEPALIVE int ui_vm_create(void) {
    if (running) return fail("VM is running");
    ui_vm_destroy();
    error_text[0] = 0;
    program_rc = 0;
    context = rxvml_create(NULL, 0);
    return context ? 0 : fail("Cannot create UI VM");
}

EMSCRIPTEN_KEEPALIVE int ui_vm_load(const void *bytes, size_t length) {
    if (!context || running) return fail("VM cannot load now");
    if (!bytes || !length || length > 64u * 1024u * 1024u - image_bytes)
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

EMSCRIPTEN_KEEPALIVE int ui_vm_run(const char *session_id) {
    const char *args[1];
    const char *message = NULL;
    int rc;
    if (!context || running || !session_id || strlen(session_id) > 64)
        return fail("Invalid UI session start");
    args[0] = session_id;
    error_text[0] = 0;
    program_rc = 0;
    running = 1;
    rc = rxvml_run(context, 1, args, &program_rc);
    running = 0;
    if (rc) {
        rxvml_last_error(context, &message);
        fail(message ? message : "UI VM run failed");
    }
    if (fflush(stdout) || fflush(stderr)) return fail("UI stream flush failed");
    return rc;
}

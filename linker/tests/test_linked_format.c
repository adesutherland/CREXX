#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rxbin.h"

static int skip_record_payload(FILE *fp, const module_header *header) {
    size_t skip = header->name_size + header->description_size +
                  header->instruction_stored_size + header->constant_stored_size;
    if (!skip) return 1;
    return fseek(fp, (long)skip, SEEK_CUR) == 0;
}

static void free_loaded_modules(module_file **modules, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        if (modules[i]) free_module(modules[i]);
    }
}

static int module_has_source_metadata(const module_file *module) {
    int meta = module->header.meta_head;

    while (meta != -1) {
        const meta_entry *entry = (const meta_entry *)(module->constant + meta);
        if (entry->base.type == META_SOURCE_STEP) return 1;
        meta = entry->next;
    }

    return 0;
}

static int module_has_trace_event_metadata(const module_file *module) {
    int meta = module->header.meta_head;

    while (meta != -1) {
        const meta_entry *entry = (const meta_entry *)(module->constant + meta);
        if (entry->base.type == META_TRACE_EVENT) return 1;
        meta = entry->next;
    }

    return 0;
}

static int module_has_inline_metadata(const module_file *module) {
    int meta = module->header.meta_head;

    while (meta != -1) {
        const meta_entry *entry = (const meta_entry *)(module->constant + meta);
        if (entry->base.type == META_INLINE) return 1;
        meta = entry->next;
    }

    return 0;
}

static int check_linked_success_format(void) {
    FILE *fp;
    module_header header;
    module_header second;
    module_header third;
    rxbin_reader reader;
    module_file *module_a = 0;
    module_file *module_b = 0;
    int rc = 1;
    expose_proc_constant *imported_expose;
    proc_constant *imported_proc;

    fp = fopen("tests_linked_success.rxbin", "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open tests_linked_success.rxbin\n");
        return 0;
    }

    if (fread(&header, sizeof(header), 1, fp) != 1) {
        fprintf(stderr, "Failed to read first linked record header\n");
        goto done;
    }
    if (header.record_type != RXBIN_RECORD_POOL_SHARED || header.instruction_size != 0 || header.constant_size == 0) {
        fprintf(stderr, "First linked record is not a shared pool\n");
        goto done;
    }
    if (!skip_record_payload(fp, &header)) {
        fprintf(stderr, "Failed to skip shared pool payload\n");
        goto done;
    }

    if (fread(&second, sizeof(second), 1, fp) != 1 || second.record_type != RXBIN_RECORD_MODULE_SHARED) {
        fprintf(stderr, "Second linked record is not a shared-backed module\n");
        goto done;
    }
    if (!skip_record_payload(fp, &second)) {
        fprintf(stderr, "Failed to skip first module payload\n");
        goto done;
    }

    if (fread(&third, sizeof(third), 1, fp) != 1 || third.record_type != RXBIN_RECORD_MODULE_SHARED) {
        fprintf(stderr, "Third linked record is not a shared-backed module\n");
        goto done;
    }

    rewind(fp);
    rxbin_reader_init_file(&reader, fp);
    if (rxbin_reader_next_module(&reader, &module_a) != 0 ||
        rxbin_reader_next_module(&reader, &module_b) != 0) {
        fprintf(stderr, "Failed to decode linked modules through rxbin_reader\n");
        rxbin_reader_close(&reader);
        goto done;
    }
    rxbin_reader_close(&reader);

    if (!module_a->shared_constant_pool || !module_b->shared_constant_pool) {
        fprintf(stderr, "Linked modules are not backed by a shared pool object\n");
        goto done;
    }

    if (module_a->shared_constant_pool != module_b->shared_constant_pool ||
        module_a->constant != module_b->constant) {
        fprintf(stderr, "Linked modules do not share the same pool in memory\n");
        goto done;
    }

    imported_expose = (expose_proc_constant *)(module_a->constant + module_a->header.expose_head);
    if (imported_expose->base.type != EXPOSE_PROC_CONST || !imported_expose->imported) {
        fprintf(stderr, "Linked root module lost its imported expose entry\n");
        goto done;
    }
    imported_proc = (proc_constant *)(module_a->constant + imported_expose->procedure);
    if (imported_proc->base.type != PROC_CONST || imported_proc->next != -1) {
        fprintf(stderr, "Linked imported procedure stub has an invalid next pointer\n");
        goto done;
    }
    if (module_has_inline_metadata(module_a) || module_has_inline_metadata(module_b)) {
        fprintf(stderr, "Default linked image still contains inline metadata\n");
        goto done;
    }
    if (!module_has_trace_event_metadata(module_a)) {
        fprintf(stderr, "Default linked image lost trace-event metadata\n");
        goto done;
    }

    rc = 0;

done:
    if (fp) fclose(fp);
    free_module(module_a);
    free_module(module_b);
    return rc == 0;
}

static int check_inline_preserve_format(void) {
    FILE *fp;
    rxbin_reader reader;
    module_file *module_a = 0;
    module_file *module_b = 0;
    int ok = 0;

    fp = fopen("tests_linked_inline.rxbin", "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open tests_linked_inline.rxbin\n");
        return 0;
    }

    rxbin_reader_init_file(&reader, fp);
    if (rxbin_reader_next_module(&reader, &module_a) != 0 ||
        rxbin_reader_next_module(&reader, &module_b) != 0) {
        fprintf(stderr, "Failed to decode inline-preserving linked modules through rxbin_reader\n");
        goto done;
    }

    if (!module_has_inline_metadata(module_a) && !module_has_inline_metadata(module_b)) {
        fprintf(stderr, "Inline-preserving linked image lost inline metadata\n");
        goto done;
    }

    ok = 1;

done:
    rxbin_reader_close(&reader);
    fclose(fp);
    free_module(module_a);
    free_module(module_b);
    return ok;
}

static int check_stripped_format(void) {
    FILE *fp;
    rxbin_reader reader;
    module_file *module_a = 0;
    module_file *module_b = 0;
    int ok = 0;

    fp = fopen("tests_linked_stripped.rxbin", "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open tests_linked_stripped.rxbin\n");
        return 0;
    }

    rxbin_reader_init_file(&reader, fp);
    if (rxbin_reader_next_module(&reader, &module_a) != 0 ||
        rxbin_reader_next_module(&reader, &module_b) != 0) {
        fprintf(stderr, "Failed to decode stripped linked modules through rxbin_reader\n");
        goto done;
    }

    if (module_has_source_metadata(module_a) || module_has_source_metadata(module_b)) {
        fprintf(stderr, "Stripped linked image still contains source/file metadata\n");
        goto done;
    }
    if (!module_has_trace_event_metadata(module_a)) {
        fprintf(stderr, "Stripped linked image lost trace-event metadata\n");
        goto done;
    }

    ok = 1;

done:
    rxbin_reader_close(&reader);
    fclose(fp);
    free_module(module_a);
    free_module(module_b);
    return ok;
}

static int check_record_stream_concatenation(void) {
    FILE *fp;
    rxbin_reader reader;
    module_file *modules[8];
    size_t count = 0;
    int read_rc;
    int ok = 0;

    memset(modules, 0, sizeof(modules));

    fp = fopen("tests_record_stream_combo.rxbin", "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open tests_record_stream_combo.rxbin\n");
        return 0;
    }

    rxbin_reader_init_file(&reader, fp);
    do {
        read_rc = rxbin_reader_next_module(&reader, &modules[count]);
        if (read_rc == 0) {
            count++;
            if (count > sizeof(modules) / sizeof(modules[0])) {
                fprintf(stderr, "Too many modules decoded from concatenated record stream\n");
                goto done;
            }
        }
    } while (read_rc == 0);

    if (read_rc != 1 || count != 6) {
        fprintf(stderr, "Unexpected module count in concatenated record stream (rc=%d count=%lu)\n",
                read_rc, (unsigned long)count);
        goto done;
    }

    if (modules[0]->shared_constant_pool || modules[3]->shared_constant_pool) {
        fprintf(stderr, "Local records unexpectedly reused a shared pool\n");
        goto done;
    }

    if (!modules[1]->shared_constant_pool || modules[1]->shared_constant_pool != modules[2]->shared_constant_pool) {
        fprintf(stderr, "First linked image did not keep a shared pool across its members\n");
        goto done;
    }

    if (!modules[4]->shared_constant_pool || modules[4]->shared_constant_pool != modules[5]->shared_constant_pool) {
        fprintf(stderr, "Second linked image did not keep a shared pool across its members\n");
        goto done;
    }

    if (modules[1]->shared_constant_pool == modules[4]->shared_constant_pool) {
        fprintf(stderr, "Concatenated linked images incorrectly reused the previous shared pool\n");
        goto done;
    }

    ok = 1;

done:
    rxbin_reader_close(&reader);
    fclose(fp);
    free_loaded_modules(modules, count);
    return ok;
}

int main(void) {
    if (!check_linked_success_format()) return 1;
    if (!check_inline_preserve_format()) return 1;
    if (!check_stripped_format()) return 1;
    if (!check_record_stream_concatenation()) return 1;
    return 0;
}

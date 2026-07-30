/* cREXX License (MIT) */

#include "contract_model.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct StringList {
    const char **items;
    size_t count;
} StringList;

static void usage(FILE *stream) {
    fputs("usage: crexx-contract --rxbin FILE --operation OWNER.MEMBER \\\n\n"
          "       --contract-version MAJOR.MINOR.PATCH --output FILE \\\n\n"
          "       [--nullable TYPE.FIELD] [--optional-field TYPE.FIELD] \\\n\n"
          "       [--error TYPE] [--previous FILE]\n",
          stream);
}

static int append_string(StringList *list, const char *value) {
    const char **grown = (const char **)realloc(list->items,
                                                (list->count + 1u) * sizeof(*grown));
    if (!grown) return 0;
    list->items = grown;
    list->items[list->count++] = value;
    return 1;
}

static int valid_semver(const char *text) {
    int component;
    const unsigned char *cursor = (const unsigned char *)text;
    if (!cursor || !*cursor) return 0;
    for (component = 0; component < 3; component++) {
        if (!isdigit(*cursor)) return 0;
        while (isdigit(*cursor)) cursor++;
        if (component < 2) {
            if (*cursor++ != '.') return 0;
        } else if (*cursor) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    CrexxContractOptions options;
    CrexxContractModel model;
    StringList nullable_fields = {0, 0};
    StringList optional_fields = {0, 0};
    StringList error_types = {0, 0};
    const char *output_path = 0;
    const char *previous_path = 0;
    char *error_message = 0;
    int i;
    int status = 1;

    memset(&options, 0, sizeof(options));
    crexx_contract_model_init(&model);
    for (i = 1; i < argc; i++) {
        const char *option = argv[i];
        const char *value;
        if (strcmp(option, "--help") == 0 || strcmp(option, "-h") == 0) {
            usage(stdout);
            status = 0;
            goto cleanup;
        }
        if (i + 1 >= argc) {
            fprintf(stderr, "crexx-contract: option '%s' requires a value\n", option);
            usage(stderr);
            goto cleanup;
        }
        value = argv[++i];
        if (strcmp(option, "--rxbin") == 0) options.rxbin_path = value;
        else if (strcmp(option, "--operation") == 0) options.operation = value;
        else if (strcmp(option, "--contract-version") == 0) options.contract_version = value;
        else if (strcmp(option, "--output") == 0) output_path = value;
        else if (strcmp(option, "--previous") == 0) previous_path = value;
        else if (strcmp(option, "--nullable") == 0) {
            if (!append_string(&nullable_fields, value)) goto allocation_error;
        } else if (strcmp(option, "--optional-field") == 0) {
            if (!append_string(&optional_fields, value)) goto allocation_error;
        } else if (strcmp(option, "--error") == 0) {
            if (!append_string(&error_types, value)) goto allocation_error;
        } else {
            fprintf(stderr, "crexx-contract: unknown option '%s'\n", option);
            usage(stderr);
            goto cleanup;
        }
    }
    if (!options.rxbin_path || !options.operation || !options.contract_version || !output_path) {
        fputs("crexx-contract: --rxbin, --operation, --contract-version and --output are required\n",
              stderr);
        usage(stderr);
        goto cleanup;
    }
    if (!valid_semver(options.contract_version)) {
        fprintf(stderr,
                "crexx-contract: contract version '%s' must be MAJOR.MINOR.PATCH\n",
                options.contract_version);
        goto cleanup;
    }
    options.nullable_fields = nullable_fields.items;
    options.nullable_count = nullable_fields.count;
    options.optional_fields = optional_fields.items;
    options.optional_count = optional_fields.count;
    options.error_types = error_types.items;
    options.error_count = error_types.count;

    if (!crexx_contract_model_from_rxbin(&options, &model, &error_message) ||
        (previous_path && !crexx_contract_check_previous(&model, previous_path, &error_message)) ||
        !crexx_contract_write_json(&model, output_path, &error_message)) {
        fprintf(stderr,
                "crexx-contract: %s\n",
                error_message ? error_message : "contract generation failed");
        goto cleanup;
    }
    status = 0;
    goto cleanup;

allocation_error:
    fputs("crexx-contract: out of memory parsing command line\n", stderr);

cleanup:
    free(error_message);
    free(nullable_fields.items);
    free(optional_fields.items);
    free(error_types.items);
    crexx_contract_model_free(&model);
    return status;
}

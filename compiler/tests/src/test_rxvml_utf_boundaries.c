#include <stdio.h>
#include <string.h>
#include "rxvml.h"

int main(void) {
    rxvml_value* value = rxvml_value_new(NULL);
    rxvml_address_request request;
    const char invalid_utf8[] = { (char)0xff, 0 };

    if (!value) {
        fprintf(stderr, "failed to allocate rxvml value\n");
        return 1;
    }

    if (rxvml_set_str(value, "valid", 5) != 0) {
        fprintf(stderr, "valid UTF-8 was rejected\n");
        rxvml_value_free(value);
        return 1;
    }

    if (rxvml_set_str(value, invalid_utf8, 1) == 0) {
        fprintf(stderr, "invalid UTF-8 was accepted by rxvml_set_str\n");
        rxvml_value_free(value);
        return 1;
    }

    memset(&request, 0, sizeof(request));
    if (rxvml_address_emit_output(NULL, &request, invalid_utf8) == 0) {
        fprintf(stderr, "invalid UTF-8 was accepted by ADDRESS output emit\n");
        rxvml_value_free(value);
        return 1;
    }

    rxvml_value_free(value);
    return 0;
}

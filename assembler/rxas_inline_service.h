/* cREXX License (MIT). Explicit RXAS entry to the shared Level L service. */
#ifndef CREXX_RXAS_INLINE_SERVICE_H
#define CREXX_RXAS_INLINE_SERVICE_H

#include <stddef.h>
#include "rxas.h"

int rxas_inline_service_run(const Assembler_Context *scanner,
        const char *service, const char *vm, const char *logical,
        const char *policy, char *const *runtime, size_t runtime_count,
        char **private_dir, char **read_path);
void rxas_inline_service_cleanup(char *private_dir, char *read_path);

#endif

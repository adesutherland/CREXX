/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 */

#ifndef CREXX_RXCP_IMPORT_REPORT_H
#define CREXX_RXCP_IMPORT_REPORT_H

#include "rxcp_types.h"

typedef enum RxcpImportRootKind {
    RXCP_IMPORT_ROOT_PRIMARY_SOURCE = 0,
    RXCP_IMPORT_ROOT_SOURCE,
    RXCP_IMPORT_ROOT_BINARY,
    RXCP_IMPORT_ROOT_EXECUTABLE
} RxcpImportRootKind;

typedef struct RxcpImportReport RxcpImportReport;

void rxcp_import_report_record(Context *context,
                               const importable_file *candidate,
                               const char *decision,
                               const char *reason,
                               const importable_file *replaced);
int rxcp_import_report_write(Context *context);
void rxcp_import_report_free(Context *context);

#endif /* CREXX_RXCP_IMPORT_REPORT_H */

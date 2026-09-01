/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 */

#include <stdio.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "rxcp_import_publish.h"

/* Keep the Windows SDK namespace in this small translation unit: several SDK
 * type names collide with existing compiler AST identifiers. */
int rxcp_import_report_publish(const char *path, const char *temporary_path) {
#if defined(_WIN32)
    if (MoveFileExA(temporary_path, path,
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return 0;
    return -1;
#else
    return rename(temporary_path, path);
#endif
}

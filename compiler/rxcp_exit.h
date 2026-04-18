/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CREXX_RXCP_EXIT_H
#define CREXX_RXCP_EXIT_H

#include "rxcp_ctx.h"

typedef struct ExitKeyword {
    char *keyword;
    struct ExitKeyword *next;
} ExitKeyword;

typedef struct ExitImport {
    char *namespace_name;
    char *provenance;
    char *flags;
    struct ExitImport *next;
} ExitImport;

typedef struct ExitEntry {
    char *primary_keyword;
    ExitKeyword *additional_keywords;
    char *class_name; /* the class name of the exit object */
    ExitImport *default_imports;
    unsigned int flags;
    struct ExitEntry *next;
} ExitEntry;

#define RXCP_EXIT_PROTOCOL_VERSION       2u
#define RXCP_EXIT_FLAG_CERTIFIED        0x0001u
#define RXCP_EXIT_FLAG_RESERVED_KEYWORD 0x0002u
#define RXCP_EXIT_FLAG_IMPLICIT_COMMAND 0x0004u

/* Registry for all additional keywords across all exits */
typedef struct ExitAdditionalKeywords {
    char *keyword;
    struct ExitAdditionalKeywords *next;
} ExitAdditionalKeywords;

void rxcp_init_exits(Context *ctx);
void rxcp_free_exits(Context *ctx);
const char *rxcp_match_certified_exit_primary(const char *keyword, size_t len);
int rxcp_is_exit_primary(Context *ctx, const char *keyword, size_t len);
int rxcp_is_exit_additional(Context *ctx, const char *keyword, size_t len);
unsigned int rxcp_get_exit_flags(Context *ctx, const char *keyword, size_t len);
int rxcp_exit_bridge_invoke(Context *ctx, ASTNode *node);
int rxcp_exit_bridge_plan_invoke(Context *ctx, ASTNode *node);

#endif

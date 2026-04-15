#ifdef ENABLE_PARSER_MODE

#ifndef CREXX_RXCP_HIGHLIGHT_CONTROLLER_H
#define CREXX_RXCP_HIGHLIGHT_CONTROLLER_H

#ifndef restrict
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* restrict is a keyword */
#elif defined(__GNUC__) || defined(__clang__)
#define restrict __restrict
#elif defined(_MSC_VER)
#define restrict __restrict
#else
#define restrict
#endif
#endif

#include "dslsyntax_common.h"

typedef struct RXCPHighlightCacheStats {
    unsigned long generation;
    unsigned long invalidation_count;
    unsigned long exit_warm_count;
    unsigned long import_inventory_warm_count;
    size_t cached_import_file_count;
    size_t watched_directory_count;
    size_t watched_file_count;
    int exits_disabled;
} RXCPHighlightCacheStats;

void rxc_highlight_controller_parse(CodeBuffer *cb);
void rxcp_highlight_controller_reset_cache(void);
void rxcp_highlight_controller_get_cache_stats(RXCPHighlightCacheStats *stats);

#endif

#endif

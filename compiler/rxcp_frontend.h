/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#ifndef CREXX_RXCP_FRONTEND_H
#define CREXX_RXCP_FRONTEND_H
#include "rxcp_ctx.h"

/* Per-driver frontend services. The backend has no linked parser/exit host. */
typedef struct RxcpFrontend {
    int (*parse)(Context *context);
    int (*parser_mode)(int stdio_mode, int port, const char *file, int debug);
    int (*source_import)(Context *context, const char *file, const char *location,
                         RexxLevel default_level);
    void (*init_exits)(Context *context);
    void (*free_exits)(Context *context);
    int (*exit_primary)(Context *, const char *, size_t);
    int (*exit_additional)(Context *, const char *, size_t);
    unsigned int (*exit_flags)(Context *, const char *, size_t);
    int (*exit_invoke)(Context *, ASTNode *);
    int (*exit_plan)(Context *, ASTNode *);
    void (*exit_diagnostics)(Context *);
    void (*concurrency_imports)(Context *);
    walker_result (*task_target)(walker_direction, ASTNode *, void *);
    void (*task_scopes)(Context *);
    walker_result (*task_calls)(walker_direction, ASTNode *, void *);
} RxcpFrontend;

const RxcpFrontend *rxcp_frontend(Context *context);
int rxcp_frontend_parse(Context *context);
char **rxcp_split_location_list(const char *locations);
int rxcp_driver_main(int argc, char **argv, const RxcpFrontend *frontend);
void rxcp_use_full_frontend(Context *context);
int rxcp_parse_classic_frontend(Context *context);
extern const RxcpFrontend rxcp_full_frontend;
extern const RxcpFrontend rxcp_classic_frontend;
#endif

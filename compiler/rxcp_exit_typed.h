/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#ifndef CREXX_RXCP_EXIT_TYPED_H
#define CREXX_RXCP_EXIT_TYPED_H
#include "rxcp_exit.h"
void typed_rxcp_init_exits(Context *);
void typed_rxcp_free_exits(Context *);
int typed_rxcp_is_exit_primary(Context *, const char *, size_t);
int typed_rxcp_is_exit_additional(Context *, const char *, size_t);
unsigned int typed_rxcp_get_exit_flags(Context *, const char *, size_t);
int typed_rxcp_exit_bridge_invoke(Context *, ASTNode *);
int typed_rxcp_exit_bridge_plan_invoke(Context *, ASTNode *);
void typed_rxcp_exit_rewrite_diagnostics(Context *);
#endif

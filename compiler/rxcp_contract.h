/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#ifndef CREXX_RXCP_CONTRACT_H
#define CREXX_RXCP_CONTRACT_H
#include "rxcp_ctx.h"

/* Metadata fields are decoded directly into declarations, never source code. */
Context *rxcp_contract_context(Context *parent, const char *file, const char *ns);
ASTNode *rxcp_contract_type(Context *context, const char *type);
ASTNode *rxcp_contract_class(Context *context, NodeType kind, const char *ns, const char *name);
void rxcp_contract_source_anchor(ASTNode *node, ASTNode *source);
void rxcp_contract_copy_anchor(ASTNode *node, const ASTNode *source);
ASTNode *rxcp_contract_callable(Context *context, NodeType kind, const char *name,
                              const char *type, const char *args, int task);
ASTNode *rxcp_contract_attribute(Context *context, const char *name,
                               const char *type, int reg, const char *view);
void rxcp_contract_finish(Context *context);
#endif

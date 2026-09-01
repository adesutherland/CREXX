/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXCP_TASK_LOWER_H
#define CREXX_RXCP_TASK_LOWER_H

#include "rxcp_ast.h"

walker_result rxcp_task_calls_walker(walker_direction direction,
                                     ASTNode *node,
                                     void *payload);

void rxcp_validate_task_scope_reuse(Context *context);

#endif

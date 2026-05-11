/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

/**
 * Level C source-tree preparation for DSLSH tracer support.
 */

#include "rxcpmain.h"
#include "rxcp_source_tree.h"

walker_result source_location_walker(walker_direction direction,
                                     ASTNode* node,
                                     void *payload);

void rxcp_levelc_prepare_source_ast(Context *context) {
    if (!context || !context->ast || context->source_tree) return;

    ast_wlkr(context->ast, source_location_walker, (void *)context);
    source_tree_build(context, context->ast);
    rxcp_levelc_validate_control_diagnostics(context);
}

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dslsyntax_common.h"
#include "serialization.h"

int main() {
    printf("Starting flatten test...\n");
    
    CB_ParseTree *tb = cb_create_token_buffer();
    if (!tb) {
        printf("Failed to create token buffer\n");
        return 1;
    }
    
    // Create root node
    CB_Node root_node = cb_create_node(PARSE_TREE_FILE, 0, 100);
    cb_add_child_node(tb, root_node);
    cb_set_current_parent_to_root_node(tb);
    
    if (tb->root == NULL) {
        printf("ERROR: tb->root is NULL after adding root_node\n");
    } else {
        printf("SUCCESS: tb->root is NOT NULL\n");
    }
    
    // Add a child
    CB_Node child_node = cb_create_node(LEXER_KEYWORD, 0, 5);
    cb_add_child_node(tb, child_node);
    
    // Test flatten
    CB_TokenStream *stream = cb_flatten_tree(tb);
    if (!stream) {
        printf("ERROR: cb_flatten_tree returned NULL\n");
    } else {
        printf("SUCCESS: cb_flatten_tree returned stream with %zu tokens\n", stream->count);
        cb_free_token_stream(stream);
    }
    
    cb_free_token_buffer(tb);
    printf("Test complete.\n");
    return 0;
}

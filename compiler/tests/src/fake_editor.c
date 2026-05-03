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
#include "dslsyntax_log.h"

void print_tree(CB_Node *node, int depth) {
    int i;
    CB_Node *child;

    if (!node) return;
    for (i = 0; i < depth; i++) printf("  ");
    printf("Node: type=%d (%s), pos=%zu, len=%zu, sev=%c", 
           (int)node->type, cb_token_type_to_string(node->type), node->pos, node->length, (char)node->severity);
    if (node->message && strlen(node->message) > 0) {
        printf(", msg='%s'", node->message);
    }
    printf("\n");
    
    child = node->child;
    while (child) {
        print_tree(child, depth + 1);
        child = child->sibling;
    }
}

int main(int argc, char *argv[]) {
    char *rxc_path;
    char *filename;
    char cmd[1024];
    CommunicationFunctions *comm;
    FILE *f;
    char line_buf[1024];
    InitialLoad load;
    CB_ParseTree *tb;

    if (argc < 3) {
        printf("Usage: %s <path_to_rxc> <filename.rexx>\n", argv[0]);
        return 1;
    }
    rxc_path = argv[1];
    filename = argv[2];

    cb_log_init(NULL); // Log to stderr

    snprintf(cmd, sizeof(cmd), "%s -d --parser", rxc_path);

    printf("Starting parser with command: %s\n", cmd);
    comm = create_stdio_communication_functions(cmd);

    /* Read file */
    f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return 1;
    }
    
    load.unique_document_id = strdup(filename);
    load.change_version = 0;
    load.line_count = 0;
    load.lines = NULL;

    while (fgets(line_buf, sizeof(line_buf), f)) {
        char *nl = strchr(line_buf, '\n');
        if (nl) *nl = '\0';
        load.lines = realloc(load.lines, (load.line_count + 1) * sizeof(CodeBufferLine));
        utf8_to_line(line_buf, &load.lines[load.line_count++]);
    }
    fclose(f);

    printf("Sending Initial Load (%zu lines)...\n", load.line_count);
    tb = comm->send_initial_load(comm, &load);

    if (!tb) {
        printf("Failed to get parse tree from parser.\n");
        return 1;
    }

    printf("Parse Tree received:\n");
    print_tree(tb->root, 0);

    /* Cleanup */
    cb_free_token_buffer(tb);
    
    return 0;
}

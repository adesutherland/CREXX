// REXX Assembler
// String Tree Library as a Header

#ifndef _CREXX_STRTREE
#define _CREXX_STRTREE

#include <string.h>
#include <stdlib.h>
#include "avl_tree.h"

/* Internal Tree node structure */
struct string_wrapper {
    char* index;
    size_t value;
    struct avl_tree_node index_node;
};

#define GET_INDEX(i) avl_tree_entry((i), struct string_wrapper, index_node)->index

#define GET_VALUE(i) avl_tree_entry((i), struct string_wrapper, index_node)->value

static int compare_node_node(const struct avl_tree_node *node1,
			    const struct avl_tree_node *node2)
{
    char* n1 = GET_INDEX(node1);
    char* n2 = GET_INDEX(node2);
    return strcmp(n1,n2);
}

static int compare_node_value(const void *value,
                        const struct avl_tree_node *nodeptr) {
    char* node = GET_INDEX(nodeptr);
    return strcmp((char*)value,node);
}

/* Adds a string */
/* Returns 0 on success, 1 on duplicate */
static int add_node(struct avl_tree_node **root, char *index, size_t value) {

    struct string_wrapper *i = malloc(sizeof(struct string_wrapper));
    i->index = index;
    i->value = value;
    if (avl_tree_insert(root, &i->index_node, compare_node_node)) {
        /* Duplicate */
        free(i);
        return 1;
    }
    return 0;
}

// Search for an instruction
// Returns 1 if found and sets value
// Returns 0 if not found
static int src_node(struct avl_tree_node *root, char* index, size_t *value) {
    struct avl_tree_node *result;

    result = avl_tree_lookup(root, index, compare_node_value);

    if (result) {
        *value = GET_VALUE(result);
        return 1;
    }
    else return 0;
}

/* Free Tree */
static void free_tree(struct avl_tree_node **root) {
    struct string_wrapper *i;

    /* This walks the tree in post order which allows each node be freed */
    avl_tree_for_each_in_postorder(i, *root, struct string_wrapper, index_node) {
        free(i);
    }
    *root = 0;
}

#endif
#ifndef TREEMAP_H
#define TREEMAP_H

typedef enum { RED, BLACK } Color;

typedef struct TreeNode {
    char *key;
    char *value;
    Color color;
    struct TreeNode *left, *right, *parent;
} TreeNode;

typedef struct TreeMap {
    TreeNode *root;
} TreeMap;

typedef struct TreeMapIterator {
    TreeNode **stack;
    int top;
    int capacity;
} TreeMapIterator;

typedef struct TreeMapRegistry {
    long long map;
    unsigned long long name;                  // uint64_t hash for a named tree map
    int entries;                              // entries in the map
    struct TreeMapRegistry *next;
} TreeMapRegistry;

static TreeMapRegistry *registry_head = NULL;

// TreeMap API
TreeMap *TreeMap_create();
int TreeMap_remove(TreeMap *map, const char *key);
void TreeMap_destroy(TreeMap *map);
// void TreeMap_keys(TreeMap *map, void (*callback)(const char *key, void *ctx), void *ctx); // directly coded for CREXX
const char *TreeMap_firstKey(TreeMap *map);
const char *TreeMap_lastKey(TreeMap *map);
int TreeMap_containsKey(TreeMap *map, const char *key);
const char *TreeMap_containsValue(TreeMap *map, const char *value);

// Iterator API
TreeMapIterator *TreeMapIterator_create(TreeMap *map);
int TreeMapIterator_hasNext(TreeMapIterator *it);
TreeNode *TreeMapIterator_next(TreeMapIterator *it);
void TreeMapIterator_destroy(TreeMapIterator *it);

#endif // TREEMAP_H


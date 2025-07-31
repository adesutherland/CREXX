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

// TreeMap API
TreeMap *TreeMap_create();
void TreeMap_put(TreeMap *map, const char *key, const char *value);
const char *TreeMap_get(TreeMap *map, const char *key);
void TreeMap_remove(TreeMap *map, const char *key);
void TreeMap_destroy(TreeMap *map);
void TreeMap_keys(TreeMap *map, void (*callback)(const char *key, void *ctx), void *ctx);
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


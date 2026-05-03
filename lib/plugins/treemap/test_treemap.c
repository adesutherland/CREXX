#include <stdio.h>
#include "treemap.h"

void print_key(const char *key, void *ctx) {
    printf("key: %s\n", key);
}

int main() {
    TreeMap *map = TreeMap_create();
    TreeMap_put(map, "delta", "4");
    TreeMap_put(map, "alpha", "1");
    TreeMap_put(map, "charlie", "3");
    TreeMap_put(map, "bravo", "2");

    TreeMap_remove(map, "charlie");

    printf("alpha → %s\n", TreeMap_get(map, "alpha"));
    printf("lastKey → %s\n", TreeMap_lastKey(map));
    printf("firstKey → %s\n", TreeMap_firstKey(map));

    printf("\nAll keys in order:\n");
    TreeMap_keys(map, print_key, NULL);

    TreeMap_put(map, "foo", "bar");
    TreeMap_put(map, "baz", "qux");

    printf("ContainsKey 'foo'? %d\n", TreeMap_containsKey(map, "foo"));
    printf("ContainsKey 'xyz'? %d\n", TreeMap_containsKey(map, "xyz"));
    printf("ContainsValue 'bar'? %d\n", TreeMap_containsValue(map, "bar"));
    printf("ContainsValue 'notfound'? %d\n", TreeMap_containsValue(map, "notfound"));

    printf("\nIterating TreeMap in order:\n");
    TreeMapIterator *it = TreeMapIterator_create(map);
    while (TreeMapIterator_hasNext(it)) {
        TreeNode *n = TreeMapIterator_next(it);
        printf("%s => %s\n", n->key, n->value);
    }
    TreeMapIterator_destroy(it);
    TreeMap_destroy(map);
    return 0;
}


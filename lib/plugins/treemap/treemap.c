// TreeMap implementation in C using a Red-Black Tree
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "treemap.h"
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

#if defined(__APPLE__)
  #include <string.h>
#endif

#define STACK_CAPACITY 1024           // this defines the maximum depth of the tree, it is only used in tmap_keys (retrieve all keys)

// Internal helper function declarations
static TreeNode *new_node(const char *key, const char *value);
static void left_rotate(TreeMap *map, TreeNode *x);
static void right_rotate(TreeMap *map, TreeNode *y);
static void insert_fixup(TreeMap *map, TreeNode *z);
static void transplant(TreeMap *map, TreeNode *u, TreeNode *v);
static TreeNode *minimum(TreeNode *node);
static TreeNode *maximum(TreeNode *node);
static void delete_fixup(TreeMap *map, TreeNode *x);
static TreeNode *find_node(TreeNode *root, const char *key);
static void free_tree(TreeNode *node);

static inline uint64_t simple_hash64(const char *str) {
    uint64_t hash = 14695981039346656037ULL;  // FNV offset basis
    const char *s = str;
    while (*s) {
        hash ^= (unsigned char)(*s++);
        hash *= 1099511628211ULL;
    }
    // Pad with nulls if length < 32
    size_t len = s - str;
    if(len>32) return hash;
    for (size_t i = len; i < 32; ++i) {
        hash ^= 0;
        hash *= 1099511628211ULL;
    }
    return hash;
}

static inline void str2upper(char* str) {
    while (*str) {
        *str = toupper((unsigned char)*str);
        str++;
    }
}

TreeMap *TreeMap_create() {
    TreeMap *map = malloc(sizeof(TreeMap));
    map->root = NULL;
    return map;
}

static inline TreeNode *new_node(const char *key, const char *value) {
    TreeNode *n = malloc(sizeof(TreeNode));
    n->key = strdup(key);
    n->value = strdup(value);
    n->color = RED;
    n->left = n->right = n->parent = NULL;
    return n;
}

static inline char *TreeMap_get(TreeMap *map, const char *key) {
    TreeNode *n = find_node(map->root, key);
    return n ? n->value : NULL;
}

static inline TreeNode *find_node(TreeNode *root, const char *key) {
    while (root) {
        int cmp = strcmp(key, root->key);
        if (cmp == 0) return root;
        root = (cmp < 0) ? root->left : root->right;
    }
    return NULL;
}

int TreeMap_put(TreeMap *map, const char *key, const char *value) {
    TreeNode *z = new_node(key, value);
    TreeNode *y = NULL;
    TreeNode *x = map->root;

    while (x) {
        y = x;
        int cmp = strcmp(z->key, x->key);
        if (cmp == 0) {
            free(x->value);
            x->value = strdup(value);
            free(z->key);
            free(z->value);
            free(z);
            return 4;
        }
        x = (cmp < 0) ? x->left : x->right;
    }

    z->parent = y;
    if (!y)
        map->root = z;
    else if (strcmp(z->key, y->key) < 0)
        y->left = z;
    else
        y->right = z;

    insert_fixup(map, z);
    return 0;
}

static void left_rotate(TreeMap *map, TreeNode *x) {
    TreeNode *y = x->right;
    x->right = y->left;
    if (y->left)
        y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent)
        map->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x;
    x->parent = y;
}

static void right_rotate(TreeMap *map, TreeNode *y) {
    TreeNode *x = y->left;
    y->left = x->right;
    if (x->right)
        x->right->parent = y;
    x->parent = y->parent;
    if (!y->parent)
        map->root = x;
    else if (y == y->parent->left)
        y->parent->left = x;
    else
        y->parent->right = x;
    x->right = y;
    y->parent = x;
}

static void insert_fixup(TreeMap *map, TreeNode *z) {
    while (z->parent && z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            TreeNode *y = z->parent->parent->right;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    left_rotate(map, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                right_rotate(map, z->parent->parent);
            }
        } else {
            TreeNode *y = z->parent->parent->left;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(map, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                left_rotate(map, z->parent->parent);
            }
        }
    }
    map->root->color = BLACK;
}

static inline TreeNode *minimum(TreeNode *node) {
    while (node && node->left) node = node->left;
    return node;
}

static inline TreeNode *maximum(TreeNode *node) {
    while (node && node->right) node = node->right;
    return node;
}

const char *TreeMap_firstKey(TreeMap *map) {
    TreeNode *min = minimum(map->root);
    return min ? min->key : NULL;
}

const char *TreeMap_lastKey(TreeMap *map) {
    TreeNode *max = maximum(map->root);
    return max ? max->key : NULL;
}

int TreeMap_remove(TreeMap *map, const char *key) {
    TreeNode *z = find_node(map->root, key);
    if (!z) return 4;

    TreeNode *y = z;
    TreeNode *x;
    Color y_original_color = y->color;

    if (!z->left) {
        x = z->right;
        transplant(map, z, z->right);
    } else if (!z->right) {
        x = z->left;
        transplant(map, z, z->left);
    } else {
        y = minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            if (x) x->parent = y;
        } else {
            transplant(map, y, y->right);
            y->right = z->right;
            if (y->right) y->right->parent = y;
        }
        transplant(map, z, y);
        y->left = z->left;
        if (y->left) y->left->parent = y;
        y->color = z->color;
    }

    free(z->key);
    free(z->value);
    free(z);

    if (y_original_color == BLACK && x)
        delete_fixup(map, x);
    return 0;
}

static void transplant(TreeMap *map, TreeNode *u, TreeNode *v) {
    if (!u->parent)
        map->root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;
    if (v)
        v->parent = u->parent;
}

static void delete_fixup(TreeMap *map, TreeNode *x) {
    while (x != map->root && (!x || x->color == BLACK)) {
        if (x == x->parent->left) {
            TreeNode *w = x->parent->right;
            if (w && w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                left_rotate(map, x->parent);
                w = x->parent->right;
            }
            if ((!w->left || w->left->color == BLACK) && (!w->right || w->right->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (!w->right || w->right->color == BLACK) {
                    if (w->left) w->left->color = BLACK;
                    w->color = RED;
                    right_rotate(map, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w->right) w->right->color = BLACK;
                left_rotate(map, x->parent);
                x = map->root;
            }
        } else {
            TreeNode *w = x->parent->left;
            if (w && w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                right_rotate(map, x->parent);
                w = x->parent->left;
            }
            if ((!w->right || w->right->color == BLACK) && (!w->left || w->left->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (!w->left || w->left->color == BLACK) {
                    if (w->right) w->right->color = BLACK;
                    w->color = RED;
                    left_rotate(map, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w->left) w->left->color = BLACK;
                right_rotate(map, x->parent);
                x = map->root;
            }
        }
    }
    if (x) x->color = BLACK;
}

void TreeMap_destroy(TreeMap *map) {
    free_tree(map->root);
    free(map);
}

static void free_tree(TreeNode *node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node->key);
    free(node->value);
    free(node);
}

int TreeMap_containsKey(TreeMap *map, const char *key) {
    TreeNode *n = map->root;
    while (n) {
        int cmp = strcmp(key, n->key);
        if (cmp == 0) return 1;
        n = (cmp < 0) ? n->left : n->right;
    }
    return 0;
}

const char * TreeMap_containsValue(TreeMap *map, const char *value) {
    TreeMapIterator *it = TreeMapIterator_create(map);
    while (TreeMapIterator_hasNext(it)) {
        TreeNode *n = TreeMapIterator_next(it);
        if (strcmp(n->value, value) == 0) {
            char* hasKey=strdup(n->key);
            TreeMapIterator_destroy(it);
            return hasKey;
        }
    }
    TreeMapIterator_destroy(it);
    return "";
}

TreeMapIterator *TreeMapIterator_create(TreeMap *map) {
    TreeMapIterator *it = malloc(sizeof(TreeMapIterator));
    it->capacity = 64;
    it->stack = malloc(sizeof(TreeNode *) * it->capacity);
    it->top = 0;
    TreeNode *cur = map->root;
    while (cur) {
        if (it->top >= it->capacity) {
            it->capacity *= 2;
            it->stack = realloc(it->stack, sizeof(TreeNode *) * it->capacity);
        }
        it->stack[it->top++] = cur;
        cur = cur->left;
    }
    return it;
}

int TreeMapIterator_hasNext(TreeMapIterator *it) {
    return it->top > 0;
}

TreeNode *TreeMapIterator_next(TreeMapIterator *it) {
    if (it->top == 0) return NULL;
    TreeNode *node = it->stack[--it->top];
    TreeNode *cur = node->right;
    while (cur) {
        if (it->top >= it->capacity) {
            it->capacity *= 2;
            it->stack = realloc(it->stack, sizeof(TreeNode *) * it->capacity);
        }
        it->stack[it->top++] = cur;
        cur = cur->left;
    }
    return node;
}

void TreeMapIterator_destroy(TreeMapIterator *it) {
    free(it->stack);
    free(it);
}

void register_map(long long mapi, char * name) {
    TreeMapRegistry *entry = malloc(sizeof(TreeMapRegistry));
    entry->map = mapi;
    entry->next = registry_head;
    entry->name = simple_hash64(name);
    entry->entries = 0;
    registry_head = entry;
}
TreeMapRegistry* is_valid_map(long long map) {
    TreeMapRegistry *curr = registry_head;
    while (curr) {
        if (curr->map == map) return curr;
        curr = curr->next;
    }
    printf("+++ Tree Map %lld does not exist\n", map);
    return NULL;
}

uint64_t lookup_map(char * tree_name) {
    uint64_t name = simple_hash64(tree_name);
    TreeMapRegistry *curr = registry_head;
    while (curr) {
        if (curr->name == name) return curr->map;
        curr = curr->next;
    }
    return 0;
}

typedef struct Entry {
    char *key;
    char *value;
    struct Entry *next;
} Entry;

typedef struct Stem {
    Entry **buckets;     // now a dynamic array
    size_t table_size;   // store the actual size
    int entries;         // added entries
    int updated;         // updated entries
    int reads;           // total reads
    int readtry;         // tried reads
    int collisionR;      // re-reads in collision situation
    int max_chain_depth; // maximum collision depth
    int collisionW;      // write collisions took place
} Stem;

Stem *create_stem(size_t table_size) {
    Stem *s = malloc(sizeof(Stem));
    if (!s) return NULL;

    s->table_size = table_size;
    s->buckets = calloc(table_size, sizeof(Entry *));
    if (!s->buckets) {
        free(s);
        return NULL;
    }
    s->entries=0;
    s->collisionW=0;
    s->collisionR=0;
    s->reads=0;
    s->readtry=0;
    s->updated=0;
    s->max_chain_depth=0;
  return s;
}

int put_stem(Stem *s, const char *key, const char *value) {
    size_t idx = simple_hash64(key)%s->table_size;
    Entry *e = s->buckets[idx];

    // Check for existing key
    int depth = 0;
    while (e) {
        if (strcmp(e->key, key) == 0) {
            // Existing key, update value — NOT a collision
            free(e->value);
            e->value = strdup(value);
            s->updated++;
            return 4;  // Updated existing key
        }
        // Mismatch — real collision
        depth++;
        e = e->next;
    }
    // At this point, we’re inserting a new key
    if (depth > 0) {
        s->collisionW++;  // ✅ One collision per collided insert
    }
    if (depth + 1 > s->max_chain_depth) {
        s->max_chain_depth = depth + 1;
    }
    // Insert new entry
    Entry *new_entry = malloc(sizeof(Entry));
    new_entry->key = strdup(key);
    new_entry->value = strdup(value);
    new_entry->next = s->buckets[idx];
    s->buckets[idx] = new_entry;
    s->entries++;
    return 0;
}

char *get_stem(Stem *s, const char *key) {
    int collr=0;
    if (!s || !key) return NULL;

    size_t idx = simple_hash64(key)% s->table_size;  // Compute bucket index
    Entry *e = s->buckets[idx];
    s->readtry++;
    while (e) {
        if (strcmp(e->key, key) == 0) {
            if(collr==0) s->reads++;
            return e->value;  // Found match
        }
        collr=1;
        s->collisionR++;
        e = e->next;
    }

    return NULL;  // Not found
}

void stem_free(Stem *s) {
    for (size_t i = 0; i < s->table_size; ++i) {
        Entry *e = s->buckets[i];
        while (e) {
            Entry *next = e->next;
            free(e->key);
            free(e->value);
            free(e);
            e = next;
        }
    }
    free(s->buckets);
    free(s);
}

/* ------------------------------------------------------------------------------------
 * Create a new Tree
 * Returned is the tree address
 * ------------------------------------------------------------------------------------
 */
PROCEDURE(tmap_create) {
    TreeMap *map = TreeMap_create();
    long long mapi=(long long) map;
    char* name=GETSTRING(ARG0);
    if(strlen(name)==0) strcpy(name,"UNNAMED");
    else str2upper(name);
    register_map(mapi,name);
    RETURNINTX(mapi)
ENDPROC
}

PROCEDURE(tmap_lookup) {
    char* name=GETSTRING(ARG0);
    if(strlen(name)==0) strcpy(name,"UNNAMED");
    else str2upper(name);
    RETURNINTX(lookup_map(name));
    ENDPROC
}
/* ------------------------------------------------------------------------------------
 * Add a new entry to the key
 * Returned is the tree address
 * ------------------------------------------------------------------------------------
 */

PROCEDURE(tmap_put) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    int rc=TreeMap_put(map, GETSTRING(ARG1), GETSTRING(ARG2));
    if (rc==0) treeCB->entries++;
    RETURNINTX(rc)
ENDPROC
}

PROCEDURE(tmap_get) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    char * key= GETSTRING(ARG1);
    char * value =TreeMap_get(map,key);
    if(value==NULL) RETURNSTRX("");
    RETURNSTRX(value) ;
ENDPROC
}

PROCEDURE(tmap_haskey) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    char * key= GETSTRING(ARG1);
    RETURNINTX(TreeMap_containsKey(map,key))   // returns 1 for key is present, or 0 not available
ENDPROC
}

PROCEDURE(tmap_containsKey) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    char * key= GETSTRING(ARG1);
    RETURNINTX(TreeMap_containsKey(map,key))   // returns 1 for key is present, or 0 not available
ENDPROC
}

PROCEDURE(tmap_hasvalue) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    char* value=GETSTRING(ARG1);
   RETURNSTRX((char *) TreeMap_containsValue(map,value))  // returns the key if value was found, else "" empty string
ENDPROC
}

PROCEDURE(tmap_containsValue) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    char* value=GETSTRING(ARG1);
   RETURNSTRX((char *) TreeMap_containsValue(map,value))  // returns the key if value was found, else "" empty string
ENDPROC
}

PROCEDURE(tmap_lastkey) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    char * key= GETSTRING(ARG1);
    RETURNSTRX((char *) TreeMap_lastKey(map));
ENDPROC
}

PROCEDURE(tmap_firstkey) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    char * key= GETSTRING(ARG1);
    RETURNSTRX((char *) TreeMap_firstKey(map));
    ENDPROC
}

PROCEDURE(tmap_size) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    RETURNINTX(treeCB->entries);
    ENDPROC
}

PROCEDURE(tmap_remove) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    char * key= GETSTRING(ARG1);
    int rc=TreeMap_remove(map,key);          // doesn't deliver valid return code TODO produce valid RC
    if (rc==0) treeCB->entries--;
    RETURNINTX(rc);
    ENDPROC
}

PROCEDURE(tmap_keys) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    int hi = 0;

    TreeNode *stack[STACK_CAPACITY];
    int top = -1;
    TreeNode *current = map->root;

    while (top >= 0 || current) {
        while (current) {
            stack[++top] = current;
            current = current->left;
        }
        current = stack[top--];
        PUSHSARRAY(ARG1, hi, current->key);
        hi++;
        current = current->right;
    }
    RETURNINTX(hi)
ENDPROC
}

PROCEDURE(tmap_dump) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    int hi = 0;
    TreeMapIterator *it = TreeMapIterator_create(map);
    while (TreeMapIterator_hasNext(it)) {
      TreeNode *n = TreeMapIterator_next(it);
      PUSHSARRAY(ARG1, hi, n->key);
      PUSHSARRAY(ARG2, hi, n->value);
      hi++;
    }
    TreeMapIterator_destroy(it);
    RETURNINTX(hi)
ENDPROC
}

PROCEDURE(tmap_free) {
    long long mapi = GETINT(ARG0);
    TreeMap *map = (TreeMap *) mapi;
    TreeMapRegistry * treeCB=is_valid_map(mapi) ;
    if(treeCB==0 ) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "invalid TreeMap")

    char * key= GETSTRING(ARG1);
    TreeMap_destroy(map);             // There is no valid return code available
    RETURNINTX(0);                    // always return 0
ENDPROC
}
PROCEDURE(stem_iterate) {
    long long tokeni = GETINT(ARG0);
    Stem *token = (Stem *) tokeni;

    if (!token) RETURNINTX(0);  // defensive check

    int hi = 0;
    for (size_t i = 0; i < token->table_size; ++i) {
        Entry *e = token->buckets[i];
        while (e) {
            PUSHSARRAY(ARG1, hi, e->key);
            PUSHSARRAY(ARG2, hi, e->value);
            hi++;
            e = e->next;  // correct and needed
        }
    }

    RETURNINTX(hi);
    ENDPROC
}

PROCEDURE(stem_create) {
    int size=GETINT(ARG0);             // expected number of entries
    if(size<1024) size=1024;
    size=size*3;                      // *10 keeps the collision probability ~ 0.01%
    Stem * token = create_stem(size);
    RETURNINTX((long long) token);
    ENDPROC
}
PROCEDURE(stem_put) {
    long long tokeni = GETINT(ARG0);
    Stem *token = (Stem *) tokeni;
    char * key=GETSTRING(ARG1);
    char * vvalue=GETSTRING(ARG2);
    int rc=put_stem(token, GETSTRING(ARG1),GETSTRING(ARG2));
    RETURNINTX(rc);
    ENDPROC
}
PROCEDURE(stem_get) {
    long long tokeni = GETINT(ARG0);
    Stem *token = (Stem *) tokeni;
    char * result=get_stem(token, GETSTRING(ARG1));
    RETURNSTRX(result);
    ENDPROC
}
PROCEDURE(stem_hi) {
    long long tokeni = GETINT(ARG0);
    Stem *token = (Stem *) tokeni;
    RETURNINTX(token->entries);
    ENDPROC
}

PROCEDURE(stem_stats) {
    long long tokeni = GETINT(ARG0);
    Stem *token = (Stem *) tokeni;
    int empty = 0;
    for (int i = 0; i < token->table_size; ++i) {
        if (token->buckets[i] == NULL) empty++;
    }
    printf("Stem Statistics (%llu)\n", tokeni);
    printf("-------------------------------\n");
    printf(" Stem Size         %d\n", token->table_size);
    printf(" Load Factor       %.3f %\n",(double) 100*token->entries / token->table_size);
    printf(" Added Entries     %d\n", token->entries);
    printf(" Updated Entries   %d\n", token->updated);
    printf(" Direct Inserts    %d\n", token->entries - token->collisionW);
    printf(" Collision Inserts %d\n", token->collisionW);
    printf(" Empty Buckets     %d\n", empty);
    printf(" Read Requests     %d\n", token->readtry);
    printf(" Direct Reads      %d\n", token->reads);
    printf(" Chained Reads     %d\n", token->collisionR);
    printf(" Max Chain Depth   %d\n", token->max_chain_depth);
    printf(" Avg Probe Depth   %.2f\n", (double)((token->reads + token->collisionR) / token->readtry));
    printf(" Chain Load (%)    %.2f%%\n", 100.0 * token->collisionW / token->entries);
    RETURNINTX(0);
    ENDPROC
}


LOADFUNCS
    ADDPROC(tmap_create,        "treemap.tmcreate",      "b",    ".int","name=''");
    ADDPROC(tmap_lookup,        "treemap.tmlookup",      "b",    ".int","name=.string");
    ADDPROC(tmap_put,           "treemap.tmput",         "b",    ".int",    "map=.int, key=.string, value=.string");
    ADDPROC(tmap_get,           "treemap.tmget",         "b",    ".string", "map=.int, key=.string");
    ADDPROC(tmap_remove,        "treemap.tmremove",      "b",    ".int",    "map=.int, key=.string");
    ADDPROC(tmap_haskey,        "treemap.tmhaskey",      "b",    ".int",    "map=.int, key=.string");
    ADDPROC(tmap_hasvalue,      "treemap.tmhasvalue",    "b",    ".string", "map=.string, value=.string");
    ADDPROC(tmap_containsKey,   "treemap.tmcontainsKey", "b",    ".int",    "map=.int, key=.string");
    ADDPROC(tmap_containsValue, "treemap.tmcontainsValue","b",    ".string", "map=.string, value=.string");
    ADDPROC(tmap_lastkey,       "treemap.tmlastkey",      "b",    ".string", "map=.int");
    ADDPROC(tmap_size,          "treemap.tmsize",         "b",    ".int",    "map=.int");
    ADDPROC(tmap_firstkey,      "treemap.tmfirstkey",     "b",   ".string",  "map=.int");
    ADDPROC(tmap_free,          "treemap.tmfree",         "b",    ".int",    "map=.int");
    ADDPROC(tmap_keys,          "treemap.tmkeys",         "b",    ".int",    "map=.int, expose list=.string[]");
    ADDPROC(tmap_dump,          "treemap.tmdump",         "b",    ".int",    "map=.int, expose keys=.string[], expose values=.string[]");
    ADDPROC(stem_create,        "treemap.stemcreate",     "b",    ".int",    "items=.int");
    ADDPROC(stem_put,           "treemap.stemput",        "b",    ".int",    "token=.int, key=.string, value=.string");
    ADDPROC(stem_get,           "treemap.stemget",        "b",    ".string", "token=.int, key=.string");
    ADDPROC(stem_iterate,       "treemap.stemiterate",    "b",    ".int",    "token=.int, expose keys=.string[], expose values=.string[]");
    ADDPROC(stem_hi,            "treemap.stemsize",        "b",    ".int",    "token=.int");
    ADDPROC(stem_stats,         "treemap.stemstats",       "b",    ".int",    "token=.int");
ENDLOADFUNCS
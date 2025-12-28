// map implementation in C using a Red-Black Tree
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

#define ENOENTRY 12
#define ENOVALUE 22

#if defined(__APPLE__)
  #include <string.h>
#endif

#define SPLITSTEM()                           \
        char *full_ = GETSTRING(ARG0);        \
        char *stem_name=NULL;                 \
        char *stem_index=NULL;                \
        char *stem_ptr  = strchr(full_, '.'); \
        if (stem_ptr==NULL) last_rhmap_rc = STEM_MSG_NOT_A_STEM; \
        else { /* is not a stem  */           \
           *stem_ptr = '\0';                  \
           stem_name  = full_;                \
           stem_index = stem_ptr + 1;         \
           str2upper(stem_name) ;             \
        }
#define GETROOT() \
        char *stem_name=GETSTRING(ARG0);      \
        char *stem  = strchr(stem_name, '.'); \
        if (stem) *stem='\0';                 \
        str2upper(stem_name);

static inline void str2upper(char* str) {
    while (*str) {
        *str = toupper((unsigned char)*str);
        str++;
    }
}

double elapsed_time(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}
double is_time(struct timespec start) {
    return ( start.tv_sec) + (start.tv_nsec) / 1e9;
}
/* ============================================================
 * Robin Hood hashmap core (generic value pointer)
 * Used for:
 *   - root map:  stem name -> stem_map_t*
 *   - stem map:  index     -> char*
 * ============================================================ */

/* Optional but recommended */
enum {
    STEM_MSG_OK               = 0,
    STEM_MSG_UNDEFINED_STEM   = 10,
    STEM_MSG_UNDEFINED_ELEM   = 11,
    STEM_MSG_NOT_A_STEM       = 12,
    STEM_MSG_INTERNAL_ERROR   = 99
};


typedef struct rhmap_slot {
    uint32_t   hash;
    char      *key;    /* owned by map; NULL = unused */
    void      *value;  /* NOT owned by map; semantics by caller */
    bool       used;
} rhmap_slot_t;

typedef struct rhmap {
    rhmap_slot_t *slots;
    size_t        capacity;      /* power of two */
    size_t        size;          /* number of live entries */
    double        max_load;      /* e.g. 0.80 */
    size_t        rehash_count;  /* number of (real) rehashes */
    double        rehash_time;   /* time used for rehash in ms */
    /* Probe statistics */
    size_t        num_lookups;
    size_t        total_lookup_probes;
    size_t        num_inserts;
    size_t        total_insert_probes;
    size_t        max_probe_len; /* max over lookups+inserts */
} rhmap_t;

/* Root map: stem name -> stem_map_t* */
static rhmap_t g_root_map = { 0 };

/* A stem map is just another rhmap (index -> char* value) */
typedef rhmap_t stem_map_t;

/* last GETSTEM rc when no stem map exists */
static int last_rhmap_rc = 0;

/* ---------- hashing ---------- */

static uint32_t
hash_fnv1a(const char *s)
{
    uint32_t h = 2166136261u;
    const unsigned char *p = (const unsigned char *)s;

    while (*p) {
        h ^= *p++;
        h *= 16777619u;
    }
    return h;
}

int round_up_power_of_two(int x)
{
    x--;                    // key difference: preserve existing powers of two
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

static inline size_t
ideal_index(uint32_t hash, size_t capacity)
{
    return (size_t)(hash & (uint32_t)(capacity - 1));
}

static inline size_t
probe_distance(size_t slot_index, uint32_t hash, size_t capacity)
{
    size_t ideal = ideal_index(hash, capacity);
    if (slot_index >= ideal) {
        return slot_index - ideal;
    }
    return (capacity - ideal) + slot_index;
}

/* ---------- core map ops ---------- */

static int
rhmap_rehash(rhmap_t *m, size_t new_capacity)
{
    new_capacity = round_up_power_of_two(new_capacity);

    rhmap_slot_t *new_slots = calloc(new_capacity, sizeof(rhmap_slot_t));
    if (!new_slots) {
        return -ENOENTRY;
    }

    rhmap_slot_t *old_slots    = m->slots;
    size_t        old_capacity = m->capacity;

    m->slots    = new_slots;
    m->capacity = new_capacity;
    m->size     = 0;
    /* Reset stats on new table */
    m->num_lookups         = 0;
    m->total_lookup_probes = 0;
    m->num_inserts         = 0;
    m->total_insert_probes = 0;
    m->max_probe_len       = 0;


    if (!old_slots) {
        return 0;  /* first allocation, don't count as "rehash" */
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);  // Startzeit
    m->rehash_count++;
    /* reinsert all */
    for (size_t i = 0; i < old_capacity; ++i) {
        rhmap_slot_t *s = &old_slots[i];
        if (!s->used || s->key == NULL) {
            continue;
        }

        uint32_t hash = s->hash;
        char    *key  = s->key;
        void    *val  = s->value;

        size_t idx  = ideal_index(hash, new_capacity);
        size_t dist = 0;

        rhmap_slot_t item = {
                .hash  = hash,
                .key   = key,
                .value = val,
                .used  = true
        };

        for (;;) {
            rhmap_slot_t *dst = &m->slots[idx];
            if (!dst->used) {
                *dst = item;
                m->size++;
                break;
            }

            size_t dst_dist = probe_distance(idx, dst->hash, new_capacity);
            if (dst_dist < dist) {
                rhmap_slot_t tmp = *dst;
                *dst = item;
                item = tmp;
            }

            idx  = (idx + 1) & (new_capacity - 1);
            dist++;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);    // End time
    m->rehash_time+=elapsed_time(start,end);
    printf("Rehash Time %f %d %d\n",m->rehash_time,old_capacity,new_capacity);
    free(old_slots);
    return 0;
}

static inline int rhmap_ensure_capacity(rhmap_t *m)
{
    if (m->capacity == 0) {
        m->max_load     = 0.80;
        m->rehash_count = 0;
        m->num_lookups         = 0;
        m->total_lookup_probes = 0;
        m->num_inserts         = 0;
        m->total_insert_probes = 0;
        m->max_probe_len       = 0;

        return rhmap_rehash(m, 32);
    }

    double load = (double)m->size / (double)m->capacity;

    if (load > m->max_load) {
        size_t old_cap = m->capacity;
        size_t new_cap;

        if (old_cap < 1000000) {   /* < 1M: always double */
            new_cap = old_cap * 4;
        } else {                      /* >= 1M: grow by 1.25x */
            new_cap = old_cap*2;
        }

        /* ensure power-of-two */
        new_cap = round_up_power_of_two(new_cap);

        return rhmap_rehash(m, new_cap);
    }

    return 0; /* nothing to do */
}


/* Early rehash based on probe length (cluster prevention) */
static inline void rhmap_maybe_rehash(rhmap_t *m)
{
    const size_t MAX_PROBE_SOFT = 20;
    const size_t MAX_PROBE_HARD = 32;

    if (m->max_probe_len > MAX_PROBE_HARD) {
        /* force double capacity */
        rhmap_rehash(m, m->capacity * 2);
        m->max_probe_len = 0;
        return;
    }

    if (m->max_probe_len > MAX_PROBE_SOFT) {
        /* mild rebuild at same size */
        rhmap_rehash(m, m->capacity);
        m->max_probe_len = 0;
    }
}

/* Insert/update generic pointer value.
 * Map OWNS the key (it will strdup on first insert), but NOT the value.
 */
static int
rhmap_put_ptr(rhmap_t *m, const char *key, void *value)
{
    if (!m || !key) {
        return -ENOVALUE;
    }

    int rc = rhmap_ensure_capacity(m);
    if (rc < 0) return rc;

    uint32_t hash = hash_fnv1a(key);
    size_t   cap  = m->capacity;
    size_t   idx  = ideal_index(hash, cap);
    size_t   dist = 0;

    rhmap_slot_t item = {
            .hash  = hash,
            .key   = NULL,   /* will be strdup'd lazily if needed */
            .value = value,
            .used  = true
    };

    for (;;) {
        rhmap_slot_t *s = &m->slots[idx];

        if (!s->used) {
            /* new key */
            if (!item.key) {
                item.key = strdup(key);
                if (!item.key) {
                    return -ENOENTRY;
                }
            }
            *s = item;
            m->size++;

            /* stats: insertion (new entry) */
            m->num_inserts++;
            m->total_insert_probes += dist;
            if (dist > m->max_probe_len) m->max_probe_len = dist;

            return 0;
        }

        if (s->hash == hash && s->key && strcmp(s->key, key) == 0) {
            /* update existing value (key already owned by map) */
            s->value = value;

            /* stats: insertion (update) */
            m->num_inserts++;
            m->total_insert_probes += dist;
            if (dist > m->max_probe_len) m->max_probe_len = dist;

            return 0;
        }

        size_t s_dist = probe_distance(idx, s->hash, cap);
        if (s_dist < dist) {
            /* Robin Hood swap */
            if (!item.key) {
                item.key = strdup(key);
                if (!item.key) {
                    return -ENOENTRY;
                }
            }
            rhmap_slot_t tmp = *s;
            *s = item;
            item = tmp;
        }

        idx  = (idx + 1) & (cap - 1);
        dist++;
    }
}

/* Lookup; returns value pointer or NULL. */
static void *
rhmap_get_ptr(rhmap_t *m, const char *key)
{
    if (!m || !key || m->capacity == 0) {
        return NULL;
    }

    /* early adaptive rehash ===> dropped for the moment too agressive */
     if (m->max_probe_len>128) {
         if (m->num_inserts%10000==0) rhmap_maybe_rehash(m);
     }

    /* writable alias for stats */
    rhmap_t *mw = (rhmap_t *)m;

    uint32_t hash = hash_fnv1a(key);
    size_t   cap  = m->capacity;
    size_t   idx  = ideal_index(hash, cap);
    size_t   dist = 0;

    for (;;) {
        rhmap_slot_t *s = &m->slots[idx];
        if (!s->used) {
            /* stats: lookup miss */
            mw->num_lookups++;
            mw->total_lookup_probes += dist;
            if (dist > mw->max_probe_len) mw->max_probe_len = dist;
            return NULL;
        }

        size_t s_dist = probe_distance(idx, s->hash, cap);
        if (s_dist < dist) {
            /* stats: lookup miss (cut-off by Robin Hood invariant) */
            mw->num_lookups++;
            mw->total_lookup_probes += dist;
            if (dist > mw->max_probe_len) mw->max_probe_len = dist;
            return NULL;
        }

        if (s->hash == hash && s->key && strcmp(s->key, key) == 0) {
            /* stats: lookup hit */
            mw->num_lookups++;
            mw->total_lookup_probes += dist;
            if (dist > mw->max_probe_len) mw->max_probe_len = dist;
            return s->value;
        }

        idx  = (idx + 1) & (cap - 1);
        dist++;
    }
}


static int
rhmap_remove_key(rhmap_t *m, const char *key)
{
    if (!m || !key || m->capacity == 0) {
        return -ENOENTRY;
    }

    uint32_t hash = hash_fnv1a(key);
    size_t   cap  = m->capacity;
    size_t   idx  = ideal_index(hash, cap);
    size_t   dist = 0;

    for (;;) {
        rhmap_slot_t *s = &m->slots[idx];
        if (!s->used) {
            return -ENOENTRY;
        }

        size_t s_dist = probe_distance(idx, s->hash, cap);
        if (s_dist < dist) {
            return -ENOENTRY;
        }

        if (s->hash == hash && s->key && strcmp(s->key, key) == 0) {
            /* Found: free key, then backward-shift delete cluster. */
            free(s->key);
            s->key = NULL;

            size_t hole = idx;
            size_t next = (idx + 1) & (cap - 1);

            for (;;) {
                rhmap_slot_t *n = &m->slots[next];
                if (!n->used) {
                    /* end of cluster; clear hole */
                    m->slots[hole].used  = false;
                    m->slots[hole].hash  = 0;
                    m->slots[hole].key   = NULL;
                    m->slots[hole].value = NULL;
                    m->size--;
                    return 0;
                }

                size_t n_ideal = ideal_index(n->hash, cap);
                size_t n_dist;
                if (next >= n_ideal) {
                    n_dist = next - n_ideal;
                } else {
                    n_dist = (cap - n_ideal) + next;
                }

                if (n_dist == 0) {
                    /* this one is at its ideal position; stop shifting */
                    m->slots[hole].used  = false;
                    m->slots[hole].hash  = 0;
                    m->slots[hole].key   = NULL;
                    m->slots[hole].value = NULL;
                    m->size--;
                    return 0;
                }

                /* shift n back into the hole */
                m->slots[hole] = *n;
                hole = next;
                next = (next + 1) & (cap - 1);
            }
        }

        idx  = (idx + 1) & (cap - 1);
        dist++;
    }
}

static void
stem_map_destroy(stem_map_t *m)
{
    if (!m) return;

    if (m->slots) {
        for (size_t i = 0; i < m->capacity; ++i) {
            rhmap_slot_t *s = &m->slots[i];
            if (!s->used) continue;

            if (s->key) {
                free(s->key);
                s->key = NULL;
            }
            if (s->value) {
                /* value is char* for per-stem maps */
                free(s->value);
                s->value = NULL;
            }
        }
        free(m->slots);
        m->slots = NULL;
    }

    free(m);
}

static int drop_stem_internal(const char *stem_name)
{
    stem_map_t *m = (stem_map_t *)rhmap_get_ptr(&g_root_map, stem_name);
    if (!m) {
        return -ENOENTRY;  /* no such stem */
    }

    /* free the stem map (keys + values + slots + struct) */
    stem_map_destroy(m);

    /* remove stem name -> m pointer from root map */
    return rhmap_remove_key(&g_root_map, stem_name);
}


/* ============================================================
 * Per-stem helpers: index -> char* value
 * ============================================================ */

/* Get or create a stem map for a given stem name (root map). */
static stem_map_t *
get_or_create_stem_map(const char *stem_name)
{
    if (!stem_name) stem_name = "";

    stem_map_t *m = (stem_map_t *)rhmap_get_ptr(&g_root_map, stem_name);
    if (m) return m;

    m = (stem_map_t *)calloc(1, sizeof(stem_map_t));
    if (!m) return NULL;

    /* lazy-init: first insert into m will call rhmap_ensure_capacity */

    int rc = rhmap_put_ptr(&g_root_map, stem_name, m);
    if (rc < 0) {
        free(m);
        return NULL;
    }
    return m;
}

/* Find stem map without creating it (for GETSTEM). */
static stem_map_t *
find_stem_map(const char *stem_name)
{
    if (!stem_name) stem_name = "";
    return (stem_map_t *)rhmap_get_ptr(&g_root_map, stem_name);
}

/* store value string in a per-stem map: key=index, value=char* */
static int
stem_put_value(stem_map_t *m, const char *index, const char *value)
{
    if (!m || !index) return -ENOVALUE;
    const char *src_val = value ? value : "";

    /* check if key exists */
    char *old_val = (char *)rhmap_get_ptr(m, index);
    if (old_val) {
        /* update existing: free old, store new */
        char *new_val = strdup(src_val);
        if (!new_val) return -ENOENTRY;
        free(old_val);
        return rhmap_put_ptr(m, index, new_val);
    }

    /* new key */
    char *new_val = strdup(src_val);
    if (!new_val) return -ENOENTRY;

    /* rhmap_put_ptr will own the key copy (strdup internally) */
    int rc = rhmap_put_ptr(m, index, new_val);
    if (rc < 0) {
        free(new_val);
        return rc;
    }
    return 0;
}

/* get value string from per-stem map; returns NULL if missing */
static char *
stem_get_value(stem_map_t *m, const char *index)
{
    if (!m || !index) return NULL;
    return (char *)rhmap_get_ptr(m, index);
}

/* remove a single tail from a per-stem map: key=index
 * returns 0 on success, -ENOENT if not found, <0 on other errors
 */
static int
stem_remove_value(stem_map_t *m, const char *index)
{
    if (!m || !index) return -ENOVALUE;

    /* find current value */
    char *val = (char *)rhmap_get_ptr(m, index);
    if (!val) {
        return -ENOENTRY;   /* no such entry */
    }

    /* free the value string (we own it at stem layer) */
    free(val);

    /* now remove key from the map (this frees the key and shifts cluster) */
    return rhmap_remove_key(m, index);
}

/* ============================================================
 * CREXX/PA procedures: GETSTEM / SETSTEM / rehash
 * ============================================================ */

/* ---------- GETSTEM(name, index) -> string ---------- */

PROCEDURE(getstem) {
    // printf("GETSTEM '%s'\n",GETSTRING(ARG0));
    SPLITSTEM()
    stem_map_t *m = find_stem_map(stem_name);
   if (!m || stem_ptr==0) {
       char stem[512]="undefined";  // STEM_MSG_NO_STEM is set in SPLITSTEM
       if(stem_ptr) {
          str2upper(stem_index);
          snprintf(stem, sizeof(stem), "%s.%s", stem_name, stem_index);
          last_rhmap_rc = STEM_MSG_UNDEFINED_STEM;
       }
       RETURNSTRX(stem);  /* stem not defined -> return upper case stem name */
   }

    char *val = stem_get_value(m, stem_index);
    if (!val) {
        str2upper(stem_index);
        char stem[512];
        snprintf(stem, sizeof(stem), "%s.%s", stem_name, stem_index);
        last_rhmap_rc = STEM_MSG_UNDEFINED_ELEM; /* message number: entry not defined */
        RETURNSTRX(stem);  /* stem not defined -> return upper case stem name */
    }
    last_rhmap_rc = STEM_MSG_OK;
    RETURNSTRX(val);
    ENDPROC
}

/* ---------- SETSTEM(name, index, value) ---------- */

PROCEDURE(setstem) {
    // printf("SETSTEM '%s' = '%s'\n",GETSTRING(ARG0),GETSTRING(ARG1));
    SPLITSTEM()
    char *value   = GETSTRING(ARG1);
    stem_map_t *m = get_or_create_stem_map(stem_name);
    if (stem_ptr==0) RETURNINTX(-last_rhmap_rc) ;  // rc set in SPLITSTEM
    if (!m) {
        last_rhmap_rc = STEM_MSG_UNDEFINED_STEM; /* message number: stem not defined */
        RETURNINTX(-last_rhmap_rc);    /* allocation or internal error */
    }

    int rc = stem_put_value(m, stem_index, value);
    if (rc < 0) {
        last_rhmap_rc = STEM_MSG_INTERNAL_ERROR; /* message number: stem not defined */
        RETURNINTX(-2);     /* allocation or internal error */
    }
    last_rhmap_rc = STEM_MSG_OK;
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(dropstem) {
    int rc=0;
    SPLITSTEM()
    if (stem_ptr>0) rc = drop_stem_internal(stem_name);

    if (stem_ptr==0) RETURNINTX(-last_rhmap_rc) ;  // rc set in SPLITSTEM
    if (rc<0) {
        last_rhmap_rc = STEM_MSG_UNDEFINED_STEM; /* message number: stem not defined */
        RETURNINTX(-last_rhmap_rc);    /* allocation or internal error */
    }

    last_rhmap_rc = STEM_MSG_OK;  /* reset global rhmap RC */
    RETURNINTX(0);
    ENDPROC
}


PROCEDURE(getstemmsg) {

    switch (last_rhmap_rc) {
        case STEM_MSG_OK: RETURNSTRX("OK");
        case STEM_MSG_UNDEFINED_STEM: RETURNSTRX("Stem not defined");
        case STEM_MSG_UNDEFINED_ELEM: RETURNSTRX("Stem element not defined");
        case STEM_MSG_NOT_A_STEM    : RETURNSTRX("is not a Stem");
        case STEM_MSG_INTERNAL_ERROR: RETURNSTRX("Internal error");
        default: RETURNSTRX("Unknown stem error");
    }
    ENDPROC
}

/* ---------- gettail(stem_name, ordinal) -> tail string or "" ----------
 *
 * Enumerates all tails of a stem in an unspecified order.
 * ordinal is 1-based:
 *   ordinal = 1 -> first tail
 *   ordinal = 2 -> second tail
 *   ...
 *   returns "" if stem undefined or ordinal > number of tails.
 */

PROCEDURE(gettail) {
    SPLITSTEM()
    int ordinal           = GETINT(ARG1);
    if (ordinal <= 0) {
        last_rhmap_rc=STEM_MSG_UNDEFINED_ELEM;
        RETURNSTRX("");
    }

    stem_map_t *m = find_stem_map(stem_name);
    if (stem_ptr==0) RETURNINTX(-last_rhmap_rc) ;  // rc set in SPLITSTEM
    if (!m) {
        last_rhmap_rc = STEM_MSG_UNDEFINED_STEM; /* message number: stem not defined */
        RETURNINTX(-last_rhmap_rc);    /* allocation or internal error */
    }

    int count = 0;

    /* walk the Robin Hood table slots and count used entries */
    for (size_t i = 0; i < m->capacity; ++i) {
        rhmap_slot_t *s = &m->slots[i];
        if (!s->used || !s->key) {
            continue;
        }

        count++;
        if (count == ordinal) {
            last_rhmap_rc = STEM_MSG_OK;
            RETURNSTRX(s->key);              /* s->key IS the tail for this stem */

        }
    }
    last_rhmap_rc = STEM_MSG_UNDEFINED_ELEM;
    /* fewer elements than requested ordinal */
    RETURNSTRX("");
    ENDPROC
}

PROCEDURE(getall) {
   GETROOT();
   stem_map_t *m = find_stem_map(stem_name);
   if (!m) {
        last_rhmap_rc = STEM_MSG_UNDEFINED_STEM; /* message number: stem not defined */
        RETURNINTX(-last_rhmap_rc);    /* allocation or internal error */
    }

    int hi = 0;
    /* walk the Robin Hood table slots and count used entries */
    for (size_t i = 0; i < m->capacity; ++i) {
        rhmap_slot_t *s = &m->slots[i];
        if (!s->used || !s->key) continue;   // empty slot
        PUSHSARRAY(ARG1, hi, s->key);
        PUSHSARRAY(ARG2, hi, s->value);
        hi++;
    }
    last_rhmap_rc = STEM_MSG_OK;
    RETURNINTX(hi);
    ENDPROC
}
PROCEDURE(getalltails) {
    GETROOT();
    SETARRAYHI(ARG1,0);
    stem_map_t *m = find_stem_map(stem_name);
    if (!m) {
        last_rhmap_rc = STEM_MSG_UNDEFINED_STEM; /* message number: stem not defined */
        RETURNINTX(-last_rhmap_rc);    /* allocation or internal error */
    }

    int hi = 0;
    /* walk the Robin Hood table slots and count used entries */
    for (size_t i = 0; i < m->capacity; ++i) {
        rhmap_slot_t *s = &m->slots[i];
        if (!s->used || !s->key) continue;   // empty slot
        PUSHSARRAY(ARG1, hi, s->key);
        hi++;
    }
    last_rhmap_rc = STEM_MSG_OK;
    RETURNINTX(hi);
    ENDPROC
}
PROCEDURE(dropentry) {
    SPLITSTEM()
    stem_map_t *m = find_stem_map(stem_name);
    if (stem_ptr==0) RETURNINTX(-last_rhmap_rc) ;  // rc set in SPLITSTEM
    if (!m) {
        last_rhmap_rc = STEM_MSG_UNDEFINED_STEM; /* message number: stem not defined */
        RETURNINTX(-last_rhmap_rc);    /* allocation or internal error */
    }

    int rc = stem_remove_value(m, stem_index);
    if (rc == -ENOENTRY) {
        /* element not defined */
        last_rhmap_rc = STEM_MSG_UNDEFINED_ELEM;
        RETURNINTX(-11);
    }
    if (rc < 0) {
        /* some internal error in removal */
        last_rhmap_rc = STEM_MSG_INTERNAL_ERROR;
        RETURNINTX(-12);
    }

    last_rhmap_rc = STEM_MSG_OK;
    RETURNINTX(0);
    ENDPROC
}

/* ---------- clonestem(src_stem, dst_stem) -> int (entries copied) ---------- */

PROCEDURE(clonestem) {
    char *src_name = GETSTRING(ARG0);
    char *dst_name = GETSTRING(ARG1);

    /* normalize to uppercase (root keys are uppercase) */
    str2upper(src_name);
    str2upper(dst_name);

    /* no-op if same stem (case-insensitive) */
    if (strcmp(src_name, dst_name) == 0) {
        last_rhmap_rc = STEM_MSG_OK;
        RETURNINTX(0);
    }

    /* find source stem */
    stem_map_t *src = find_stem_map(src_name);
    if (!src) {
        last_rhmap_rc = STEM_MSG_UNDEFINED_STEM;  /* source stem not defined */
        RETURNINTX(-10);
    }

    /* drop any existing destination stem (ignore if missing) */
    (void)drop_stem_internal(dst_name);

    /* create fresh destination stem map */
    stem_map_t *dst = get_or_create_stem_map(dst_name);
    if (!dst) {
        last_rhmap_rc = STEM_MSG_INTERNAL_ERROR;
        RETURNINTX(-11);
    }

    int copied = 0;

    /* walk all entries in source and copy into dst */
    for (size_t i = 0; i < src->capacity; ++i) {
        rhmap_slot_t *s = &src->slots[i];
        if (!s->used || !s->key) continue;

        const char *key = s->key;
        const char *val = (const char *)s->value;

        int rc = stem_put_value(dst, key, val);
        if (rc < 0) {
            last_rhmap_rc = STEM_MSG_INTERNAL_ERROR;
            RETURNINTX(-12);   /* partial copy, but we signal error */
        }
        copied++;
    }

    last_rhmap_rc = STEM_MSG_OK;
    RETURNINTX(copied);
    ENDPROC
}

/* ---------- liststems(expose stems=.string[]) -> int (number of stems) ---------- */

PROCEDURE(liststems) {

    int hi = 0;

    /* walk the root map: stem name -> stem_map_t* */
    for (size_t i = 0; i < g_root_map.capacity; ++i) {
        rhmap_slot_t *s = &g_root_map.slots[i];
        if (!s->used || !s->key) continue;

        /* s->key is the uppercase stem root name */
        PUSHSARRAY(ARG0, hi, s->key);
        hi++;
    }

    last_rhmap_rc = STEM_MSG_OK;
    RETURNINTX(hi);
    ENDPROC
}

/* ---------- stemstats(stem_name) -> string ----------
 * Returns a short summary like:
 *   "STEM FRED: size=100 capacity=256 load=0.39 rehash=3"
 */

PROCEDURE(statsstem) {
    SPLITSTEM()
    char buf[256];

    stem_map_t *m = find_stem_map(stem_name);
    if (stem_ptr==0) RETURNINTX(-last_rhmap_rc) ;  // rc set in SPLITSTEM
    if (!m) {
        last_rhmap_rc = STEM_MSG_UNDEFINED_STEM; /* message number: stem not defined */
        RETURNINTX(-last_rhmap_rc);    /* allocation or internal error */
    }

    double load = 0.0;
    if (m->capacity > 0) {
        load = (double)m->size / (double)m->capacity;
    }

    double avg_lookup_probe = 0.0;
    if (m->num_lookups > 0) {
        avg_lookup_probe = (double)m->total_lookup_probes /
                           (double)m->num_lookups;
    }

    double avg_insert_probe = 0.0;
    if (m->num_inserts > 0) {
        avg_insert_probe = (double)m->total_insert_probes /
                           (double)m->num_inserts;
    }

    snprintf(buf, sizeof(buf),
             "STEM %s: size=%zu capacity=%zu load=%.2f "
             "rehash=%zu rehashTime=%f[sec] maxprobe=%zu avgL=%.2f avgI=%.2f",
             stem_name,
             m->size,
             m->capacity,
             load,
             m->rehash_count,
             m->rehash_time,
             m->max_probe_len,
             avg_lookup_probe,
             avg_insert_probe);

    last_rhmap_rc = STEM_MSG_OK;
    RETURNSTRX(buf);
    ENDPROC
}
/*
 * Transform:
 *   Fred.bert.name.indx
 * into:
 *   "Fred."bert"."name"."indx"
 *
 * Contract:
 *   - returns heap-allocated string (caller must free), or NULL on invalid input
 *   - never returns borrowed pointer
 */

static char *quote_stem_path(char *in) {
    size_t in_len = strlen(in);
    if (in_len == 0) return NULL;

 /* One-shot allocation: worst case <= about 2*in_len + small constant */
    size_t cap = in_len * 2 + 8;
    char *out = (char *) malloc(cap);
    if (!out) return NULL;

    size_t w = 0;
    const char *dot = strchr(in, '.');
    if (!dot) return in;

    /* Root before first '.' */
    size_t root_len = (size_t) (dot - in);
    if (root_len == 0) {
        free(out);
        return NULL;
    } /* ".a" invalid */

    /* Prefix: "Root." (dot belongs to root, never variable) */
    out[w++] = '"';
    memcpy(out + w, in, root_len);
    w += root_len;
    out[w++] = '.';
    out[w++] = '"';

    /* Tail segments */
    const char *p = dot + 1;
    int first = 1;

    while (*p) {
        const char *next = strchr(p, '.');
        size_t seg_len = next ? (size_t) (next - p) : strlen(p);

        if (seg_len == 0) {
            free(out);
            return NULL;
        } /* "a..b" or "a." */

        if (first) {
            /* First tail: no opening quote, only closing quote => bert" */
            memcpy(out + w, p, seg_len);
            w += seg_len;
            out[w++] = '"';
            first = 0;
        } else {
            /* Others: ."seg" */
            out[w++] = '.';
            out[w++] = '"';
            memcpy(out + w, p, seg_len);
            w += seg_len;
            out[w++] = '"';
        }

        if (!next) break;
        p = next + 1;
    }

    out[w - 1] = '\0';
    return out;
}


PROCEDURE(stemquote)
{
    char *input = GETSTRING(ARG0);
    if (!input || !*input) RETURNSTRX("");    // no input no stem

    char *out = quote_stem_path(input);
    if (!out) {
        RETURNSTRX(input); // invalid path: return and let rxpp decided what to do
    }
    RETURNSTR(out);
    if(input != out) free(out);
    ENDPROC
}

/* ============================================================
 * Standard CREXX/PA export table
 * ============================================================ */

LOADFUNCS
    ADDPROC(getstem,            "map.getstem",        "b",     ".string","stem=.string");
    ADDPROC(setstem,            "map.putstem",        "b",     ".int",   "stem=.string,value=.string");
    ADDPROC(dropstem,           "map.dropstem",       "b",     ".int",   "stem=.string");
    ADDPROC(getstemmsg,         "map.getstemmsg",     "b",     ".string"," ");
    ADDPROC(gettail,            "map.getstemtail",    "b",     ".string","stem=.string,ordinal=.int");
    ADDPROC(getall,             "map.getall",         "b",     ".int",   "stem=.string, expose keys=.string[], expose values=.string[]");
    ADDPROC(getalltails,        "map.getalltails",    "b",     ".int",   "stem=.string, expose keys=.string[]");
    ADDPROC(dropentry,          "map.dropentry",      "b",     ".int",   "stem=.string,key=.string");
    ADDPROC(liststems,          "map.liststems",      "b",     ".int",   "expose stems=.string[]");
    ADDPROC(clonestem,          "map.clonestem",      "b",     ".int",   "source=.string,target=.string");
    ADDPROC(statsstem,          "map.stemstat",       "b",     ".string","stem=.string");
    ADDPROC(stemquote,          "map.stemquote",      "b",     ".string","path=.string");
ENDLOADFUNCS

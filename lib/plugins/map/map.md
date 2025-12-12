# CREXX Stem Engine
### High-Performance Multi-Stem Storage Using Robin Hood Hashing

---

## 1. Overview

The CREXX Stem Engine provides a fast, memory-efficient, multi-stem storage subsystem based on **Robin Hood open-addressing hash maps**.  
It supports full REXX compound variable semantics:

- Multiple independent stems (`A.`, `B.`, `MYDATA.`, …)
- Arbitrary tail strings (e.g. `a.b.c.1500`)
- REXX semantics for stem case sensitivity:
    - **Stem name** → always uppercased (`Fred.` → `FRED.`)
    - **Tail** → preserved exactly as passed (`A.b.C`)
- Undefined stems/elements return `"STEM.TAIL"` per REXX rules
- Full enumeration and management primitives

This file documents the stem subsystem as implemented in C for a CREXX plugin.

---

## 1.5 Robin Hood Hashing (Brief Description)

Robin Hood hashing is an open-addressing collision‑resolution strategy designed to keep probe distances *evenly balanced* across the table. Its guiding principle is:

**“Steal from the rich, give to the poor.”**

Meaning:
- When inserting, if the incoming key has probed farther than the existing key in a slot, the two swap places.
- This ensures that no entry gets “too far” from where it ideally belongs.

### Benefits
- **Dynamic growth**: the table automatically expands as needed. No predetermined fixed size is required.
- **Lower maximum probe length** → faster lookups
- **More predictable performance** even at high load factors
- **Very cache‐friendly** due to contiguous storage
- **Lower maximum probe length** → faster lookups
- **More predictable performance** even at high load factors
- **Very cache‐friendly** due to contiguous storage

### Why it works well for stems
- GETSTEM and SETSTEM become consistently fast
- Few rehashes needed, even for tens of thousands of elements
- No pointer chasing (unlike chained hash tables)

---

## 2. Architecture

The engine uses **two hashmap layers**, both using the same Robin Hood algorithm:

```
Root Map
   key:   stem name (uppercase)
   value: pointer to per-stem map

Stem Map
   key:   tail string
   value: strdup’d value string
```

Each stem behaves like an independent namespace.

---

## 3. Data Structures

### `rhmap_slot_t`
```c
typedef struct rhmap_slot {
    uint32_t   hash;
    char      *key;     /* owned by map */
    void      *value;   /* owned by caller in stem maps */
    bool       used;
} rhmap_slot_t;
```

### `rhmap_t`
```c
typedef struct rhmap {
    rhmap_slot_t *slots;
    size_t        capacity;
    size_t        size;
    double        max_load;
    size_t        rehash_count;
} rhmap_t;
```

### Global root map
```c
static rhmap_t g_root_map = {0};
```

### Per-stem map
Alias:
```c
typedef rhmap_t stem_map_t;
```

---

## 4. Error Handling

The engine uses a single global status variable:

```c
static int last_rhmap_rc = 0;
```

Codes:
```c
enum {
    STEM_MSG_OK               = 0,
    STEM_MSG_UNDEFINED_STEM   = 10,
    STEM_MSG_UNDEFINED_ELEM   = 11,
    STEM_MSG_INTERNAL_ERROR   = 99
};
```

Retrieve message text via:

```
map.getstemmsg
```

---

## 5. CREXX Procedures

The module exports the following functions:

| CREXX Name               | Purpose |
|--------------------------|---------|
| `getstem`        | Get a stem element |
| `setstem`        | Set a stem element |
| `dropstem`       | Delete entire stem |
| `clonestem`      | Clone stem A → B |
| `getstemmsg`     | Retrieve message text |
| `rehashstem`     | Rehash count for a stem |
| `getstemtail`    | Get Nth tail (unordered) |
| `getall`         | Get all tails + values |
| `getalltails`    | Get all tails only |
| `liststems`      | List all stem roots |
| `stemstats`      | Get statistics for one stem |

All procedures work with the CREXX/PA calling conventions.

---

## 6. Behavior and Semantics

### 6.1 Setting Stem Values
```rexx
call setstem "Fred", "A.B", "Hello"
```
Internally stored as:
```
FRED.A.B = "Hello"
```

### 6.2 Getting Stem Values

Existing:
```rexx
say getstem("Fred","A.B")
---> Hello
```

Undefined element:
```rexx
say getstem("Fred","ZZ")
---> FRED.ZZ
say getstemmsg()
---> Stem element not defined
```

Undefined stem:
```rexx
say getstem("NoSuch","X")
---> NOSUCH.X
say getstemmsg()
---> Stem not defined
```

### 6.3 Dropping a Stem
```rexx
call dropstem "Fred"
```
Deletes the entire stem map.

### 6.4 Cloning a Stem
```rexx
count = clonestem("SRC", "DST")
```
`DST` is removed first if it exists.

---

## 7. Enumeration

### 7.1 `getstemtail(stem, ordinal)`

Enumerate tails in hash order:
```rexx
i = 1
do while 1
    t = getstemtail("DATA", i)
    if t = "" then leave
    say t getstem("DATA", t)
    i = i + 1
end
```
Returns empty string when no more items.

### 7.2 `getall`
```rexx
n = getall("FRED", keys., vals.)
```
Fills:
- `keys.0 ... keys.n-1`
- `vals.0 ... vals.n-1`

### 7.3 `getalltails`
```rexx
n = getalltails("FRED", keys.)
```

---

## 8. Listing All Defined Stems
```rexx
n = liststems(stems.)
do i = 1 to n
    say stems.i
end
```

---

## 9. Stem Statistics

```rexx
say stemstats("FRED")
```
Example output:
```
STEM FRED: size=5000 capacity=8192 load=0.61 rehash=7
```
If undefined:
```
STEM FRED: (undefined)
```

---

## 10. Memory Management

- Keys in any map are `strdup`’d → freed by the map.
- Values in stem maps are `strdup`’d → freed by stem_map_destroy.
- Dropping a stem frees:
    - All tail keys
    - All value strings
    - Slot array
    - The stem map struct

Root map is never freed until the interpreter shuts down.

---

## 11. Performance

Robin Hood hashing provides:

- Very fast average lookup and insertion (O(1))
- Small probe distances (stable performance)
- High load factors (≈80%)
- Predictable growth
- Efficient memory layout (contiguous cache-friendly arrays)

The implementation also tracks:

```
rehash_count
```

allowing developers to monitor stress levels of stem maps.

---

## 12. Limitations and Notes

- Enumeration order is **not sorted** (hash order).
- Not (yet) thread-safe.
- Tails are treated as raw strings (no type parsing).
- Case sensitivity is REXX-compliant:
    - Stem name is uppercase.
    - Tail is preserved.

---

## 15. Maintainer Notes

- Keep stem names uppercase before lookup.
- Always check `getstemmsg()` after reading from a stem.
- `gettail()` is guaranteed to return empty string at end-of-data.


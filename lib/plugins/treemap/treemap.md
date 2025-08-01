# TreeMap CREXX Interface Documentation

This document describes the CREXX TreeMap plugin functionality.
Each TreeMap is uniquely identified by a 64-bit integer handle (int64) and may be optionally associated with a name at creation time.
---

## Function Index
- [`tmcreate`](#tmcreate)
- [`tmlookup`](#tmlookup)
- [`tmput`](#tmput)
- [`tmget`](#tmget)
- [`tmhaskey`](#tmhaskey)
- [`tmhasvalue`](#tmhasvalue)
- [`tmfirstkey`](#tmfirstkey)
- [`tmlastkey`](#tmlastkey)
- [`tmremove`](#tmremove)
- [`tmkeys`](#tmkeys)
- [`tmdump`](#tmdump)
- [`tmsize`](#tmsize)
- [`tmfree`](#tmfree)

---

## `tmcreate`
**Description:** Creates a new TreeMap and registers it with a given name.

**Arguments:**
- `ARG0`: Optional name (string). If empty, defaults to `UNNAMED`. Automatically converted to uppercase.

**Returns:**
- TreeMap handle as `int64`.

---

## `tmlookup`
**Description:** Looks up a TreeMap handle by its name.

**Arguments:**
- `ARG0`: Name of the map (string). Converted to uppercase. Defaults to `UNNAMED` if empty.

**Returns:**
- TreeMap handle as `int64`.

---

## `tmput`
**Description:** Inserts a key-value pair into the specified TreeMap.

**Arguments:**
- `ARG0`: TreeMap handle (int64).
- `ARG1`: Key (string).
- `ARG2`: Value (string).

**Returns:**
- `0` on success. Error signal on invalid map.

---

## `tmget`
**Description:** Retrieves the value for a given key from the specified TreeMap.

**Arguments:**
- `ARG0`: TreeMap handle (int64).
- `ARG1`: Key (string).

**Returns:**
- Value (string), or error signal if map is invalid.

---

## `tmhaskey`
**Description:** Checks if the specified key exists in the TreeMap.

**Arguments:**
- `ARG0`: TreeMap handle (int64).
- `ARG1`: Key (string).

**Returns:**
- `1` if key is present, `0` otherwise.

---

## `tmhasvalue`
**Description:** Searches the TreeMap for a given value.

**Arguments:**
- `ARG0`: TreeMap handle (int64).
- `ARG1`: Value (string).

**Returns:**
- Key (string) associated with the value if found, else empty string.

---

## `tmfirstkey`
**Description:** Returns the first key (smallest) in the TreeMap.

**Arguments:**
- `ARG0`: TreeMap handle (int64).

**Returns:**
- First key (string).

---

## `tmlastkey`
**Description:** Returns the last key (largest) in the TreeMap.

**Arguments:**
- `ARG0`: TreeMap handle (int64).

**Returns:**
- Last key (string).

---

## `tmremove`
**Description:** Removes a key (and its value) from the TreeMap.

**Arguments:**
- `ARG0`: TreeMap handle (int64).
- `ARG1`: Key to remove (string).

**Returns:**
- 0 if remove was successful
- 4 if entry is not available

---

## `tmkeys`
**Description:** Retrieves all keys in sorted order from the TreeMap.

**Arguments:**
- `ARG0`: TreeMap handle (int64).
- `ARG1`: Target Rexx array name for key values.

**Returns:**
- Number of keys added to the array.

---

## `tmsize`
**Description:** Returns the number of entries in the TreeMap.

**Arguments:**
- `ARG0`: TreeMap handle (int64).

**Returns:**
- numer of tree entries.

---

## `tmdump`
**Description:** Dumps all key-value pairs to Rexx arrays.

**Arguments:**
- `ARG0`: TreeMap handle (int64).
- `ARG1`: Target Rexx array name for keys.
- `ARG2`: Target Rexx array name for values.

**Returns:**
- Number of key-value pairs added.

---

## `tmfree`
**Description:** Frees all entries and the memory associated with a TreeMap.

**Arguments:**
- `ARG0`: TreeMap handle (int64).

**Returns:**
- Always returns `0`. (No return code from destroy function.)

---

## Notes
- All string comparisons and lookups are case-insensitive (converted to uppercase).
- `is_valid_map()` is used in most procedures to guard against invalid handles.
- `SIGNAL_INVALID_ARGUMENTS` is raised when operations are attempted on an invalid or nonexistent map.


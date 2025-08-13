# ID Library Documentation

## Overview
The **ID Library** provides functions for generating unique identifiers in multiple formats.
It extends Adrian's `rxuuid` plugin by adding more identifier types and is designed to be **cross-platform** (Windows, Linux, macOS).
Currently tested only on Windows, but the code is platform-agnostic.

Supported ID types:
- UUID (v4 random)
- UUIDv7 (time-ordered per RFC 9562)
- ULID (Universally Unique Lexicographically Sortable Identifier)
- NanoID (small, URL-friendly random IDs)
- Snowflake IDs (time-ordered 64-bit integers)
- Base58 IDs (compact, human-friendly random strings)

---

## ID Types

### 1. UUID (Version 4)
**Description:** A 128-bit identifier generated using random numbers, per [RFC 4122](https://www.rfc-editor.org/rfc/rfc4122).
**Use case:** General-purpose unique IDs where cryptographic randomness is desired.

**REXX Example:**
```rexx
say uuid()
## Example output: 550e8400-e29b-41d4-a716-446655440000
```

---

### 2. UUIDv7
**Description:** A 128-bit identifier with millisecond-precision timestamp as the most significant bits. Designed for time ordering and uniqueness, per [RFC 9562](https://www.rfc-editor.org/rfc/rfc9562).
**Use case:** Databases and distributed systems requiring ordered unique IDs.

**REXX Example:**
```rexx
say uuidv7()
## Example output: 01890f51-b7e6-7cc0-bf37-8f92cdecff99
```

---

### 3. ULID
**Description:** 128-bit identifier, sortable by creation time, but using Crockford's Base32 encoding for compactness and human readability.
**Use case:** Similar to UUIDv7 but more human-friendly and URL-safe.

**REXX Example:**
```rexx
say ulid()
## Example output: 01HZY0B9ZN4T14QZKZEMB7S6FJ
```

---

### 4. NanoID
**Description:** Small, secure, URL-friendly random ID generator. Default size is 21 characters, providing ~128-bit randomness.
**Use case:** Short, unique IDs for URLs, API keys, etc.

**REXX Example:**
```rexx
say nanoid()
## Example output: LD30tIxo01f0M6N--h7rI
```

---

### 5. Snowflake IDs
**Description:** 64-bit, time-ordered IDs used by Twitter. Combines a timestamp, node ID, and sequence number.
**Use case:** Sequential but distributed-safe IDs for high-performance systems.

**Bit layout:**
```
0 - 41 bits timestamp - 10 bits node ID - 12 bits sequence
```

**REXX Example:**
```rexx
say snowflake()
## Example output: 1955556938294697984
```

---

### 6. Base58 IDs
**Description:** Randomly generated IDs encoded with Bitcoin's Base58 alphabet (`123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz`), avoiding confusing characters like `0`, `O`, `I`, and `l`.
**Use case:** Short, human-friendly IDs that are still secure. Good for coupons, referral codes, or public tokens.

**REXX Example:**
```rexx
say base58()
## Example output: 7kjsZaSMgBoLtNUqaEBRZT
```

---

## Comparison Table

| Type        | Size    | Sortable | Encoding  | Use Case |
|-------------|---------|----------|-----------|----------|
| UUID v4     | 128-bit | No       | Hex       | General-purpose unique IDs |
| UUIDv7      | 128-bit | Yes      | Hex       | Ordered DB keys, logs |
| ULID        | 128-bit | Yes      | Base32    | Human-readable ordered IDs |
| NanoID      | ~128-bit| No       | URL-safe  | Short secure IDs |
| Snowflake   | 64-bit  | Yes      | Integer   | High-perf sequential IDs |
| Base58      | ~128-bit| No       | Base58    | Human-friendly random codes |

---

## Choosing the Right ID
- **Need ordering?** → UUIDv7, ULID, or Snowflake.
- **Need compact and human-friendly?** → ULID or Base58.
- **Need short URL-safe random IDs?** → NanoID.
- **Need cryptographic randomness?** → UUIDv4 or NanoID.
- **Need fast, sortable numeric IDs?** → Snowflake.

---

## REXX Function Reference

| Function          | Plugin Name    | Returns       |
|-------------------|---------------|---------------|
| `rxuuid.uuid()`   | `rxuuid`       | UUID v4 string |
| `uuidv7()`  | `idlib`        | UUIDv7 string |
| `ulid()`    | `idlib`        | ULID string   |
| `nanoid()`  | `idlib`        | NanoID string |
| `snowflake()` | `idlib`      | Snowflake integer |
| `base58()`  | `idlib`        | Base58 string |

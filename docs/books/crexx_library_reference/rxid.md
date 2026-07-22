# The .rxid identifier generator

## Identifier Types

UUID (Version 4)
: Random-based Universally Unique Identifier as specified in RFC 4122.
Uses a cryptographically secure pseudo-random number generator (CSPRNG) to produce 128-bit values with no meaningful embedded metadata.

Usage:
Ideal when you need a strong, globally unique value without exposing generation time or location.
Function: rxuuid.uuid

UUIDt (Simple Random)
: A simple, quick-to-generate version of UUIDv4 that uses the rand() function instead of a CSPRNG.
This makes it faster but not cryptographically secure.
Best suited for non-security-critical use cases such as temporary IDs or internal testing.

Usage:
Use for lightweight operations where uniqueness is important but absolute security is not.
Function: rxuuid.uuidt

UUIDv7
: A time-ordered UUID format defined in RFC 9562.
Encodes the current Unix timestamp in milliseconds plus random bits for uniqueness.
Maintains lexicographical ordering by creation time, making it highly efficient for database indexing and sorting.

Usage:
Perfect for scenarios where chronological ordering and high performance in queries are required, such as event logs or time-series data.
Function: idlib.uuidv7

ULID
: The Universally Unique Lexicographically Sortable Identifier.
Similar to UUIDv7, but uses a Crockford’s Base32 encoding, producing a 26-character string that is URL-safe and lexicographically sortable.
Includes a 48-bit timestamp (millisecond precision) and 80 random bits.

Usage:
Great for human-friendly IDs in APIs, URLs, and logs, where sorting by generation time is required but readability matters.
Function: idlib.ulid


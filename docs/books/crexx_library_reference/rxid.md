# Identifier generation with `rxid`

The optional Level G `rxid` native provider supplies six zero-argument string
generators:

| Procedure | Result |
|---|---|
| `uuid4()` | 36-character random UUID with version 4 and RFC variant bits. |
| `uuid7()` | 36-character time-ordered UUID with version 7 and RFC variant bits. |
| `ulid()` | 26-character time-ordered Crockford Base32 ULID. |
| `nanoid()` | 21-character URL-safe Nano ID. |
| `snowflake()` | Decimal text representation of a time/node/sequence identifier. |
| `base58()` | Bitcoin-alphabet Base58 text generated from random bytes. |

```rexx
options levelg
import rxid

request_id = rxid..uuid4()
ordered_id = rxid..uuid7()
```

UUIDv4, UUIDv7, ULID, Nano ID, and Base58 randomness comes from the platform
cryptographically secure random source. UUIDv7 and ULID generation serialize
their monotonic same-millisecond state; Snowflake maintains its own synchronized
sequence state. A platform random-source failure raises `FAILURE`.

The pre-release private `id._*`, `rxuuid`, and `idlib` draft names have been
retired. Import `rxid` and use the public names above. The provider is installed
in dynamic and static forms and is found automatically from RXBIN dependency
metadata; no Rexx wrapper or explicit VM plugin list is required.

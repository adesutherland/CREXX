<!-- # cREXX Level B JSON Library -->
# The .rxjson library

`rxjson` is the Level B strict JSON foundation library. It is implemented in
cREXX in `lib/rxfnsb/rexx/rxjson.crexx`, is compiled into `library.rxbin`, and does not
require a native plugin.

The legacy functions remain string-oriented. Code that traverses the same JSON
more than once can instead construct one immutable `.jsondocument` and reuse
its private structural index.

```rexx
options levelb
import rxjson

response = '{"choices":[{"message":{"content":"Hello"}}]}'
say jsonget(response, "choices.1.message.content")
```

## Import

```rexx
import rxjson
```

The module exposes:

- `.jsondocument(json)`
- `jsonvalid(json)`
- `jsontype(json, path)`
- `jsonget(json, path)`
- `jsoncount(json, path)`
- `jsonmembers(json, path, names[])`
- `jsonscancontainer(text, from, document, start, after)`
- `jsonquote(text)`
- `jsonunquote(json)`
- `jsonarray(values[])`
- `jsonobject(keys[], values[])`

## Value Model

The legacy functions accept and return JSON values as strings. A
`.jsondocument` owns the exact source string supplied to its constructor and a
private index of every value. The source cannot be changed through the public
API. Node identifiers are positive, document-local and ephemeral: never retain
them across documents or treat them as an ABI or serialized format. Zero means
that lookup found no node.

`jsonget()` returns Rexx strings:

- JSON strings are unquoted and unescaped.
- JSON numbers are returned as their JSON text.
- JSON booleans are returned as `true` or `false`.
- JSON null is returned as `null`.
- JSON arrays and objects are returned as their JSON substring.
- Missing paths and invalid JSON return the empty string.

Use `jsontype()` when an empty result could be ambiguous.

## Path Syntax

Paths select values inside JSON objects and arrays.

- `""` selects the root value.
- Object keys are separated with dots, for example `usage.total_tokens`.
- Array indexes are one-based, for example `choices.1.message.content`.
- Bracket array indexes are also supported, for example `choices[1].finish_reason`.
- Object keys are case-sensitive.

The simple path syntax is intentionally small. Keys containing dots or square
brackets are not addressable through paths. Use `member(parent, name)` for
arbitrary object keys.

## Immutable Document

Constructing a document validates one complete strict JSON value and builds its
index once:

```rexx
document = .jsondocument('{"items":[{"name":"first"}]}')
if document.valid() then do
  items = document.member(document.root(), "items")
  first = document.element(items, 1)
  say document.node_get(document.member(first, "name"))
end
```

Document status and storage methods are:

| Method | Contract |
| --- | --- |
| `valid()` | `1` for one complete strict JSON value, otherwise `0`. |
| `error_code()` | `none`, `syntax`, `truncated`, or `trailing`. |
| `error_position()` | One-based character position, or `0` when valid. |
| `error()` | Human-readable diagnostic; do not parse it as a stable code. |
| `source()` | The immutable source owned by the document. |
| `source_bytes()` | UTF-8 source size in bytes. |
| `index_bytes()` | Current private index footprint in bytes. |
| `node_count()` | Total number of indexed JSON values. |
| `node_count(node)` | Immediate child count, `0` for a scalar, or `-1` for an invalid node. |

The compatibility methods `type(path)`, `get(path)`, `count(path)`, and
`members(path, names[])` have the same results as their `json*` function
counterparts. `find(path)` returns a positive node identifier, `0` for a
missing path, or `-1` for an invalid document or path.

Use node traversal when a key cannot be represented by the legacy path syntax
or when repeated traversal should avoid reparsing:

| Method | Contract |
| --- | --- |
| `root()` | Root node, or `0` for an invalid document. |
| `member(parent, name)` | Exact decoded object-key lookup: positive node, `0` missing, `-1` invalid node/document, `-2` wrong container type. |
| `element(parent, index)` | One-based array lookup with the same status convention as `member()`. |
| `node_type(node)` | `object`, `array`, `string`, `number`, `boolean`, `null`, or `error`. |
| `node_get(node)` | Decoded string value or exact JSON source text; empty for an invalid node. |
| `children(node, nodes[])` | Replaces `nodes[]` with immediate child IDs in source order; returns their count, `0` for a scalar, or `-1` for invalid input. |
| `node_name(node)` | Decoded object key or one-based array index; empty for a root or invalid node. |

Typed getters write through an exposed result and return a status. They never
coerce JSON null or a different JSON type:

| Method | Success value | Additional failures |
| --- | --- | --- |
| `node_string(node, value)` | Decoded `.string` | `-1` invalid, `-2` wrong type. |
| `node_boolean(node, value)` | `.int` `0` or `1` | `-1` invalid, `-2` wrong type. |
| `node_int(node, value)` | Exact signed 64-bit `.int`; integral forms such as `1.0` and `1e3` are accepted. | `-1` invalid, `-2` wrong type, `-3` non-integral, `-4` range. |
| `node_float(node, value)` | Finite binary64 `.float` | `-1` invalid, `-2` wrong type, `-4` overflow or underflow. |

### Explicit packed numeric projection

Numeric JSON arrays remain ordinary JSON arrays. A caller that explicitly
chooses a numeric width may project an indexed array directly to an owning,
headerless, canonical-little-endian `.binary`:

| Method | Successful output |
| --- | --- |
| `node_f32_array(node, expected_count, packed, error)` | Exactly `count * 4` bytes containing IEEE binary32 values. |
| `node_i64_array(node, expected_count, packed, error)` | Exactly `count * 8` bytes containing signed 64-bit integers. |

`packed` and `error` are exposed outputs. `expected_count` is `-1` to accept
the JSON array's count, or a required non-negative count. Success returns `0`,
replaces `packed`, and clears `error`. Failure clears `packed`, writes a
diagnostic, and returns `-1` for an invalid document/node, `-2` for a non-array
node, `-3` for an invalid expected count, `-4` for a count mismatch, `-5` for a
non-number element, or `-6` for conversion/range failure.

The f32 projection rejects overflow and any nonzero JSON value that underflows
to stored zero; conversion signals are reported as status `-6` rather than
escaping the method. The i64 projection accepts exact integral decimal and
exponent forms such as `1.0` and `1e3`, and rejects non-integral or out-of-range
values. Empty arrays produce empty binaries when the count policy permits.
There is no type inference, header, magic, version, normalization flag or
hidden cache. Persisted users must retain the chosen type and count in their
own schema.

## API Reference

### `jsonvalid(json) = .int`

Returns `1` when `json` is a complete valid JSON value, otherwise `0`.

```rexx
ok = jsonvalid('{"ready":true}')
```

### `jsontype(json, path) = .string`

Returns the type at `path`.

Possible return values are:

- `object`
- `array`
- `string`
- `number`
- `boolean`
- `null`
- `missing`
- `error`

`missing` means the JSON was valid but the path did not resolve. `error` means
the JSON or path expression could not be parsed.

```rexx
kind = jsontype('{"ready":true}', "ready")
```

### `jsonget(json, path) = .string`

Returns the value at `path` as a Rexx string. Strings are unquoted; scalar JSON
values are returned as their JSON spelling; object and array values are returned
as JSON text.

```rexx
content = jsonget(response, "choices.1.message.content")
tokens = jsonget(response, "usage.total_tokens")
```

### `jsoncount(json, path) = .int`

Returns the number of members in an object or elements in an array.

- Returns `0` for scalar values.
- Returns `-1` for invalid JSON or missing paths.

```rexx
count = jsoncount(response, "choices")
```

### `jsonmembers(json, path, names[]) = .int`

Writes member names or array indexes into `names[]` and returns the count.

- Object members write their object keys.
- Array members write one-based indexes as strings.
- Returns `-1` for invalid JSON or missing paths.

Pass a fresh `.string[]` when possible. The function writes `names[1]` through
`names[count]`; it does not clear older entries beyond the returned count.

```rexx
names = .string[]
count = jsonmembers(response, "", names)
do i = 1 to count
  say names[i]
end
```

### `jsonscancontainer(text, from, document, start, after) = .int`

Finds the first complete strict JSON object or array whose opening delimiter is
at or after the one-based character position `from`. On success it returns `1`,
writes an already indexed `.jsondocument`, and writes one-based character
positions `start` (inclusive) and `after` (one past the value). Pass `after` as
the next `from` value to continue scanning. Values that are only JSON scalars
are not candidates.

On failure it returns `0`, sets `start` and `after` to zero, and returns an
invalid empty document. Values below `1` are normalized to `1`; a position past
the input returns no match. The scanner identifies structural JSON only. It
does not decide whether a found value satisfies an application contract, so a
caller may validate the document and continue from `after` when necessary.

```rexx
text = 'prefix {"kind":"first"} then {"kind":"second"}'
document = .jsondocument("")
start = 0
after = 0
if jsonscancontainer(text, 1, document, start, after) then do
  say document.get("kind")
  if jsonscancontainer(text, after, document, start, after) then
    say document.get("kind")
end
```

The successful-slice construction deliberately uses two grammar passes: one
allocation-free boundary-validation pass over the original input, followed by
one indexing pass that owns the exact slice and builds its sole index. Rejected
candidate openers construct no candidate document or index; the public call
initializes its output to one invalid empty document so failure has a
deterministic result. This lifecycle avoids copying every candidate while
keeping `.jsondocument` construction and ownership uniform.

### `jsonquote(text) = .string`

Returns `text` as a JSON string literal, including surrounding quotes.

```rexx
name = jsonquote('Ada "Lovelace"')
```

### `jsonunquote(json) = .string`

Returns the Rexx string represented by a JSON string literal. If `json` is not a
valid JSON string, returns the empty string.

```rexx
text = jsonunquote('"hello \"json\""')
```

### `jsonarray(values[]) = .string`

Builds a JSON array from an array of already encoded JSON values. Returns the
empty string if any element is not valid JSON.

```rexx
values = .string[]
values[1] = jsonquote("system")
values[2] = jsonquote("hello")
roles = jsonarray(values)
```

### `jsonobject(keys[], values[]) = .string`

Builds a JSON object from string keys and already encoded JSON values. Keys are
quoted by `jsonobject()`. Values must already be valid JSON. Returns the empty
string when key and value counts differ or any value is invalid JSON.

```rexx
keys = .string[]
values = .string[]
keys[1] = "role"
values[1] = jsonquote("user")
keys[2] = "content"
values[2] = jsonquote("Say hi")
message = jsonobject(keys, values)
```

## Constructing JSON Example

This example builds a small nested JSON value from already encoded values.

```rexx
options levelb
import rxjson

item_keys = .string[]
item_values = .string[]
item_keys[1] = "name"
item_values[1] = jsonquote("sample")
item_keys[2] = "enabled"
item_values[2] = "true"
item = jsonobject(item_keys, item_values)

items = .string[]
items[1] = item

body_keys = .string[]
body_values = .string[]
body_keys[1] = "version"
body_values[1] = "1"
body_keys[2] = "items"
body_values[2] = jsonarray(items)

body = jsonobject(body_keys, body_values)
```

## Current Limits

- `rxjson` is not a schema validator or repair engine. Required fields,
  additional fields, nullability, and application-specific selection remain
  caller policy.
- Paths cannot address object keys that contain dots or square brackets; use
  document node traversal for those keys.
- Object key matching is case-sensitive.
- Unicode `\uXXXX` unquoting decodes Unicode code points and paired UTF-16
  surrogate escapes; malformed or unpaired surrogate escapes are rejected.
- Duplicate object key handling follows first-match lookup behaviour today; do
  not depend on duplicates in new data.
- JSON numeric arrays remain ordinary JSON arrays. The library does not infer
  integer widths, floating-point widths, byte order, or packed storage.
- The index representation and node identifiers are private and may change
  between builds.

## Tests

Functional coverage lives in:

- `lib/rxfnsb/tests_functional/ts_rxjson.crexx`
- `lib/rxfnsb/tests_functional/ts_rxjson_document.crexx`
- `lib/rxfnsb/tests_functional/ts_rxjson_noisy_contract.crexx`

The focused test target is:

```sh
ctest --test-dir cmake-build-debug -R '^ts_rxjson' --output-on-failure
```

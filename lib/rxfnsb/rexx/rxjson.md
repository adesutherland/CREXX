# cREXX Level B JSON Library

`rxjson` is the Level B JSON foundation library. It is implemented in Rexx in
`lib/rxfnsb/rexx/rxjson.rexx`, is compiled into `library.rxbin`, and does not
require a native plugin.

The first contract is deliberately string-oriented. JSON values are passed as
`.string` values, arrays of JSON values are passed as `.string[]`, and callers
extract fields using a simple Rexx-friendly path syntax.

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

- `jsonvalid(json)`
- `jsontype(json, path)`
- `jsonget(json, path)`
- `jsoncount(json, path)`
- `jsonmembers(json, path, names[])`
- `jsonquote(text)`
- `jsonunquote(json)`
- `jsonarray(values[])`
- `jsonobject(keys[], values[])`

## Value Model

JSON documents and nested JSON values are represented as strings. This keeps the
library usable for web-service requests, LLM provider APIs, ADDRESS transport
experiments, and later REXXSAA-style adapters without committing to a full object
mapper too early.

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
brackets are not addressable yet; add an escaped path syntax before relying on
those key shapes.

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

## LLM Request Example

This example builds a small chat-style request body without using provider
specific code.

```rexx
options levelb
import rxjson

msg_keys = .string[]
msg_values = .string[]
msg_keys[1] = "role"
msg_values[1] = jsonquote("user")
msg_keys[2] = "content"
msg_values[2] = jsonquote("Say hi")
message = jsonobject(msg_keys, msg_values)

messages = .string[]
messages[1] = message

body_keys = .string[]
body_values = .string[]
body_keys[1] = "model"
body_values[1] = jsonquote("demo-model")
body_keys[2] = "messages"
body_values[2] = jsonarray(messages)

body = jsonobject(body_keys, body_values)
```

## Current Limits

- This is a foundation parser/builder, not a full JSON object model.
- Paths cannot yet address object keys that contain dots or square brackets.
- Object key matching is case-sensitive.
- Unicode `\uXXXX` unquoting currently handles ASCII code points directly and
  substitutes `?` for non-ASCII code points.
- Duplicate object key handling follows first-match lookup behaviour today; do
  not depend on duplicates in new data.

## Tests

Functional coverage lives in:

- `lib/rxfnsb/tests_functional/ts_rxjson.rexx`

The focused test target is:

```sh
ctest --test-dir cmake-build-debug -R '^ts_rxjson' --output-on-failure
```

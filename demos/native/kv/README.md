# Native KV ADDRESS Environment Demo

This demo is the native counterpart to the Rexx CMS ADDRESS demo. It keeps the
provider deliberately small: a C-backed in-memory key/value store registered as
`ADDRESS KV`.

It demonstrates the same protocol surface as the Rexx provider:

- ADDRESS instructions dispatch to a native C callback.
- `_address_environment("kv")` returns an object implementing
  `.addressinstance`.
- `_address_call("kv", name, ...)` calls native ADDRESS functions and returns a
  string.
- Native commands can now emit to `ADDRESS ... output out`, matching Rexx
  providers.
- Quoted ADDRESS commands can use host-variable anchors. `:name` and `${name}`
  auto-expose the matching Rexx scalar to the native request; the KV handler
  decides how to resolve those anchors.

Build and run it from the repository root:

```sh
cmake --build cmake-build-debug --target native_kv_address_demo
cmake-build-debug/compiler/tests/native_kv_address_demo
```

The native commands are:

- `PUT key value`
- `GET key`
- `GET key INTO target`
- `DEL key`
- `CLEAR`
- `COUNT`
- `COUNT INTO target`
- `DUMP`

For example, the demo uses:

```rexx
key = "user"
value = "Adrian"
"PUT :key :value"
"GET :key INTO ${fetched}"
```

The native functions are:

- `ID()`
- `GET(key)`
- `PUT(key, value)`
- `COUNT()`
- `EXISTS(key)`
- `KEYS()`

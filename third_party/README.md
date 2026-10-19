# Root-level dependencies

`re2c` (scanner generator), `lemon` (parser generator), `avl_tree` (AVL support)
and `utf8/utf.h` (UTF-8 support) retain their upstream sources, notices and local
patches. These moves do not upgrade or prune a dependency. Upstream tests and
bootstrap files are deliberate offline build inputs, not disposable output.
The first-party scanner fragment lives at `platform/encoding.re`.
Package-specific vendors, including SQLite and nlohmann, stay with their owners.
